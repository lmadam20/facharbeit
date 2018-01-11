ORG $4000

; Vectors used here:
; 	$E004 - gpu_fill_screen

ENTRY:
	IN A, ($02) ; get last pressed keyboard character
	LD B, A		; save A to B for further operations

	SUB 'a'			; if last character not A
	JP NZ, not_a	; goto not_a

	LD BC, $4B00 	; count
	LD HL, $0000 	; address
	LD D, $C0 		; value
	CALL $E004 		; gpu_fill_screen

	JP ENTRY

not_a:
	LD A, B		; restore A
	SUB 'b'
	JP NZ, ENTRY

	LD BC, $4B00 	; count
	LD HL, $0000 	; address
	LD D, $3 		; value
	CALL $E004 		; gpu_fill_screen

	JP ENTRY

	HALT 	; should never be reached

