;
;
; Created: 12/06/2020 05:17:05 p. m.
; Author : Javier
;

.equ	fosc		= 3686400
.equ	bps			= 9600
; MASI_V3_BAT.asm

.equ	adc_factor	= 1182

.cseg
.org	0x00
; Replace with your application code
start:
	ldi		R16,high(RAMEND)
	out		SPH,R16
	ldi		R16,low(RAMEND)
	out		SPL,R16

	rcall	ports_init
	rcall	adc_init
	rcall	serial_init
loop:
	rcall	adc_read
	rcall	adc2volt
	rcall	serial_write
	rjmp	loop




ports_init:
	ldi		R16,0xff
	out		PORTB,R16
	ldi		R16,0x00
	out		DDRB,R16

	ldi		R16,0x3d			; hi-z: adc_bat(1)
	out		PORTC,R16
	ldi		R16,0x00
	out		DDRC,R16

	ldi		R16,0xff			; out high: txd(1)
	out		PORTD,R16
	ldi		R16,0x02
	out		DDRD,R16
	ret


adc_init:
	ldi		R16,0x41			; avcc, adc1
	sts		ADMUX,R16
	ldi		R16,0x85			; aden, pre=32 (fadc=115200)
	sts		ADCSRA,R16
	ret

; x -> adc 14 bits
adc_read:
	clr		R0
	clr		R20
	clr		R21
	clr		R22
	ldi		YH,high(4096)
	ldi		YL,low(4096)
adc_read_loop:
	ldi		R16,0xc5			; aden,adsc, pre=32 (fadc=115200)
	sts		ADCSRA,R16
adc_read_loop1:
	lds		R16,ADCSRA
	sbrc	R16,6
	rjmp	adc_read_loop1
	lds		XL,ADCL
	lds		XH,ADCH
	add		R20,XL
	adc		R21,XH
	adc		R22,R0
	sbiw	YL,1
	brne	adc_read_loop
	mov		XL,R21
	mov		XH,R22
	ret


serial_init:
	ldi		R16,0x00				; U2X=0
	sts		UCSR0A,R16
	ldi		R16,0x00
	sts		UCSR0D,R16
	ldi		R16,0x06				; 8,n,1
	sts		UCSR0C,R16
	ldi		R16,high(fosc/16/bps-1)
	sts		UBRR0H,R16
	ldi		R16,low(fosc/16/bps-1)
	sts		UBRR0L,R16
	ldi		R16,0x08				; TXEN=1
	sts		UCSR0B,R16
	ret



serial_txbyte:
	lds		R17,UCSR0A
	sbrs	R17,5					; UDRE=1
	rjmp	serial_txbyte
	sts		UDR0,R16
	ret



serial_write:
	ldi		R16,0x5a				; header
	rcall	serial_txbyte
	ldi		R16,0xa5
	rcall	serial_txbyte

	ldi		R16,0x3					; len
	rcall	serial_txbyte

	ldi		R16,0x11				; cmd
	rcall	serial_txbyte

	mov		R16,XL					; dat low
	rcall	serial_txbyte
	mov		R16,XH					; dat high
	rcall	serial_txbyte
	ret


adc2volt:
	clr		R16

	ldi		YH,high(adc_factor)
	ldi		YL,low(adc_factor)

	mul		XL,YL
	mov		R20,R0
	mov		R21,R1

	mul		XH,YH
	mov		R22,R0
	mov		R23,R1

	mul		XL,YH
	add		R21,R0
	adc		R22,R1
	adc		R23,R16

	mul		XH,YL
	add		R21,R0
	adc		R22,R1
	adc		R23,R16

	mov		XL,R22
	mov		XH,R23
	ret