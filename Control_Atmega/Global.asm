/*
 * Global.asm
 *
 *  Created: 9/02/2019 13:03:59
 *   Author: Computer
 */ 

.cseg

; Convierte el nibble bajo de R16 en un texto hexadecimal
;
; Entrada: R16 -> Numero binario. considera solo 4 bits menos significativos
; Salida : R16 -> Texto Hexadecimal
Nibble2Hex:
	andi	R16,$0f
	ori		R16,$30
	cpi		R16,$3a
	brlo	Nibble2Hex_Fin
	subi	R16,-7
Nibble2Hex_Fin:
	ret


; Convierte el nibble alto y bajo de R16 en dos textos hexadecimales
;
; Entrada: R16 -> Numero binario
; Salida : R16 -> Texto Hexadecimal parte alta
;		   R17 -> Texto Hexadecimal parte baja
Bin2Hex:
	push	R16
	swap	R16
	rcall	Nibble2Hex
	mov		R17,R16
	pop		R16
	rcall	Nibble2Hex
	ret

; Convierte R16 en un texto decimal (3 caracteres)
;
; Entrada: R16 -> Numero binario
; Salida : R16 -> Texto Decimal centenas
;		   R17 -> Texto Decimal decenas
;		   R18 -> Texto Decimal unidades
Bin2Dec:
	ldi		R19,'0'-1
	ldi		R17,'0'-1
	ldi		R18,'0'+10

Bin2Dec_Centenas:
	inc		R19
	subi	R16,100
	brsh	Bin2Dec_Centenas
	subi	R16,-100

Bin2Dec_Decenas:
	inc		R17
	subi	R16,10
	brsh	Bin2Dec_Decenas

Bin2Dec_Unidades:
	add		R18,R16
	mov		R16,R19
	ret



; Convierte X en un texto decimal (5 caracteres)
;
; Entrada:   X -> Numero binario
; Salida : R16 -> Texto Decimal x10000
;		   R17 -> Texto Decimal x1000
;		   R18 -> Texto Decimal x100
;		   R19 -> Texto Decimal x10
;		   R20 -> Texto Decimal x1
Bin16aDec:
	ldi		R20,0
	ldi		R21,$0f
	ldi		R22,10
	ldi		R23,205

	mov		R10,XL
	mov		R11,XL
	mov		R12,XH
	mov		R13,XH
	swap	R11
	swap	R13
	and		R10,R21				; n0
	and		R11,R21				; n1
	and		R12,R21				; n2
	and		R13,R21				; n3

	mov		R16,R11				; 6*(n3+n2+n1)+n0 -> a0(R1_H:R10_L). Max(a0)=285
	add		R16,R12
	add		R16,R13
	ldi		R17,6
	mul		R16,R17
	add		R10,R0
	adc		R1,R20

	mul		R1,R23				; cociente(a0/10) -> R16
	mov		R16,R0
	mul		R10,R23
	add		R16,R1
	lsr		R16
	lsr		R16
	lsr		R16
	mul		R16,R22				; residuo(a0/10) -> R10				*** UNIDADES ***
	sub		R10,R0

	mov		R19,R12				; 5*(n3+n2)+4*n3+n1  +cociente(a0/10) -> a1(R11). Max(a1)=225+28=253
	add		R19,R13
	ldi		R17,5
	mul		R19,R17
	lsl		R13
	lsl		R13
	add		R11,R0
	add		R11,R13
	add		R11,R16

	mul		R11,R23				; cociente(a1/10)+2*n2 -> a2(R12)
	lsr		R1
	lsr		R1
	lsr		R1
	lsl		R12
	add		R12,R1
	mul		R1,R22				; residuo(a1/10) -> R11				*** DECENAS ***
	sub		R11,R0

	mul		R12,R23				; cociente(a2/10)+4*n3 -> a3(R13)
	lsr		R1
	lsr		R1
	lsr		R1
	add		R13,R1
	mul		R1,R22				; residuo(a2/10) -> R12				*** CENTENAS ***
	sub		R12,R0

	mul		R13,R23				; cociente(a3/10) -> a3(R14)		*** DECENAS DE MILLAR ***
	lsr		R1
	lsr		R1
	lsr		R1
	mov		R14,R1
	mul		R1,R22				; residuo(a3/10) -> R13				*** MILLAR ***
	sub		R13,R0

	ldi		R16,'0'
	ldi		R17,'0'
	ldi		R18,'0'
	ldi		R19,'0'
	ldi		R20,'0'
	or		R16,R14
	or		R17,R13
	or		R18,R12
	or		R19,R11
	or		R20,R10
	ret




