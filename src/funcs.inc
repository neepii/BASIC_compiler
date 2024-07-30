
#;rax - buffer
#;rdi - dword (a integer to convert)
uitoa:
	movq %rax, %rsi

	movq %rdi, %rax

	cmp $0, %rax
 	jnz uitoa_convert_reg
	movb $48, (%rsi)
	inc %esi
	movb $0, (%rsi)
	mov $1, %rax
	jmp uitoa_end
	
uitoa_convert_reg:
	mov $10, %r9

	xor %rcx, %rcx
uitoa_loop:
	xor %rdx, %rdx
	div %r9
	inc %ecx
	cmp $0, %rax
	jnz uitoa_loop

	inc %ecx

	mov %rcx, %r8
	add %rcx, %rsi

	movb $0,(%rsi)
	mov %rdi, %rax
	dec %ecx
uitoa_convert:
	xor %rdx, %rdx
	dec %rsi
	div %r9
	add $48, %rdx
	movb %dl,(%rsi)
	loopnz uitoa_convert

	mov %r8, %rax
uitoa_end:
	ret
