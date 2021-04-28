/*
 * screen.asm
 *
 *  Created: 12/05/2020 09:50:13 p. m.
 *   Author: Javier
 */ 


;	PF0: (HI-Z)		screen tx. usart 2
;   PF1: (HI-Z)     screen rx. usart 2
.equ	screen_BPS = 115200
.equ	screen_RX_BUFFER_LEN = 64
.equ	screen_rx_TIMEOUT_ms = 100

.dseg
screen_rx_buffer:	.byte screen_RX_BUFFER_LEN
screen_rx_ptrwr:	.byte 1
screen_rx_ptrrd:	.byte 1
screen_tx_tmr_ms:	.byte 1
screen_rx_tmr_ms:	.byte 1
screen_cmd_buffer:	.byte 2+screen_RX_BUFFER_LEN
vent_mode_data:		.byte screen_RX_BUFFER_LEN
screen_send_status:	.byte 1

.cseg
screen_task:
	rcall	screen_init
screen_loop:
	rcall	screen_rx_frame
	rcall	screen_cmd_stop
	rcall	screen_cmd_vc_cmv
	rcall	screen_cmd_pc_cmv
	rcall	screen_cmd_psv
	rcall	screen_cmd_shutdown
	rcall	screen_cmd_buzzer
	rcall	screen_cmd_status_sensors
	rcall	screen_cmd_calibra_O2
	rcall	screen_cmd_autotest
	rcall	screen_cmd_ee_write
	rcall	screen_cmd_ee_read
	rjmp	screen_loop




screen_init:
	ldi		R16,250
	rcall	screen_delay_ms

	ldi		R16,0x40							; screeen power enable
	sts		PORTD_OUTSET,R16

	clr		R16
	sts		USART2_CTRLA,R16					; disable interrupts
	sts		screen_rx_ptrwr,R16					; clear buffer pointers
	sts		screen_rx_ptrrd,R16
	sts		screen_send_status,R16

	ldi		R16,0x01							; init tx(PF0)
	sts		PORTF_OUTSET,R16
	sts		PORTF_DIRSET,R16
	ldi		R16,0x00
	sts		PORTF_PIN0CTRL,R16
	ldi		R16,0x02							; init rx(PF1)
	sts		PORTF_OUTCLR,R16
	sts		PORTF_DIRCLR,R16
	ldi		R16,0x00
	sts		PORTF_PIN1CTRL,R16

	ldi		R16, low(64*fosc/(screen_BPS*16))	; set baudrate
	ldi		R17, high(64*fosc/(screen_BPS*16))
	sts		USART2_BAUDL,R16				
	sts		USART2_BAUDH,R17

	ldi		R16,0x03							; Async mode, parity disable, 
	sts		USART2_CTRLC,R16					; 1 stop bit, 8 bits

	ldi		R16,USART_RXCIE_bm					; rxcie interrupt
	sts		USART2_CTRLA,R16
	ldi		R16,0xc0							; Enable receiver and transmitter
	sts		USART2_CTRLB,R16					; Normal mode
	ret



screen_isr_rxc:
	push	R16
	in		R16,CPU_SREG
	push	R16
	push	XL
	push	XH

	ldi		XH,high(screen_rx_buffer)
	ldi		XL,low(screen_rx_buffer)
	lds		R16,screen_rx_ptrwr
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	lds		R16,USART2_RXDATAL
	st		X,R16
	lds		R16,screen_rx_ptrwr
	inc		R16
	cpi		R16,screen_RX_BUFFER_LEN
	brlo	screen_isr_rxc_ovf
	ldi		R16,0
screen_isr_rxc_ovf:
	sts		screen_rx_ptrwr,R16

	pop		XH
	pop		XL
	pop		R16
	out		CPU_SREG,R16
	pop		R16
	reti



screen_rx_byte_tout:
	ldi		R16,screen_rx_TIMEOUT_ms
	sts		screen_rx_tmr_ms,R16
screen_rx_byte_tout_loop:
	task_change
	lds		R16,screen_rx_ptrrd
	lds		R17,screen_rx_ptrwr
	cp		R16,R17
	brne	screen_rx_byte_tout_data
	lds		R16,screen_rx_tmr_ms
	cpi		R16,0
	brne	screen_rx_byte_tout_loop
	clz
	rjmp	screen_rx_byte_tout_ret
