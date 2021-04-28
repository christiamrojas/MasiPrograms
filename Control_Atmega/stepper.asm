/*
 * stepper.asm
 *
 *  Created: 09/05/2020 05:25:35 p. m.
 *   Author: Javier
 */ 

.equ	stepper_pos_max				= 2800;2800 ;2650 ;2750 ;2768
.equ	stepper_zero_offset			= -300 ;-10
.equ	stepper_acceleration_max	= 8		; [1,63]
.equ	stepper_sw_open_pin			= 1			; PORTE1
.equ	stepper_sw_close_pin		= 2			; PORTE2


.dseg
stepper_speed_sp:			.byte 2					; LO-HI
stepper_speed_cur:			.byte 2					; LO-HI
stepper_pos_cur:			.byte 2					; LO-HI
stepper_alarm:				.byte 1					; bit0=alarm enable, bit1=alarm detected
stepper_pressure_alarm:		.byte 1
stepper_flow_alarm:			.byte 1
stepper_pos_alarm:			.byte 1
stepper_sensors_alarm:		.byte 1
stepper_disconnect_alarm:	.byte 1
stepper_obstruction_alarm:	.byte 1
stepper_resistance_alarm:	.byte 1


.cseg
.macro stepper_read_pos
	cli
	lds		XL,stepper_pos_cur
	lds		XH,stepper_pos_cur+1
	sei
.endm

.macro stepper_write_pos
	cli
	sts		stepper_pos_cur,XL
	sts		stepper_pos_cur+1,XH
	sei
.endm

.cseg
stepper_init:
 	ldi		R16,0x07												; pulse,dir,enable config
	sts		PORTB_OUTCLR,R16
	sts		PORTB_DIRSET,R16
	ldi		R16,0x00
	sts		PORTB_PIN0CTRL,R16
	sts		PORTB_PIN1CTRL,R16
	sts		PORTB_PIN2CTRL,R16
	ldi		R16,PORTMUX_TCA0_PORTB_gc
	sts		PORTMUX_TCAROUTEA,R16

	ldi		R16,0x00
	sts		TCA0_SINGLE_CTRLA,R16									; stop TCA0
	sts		TCA0_SINGLE_CTRLD,R16									; no split

	ldi		R16,TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_FRQ_gc		; frequency on cmp0 (PB0)
	sts		TCA0_SINGLE_CTRLB,R16
	ldi		R16,TCA_SINGLE_OVF_bm
	sts		TCA0_SINGLE_INTFLAGS,R16								; tca_ovf interrupt flag clear
	sts		TCA0_SINGLE_INTCTRL,R16									; tca_ovf interrupt enable

	rcall	stepper_alarm_disable									; clear alarm flag

	ldi		XL,0
	ldi		XH,0
	sts		stepper_speed_sp,XL										; speed to 0
	sts		stepper_speed_sp+1,XH
	rcall	stepper_write_speed

	ldi		R16,TCA_SINGLE_CLKSEL_DIV8_gc | TCA_SINGLE_ENABLE_bm	; start frequency
	sts		TCA0_SINGLE_CTRLA,R16
	ret


; *** isr_TCA_ovf -> stepper interrup service rutine
; - move stepper according to stepper_speed_sp and stepper_speed_cur
; - if pressure > PRESSURE_MAX stops
; - if flow > FLOW_MAX stops
; - if stepper_pos_cur out of range stops

isr_TCA_ovf:
	push	R16
	in		R16,CPU_SREG
	push	R16

	ldi		R16,TCA_SINGLE_OVF_bm				; tca_ovf interrupt flag clear
	sts		TCA0_SINGLE_INTFLAGS,R16

	lds		R16,PORTB_IN						; pulse (PB0) low?
	sbrc	R16,0
	rjmp	isr_TCA_ovf_high_ret
	
	push	R17
	push	XL
	push	XH
	push	ZL
	push	ZH

