/*
 * expiration.asm
 *
 *  Created: 19/05/2020 12:18:40 a. m.
 *   Author: Javier
 */ 


.dseg
exp_zero2:			.byte 2
exp_fall_time:		.byte 1
exp_start_flag:		.byte 1

.cseg
expiration_pid_parameters:
	.dw		int( 7.00 *256)			; kp (int8+frac8)
	.dw		int( 0.00 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		-127					; out_min
	.dw		0						; out_max
	.dw		exp_zero2


expiration_load_pid_settings:
	ldi		R16,40
	sts		exp_zero2,R16			; int8
	ldi		R16,0
	sts		exp_zero2+1,R16			; frac8

load_pid_settings:
	lpm		XL,Z+
	lpm		XH,Z+
	sts		pid_kp,XL
	sts		pid_kp+1,XH

	lpm		XL,Z+
	lpm		XH,Z+
	sts		pid_ki,XL
	sts		pid_ki+1,XH

	lpm		XL,Z+
	lpm		XH,Z+
	sts		pid_kd,XL
	sts		pid_kd+1,XH

	lpm		XL,Z+
	lpm		XH,Z+
	sts		pid_out_min,XL
	sts		pid_out_min+1,XH

	lpm		XL,Z+
	lpm		XH,Z+
	sts		pid_out_max,XL
	sts		pid_out_max+1,XH

	lpm		XL,Z+
	lpm		XH,Z+
	ld		R16,X+
	ld		R17,X+
	sts		pid_sp,R16
	sts		pid_sp+1,R17

	clr		R16
	sts		pid_e,R16
	sts		pid_e+1,R16
	sts		pid_ei,R16
	sts		pid_ei+1,R16
	ret









process_expiration:
	ldi		R16,3								; transmit expiration event to screen
	sts		vent_phase,R16
	rcall	screen_tx_event

	ldi		ZH,high(expiration_pid_parameters*2)
	ldi		ZL,low(expiration_pid_parameters*2)
	rcall	expiration_load_pid_settings
	rcall	process_trigger_init
	ldi		R16,0								; Limit switch count
	sts		exp_limit_sw,R16
	sts		exp_fall_time,R16
	sts		exp_start_flag,R16

	lds		XL,set_exp_time_cs
	lds		XH,set_exp_time_cs+1
process_expiration_loop:
	push	XL
	push	XH
	rcall	pid_wait_sampling_time				; wait pid sampling time
	stepper_read_pos
	clr		R15
	rcall	pid_process							; pid control
	sts		stepper_speed_sp,XL					; update control variable
	sts		stepper_speed_sp+1,XH
	call	screen_tx_data_PFV					; transmit data
	rcall	check_expiration_start
	rcall	process_trigger						; z flag indicate trigger!!!

	pop		XH
	pop		XL
	breq	process_expiration_chk_res_alarm
	
	rcall	check_limit_sw						; detect limit switch
	brne	process_expiration_chk_res_alarm

	sbiw	XL,1
	brne	process_expiration_loop

process_expiration_chk_res_alarm:
	rcall	chk_resistance_alarm

	ret



check_expiration_start:
	lds		R18,exp_start_flag
	cpi		R18,1
	breq	check_expiration_start_ret

	lds		ZH,sensors_flow_value+3
	lds		ZL,sensors_flow_value+2
	sbrs	ZH,7
	rjmp	check_expiration_start_ok
	cpi		ZH,0xff
	brne	check_expiration_start_flag
	cpi		ZL,-2
	brlt	check_expiration_start_flag

check_expiration_start_ok:
	lds		R18,exp_fall_time
	cpi		R18,50
	brsh	check_expiration_start_flag
	inc		R18
	sts		exp_fall_time,R18
	clz
	rjmp	check_expiration_start_ret

check_expiration_start_flag:
	ldi		R18,1
	sts		exp_start_flag,R18
	sez
check_expiration_start_ret:
	ret

.dseg
exp_limit_sw:		.byte 1
.cseg
check_limit_sw:
	lds		R17,exp_limit_sw					; detect limit switch
	lds		R16,PORTE_IN
	sbrs	R16,stepper_sw_open_pin				; normal sbrs
	inc		R17
	sts		exp_limit_sw,R17
	cpi		R17,5
	brlo	check_limit_sw_ok
	call	stepper_home						; restore position
	clz
	rjmp	check_limit_sw_ret
check_limit_sw_ok:
	sez
check_limit_sw_ret:
	ret



chk_resistance_alarm:
	lds		R16,EEPROM_START+ee_res_alarm_thr	; resistance threshold
	cpi		R16,0
	breq	chk_resistance_alarm_ret

	lds		XH,sensors_pressure_max+2			; calc driving pressure
	lds		XL,sensors_pressure_max+1
	lds		YH,sensors_pressure_value+2
	lds		YL,sensors_pressure_value+1
	sub		XL,YL
	sbc		XH,YH
	brlt	chk_resistance_alarm_ret
	sbrc	XL,7
	inc		XH
	lds		R17,EEPROM_START+ee_res_alarm_drv_press_thr
	cp		XH,R17
	brlo	chk_resistance_alarm_ret

	lds		YH,sensors_flow_max+2				; if (P*60>R*F) => obstruction
	lds		YL,sensors_flow_max+1
	sbrc	YL,7
	inc		YH
	mul		R16,YH
	mov		R2,R0
	mov		R3,R1
	ldi		R17,60
	mul		XH,R17
	sub		R0,R2
	sbc		R1,R3
	brlo	chk_resistance_alarm_ret
	ldi		R16,1
	sts		stepper_resistance_alarm,R16
chk_resistance_alarm_ret:
	ret