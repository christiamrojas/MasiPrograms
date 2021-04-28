/*
 * sensors.asm
 *
 *  Created: 11/05/2020 08:53:52 p. m.
 *   Author: Javier
 */ 

.include "adc.asm"
.include "i2c.asm"
.include "fir_filter.asm"


.equ bat_adc_BPS = 9600

.equ sensors_pressure_sampling_ms	= 5
.equ sensors_flow_sampling_ms		= 2
.equ sensors_oxygen_sampling_ds		= 2
.equ sensors_VDD33_sampling_ds		= 10
.equ sensors_VDD50_sampling_ds		= 10
.equ sensors_current_sampling_cs	= 1

;.equ sensors_pressure_offset		= 1500*16;1495*16				; 4 fractional bits	
.equ sensors_pressure_offset		= 0*16				; 4 fractional bits
.equ sensors_flow_offset			= 8191	
;.equ sensors_flow_offset			= 8166;8187;8191					; integer
;.equ sensors_flow_offset			= 8186								; masi3. sn: 0000
;.equ sensors_flow_offset			= 8159								; SN:0001
;.equ sensors_flow_offset			= 8171								; SN:0002
.equ sensors_oxygen_offset			= 13312					; 4 fractional bits	
.equ sensors_VDD33_offset			= 0						; 4 fractional bits	
.equ sensors_VDD50_offset			= 0						; 4 fractional bits	
.equ sensors_current_offset			= 0						; 4 fractional bits	

.equ sensors_flow_address			= 0x28

.dseg
sensors_pressure_rdy:			.byte 1
sensors_pressure_tmr_ms:		.byte 1
sensors_pressure_value:			.byte 4		; frac(L-H), int16 (L-H)
sensors_pressure_buffer:		.byte 2*(fir_filter_pressure_end-fir_filter_pressure)
sensors_pressure_bufffer_end:	.byte 0

sensors_flow_rdy:				.byte 1
sensors_flow_tmr_ms:			.byte 1
sensors_flow_value:				.byte 4		; frac(L-H), int16 (L-H)
sensors_flow_buffer:			.byte 2*(fir_filter_flow_end-fir_filter_flow)
sensors_flow_bufffer_end:		.byte 0

sensors_oxygen_rdy:				.byte 1
sensors_oxygen_tmr_ds:			.byte 1
sensors_oxygen_value:			.byte 4		; frac(L-H), int16 (L-H)
sensors_oxygen_buffer:			.byte 2*(fir_filter_oxygen_end-fir_filter_oxygen)
sensors_oxygen_bufffer_end:		.byte 0

sensors_VDD33_rdy:				.byte 1
sensors_VDD33_tmr_ds:			.byte 1
sensors_VDD33_value:			.byte 4		; frac(L-H), int16 (L-H)
sensors_VDD33_buffer:			.byte 2*(fir_filter_volt_end-fir_filter_volt)
sensors_VDD33_bufffer_end:		.byte 0

sensors_VDD50_rdy:				.byte 1
sensors_VDD50_tmr_ds:			.byte 1
sensors_VDD50_value:			.byte 4		; frac(L-H), int16 (L-H)
sensors_VDD50_buffer:			.byte 2*(fir_filter_volt_end-fir_filter_volt)
sensors_VDD50_bufffer_end:		.byte 0

sensors_current_rdy:			.byte 1
sensors_current_tmr_cs:			.byte 1
sensors_current_value:			.byte 4		; frac(L-H), int16 (L-H)
sensors_current_buffer:			.byte 2*(fir_filter_current_end-fir_filter_current)
sensors_current_bufffer_end:	.byte 0

sensors_volume_value:			.byte 4		; frac(L-H), int16 (L-H)

sensors_pressure_max:			.byte 4
sensors_flow_max:				.byte 4
sensors_volume_max:				.byte 4

sensors_pressure_ini:			.byte 4
sensors_flow_ini:				.byte 4
sensors_volume_ini:				.byte 4

sensors_flow_calc_process:		.byte 1
sensors_flow_offset_cal:		.byte 4
sensors_pressure_offset_cal:	.byte 4
sensors_o2_21_cal:				.byte 4
sensors_o2_Hi_cal:				.byte 4
sensors_o2_Hi_ref:				.byte 1
sensors_o2_0_cal:				.byte 4
sensors_o2_gain_cal:			.byte 4


sensors_oxygen_Calibrado:		.byte 4		; frac(L-H), int16 (L-H)


; sensors_adc_task	->	pool adc sensors
; ****************		max sampling = fosc/adc_pre/adc_acc/13 = 1384Hz = 0.722ms //692Hz = 1.45ms
.dseg
sensors_adc_list_ptr:	.byte 2
sensors_adc_var_ptr:	.byte 2
.cseg
sensors_adc_list:
	.db sensors_pressure_sampling_ms,	ADC_MUXPOS_AIN0_gc
	.dw	sensors_pressure_rdy
	.dw fir_filter_pressure,			fir_filter_pressure_end
	.dw sensors_pressure_offset,		fir_filter_pressure_shift

	.db sensors_oxygen_sampling_ds,		ADC_MUXPOS_AIN1_gc
	.dw	sensors_oxygen_rdy
	.dw fir_filter_oxygen,			fir_filter_oxygen_end
	.dw sensors_oxygen_offset,		fir_filter_oxygen_shift

	.db sensors_VDD33_sampling_ds,		ADC_MUXPOS_AIN2_gc
	.dw	sensors_VDD33_rdy
	.dw fir_filter_volt,			fir_filter_volt_end
	.dw sensors_VDD33_offset,		fir_filter_volt_shift

	.db sensors_VDD50_sampling_ds,		ADC_MUXPOS_AIN3_gc
	.dw	sensors_VDD50_rdy
	.dw fir_filter_volt,			fir_filter_volt_end
	.dw sensors_VDD50_offset,		fir_filter_volt_shift

	.db sensors_current_sampling_cs,	ADC_MUXPOS_AIN5_gc
	.dw	sensors_current_rdy
	.dw fir_filter_current,			fir_filter_current_end
	.dw sensors_current_offset,		fir_filter_current_shift

	.db 0,0


sensors_adc_task:
	rcall	sensors_adc_init
sensors_adc_task_start:
	rcall	sensors_adc_start_list			; points to sensors_adc_list
sensors_adc_task_loop:
	task_change
	rcall	sensors_adc_restart_list		; checks for end of sensors_adc_list
	breq	sensors_adc_task_start

	rcall	sensors_adc_sampling			; checks for sensor sampling time. data on X
	brne	sensors_adc_task_next

	rcall	sensors_adc_fir_filter			; fir filter data. X->int16, Y->factional
	rcall	sensors_adc_scale				; offset and shift. X->int16, Y->factional

	rcall	sensors_pressure_cal

	rcall	sensors_adc_save

sensors_adc_task_next:
	rcall	sensors_adc_next
	rjmp	sensors_adc_task_loop




; sensors_adc_init	-> power sensors. clear data. set sampling times
; ****************
sensors_adc_init:
	ldi		R16,0x10							; pressure/oxigen power enable
	sts		portd_outset,R16

	ldi		XH,high(sensors_pressure_rdy)		; clear all data
	ldi		XL,low(sensors_pressure_rdy)
	clr		R16
sensors_adc_init_loop:
	st		X+,R16
	cpi		XL,low(sensors_current_bufffer_end)
	brne	sensors_adc_init_loop
	cpi		XH,high(sensors_current_bufffer_end)
	brne	sensors_adc_init_loop


	ldi		R16,255								; set sampling times for first read
	sts		sensors_pressure_tmr_ms,R16

	ldi		R16,sensors_oxygen_sampling_ds
	sts		sensors_oxygen_tmr_ds,R16

	ldi		R16,sensors_VDD33_sampling_ds
	sts		sensors_VDD33_tmr_ds,R16

	ldi		R16,sensors_VDD50_sampling_ds
	sts		sensors_VDD50_tmr_ds,R16

	ldi		R16,sensors_current_sampling_cs
	sts		sensors_current_tmr_cs,R16

	ldi		R16,255								; just in case set flow sampling time  for first read
	sts		sensors_flow_tmr_ms,R16

;	ldi		R16,0								; pressure calibration to zero
;	sts		sensors_pressure_offset_cal,R16
;	sts		sensors_pressure_offset_cal+1,R16
;	sts		sensors_pressure_offset_cal+2,R16
;	sts		sensors_pressure_offset_cal+3,R16


	rcall	adc_init							; init adc hardware
	ret




sensors_adc_start_list:
	ldi		ZH,high(sensors_adc_list*2)		; top of adc list
	ldi		ZL,low(sensors_adc_list*2)
	sts		sensors_adc_list_ptr,ZL
	sts		sensors_adc_list_ptr+1,ZH
	ret




sensors_adc_restart_list:
	lds		ZL,sensors_adc_list_ptr
	lds		ZH,sensors_adc_list_ptr+1
	lpm		R17,Z+							; sampling time=0 -> restart
	cpi		R17,0
	ret


