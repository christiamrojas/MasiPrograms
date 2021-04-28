/*
 * trigger.asm
 *
 *  Created: 19/05/2020 12:24:44 a. m.
 *   Author: Javier
 */ 


.equ trigger_base_count = 8			; 2,4,8,16,32,64	promedio de valores
.equ trigger_width		= 2			; 10 cs				tiempo del pulso trigger respecto a la base
.equ trigger_pos_width	= 4			; 4 cs				tiempo del pulso trigger para flujo positivos
.equ trigger_neg_width	= 10		; 10 cs				tiempo de flujo negativo antes de evaluar flujo positivo

.dseg
flow_buff:		.byte 64
flow_count:		.byte 1
flow_base:		.byte 2
flow_sum:		.byte 3
flow_idx:		.byte 1
flow_flag:		.byte 1
flow_trigger:	.byte 1
flow_apnea:		.byte 1
flow_pos_count:	.byte 1
flow_neg_count:	.byte 1
flow_end:		.byte 0

.cseg
process_trigger_init:
	ldi		XH,high(flow_buff)
	ldi		XL,low(flow_buff)
	ldi		R16,0
	ldi		R17,flow_end-flow_buff
process_trigger_init_loop:
	st		X+,R16
	dec		R17
	brne	process_trigger_init_loop
	sts		flow_flag,R16
	ret


; output:	->	sez - trigger detected
;				clz - trigger not detected
process_trigger:
	ldi		YH,high(flow_buff)		; point to flow_buffer+idx
	ldi		YL,low(flow_buff)
	lds		R16,flow_idx
	lsl		R16
	add		YL,R16
	ldi		R16,0
	adc		YH,R16

	lds		ZL,flow_sum				; flow_sum -= buffer[idx]
	lds		ZH,flow_sum+1
	ldd		XL,Y+0
	ldd		XH,Y+1
	sub		ZL,XL
	sbc		ZH,XH

	lds		XL,sensors_flow_value+2	; flow_sum += flow
	lds		XH,sensors_flow_value+3
	add		ZL,XL
	adc		ZH,XH
	sts		flow_sum,ZL
	sts		flow_sum+1,ZH

	std		Y+0,XL					; buffer[idx] = flow
	std		Y+1,XH

	lds		R16,flow_idx			; if (++idx==trigger_base_count) idx=0
	inc		R16
	cpi		R16,trigger_base_count
	brlo	process_trigger_no_0
	clr		R16
process_trigger_no_0:
	sts		flow_idx,R16


/*	rcall	process_trigger_pos_flow	; Analiza flujos positivos antes de apertura de paletas
;	breq	process_trigger_ret
	brne	process_trigger_nopos
	rjmp	process_trigger_ret
process_trigger_nopos:
*/


	stepper_read_pos				; if (pos>40) return .... if (pos>0) return
	sbrc	XH,7
	rjmp	process_trigger_pos
;	or		XH,XL
;	breq	process_trigger_pos
;	rjmp	process_trigger_ko
	cpi		XH,0
	brne	process_trigger_ko2
	cpi		XL,41
	brlo	process_trigger_pos
process_trigger_ko2:
	ldi		R16,0						; Reinicia contador tiempo pulso positivo
	sts		flow_pos_count,R16
	rjmp	process_trigger_ko
process_trigger_pos:

	lds		XL,sensors_flow_value+2		; if (|flow|>6) flow_count=0 else flow_count++
	lds		XH,sensors_flow_value+3
	lds		R16,flow_count
	sbrs	XH,7
	rjmp	process_trigger_abs
	com		XH
	neg		XL
	sbci	XH,255
process_trigger_abs:
	cpi		XH,0
	brne	process_trigger_count0
	cpi		XL,6
	brsh	process_trigger_count0
	cpi		R16,trigger_base_count
	brsh	process_trigger_count1
	inc		R16
	rjmp	process_trigger_count1
process_trigger_count0:
	ldi		R16,0
