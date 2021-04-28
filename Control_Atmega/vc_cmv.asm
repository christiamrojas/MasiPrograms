/*
 * vc_cmv.asm
 *
 *  Created: 18/05/2020 07:47:10 p. m.
 *   Author: Javier
 */ 

.equ	pause_min_time_cs	= 10	; minimum pause time from 1 to 63
.equ	ctrl_flow_max		= 90


.cseg

flow_pid_5:
	.dw		int( 3.0 *256)			; kp (int8+frac8)
	.dw		int( 0.3 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		14						; out_min
	.dw		50						; out_max
	.dw		set_flow

flow_pid_10:
	.dw		int( 3.0 *256)			; kp (int8+frac8)
	.dw		int( 0.3 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		19						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_15:
	.dw		int( 3.0 *256)			; kp (int8+frac8) 5 l/m
	.dw		int( 0.2 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		18						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_20:
	.dw		int( 2.0 *256)			; kp (int8+frac8) 10 l/m 6
	.dw		int( 0.3 *256)			; ki (int8+frac8)		.15
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		22						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_25:
	.dw		int( 2.0 *256)			; kp (int8+frac8) 10 l/m
	.dw		int( 0.4 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		35						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_30:
	.dw		int( 2.0 *256)			; kp (int8+frac8) 10 l/m
	.dw		int( 0.4 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		40						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_35:
	.dw		int( 3.0 *256)			; kp (int8+frac8) 20 l/m
	.dw		int( 0.4 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		35						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_40:
	.dw		int( 3.0 *256)			; kp (int8+frac8) 25 l/m
	.dw		int( 0.4 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		38						; out_min
	.dw		150						; out_max
	.dw		set_flow

flow_pid_45:
	.dw		int( 2.0 *256)			; kp (int8+frac8) 25 l/m
	.dw		int( 0.5 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		45						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_50:
	.dw		int( 3.0 *256)			; kp (int8+frac8) 25 l/m
	.dw		int( 0.5 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		60						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_55:
	.dw		int( 3.0 *256)			; kp (int8+frac8) 25 l/m
	.dw		int( 0.7 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		52						; out_min
	.dw		250						; out_max
	.dw		set_flow

flow_pid_60:
	.dw		int( 3.0 *256)			; kp (int8+frac8)	60 l/m
	.dw		int( 0.7 *256)			; ki (int8+frac8)
	.dw		int( 0.0 *256)			; kd (int8+frac8)
	.dw		60						; out_min
	.dw		250						; out_max
	.dw		set_flow


vc_cmv_pid_table:
	; flow<sp, pid
	.dw		8,flow_pid_5
	.dw		13,flow_pid_10
	.dw		18,flow_pid_15
	.dw		23,flow_pid_20
	.dw		28,flow_pid_25
	.dw		33,flow_pid_30
	.dw		38,flow_pid_35
	.dw		43,flow_pid_40
	.dw		48,flow_pid_45
	.dw		53,flow_pid_50
	.dw		58,flow_pid_55
	.dw		61,flow_pid_60

; Verify range of last line!!!


.equ flow_table_first = 14
.equ flow_table_last  = 78
vc_cmv_flow_table:
	.dw  39 , 15      ; 14
	.dw  41 , 16      ; 15
	.dw  43 , 18      ; 16
	.dw  45 , 20      ; 17
	.dw  48 , 22      ; 18
	.dw  51 , 25      ; 19
	.dw  54 , 28      ; 20
	.dw  57 , 31      ; 21
	.dw  60 , 34      ; 22
	.dw  63 , 38      ; 23
	.dw  66 , 42      ; 24
	.dw  69 , 46      ; 25
	.dw  72 , 50      ; 26
	.dw  75 , 55      ; 27
	.dw  78 , 60      ; 28
	.dw  81 , 65      ; 29
	.dw  84 , 70      ; 30
	.dw  87 , 76      ; 31
	.dw  90 , 82      ; 32
	.dw  93 , 88      ; 33
	.dw  96 , 94      ; 34
	.dw  99 , 100      ; 35
	.dw  102 , 107      ; 36
	.dw  105 , 114      ; 37
	.dw  108 , 121      ; 38
	.dw  111 , 128      ; 39
	.dw  114 , 135      ; 40
	.dw  117 , 143      ; 41
	.dw  120 , 151      ; 42
	.dw  123 , 159      ; 43
	.dw  126 , 167      ; 44
	.dw  129 , 175      ; 45
	.dw  132 , 184      ; 46
	.dw  136 , 193      ; 47
	.dw  140 , 202      ; 48
	.dw  144 , 211      ; 49
	.dw  148 , 220      ; 50
	.dw  152 , 229      ; 51
	.dw  156 , 239      ; 52
	.dw  160 , 249      ; 53
	.dw  164 , 259      ; 54
	.dw  167 , 269      ; 55
	.dw  170 , 279      ; 56
	.dw  173 , 289      ; 57
	.dw  176 , 300      ; 58
	.dw  179 , 311      ; 59
	.dw  182 , 322      ; 60
	.dw  185 , 333      ; 61
	.dw  188 , 344      ; 62
	.dw  191 , 355      ; 63
	.dw  194 , 366      ; 64
	.dw  197 , 378      ; 65
	.dw  200 , 390      ; 66
	.dw  203 , 402      ; 67
	.dw  206 , 414      ; 68
	.dw  209 , 426      ; 69
	.dw  212 , 437      ; 70
	.dw  215 , 448      ; 71
	.dw  218 , 459      ; 72
	.dw  221 , 470      ; 73
	.dw  224 , 481      ; 74
	.dw  227 , 492      ; 75
	.dw  230 , 503      ; 76
	.dw  233 , 514      ; 77
	.dw  236 , 525      ; 78





.dseg
vc_cmv_ctrl_curr_flow:		.byte 1
vc_cmv_insp_time_extra:		.byte 1
vc_cmv_insp_rise_time:		.byte 1
vc_cmv_exp_time_save:		.byte 2
vc_cmv_vol_thr_low:			.byte 2
vc_cmv_vol_thr_high:		.byte 2
vc_cmv_vol_err_min:			.byte 1

vc_cmv_last_set_vol:		.byte 2
vc_cmv_last_set_time:		.byte 2

vc_cmv_alarm:				.byte 1
vc_cmv_insp_flag:			.byte 1
vc_cmv_over_flag:			.byte 1

insp_flow_add:			.byte 1
vc_cmv_vol_low:			.byte 2
vc_cmv_vol_high:		.byte 2
vc_cmd_last_set_vol:	.byte 2
vc_cmd_last_set_flow:	.byte 1
vc_cmd_last_set_time:	.byte 2


.cseg
main_vc_cmv_pid:
	ldi		ZH,high(vc_cmv_pid_table*2)			; pid settings
	ldi		ZL,low(vc_cmv_pid_table*2)
	rcall	vc_cmv_sp_flow						; sp flow compensated
	mov		R16,XL
	rcall	vc_cmv_load_pid_settings

	rcall	vc_cmv_sp_flow						; sp flow compensated
	sts		pid_sp,XL
	sts		pid_sp+1,XH

	call	sensors_volume_reset				; clear volume data
	call	sensors_max_reset

	call	stepper_alarm_enable				; stepper alarm enable
	rcall	process_inspiration_flow_pid
	rcall	process_inspiration_flow_vol		; extra time to try complete minimum volume
	rcall	screen_tx_data_sensors
	call	stepper_alarm_disable				; clear alarm flag

	rcall	process_pause
	rcall	process_expiration

	rcall	main_vc_cmv_follow_volume
	ret



; sp flow compensated
vc_cmv_sp_flow:
	lds		XH,set_flow+1						; compensated target flow
	lds		XL,set_flow
	lds		R16,insp_flow_add
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	cpi		XH,0
	brne	vc_cmv_sp_flow_top
	cpi		XL,60
	brsh	vc_cmv_sp_flow_top
	cpi		XL,5
	brsh	vc_cmv_sp_flow_ret
	ldi		XL,5
	ldi		XH,0
	rjmp	vc_cmv_sp_flow_ret
vc_cmv_sp_flow_top:
	ldi		XL,60
	ldi		XH,00
vc_cmv_sp_flow_ret:
	ret




main_vc_cmv_follow_volume:
	lds		R16,vc_cmv_insp_time_extra			; check if extra inspiration time was needed
	cpi		R16,14
	brsh	main_vc_cmv_follow_volume_inc

	lds		YH,sensors_volume_max+3				; check tidal volume
	lds		YL,sensors_volume_max+2
	lds		ZH,vc_cmv_vol_high+1
	lds		ZL,vc_cmv_vol_high
	cp		YL,ZL
	cpc		YH,ZH
	brlo	main_vc_cmv_follow_volume_no_dec
	lds		R16,insp_flow_add
	dec		R16
	breq	main_vc_cmv_follow_volume_ret
	sts		insp_flow_add,R16
	rjmp	main_vc_cmv_follow_volume_ret
main_vc_cmv_follow_volume_no_dec:


	lds		ZH,vc_cmv_vol_low+1
	lds		ZL,vc_cmv_vol_low
	cp		YL,ZL
	cpc		YH,ZH
	brsh	main_vc_cmv_follow_volume_ret
main_vc_cmv_follow_volume_inc:
	lds		R16,insp_flow_add
	inc		R16
	breq	main_vc_cmv_follow_volume_ret
	sts		insp_flow_add,R16

main_vc_cmv_follow_volume_ret:
	ret




; input:	z   -> pid settings table
;			R16 -> set point
vc_cmv_load_pid_settings:
	lpm		R17,Z+									; setpoint
	lpm		R18,Z+
	lpm		R18,Z+									; pid table
	lpm		R19,Z+

	cp		R16,R17
	brsh	vc_cmv_load_pid_settings

	mov		ZL,R18
	mov		ZH,R19
	lsl		ZL
	rol		ZH

	rcall	load_pid_settings

	ret



; do not modify R16
vc_cmv_chk_compensated_flow:
	lds		R17,insp_flow_add						; check compensated flow
	lds		ZH,set_vol+1
	lds		ZL,set_vol
	lds		R18,set_insp_time_cs
	lds		R19,set_insp_time_cs+1
	lds		YL,set_flow
	lds		XL,vc_cmd_last_set_vol
	lds		XH,vc_cmd_last_set_vol+1
	lds		R20,vc_cmd_last_set_time
	lds		R21,vc_cmd_last_set_time+1
	lds		YH,vc_cmd_last_set_flow
	cpi		R17,0
	breq	vc_cmv_chk_compensated_flow_restart

	cp		ZL,XL									; check volume change
	cpc		ZH,XH
	brlo	vc_cmv_chk_compensated_flow_restart
	breq	vc_cmv_chk_compensated_flow_no_vol_change
	cp		R20,R18
	cpc		R21,R19
	brlo	vc_cmv_chk_compensated_flow_restart
	rjmp	vc_cmv_chk_compensated_flow_calc
vc_cmv_chk_compensated_flow_no_vol_change:

	cp		R20,R18									; check time change
	cpc		R21,R19
	brlo	vc_cmv_chk_compensated_flow_restart
	breq	vc_cmv_chk_compensated_flow_ret
vc_cmv_chk_compensated_flow_calc:

	add		R17,YH									; check last compensated flow
	cp		YL,R17
	brsh	vc_cmv_chk_compensated_flow_restart
	sub		R17,YL
	rjmp	vc_cmv_chk_compensated_flow_update

vc_cmv_chk_compensated_flow_restart:
	ldi		R17,0									; Init compensated flow

vc_cmv_chk_compensated_flow_update:
	sts		insp_flow_add,R17
	sts		vc_cmd_last_set_vol,ZL
	sts		vc_cmd_last_set_vol+1,ZH
	sts		vc_cmd_last_set_time,R18
	sts		vc_cmd_last_set_time+1,R19
	sts		vc_cmd_last_set_flow,YL

vc_cmv_chk_compensated_flow_ret:
	ret



process_inspiration_flow_pid:
;	ldi		R16,1								; transmit inspiracion event to screen
;	sts		vent_phase,R16
;	rcall	screen_tx_event

	lds		XL,set_insp_time_cs					; inspiration time
	lds		XH,set_insp_time_cs+1
process_inspiration_flow_pid_loop:
	push	XL
	push	XH
	rcall	pid_wait_sampling_time				; wait pid sampling time
	lds		XH,sensors_flow_value+3				; control variable
	lds		XL,sensors_flow_value+2
	lds		R16,sensors_flow_value+1
	rcall	pid_process							; pid control
	sts		stepper_speed_sp,XL					; update control variable
	sts		stepper_speed_sp+1,XH
	call	sensors_volume_reset_inspDetect
	rcall	screen_tx_data_PFV

	call	check_alarms						; skip if alarm detected
	pop		XH
	pop		XL
	brne	process_inspiration_flow_pid_ret

	sbiw	XL,1
	brne	process_inspiration_flow_pid_loop
process_inspiration_flow_pid_ret:
	ret




process_inspiration_flow_vol:
	ldi		R16,0								; extra inpiration time for tidal volume
	sts		vc_cmv_insp_time_extra,R16
	lds		YH,sensors_volume_max+3				; check tidal volume
	lds		YL,sensors_volume_max+2
	lds		ZH,vc_cmv_vol_low+1
	lds		ZL,vc_cmv_vol_low
	cp		YL,ZL
	cpc		YH,ZH
	brsh	process_inspiration_flow_vol_ret

process_inspiration_flow_vol_loop:
	rcall	pid_wait_sampling_time				; wait pid sampling time
	lds		XH,sensors_flow_value+3				; control variable
	lds		XL,sensors_flow_value+2
	lds		R16,sensors_flow_value+1
	rcall	pid_process							; pid control
	sts		stepper_speed_sp,XL					; update control variable
	sts		stepper_speed_sp+1,XH
	rcall	screen_tx_data_PFV
	lds		R16,vc_cmv_insp_time_extra
	inc		R16
	sts		vc_cmv_insp_time_extra,R16

	call	check_alarms						; skip if alarm detected
	brne	process_inspiration_flow_vol_ret

	lds		XH,sensors_volume_value+3			; control variable
	lds		XL,sensors_volume_value+2
	lds		ZH,vc_cmv_vol_low+1
	lds		ZL,vc_cmv_vol_low
	cp		XL,ZL
	cpc		XH,ZH
	brsh	process_inspiration_flow_vol_ret
	cpi		R16,255
	brne	process_inspiration_flow_vol_loop
process_inspiration_flow_vol_ret:
	ret











; volume control continous mandatory ventilation
main_vc_cmv:
	ldi		ZH,high(vc_cmv_pid_table*2)			; pid settings
	ldi		ZL,low(vc_cmv_pid_table*2)
	rcall	vc_cmv_load_sp
;	lds		R16,set_flow
	rcall	vc_cmv_load_pid_settings

;	ldi		R16,15
;	sts		insp_flow_add,R16
;	rcall	vc_cmv_sp_flow						; sp flow compensated
;	ldi		XL,25 ;25
	lds		XL,EEPROM_START+ee_pulse_flow_sp
	sts		pid_sp,XL
;	sts		pid_sp+1,XH

	call	sensors_volume_reset				; clear volume data
	call	sensors_max_reset

	call	stepper_alarm_enable				; stepper alarm enable
	rcall	process_inspiration_flow
	rcall	screen_tx_data_sensors

	lds		R16,stepper_pressure_alarm			; save alarm pressure for later
	sts		vc_cmv_alarm,R16
	call	stepper_alarm_disable				; clear alarm flag

	rcall	process_pause

	rcall	vc_cmv_calc_time_exp
	rcall	process_expiration
	rcall	vc_cmv_restore_time_exp

	rcall	vc_cmv_follow_volume
	ret


vc_cmv_load_sp:
	lds		R16,EEPROM_START+ee_pulse_flow_sp
	cpi		R16,15
	brsh	vc_cmv_load_sp_top
	ldi		R16,15
	rjmp	vc_cmv_load_sp_ret
vc_cmv_load_sp_top:
	cpi		R16,50
	brlo	vc_cmv_load_sp_ret
	ldi		R16,50
vc_cmv_load_sp_ret:
;	ldi		R16,25
	ret



.dseg
process_flow_init_pulse_time:	.byte 1
.cseg
process_inspiration_flow:
;	ldi		R16,1								; transmit inspiracion event to screen
;	sts		vent_phase,R16
;	rcall	screen_tx_event

	lds		XL,set_insp_time_cs					; inspiration time
	lds		XH,set_insp_time_cs+1
	lds		R16,vc_cmv_insp_time_extra
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	sts		vc_cmv_insp_rise_time,R16
	sts		vc_cmv_insp_flag,R16
	sts		exp_start_flag,R16
	sts		vc_cmv_over_flag,R16

	;ldi		R16,50 ;15
	lds		R16,EEPROM_START+ee_pulse_time
	sts		process_flow_init_pulse_time,R16
	rcall	ctrl_flow_parameters_read
	sts		stepper_speed_sp,R17				; update control variable
	sts		stepper_speed_sp+1,R16
process_inspiration_flow_loop1:
	push	R16
	push	R17
	push	XL
	push	XH
	push	YL
	push	YH
	rcall	pid_wait_sampling_time				; wait pid sampling time
	call	check_alarms						; skip if alarm detected
	call	sensors_volume_reset_inspDetect
	rcall	screen_tx_data_PFV
	pop		YH
	pop		YL
	pop		XH
	pop		XL
	pop		R17
	pop		R16
	sub		R16,YL
	sbc		R17,YH
	rcall	vc_cmv_check_overflow
	push	R17
	rcall	vc_cmv_check_init_pulse
	sts		stepper_speed_sp,R17				; update control variable
	pop		R17

	rcall	check_inspiration_start
	brne	process_inspiration_flow_loop1

	sbiw	XL,1
	brne	process_inspiration_flow_loop1

	ldi		R16,0
	sts		stepper_speed_sp,R16				; pause stepper
	sts		stepper_speed_sp+1,R16
	ret



; do not change X,Y,R16,R17
check_inspiration_start:
	lds		R18,vc_cmv_insp_flag
	cpi		R18,1
	breq	check_inspiration_start_ret

	lds		ZH,sensors_flow_value+3
	lds		ZL,sensors_flow_value+2
	sbrc	ZH,7
	rjmp	check_inspiration_start_ko
	cpi		ZH,0
	brne	check_inspiration_start_ok
	cpi		ZL,2							; inspiration flow threshold
	brsh	check_inspiration_start_ok
check_inspiration_start_ko:
	lds		R18,vc_cmv_insp_rise_time
	cpi		R18,150							; max inspiration time
	brsh	check_inspiration_start_ok
	inc		R18
	sts		vc_cmv_insp_rise_time,R18
	clz
	rjmp	check_inspiration_start_ret
check_inspiration_start_ok:
	lds		R18,vc_cmv_insp_rise_time
	lds		R18,exp_fall_time				; compensate time with expiration fall
	sub		XL,R18
	ldi		R18,0
	sbc		XH,R18
	brcs	check_inspiration_start_min_exp
	brne	check_inspiration_start_flag
check_inspiration_start_min_exp:
	ldi		XL,1
	ldi		XH,0
check_inspiration_start_flag:
	ldi		R18,1
	sts		vc_cmv_insp_flag,R18
	sez
check_inspiration_start_ret:
	ret


; do not modify R16,R17,XL,XH,YL,YH
; but if overflow -> R17=0
vc_cmv_check_overflow:
	lds		R19,sensors_volume_max+3				; course flow tunning up
	lds		R18,sensors_volume_max+2
	lds		ZH,set_vol+1
	lds		ZL,set_vol
	lds		R20,vc_cmv_ctrl_curr_flow			; read flow index
	cpi		R20,flow_table_first+1
	brlo	vc_cmv_check_overflow_flow_min
	lds		ZH,vc_cmv_vol_thr_high+1
	lds		ZL,vc_cmv_vol_thr_high
	rjmp	vc_cmv_check_overflow_flow_check
vc_cmv_check_overflow_flow_min:
	lds		ZH,vc_cmv_vol_thr_low+1
	lds		ZL,vc_cmv_vol_thr_low

vc_cmv_check_overflow_flow_check:
	cp		R18,ZL
	cpc		R19,ZH
	brlo	vc_cmv_check_overflow_ret
	ldi		R17,1
	sts		vc_cmv_over_flag,R17
	ldi		R17,0
vc_cmv_check_overflow_ret:
	ret


vc_cmv_check_init_pulse:
	push	R18
	lds		R18,process_flow_init_pulse_time
	subi	R18,1
	brcs	vc_cmv_check_init_pulse_ret
	sts		process_flow_init_pulse_time,R16
	lds		R18,EEPROM_START+ee_pulse_min_speed
	cp		R17,R18
	brsh	vc_cmv_check_init_pulse_ret
	push	R16
	push	R17
	push	XL
	push	XH
	push	YL
	push	YH
	lds		XH,sensors_flow_value+3				; control variable
	lds		XL,sensors_flow_value+2
	lds		R16,sensors_flow_value+1
	rcall	pid_process							; pid control
;	sts		stepper_speed_sp,XL					; update control variable
;	sts		stepper_speed_sp+1,XH
	cpi		XH,0
	breq	vc_cmv_check_init_pulse_set
	ldi		XL,$ff
vc_cmv_check_init_pulse_set:
	mov		R18,XL
	pop		YH
	pop		YL
	pop		XH
	pop		XL
	pop		R17
	pop		R16
	mov		R17,R18
vc_cmv_check_init_pulse_ret:
	pop		R18
	ret


ctrl_flow_parameters_read:
	lds		R16,vc_cmv_ctrl_curr_flow			; read flow index
	subi	R16,flow_table_first

	brcc	ctrl_flow_parameters_read_min		; check minimum value
	ldi		R16,flow_table_first
	sts		vc_cmv_ctrl_curr_flow,R16
	clr		R16
ctrl_flow_parameters_read_min:

	cpi		R16,(flow_table_last-flow_table_first)+1	; check maximun value
	brlo	ctrl_flow_parameters_read_max
	ldi		R16,flow_table_last
	sts		vc_cmv_ctrl_curr_flow,R16
	ldi		R16,flow_table_last-flow_table_first
ctrl_flow_parameters_read_max:

	ldi		ZH,high(vc_cmv_flow_table*2)		; read table
	ldi		ZL,low(vc_cmv_flow_table*2)
	clr		R17
	lsl		R16
	rol		R17
	lsl		R16
	rol		R17
	add		ZL,R16
	adc		ZH,R17
	lpm		R17,Z+
	lpm		R16,Z+
	lpm		YL,Z+
	lpm		YH,Z+
	ret


vc_cmv_calc_time_exp:
	lds		R16,vc_cmv_insp_rise_time
	lds		XH,set_exp_time_cs+1
	lds		XL,set_exp_time_cs
	sts		vc_cmv_exp_time_save+1,XH
	sts		vc_cmv_exp_time_save,XL
	sub		XL,R16
	ldi		R16,0
	sbc		XH,R16
	brcs	vc_cmv_sub_time_exp_min
	brne	vc_cmv_sub_time_exp_update
vc_cmv_sub_time_exp_min:
	ldi		XL,1
	ldi		XH,0
vc_cmv_sub_time_exp_update:
	lds		R16,exp_fall_time
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	sts		set_exp_time_cs+1,XH
	sts		set_exp_time_cs,XL
	ret


vc_cmv_restore_time_exp:
	lds		XH,vc_cmv_exp_time_save+1
	lds		XL,vc_cmv_exp_time_save
	sts		set_exp_time_cs+1,XH
	sts		set_exp_time_cs,XL
	ret
	

vc_cmv_copy_set_values:
	ld		R17,X+
	sts		set_vol,R17
	ld		R17,X+
	sts		set_vol+1,R17

	ld		R17,X+
	sts		set_flow,R17
	ld		R17,X+
	sts		set_flow+1,R17

	ld		R17,X+
	sts		set_insp_time_cs,R17
	ld		R17,X+
	sts		set_insp_time_cs+1,R17

	ld		R17,X+
	sts		set_pause_time_cs,R17
	ld		R17,X+
	sts		set_pause_time_cs+1,R17

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

	ldi		R17,0
	sts		pc_pressure_add,R17

	rcall	vc_cmv_chk_ventilation_changes
	rcall	vc_cmv_calc_vol_thresholds
	ret


; do not modify R16
vc_cmv_chk_ventilation_changes:
	lds		XH,set_vol+1
	lds		XL,set_vol
	lds		YH,set_insp_time_cs+1
	lds		YL,set_insp_time_cs

	lds		ZL,vc_cmv_last_set_vol					; check volume change
	lds		ZH,vc_cmv_last_set_vol+1
	cp		XL,ZL
	cpc		XH,ZH
	brne	vc_cmv_chk_ventilation_changes_update

	lds		ZL,vc_cmv_last_set_time					; check time change
	lds		ZH,vc_cmv_last_set_time+1
	cp		YL,ZL
	cpc		YH,ZH
	breq	vc_cmv_chk_ventilation_changes_ret

vc_cmv_chk_ventilation_changes_update:
	sts		vc_cmv_last_set_vol,XL
	sts		vc_cmv_last_set_vol+1,XH
	sts		vc_cmv_last_set_time,YL
	sts		vc_cmv_last_set_time+1,YH
	ldi		R17,0
	sts		vc_cmv_insp_time_extra,R17
	sts		exp_fall_time,R17

	lds		R17,set_flow
	subi	R17,-3
	sts		vc_cmv_ctrl_curr_flow,R17

vc_cmv_chk_ventilation_changes_ret:
	ret


; do not modify R16
vc_cmv_calc_vol_thresholds:
	lds		ZH,set_vol+1							; calc vol thresholds within +/-6.25%
	lds		ZL,set_vol
	mov		XH,ZH
	mov		XL,ZL
	lsr		XH
	ror		XL
	lsr		XH
	ror		XL
	lsr		XH
	ror		XL
	lsr		XH
	ror		XL

	add		ZL,XL
	adc		ZH,XH
	sts		vc_cmv_vol_high+1,ZH
	sts		vc_cmv_vol_high,ZL
	sts		vc_cmv_vol_thr_high+1,ZH
	sts		vc_cmv_vol_thr_high,ZL
	lds		ZH,set_vol+1
	lds		ZL,set_vol
	sub		ZL,XL
	sbc		ZH,XH
	sts		vc_cmv_vol_low+1,ZH
	sts		vc_cmv_vol_low,ZL
	sts		vc_cmv_vol_thr_low+1,ZH
	sts		vc_cmv_vol_thr_low,ZL
	lsr		XH
	ror		XL
	lsr		XH
	ror		XL
	ldi		XH,0
	adc		XL,XH
	sts		vc_cmv_vol_err_min,XL
	ret



process_pause:
	ldi		R16,2									; transmit pause event to screen
	sts		vent_phase,R16
	call	screen_tx_event
	ldi		R16,0
	sts		stepper_speed_sp,R16					; pause stepper
	sts		stepper_speed_sp+1,R16
	lds		XL,set_pause_time_cs
	lds		XH,set_pause_time_cs+1
	
	lds		YL,vc_cmv_insp_time_extra				; compensate inspiration extra time
	sub		XL,YL
	brsh	process_pause_min
	ldi		XL,0
process_pause_min:
	cpi		XL,pause_min_time_cs
	brsh	process_pause_loop
	ldi		XL,pause_min_time_cs

process_pause_loop:
	push	XL
	rcall	pid_wait_sampling_time					; wait pid sampling time
	call	screen_tx_data_PFV
	pop		XL
	dec		XL
	brne	process_pause_loop
	ret




vc_cmv_follow_volume:
	lds		R16,vc_cmv_alarm					; decrement if pressure alarm
	cpi		R16,0
	breq	vc_cmv_follow_volume_no_alarm
	lds		R16,vc_cmv_ctrl_curr_flow
	dec		R16
	sts		vc_cmv_ctrl_curr_flow,R16
	rjmp	vc_cmv_follow_volume_ret
vc_cmv_follow_volume_no_alarm:

	lds		R16,vc_cmv_over_flag
	cpi		R16,0
	breq	vc_cmv_follow_volume_no_overflow
	lds		R16,vc_cmv_ctrl_curr_flow
	dec		R16
	sts		vc_cmv_ctrl_curr_flow,R16
	rjmp	vc_cmv_follow_volume_ret
vc_cmv_follow_volume_no_overflow:

	rcall	vc_cmv_flow_up						; course flow tunning up
	breq	vc_cmv_follow_volume_ret

	rcall	vc_cmv_flow_down					; flow tunning down
	breq	vc_cmv_follow_volume_ret

	rcall	vc_cmv_flow_fine
vc_cmv_follow_volume_ret:
	ret



vc_cmv_flow_up:
	lds		XH,sensors_volume_max+3				; course flow tunning up
	lds		XL,sensors_volume_max+2
	lds		YH,vc_cmv_vol_thr_low+1
	lds		YL,vc_cmv_vol_thr_low
	cp		XL,YL
	cpc		XH,YH
	brlo	vc_cmv_flow_course_calc

	lds		YH,vc_cmv_vol_thr_high+1
	lds		YL,vc_cmv_vol_thr_high
	cp		XL,YL
	cpc		XH,YH
	brlo	vc_cmv_flow_up_ko

vc_cmv_flow_course_calc:
	lds		ZH,set_vol+1						; calc next flow. course flow up.
	lds		ZL,set_vol
	lds		R16,vc_cmv_ctrl_curr_flow
	cpi		R16,flow_table_last
	brsh	vc_cmv_flow_up_ko
	clr		R7
	clr		R6
	mov		R5,XH
	mov		R4,XL
	clr		R3
	mul		R16,ZH
	mov		R2,R1
	mov		R8,R0
	mul		R16,ZL
	add		R1,R8
	adc		R2,R3
	adc		R3,R3
;*** Dividendo 	R3:R0 (H-L)
;*** Divisor	R7:R4 (H-L)
;*** = Cociente	R3:R0 (H-L)
;***   Residuo  R11:R8 (H-L)
	call	Divide32
	or		R3,R2
	or		R2,R1
	brne	vc_cmv_flow_up_top
	mov		R16,R0
	cpi		R16,flow_table_last
	brlo	vc_cmv_flow_up_max
vc_cmv_flow_up_top:
	ldi		R16,flow_table_last
vc_cmv_flow_up_max:

	cpi		R16,flow_table_first
	brsh	vc_cmv_flow_up_min
	ldi		R16,flow_table_first
vc_cmv_flow_up_min:

	lds		R17,vc_cmv_ctrl_curr_flow
	cp		R16,R17
	breq	vc_cmv_flow_up_ko
	brsh	vc_cmv_flow_up_inc
	mov		R18,R17
	sub		R18,R16
	cpi		R18,20
	brlo	vc_cmv_flow_up_update
	mov		R16,R17
	subi	R16,20
	rjmp	vc_cmv_flow_up_update
vc_cmv_flow_up_inc:
	mov		R18,R16
	sub		R18,R17
	cpi		R18,20
	brlo	vc_cmv_flow_up_update
	mov		R16,R17
	subi	R16,-20
vc_cmv_flow_up_update:
	add		R16,R17
	ror		R16
	ldi		R17,0
	adc		R16,R17
	sts		vc_cmv_ctrl_curr_flow,R16
	ldi		R16,0
	sts		vc_cmv_insp_time_extra,R16
	sez
	rjmp	vc_cmv_flow_up_ret
vc_cmv_flow_up_ko:
	clz
vc_cmv_flow_up_ret:
	ret



vc_cmv_flow_down:
	ret
	lds		XH,sensors_volume_max+3				; course flow down?
	lds		XL,sensors_volume_max+2
	lds		YH,vc_cmv_vol_thr_high+1
	lds		YL,vc_cmv_vol_thr_high
	cp		XL,YL
	cpc		XH,YH
	brsh	vc_cmv_flow_down_ok

	lds		R16,vc_cmv_insp_time_extra			; fine flow down?
	cpi		R16,0
	brne	vc_cmv_flow_down_ko

	lds		YH,set_vol+1
	lds		YL,set_vol
	cp		XL,YL
	cpc		XH,YH
	brlo	vc_cmv_flow_down_ko

vc_cmv_flow_down_ok:
	lds		R16,vc_cmv_ctrl_curr_flow					; course flow down.
	dec		R16
	cpi		R16,flow_table_first
	brsh	vc_cmv_flow_down_min
	ldi		R16,flow_table_first
vc_cmv_flow_down_min:
	sts		vc_cmv_ctrl_curr_flow,R16
	ldi		R16,0
	sts		vc_cmv_insp_time_extra,R16
	sez
	rjmp	vc_cmv_flow_down_ret
vc_cmv_flow_down_ko:
	clz
vc_cmv_flow_down_ret:
	ret


vc_cmv_flow_fine:
	lds		YH,set_vol+1						; fine tunning
	lds		YL,set_vol
	lds		XH,sensors_volume_max+3
	lds		XL,sensors_volume_max+2
	lds		R16,vc_cmv_insp_time_extra
	lds		R17,vc_cmv_vol_err_min

	cp		XL,YL
	cpc		XH,YH
	brsh	vc_cmv_flow_fine_pos
	sub		YL,XL
	sbc		YH,XH
	cpi		YH,0
	brne	vc_cmv_flow_fine_up
	cp		YL,R17
	brsh	vc_cmv_flow_fine_up
	rjmp	vc_cmv_flow_fine_ret
vc_cmv_flow_fine_pos:

	sub		XL,YL
	sbc		XH,YH
	cpi		XH,0
	brne	vc_cmv_flow_fine_down
	cp		XL,R17
	brlo	vc_cmv_flow_fine_ret
vc_cmv_flow_fine_down:
	dec		R16									; fine flow down.
	rjmp	vc_cmv_flow_fine_set
vc_cmv_flow_fine_up:
	inc		R16									; fine flow up.
vc_cmv_flow_fine_set:
	cpi		R16,50
	brsh	vc_cmv_flow_fine_ret
	sts		vc_cmv_insp_time_extra,R16
vc_cmv_flow_fine_ret:
	ret



vc_cmv_follow_volume_v3:
	lds		XH,sensors_volume_max+3				; course flow tunning up
	lds		XL,sensors_volume_max+2
	lds		ZH,set_vol+1						; calc next flow. course flow up.
	lds		ZL,set_vol
	mov		YL,ZH
	mov		YH,ZL
	lsl		YH									; 1/32 error
	rol		YL
	lsl		YH
	rol		YL
	lsl		YH
	rol		YL
	clr		YH
	cp		XL,ZL
	cpc		XH,ZH
	brlo	vc_cmv_follow_volume_v3_inc
	sub		XL,ZL
	sbc		XH,ZH
	cp		XL,YL
	cpc		XH,YH
	brlo	vc_cmv_follow_volume_v3_ret
	lds		R16,vc_cmv_ctrl_curr_flow
	dec		R16
	sts		vc_cmv_ctrl_curr_flow,R16
	rjmp	vc_cmv_follow_volume_v3_ret
vc_cmv_follow_volume_v3_inc:
	sub		ZL,XL
	sbc		ZH,XH
	cp		ZL,YL
	cpc		ZH,YH
	brlo	vc_cmv_follow_volume_v3_ret
	lds		R16,vc_cmv_ctrl_curr_flow
	inc		R16
	sts		vc_cmv_ctrl_curr_flow,R16
vc_cmv_follow_volume_v3_ret:
	ret
