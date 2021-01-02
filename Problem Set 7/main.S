// Tamar Bacalu & Mark Koszykowski
// ECE357 - Operating Systems Problem Set 7, 2B

.data
	msg: .ascii "hello world\n"
	len = .-msg

.text
	.global _start

_start:
	mov $1, %rdi
	mov $msg, %rsi
	mov $len, %rdx
	mov $1, %rax
	syscall
	mov %rax, %rdi
	mov $60, %rax
	syscall
