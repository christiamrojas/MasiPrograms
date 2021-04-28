;
; multitask.asm
;
; Created: 05/05/2020 04:38:50 p. m.
; Author : jchangfu
;

 .equ	multitask_TASK_STACK_SIZE = 128

.cseg
multitask_task_table:
	.dw		main_task
	.dw		debug_task
	.dw		timer_task
	.dw		sensors_adc_task
	.dw		sensors_i2c_task
	.dw		sensors_serial_task
	.dw		screen_task
	.dw		power_task
	.dw		buzzer_task
multitask_task_table_end:

.equ	multitask_TASKS_COUNT = (multitask_task_table_end-multitask_task_table)

.dseg
multitask_curr_task:		.byte 1
multitask_sp_table:			.byte 2*multitask_TASKS_COUNT							; LO-HI
multitask_stack_pool:		.byte multitask_TASK_STACK_SIZE*multitask_TASKS_COUNT
multitask_stack_pool_end:

.cseg
multitask_init:
	cli												;disable global interrupts
	ldi		R16,high(RAMEND)						;initialize stack
	out		CPU_SPH,R16
	ldi		R16,low(RAMEND)
	out		CPU_SPL,R16
	
	rcall	multitask_ports_init					;initialize ports
	rcall	multitask_osc_init						;change to external oscillator

	ldi		ZH,high(multitask_task_table*2)
	ldi		ZL,low(multitask_task_table*2)
	ldi		XH,high(multitask_sp_table)
	ldi		XL,low(multitask_sp_table)
	ldi		YH,high(multitask_stack_pool+multitask_TASK_STACK_SIZE)
	ldi		YL,low(multitask_stack_pool+multitask_TASK_STACK_SIZE)
	ldi		R18,multitask_TASKS_COUNT
multitask_init_loop:
	lpm		R16,Z+									; task start address
	lpm		R17,Z+
	st		-Y,R16									; push task address to stack
	st		-Y,R17
	sbiw	YL,1
	st		X+,YL									; store task sp to table
	st		X+,YH
	ldi		R16,low(multitask_TASK_STACK_SIZE+3)	; next stack
	add		YL,R16
	ldi		R16,high(multitask_TASK_STACK_SIZE+3)
	adc		YH,R16
	dec		R18										; next task
	brne	multitask_init_loop

	ldi		R16,0									; start on first task
	sts		multitask_curr_task,R16
	lds		R16,multitask_sp_table					; SPL
	lds		R17,multitask_sp_table+1				; SPH
	out		CPU_SPH,R17
	out		CPU_SPL,R16
		
	sei												; enable global interrupts
	ret




multitask_ports_init:
;	PA0: (HI-Z)		external oscillator
;	PA1: (OUT-LOW)	flow sensor power enable
;	PA2: (PULL-UP)	flow sensor SDA
;   PA3: (PULL-UP)	flow sensor SCL
;	PA4: (HI-Z)		debug tx. usart 0
;   PA5: (HI-Z)     debug rx. usart 0
;	PA6: (PULL-UP)	nc
;   PA7: (PULL-UP)	nc
	ldi		R16,0x00
	sts		PORTA_OUT,R16
	ldi		R16,0x02
	sts		PORTA_DIR,R16
	ldi		R16,0x00
	sts		PORTA_PIN0CTRL,R16
	sts		PORTA_PIN1CTRL,R16
	sts		PORTA_PIN4CTRL,R16
	sts		PORTA_PIN5CTRL,R16
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTA_PIN2CTRL,R16
	sts		PORTA_PIN3CTRL,R16
	sts		PORTA_PIN4CTRL,R16
	sts		PORTA_PIN7CTRL,R16

;	PB0: (OUT-LOW)	stepper pulse
;	PB1: (OUT-LOW)	stepper dir
;	PB2: (OUT-HIGH)	stepper enable
;   PB3: (OUT-LOW)	flow sensor level translator enable
;	PB4: (PULL-UP)	nc
;   PB5: (HI-Z)     Vac detect (inverse logic)
	ldi		R16,0x04
	sts		PORTB_OUT,R16
	ldi		R16,0x0f
	sts		PORTB_DIR,R16
	ldi		R16,0x00
	sts		PORTB_PIN0CTRL,R16
	sts		PORTB_PIN1CTRL,R16
	sts		PORTB_PIN2CTRL,R16
	sts		PORTB_PIN3CTRL,R16
	sts		PORTB_PIN5CTRL,R16
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTB_PIN4CTRL,R16