;*** Convierte un numero binario de 4 bytes apuntado por X (L-H)
;*** en texto (10 bytes) a partir de la posicion Y
;
; 0<=a8<=42 (30+12)		a8 = 2n7
; 0<=a7<=129 (105+24)	a7 = 6n7 + 1n6
; 0<=a6<=244 (225+19)	a6 = 8n7 + 6n6 + 1n5
; 0<=a5<=199 (165+34)	a5 = 4n7 + 7n6 + 0n5
; 0<=a4<=346 (300+46)	a4 = 3n7 + 7n6 + 4n5 + 6n4
; 0<=a3<=467 (435+32)	a3 = 5n7 + 7n6 + 8n5 + 5n4 + 4n3
; 0<=a2<=322 (270+52)	a2 = 4n7 + 2n6 + 5n5 + 5n4 + 0n3 + 2n2
; 0<=a1<=529 (465+64)	a1 = 5n7 + 1n6 + 7n5 + 3n4 + 9n3 + 5n2 + 1n1
; 0<=a0<=645			a0 = 6n7 + 6n6 + 6n5 + 6n4 + 6n3 + 6n2 + 6n1 + 1n0
Bin32aDec:
	clr		R15
	ldi		R20,10
	ldi		R21,205
	ldi		R23,6
	ldi		R24,'0'

	ld		R16,X+
	mov		R17,R16
	andi	R16,$0f					; a0 = 1n0

	swap	R17
	andi	R17,$0f					; a1 = 1n1
	mul		R17,R23
	add		R16,R0					; a0 += 6n1

	ld		R18,X+
	mov		R19,R18
	andi	R18,$0f
	mul		R18,R23
	add		R16,R0					; a0 += 6n2
	sub		R0,R18
	add		R17,R0					; a1 += 5n2
	lsl		R18
	mov		R2,R18					; a2 = 2n2

	swap	R19
	andi	R19,$0f
	ldi		R18,9
	mul		R19,R18
	add		R17,R0					; a1 += 9n3
	mul		R19,R23
	add		R16,R0					; a0 += 6n3		(a partir de aqui considerar a0 16 bits)
	clr		R10
	adc		R10,R1
	lsl		R19
	lsl		R19
	mov		R3,R19					; a3 = 4n3

	ld		R18,X+
	mov		R19,R18
	andi	R18,$0f
	mul		R18,R23
	add		R16,R0					; a0 += 6n4
	adc		R10,R1
	mov		R4,R0					; a4 = 6n4
	sub		R0,R18
	add		R2,R0					; a2 += 5n4
	add		R3,R0					; a3 += 5n4
	lsl		R18
	sub		R0,R18
	add		R17,R0					; a1 += 3n4		(a partir de aqui considerar a1 16 bits)
	clr		R11
	adc		R11,R15

	swap	R19
	andi	R19,$0f
	mov		R6,R19					; a6 = 1n5
	mov		R18,R19
	lsl		R19
	lsl		R19
	add		R4,R19					; a4 += 4n5
	add		R19,R18
	add		R2,R19					; a2 += 5n5
	add		R19,R18
	add		R16,R19					; a0 += 6n5
	adc		R10,R15
	add		R19,R18
	add		R17,R19					; a1 += 7n5
	adc		R11,R15
	add		R19,R18
	add		R3,R19					; a3 += 8n5

	ld		R18,X+
	mov		R19,R18
	andi	R18,$0f
	mov		R7,R18					; a7 = 1n6
	add		R17,R18					; a1 += 1n6
	adc		R11,R15
	mul		R18,R23
	add		R16,R0					; a0 += 6n6
	adc		R10,R15
	add		R6,R0					; a6 += 6n6
	add		R0,R18
	add		R3,R0					; a3 += 7n6		(a partir de aqui considerar a3 16 bits)
	clr		R13
	adc		R13,R1
	add		R4,R0					; a4 += 7n6
	mov		R5,R0					; a5 = 7n6
	lsl		R18
	add		R2,R18					; a2 += 2n6

	swap	R19
	andi	R19,$0f
	mov		R18,R19
	lsl		R19
	mov		R8,R19					; a8 = 2n7
	add		R19,R18
	add		R4,R19					; a4 += 3n7		(a partir de aqui considerar a4 16 bits)
	clr		R14
	adc		R14,R15
	add		R19,R18
	add		R2,R19					; a2 += 4n7		(a partir de aqui considerar a2 16 bits)
	clr		R12
	adc		R12,R15
	add		R5,R19					; a5 += 4n7
	add		R19,R18
	add		R17,R19					; a1 += 5n7
	add		R11,R15
	add		R3,R19					; a3 += 5n7
	adc		R13,R15
	add		R19,R18
	add		R16,R19					; a0 += 6n7
	adc		R10,R15
	add		R7,R19					; a7 += 6n7
	add		R19,R18
	add		R19,R18
	add		R6,R19					; a6 += 8n7



	mul		R16,R21					; a1 += Cociente(a0/10)
	mov		R18,R1
	mul		R10,R21
	add		R0,R18
	adc		R1,R15
	lsr		R1
	ror		R0
	lsr		R1
	ror		R0
	lsr		R0
	add		R17,R0
	adc		R11,R1
	mul		R0,R20					; UNIDADES = Residuo (a0/10)
	sub		R16,R0
	or		R16,R24
	std		Y+9,R16

	mul		R17,R21					; a2 += Cociente(a1/10)
	mov		R18,R1
	mul		R11,R21
	add		R0,R18
	adc		R1,R15
	lsr		R1
	ror		R0
	lsr		R0
	lsr		R0
	add		R2,R0
	adc		R12,R1
	mul		R0,R20					; DECENAS = Residuo (a1/10)
	sub		R17,R0
	or		R17,R24
	std		Y+8,R17

	mul		R2,R21					; a3 += Cociente(a2/10)
	mov		R18,R1
	mul		R12,R21
	add		R0,R18
	adc		R1,R15
	lsr		R1
	ror		R0
	lsr		R0
	lsr		R0
	add		R3,R0
	adc		R13,R1
	mul		R0,R20					; CENTENAS = Residuo (a2/10)
	sub		R2,R0
	or		R2,R24
	std		Y+7,R2

	mul		R3,R21					; a4 += Cociente(a3/10)
	mov		R18,R1
	mul		R13,R21
	add		R0,R18
	adc		R1,R15
	lsr		R1
	ror		R0
	lsr		R0
	lsr		R0
	add		R4,R0
	adc		R14,R1
	mul		R0,R20					; MILLARES = Residuo (a3/10)
	sub		R3,R0
	or		R3,R24
	std		Y+6,R3

	mul		R4,R21					; a5 += Cociente(a4/10)
	mov		R18,R1
	mul		R14,R21
	add		R0,R18
	adc		R1,R15
	lsr		R1
	ror		R0
	lsr		R0
	lsr		R0
	add		R5,R0
	mul		R0,R20					; DECENAS de MILLAR = Residuo (a4/10)
	sub		R4,R0
	or		R4,R24
	std		Y+5,R4

	mul		R5,R21					; a6 += Cociente(a5/10)
	lsr		R1
	lsr		R1
	lsr		R1
	add		R6,R1
	mul		R1,R20					; CENTENAS de MILLAR = Residuo (a5/10)
	sub		R5,R0
	or		R5,R24
	std		Y+4,R5

	mul		R6,R21					; a7 += Cociente(a6/10)
	lsr		R1
	lsr		R1
	lsr		R1
	add		R7,R1
	mul		R1,R20					; MILLONES = Residuo (a6/10)
	sub		R6,R0
	or		R6,R24
	std		Y+3,R6

	mul		R7,R21					; a8 += Cociente(a7/10)
	lsr		R1
	lsr		R1
	lsr		R1
	add		R8,R1
	mul		R1,R20					; DECENAS de MILLON = Residuo (a7/10)
	sub		R7,R0
	or		R7,R24
	std		Y+2,R7

	mul		R8,R21					; MILES de MILLON  = Cociente(a8/10)
	lsr		R1
	lsr		R1
	lsr		R1
	mov		R9,R1
	or		R9,R24
	st		Y,R9
	mul		R1,R20					; CENTENAS de MILLON = Residuo (a8/10)
	sub		R8,R0
	or		R8,R24
	std		Y+1,R8

	ret



