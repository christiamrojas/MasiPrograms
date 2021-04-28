;
; debug.asm
;
; Created: 05/05/2020 20:25:42 p. m.
; Author : jchangfu
;

;	PA4: (HI-Z)		debug tx. usart 0
;   PA5: (HI-Z)     debug rx. usart 0
.equ	debug_BPS = 115200

.dseg
debug_print:	.byte 1


.cseg
debug_msg:
	.db "SP pressure flow vel_sp vel_cur pos t+ f+",13,10,0
;	.db "O2 3.3 5.0 mA",13,10,0,0

.macro debug_printf_3
	lds		R16,@0-1
	lds		XL,@0
	lds		XH,@0+1
	ldi		R17,@1
	rcall	debug_print_dec_mul
.endm

.macro debug_printf_2
	ldi		R16,0
	lds		XL,@0
	lds		XH,@0+1
	ldi		R17,@1
	rcall	debug_print_dec_mul
.endm

.macro debug_printf_1
	ldi		R16,0
	lds		XL,@0
	ldi		XH,0
	sbrc	XL,7
	ldi		XH,0xff
	ldi		R17,@1
	rcall	debug_print_dec_mul
.endm

.dseg
test:	.byte 2


.cseg
debug_task:
	clr		r16
	sts		sensors_oxygen_Calibrado,r16
	sts		sensors_oxygen_Calibrado+1,r16
	sts		sensors_oxygen_Calibrado+2,r16
	sts		sensors_oxygen_Calibrado+3,r16
	rcall	debug_init
	ldi		ZH,high(debug_msg*2)
	ldi		ZL,low(debug_msg*2)
	rcall	debug_printf
debug_loop:
	task_change
	lds		R16,debug_print
	cpi		R16,1
	brne	debug_loop
	ldi		R16,0
	sts		debug_print,R16

	rcall	sensors_o2_cal
	clr		R0
	mov		R16,XL
	lsl		YH
	adc		R16,R0
	sts		sensors_oxygen_Calibrado+2,r16
;	debug_printf_2 sensors_oxygen_Calibrado+2,1
;	debug_printf_2 sensors_oxygen_value+2,1
;	debug_printf_2 sensors_oxygen_value+1,1
;	debug_printf_2 sensors_VDD33_value+2,1
;	debug_printf_2 sensors_VDD50_value+2,1
;	debug_printf_2 sensors_current_value+2,1
;	rcall	debug_tx_crlf
;	rjmp	debug_loop


;	debug_printf_1 pid_sp,100
;	debug_printf_1 vc_cmv_ctrl_curr_flow,1
;	debug_printf_1 vc_cmv_insp_time_extra,1
;	debug_printf_2 sensors_volume_max+2,1
;	debug_printf_2 sensors_pressure_max+2,1

;	debug_printf_3 sensors_pressure_value+2,100
	debug_printf_3 sensors_flow_value+2,100
	debug_printf_3 sensors_volume_value+2,10
;	debug_printf_2 pid_e+0,1
;	debug_printf_2 stepper_speed_sp,10
;	debug_printf_2 stepper_speed_cur,10
	debug_printf_2 stepper_pos_cur,1
;	debug_printf_3 flow_sum,10
;	debug_printf_1 flow_count,100
;	debug_printf_1 flow_trigger,100
;	debug_printf_1 vc_cmv_insp_time_extra,100
;	debug_printf_1 insp_flow_add,100
;	debug_printf_1 flow_flag,100

	rcall	debug_tx_crlf
	rjmp	debug_loop




debug_init:
	clr		R16
	sts		USART0_CTRLA,R16					; disable interrupts

	ldi		R16,0x10							; init tx(PA4)
	sts		PORTA_OUTSET,R16
	sts		PORTA_DIRSET,R16
	ldi		R16,0x00
	sts		PORTA_PIN4CTRL,R16
	ldi		R16,0x20							; init rx(PA5)
	sts		PORTA_OUTCLR,R16
	sts		PORTA_DIRCLR,R16
	ldi		R16,0x00
	sts		PORTA_PIN5CTRL,R16

	lds		R16, PORTMUX_USARTROUTEA			; routing usart 0 to alternate pins
	andi	R16, ~PORTMUX_USART0_gm
	ori		R16,PORTMUX_USART0_ALT1_gc
	sts		PORTMUX_USARTROUTEA,R16

	ldi		R16, low(64*fosc/(debug_BPS*16))	; set baudrate
	ldi		R17, high(64*fosc/(debug_BPS*16))
	sts		USART0_BAUDL,R16				
	sts		USART0_BAUDH,R17

	ldi		R16,0x03							; Async mode, parity disable, 
	sts		USART0_CTRLC,R16					; 1 stop bit, 8 bits

	ldi		R16,0xc0							; Enable receiver and transmitter
	sts		USART0_CTRLB,R16					; Normal mode
	ret



debug_tx_byte:
	push	R16
debug_tx_byte_Lazo:
	lds		R16,USART0_STATUS
	sbrc	R16,USART_DREIF_bp
	rjmp	debug_tx_byte_tx
	task_change
	rjmp	debug_tx_byte_Lazo
debug_tx_byte_tx:
	pop		R16
	sts		USART0_TXDATAL,R16
	ret


debug_tx_crlf:
	ldi		R16, 13
	rcall	debug_tx_byte
	ldi		R16, 10
	rcall	debug_tx_byte
	ret



debug_printf:
	lpm		R16,Z+
	cpi		R16,0
	breq	debug_printf_ret
	push	ZL
	push	ZH
	rcall	debug_tx_byte
	pop		ZH
	pop		ZL
	rjmp	debug_printf
debug_printf_ret:
	ret


debug_tx_int16:
	sbrs	XH,7					; checks for sign
	rjmp	debug_tx_int16_pos
	push	XL						; negative number
	push	XH
	ldi		R16,'-'
	rcall	debug_tx_byte
	pop		XH
	pop		XL
	com		XH						; neg int16
	neg		XL
	sbci	XH,255 

;	ldi		R16,'9'					; limita negativos a "-9"
;	rcall	debug_tx_byte
;	rjmp	debug_tx_int16_ret
debug_tx_int16_pos:

	rcall	Bin16aDec
	push	R20
	push	R19
	push	R18
	push	R17
	cpi		R16,'0'
	brne	debug_tx_int16_dig5
	pop		R16
	cpi		R16,'0'
	brne	debug_tx_int16_dig4
	pop		R16
	cpi		R16,'0'
	brne	debug_tx_int16_dig3
	pop		R16
	cpi		R16,'0'
	brne	debug_tx_int16_dig2
	pop		R16	
	rjmp	debug_tx_int16_dig1
debug_tx_int16_dig5:
	rcall	debug_tx_byte
	pop		R16
debug_tx_int16_dig4:
	rcall	debug_tx_byte
	pop		R16
debug_tx_int16_dig3:
	rcall	debug_tx_byte
	pop		R16
debug_tx_int16_dig2:
	rcall	debug_tx_byte
	pop		R16
debug_tx_int16_dig1:
	rcall	debug_tx_byte
debug_tx_int16_ret:
	ret




; show XH:XL:R16 _xR17
debug_print_dec_mul:
	mul		R16,R17
	mov		R10,R0
	mov		R11,R1
	mul		XH,R17
	mov		R12,R0
	mul		XL,R17
	add		R11,R0
	adc		R12,R1

	mov		XH,R12
	mov		XL,R11

	sbrc	R10,7
	adiw	XL,1

	rcall	debug_tx_int16
	ldi		R16,' '
	rcall	debug_tx_byte
debug_pressure_ret:
	ret