screen_rx_byte_tout_data:
	ldi		XH,high(screen_rx_buffer)
	ldi		XL,low(screen_rx_buffer)
	add		XL,R16
	ldi		R16,0
	adc		XH,R16
	ld		R16,X
	lds		R17,screen_rx_ptrrd
	inc		R17
	cpi		R17,screen_RX_BUFFER_LEN
	brlo	screen_rx_byte_tout_ovf
	ldi		R17,0
screen_rx_byte_tout_ovf:
	sts		screen_rx_ptrrd,R17
	sez
screen_rx_byte_tout_ret:
	ret

screen_tx_byte:
	push	R16
screen_tx_byte_Lazo:
	lds		R16,USART2_STATUS
	sbrc	R16,USART_DREIF_bp
	rjmp	screen_tx_byte_tx
	task_change
	rjmp	screen_tx_byte_Lazo
screen_tx_byte_tx:
	pop		R16
	sts		USART2_TXDATAL,R16
	ret


screen_wait_txc:
	ldi		R16,USART_TXCIF_bm				; clear txc flag
	sts		USART2_STATUS,R16
screen_wait_txc_loop:
	task_change
	lds		R16,USART2_STATUS
	sbrs	R16,USART_TXCIF_bp
	rjmp	screen_wait_txc_loop
	ret




screen_tx_header:
	ldi		R16,$5a
	rcall	screen_tx_byte
	ldi		R16,$a5
	rcall	screen_tx_byte
	ret


screen_tx_float:
	ld		R16,X+
	ld		R17,X+
	ld		R18,X+
	ld		R19,X+

	call	int16_16_to_float

	push	R19
	push	R18
	push	R17
	rcall	screen_tx_byte
	pop		R16
	rcall	screen_tx_byte
	pop		R16
	rcall	screen_tx_byte
	pop		R16
	rcall	screen_tx_byte
	ret



screen_check_data_tx:
	lds		R16,screen_tx_tmr_ms
	cpi		R16,0
	brne	screen_check_data_tx_ret
	ldi		R16,10
	sts		screen_tx_tmr_ms,R16

	rcall	screen_tx_data_PFV

screen_check_data_tx_ret:
	ret




screen_tx_data_PFV:
	ldi		R16,1
	sts		debug_print,R16

	call	chk_power_button

	rcall	sensors_calc_max

	rcall	screen_tx_header

	ldi		R16,1+12
	rcall	screen_tx_byte
	ldi		R16,$a1
	rcall	screen_tx_byte

	ldi		XH,high(sensors_pressure_value)
	ldi		XL,low(sensors_pressure_value)
	rcall	screen_tx_float

	ldi		XH,high(sensors_flow_value)
	ldi		XL,low(sensors_flow_value)
	rcall	screen_tx_float

	ldi		XH,high(sensors_volume_value)
	ldi		XL,low(sensors_volume_value)
	rcall	screen_tx_float


	lds		R16,screen_send_status
	cpi		R16,1
	brne	screen_tx_data_PFV_ret
	ldi		R16,0
	sts		screen_send_status,R16
	rcall	screen_tx_data_sensors
screen_tx_data_PFV_ret:
	ret



screen_tx_event:
	push	R16
	rcall	screen_tx_header

	ldi		R16,1+2
	rcall	screen_tx_byte
	ldi		R16,$a3
	rcall	screen_tx_byte

	pop		R16
	rcall	screen_tx_byte

	lds		R16,flow_apnea
	ldi		R17,1
	eor		R16,R17
	rcall	screen_tx_byte

	ret




screen_tx_data_sensors:
	rcall	screen_tx_header

	ldi		R16,1+14
	rcall	screen_tx_byte
	ldi		R16,$a4
	rcall	screen_tx_byte

