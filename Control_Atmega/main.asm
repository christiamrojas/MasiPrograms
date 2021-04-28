;
; Masi_V1.asm
;
; Created: 05/05/2020 04:37:24 p. m.
; Author : jchangfu
;


.cseg
.org	$0
	rjmp	multitask_init

.org TCA0_OVF_vect					; = 14 TCA0 overflow interrupt vector. stepper motor
	jmp	isr_TCA_ovf
	
.org TCB0_INT_vect					; = 24 TCB0 interrupt vectors. timers
	rjmp	isr_TCB0_capture

.org USART1_RXC_vect				; = 52 USART1 receive interrupt vector. adc sensor
	jmp	bat_adc_isr_rxc

.org USART2_RXC_vect				; = 62 USART2 receive interrupt vector. screen
	jmp		screen_isr_rxc


.include "multitask.asm"
.include "timer.asm"
.include "debug.asm"
.include "Global.asm"
.include "stepper.asm"
.include "sensors.asm"
.include "screen.asm"

.include "power.asm"
.include "buzzer.asm"
.include "eeprom.asm"

.include "pid.asm"
.include "expiration.asm"
.include "trigger.asm"
.include "pc_cmv.asm"
.include "vc_cmv.asm"
.include "psv.asm"

.dseg

//*** SETTING
set_insp_time_cs:	.byte 2
set_pause_time_cs:	.byte 2
set_exp_time_cs:	.byte 2
set_pressure:		.byte 2
set_flow:			.byte 2
set_vol:			.byte 2
set_trigger:		.byte 2
set_flow_cycle:		.byte 1
set_fio2:			.byte 1

//*** CONTROL
vent_mode:			.byte 1
vent_mode_new:		.byte 1
vent_phase:			.byte 1		; 0=stopped, 1=inspiration, 2=pause, 3=expiration

autotest_flag:		.byte 1
calibrate_o2_flag:	.byte 1
main_tmr_cs:		.byte 1


.cseg

test_default:
	ldi		R16,low(400)
	sts		set_insp_time_cs,R16
	ldi		R16,high(400)
	sts		set_insp_time_cs+1,R16

	ldi		R16,low(30)
	sts		set_pause_time_cs,R16
	ldi		R16,high(30)
	sts		set_pause_time_cs+1,R16

	ldi		R16,low(200)
	sts		set_exp_time_cs,R16
	ldi		R16,high(200)
	sts		set_exp_time_cs+1,R16

	ldi		R16,low(5)
	sts		set_flow,R16
	ldi		R16,high(5)
	sts		set_flow+1,R16

	ldi		R16,low(15)
	sts		set_pressure,R16
	ldi		R16,high(15)
	sts		set_pressure+1,R16

	ldi		R17,0					; Init compensated flow and presssure
	sts		insp_flow_add,R17
	sts		pc_pressure_add,R17
	ldi		R16,0
	sts		sensors_flow_calc_process,R16
	rcall	ee_read_o2_calibration
	rcall	calc_calibrate_o2
	rcall	ee_read_pressure_calibration
	rcall	ee_read_flow_calibration

	ldi		R16,60					; timeout for eeprom write enable
	sts		ee_write_tmr_s,R16
	ret

.cseg
main_task:
	rcall	test_default			; default ventilation values for test
	rcall	main_init				; init variables
	call	buzzer					; beep
	ldi		R16,100
	rcall	main_delaycs

/*	ldi		R16,1
	sts		autotest_flag,R16
	rcall	pressure_flow_cal_zero
	ldi		R16,0
	sts		autotest_flag,R16
	rcall	buzzer					; beep
*/


;	rjmp	main_test_limits_no_tasks
	call	stepper_home			; stepper home
	ldi		R16,100
	rcall	main_delaycs

main_task_loop:
;	rjmp	main_test
;	rjmp	main_test_limits
	task_change

	lds		R16,power_shut_down		; power down?
	cpi		R16,1
	breq	main_task_loop

	rcall	chk_power_button
	rcall	chk_autotest
	rcall	chk_calibrate_o2

	lds		R16,vent_mode			; show data if stopped
	cpi		R16,0
	brne	main_task_loop_nostop