; *** update stepper position
	lds		XL,stepper_pos_cur					; current position (signed)
	lds		XH,stepper_pos_cur+1
	lds		R16,TCA0_SINGLE_CTRLB
	sbrs	R16,TCA_SINGLE_CMP0EN_bp			; frequency on cmp0 (PB0)
	rjmp	isr_TCA_ovf_no_count
	lds		R16,PORTB_IN
	sbrs	R16,1								; stepper dir pin
	sbiw	XL,1
	sbrc	R16,1
	adiw	XL,1
	sts		stepper_pos_cur,XL
	sts		stepper_pos_cur+1,XH
isr_TCA_ovf_no_count:


; *** verifies previous alarm
	lds		R16,stepper_alarm
	sbrc	R16,1
	rjmp	isr_TCA_ovf_stop
	cpi		R16,0
	breq	isr_TCA_ovf_noalarm

; *** verifies maximum pressure
	lds		R16,sensors_pressure_value+3
	sbrc	R16,7
	rjmp	isr_TCA_ovf_pressure_nomax
	cpi		R16,0
	brne	isr_TCA_ovf_presssure_alarm
	lds		R16,sensors_pressure_value+2
	lds		R17,EEPROM_START+ee_pressure_max
	cp		R16,R17
	brlo	isr_TCA_ovf_pressure_nomax
isr_TCA_ovf_presssure_alarm:
	ldi		R16,1
	sts		stepper_pressure_alarm,R16
	rjmp	isr_TCA_ovf_alarm
isr_TCA_ovf_pressure_nomax:

; verifies maximum flow
	lds		R16,sensors_flow_value+3
	sbrc	R16,7
	rjmp	isr_TCA_ovf_flow_nomax
	cpi		R16,0
	brne	isr_TCA_ovf_flow_alarm
	lds		R16,sensors_flow_value+2
	lds		R17,EEPROM_START+ee_flow_max
	cp		R16,R17
	brlo	isr_TCA_ovf_flow_nomax
isr_TCA_ovf_flow_alarm:
	ldi		R16,1
	sts		stepper_flow_alarm,R16
	rjmp	isr_TCA_ovf_alarm
isr_TCA_ovf_flow_nomax:

; verifies maximum future position 
	lds		ZL,stepper_speed_sp					; direcction of setpoint speed
	lds		ZH,stepper_speed_sp+1
	sbrc	ZH,7
	rjmp	isr_TCA_ovf_negative_speed
	cpi		XH,high(stepper_pos_max)		; greater or equal to top position?
	brlt	isr_TCA_ovf_update_speed
	brne	isr_TCA_ovf_pos_alarm
	cpi		XL,low(stepper_pos_max)
	brlo	isr_TCA_ovf_update_speed

isr_TCA_ovf_pos_alarm:
	ldi		R16,1
	sts		stepper_pos_alarm,R16
	rjmp	isr_TCA_ovf_alarm

isr_TCA_ovf_noalarm:
	lds		ZL,stepper_speed_sp					; direcction of setpoint speed
	lds		ZH,stepper_speed_sp+1
	sbrc	ZH,7
	rjmp	isr_TCA_ovf_negative_speed

	cpi		XH,high(stepper_pos_max)			; greater or equal to top position?
	brlt	isr_TCA_ovf_update_speed
	brne	isr_TCA_ovf_stop
	cpi		XL,low(stepper_pos_max)
	brlo	isr_TCA_ovf_update_speed
	rjmp	isr_TCA_ovf_stop

	
isr_TCA_ovf_negative_speed:
	sbrs	XH,7								; less to bottom position (=0)?
	rjmp	isr_TCA_ovf_update_speed

;*** out of limits -> set point to stop
isr_TCA_ovf_alarm:
	ldi		R16,3
	sts		stepper_alarm,R16