;	lds		R16,sensors_oxygen_value+2		; oxygen
	rcall	sensors_o2_cal
	clr		R0
	mov		R16,XL
	lsl		YH
	adc		R16,R0
	rcall	screen_tx_byte

	lds		R16,bat_adc_data+1				; Bat hi-lo
	rcall	screen_tx_byte
	lds		R16,bat_adc_data
	rcall	screen_tx_byte

	lds		R16,sensors_current_value+2		; current
	rcall	screen_tx_byte
	lds		R16,sensors_current_value+3
	rcall	screen_tx_byte

	lds		R17,PORTB_IN					; AC
	ldi		R16,0
	sbrs	R17,5
	ldi		R16,1
	rcall	screen_tx_byte

	lds		R16,stepper_pressure_alarm		; pressure alarm
	rcall	screen_tx_byte

	lds		R16,stepper_flow_alarm			; flow alarm
	rcall	screen_tx_byte

	lds		R16,stepper_pos_alarm			; position alarm
	rcall	screen_tx_byte

	lds		R16,flow_apnea					; apnea alarm
	rcall	screen_tx_byte

	lds		R16,stepper_sensors_alarm		; sensors alarm
	rcall	screen_tx_byte

	lds		R16,stepper_disconnect_alarm	; disconecction alarm
	rcall	screen_tx_byte

	lds		R16,stepper_obstruction_alarm	; obstruction alarm
	lds		R17,stepper_resistance_alarm
	or		R16,R17
	rcall	screen_tx_byte

	lds		R16,vent_mode					; ventilation mode
	rcall	screen_tx_byte

	ret




screen_tx_shut_down:
	rcall	screen_tx_header

	ldi		R16,1+1
	rcall	screen_tx_byte
	ldi		R16,$a5
	rcall	screen_tx_byte

	ldi		R16,0x55
	rcall	screen_tx_byte
	ret



screen_tx_ack_O2:
	push	R16
	rcall	screen_tx_header

	ldi		R16,1+1
	rcall	screen_tx_byte
	ldi		R16,$a6
	rcall	screen_tx_byte

	pop		R16
	rcall	screen_tx_byte

	ret


screen_tx_autotest:
	push	R16
	rcall	screen_tx_header

	ldi		R16,1+1
	rcall	screen_tx_byte
	ldi		R16,$a7
	rcall	screen_tx_byte

	pop		R16
	rcall	screen_tx_byte

	ret



.dseg
screen_tx_ee_read_cont:	.byte 1
screen_tx_ee_read_ptr:	.byte 2

.cseg
screen_tx_ee_read:
	sts		screen_tx_ee_read_cont,R16
	sts		screen_tx_ee_read_ptr,XL
	sts		screen_tx_ee_read_ptr+1,XH

	rcall	screen_tx_header
	lds		R16,screen_tx_ee_read_cont
	inc		R16
	rcall	screen_tx_byte
	ldi		R16,$a8
	rcall	screen_tx_byte

screen_tx_ee_read_loop:
	lds		XL,screen_tx_ee_read_ptr
	lds		XH,screen_tx_ee_read_ptr+1
	ld		R16,X+
	sts		screen_tx_ee_read_ptr,XL
	sts		screen_tx_ee_read_ptr+1,XH
	rcall	screen_tx_byte
	lds		R16,screen_tx_ee_read_cont
	dec		R16
	sts		screen_tx_ee_read_cont,R16
	brne	screen_tx_ee_read_loop
	ret






screen_rx_frame:
	rcall	screen_rx_byte_tout
	brne	screen_rx_frame
screen_rx_frame_header:
	cpi		R16,0x5a					; frame header
	brne	screen_rx_frame

	rcall	screen_rx_byte_tout
	brne	screen_rx_frame
	cpi		R16,0xa5
	brne	screen_rx_frame_header

	rcall	screen_rx_byte_tout			; frame len
	brne	screen_rx_frame
	cpi		R16,screen_RX_BUFFER_LEN
	brsh	screen_rx_frame
	sts		screen_cmd_buffer,R16
	mov		R17,R16
	ldi		XH,high(screen_cmd_buffer+1)
	ldi		XL,low(screen_cmd_buffer+1)
screen_rx_frame_data:
	push	R17
	push	XL
	push	XH
	rcall	screen_rx_byte_tout
	pop		XH
	pop		XL
	pop		R17
	brne	screen_rx_frame
	st		X+,R16
	dec		R17
	brne	screen_rx_frame_data
	ret



screen_cmd_stop:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb1
	brne	screen_cmd_stop_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+1
	brne	screen_cmd_stop_ret

	ldi		R16,0
	sts		vent_mode_new,R16
screen_cmd_stop_ret:
	ret

screen_cmd_vc_cmv:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb2
	brne	screen_cmd_vc_cmv_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+14
	brne	screen_cmd_vc_cmv_ret

	rcall	screen_cmd_cpy_vent_data
	ldi		R16,1
	sts		vent_mode_new,R16
screen_cmd_vc_cmv_ret:
	ret

