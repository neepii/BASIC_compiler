# rdi - string buffer
# uses rax rsi r10
atoi:

	xor %rax, %rax
    xor %r10, %r10
	movzx (%rdi), %rsi
    cmp $45, %rsi
    jne atoi_convert
    mov $1, %r10
    inc %rdi
atoi_convert:
	movzx (%rdi), %rsi
	test %rsi, %rsi
	jz atoi_check_sign

	cmp $10, %rsi
	je atoi_check_sign

	cmp $48, %rsi
	jl atoi_error

	cmp $57, %rsi
	jg atoi_error

	sub $48, %rsi
	imul $10, %rax
	add %rsi, %rax

	inc %rdi
	jmp atoi_convert
atoi_error:
	mov $-1, %rax
atoi_check_sign:
    cmp $1, %r10
    jne atoi_ret
    xor $0xffffffff, %eax
    inc %eax
atoi_ret:
	ret
	

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
# uses rbx and rcx
strlen:
	mov %rax, %rcx
strlen_loop:
	xor %rbx, %rbx
	movb (%rax), %bl
	cmp $0, %rbx
	jz strlen_end
	cmp $10, %rbx
	je strlen_end
	inc %eax
	jmp strlen_loop
strlen_end:
	sub %rcx, %rax
	ret

#;rax - buffer
#;rdi - dword (a integer to convert)
#uses rsi rdi r8 r9 rcx rdx r10
itoa:

	movq %rax, %rsi

	movl %edi, %eax

	cmp $0, %rax
 	jnz itoa_convert_reg
	movb $48, (%rsi)
	inc %esi
	movb $0, (%rsi)
	mov $1, %rax
	jmp itoa_end
	
itoa_convert_reg:
    xor %rcx, %rcx
    xor %r10, %r10
    mov $10, %r9
    
    cmp $0x7fffffff, %rax
    jle itoa_loop
    xor $0xffffffff, %edi
    inc %edi
    mov %edi, %eax
    mov $1, %r10

itoa_loop:
	xor %rdx, %rdx
	div %r9
	inc %ecx
	cmp $0, %rax
	jnz itoa_loop

	inc %ecx

	mov %rcx, %r8
	add %rcx, %rsi

	movb $0,(%rsi)
	movl %edi, %eax
	dec %ecx
itoa_convert:
	xor %rdx, %rdx
	dec %rsi
	div %r9
	add $48, %rdx
	movb %dl,(%rsi)
	loopnz itoa_convert
	mov %r8, %rax
    cmp $0, %r10
    je itoa_end
    dec %rsi
    movb $45, (%rsi) 
itoa_end:
	ret