; Copia la cadena 

;IZ apuntada por Z a X
;
; Entrada:   Z -> Cadena ASCIIZ en memoria Flash
;			 X -> Cadena ASCIIZ en SRAM
cpyZaX:
	lpm		R16,Z+
	st		X+,R16
	cpi		R16,0
	brne	cpyZaX
	ret



; Copia n caracteres de Z a X
;
; Entrada:   Z -> Cadena en memoria Flash
;			 X -> Cadena en SRAM
;			R16-> Cantidad de caracteres a copiar (1..255)
cpynZaX:
	cpi		R16,0
	breq	cpynZaX_Fin
cpynZaX_Lazo:
	lpm		R17,Z+
	st		X+,R17
	dec		R16
	brne	cpynZaX_Lazo
cpynZaX_Fin:
	ret


; Copia n caracteres de Y a X
;
; Entrada:   Y -> Cadena en SRM
;			 X -> Cadena en SRAM
;			R16-> Cantidad de caracteres a copiar (1..255)
cpynYaX:
	cpi		R16,0
	breq	cpynYaX_Fin
cpynYaX_Lazo:
	ld		R17,Y+
	st		X+,R17
	dec		R16
	brne	cpynYaX_Lazo
cpynYaX_Fin:
	ret



; Devuelve la longitud de una cadena ASCIIZ en SRAM. Hasta 255 caracteres
;
; Entrada:   Z -> Cadena ASCIIZ en memoria SRAM
; Salida:	R16-> Longitud de cadena (0..255)
strlen_Z:
	clr		R16
