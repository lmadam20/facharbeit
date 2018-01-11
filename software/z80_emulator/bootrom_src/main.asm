ORG $E100

RESET:
	; disable interrupts
	DI

	; setup stack to first 256 bytes of RAM
	LD HL, $1100
	LD SP, HL

	; setup RST table in zero page
	LD HL, $00C3 		; lower two bytes (JP $..00)
	LD ($0000), HL
	LD A, $40 		; upper byte (..40..)
	LD ($0002), A
	LD A, $0
	LD ($0003), A		; NOP


	; fill screen blue
	LD BC, $4B00 	; count
	LD HL, $0000 	; address
	LD D, $3 	; value = $3
	CALL $E300 	; gpu_fill_screen


	RST 0
	HALT

