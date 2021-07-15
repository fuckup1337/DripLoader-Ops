.code

 ; by Filip Olszak (@_lpvoid)
 ; based on KiServiceTable from Windows 10.0.18363+

bye:
	ret

		ANtAVM proc
			mov r8, r10
			mov r10, 02h
			xor r10, r10
			mov r10, 0Bh        
			mov r10, rcx
			xor eax, eax
			sub r8, r10
				add eax, 18h		; 1507+
			xor r8, r8
			syscall
			ret
		ANtAVM endp


		ANtWVM proc
			add rcx, 0Bh
			xor eax, eax
			mov r10, rcx
				add eax, 3Ah		; 1507+
			sub r10, 0Bh
			sub rcx, 0Bh
			syscall
			ret
		ANtWVM endp


		ANtPVM proc
			add r10, 1Ah
			xor eax, eax
			mov r10, rcx
			sub r10, 02h
				add eax, 50h		; 1507+
			add r10, 02h
			syscall
			ret
		ANtPVM endp


		ANtCTE proc
			mov r12, rcx
			mov r13, rdx
			mov r14, r8
			mov r15, r9

			mov r10, rcx
			xor rax, rax
				add eax, 0C1h		; 2004, 20H2
			syscall
			cmp rax, 00
			je bye

			mov rcx, r12
			mov rdx, r13
			mov r8, r14
			mov r9, r15

			mov r10, rcx
			xor rax, rax
				add eax, 0BDh		; 1903, 1909
			syscall
			cmp rax, 00
			je bye

			mov rcx, r12
			mov rdx, r13
			mov r8, r14
			mov r9, r15

			mov r10, rcx
			xor rax, rax
				add eax, 0BCh		; 1809
			syscall
			cmp rax, 00
			je bye
		ANtCTE endp
		

		end
