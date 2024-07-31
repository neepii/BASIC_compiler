#;rax - buffer
#;rdi - dword (a integer to convert)

newline:
	mov $10, %rax
	mov %rax, (bytestorage)
	mov $1, %rax
	mov $1, %rdi
	mov $1, %rdx
	mov $bytestorage, %rsi
	syscall
	ret

#;rax - buffer address
#;return in rax
strlen:
	mov %rax, %rcx
strlen_loop:
	xor %rbx, %rbx
	movb (%rax), %bl
	cmp $0, %rbx
	jz strlen_end
	inc %eax
	jmp strlen_loop
strlen_end:
	sub %rax, %rcx
	dec %rax
	ret


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