strlen_Z_Lazo:
	ld		R18,Z+
	cpi		R18,0
	breq	strlen_Z_Fin
	inc		R16
	cpi		R16,0
	brne	strlen_Z_Lazo
	ldi		R16,255
strlen_Z_Fin:
	ret



; Devuelve la longitud de una cadena ASCIIZ en SRAM. Hasta R16 caracteres
;
; Entrada:   Z -> Cadena ASCIIZ en memoria SRAM
;			R16-> Longitud maxima permitida
; Salida:	R16-> Longitud de cadena
strnlen:
	inc		R16							; Un caracter mas para considerar el 0 final
	mov		R17,R16
	clr		R16
strnlen_Lazo:
	ld		R18,Z+
	cpi		R18,0
	breq	strnlen_Fin
	inc		R16
	dec		R17
	brne	strnlen_Lazo
	dec		R16
strnlen_Fin:
	ret






; Devuelve la longitud de una cadena ASCIIZ en flash. Hasta 255 caracteres
;
; Entrada:   Z -> Cadena ASCIIZ en memoria Flash
; Salida:	R16-> Longitud de cadena (0..255)
strlen_Flash:
	clr		R16
strlen_Flash_Lazo:
	lpm		R18,Z+
	cpi		R18,0
	breq	strlen_Flash_Fin
	inc		R16
	cpi		R16,0
	brne	strlen_Flash_Lazo
	ldi		R16,255
