; timer.asm
;
; Created: 05/05/2020 06:21:17 p. m.
; Author : jchangfu
;


 .cseg
adc_init:	
	ldi		R16,0x00
	sts		ADC0_CTRLA,R16				; stop adc
	sts		ADC0_CTRLD,R16				; no delay
	sts		ADC0_CTRLE,R16				; no compare
	sts		ADC0_SAMPCTRL,R16			; sample length

	ldi		R16,ADC_SAMPNUM_ACC32_gc	; Sample accumulation 32
	sts		ADC0_CTRLB,R16
	ldi		R16,ADC_SAMPCAP_bm|ADC_REFSEL_VREFA_gc|ADC_PRESC_DIV32_gc
	sts		ADC0_CTRLC,R16
	ldi		R16,ADC_ENABLE_bm			; enable adc	
	sts		ADC0_CTRLA,R16
	ret


; input:	R16 -> adc channel. example: ldi R16,ADC_MUXPOS_AIN0_gc
; output:	X   -> value, 16 bits
adc_read:
	sts		ADC0_MUXPOS,R16
	ldi		R16,ADC_STCONV_bm			; start conversion
	sts		ADC0_COMMAND,R16
adc_read_loop:
	task_change
	lds		R16,ADC0_COMMAND
	sbrc	r16,0
	rjmp	adc_read_loop
	lds		XL,ADC0_RESL
	lds		XH,ADC0_RESH
	ret

