# rdi - string buffer
# uses rax rsi
atoi:

	xor %rax, %rax
atoi_convert:
	movzx (%rdi), %rsi
	test %rsi, %rsi
	jz atoi_end

	cmp $10, %rsi
	je atoi_end

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
atoi_end:
	ret
	

# use cmp then this function
# return value is in the rax
bool_equal:
	je bool_true
	mov $0, %rax
	ret
bool_true:
	mov $1, %rax
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
#uses rsi rdi r8 r9 rcx rdx
uitoa:
	movq %rax, %rsi

	movl %edi, %eax
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
	movl %edi, %eax
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
