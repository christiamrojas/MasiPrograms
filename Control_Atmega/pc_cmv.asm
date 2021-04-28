/*
 * PC_CMV.asm
 *
 *  Created: 17/05/2020 06:20:14 p. m.
 *   Author: Javier
 */ 

.cseg
pressure_pid_10:
	.dw		int(20.00 *256)			; kp (int8+frac8)
	.dw		int( 0.025 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure

pressure_pid_16:
	.dw		int(15.00 *256)			; kp (int8+frac8)
	.dw		int( 0.03 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure

	.dw		int(10.00 *256)			; kp (int8+frac8)
	.dw		int( 0.00 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure

pressure_pid_22:
	.dw		int( 14.00 *256)		; kp (int8+frac8)
	.dw		int( 0.25 *256)			; ki (int8+frac8)
;	.dw		int( 15.00 *256)		; kp (int8+frac8)
;	.dw		int( 0.00 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure

pressure_pid_28:
	.dw		int(13.00 *256)			; kp (int8+frac8) 10
	.dw		int( 0.00 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure

pressure_pid_34:
	.dw		int(11.00 *256)			; kp (int8+frac8)
	.dw		int( 0.00 *256)			; ki (int8+frac8)
	.dw		int( 0.00 *256)			; kd (int8+frac8)
	.dw		0						; out_min
	.dw		250						; out_max
	.dw		set_pressure


pc_cmv_pid_table:
	; presure<sp, t<1.0 pid, t>=1.0 pid
	.dw		13,pressure_pid_10
	.dw		19+1,pressure_pid_16
	.dw		25+1,pressure_pid_22
	.dw		31+1,pressure_pid_28
	.dw		35+1,pressure_pid_34
; Verify range of last line!!!



.dseg
pc_pressure_add:		.byte 1

.cseg
; pressure control continous mandatory ventilation
pc_cmv_process:
	ldi		ZH,high(pc_cmv_pid_table*2)			; pid settings
	ldi		ZL,low(pc_cmv_pid_table*2)
	lds		R16,set_pressure					; sp pressure
	rcall	pc_cmv_load_pid_settings

	rcall	sensors_volume_reset				; clear volume data
	rcall	sensors_max_reset

	call	stepper_alarm_enable				; stepper alarm enable
	rcall	process_inspiration_pressure
	rcall	screen_tx_data_sensors
	call	stepper_alarm_disable				; clear alarm flag


	rcall	vc_cmv_calc_time_exp
	rcall	process_expiration
	rcall	vc_cmv_restore_time_exp

	rcall	pc_cmv_follow_pressure
	ret






; input:	z   -> pid settings table
;			R16 -> set point
pc_cmv_load_pid_settings:
	lpm		R17,Z+									; setpoint
	lpm		R18,Z+
	lpm		R18,Z+									; pid table
	lpm		R19,Z+

	cp		R16,R17
	brsh	pc_cmv_load_pid_settings

	mov		ZL,R18
	mov		ZH,R19
	lsl		ZL
	rol		ZH

	rcall	load_pid_settings

	lds		R16,set_pressure					; sp pressure
	lds		R17,pc_pressure_add
	add		R16,R17
	sts		pid_sp,R16
	ret




pc_cmv_copy_set_values:
	ld		R17,X+
	sts		set_pressure,R17
	ld		R17,X+
	sts		set_pressure+1,R17

	ld		R17,X+
	sts		set_insp_time_cs,R17
	ld		R17,X+
	sts		set_insp_time_cs+1,R17

	ld		R17,X+
	sts		set_exp_time_cs,R17
	ld		R17,X+
	sts		set_exp_time_cs+1,R17

	ld		R17,X+
	sts		set_trigger,R17
	ld		R17,X+
	sts		set_trigger+1,R17

	ld		R17,X+
	sts		set_fio2,R17
	ld		R17,X+
	sts		set_fio2+1,R17

	ldi		R17,0									; Init compensated flow
	sts		vc_cmv_insp_time_extra,R17
	sts		exp_fall_time,R17

	ldi		R17,0									; Init compensated flow
	sts		insp_flow_add,R17
	sts		pc_pressure_add,R17

	ret





process_inspiration_pressure:
;	ldi		R16,1								; transmit inspiracion event to screen
;	sts		vent_phase,R16
;	rcall	screen_tx_event

	lds		XL,set_insp_time_cs					; inspiration time
	lds		XH,set_insp_time_cs+1

	ldi		R16,0
	sts		vc_cmv_insp_rise_time,R16
	sts		vc_cmv_insp_flag,R16
	sts		exp_start_flag,R16

process_inspiration_pressure_loop:
	push	XL
	push	XH
	rcall	pid_wait_sampling_time				; wait pid sampling time
	rcall	check_alarms						; skip if alarm detected
	brne	process_inspiration_pressure_alarm
	lds		XH,sensors_pressure_value+3			; control variable
	lds		XL,sensors_pressure_value+2
	lds		R15,sensors_pressure_value+1
	rcall	pid_process							; pid control
	sts		stepper_speed_sp,XL					; update control variable
	sts		stepper_speed_sp+1,XH
process_inspiration_pressure_alarm:
	call	sensors_volume_reset_inspDetect
	rcall	screen_tx_data_PFV

	pop		XH
	pop		XL

	rcall	check_inspiration_start
	brne	process_inspiration_pressure_loop

	sbiw	XL,1
	brne	process_inspiration_pressure_loop
	ret



pc_cmv_follow_pressure:
	lds		R16,set_pressure					; sp pressure
	lds		R17,sensors_pressure_max+2
	lds		R18,sensors_pressure_max+1
	sbrc	R18,7
	inc		R17
	lds		R18,pc_pressure_add
	subi	R16,-1+1
	cp		R16,R17
	brlo	pc_cmv_follow_pressure_dec
	subi	R16,3-2
	cp		R16,R17
	brsh	pc_cmv_follow_pressure_inc
	rjmp	pc_cmv_follow_pressure_ret
pc_cmv_follow_pressure_inc:
	cpi		R18,4+1
	brge	pc_cmv_follow_pressure_ret
	inc		R18
	rjmp	pc_cmv_follow_pressure_update
pc_cmv_follow_pressure_dec:
	cpi		R18,-4
	brlt	pc_cmv_follow_pressure_ret
	dec		R18
pc_cmv_follow_pressure_update:
	sts		pc_pressure_add,R18
pc_cmv_follow_pressure_ret:
	ret

