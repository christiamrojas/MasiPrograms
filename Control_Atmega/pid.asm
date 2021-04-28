/*
 * pid.asm
 *
 *  Created: 13/05/2020 04:25:14 p. m.
 *   Author: Javier
 */ 

.equ	pid_SAMPLING_TIME_ms = 10

.dseg
pid_sp:			.byte 2			; int16 (LO-HI)
pid_kp:			.byte 2			; frac8+int8
pid_ki:			.byte 2			; frac8+int8
pid_kd:			.byte 2			; frac8+int8

pid_e:			.byte 2			; frac8+int8
pid_ei:			.byte 2			; frac8+int8
pid_ei_max:		.byte 2			; frac8+int8
pid_ei_min:		.byte 2			; frac8+int8

pid_out_max:	.byte 2			; int16 (LO-HI)
pid_out_min:	.byte 2			; int16 (LO-HI)

pid_tmr_ms:		.byte 1
.cseg
; input:	XH:XL:R15 -> input process variable (frac8+int16)
; output:	XH:XL	  -> output control variable (int16)
pid_process:
	rcall	pid_process_kp
	rcall	pid_process_ki
	rcall	pid_process_kd
	rcall	pid_process_out
	ret	


; input:	XH:XL:R15 -> input process variable (frac8+int16)
; output:	YH:YL	  -> error (frac8+int8)
pid_process_kp:
	clr		YL						;   e =  sp-in
	lds		YH,pid_sp
	lds		R24,pid_sp+1
	sub		YL,R15
	sbc		YH,XL
	sbc		R24,XH
	sbrc	R24,7					; error limits
	rjmp	pid_process_kp_e_neg
	cpi		R24,0
	brne	pid_process_kp_e_max
	sbrc	YH,7
	rjmp	pid_process_kp_e_max
	rjmp	pid_process_kp_out
pid_process_kp_e_neg:
	cpi		R24,0xff
	brne	pid_process_kp_e_min
	sbrs	YH,7
	rjmp	pid_process_kp_e_min
	rjmp	pid_process_kp_out
pid_process_kp_e_min:
	ldi		YH,0x80				; min error
	ldi		YL,0x00
	rjmp	pid_process_kp_out
pid_process_kp_e_max:
	ldi		YH,0x7f				; max error
	ldi		YL,0xff

pid_process_kp_out:
;	rcall	pid_e2
	mov		R20,YL				; out += kp*e
	mov		R21,YH
	lds		R22,pid_kp
	lds		R23,pid_kp+1
;	rcall	mac16x16_32_method_B
	call	muls16x16_32
	ret


pid_e2:
	ldi		R16,0
	sbrs	YH,7
	rjmp	pid_e2_pos
	ldi		R16,1
	com		YH
	neg		YL
	sbci	YH,255
pid_e2_pos:
	mul		YL,YL
	mov		R20,R0
	mov		R21,R1

	mul		YH,YH
	mov		R22,R0
	mov		R23,R1

	mul		YH,YL
	add		R21,R0
	adc		R22,R1
	clr		R2
	adc		R23,R2
	add		R21,R0
	adc		R22,R1
	adc		R23,R2
	
	cpi		R23,0
	brne	pid_e2_max
	sbrs	R22,7
	rjmp	pid_e2_no_max
pid_e2_max:
	ldi		R21,0xfe
	ldi		R22,0x7f
pid_e2_no_max:
	mov		YL,R21
	mov		YH,R22
	sbrs	R16,0
	rjmp	pid_e2_ret
	com		YH
	neg		YL
	sbci	YH,255
pid_e2_ret:
	ret



; input:	YH:YL	  -> error (frac8+int8)
pid_process_ki:
	lds		R20,pid_ei			;  ei += e
	lds		R21,pid_ei+1
	add		R20,YL
	adc		R21,YH
	brvc	pid_process_ki_ei_add
	sbrc	YH,7
	rjmp	pid_process_ki_ei_min
	ldi		R20,low(0x7fff)
	ldi		R21,high(0x7fff)
	rjmp	pid_process_ki_ei_add
pid_process_ki_ei_min:
	ldi		R20,low(0x8000)
	ldi		R21,high(0x8000)
pid_process_ki_ei_add:

/*	sbrc	R21,7						; check programable limit
	rjmp	pid_process_ki_ei_neg
	cpi		R21,high(70*16)
	brlo	pid_process_ki_ei_process
	brne	pid_process_ki_ei_max2
	cpi		R20,low(70*16)
	brlo	pid_process_ki_ei_process
pid_process_ki_ei_max2:
	ldi		R21,high(70*16)
	ldi		R20,low(70*16)
	rjmp	pid_process_ki_ei_process
pid_process_ki_ei_neg:
	cpi		R21,high(-70*16)
	brlt	pid_process_ki_ei_min2
	brne	pid_process_ki_ei_process
	cpi		R20,low(-70*16)
	brge	pid_process_ki_ei_process
pid_process_ki_ei_min2:
	ldi		R21,high(-70*16)
	ldi		R20,low(-70*16)
pid_process_ki_ei_process:
*/

	sts		pid_ei,R20
	sts		pid_ei+1,R21

	lds		R22,pid_ki			; out += ki*ei
	lds		R23,pid_ki+1
	call	mac16x16_32_method_B
	ret


; input:	YH:YL	  -> error (frac8+int8)
pid_process_kd:
	lds		R20,pid_e				;    ed =  e-e_old
	lds		R21,pid_e+1				; e_old =  e
	sts		pid_e,YL
	sts		pid_e+1,YH
	sub		YL,R20
	sbc		YH,R21
	mov		R20,YL
	mov		R21,YH
	lds		R22,pid_kd				;   out += kd*ed
	lds		R23,pid_kd+1
	call	mac16x16_32_method_B
	ret


; output:	XH:XL	-> output control
pid_process_out:
	mov		XH,R19
	mov		XL,R18
	lds		YL,pid_out_min				; check minimum
	lds		YH,pid_out_min+1
	cp		XL,YL
	cpc		XH,YH
	brge	pid_process_out_check_max
	mov		XL,YL
	mov		XH,YH
	rjmp	pid_process_out_ret

pid_process_out_check_max:
	lds		YL,pid_out_max				; check maximum
	lds		YH,pid_out_max+1
	cp		XL,YL
	cpc		XH,YH
	brlt	pid_process_out_ret
	mov		XL,YL
	mov		XH,YH

pid_process_out_ret:
	ret






pid_wait_sampling_time:
	task_change
	lds		R16,pid_tmr_ms
	cpi		R16,0
	brne	pid_wait_sampling_time
	ldi		R16,pid_SAMPLING_TIME_ms
	sts		pid_tmr_ms,R16
	ret

pid_check_sampling_time:
	task_change
	lds		R16,pid_tmr_ms
	cpi		R16,0
	brne	pid_check_sampling_time_ret
	ldi		R16,pid_SAMPLING_TIME_ms
	sts		pid_tmr_ms,R16
pid_check_sampling_time_ret:
	ret