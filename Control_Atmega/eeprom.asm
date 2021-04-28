/*
 * eeprom.asm
 *
 *  Created: 07/07/2020 11:54:46 p. m.
 *   Author: Javier
 */ 

.eseg
; EEPROM_START+
ee_pressure_zero_cal:		.db 0x88,0x76,0x05,0x00		; LO-HI
ee_flow_zero_cal:			.db 0xaa,0x82,0xfe,0xff		; LO-HI

ee_o2_21_cal:				.db 0xe5,0xf8,0x1b,0x00		; LO-HI
ee_o2_hi_cal:				.db 0x72,0xf4,0x9b,0x00		; LO-HI
ee_o2_hi_ref:				.db 97

ee_flow_insp_range0:		.db 0x40,0x00				; LO-HI
ee_flow_insp_gain0_cal:		.db 0x00,0x00,0x00			; LO-HI
ee_flow_insp_off0_cal:		.db 0x00,0x00,0x00			; LO-HI

ee_flow_insp_range1:		.db 0x40,0x13				; LO-HI
ee_flow_insp_gain1_cal:		.db 0x55,0x55,0x01			; LO-HI
ee_flow_insp_off1_cal:		.db 0x55,0x55,0x00			; LO-HI

ee_flow_insp_range2:		.db 0x40,0x2d				; LO-HI
ee_flow_insp_gain2_cal:		.db 0x00,0x00,0x01			; LO-HI
ee_flow_insp_off2_cal:		.db 0x00,0xc0,0x06			; LO-HI

ee_flow_insp_range3:		.db 0x00,0xfe				; LO-HI
ee_flow_insp_gain3_cal:		.db 0x8f,0xb5,0x00			; LO-HI
ee_flow_insp_off3_cal:		.db 0xb0,0xdb,0x13			; LO-HI

ee_flow_insp_range4:		.db 0xff,0xff				; LO-HI
ee_flow_insp_gain4_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_flow_insp_off4_cal:		.db 0xff,0xff,0xff			; LO-HI



ee_flow_exp_range0:			.db 0x40,0x00				; LO-HI
ee_flow_exp_gain0_cal:		.db 0x00,0x00,0x00			; LO-HI
ee_flow_exp_off0_cal:		.db 0x00,0x40,0x00			; LO-HI

ee_flow_exp_range1:			.db 0x01,0x2a				; LO-HI
ee_flow_exp_gain1_cal:		.db 0x08,0x1e,0x01			; LO-HI
ee_flow_exp_off1_cal:		.db 0x08,0x1e,0x01			; LO-HI

ee_flow_exp_range2:			.db 0x00,0xfe				; LO-HI
ee_flow_exp_gain2_cal:		.db 0x00,0xc8,0x00			; LO-HI
ee_flow_exp_off2_cal:		.db 0x00,0x3c,0x0f			; LO-HI

ee_flow_exp_range3:			.db 0xff,0xff				; LO-HI
ee_flow_exp_gain3_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_flow_exp_off3_cal:		.db 0xff,0xff,0xff			; LO-HI

ee_flow_exp_range4:			.db 0xff,0xff				; LO-HI
ee_flow_exp_gain4_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_flow_exp_off4_cal:		.db 0xff,0xff,0xff			; LO-HI



ee_press_range0:		.db 0x00,0xfe				; LO-HI
ee_press_gain0_cal:		.db 0x00,0x00,0x01			; LO-HI
ee_press_off0_cal:		.db 0x00,0x00,0x00			; LO-HI

ee_press_range1:		.db 0xff,0xff				; LO-HI
ee_press_gain1_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_press_off1_cal:		.db 0xff,0xff,0xff			; LO-HI

ee_press_range2:		.db 0xff,0xff				; LO-HI
ee_press_gain2_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_press_off2_cal:		.db 0xff,0xff,0xff			; LO-HI

ee_press_range3:		.db 0xff,0xff				; LO-HI
ee_press_gain3_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_press_off3_cal:		.db 0xff,0xff,0xff			; LO-HI

ee_press_range4:		.db 0xff,0xff				; LO-HI
ee_press_gain4_cal:		.db 0xff,0xff,0xff			; LO-HI
ee_press_off4_cal:		.db 0xff,0xff,0xff			; LO-HI



ee_pulse_time:				.db 00						; pulse time (ej:50)
ee_pulse_flow_sp:			.db 25						; flow set point (ej:25)
ee_pulse_min_speed:			.db 70						; minimum motor speed (ej:70)

ee_pressure_max:			.db 38						; maximum pressure
ee_flow_max:				.db 150						; maximum flow
ee_pos_alarm_thr:			.db low(1500),high(1500)	; stepper position for alarm evaluation
ee_dis_alarm_pres_thr:		.db 1						; pressure (cmh2o) threshold for patient disconection alarm. 0=disable
ee_dis_alarm_flow_thr:		.db 5						; flow (l/m) threshold for patient disconection alarm
ee_obs_alarm_pres_thr:		.db 38						; pressure (cmh2o) threshold for obstruction alarm. 0=disable
ee_obs_alarm_flow_thr:		.db 10						; flow (l/m) threshold for obstruction alarm
ee_res_alarm_thr:			.db 100						; resistance threshold for obstruction alarm. 0=disable
ee_res_alarm_drv_press_thr:	.db 2						; minimum driving pressure for resistance calculation

.dseg
ee_write_tmr_s:			.byte 1


.cseg
eeprom_write_byte:
	st		X+,R16

	push	XL
	push	XH
	rcall	eeprom_write_page
	pop		XH
	pop		XL
	ret


eeprom_write_page:
	ldi		R17,CPU_CCP_SPM_gc
	ldi		R18,0x03					; ERWP Erase and write page
	cli
	out		CPU_CCP,R17
	sts		NVMCTRL_CTRLA,R18
	sei
eeprom_write_page_loop:
	task_change
	lds		R16,NVMCTRL_STATUS
	sbrc	R16,NVMCTRL_EEBUSY_bp
	rjmp	eeprom_write_page_loop
	ret
