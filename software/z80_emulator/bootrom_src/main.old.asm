ORG $E100

RESET:
	; disable interrupts
	DI

	; setup stack to first 256 bytes of RAM
	LD HL, $1100
	LD SP, HL

	; setup RST table in zero page
	LD HL, $00C3 	; lower two bytes (JP $..00)
	LD ($0000), HL
	LD A, $40 		; upper byte (..40..)
	LD ($0002), A
	LD A, $0
	LD ($0003), A		; NOP


; screen filling pseudocode:
;
; framebuffer_page = 0
; C = $3
; for D = 12; D > 0; D--
; {
;	for B = 7; B > 7; B--
; 		for A = 255; A > 0; A--
;			framebuffer_n[B * 255 + A] = C
;	framebuffer_page = framebuffer_page + 1
; }

	; setup framebuffer page
	LD A, 0
	OUT ($0A), A

	; fill screen blue
	LD HL, $8000 ; HL = framebuffer page n
	LD C, $3
	LD B, 7 ; counter 2
	LD D, 12 ; fb page counter

fill1:
	LD A, 255 ; counter 1

fill2:
	LD (HL), C
	INC HL ; move to next pixel
	SUB 1
	JP NZ, fill2 ; if counter 1 != 0 -> fill2

	LD A, B
	SUB 1
	LD B, A
	JP NZ, fill1 ; if counter 2 != 0 -> fill1

	LD A, D
	SUB 1
	LD D, A
	JP Z, fill_done ; we are done with filling when D == 0
	LD A, 12
	SUB D
	OUT ($0A), A ; switch to next page
	LD HL, $8000
	LD B, 7 ; counter 2
	JP fill1

fill_done:
	CALL check_kbd

	LD B, $FF
	CALL gpu_fill_full_screen

	LD A, 1
	LD C, $02
	CALL gpu_cmd_write

	LD A, 2
	LD C, 'Z'
	CALL gpu_cmd_write

	LD A, 3
	LD C, '8'
	CALL gpu_cmd_write

	LD A, 4
	LD C, '0'
	CALL gpu_cmd_write

	LD A, 0
	LD C, 1

	RST 0
	HALT

check_kbd:
	LD A, 'a'
	OUT ($00), A
	NOP
	IN A, ($01)
	SUB $1
	JP NZ, check_kbd

	RET


gpu_fill_full_screen:
	; address
	LD A, 2
	LD C, $00
	CALL gpu_cmd_write

	LD A, 3
	LD C, $00
	CALL gpu_cmd_write

	LD A, 4
	LD C, $00
	CALL gpu_cmd_write

	; count
	LD A, 5
	LD C, $4B
	CALL gpu_cmd_write

	LD A, 6
	LD C, $00
	CALL gpu_cmd_write

	; value
	LD A, 7
	LD C, B
	CALL gpu_cmd_write

	; command
	LD A, 1
	LD C, $01
	CALL gpu_cmd_write

	LD A, 0
	LD C, $1
	CALL gpu_cmd_write

	CALL gpu_wait_response

	RET

gpu_cmd_write:
	OUT ($0B), A
	LD A, C
	OUT ($0C), A

	RET

gpu_cmd_read:
	OUT ($0B), A
	IN A, ($0C)

	RET

gpu_wait_response:
	LD A, 1
	CALL gpu_cmd_read
	LD B, A

	SUB $FF
	JP Z, resp_ack
	JP NZ, resp_other

resp_other:
	LD A, B
	SUB $FE
	JP Z, resp_nack
	JP NZ, gpu_wait_response
	RET

resp_nack:
	LD A, 0
	RET

resp_ack:
	LD A, 1
	RET