isr_TCA_ovf_stop:
	ldi		XH,0								; out of range -> stop
	ldi		XL,0
	rjmp	isr_TCA_ovf_update_set

;*** update movement
isr_TCA_ovf_update_speed:
	lds		XL,stepper_speed_cur				; speed change?
	lds		XH,stepper_speed_cur+1
	cp		ZL,XL
	cpc		ZH,XH
	breq	isr_TCA_ovf_stop_ret
	brlt	isr_TCA_ovf_update_speed_dec

	adiw	XL,stepper_acceleration_max			; increment
	cp		ZL,XL
	cpc		ZH,XH
	brge	isr_TCA_ovf_update_set
	mov		XL,ZL
	mov		XH,ZH
	rjmp	isr_TCA_ovf_update_set

isr_TCA_ovf_update_speed_dec:
	sbiw	XL,stepper_acceleration_max			; decrement
	cp		ZL,XL
	cpc		ZH,XH
	brlt	isr_TCA_ovf_update_set
	mov		XL,ZL
	mov		XH,ZH

isr_TCA_ovf_update_set:
	rcall	stepper_write_speed					; speed update

isr_TCA_ovf_stop_ret:
	pop		ZH
	pop		ZL
	pop		XH
	pop		XL
	pop		R17
isr_TCA_ovf_high_ret:
	pop		R16
	out		CPU_SREG,R16
	pop		R16
	reti




stepper_home:
	rcall	stepper_init

	ldi		XH,high(int(stepper_pos_max*1.5))
	ldi		XL,low(int(stepper_pos_max*1.5))
	stepper_write_pos

	ldi		XL,low(-10)						; detect bottom limit switch
	ldi		XH,high(-10)
	sts		stepper_speed_sp,XL
	sts		stepper_speed_sp+1,XH
stepper_home_loop:
;	task_change

	rcall	screen_chk_tx_data_PFV

	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_open_pin			; normal sbrs
	rjmp	stepper_home_stop
	sbrs	R16,stepper_sw_close_pin
	rcall	stepper_home_change_dir
	stepper_read_pos
	or		XL,XH
	brne	stepper_home_loop
stepper_home_stop:
	rcall	stepper_stop

	rcall	stepper_release
	ret




	ldi		YL,low(10)
	ldi		YH,high(10)
	ldi		XH,high(stepper_pos_max)
	ldi		XL,low(stepper_pos_max)
	rcall	stepper_close

	ldi		YL,low(-10)
	ldi		YH,high(-10)
	clr		XL
	clr		XH
	rcall	stepper_open

	ret




stepper_home_change_dir:
	ldi		XH,high(int(-stepper_pos_max*1.5))
	ldi		XL,low(int(-stepper_pos_max*1.5))
	stepper_write_pos

	ldi		XL,low(10)
	ldi		XH,high(10)
	sts		stepper_speed_sp,XL
	sts		stepper_speed_sp+1,XH

	ret







screen_chk_tx_data_PFV:
	call	pid_check_sampling_time			; pid sampling time
	brne	screen_chk_tx_data_PFV_no_data
	rcall	screen_tx_data_PFV				; transmit data
screen_chk_tx_data_PFV_no_data:
	ret



stepper_release:
	clr		XL
	clr		XH
	stepper_write_pos
	ldi		XL,low(10)							; release bottom limit switch
	ldi		XH,high(10)
	sts		stepper_speed_sp,XL
	sts		stepper_speed_sp+1,XH
stepper_home_release:
;	task_change

	rcall	screen_chk_tx_data_PFV

	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_open_pin				; normal sbrs
	rjmp	stepper_home_release
	rcall	stepper_stop

	ldi		XH,high(stepper_zero_offset)
	ldi		XL,low(stepper_zero_offset)
	stepper_write_pos
	ldi		YL,low(10)
	ldi		YH,high(10)
	clr		XL
	clr		XH
	rcall	stepper_close
	ret





.dseg
stepper_close_pos:	.byte 2
.cseg
stepper_close:
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH
	sts		stepper_close_pos,XL
	sts		stepper_close_pos+1,XH
stepper_close_loop:
;	task_change
;	rcall	screen_check_data_tx
	rcall	screen_chk_tx_data_PFV

	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_open_pin				; normal sbrc
	rjmp	stepper_close_no_limit
	ldi		XH,high(stepper_zero_offset)
	ldi		XL,low(stepper_zero_offset)
;	stepper_write_pos
stepper_close_no_limit:
	stepper_read_pos
	lds		YL,stepper_close_pos
	lds		YH,stepper_close_pos+1
	cp		XL,YL
	cpc		XH,YH
	brlt	stepper_close_loop
	rcall	stepper_stop
	ret


.dseg
stepper_open_pos:	.byte 2
.cseg
stepper_open:
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH
	sts		stepper_open_pos,XL
	sts		stepper_open_pos+1,XH
stepper_open_loop:
;	task_change
;	rcall	screen_check_data_tx
	rcall	screen_chk_tx_data_PFV

	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_open_pin				; normal sbrc
	rjmp	stepper_open_limit
	ldi		XH,high(stepper_zero_offset)
	ldi		XL,low(stepper_zero_offset)
;	stepper_write_pos
stepper_open_limit:

	stepper_read_pos
	lds		YL,stepper_open_pos
	lds		YH,stepper_open_pos+1
	cp		YL,XL
	cpc		YH,XH
	brlt	stepper_open_loop
	rcall	stepper_stop
	ret


stepper_stop:
	ldi		R16,0
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16
stepper_stop_loop:
;	task_change
;	rcall	screen_check_data_tx
	rcall	screen_chk_tx_data_PFV

	lds		XL,stepper_speed_cur
	lds		XH,stepper_speed_cur+1
	or		XL,XH
	brne	stepper_stop_loop
	ret




stepper_alarm_enable:
	ldi		R16,0
	sts		stepper_pressure_alarm,R16
	sts		stepper_flow_alarm,R16
	sts		stepper_pos_alarm,R16
	sts		stepper_sensors_alarm,R16
	sts		stepper_disconnect_alarm,R16
	sts		stepper_obstruction_alarm,R16
	ldi		R16,1
	sts		stepper_alarm,R16
	ret


stepper_alarm_disable:
	ldi		R16,0
	sts		stepper_alarm,R16
	sts		stepper_pressure_alarm,R16
	sts		stepper_flow_alarm,R16
	sts		stepper_pos_alarm,R16
	sts		stepper_sensors_alarm,R16
	sts		stepper_disconnect_alarm,R16
	sts		stepper_obstruction_alarm,R16
	sts		stepper_resistance_alarm,R16
	ret


; X = speed
stepper_write_speed:
	sts		stepper_speed_cur,XL				; speed update
	sts		stepper_speed_cur+1,XH
	cpi		XL,0
	brne	stepper_write_speed_no_stop
	cpi		XH,0
	brne	stepper_write_speed_no_stop
	ldi		XH,high(fosc/8/200)					; 0.5ms start up
	ldi		XL,low(fosc/8/200)					; 0.5ms start up
	sts		TCA0_SINGLE_CMP0,XL
	sts		TCA0_SINGLE_CMP0+1,XH
	ldi		R16,TCA_SINGLE_WGMODE_FRQ_gc		; no cmp0 (PB0).
	sts		TCA0_SINGLE_CTRLB,R16
	rjmp	stepper_write_speed_ret
stepper_write_speed_no_stop:

	sbrs	XH,7								; set dir
	rjmp	stepper_write_speed_pos
	ldi		R16,0x02							; set dir expiration
	sts		PORTB_OUTCLR,R16
	com		XH									; neg int16
	neg		XL
	sbci	XH,255 
	rjmp	stepper_write_speed_set