process_trigger_count1:
	sts		flow_count,R16

	lds		R18,flow_flag				; if ((flag==0) && (count>=trigger_base_count)) base=sum/32, flag=1
	cpi		R18,0
	brne	process_trigger_nordy
	cpi		R16,trigger_base_count
	brlo	process_trigger_nordy
process_trigger_base:
	asr		ZH
	ror		ZL
	lsr		R16
	brne	process_trigger_base
	sts		flow_base,ZL
	sts		flow_base+1,ZH
	ldi		R16,1
	sts		flow_flag,R16
process_trigger_nordy:

	rcall	process_trigger_pos_flow	; Analiza flujos positivos despues de apertura de paletas
	breq	process_trigger_ret

	lds		R16,flow_flag				; if ((flag==1) && (flow-base)>trigger) flow_trigger++ else flow_trigger=0
	cpi		R16,1
	brne	process_trigger_ko
	lds		XH,sensors_flow_value+3
	lds		XL,sensors_flow_value+2
	lds		ZH,flow_base+1
	lds		ZL,flow_base
	clr		ZL							; base=0
	clr		ZH
	sub		XL,ZL
	sbc		XH,ZH
	brlt	process_trigger_ko
	lds		ZH,set_trigger+1
	lds		ZL,set_trigger
	sub		XL,ZL
	sbc		XH,ZH
	brlt	process_trigger_ko
	lds		R16,flow_trigger
	inc		R16
	sts		flow_trigger,R16
	cpi		R16,trigger_width				; if  (flow_trigger>=trigger_width) return 1;
	brlo	process_trigger_ko1
	ldi		R16,0
	sts		flow_apnea,R16
	sez
	rjmp	process_trigger_ret
process_trigger_ko:
	ldi		R16,0
	sts		flow_trigger,R16
process_trigger_ko1:
	ldi		R16,1
	sts		flow_apnea,R16
	clz

process_trigger_ret:
	ret



; Analiza flujos positivos
; lanza triger si flow>trigger por el tiempo trigger_pos_width
; y previamente  flow<-6 por el tiempo trigger_neg_width
process_trigger_pos_flow:
	lds		XL,sensors_flow_value+2				; Solo analiza flujo positivo
	lds		XH,sensors_flow_value+3
	sbrc	XH,7
	rjmp	process_trigger_pos_ko
	rjmp	process_trigger_pos_flow_pos_pulse



	lds		XL,sensors_flow_value+2
	lds		XH,sensors_flow_value+3
	sbrs	XH,7
	rjmp	process_trigger_pos_flow_cmp_pos
	subi	XL,low(-6)					; if (flow<6) flow_neg_count++
	sbci	XH,high(-6)
	brge	process_trigger_pos_flow_cmp_fin
	ldi		R16,0						; Reinicia contador tiempo pulso positivo
	sts		flow_pos_count,R16
	lds		R16,flow_neg_count			; flow_neg_count++
	inc		R16
	breq	process_trigger_pos_flow_cmp_fin
	sts		flow_neg_count,R16
	rjmp	process_trigger_pos_flow_cmp_fin

process_trigger_pos_flow_cmp_pos:
	lds		R16,flow_neg_count			; verifica (flow_neg_count >= trigger_neg_width)
	cpi		R16,trigger_neg_width
	brlo	process_trigger_pos_flow_cmp_fin

process_trigger_pos_flow_pos_pulse:
	lds		ZH,set_trigger+1
	lds		ZL,set_trigger
	sub		XL,ZL
	sbc		XH,ZH
	brlt	process_trigger_pos_flow_cmp_fin
	lds		R16,flow_pos_count
	inc		R16
	breq	process_trigger_pos_flow_cmp_fin
	sts		flow_pos_count,R16
process_trigger_pos_flow_cmp_fin:

	lds		R16,flow_pos_count			; verifica (flow_pos_count >= trigger_pos_width)
	cpi		R16,trigger_pos_width
	brlo	process_trigger_pos_ko
	ldi		R16,0						; trigger detected
	sts		flow_apnea,R16
	sez
	rjmp	process_trigger_pos_ret

process_trigger_pos_ko:
	clz

process_trigger_pos_ret:
	ret