; output:	sez -> data read
;			clz -> not sampled
sensors_adc_sampling:
	lds		ZL,sensors_adc_list_ptr
	lds		ZH,sensors_adc_list_ptr+1
	lpm		R17,Z+							; sampling time
	lpm		R16,Z+							; adc channel
	lpm		YL,Z+							; sensor variables
	lpm		YH,Z+
	ldd		R18,Y+1							; sampling timer
	cpi		R18,0
	brne	sensors_adc_sampling_ret

	std		Y+1,R17							; update sampling timer
	sts		sensors_adc_var_ptr,YL			; save sensor variables pointer
	sts		sensors_adc_var_ptr+1,YH

	rcall	adc_read						; read adc data to X
	sez
sensors_adc_sampling_ret:
	ret



sensors_adc_fir_filter:
	lds		ZL,sensors_adc_list_ptr
	lds		ZH,sensors_adc_list_ptr+1
	adiw	ZL,4
	lpm		R18,Z+							; fir filter size
	lpm		R19,Z+
	lpm		R16,Z+
	lpm		R17,Z+
	sub		R16,R18
	sbc		R17,R19

	movw	ZH:ZL,R19:R18					; fir filter pointer
	lsl		ZL
	rol		ZH
	lds		YL,sensors_adc_var_ptr			; data buffer pointer
	lds		YH,sensors_adc_var_ptr+1
	adiw	YL,6							; buffer pointer ***OFFSET***

	rcall	fir_filter						; X -> int16, Y -> fractional
	ret



; offset and shift. X->int16, Y->factional
sensors_adc_scale:
	lds		ZL,sensors_adc_list_ptr
	lds		ZH,sensors_adc_list_ptr+1
	adiw	ZL,8
	lpm		R18,Z+							; offset. 4 fractional bits
	lpm		R19,Z+
	lpm		R16,Z+							; shift

	swap	R18								; offset
	mov		R17,R18
	andi	R17,0xf0
	andi	R18,0x0f
	swap	R19
	mov		R20,R19
	andi	R20,0xf0
	andi	R19,0x0f
	or		R18,R20
	sub		YH,R17
	sbc		XL,R18
	sbc		XH,R19

	rcall	sensors_adc_shift
	ret




sensors_adc_save:
	mov		ZH,YH
	mov		ZL,YL
	lds		YL,sensors_adc_var_ptr
	lds		YH,sensors_adc_var_ptr+1
	cli
	std		Y+2,ZL
	std		Y+3,ZH
	std		Y+4,XL
	std		Y+5,XH
	sei
	ldi		R16,1							; update rdy flag
	std		Y+0,R16
	ret



sensors_adc_next:
	lds		ZL,sensors_adc_list_ptr			; next adc list ptr
	lds		ZH,sensors_adc_list_ptr+1
	adiw	ZL,12							; list entry ***LENGTH***
	sts		sensors_adc_list_ptr,ZL
	sts		sensors_adc_list_ptr+1,ZH
	ret




sensors_adc_shift:
	cpi		R16,0							; shift
	breq	sensors_adc_shift_ret
	cpi		R16,8
	brlo	sensors_adc_shift8
	mov		YL,YH
	mov		YH,XL
	mov		XL,XH
	ldi		XH,0
	sbrc	XL,7
	ldi		XH,0xff
	subi	R16,8
	breq	sensors_adc_shift_ret
sensors_adc_shift8:
	asr		XH
	ror		XL
	ror		YH
	ror		YL
	dec		R16
	brne	sensors_adc_shift8
sensors_adc_shift_ret:
	ret




; X->integer, Y->frac
sensors_pressure_cal:
	lds		ZL,sensors_adc_list_ptr			; check if pressure
	lds		ZH,sensors_adc_list_ptr+1
	cpi		ZL,low(sensors_adc_list*2)
	brne	sensors_pressure_cal_ret
	cpi		ZH,high(sensors_adc_list*2)
	brne	sensors_pressure_cal_ret

;	lds		XH,sensors_pressure_value+3
;	lds		XL,sensors_pressure_value+2
;	lds		YH,sensors_pressure_value+1
;	lds		YL,sensors_pressure_value+0
	lds		R13,sensors_pressure_offset_cal+3
	lds		R12,sensors_pressure_offset_cal+2
	lds		R11,sensors_pressure_offset_cal+1
	lds		R10,sensors_pressure_offset_cal+0

	mov		R16,R13							; skip if calibration process
	or		R16,R12
	or		R16,R11
	or		R16,R10
	breq	sensors_pressure_cal_ret

	sub		YL,R10
	sbc		YH,R11
	sbc		XL,R12
	sbc		XH,R13
	brge	sensors_pressure_cal_pos
	clr		XH
	clr		XL
	clr		YH
	clr		YL
	rjmp	sensors_pressure_cal_ret
sensors_pressure_cal_pos:
	ldi		ZH,high(EEPROM_START+ee_press_range0)
	ldi		ZL,low(EEPROM_START+ee_press_range0)
	rcall	linearize_pos

