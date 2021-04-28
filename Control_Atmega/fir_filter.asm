/*
 * fir_filter.asm
 *
 *  Created: 11/05/2020 11:36:58 p. m.
 *   Author: Javier
 */ 

.cseg

; fir_filter  -> receive new data X and apply fir filter
; **********
; Input:
; 	R16 -> filter size
; 	X   -> new data, signed 16 bits.
; 	Y   -> buffer pointer, signed 16 bits, low-high,old data first.
; 	Z   -> filter pointer, signed 16 bits, low-high.
; Output:
;	X   -> filtered data, signed 16 bits.
;   Y   -> filtered data, fractional 16 bits
fir_filter:
	mov		R15,R16			; last loop for new data X
	dec		R15

	clr 	R16				; accumulator
	clr		R17
	clr		R18
	clr		R19
fir_filter_loop:
	ldd		R20,Y+2			; data from buffer
	ldd		R21,Y+3
	st		Y+,R20			; shift data
	st		Y+,R21
	lpm		R22,Z+			; data from filter
	lpm		R23,Z+
	rcall	mac16x16_32_method_B
	dec		R15				; next tab
	brne	fir_filter_loop

	movw	R21:R20,XH:XL	; new data
	st		Y+,R20			; shift data
	st		Y+,R21
	lpm		R22,Z+			; data from filter
	lpm		R23,Z+
	rcall	mac16x16_32_method_B

	movw	XH:XL,R19:R18
	movw	YH:YL,R17:R16
	ret


; filter input:  signed 16 bits accumulated adc
; filter output: signed 16 bits. 8 msb = cmH2O (-128,127)
.equ fir_filter_pressure_shift = 8
fir_filter_pressure:
;	.dw 529,1342,2240,3638,4886,6167,6917,7284,6917,6167,4886,3638,2240,1342,529
	.dw 527,1337,2232,3625,4869,6145,6892,7258,6892,6145,4869,3625,2232,1337,527

fir_filter_pressure_end:

; filter input:  signed 16 bits (from 14 bits flow sensor)
; filter output: signed 16 bits. 10 msb = l/min (-512,512)
.equ fir_filter_flow_shift = 6
fir_filter_flow:
;	.dw -880,322,482,754,1138,1628,2221,2913,3695
;	.dw 4557,5483,6453,7445,8434,9392,10292,11109,11815,12391
;	.dw 12816,13077,13164,13077,12816,12391,11815,11109,10292,9392
;	.dw 8434,7445,6453,5483,4557,3695,2913,2221,1628,1138
;	.dw 754,482,322,-880

	.dw	-800,293,438,686,1035,1480,2019,2648,3359
	.dw 4143,4985,5867,6769,7667,8538,9357,10099,10741,11264
	.dw 11651,11888,11968,11888,11651,11264,10741,10099,9357,8538
	.dw 7667,6769,5867,4985,4143,3359,2648,2019,1480,1035
	.dw 686,438,293,-800

;	.dw 687,2950,4679,7891,11408,15266,18924,21971,23986,24693,23986,21971,18924,15266,11408,7891,4679,2950,687


//	.dw 21845,21846,21845
fir_filter_flow_end:

; filter input:  signed 16 bits accumulated adc
; filter output: signed 16 bits. 9 msb = %oxygen (-128,127)
.equ fir_filter_oxygen_shift = 7
fir_filter_oxygen:
	.dw 1851,9341,19610,24775,19610,9341,1851

;	.dw 21845,21846,21845
fir_filter_oxygen_end:

; filter input:  signed 16 bits accumulated adc
; filter output: signed 16 bits. 8 msb = volt (-128,127)
.equ fir_filter_volt_shift = 0
fir_filter_volt:
	.dw 21845,21846,21845
fir_filter_volt_end:

; filter input:  signed 16 bits accumulated adc
; filter output: signed 16 bits. 8 msb = volt (-128,127)
.equ fir_filter_current_shift = 0
fir_filter_current:
	.dw 21845,21846,21845
fir_filter_current_end:
