/*
 * psv.asm
 *
 *  Created: 18/05/2020 07:48:14 p. m.
 *   Author: Javier
 */ 


.cseg



; pressure support ventilation
main_psv:
	ldi		ZH,high(pc_cmv_pid_table*2)				; pid settings
	ldi		ZL,low(pc_cmv_pid_table*2)
	lds		R16,set_pressure						; sp pressure
	rcall	pc_cmv_load_pid_settings

	call	sensors_volume_reset				; clear volume data
	call	sensors_max_reset

	call	stepper_alarm_enable				; stepper alarm enable
	rcall	process_inspiration_psv
	call	screen_tx_data_sensors
	call	stepper_alarm_disable				; clear alarm flag

	rcall	process_expiration
	ret




psv_copy_set_values:
	ld		R17,X+
	sts		set_pressure,R17
	ld		R17,X+
	sts		set_pressure+1,R17

	ld		YL,X+
	sts		set_flow_cycle,YL
	ld		YH,X+
	sts		set_flow_cycle+1,YH

	ld		R17,X+
	sts		set_trigger,R17
	ld		R17,X+
	sts		set_trigger+1,R17

	ld		R17,X+
	sts		set_fio2,R17
	ld		R17,X+
	sts		set_fio2+1,R17

	ld		R17,X+
	sts		set_exp_time_cs,R17
	ld		R17,X+
	sts		set_exp_time_cs+1,R17

	ldi		XH,high(655)				; psv_cycle_factor = flow_cycle*2.56
	ldi		XL,low(655)
	mul		YL,XL
	mov		R10,R0
	mov		R11,R1
	mul		YL,XH
	add		R11,R0
	sbrc	R10,7
	inc		R11
	sts		psv_cycle_factor,R11

	ldi		R17,0									; Init compensated flow
	sts		vc_cmv_insp_time_extra,R17

	ldi		R17,0									; Init compensated flow
	sts		insp_flow_add,R17
	sts		pc_pressure_add,R17
	ret



.dseg
psv_insp_time:		.byte 2
psv_cycle_factor:	.byte 1
.cseg
process_inspiration_psv:
;	ldi		R16,1								; transmit inspiracion event to screen
;	sts		vent_phase,R16
;	call	screen_tx_event

	ldi		R16,0								; inspiration time
	sts		psv_insp_time,R16
	sts		psv_insp_time+1,R16
process_inspiration_psv_loop:
	rcall	pid_wait_sampling_time				; wait pid sampling time
	lds		XH,sensors_pressure_value+3			; control variable
	lds		XL,sensors_pressure_value+2
	lds		R15,sensors_pressure_value+1
	rcall	pid_process							; pid control
	sts		stepper_speed_sp,XL					; update control variable
	sts		stepper_speed_sp+1,XH
	call	sensors_volume_reset_inspDetect
	call	screen_tx_data_PFV

	call	check_alarms						; skip if alarm detected
	brne	process_inspiration_psv_ret

	lds		XH,psv_insp_time+1					; check more than 0.5 seg
	lds		XL,psv_insp_time
	cpi		XH,0
	brne	process_inspiration_psv_cycling
	cpi		XL,50
	brlo	process_inspiration_psv_timeout
process_inspiration_psv_cycling:
	lds		YH,sensors_flow_max+3				; cycling threshold
	lds		YL,sensors_flow_max+2
	lds		R16,psv_cycle_factor
	mul		YH,R16
	mov		R11,R0
	mov		R12,R1
	mul		YL,R16
	mov		R10,R0
	add		R11,R1
	ldi		R16,0
	adc		R12,R16

	lds		YH,sensors_flow_value+3				; flow<threshold?
	lds		YL,sensors_flow_value+2
	cp		YL,R11
	cpc		YH,R12
	brlo	process_inspiration_psv_ret

process_inspiration_psv_timeout:
	adiw	XL,1								; check inpiration timeout
	sts		psv_insp_time+1,XH
	sts		psv_insp_time,XL
	cpi		XH,high(750)
	brne	process_inspiration_psv_loop
	cpi		XL,low(750)
	brne	process_inspiration_psv_loop
process_inspiration_psv_ret:
	ret