;	call	screen_check_data_tx
	call	screen_chk_tx_data_PFV
main_task_loop_nostop:

	rcall	check_vent_mode			; checks ventilation mode
	brne	main_task_loop
	rcall	run_vent_mode			; run one ventilation mode cycle
	rjmp	main_task_loop



main_init:
	ldi		R16,0
	sts		vent_mode,R16
	ldi		R16,0xff				; no new ventiation mode
	sts		vent_mode_new,R16
	ldi		R17,0					; Init compensated flow
	sts		vc_cmv_insp_time_extra,R17
	sts		vent_phase,R17			; ventilation stopped
	sts		autotest_flag,R17
	sts		calibrate_o2_flag,R17

	ldi		R16,0					; apnea false inicial
	sts		flow_apnea,R16
	ret


chk_autotest:
	lds		R16,autotest_flag
	cpi		R16,0
	breq	chk_autotest_ret

	call	stepper_home					; stepper home
	ldi		R16,250
	rcall	main_delaycs

	rcall	pressure_flow_cal_zero

	ldi		R16,1
	call	screen_tx_autotest
	ldi		R16,0
	sts		autotest_flag,R16
chk_autotest_ret:
	ret


.dseg
pressure_cal_zero:	.byte 5
flow_cal_zero:		.byte 5
.cseg
pressure_flow_cal_zero:
	clr		R0
	sts		sensors_pressure_offset_cal,R0
	sts		sensors_pressure_offset_cal+1,R0
	sts		sensors_pressure_offset_cal+2,R0
	sts		sensors_pressure_offset_cal+3,R0
	sts		sensors_flow_offset_cal,R0
	sts		sensors_flow_offset_cal+1,R0
	sts		sensors_flow_offset_cal+2,R0
	sts		sensors_flow_offset_cal+3,R0
	sts		pressure_cal_zero,R0
	sts		pressure_cal_zero+1,R0
	sts		pressure_cal_zero+2,R0
	sts		pressure_cal_zero+3,R0
	sts		pressure_cal_zero+4,R0
	sts		flow_cal_zero,R0
	sts		flow_cal_zero+1,R0
	sts		flow_cal_zero+2,R0
	sts		flow_cal_zero+3,R0
	sts		flow_cal_zero+4,R0
	ldi		R16,1
	sts		sensors_flow_calc_process,R16
	ldi		R17,100
pressure_cal_zero_purge_loop:
	push	R17
	rcall	pid_wait_sampling_time
	pop		R17
	brne	pressure_cal_zero_purge_loop

	clr		R17
pressure_cal_zero_loop:
	push	R17
	rcall	pid_wait_sampling_time			; wait pid sampling time
	call	screen_tx_data_PFV
	pop		R17
	rcall	pressure_zero_calc
	rcall	flow_zero_calc
	dec		R17
	brne	pressure_cal_zero_loop

	ldi		R16,0
	sts		sensors_flow_calc_process,R16
	rcall	pressure_cal_save
	rcall	flow_cal_save
	ret


pressure_zero_calc:
	lds		XH,sensors_pressure_value+3
	lds		XL,sensors_pressure_value+2
	lds		YH,sensors_pressure_value+1
	lds		YL,sensors_pressure_value+0
	lds		R10,pressure_cal_zero
	lds		R11,pressure_cal_zero+1
	lds		R12,pressure_cal_zero+2
	lds		R13,pressure_cal_zero+3
	lds		R14,pressure_cal_zero+4
	add		R10,YL
	adc		R11,YH
	adc		R12,XL
	adc		R13,XH
	sbrs	XH,7
	ldi		R16,0
	sbrc	XH,7
	ldi		R16,$ff	
	adc		R14,R16
	sts		pressure_cal_zero,R10
	sts		pressure_cal_zero+1,R11
	sts		pressure_cal_zero+2,R12
	sts		pressure_cal_zero+3,R13
	sts		pressure_cal_zero+4,R14
	ret