;	PC0: (OUT-LOW)	battery meassure enable
;	PC1: (HI-Z)		battery meassure rx. usart 1
;	PC2: (HI-Z)		on/off switch
;	PC3: (OUT-HIGH)	self locking power
;	PC4: (PULL-UP)	nc
;	PC5: (PULL-UP)	nc
;	PC6: (OUT-LOW)	buzzer
;	PC7: (PULL-UP)	nc
	ldi		R16,0x08
	sts		PORTC_OUT,R16
	ldi		R16,0x49
	sts		PORTC_DIR,R16
	ldi		R16,0x00
	sts		PORTC_PIN0CTRL,R16
	sts		PORTC_PIN1CTRL,R16
	sts		PORTC_PIN2CTRL,R16
	sts		PORTC_PIN3CTRL,R16
	sts		PORTC_PIN6CTRL,R16
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTC_PIN4CTRL,R16
	sts		PORTC_PIN5CTRL,R16
	sts		PORTC_PIN7CTRL,R16

;	PD0: (HI-Z)		pressure sensor ADC
;	PD1: (HI-Z)		oxigen sensor ADC
;	PD2: (HI-Z)		flow sensor VDD(3.3v/2) ADC
;	PD3: (HI-Z)		pressure/oxygen sensor VDD(5v/2) ADC
;	PD4: (OUT-LOW)	pressure/oxygen power enable
;	PD5: (HI-Z)		global current sensor ADC
;	PD6: (OUT-LOW)	screen power enable
;	PD7: (HI-Z)		AREF
	ldi		R16,0x00
	sts		PORTD_OUT,R16
	ldi		R16,0x50
	sts		PORTD_DIR,R16
	ldi		R16,0x00
	sts		PORTD_PIN0CTRL,R16
	sts		PORTD_PIN1CTRL,R16
	sts		PORTD_PIN2CTRL,R16
	sts		PORTD_PIN3CTRL,R16
	sts		PORTD_PIN4CTRL,R16
	sts		PORTD_PIN5CTRL,R16
	sts		PORTD_PIN6CTRL,R16
	sts		PORTD_PIN7CTRL,R16

;	PE0: (HI-Z)		Limit SW1
;	PE1: (HI-Z)		Limit SW2
;	PE2: (HI-Z)		Limit SW3
;	PE3: (HI-Z)		Limit SW4
	ldi		R16,0x00
	sts		PORTE_OUT,R16
	ldi		R16,0x00
	sts		PORTE_DIR,R16
;	ldi		R16,0x00
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTE_PIN0CTRL,R16
	sts		PORTE_PIN1CTRL,R16
	sts		PORTE_PIN2CTRL,R16
	sts		PORTE_PIN3CTRL,R16


;	PF0: (HI-Z)		screen tx. usart 2
;   PF1: (HI-Z)     screen rx. usart 2
;	PF2: (OUT-LOW)	debug led
;	PF3: (HI-Z)		wdi
;	PF4: (PULL-UP)	nc
;	PF5: (PULL-UP)	nc
	ldi		R16,0x00
	sts		PORTF_OUT,R16
	ldi		R16,0x04
	sts		PORTF_DIR,R16
	ldi		R16,0x00
	sts		PORTF_PIN0CTRL,R16
	sts		PORTF_PIN1CTRL,R16
	sts		PORTF_PIN2CTRL,R16
	sts		PORTF_PIN3CTRL,R16
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTF_PIN4CTRL,R16
	sts		PORTF_PIN5CTRL,R16

	ret




multitask_osc_init:	
	ldi		R17,CPU_CCP_IOREG_gc					; assign key IOREG

	ldi		R16,0x00								; prescaler disable	
	out		CPU_CCP,R17
	sts		CLKCTRL_MCLKCTRLB,R16	

	ldi		R16,CLKCTRL_CLKSEL_EXTCLK_gc			; select external clock
	out		CPU_CCP,R17
	sts		CLKCTRL_MCLKCTRLA,R16
	
multitask_osc_init_loop_wait_change:				; wait for system oscillator changing to finish
	lds		R16,CLKCTRL_MCLKSTATUS
	sbrc	R16,CLKCTRL_SOSC_bp
	rjmp	multitask_osc_init_loop_wait_change

	ret


.macro task_change
	call	multitask_task_change
.endm

multitask_task_change:
	ldi		XH,high(multitask_sp_table)				; save sp in table
	ldi		XL,low(multitask_sp_table)
	lds		R16,multitask_curr_task
	lsl		R16
	add		XL,R16
	clr		R16
	adc		XH,R16
	in		R16,CPU_SPL
	st		X+,R16
	in		R16,CPU_SPH
	st		X+,R16

	lds		R16,multitask_curr_task					; next task
	inc		R16
	cpi		R16,multitask_TASKS_COUNT
	brlo	multitask_task_change_no_first
	clr		R16										; roll to first task
	ldi		XH,high(multitask_sp_table)
	ldi		XL,low(multitask_sp_table)
multitask_task_change_no_first:
	sts		multitask_curr_task,R16

	ld		R16,X+									; load next sp
	ld		R17,X+
	cli
	out		CPU_SPH,R17
	out		CPU_SPL,R16
	sei
	ret