strlen_Flash_Fin:
	ret



; Devuelve la longitud de una cadena ASCIIZ en flash hasta R16 caracteres
;
; Entrada:   Z -> Cadena ASCIIZ en memoria Flash
;			R16-> Longitud maxima permitida
; Salida:	R16-> Longitud de cadena
strnlen_Flash:
	inc		R16							; Un caracter mas para considerar el 0 final
	mov		R17,R16
	clr		R16
strnlen_Flash_Lazo:
	lpm		R18,Z+
	cpi		R18,0
	breq	strnlen_Flash_Fin
	inc		R16
	dec		R17
	brne	strnlen_Flash_Lazo
	dec		R16
strnlen_Flash_Fin:
	ret


; Divide aproximadamente X/R16
;
; Entrada:	  X -> Dividendo
;			R16 -> Divisor (de 1 a 18)
; Salida:	  X -> Residuo
Tabla_Div16_8_aprox:
	.db	128,85,64,51,43,37,32,28,26,23,21,20,18,17,16,15,14,13
;Divisor  2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18
Div16_8_aprox:
	cpi		R16,2							; Verifica rango de 2 a 18. Division entre 1 no hay que hacer nada
	brlo	Div16_8_aprox_Fin
	cpi		R16,18+1
	brsh	Div16_8_aprox_Fin

	subi	R16,2							; Apunta a tabla de factores
	ldi		ZH,high(Tabla_Div16_8_aprox*2)
	ldi		ZL,low(Tabla_Div16_8_aprox*2)
	add		ZL,R16
	ldi		R16,0
	adc		ZH,R16
	lpm		R16,Z

	mul		XL,R16							; Multiplica por factor
	mov		ZL,R0
	mov		XL,R1
	mul		XH,R16
	add		XL,R0
	ldi		XH,0
	adc		XH,R1

	sbrc	ZL,7							; Redondea
	adiw	XL,1
Div16_8_aprox_Fin:
	ret


; Multiplica X (16 bits) por R16 (8 bits)
;
; Entrada:    X -> Numero de 16 bits
;			R16 -> Numero de 8 bits
; Salida:	  X -> Resultado
;			sec -> Overflow
;		    clc -> No Overflow
;			sez -> Resultado 0
;			clz -> Resultado no 0
Mul16_8:
	mul		XH,R16
	mov		XH,R0
	mov		R2,R1
	mul		XL,R16
	mov		XL,R0
	add		XH,R1
	adc		R2,R2
	brne	Mul16_8_KO
	mov		R2,XL
	or		R2,XH
	clc
	rjmp	Mul16_8_Fin
Mul16_8_KO:
	sec
Mul16_8_Fin:
	ret



;*** Dividendo 	R3:R0 (H-L)
;*** Divisor	R7:R4 (H-L)
;*** = Cociente	R3:R0 (H-L)
;***   Residuo  R11:R8 (H-L)
Divide32:
	push	R16

	clr		R11		; Borra Residuo y Carry
	clr		R10
	clr		R9
	clr		R8
	clc
	ldi		R16,33	; Carga Contador de Lazo con 33
Divide32Lazo:
	rol		R0		; Desplaza izquierda dividendo hacia el carry
	rol		R1
	rol		R2
	rol		R3
	dec		R16		; Decrementa contador de lazo
	breq	Divide32Fin
	rol		R8		; Desplaza izquierda carry(dividendo) hacia el residuo
	rol		R9
	rol		R10
	rol		R11
	sub		R8,R4	; Residuo= Residuo - Divisor
	sbc		R9,R5
	sbc		R10,R6
	sbc		R11,R7
	brcc	Divide32Pos
	add		R8,R4	; Resultado negativo-> Suma residuo, Borra Carry, y Lazo
	adc		R9,R5
	adc		R10,R6
	adc		R11,R7
	clc
	rjmp	Divide32Lazo
Divide32Pos:
	sec				; Activa Carry y va al Lazo
	rjmp	Divide32Lazo
Divide32Fin:

	pop		R16
	ret