;	sts		sensors_pressure_value+3,XH
;	sts		sensors_pressure_value+2,XL
;	sts		sensors_pressure_value+1,YH
;	sts		sensors_pressure_value+0,YL
sensors_pressure_cal_ret:
	ret





sensors_o2_cal:
	lds		R23,sensors_oxygen_value+3
	lds		R22,sensors_oxygen_value+2
	lds		R21,sensors_oxygen_value+1
	lds		R20,sensors_oxygen_value+0

	lds		R13,sensors_o2_0_cal+3
	lds		R12,sensors_o2_0_cal+2
	lds		R11,sensors_o2_0_cal+1
	lds		R10,sensors_o2_0_cal+0

	sub		R20,R10
	sbc		R21,R11
	sbc		R22,R12
	sbc		R23,R13
	brge	sensors_o2_cal_pos
	clr		R23
	clr		R22
	clr		R21
	clr		R20
sensors_o2_cal_pos:

	lds		R13,sensors_o2_gain_cal+3
	lds		R12,sensors_o2_gain_cal+2
	lds		R11,sensors_o2_gain_cal+1
	lds		R10,sensors_o2_gain_cal+0

	ldi		R16,0
	mul		R22,R11
	mov		XH,R1
	mov		XL,R0
	mul		R21,R10
	mov		YH,R1
	mov		YL,R0
	mul		R22,R10
	add		YH,R0
	adc		XL,R1
	adc		XH,R16
	mul		R21,R11
	add		YH,R0
	adc		XL,R1
	adc		XH,R16
	
	ret




; sensors_i2c_task	->	flow sensors
; ****************
sensors_i2c_task:
	rcall	sensors_i2c_init
sensors_i2c_task_loop:
	task_change
	lds		R16,sensors_flow_tmr_ms				; check sampling time
	cpi		R16,0
	brne	sensors_i2c_task_loop
	ldi		R16,sensors_flow_sampling_ms
	sts		sensors_flow_tmr_ms,R16

	ldi		R16,0x04							; debug led
	sts		PORTF_OUTSET,R16

	ldi		R16,sensors_flow_address			; read-retry
	rcall	i2c_master_read_retry
	brne	sensors_i2c_task_error_retry
	mov		R16,XH								; data ok?
	andi	R16,$c0
	breq	sensors_i2c_task_data_ok

	cpi		R16,$80								; stale data?
	brne	sensors_i2c_task_error_sensor
	task_change									; delay
	task_change
	ldi		R16,sensors_flow_address			; read-retry
	rcall	i2c_master_read_retry
	brne	sensors_i2c_task_error_stale_retry
	mov		R16,XH								; data ok?
	andi	R16,$c0
	brne	sensors_i2c_task_error_stale

sensors_i2c_task_data_ok:
	ldi		YH,high(sensors_flow_buffer)		; fir filter X value
	ldi		YL,low(sensors_flow_buffer)
	ldi		ZH,high(fir_filter_flow*2)
	ldi		ZL,low(fir_filter_flow*2)
	ldi		R16,(fir_filter_flow_end-fir_filter_flow)
	subi	XL,low(sensors_flow_offset)
	sbci	XH,high(sensors_flow_offset)

	rcall	fir_filter

	ldi		R16,fir_filter_flow_shift
	rcall	sensors_adc_shift
	rcall	sensors_offset_flow_cal
	lds		R16,sensors_flow_calc_process
	cpi		R16,1
	breq	sensors_i2c_task_calc_process

;	rjmp	sensors_i2c_task_calc_process

	ldi		ZH,high(EEPROM_START+ee_flow_insp_range0)
	ldi		ZL,low(EEPROM_START+ee_flow_insp_range0)
	rcall	linearize_pos

	ldi		ZH,high(EEPROM_START+ee_flow_exp_range0)
	ldi		ZL,low(EEPROM_START+ee_flow_exp_range0)
	rcall	linearize_neg

;	rcall	Neg_Flow_Gain

sensors_i2c_task_calc_process:
	cli
	sts		sensors_flow_value,YL				; save filtered value
	sts		sensors_flow_value+1,YH
	sts		sensors_flow_value+2,XL
	sts		sensors_flow_value+3,XH
	sei
	ldi		R16,1
	sts		sensors_flow_rdy,R16

	rcall	sensors_calc_vol

	ldi		R16,0x04							; debug led
	sts		PORTF_OUTCLR,R16

	rjmp	sensors_i2c_task_loop


; error conditions
sensors_i2c_task_error_retry:					; first read-retry error
	nop
sensors_i2c_task_error_sensor:					; first data not ok
	nop
sensors_i2c_task_error_stale_retry:				; first stale, second read-retry error
	nop