stepper_write_speed_pos:
	ldi		R16,0x02								; set dir inspiration
	sts		PORTB_OUTSET,R16

stepper_write_speed_set:
	ldi		ZH,high(stepper_table_speed2time*2)
	ldi		ZL,low(stepper_table_speed2time*2)
	lsl		XL
	rol		XH
	add		ZL,XL
	adc		ZH,XH
	lpm		XL,Z+
	lpm		XH,Z+
	sts		TCA0_SINGLE_CMP0,XL
	sts		TCA0_SINGLE_CMP0+1,XH
	ldi		R16,TCA_SINGLE_CMP0EN_bm | TCA_SINGLE_WGMODE_FRQ_gc		; frequency on cmp0 (PB0)
	sts		TCA0_SINGLE_CTRLB,R16
stepper_write_speed_ret:
	ret





stepper_table_speed2time:
//.dw  65535  ,50000 ,25000 ,16667 ,12500 ,10000 ,8333  ,7143  ,6250  ,5556  ,5000  ,4545  ,4167  ,3846  ,3571  ,3333
.dw  65535  ,50000 ,25000 ,16667 ,12500 ,10000 ,8333  ,7143  ,6250  ,5556  ,5000  ,4545  ,4167  ,3846  ,3571  ,3333
.dw  3125  ,2941  ,2778  ,2632  ,2500  ,2381  ,2273  ,2174  ,2083  ,2000  ,1923  ,1852  ,1786  ,1724  ,1667  ,1613
.dw  1563  ,1515  ,1471  ,1429  ,1389  ,1351  ,1316  ,1282  ,1250  ,1220  ,1190  ,1163  ,1136  ,1111  ,1087  ,1064
.dw  1042  ,1020  ,1000  ,980 ,962 ,943 ,926 ,909 ,893 ,877 ,862 ,847 ,833 ,820 ,806 ,794
.dw  781 ,769 ,758 ,746 ,735 ,725 ,714 ,704 ,694 ,685 ,676 ,667 ,658 ,649 ,641 ,633
.dw  625 ,617 ,610 ,602 ,595 ,588 ,581 ,575 ,568 ,562 ,556 ,549 ,543 ,538 ,532 ,526
.dw  521 ,515 ,510 ,505 ,500 ,495 ,490 ,485 ,481 ,476 ,472 ,467 ,463 ,459 ,455 ,450
.dw  446 ,442 ,439 ,435 ,431 ,427 ,424 ,420 ,417 ,413 ,410 ,407 ,403 ,400 ,397 ,394
.dw  391 ,388 ,385 ,382 ,379 ,376 ,373 ,370 ,368 ,365 ,362 ,360 ,357 ,355 ,352 ,350
.dw  347 ,345 ,342 ,340 ,338 ,336 ,333 ,331 ,329 ,327 ,325 ,323 ,321 ,318 ,316 ,314
.dw  313 ,311 ,309 ,307 ,305 ,303 ,301 ,299 ,298 ,296 ,294 ,292 ,291 ,289 ,287 ,286
.dw  284 ,282 ,281 ,279 ,278 ,276 ,275 ,273 ,272 ,270 ,269 ,267 ,266 ,265 ,263 ,262
.dw  260 ,259 ,258 ,256 ,255 ,254 ,253 ,251 ,250 ,249 ,248 ,246 ,245 ,244 ,243 ,242
.dw  240 ,239 ,238 ,237 ,236 ,235 ,234 ,233 ,231 ,230 ,229 ,228 ,227 ,226 ,225 ,224
.dw  223 ,222 ,221 ,220 ,219 ,218 ,217 ,216 ,216 ,215 ,214 ,213 ,212 ,211 ,210 ,209
.dw  208 ,207 ,207 ,206 ,205 ,204 ,203 ,202 ,202 ,201 ,200 ,199 ,198 ,198 ,197 ,196
