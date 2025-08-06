bits 64
global main
section .data
section .text
; instruction: label
main:
; end of instruction: label
; instruction: reserve
sub rsp, 0
lea rax, [rsp + 0]
push rax
; end of instruction: reserve
; instruction: dup
pop rax
push rax
push rax
; end of instruction: dup
; instruction: get_ptr
pop rax
mov rbx, [rax]
push rbx
; end of instruction: get_ptr
; instruction: swap
pop rax
pop rbx
push rax
push rbx
; end of instruction: swap
; instruction: push
push 8
; end of instruction: push
; instruction: add
pop qword rax
pop qword rbx
add rax, rbx
push rax
; end of instruction: add
; instruction: get_ptr
pop rax
mov rbx, [rax]
push rbx
; end of instruction: get_ptr
; instruction: swap
pop rax
pop rbx
push rax
push rbx
; end of instruction: swap
; instruction: push
push 0
; end of instruction: push
; instruction: label
loop:
; end of instruction: label
; instruction: reserve
sub rsp, 0
lea rax, [rsp + 0]
push rax
; end of instruction: reserve
; instruction: push
push 32
; end of instruction: push
; instruction: add
pop qword rax
pop qword rbx
add rax, rbx
push rax
; end of instruction: add
; instruction: get_ptr
pop rax
mov rbx, [rax]
push rbx
; end of instruction: get_ptr
; instruction: push
push 1
; end of instruction: push
; instruction: add
pop qword rax
pop qword rbx
add rax, rbx
push rax
; end of instruction: add
; instruction: dup
pop rax
push rax
push rax
; end of instruction: dup
; instruction: reserve
sub rsp, 0
lea rax, [rsp + 0]
push rax
; end of instruction: reserve
; instruction: push
push 16
; end of instruction: push
; instruction: add
pop qword rax
pop qword rbx
add rax, rbx
push rax
; end of instruction: add
; instruction: get_ptr
pop rax
mov rbx, [rax]
push rbx
; end of instruction: get_ptr
; instruction: eq
pop rcx
pop rax
cmp rax, rcx
sete al
movzx rax, al
push rax
; end of instruction: eq
; instruction: branch
pop rax
lea rbx, [end]
lea rcx, [loop]
cmp rax, 0
cmovz rbx,rcx
jmp rbx
; end of instruction: branch
; instruction: label
end:
; end of instruction: label
; instruction: exit
pop qword rdi
mov rax, 60
syscall
; end of instruction: exit