sensors_i2c_task_error_stale:					; first stale, second data not ok
 	nop

	rcall	sensors_i2c_restart					; restart sensor and i2c

	ldi		R16,0x04							; debug led
	sts		PORTF_OUTCLR,R16

	rjmp	sensors_i2c_task_loop



; sensors_i2c_init	-> power sensor. enable level translator. clear data. set sampling times
; ****************
sensors_i2c_init:
	ldi		R16,0x02						; flow sensor power enable
	sts		PORTA_OUTSET,R16
	ldi		R16,0x08						; i2c level translator enable
	sts		PORTB_OUTSET,R16

	ldi		XH,high(sensors_flow_rdy)		; clear all flow data
	ldi		XL,low(sensors_flow_rdy)
	clr		R16
sensors_i2c_init_loop:
	st		X+,R16
	cpi		XL,low(sensors_flow_bufffer_end)
	brne	sensors_i2c_init_loop
	cpi		XH,high(sensors_flow_bufffer_end)
	brne	sensors_i2c_init_loop

	sts		sensors_volume_value,R16		; clear volume data
	sts		sensors_volume_value+1,R16
	sts		sensors_volume_value+2,R16
	sts		sensors_volume_value+3,R16

	ldi		R16,255							; set flow sampling time for first read
	sts		sensors_flow_tmr_ms,R16

	rcall	i2c_master_init					; init i2c hardware
	ret



; sensors_i2c_restart	-> cycle power sensor and level translator. init.
; ****************
sensors_i2c_restart:
	ldi		R16,0x02						; flow sensor power disable
	sts		PORTA_OUTCLR,R16
	ldi		R16,0x08						; i2c level translator disable
	sts		PORTB_OUTCLR,R16

	ldi		R16,2							; 2 ms off
	sts		sensors_flow_tmr_ms,R16
sensors_i2c_restart_loop:
	task_change
	lds		R16,sensors_flow_tmr_ms
	cpi		R16,0
	brne	sensors_i2c_restart_loop

	ldi		R16,0x02						; flow sensor power enable
	sts		PORTA_OUTSET,R16
	ldi		R16,0x08						; i2c level translator enable
	sts		PORTB_OUTSET,R16

	ldi		R16,255							; set flow sampling time for first read
	sts		sensors_flow_tmr_ms,R16

	rcall	i2c_master_init					; init i2c hardware
	ret






; vol+=flow/30
; input:	flow -> X(int16), Y(frac16)
sensors_calc_vol:
	lds		R16,vent_phase					; check if expiration process
	cpi		R16,3
;	brne	sensors_calc_vol_cont
;	sbrs	XH,7							; minimun negative flow
;	rjmp	sensors_calc_vol_cont
;	cpi		XH,0xff
;	brne	sensors_calc_vol_cont
;	cpi		XL,0xff
;	brne	sensors_calc_vol_cont
;	cpi		YH,0xc0
;	brsh	sensors_calc_vol_cont
;	ldi		YH,0xc0
;	ldi		YL,0
;sensors_calc_vol_cont:

	lsl		YH
	rol		XL
	rol		XH
	lsl		YH
	rol		XL
	rol		XH
	lsl		YH
	rol		XL
	rol		XH
	mov		R21,XH
	mov		R20,XL
	ldi		R23,high(273)
	ldi		R22,low(273)

	lds		R16,sensors_volume_value			; accumulate volume
	lds		R17,sensors_volume_value+1
	lds		R18,sensors_volume_value+2
	lds		R19,sensors_volume_value+3

	rcall	mac16x16_32_method_B

	sbrs	R19,7
	rjmp	sensors_calc_vol_pos
	clr		R19
	clr		R18
	clr		R17
	clr		R16
sensors_calc_vol_pos:
	sts		sensors_volume_value,R16
	sts		sensors_volume_value+1,R17
	sts		sensors_volume_value+2,R18
	sts		sensors_volume_value+3,R19
	ret




sensors_offset_flow_cal:
	lds		R13,sensors_flow_offset_cal+3
	lds		R12,sensors_flow_offset_cal+2
	lds		R11,sensors_flow_offset_cal+1
	lds		R10,sensors_flow_offset_cal+0
	sub		YL,R10
	sbc		YH,R11
	sbc		XL,R12
	sbc		XH,R13
	ret




.dseg
inspDetect_flag:	.byte 1
.cseg
sensors_volume_reset:
	ldi		R16,0
	sts		inspDetect_flag,R16
	ret

	ldi		R16,0
	sts		sensors_volume_value,R16			; clear volume data
	sts		sensors_volume_value+1,R16
	sts		sensors_volume_value+2,R16
	sts		sensors_volume_value+3,R16
	ret


