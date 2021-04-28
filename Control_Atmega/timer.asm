; timer.asm
;
; Created: 05/05/2020 06:05:11 p. m.
; Author : jchangfu
;

.equ fosc = 18432000

.dseg					
timer_flags:	.byte 1	
timer_cs:		.byte 1 
timer_ds:		.byte 1 
timer_s:		.byte 1


.macro dec8
	lds		R16,@0
	subi	R16,1
	brcs	dec8_Zero
	sts		@0,R16
dec8_Zero:
.endm

.macro dec16
	lds		XL,@0
	lds		XH,@0+1
	sbiw	XL,1
	brcs	dec16_Zero
	sts		@0,XL
	sts		@0+1,XH
dec16_Zero:
.endm





.cseg
timer_task:
	rcall	timer_init
timer_task_Loop:
	task_change
	lds		R18,timer_flags
	sbrs	R18,0
	rjmp	timer_task_Loop

	cbr		R18,0x01
// update ms soft timers
	dec8	sensors_pressure_tmr_ms
	dec8	sensors_flow_tmr_ms

	dec8	screen_tx_tmr_ms
	dec8	screen_rx_tmr_ms
	dec8	pid_tmr_ms
	dec8	power_buzzer_tmr_ms
	dec8	i2c_master_wait_tmr_ms
	dec8	bat_adc_rx_tmr_ms

	sbrs	R18,1
	rjmp	timer_task_update
	cbr		R18,0x02
// update cs soft timers
	dec8	sensors_current_tmr_cs
	dec8	main_tmr_cs
	dec8	buzzer_tmr_cs
	dec8	bat_adc_tmr_cs	
	
	sbrs	R18,2
	rjmp	timer_task_update
	cbr		R18,0x04
// update ds soft timers
	dec8	sensors_oxygen_tmr_ds
	dec8	sensors_VDD33_tmr_ds
	dec8	sensors_VDD50_tmr_ds
	dec8	power_tmr_ds
	

	sbrs	R18,3
	rjmp	timer_task_update
	cbr		R18,0x08
// update seg soft timers
	dec8	ee_write_tmr_s
	

timer_task_update:
	sts		Timer_Flags,R18
	rjmp	timer_task_Loop



timer_init:
	ldi		R16,0x00
	sts		TCB0_CTRLA,R16				; turn off timer
	sts		TCB0_CTRLB,R16				; periodic interrupt mode
	sts		TCB0_EVCTRL,R16				; no capture

	ldi		R16,low(fosc/1000)			; period: 1 ms
	ldi		R17,high(fosc/1000)		
	sts		TCB0_CCMPL,R16
	sts		TCB0_CCMPH,R17

	ldi		R16,TCB_CAPT_bm				; enable capture interruptions
	sts		TCB0_INTCTRL,R16

	ldi		R16,0						; initialize variables
	sts		timer_flags,R16
	ldi		R16,10
	sts		timer_cs,R16
	sts		timer_ds,R16
	sts		timer_s,R16

	ldi		R16,TCB_ENABLE_bm			; turn on timer
	sts		TCB0_CTRLA,R16
	ret



isr_TCB0_capture:
	push	R16							; save registers and flags
	in		R16,CPU_SREG
	push	R16
	push	R17
	push	R18

	ldi		R16,1
	sts		TCB0_INTFLAGS,R16			; clear flag

	lds		R17,Timer_Flags				; ms flag
	ori		R17,0x01

	lds		R16,timer_cs				; cs flag
	dec		R16
	sts		timer_cs,R16
	brne	isr_TCB0_capture_update
	ldi		R18,10
	sts		Timer_cs,R18
	ori		R17,0x02

	lds		R16,Timer_ds				; ds flag
	dec		R16
	sts		Timer_ds,R16
	brne	isr_TCB0_capture_update
	sts		Timer_ds,R18
	ori		R17,0x04

	lds		R16,Timer_s					; seg flag
	dec		R16
	sts		Timer_s,R16
	brne	isr_TCB0_capture_update
	sts		Timer_s,R18
	ori		R17,0x08

isr_TCB0_capture_update:
	sts		Timer_Flags,R17
	pop		R18
	pop		R17
	pop		R16
	out		CPU_SREG,R16
	pop		R16
	reti


