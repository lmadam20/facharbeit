ORG $E300

gpu_fill_screen_full:
	; address
	LD A, 2
	LD E, $00
	CALL gpu_cmd_write

	LD A, 3
	LD E, H
	CALL gpu_cmd_write

	LD A, 4
	LD E, L
	CALL gpu_cmd_write

	; count
	LD A, 5
	LD E, B
	CALL gpu_cmd_write

	LD A, 6
	LD E, C
	CALL gpu_cmd_write

	; value
	LD A, 7
	LD E, D
	CALL gpu_cmd_write

	; command
	LD A, 1
	LD E, $01
	CALL gpu_cmd_write

	LD A, 0
	LD E, $1
	CALL gpu_cmd_write

	CALL gpu_wait_response

	RET

gpu_cmd_write:
	OUT ($0B), A
	LD A, E
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

resp_nack:
	LD A, 0
	RET

resp_ack:
	LD A, 1
	RET