;******************************************************************************
;*
;* FUNCTION
;*	mac16x16_32
;* DECRIPTION
;*	Signed multiply accumulate of two 16bits numbers with
;*	a 32bits result.
;* USAGE
;*	r19:r18:r17:r16 += r23:r22 * r21:r20
;* STATISTICS
;*	Cycles :	23 + ret
;*	Words :		19 + ret
;*	Register usage: r0 to r2 and r16 to r23 (11 registers)
;*
;******************************************************************************

mac16x16_32:
	clr	r2

	muls	r23, r21		; (signed)ah * (signed)bh
	add	r18, r0
	adc	r19, r1

	mul	r22, r20		; al * bl
	add	r16, r0
	adc	r17, r1
	adc	r18, r2
	adc	r19, r2

	mulsu	r23, r20		; (signed)ah * bl
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2

	mulsu	r21, r22		; (signed)bh * al
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2

	ret

mac16x16_32_method_B:			; uses two temporary registers
					; (r4,r5), but reduces cycles/words
					; by 1
	clr	r2

	muls	r23, r21		; (signed)ah * (signed)bh
	movw	r5:r4,r1:r0

	mul	r22, r20		; al * bl

	add	r16, r0
	adc	r17, r1
	adc	r18, r4
	adc	r19, r5

	mulsu	r23, r20		; (signed)ah * bl
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2

	mulsu	r21, r22		; (signed)bh * al
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2

	ret




;******************************************************************************
;*
;* FUNCTION
;*	muls16x16_32
;* DECRIPTION
;*	Signed multiply of two 16bits numbers with 32bits result.
;* USAGE
;*	r19:r18:r17:r16 = r23:r22 * r21:r20
;* STATISTICS
;*	Cycles :	19 + ret
;*	Words :		15 + ret
;*	Register usage: r0 to r2 and r16 to r23 (11 registers)
;* NOTE
;*	The routine is non-destructive to the operands.
;*
;******************************************************************************

muls16x16_32:
	clr	r2
	muls	r23, r21		; (signed)ah * (signed)bh
	movw	r19:r18, r1:r0
	mul	r22, r20		; al * bl
	movw	r17:r16, r1:r0
	mulsu	r23, r20		; (signed)ah * bl
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2
	mulsu	r21, r22		; (signed)bh * al
	sbc	r19, r2
	add	r17, r0
	adc	r18, r1
	adc	r19, r2
	ret



; int16_16_to_float -> convert 16 bits signed integer + 16 bits fractional, to 32 bits float
; *****************
; input:	R19,R18			-> signed 16 bits integer HI-LO
;			R17,R16			-> 16 bits fractional HI-LO
; output:	R19,R18,R17,R16 -> 32 bits float Hi-LO
int16_16_to_float:
	ldi		R20,134					; exponent
	ldi		R21,0x80				; sign
	and		R21,R19
	breq	int16_16_to_float_pos
	com		R19						; neg int 32
	com		R18
	com		R17
	neg		R16
	sbci	R17,255
	sbci	R18,255
	sbci	R19,255
int16_16_to_float_pos:

	cpi		R19,0
	breq	int16_16_to_float_zero_1
	mov		R16,R17
	mov		R17,R18
	mov		R18,R19
	subi	R20,-8
	rjmp	int16_16_to_float_left
int16_16_to_float_zero_1:

	cpi		R18,0
	brne	int16_16_to_float_left

	cpi		R17,0
	breq	int16_16_to_float_zero_2
	mov		R18,R17
	mov		R17,R16
	clr		R16
	subi	R20,8
	rjmp	int16_16_to_float_left

int16_16_to_float_zero_2:
	cpi		R16,0
	breq	int16_16_to_float_ret
	mov		R18,R16
	clr		R17
	clr		R16
	subi	R20,16

int16_16_to_float_left:
	sbrc	R18,7
	rjmp	int16_16_to_float_exp
	lsl		R16
	rol		R17
	rol		R18
	dec		R20
	rjmp	int16_16_to_float_left