screen_cmd_pc_cmv:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb3
	brne	screen_cmd_pc_cmv_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+10
	brne	screen_cmd_pc_cmv_ret

	rcall	screen_cmd_cpy_vent_data
	ldi		R16,2
	sts		vent_mode_new,R16
screen_cmd_pc_cmv_ret:
	ret

screen_cmd_psv:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb4
	brne	screen_cmd_psv_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+10
	brne	screen_cmd_psv_ret

	rcall	screen_cmd_cpy_vent_data
	ldi		R16,3
	sts		vent_mode_new,R16
screen_cmd_psv_ret:
	ret


screen_cmd_cpy_vent_data:
	ldi		YH,high(screen_cmd_buffer+2)
	ldi		YL,low(screen_cmd_buffer+2)
	ldi		XH,high(vent_mode_data)
	ldi		XL,low(vent_mode_data)
	ldi		R16,screen_RX_BUFFER_LEN
	call	cpynYaX
	ret



screen_cmd_shutdown:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb5
	brne	screen_cmd_shutdown_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+1
	brne	screen_cmd_shutdown_ret

	lds		R16,screen_cmd_buffer+2
	cpi		R16,0x55
	brne	screen_cmd_shutdown_ret

	ldi		R16,1
	sts		power_shut_down,R16
screen_cmd_shutdown_ret:
	ret



screen_cmd_buzzer:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb6
	brne	screen_cmd_buzzer_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+1
	brne	screen_cmd_buzzer_ret

	lds		R16,screen_cmd_buffer+2
	cpi		R16,0x00
	brne	screen_cmd_buzzer_no0
	ldi		R16,0						; buzzer off
	sts		buzzer_flag,R16
	rjmp	screen_cmd_buzzer_ret
screen_cmd_buzzer_no0:

	cpi		R16,0x01
	brne	screen_cmd_buzzer_no1
	ldi		R16,1						; buzzer on
	sts		buzzer_flag,R16
	rjmp	screen_cmd_buzzer_ret
screen_cmd_buzzer_no1:

screen_cmd_buzzer_ret:
	ret



screen_cmd_status_sensors:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb7
	brne	screen_cmd_status_sensors_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+0
	brne	screen_cmd_status_sensors_ret

	ldi		R16,1
	sts		screen_send_status,R16
screen_cmd_status_sensors_ret:
	ret


	
screen_cmd_calibra_O2:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb8
	breq	screen_cmd_calibra_O2_cont
screen_cmd_calibra_O2_ret1:
	rjmp	screen_cmd_calibra_O2_ret
screen_cmd_calibra_O2_cont:
	lds		R16,screen_cmd_buffer
	cpi		R16,1+1
	brne	screen_cmd_calibra_O2_ret1
	lds		R16,screen_cmd_buffer+2
	cpi		R16,21
	brne	screen_cmd_calibra_O2_cal21
	lds		R3,sensors_oxygen_value+3
	lds		R2,sensors_oxygen_value+2
	lds		R1,sensors_oxygen_value+1
	lds		R0,sensors_oxygen_value+0
	sts		sensors_o2_21_cal+3,R3
	sts		sensors_o2_21_cal+2,R2
	sts		sensors_o2_21_cal+1,R1
	sts		sensors_o2_21_cal+0,R0
	ldi		XH,high(EEPROM_START+ee_o2_21_cal)
	ldi		XL,low(EEPROM_START+ee_o2_21_cal)
	lds		R16,sensors_o2_21_cal+0
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_21_cal+1
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_21_cal+2
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_21_cal+3
	rcall	eeprom_write_byte
	ldi		R16,1
	sts		calibrate_o2_flag,R16
	rjmp	screen_cmd_calibra_O2_ret
screen_cmd_calibra_O2_cal21:
	cpi		R16,60
	brlo	screen_cmd_calibra_O2_ret
	cpi		R16,100+1
	brsh	screen_cmd_calibra_O2_ret
	sts		sensors_o2_Hi_ref,R16
	lds		R3,sensors_oxygen_value+3
	lds		R2,sensors_oxygen_value+2
	lds		R1,sensors_oxygen_value+1
	lds		R0,sensors_oxygen_value+0
	sts		sensors_o2_Hi_cal+3,R3
	sts		sensors_o2_Hi_cal+2,R2
	sts		sensors_o2_Hi_cal+1,R1
	sts		sensors_o2_Hi_cal+0,R0
	ldi		XH,high(EEPROM_START+ee_o2_hi_cal)
	ldi		XL,low(EEPROM_START+ee_o2_hi_cal)
	lds		R16,sensors_o2_Hi_cal+0
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_Hi_cal+1
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_Hi_cal+2
	rcall	eeprom_write_byte
	lds		R16,sensors_o2_Hi_cal+3
	rcall	eeprom_write_byte

	ldi		XH,high(EEPROM_START+ee_o2_hi_ref)
	ldi		XL,low(EEPROM_START+ee_o2_hi_ref)
	lds		R16,sensors_o2_Hi_ref
	rcall	eeprom_write_byte
	ldi		R16,1
	sts		calibrate_o2_flag,R16

