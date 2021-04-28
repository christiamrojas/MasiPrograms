/*
 * i2c.asm
 *
 *  Created: 08/05/2020 03:34:03 p. m.
 *   Author: Javier
 */ 

 .equ i2c_fscl		 = 200000			; from 100000 to 400000
 .equ i2c_timeout_ms = 2				; 2 ms timeout

; i2c_master_init -> initialize i2c for master operation at i2c_fscl scl speed
; ***************
.cseg
i2c_master_init:
;	PA2: (PULL-UP)	flow sensor SDA
;   PA3: (PULL-UP)	flow sensor SCL
	ldi		R16,0x0c
	sts		PORTA_OUTSET,R16
	sts		PORTA_DIRCLR,R16
	ldi		R16,PORT_PULLUPEN_bm
	sts		PORTA_PIN2CTRL,R16
	sts		PORTA_PIN3CTRL,R16

	ldi		R16,0x00						; stop i2c
	sts		TWI0_MCTRLA,R16
	sts		TWI0_DUALCTRL,R16
	sts		TWI0_DBGCTRL,R16

	ldi		R16,(fosc/i2c_fscl-10)/2		; set baud rate
	sts		TWI0_MBAUD,R16
	ldi		R16,TWI_DEFAULT_SDASETUP_8CYC_gc | TWI_DEFAULT_SDAHOLD_500NS_gc
	sts		TWI0_CTRLA,R16
	ldi		R16,TWI_TIMEOUT_200US_gc | TWI_ENABLE_bm
	sts		TWI0_MCTRLA,R16

	ldi		R16,TWI_FLUSH_bm				; flush data
	sts		TWI0_MCTRLB,R16
	ldi		R16,TWI_BUSSTATE_IDLE_gc		; go idle state
	sts		TWI0_MSTATUS,R16
	ldi		R16,TWI_RIF_bm | TWI_WIF_bm		; reset flags
	sts		TWI0_MSTATUS,R16
	ret

; i2c_master_read -> reads 2 byte data (high-low) from device address R16
; ***************
; input:	R16 -> device address
; output:	X   -> data
;			sez -> ok
;			clz -> error
.dseg
i2c_master_read_data:	.byte 2
.cseg
i2c_master_read:
	lsl		R16								; shift address
	ori		R16,0x01						; read
	sts		TWI0_MADDR,R16

	rcall	i2c_master_wait_rif				; send address, receive data high
	brne	i2c_master_read_ret
	lds		R16,TWI0_MDATA
	sts		i2c_master_read_data+1,R16
	ldi		R16,TWI_MCMD_RECVTRANS_gc		; send ack
	sts		TWI0_MCTRLB,R16

	rcall	i2c_master_wait_rif				; receive data low
	brne	i2c_master_read_ret
	lds		R16,TWI0_MDATA
	sts		i2c_master_read_data,R16
	ldi		R16,TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc	; send nack + stop
	sts		TWI0_MCTRLB,R16
	lds		XL,i2c_master_read_data
	lds		XH,i2c_master_read_data+1
	sez
i2c_master_read_ret:
	ret


; i2c_master_read_retry -> reads 2 byte data (high-low) from device address R16. One retry.
; *********************
; input:	R16 -> device address
; output:	X   -> data
;			sez -> ok
;			clz -> error
.dseg
i2c_master_read_device_addr:	.byte 1
.cseg
i2c_master_read_retry:
	sts		i2c_master_read_device_addr,R16	; first: try
	rcall	i2c_master_read
	breq	i2c_master_read_retry_ret

	task_change								; delay before retry
	task_change

	lds		R16,i2c_master_read_device_addr	; second: retry
	rcall	i2c_master_read
i2c_master_read_retry_ret:
	ret


; i2c_master_wait_rif -> waits i2c rif flag.
; *******************
; sez -> ok
; clz -> error
.dseg
i2c_master_wait_tmr_ms:	.byte 1
.cseg
i2c_master_wait_rif:
	ldi		R16,i2c_timeout_ms
	sts		i2c_master_wait_tmr_ms,R16
i2c_master_wait_rif_loop:
	task_change
	lds		R16,i2c_master_wait_tmr_ms		; timeout?
	cpi		R16,0
	breq	i2c_master_wait_rif_recover

	lds		R16,TWI0_MSTATUS
	cpi		R16,0xa2						; data ack?
	breq	i2c_master_wait_rif_ret
	andi	R16,0xef
	cpi		R16,0x02						; still waiting?
	breq	i2c_master_wait_rif_loop
	cpi		R16,0x22						; hold?
	breq	i2c_master_wait_rif_loop
i2c_master_wait_rif_recover:
	rcall	i2c_master_recover
	clz
i2c_master_wait_rif_ret:
	ret


; i2c_master_recover -> if no ack => stop
; *******************	else => stop i2c, restart
.dseg
i2c_err:	.byte 1
.cseg
i2c_master_recover:
	sts		i2c_err,R16
	cpi		R16,0x62						; no ack?
	brne	i2c_master_recover_ack
	ldi		R16,TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc
	sts		TWI0_MCTRLB,R16
	rjmp	i2c_master_recover_ret
i2c_master_recover_ack:
	task_change
	ldi		R16,0x00						; stop i2c
	sts		TWI0_MCTRLA,R16
	rcall	i2c_master_recover_scl_pulse	; send scl pulse
	lds		R16,i2c_err
	rcall	i2c_master_init					; restart i2c
i2c_master_recover_ret:
	ret


; i2c_master_recover_scl_pulse -> send scl negative pulse
; *******************
i2c_master_recover_scl_pulse:
	ldi		R16,$08					; scl output
	sts		PORTA_DIRSET,R16
	task_change
	ldi		R16,$08					; clear scl
	sts		PORTA_OUTCLR,R16
	task_change
	ldi		R16,$08					; set scl
	sts		PORTA_OUTSET,R16
	task_change
	ret