flow_zero_calc:
	lds		XH,sensors_flow_value+3
	lds		XL,sensors_flow_value+2
	lds		YH,sensors_flow_value+1
	lds		YL,sensors_flow_value+0
	lds		R10,flow_cal_zero
	lds		R11,flow_cal_zero+1
	lds		R12,flow_cal_zero+2
	lds		R13,flow_cal_zero+3
	lds		R14,flow_cal_zero+4
	add		R10,YL
	adc		R11,YH
	adc		R12,XL
	adc		R13,XH
	sbrs	XH,7
	ldi		R16,0
	sbrc	XH,7
	ldi		R16,$ff
	adc		R14,R16
	sts		flow_cal_zero,R10
	sts		flow_cal_zero+1,R11
	sts		flow_cal_zero+2,R12
	sts		flow_cal_zero+3,R13
	sts		flow_cal_zero+4,R14
	ret

pressure_cal_save:
	lds		R11,pressure_cal_zero+1
	lds		R12,pressure_cal_zero+2
	lds		R13,pressure_cal_zero+3
	lds		R14,pressure_cal_zero+4
	sts		sensors_pressure_offset_cal+3,R14
	sts		sensors_pressure_offset_cal+2,R13
	sts		sensors_pressure_offset_cal+1,R12
	sts		sensors_pressure_offset_cal+0,R11
	ldi		XH,high(EEPROM_START+ee_pressure_zero_cal)
	ldi		XL,low(EEPROM_START+ee_pressure_zero_cal)
	lds		R16,sensors_pressure_offset_cal+0
	call	eeprom_write_byte
	lds		R16,sensors_pressure_offset_cal+1
	call	eeprom_write_byte
	lds		R16,sensors_pressure_offset_cal+2
	call	eeprom_write_byte
	lds		R16,sensors_pressure_offset_cal+3
	call	eeprom_write_byte
	ret


flow_cal_save:
	lds		R11,flow_cal_zero+1
	lds		R12,flow_cal_zero+2
	lds		R13,flow_cal_zero+3
	lds		R14,flow_cal_zero+4
	sts		sensors_flow_offset_cal+3,R14
	sts		sensors_flow_offset_cal+2,R13
	sts		sensors_flow_offset_cal+1,R12
	sts		sensors_flow_offset_cal+0,R11
	ldi		XH,high(EEPROM_START+ee_flow_zero_cal)
	ldi		XL,low(EEPROM_START+ee_flow_zero_cal)
	lds		R16,sensors_flow_offset_cal+0
	call	eeprom_write_byte
	lds		R16,sensors_flow_offset_cal+1
	call	eeprom_write_byte
	lds		R16,sensors_flow_offset_cal+2
	call	eeprom_write_byte
	lds		R16,sensors_flow_offset_cal+3
	call	eeprom_write_byte
	ret


ee_read_pressure_calibration:
	ldi		XH,high(EEPROM_START+ee_pressure_zero_cal)
	ldi		XL,low(EEPROM_START+ee_pressure_zero_cal)
	ld		R16,X+
	sts		sensors_pressure_offset_cal,R16
	ld		R16,X+
	sts		sensors_pressure_offset_cal+1,R16
	ld		R16,X+
	sts		sensors_pressure_offset_cal+2,R16
	ld		R16,X+
	sts		sensors_pressure_offset_cal+3,R16
	ret



ee_read_flow_calibration:
	ldi		XH,high(EEPROM_START+ee_flow_zero_cal)
	ldi		XL,low(EEPROM_START+ee_flow_zero_cal)
	ld		R16,X+
	sts		sensors_flow_offset_cal,R16
	ld		R16,X+
	sts		sensors_flow_offset_cal+1,R16
	ld		R16,X+
	sts		sensors_flow_offset_cal+2,R16
	ld		R16,X+
	sts		sensors_flow_offset_cal+3,R16
	ret