sensors_volume_reset_inspDetect:
	lds		R16,inspDetect_flag					; inspiration already detected?
	cpi		R16,0
	brne	sensors_volume_reset_inspDetect_ret

	lds		R16,sensors_flow_value+3			; Positive flow?
	sbrc	R16,7
	rjmp	sensors_volume_reset_inspDetect_ret

	ldi		R16,1								; inspiration detected!
	sts		inspDetect_flag,R16

	ldi		R16,1								; transmit inspiracion event to screen
	sts		vent_phase,R16
	rcall	screen_tx_event

	ldi		R16,0
	sts		sensors_volume_value,R16			; clear volume data
	sts		sensors_volume_value+1,R16
	sts		sensors_volume_value+2,R16
	sts		sensors_volume_value+3,R16
sensors_volume_reset_inspDetect_ret:
	ret






sensors_max_reset:
	lds		R10,sensors_pressure_value
	lds		R11,sensors_pressure_value+1
	lds		R12,sensors_pressure_value+2
	lds		R13,sensors_pressure_value+3
	sts		sensors_pressure_max,R10
	sts		sensors_pressure_max+1,R11
	sts		sensors_pressure_max+2,R12
	sts		sensors_pressure_max+3,R13
	sts		sensors_pressure_ini,R10
	sts		sensors_pressure_ini+1,R11
	sts		sensors_pressure_ini+2,R12
	sts		sensors_pressure_ini+3,R13

	lds		R10,sensors_flow_value
	lds		R11,sensors_flow_value+1
	lds		R12,sensors_flow_value+2
	lds		R13,sensors_flow_value+3
	sts		sensors_flow_max,R10
	sts		sensors_flow_max+1,R11
	sts		sensors_flow_max+2,R12
	sts		sensors_flow_max+3,R13
	sts		sensors_flow_ini,R10
	sts		sensors_flow_ini+1,R11
	sts		sensors_flow_ini+2,R12
	sts		sensors_flow_ini+3,R13

	lds		R10,sensors_volume_value
	lds		R11,sensors_volume_value+1
	lds		R12,sensors_volume_value+2
	lds		R13,sensors_volume_value+3
	sts		sensors_volume_max,R10
	sts		sensors_volume_max+1,R11
	sts		sensors_volume_max+2,R12
	sts		sensors_volume_max+3,R13
	sts		sensors_volume_ini,R10
	sts		sensors_volume_ini+1,R11
	sts		sensors_volume_ini+2,R12
	sts		sensors_volume_ini+3,R13

	ret



sensors_calc_max:
	lds		R10,sensors_pressure_value
	lds		R11,sensors_pressure_value+1
	lds		R12,sensors_pressure_value+2
	lds		R13,sensors_pressure_value+3
	lds		R0,sensors_pressure_max
	lds		R1,sensors_pressure_max+1
	lds		R2,sensors_pressure_max+2
	lds		R3,sensors_pressure_max+3
	sub		R0,R10
	sbc		R1,R11
	sbc		R2,R12
	sbc		R3,R13
	brge	sensors_calc_max_pressure
	sts		sensors_pressure_max,R10
	sts		sensors_pressure_max+1,R11
	sts		sensors_pressure_max+2,R12
	sts		sensors_pressure_max+3,R13
sensors_calc_max_pressure:

	lds		R10,sensors_flow_value
	lds		R11,sensors_flow_value+1
	lds		R12,sensors_flow_value+2
	lds		R13,sensors_flow_value+3
	lds		R0,sensors_flow_max
	lds		R1,sensors_flow_max+1
	lds		R2,sensors_flow_max+2
	lds		R3,sensors_flow_max+3
	sub		R0,R10
	sbc		R1,R11
	sbc		R2,R12
	sbc		R3,R13
	brge	sensors_calc_max_flow
	sts		sensors_flow_max,R10
	sts		sensors_flow_max+1,R11
	sts		sensors_flow_max+2,R12
	sts		sensors_flow_max+3,R13
sensors_calc_max_flow:

	lds		R10,sensors_volume_value
	lds		R11,sensors_volume_value+1
	lds		R12,sensors_volume_value+2
	lds		R13,sensors_volume_value+3
	lds		R0,sensors_volume_max
	lds		R1,sensors_volume_max+1
	lds		R2,sensors_volume_max+2
	lds		R3,sensors_volume_max+3
	sub		R0,R10
	sbc		R1,R11
	sbc		R2,R12
	sbc		R3,R13
	brge	sensors_calc_max_volume
	sts		sensors_volume_max,R10
	sts		sensors_volume_max+1,R11
	sts		sensors_volume_max+2,R12
	sts		sensors_volume_max+3,R13
sensors_calc_max_volume:

	ret