screen_cmd_calibra_O2_ret:
	ret


	
screen_cmd_autotest:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xb9
	brne	screen_cmd_autotest_ret
	lds		R16,screen_cmd_buffer
	cpi		R16,1+0
	brne	screen_cmd_autotest_ret

	ldi		R16,1
	sts		autotest_flag,R16
screen_cmd_autotest_ret:
	ret



screen_cmd_ee_write:
	lds		R16,ee_write_tmr_s				; ee write only first 20 seconds at power on
	cpi		R16,0
	breq	screen_cmd_ee_write_ret

	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xba
	brne	screen_cmd_ee_write_ret

	lds		R16,screen_cmd_buffer+2			; address H
	cpi		R16,0x14
	brne	screen_cmd_ee_write_ret

	lds		R16,screen_cmd_buffer+4			; data length
	cpi		R16,0
	breq	screen_cmd_ee_write_ret
	lds		R17,screen_cmd_buffer			; cmd length
	subi	R17,(1+3)
	cp		R16,R17
	brne	screen_cmd_ee_write_ret

	lds		XH,screen_cmd_buffer+2			; address H-L
	lds		XL,screen_cmd_buffer+3
	ldi		YH,high(screen_cmd_buffer+5)
	ldi		YL,low(screen_cmd_buffer+5)
	lds		R16,screen_cmd_buffer+4			; data length
	rcall	screen_tx_ee_write

screen_cmd_ee_write_ret:
	ret


screen_tx_ee_write:
	ld		R17,Y+
	st		X+,R17

	mov		R17,XL
	andi	R17,$3f
	brne	screen_tx_ee_write_next
	rcall	screen_tx_ee_write_page
	dec		R16
	breq	screen_tx_ee_write_ret
	rjmp	screen_tx_ee_write
screen_tx_ee_write_next:

	dec		R16
	brne	screen_tx_ee_write
	rcall	screen_tx_ee_write_page

screen_tx_ee_write_ret:
	ret


.dseg
screen_tx_ee_write_page_buff:	.byte 5
.cseg
screen_tx_ee_write_page:
	sts		screen_tx_ee_write_page_buff,R16
	sts		screen_tx_ee_write_page_buff+1,XL
	sts		screen_tx_ee_write_page_buff+2,XH
	sts		screen_tx_ee_write_page_buff+3,YL
	sts		screen_tx_ee_write_page_buff+4,YH
	rcall	eeprom_write_page
	lds		YH,screen_tx_ee_write_page_buff+4
	lds		YL,screen_tx_ee_write_page_buff+3
	lds		XH,screen_tx_ee_write_page_buff+2
	lds		XL,screen_tx_ee_write_page_buff+1
	lds		R16,screen_tx_ee_write_page_buff
	ret

screen_cmd_ee_read:
	lds		R16,screen_cmd_buffer+1
	cpi		R16,0xbb
	brne	screen_cmd_ee_read_ret

	lds		R16,screen_cmd_buffer			; cmd length
	cpi		R16,1+3
	brne	screen_cmd_ee_read_ret

	lds		XH,screen_cmd_buffer+2			; address H
	cpi		XH,0x14
	brne	screen_cmd_ee_read_ret

	ldi		R16,1
	rcall	screen_delay_ms

	lds		XH,screen_cmd_buffer+2			; address H-L
	lds		XL,screen_cmd_buffer+3
	lds		R16,screen_cmd_buffer+4			; data length
	rcall	screen_tx_ee_read
screen_cmd_ee_read_ret:
	ret



screen_delay_ms:
	sts		screen_tx_tmr_ms,R16
screen_delay_ms_loop:
	task_change
	lds		R16,screen_tx_tmr_ms
	cpi		R16,0
	brne	screen_delay_ms_loop
	ret