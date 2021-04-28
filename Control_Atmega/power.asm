/*
 * power.asm
 *
 *  Created: 26/05/2020 01:15:01 a. m.
 *   Author: Javier
 */ 
 .equ	power_normal_off	= 10		; 1 second normal shut down
 .equ	power_force_off		= 150		; 15 seconds force shut down

.dseg
power_shut_down:		.byte 1
power_screen_flag:		.byte 1
power_tmr_ds:			.byte 1
power_buzzer_tmr_ms:	.byte 1
.cseg
power_task:
	ldi		R16,0
	sts		power_shut_down,R16
power_task_reset:
	ldi		R16,power_force_off
	sts		power_tmr_ds,R16
	ldi		R16,0
	sts		power_screen_flag,R16
power_task_loop:
	task_change
;	rjmp	power_task_loop
	lds		R16,power_shut_down						; screen shutdown?
	cpi		R16,1
	breq	power_task_shut_down

	lds		R16,PORTC_IN							; off switch
	andi	R16,0x04
	brne	power_task_reset

	lds		R16,power_tmr_ds						; force shutdown?
	cpi		R16,0
	breq	power_task_shut_down

	cpi		R16,power_force_off-power_normal_off	; normal shutdown?
	brsh	power_task_loop

	lds		R16,power_screen_flag
	cpi		R16,0
	brne	power_task_loop
	ldi		R16,1
	sts		power_screen_flag,R16
	rjmp	power_task_loop



power_task_shut_down:
	ldi		R16,1
	sts		power_shut_down,R16

 	ldi		R16,0x04							; motor disable
	sts		PORTB_OUTSET,R16
	ldi		R16,0x00
	sts		TCA0_SINGLE_CTRLA,R16				; stop TCA0
	sts		TCA0_SINGLE_CTRLD,R16				; no split
	sts		TCA0_SINGLE_CTRLB,R16
	sts		TCA0_SINGLE_INTFLAGS,R16			; tca_ovf interrupt flag clear
	sts		TCA0_SINGLE_INTCTRL,R16				; tca_ovf interrupt enable
 	ldi		R16,0x03							; motor dir,pulse
	sts		PORTB_OUTCLR,R16

	ldi		R16,0x40							; screeen power enable
	sts		PORTD_OUTCLR,R16

	ldi		R16,0x00							; power unlock
	sts		PORTC_OUT,R16
power_task_shut_down_wait:
	ldi		R16,1
	sts		buzzer_flag,R16
	task_change
	rjmp	power_task_shut_down_wait




buzzer_tgl:
	ldi		R17,100
buzzer_tgl_loop:
	push	R17
	ldi		R16,0x40					; toggle buzzer
	sts		PORTC_OUTTGL,R16
	ldi		R16,1
	rcall	power_delayms
	pop		R17
	dec		R17
	brne	buzzer_tgl_loop

	ldi		R16,100
	rcall	power_delayms
	ret



power_delayms:
	sts		power_buzzer_tmr_ms,R16
power_delayms_loop:
	task_change
	lds		R16,power_buzzer_tmr_ms
	cpi		R16,0
	brne	power_delayms_loop
	ret