.equ	bat_adc_RX_BUFFER_LEN = 16
.equ	bat_adc_rx_TIMEOUT_ms = 100
.equ	bat_adc_TIMEOUT_cs = 200

.dseg
bat_adc_rx_buffer:	.byte bat_adc_RX_BUFFER_LEN
bat_adc_rx_ptrwr:	.byte 1
bat_adc_rx_ptrrd:	.byte 1
bat_adc_rx_tmr_ms:	.byte 1
bat_adc_tmr_cs:		.byte 1
bat_adc_tmp:		.byte 1
bat_adc_data:		.byte 2


.cseg
sensors_serial_task:
	rcall	sensors_serial_init
sensors_serial_task_loop_reset_tout:
	ldi		R16,bat_adc_TIMEOUT_cs
	sts		bat_adc_tmr_cs,R16
sensors_serial_task_loop:
	lds		R16,bat_adc_tmr_cs					; check timeout
	cpi		R16,0
	breq	sensors_serial_task

	rcall	bat_adc_rx_byte_tout				; header 0x5a,0xa5
	brne	sensors_serial_task_loop
sensors_serial_task_loop1:
	cpi		R16,0x5a
	brne	sensors_serial_task_loop

	rcall	bat_adc_rx_byte_tout
	brne	sensors_serial_task_loop
	cpi		R16,0xa5
	brne	sensors_serial_task_loop1

	rcall	bat_adc_rx_byte_tout				; len 3
	brne	sensors_serial_task_loop
	cpi		R16,0x3
	brne	sensors_serial_task_loop1

	rcall	bat_adc_rx_byte_tout				; cmd 0x11
	brne	sensors_serial_task_loop
	cpi		R16,0x11
	brne	sensors_serial_task_loop1

	rcall	bat_adc_rx_byte_tout				; data low-high
	brne	sensors_serial_task_loop
	sts		bat_adc_tmp,R16
	rcall	bat_adc_rx_byte_tout
	brne	sensors_serial_task_loop
	sts		bat_adc_data+1,R16
	lds		R16,bat_adc_tmp
	sts		bat_adc_data,R16

	rjmp	sensors_serial_task_loop_reset_tout


sensors_serial_init:
	ldi		R16,0x01							; bat sens power off
	sts		PORTC_OUTCLR,R16

	clr		R16
	sts		USART1_CTRLA,R16					; disable interrupts
	sts		bat_adc_rx_ptrwr,R16				; clear buffer pointers
	sts		bat_adc_rx_ptrrd,R16
	sts		bat_adc_data+1,R16					; battery to zero at reset
	sts		bat_adc_data,R16

	ldi		R16,100								; wait 1 second
	rcall	bat_adc_delay_cs

	ldi		R16,0x01							; bat sens power enable
	sts		PORTC_OUTSET,R16


	ldi		R16, low(64*fosc/(bat_adc_BPS*16))	; set baudrate
	ldi		R17, high(64*fosc/(bat_adc_BPS*16))
	sts		USART1_BAUDL,R16				
	sts		USART1_BAUDH,R17

	ldi		R16,0x03							; Async mode, parity disable, 
	sts		USART1_CTRLC,R16					; 1 stop bit, 8 bits

	ldi		R16,USART_RXCIE_bm					; rxcie interrupt
	sts		USART1_CTRLA,R16
	ldi		R16,USART_RXEN_bm						; Enable receiver
	sts		USART1_CTRLB,R16					; Normal mode
	ret


bat_adc_isr_rxc:
	push	R16
	in		R16,CPU_SREG
	push	R16
	push	XL
	push	XH

	ldi		XH,high(bat_adc_rx_buffer)
	ldi		XL,low(bat_adc_rx_buffer)
	lds		R16,bat_adc_rx_ptrwr
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	lds		R16,USART1_RXDATAL
	st		X,R16

	lds		R16,bat_adc_rx_ptrwr
	inc		R16
	cpi		R16,bat_adc_RX_BUFFER_LEN
	brlo	bat_adc_isr_rxc_ovf
	ldi		R16,0
bat_adc_isr_rxc_ovf:
	sts		bat_adc_rx_ptrwr,R16

	pop		XH
	pop		XL
	pop		R16
	out		CPU_SREG,R16
	pop		R16
	reti


bat_adc_rx_byte_tout:
	ldi		R16,bat_adc_rx_TIMEOUT_ms
	sts		bat_adc_rx_tmr_ms,R16
bat_adc_rx_byte_tout_loop:
	task_change
	lds		R16,bat_adc_rx_ptrrd
	lds		R17,bat_adc_rx_ptrwr
	cp		R16,R17
	brne	bat_adc_rx_byte_tout_data
	lds		R16,bat_adc_rx_tmr_ms
	cpi		R16,0
	brne	bat_adc_rx_byte_tout_loop
	clz
	rjmp	bat_adc_rx_byte_tout_ret
