/*
 * buzzer.asm
 *
 *  Created: 05/06/2020 03:48:13 a. m.
 *   Author: Javier
 */ 

.dseg
buzzer_flag:	.byte 1
buzzer_tmr_cs:	.byte 1

.cseg
buzzer_task:
	ldi		R16,0
	sts		buzzer_flag,R16
	ldi		R16,0x40					; buzzer off
	sts		PORTC_OUTCLR,R16
buzzer_task_loop:
	task_change
	lds		R16,buzzer_flag
	cpi		R16,0
	breq	buzzer_task_loop

	rcall	buzzer_pulse
	rjmp	buzzer_task_loop


buzzer_delay_cs:
	sts		buzzer_tmr_cs,R16
buzzer_delay_cs_loop:
	task_change
	lds		R16,buzzer_tmr_cs
	cpi		R16,0
	brne	buzzer_delay_cs_loop
	ret


buzzer_pulse:
	ldi		R16,0x40					; buzzer on
	sts		PORTC_OUTSET,R16
	ldi		R16,10
	rcall	buzzer_delay_cs
	ldi		R16,0x40					; buzzer off
	sts		PORTC_OUTCLR,R16
	ldi		R16,10
	rcall	buzzer_delay_cs
	ret


buzzer:
	task_change
	ldi		R16,1
	sts		buzzer_flag,R16
	task_change
	ldi		R16,0
	sts		buzzer_flag,R16
	ret