ee_read_o2_calibration:
	ldi		XH,high(EEPROM_START+ee_o2_21_cal)
	ldi		XL,low(EEPROM_START+ee_o2_21_cal)
	ld		R16,X+
	sts		sensors_o2_21_cal+0,R16
	ld		R16,X+
	sts		sensors_o2_21_cal+1,R16
	ld		R16,X+
	sts		sensors_o2_21_cal+2,R16
	ld		R16,X+
	sts		sensors_o2_21_cal+3,R16

	ldi		XH,high(EEPROM_START+ee_o2_hi_cal)
	ldi		XL,low(EEPROM_START+ee_o2_hi_cal)
	ld		R16,X+
	sts		sensors_o2_Hi_cal+0,R16
	ld		R16,X+
	sts		sensors_o2_Hi_cal+1,R16
	ld		R16,X+
	sts		sensors_o2_Hi_cal+2,R16
	ld		R16,X+
	sts		sensors_o2_Hi_cal+3,R16

	ldi		XH,high(EEPROM_START+ee_o2_hi_ref)
	ldi		XL,low(EEPROM_START+ee_o2_hi_ref)
	ld		R16,X+
	sts		sensors_o2_Hi_ref,R16
	ret



chk_calibrate_o2:
	lds		R16,calibrate_o2_flag
	cpi		R16,0
	breq	chk_calibrate_o2_ret
	
	rcall	calc_calibrate_o2

	ldi		R16,1
	call	screen_tx_ack_O2
	ldi		R16,0
	sts		calibrate_o2_flag,R16
chk_calibrate_o2_ret:
	ret