bat_adc_rx_byte_tout_data:
	ldi		XH,high(bat_adc_rx_buffer)
	ldi		XL,low(bat_adc_rx_buffer)
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	ld		R16,X
	lds		R17,bat_adc_rx_ptrrd
	inc		R17
	cpi		R17,bat_adc_RX_BUFFER_LEN
	brlo	bat_adc_rx_byte_tout_ovf
	ldi		R17,0
bat_adc_rx_byte_tout_ovf:
	sts		bat_adc_rx_ptrrd,R17
	sez
bat_adc_rx_byte_tout_ret:
	ret


bat_adc_delay_cs:
	sts		bat_adc_tmr_cs,R16
bat_adc_delay_cs_loop:
	task_change
	lds		R16,bat_adc_tmr_cs
	cpi		R16,0
	brne	bat_adc_delay_cs_loop
	ret




; sez -> OK
; clz -> ALARM
check_alarms:
	lds		R16,stepper_alarm						; skip if alarm detected
	sbrc	R16,1
	rjmp	check_alarms_ko

	stepper_read_pos
	lds		YL,EEPROM_START+ee_pos_alarm_thr		; inspection position
	lds		YH,EEPROM_START+ee_pos_alarm_thr+1
	sub		XL,YL
	sbc		XH,YH
	brlt	check_alarms_ok

	lds		XH,sensors_pressure_max+3				; pressure
	lds		XL,sensors_pressure_max+2
	lds		YH,sensors_pressure_ini+3
	lds		YL,sensors_pressure_ini+2
	rcall	sensor_limit_range
	mov		R17,XL
	lds		XH,sensors_flow_max+3					; flow
	lds		XL,sensors_flow_max+2
	lds		YH,sensors_flow_ini+3
	lds		YL,sensors_flow_ini+2
	rcall	sensor_limit_range

	cpi		R17,1									; sensor disconnect: pressure and flow low
	brge	check_alarm_no_sensors
	cpi		XL,1
	brge	check_alarm_no_sensors
	ldi		R16,1
	sts		stepper_sensors_alarm,R16
	rjmp	check_alarms_ko
check_alarm_no_sensors:

	lds		R16,EEPROM_START+ee_dis_alarm_pres_thr	; patient disconnect: pressure low, flow high
	cpi		R16,0
	breq	check_alarm_no_disconnect
	cp		R17,R16
	brge	check_alarm_no_disconnect
	lds		R16,EEPROM_START+ee_dis_alarm_flow_thr
	cp		XL,R16
	brlt	check_alarm_no_disconnect
	ldi		R16,1
	sts		stepper_disconnect_alarm,R16
	rjmp	check_alarms_ko
check_alarm_no_disconnect:

	lds		R16,EEPROM_START+ee_obs_alarm_pres_thr	; obstruction: pressure high, flow low
	cpi		R16,0
	breq	check_alarm_no_obstruction
	cp		R17,R16
	brlt	check_alarm_no_obstruction
	lds		R16,EEPROM_START+ee_obs_alarm_flow_thr
	cp		XL,R16
	brge	check_alarm_no_obstruction
	ldi		R16,1
	sts		stepper_obstruction_alarm,R16
	rjmp	check_alarms_ko
check_alarm_no_obstruction:

/*	mul		XL,R17								; flow power
	ldi		YH,high(2100)		; 1400
	ldi		YL,low(2100)
	cp		R0,YL
	cpc		R1,YH
	brlo	check_alarms_no_ovl
	ldi		R16,1
	sts		stepper_obstruction_alarm,R16
	rjmp	check_alarms_ko
check_alarms_no_ovl:
*/

check_alarms_ok:
	sez
	rjmp	check_alarms_ret
check_alarms_ko:
	lds		R16,stepper_alarm					; set alarm flag
	ori		R16,$02
	sts		stepper_alarm,R16
	ldi		R16,0								; stop motor
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16
	clz
check_alarms_ret:
	ret


sensor_limit_range:
	sub		XL,YL						; max-min
	sbc		XH,YH
	brlt	sensor_limit_range_neg
	cpi		XH,0
	brne	sensor_limit_range_top
	sbrs	XL,7
	rjmp	sensor_limit_range_ret
sensor_limit_range_top:
	ldi		XL,0x7f
	rjmp	sensor_limit_range_ret
sensor_limit_range_neg:
	cpi		XH,0xff
	brne	sensor_limit_range_bottom
	cpi		XL,0x80
	brsh	sensor_limit_range_ret
sensor_limit_range_bottom:
	ldi		XL,0x80
sensor_limit_range_ret:
	ret