int16_16_to_float_exp:

	lsl		R18
	mov		R19,R20
	lsl		R21
	ror		R19
	ror		R18

int16_16_to_float_ret:
	ret






; X int16, Y frac16
linearize_neg:
	sbrs	XH,7
	rjmp	linearize_neg_ret
	com		XH						; neg int 32
	com		XL
	com		YH
	neg		YL
	sbci	YH,255
	sbci	XL,255
	sbci	XH,255

	rcall	linearize_pos
	com		XH						; neg int 32
	com		XL
	com		YH
	neg		YL
	sbci	YH,255
	sbci	XL,255
	sbci	XH,255
linearize_neg_ret:
	ret



; input:	flow	= (X int16, Y frac16)		, only positive value
;			Z		= pointer to line data: range(1.1),gain(1.2),offset(1.2)   x5
; output:   
linearize_pos:
	sbrc	XH,7
	rjmp	linearize_pos_ret
	cpi		XH,0								; must be 0. No more than 255 l/m
	brne	linearize_pos_top

	ldi		R18,5
linearize_pos_loop:
	ld		R16,Z+								; range Lo-Hi
	ld		R17,Z+
	cpi		R17,$ff
	breq	linearize_pos_top
	cp		YH,R16
	cpc		XL,R17
	brsh	linearize_pos_next
	ld		R18,Z+								; gain Lo-Hi
	ld		R17,Z+
	ld		R16,Z+
	ld		R21,Z+								; offset
	ld		R20,Z+
	ld		R19,Z+
	rcall	linearize_gain_offset
	rjmp	linearize_pos_ret
linearize_pos_next:
	adiw	ZL,6								; next range
	dec		R18
	brne	linearize_pos_loop

linearize_pos_top:								; top value
	ldi		XH,0
	ldi		XL,0xff
	ldi		YH,0xff
	ldi		YL,0xff

linearize_pos_ret:
	ret




linearize_gain_offset:
	clr		R4
	mul		R16,XL
	mov		R13,R1
	mov		R12,R0
	mul		R16,YL
	mov		R11,R1
	mov		R10,R0
	mul		R18,YH
	mov		R2,R1
	mul		R16,YH
	mov		R3,R1
	mov		R2,R0
	mul		R17,YL
	add		R10,R1
	adc		R11,R2
	adc		R12,R3
	adc		R13,R4
	mul		R17,XL
	mov		R3,R1
	mov		R2,R0
	mul		R18,YH
	add		R10,R1
	adc		R11,R2
	adc		R12,R3
	adc		R13,R4
	mul		R18,XL
	add		R10,R0
	adc		R11,R1
	adc		R12,R4
	adc		R13,R4
	mul		R17,YH
	add		R10,R0
	adc		R11,R1
	adc		R12,R4
	adc		R13,R4
	
	ldi		R16,0
	sbrc	R19,7
	ldi		R16,$ff

	add		R10,R21
	adc		R11,R20
	adc		R12,R19
	adc		R13,R16

	sbrs	R13,7
	rjmp	linearize_gain_offset_pos
linearize_gain_offset_zero:
	clr		XH
	clr		XL
	clr		YH
	clr		YL
	rjmp	linearize_gain_offset_ret
linearize_gain_offset_pos:
	mov		XH,R13
	mov		XL,R12
	mov		YH,R11
	mov		YL,R10
linearize_gain_offset_ret:
	ret



; X int16, Y frac16
Neg_Flow_Gain:
	sbrs	XH,7
	rjmp	Neg_Flow_Gain_ret

	mov		R0,YL
	mov		R1,YH
	mov		R2,XL
	mov		R3,XH

	asr		R3
	ror		R2
	ror		R1
	ror		R0
	asr		R3
	ror		R2
	ror		R1
	ror		R0
	asr		R3
	ror		R2
	ror		R1
	ror		R0

	add		YL,R0
	adc		YH,R1
	adc		XL,R2
	adc		XH,R3
Neg_Flow_Gain_ret:
	ret