; factor for ref=(60 to 100): factor=(21/(ref-21)
table_cal_o2:
	.db 138,134,131,128,125,122,119,117,114,112
	.db 110,108,105,103,101,100, 98, 96, 94, 93
	.db	 91, 90, 88, 87, 85, 84, 83, 81, 80, 79
	.db	 78, 77, 76, 75, 74, 73, 72, 71, 70, 69
	.db	 68, 68

calc_calibrate_o2:
	lds		R16,sensors_o2_Hi_ref
	cpi		R16,60
	brlo	calc_calibrate_o2_ret1
	cpi		R16,100+1
	brlo	calc_calibrate_o2_cont
calc_calibrate_o2_ret1:
	rjmp	calc_calibrate_o2_ret
calc_calibrate_o2_cont:

	lds		R23,sensors_o2_Hi_cal+3		;	d=oHi-o21
	lds		R22,sensors_o2_Hi_cal+2
	lds		R21,sensors_o2_Hi_cal+1
	lds		R20,sensors_o2_Hi_cal+0
	lds		R13,sensors_o2_21_cal+3
	lds		R12,sensors_o2_21_cal+2
	lds		R11,sensors_o2_21_cal+1
	lds		R10,sensors_o2_21_cal+0
	sub		R20,R10
	sbc		R21,R11
	sbc		R22,R12
	sbc		R23,R13

	ldi		ZH,high(table_cal_o2*2)			; read calibration factor
	ldi		ZL,low(table_cal_o2*2)
	subi	R16,60
	add		ZL,R16
	ldi		R16,0
	adc		ZH,R16

	lpm		R16,Z							; o0=o21-d*21/(hi-21)	; (70-21)=49
	ldi		R17,0
	mul		R21,R16
	mov		R2,R0
	mov		R3,R1
	mul		R23,R16
	mov		R4,R0
	mov		R5,R1
	mul		R20,R16
	add		R2,R1
	adc		R3,R17
	adc		R4,R17
	adc		R5,R17
	mul		R22,R16
	add		R3,R0
	adc		R4,R1
	adc		R5,R17
	sub		R10,R2
	sbc		R11,R3
	sbc		R12,R4
	sbc		R13,R5
	sts		sensors_o2_0_cal+0,R10
	sts		sensors_o2_0_cal+1,R11
	sts		sensors_o2_0_cal+2,R12
	sts		sensors_o2_0_cal+3,R13


	lds		R16,sensors_o2_Hi_ref			;	g=(hi-21)*256*256*256/d
	subi	R16,21
	mov		R3,R16
	clr		R2
	clr		R1
	clr		R0
	mov		R7,R23
	mov		R6,R22
	mov		R5,R21
	mov		R4,R20
	call	Divide32
	sts		sensors_o2_gain_cal+0,R0
	sts		sensors_o2_gain_cal+1,R1
	sts		sensors_o2_gain_cal+2,R2
	sts		sensors_o2_gain_cal+3,R3
calc_calibrate_o2_ret:
	ret



chk_power_button:
	lds		R16,power_screen_flag
	cpi		R16,1
	brne	chk_power_button_ret
	ldi		R16,2
	sts		power_screen_flag,R16
	call	screen_tx_shut_down
chk_power_button_ret:
	ret





; checks ventilation mode
check_vent_mode:
	lds		R16,vent_mode_new				; new ventilation mode?
	cpi		R16,0xff
	breq	check_vent_mode_no_new
	ldi		R17,0xff
	sts		vent_mode_new,R17

	ldi		XH,high(vent_mode_data)			; pointer to parameters
	ldi		XL,low(vent_mode_data)

	cpi		R16,0							; stop?
	breq	check_vent_mode_set

	cpi		R16,1							; vc-cmv?
	brne	check_vent_mode_no_vc_cmv
	rcall	vc_cmv_copy_set_values
	rjmp	check_vent_mode_set
check_vent_mode_no_vc_cmv:

	cpi		R16,2							; pc-cmv?
	brne	check_vent_mode_no_pc_cmv
	rcall	pc_cmv_copy_set_values
	rjmp	check_vent_mode_set
check_vent_mode_no_pc_cmv:

	cpi		R16,3							; psv?
	brne	check_vent_mode_no_psv
	rcall	psv_copy_set_values
	rjmp	check_vent_mode_set
check_vent_mode_no_psv:

check_vent_mode_no_new:
	rjmp	check_vent_mode_check
check_vent_mode_set:
	sts		vent_mode,R16
check_vent_mode_check:
	lds		R16,vent_mode
	cpi		R16,0
	brne	check_vent_mode_ok
	clz
	rjmp	check_vent_mode_ret
check_vent_mode_ok:
	sez
check_vent_mode_ret:
	ret









; run one cycle of ventilation mode
run_vent_mode:
	ldi		ZH,high(main_vent_mode_table)
	ldi		ZL,low(main_vent_mode_table)
	lds		R16,vent_mode
	cpi		R16,(main_vent_mode_table_end-main_vent_mode_table)
	brsh	run_vent_mode_ret

	add		ZL,R16					; points to ventilation mode
	ldi		R16,0
	adc		ZH,R16
	ijmp
main_vent_mode_table:
	rjmp	run_vent_mode_ret		; stop mode
	rjmp	main_vc_cmv				; volume control continous mandatory ventilation
;	rjmp	main_vc_cmv_pid				; volume control continous mandatory ventilation
	rjmp	pc_cmv_process			; pressure control continous mandatory ventilation
	rjmp	main_psv				; pressure support ventilation
main_vent_mode_table_end:

run_vent_mode_ret:
	ret
































main_test:
	ldi		R16,1							; inspiracion
	sts		vent_phase,R16
	call	screen_tx_event

	ldi		R16,0
	sts		sensors_volume_value,R16		; clear volume data
	sts		sensors_volume_value+1,R16
	sts		sensors_volume_value+2,R16
	sts		sensors_volume_value+3,R16

	ldi		YL,low(75)
	ldi		YH,high(75)
	ldi		XL,low(stepper_pos_max-50)
	ldi		XH,high(stepper_pos_max-50)
	call	stepper_close

	ldi		R16,2							; hold
	sts		vent_phase,R16
	call	screen_tx_event
	ldi		R16,255
	rcall	main_delaycs
	ldi		R16,255
	rcall	main_delaycs

	ldi		R16,3							; expiration
	sts		vent_phase,R16
	call	screen_tx_event

	ldi		YL,low(-126)
	ldi		YH,high(-126)
	ldi		XL,low(100)
	ldi		XH,high(100)
	call	stepper_open

	ldi		R16,255
	rcall	main_delaycs
	ldi		R16,255
	rcall	main_delaycs

	rjmp	main_task_loop





main_test_limits:
	ldi		XH,high(400)
	ldi		XL,low(400)
	stepper_write_pos
main_test_limits_loop:
	rcall	test_limits_close
	rcall	test_limits_close_release
	rcall	test_limits_open
	rcall	test_limits_open_release
	rjmp	main_test_limits_loop


test_limits_close:
	ldi		YH,high(10)
	ldi		YL,low(10)
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH

test_limits_close_loop:
	task_change
	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_close_pin
	rjmp	test_limits_close_loop

	ldi		R16,0
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16

	stepper_read_pos

	ldi		R16,255
	rcall	main_delaycs
	ret


test_limits_close_release:
	ldi		YH,high(-10)
	ldi		YL,low(-10)
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH

test_limits_close_release_loop:
	task_change
	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_close_pin
	rjmp	test_limits_close_release_loop

	ldi		R16,0
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16

	stepper_read_pos

	ldi		R16,255
	rcall	main_delaycs
	ret


test_limits_open:
	ldi		YH,high(-10)
	ldi		YL,low(-10)
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH

test_limits_open_loop:
	task_change
	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_open_pin
	rjmp	test_limits_open_loop

	ldi		R16,0
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16

	stepper_read_pos

	ldi		R16,255
	rcall	main_delaycs
	ret


test_limits_open_release:
	ldi		YH,high(10)
	ldi		YL,low(10)
	sts		stepper_speed_sp,YL
	sts		stepper_speed_sp+1,YH

test_limits_open_release_loop:
	task_change
	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_open_pin
	rjmp	test_limits_open_release_loop

	ldi		R16,0
	sts		stepper_speed_sp,R16
	sts		stepper_speed_sp+1,R16

	stepper_read_pos

	ldi		R16,255
	rcall	main_delaycs
	ret



main_delaycs:
	sts		main_tmr_cs,R16
main_delaycs_loop:
;	task_change
;	call	screen_check_data_tx
	call	screen_chk_tx_data_PFV

	lds		R16,main_tmr_cs
	cpi		R16,0
	brne	main_delaycs_loop
	ret







;	PB0: (OUT-LOW)	stepper pulse
;	PB1: (OUT-LOW)	stepper dir
;	PB2: (OUT-HIGH)	stepper enable
main_test_limits_no_tasks:
	cli
	ldi		R16,$04
	sts		PORTB_OUTCLR,R16
	rcall	test_limits_no_tasks_close
	rcall	test_limits_no_tasks_close_release
	rcall	test_limits_no_tasks_open
	rcall	test_limits_no_tasks_open_release
	rjmp	main_test_limits_no_tasks


test_limits_no_tasks_close:
	ldi		R16,$02
	sts		PORTB_OUTSET,R16
test_limits_no_tasks_close_loop:
	rcall	pulso_stepper
	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_close_pin
	rjmp	test_limits_no_tasks_close_loop
	rcall	delay_test_long
	ret
	
		
test_limits_no_tasks_close_release:
	ldi		R16,$02
	sts		PORTB_OUTCLR,R16
test_limits_no_tasks_close_release_loop:
	rcall	pulso_stepper
	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_close_pin
	rjmp	test_limits_no_tasks_close_release_loop
	rcall	delay_test_long
	ret


test_limits_no_tasks_open:
	ldi		R16,$02
	sts		PORTB_OUTCLR,R16
test_limits_no_tasks_open_loop:
	rcall	pulso_stepper
	lds		R16,PORTE_IN
	sbrc	R16,stepper_sw_open_pin
	rjmp	test_limits_no_tasks_open_loop
	rcall	delay_test_long
	ret


test_limits_no_tasks_open_release:
	ldi		R16,$02
	sts		PORTB_OUTSET,R16
test_limits_no_tasks_open_release_loop:
	rcall	pulso_stepper
	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_open_pin
	rjmp	test_limits_no_tasks_open_release_loop
	rcall	delay_test_long
	ret




pulso_stepper:
	ldi		R16,$01
	sts		PORTB_OUTSET,R16
	rcall	delay_test
	ldi		R16,$01
	sts		PORTB_OUTCLR,R16
	rcall	delay_test
	ret



delay_test:
	ldi		XH,high(6000)
	ldi		XL,low(6000)
delay_test_loop:
	sbiw	XL,1
	brne	delay_test_loop
	ret

delay_test_long:
	ldi		XH,high(100)
	ldi		XL,low(100)
delay_test_long_loop:
	ldi		YH,high(65535)
	ldi		YL,low(65535)
delay_test_long_loop2:
	sbiw	YL,1
	brne	delay_test_long_loop2
	sbiw	XL,1
	brne	delay_test_long_loop
	ret
