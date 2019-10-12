.386p ; ���������� ���������� ����, � ���
			; ����� ����������������� ������ �� 386
			; � 486

descr struc  ; C�������� ��� �������� ����������� ��������
		limit 	dw 0	; ������� (���� 0..15)
		base_l 	dw 0	; ����, ���� 0..15
		base_m 	db 0	; ����, ���� 16..23
		attr_1 	db 0	; ���� ��������� 1
		arrt_2 	db 0	; �������(���� 16..19) � �������� 2
		base_h 	db 0	; ����, ���� 24..31
descr ends

intr struc ; ��������� ��� �������� ������������ ����������
	  offs_l 	dw 0  ; �������� �����������, ������ ����� (���� 0..15)
	  sel	    dw 0	; �������� �������� ������
	  rsrv    db 0  ; ���������������
	  attr	  db 0  ; ��������
	  offs_h 	dw 0  ; �������� �����������, ������� ����� (���� 16..31)
intr ends

; ���������� �����
PM_seg segment para public 'code' use32
		assume CS:PM_seg

		GDT	label	byte ; ������� ���������� ������������
		gdt_null	descr <> ; ������� ����������
		gdt_flatDS	descr <0FFFFh,0,0,92h,0CFh,0> ; 32-������ 4-����������� ������� � ����� = 0
		gdt_16bitCS	descr <RM_seg_size-1,0,0,98h,0,0> ; 16-������ 64-����������� ������� ���� � ����� RM_seg
		gdt_32bitCS	descr <PM_seg_size-1,0,0,98h,0CFh,0> ; 32-������ 4-����������� ������� ���� � ����� PM_seg
		gdt_32bitDS	descr <PM_seg_size-1,0,0,92h,0CFh,0> ; 32-������ 4-����������� ������� ������ � ����� PM_seg
		gdt_32bitSS	descr <stack_l-1,0,0, 92h, 0CFh,0> ; 32-������ 4-����������� ������� ������ � ����� stack_seg
		gdt_size = $-GDT ; ������ ����� ������� GDT+1���� (�� ���� �����)

		gdtr dw gdt_size-1 ; ����� GDT
				 dd ? ; �������� ����� GDT

		; ����� ��� ����������
		SEL_flatDS equ 8
		SEL_16bitCS equ 16
		SEL_32bitCS equ 24
		SEL_32bitDS equ 32
		SEL_32bitSS equ 40

		IDT	label	byte ; ������� ������������ ���������� IDT
		intr 32 dup (<0, SEL_32bitCS,0, 8Eh, 0>) ; ������ 32 �������� ������� (� ��������� �� ������������)
		int08 intr <0, SEL_32bitCS,0, 8Eh, 0> ; ���������� ���������� �� �������
		int09 intr <0, SEL_32bitCS,0, 8Eh, 0> ; ���������� ���������� �� ����������
		idt_size = $-IDT ; ������ IDT

		idtr dw idt_size-1 ; ����� IDT
				 dd ? ; �������� ����� IDT

		idtr_real dw 3FFh,0,0 ; ���������� �������� IDTR � �������� ������

		msgprotected db 'Protected mode$'
		msgreal db 'Real mode$'
		msgtime db 'Timer:$'
		msgmemory db 'Memory:$'

		; ������� �������� ASCII ��� �������� �� ���� ���� � ��� ASCII.
		; ����� ���� ���� = ������ ���������������� �������� � �������:
		scan2ascii db	0,1Bh,'1','2','3','4','5','6','7','8','9','0','-','=',8
							 db ' ','q','w','e','r','t','y','u','i','o','p','[',']','$'
							 db ' ','a','s','d','f','g','h','j','k','l',';','""',0
							 db '\','z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0,0
							 db 0,0,0,0,0,0,0,0,0,0,0,0

		screen_addr	dd 1E0h ; ������� ������ ��������� ������
		time_08 dd 0 ; ������� ��������� ����� �������

		master	db 0 ; ����� ���������� �������� �����������
    slave		db 0 ; ��������

	; ������ ������ ������
	print_string macro msg
	local screen
	screen:
		mov al,word ptr msg[si] ; ������ ��� ������ �� ��������� SI
		mov es:[ebp],al ; ����� � ����������
		add ebp,2 ; ��������� � �����������
		inc si ; ��������� ������ ������
		loop screen ; ���� ������ �� �����
	endm

	; ������ ������ �����
	print_number macro
	local cycle, number, print
	cycle:
		mov dl,al	; ����� � DL ������� �������� AL (����� ������� ���� ���)
		and dl,0Fh ; ��������� �� ���� ���� 16������ ����� (��������� �����)
		cmp dl,10
		jl number
		sub dl, 10 ; ���������� ����� � �����
		add dl,'A'
		jmp print
	number:
		add dl,'0' ; ���������� ��� ����� � ������
	print:
		mov es:[ebp],dl	; ������� � ����������
		ror eax,4	; ���������� ������� ���� � ��� - ����� �������, ����� ���� ������������,
							; ��� �������� ��� �� ��� � � ������, ��� ������������� �� PUSH; POP
		sub ebp,2	; ������� ������� ������
		loop cycle	; ����
	endm

	; ����� ����� � 32-������ ���������� �����
	PM_entry:
		; ���������� 32-� ������ ���� � ������ �������� (����������� � �����������)
		mov	ax,SEL_flatDS
		mov	ds,ax
		mov	es,ax
		mov	ax,SEL_32bitSS
		mov	ebx,stack_l
		mov	ss,ax
		mov	esp,ebx

		; ��������� ����������
		sti

		; Protected mode - ����� �� �����
		push ebp
		xor eax,eax
		mov ebp,0
		add ebp,0B8000h ; ��������� �������� �� ������
		mov ecx,14 ; ����� ��������� ��������
		mov si,0 ; �������� (�������� �� ������)
		print_string msgprotected
		pop ebp

		; Timer - ����� �� �����
		push ebp
		xor eax,eax
		mov ebp,160 ; ���� ������ 160 ��������
		add ebp,0B8000h ; ��������� �������� �� ������
		mov ecx,6 ; ����� ��������� ��������
		mov si,0 ; �������� (�������� �� ������)
		print_string msgtime
		pop ebp

		; Memory - ����� �� �����
		push ebp
		xor eax,eax
		mov ebp,1600 ; ���� ������ 160 ��������
		add ebp,0B8000h ; ��������� �������� �� ������
		mov ecx,7 ; ����� ��������� ��������
		mov si,0 ; �������� (�������� �� ������)
		print_string msgmemory
		pop ebp

		; C������ ���������� ��������� ������ � �������� ��� �� �����
		call	compute_memory

		; R������� � ����������� �����, ������������ ��������� �� ���������� ���������� � �������
		; ����� �� ����� - �� ������� ESC
		jmp short $

	; ���������� ���������� �������
	int08_handler:
		push eax ; ��� ���������� ����������, ��������� ��������
		push ebp
		push ecx
		push dx
		mov eax,time_08

		mov ebp,190 ; ��� 30h - ��� ����� timer � ��������� ����� ���������� ������-������
		mov ecx,8	; ����� ��������� ��������
		add ebp,0B8000h ; ��������� �������� �� ������
										; 0B8000h - �������� ������������ ������������ ������ ��������.
		print_number

		; ���������
		inc eax
		mov time_08,eax

		; �������� ������ EOI ����������� ���������� � ��������������� ��������
		mov	al,20h
		out	20h,al
		pop dx
		pop ecx
		pop ebp
		pop eax
		; ������� �� ����������
		iretd

	; ���������� ���������� ����������
	int09_handler:
		push eax ; ��� ���������� ����������, ��������� ��������
		push ebx
		push es
		push ds

		in	al,60h ; ��������� ����-��� ������� ������� �� ����� ����������
		cmp	al,01h ; ���������� � ����� ESC
		je esc_pressed ; ���� ESC, ������� � �������� �����
		cmp al,80h ; ������� ����� ����-��� ������: ������� ������� ��� �������?
		ja skip_translate ; ���� �������, �� ������ �� �������
		mov bx,SEL_32bitDS ; �����
		mov ds,bx ; DS:EBX - ������� ��� �������� ����-����
		mov ebx,offset scan2ascii ; � ASCII
		xlatb ; �������������
		mov bx,SEL_flatDS
		mov es,bx ; ES:EBX - ����� �������
		mov ebx,screen_addr ; ������� �� ������
		cmp al,8 ; ���� �� ���� ������ Backspace
		je bs_pressed
		mov es:[ebx+0B8000h],al ; ������� ������ �� �����
		add dword ptr screen_addr,2 ; ��������� ����� ������� �� 2
		jmp short skip_translate
	bs_pressed: ; �����
		mov al,' ' ; ���������� ������
		sub ebx,2 ; � ������� ����������� �������
		mov es:[ebx+0B8000h],al ; ������� ������ �� �����
		mov screen_addr,ebx ; � ��������� ����� ����������� ������� ��� �������
	skip_translate:
		; ��������� ������ ����������
		in	al,61h
		or	al,80h
		out	61h,al
		; �������� ������ EOI ����������� ����������
		mov	al,20h
		out	20h,al
		; ������������ �������� � �����
		pop ds
		pop es
		pop ebx
		pop	eax
		iretd

	; ���� ���������� ���������� �� ����������� int09h ��� ������� ESC
	esc_pressed:
		; ��������� ������ ����������, ������� EOI � ������������ ��������.
		in	al,61h
		or	al,80h
		out	61h,al
		mov	al,20h
		out	20h,al
		pop ds
		pop es
		pop ebx
		pop	eax
		; ������ ����������
		cli

		; ������� � �������� �����
		db	0EAh
		dd	offset RM_return
		dw	SEL_16bitCS

;������� �������� ��������� ������
	compute_memory proc
		push ds ; ���������� ���������
		mov	ax,SEL_flatDS ; ������ � ���� ������� �� 4 �� - ��� ��������� ����������� �������� ������������
		mov	ds,ax
		mov	ebx,100001h	; ���������� ������ �������� �������� (������, ��� � ��������� ������ ����� ���������
																													; ������� �������������� ��������� ������������ ����)
		mov	dl,11111111b ; ����������� ����

		mov	ecx,0FFEFFFFFh ; � ECX ����� ���������� ���������� ������ - ����� �� ���� ������������
											 ; ����� � 4 �� = 4096 ��, ��� ������ �� = 4293918719 ����

		; � ����� ������� ������
	check:
		mov	dh,ds:[ebx]	; ��������� � DH ������� �������� �� ���������� ����� ������
		mov	ds:[ebx],dl ; ����� ��������� �������� � ���� ����
		cmp	ds:[ebx],dl ; ��������� - ��������� ������� �� �� DL, ���� �������, �� ����� ���� ������
		jnz	end_of_memory	; ���� �� ������� - �� �� �������� ����� ������, ������� �� �����
		mov	ds:[ebx], dh ; ���� �� �������� - ����� ������� ���������� ��������
		inc	ebx	; ��������� ��������� ���� � ��� ����� (������ ������ ����� ������ � ���������� DOSBOX �� ��������� 16 ��)
		loop	check

end_of_memory: ; �������� ����� ������
		pop	ds ; ��������������� ��������
		xor	edx,edx
		mov	eax,ebx ; � EBX ����� ���������� ����������� ������ � ������; ����� ��� � EAX,
		mov	ebx,100000h ; ����� �� 1 ��, ����� �������� ��������� � ��
		div	ebx

		push ecx ; ���������� ���������
		push dx
		push ebp

		mov ebp,1630 ; ��������� �������� � ����������� ������������ ������ ������ (30h �� memory � ��������� ����� ���������� ������-������)
		mov ecx,8	; ����� ��������� ��������
		add ebp,0B8000h ; ��������� �������� �� ������
		                ; 0B8000h - �������� ������������ ������������ ������ ��������
		print_number
		sub ebp,0B8000h
		pop ebp ; ��������������� ��������
		pop dx
		pop ecx

		ret
	compute_memory	endp

	PM_seg_size = $-GDT
PM_seg ENDS


RM_seg segment para public 'CODE' use16
		assume cs:RM_seg, ds:PM_seg, ss:stack_seg
	start:
		; �������� �����
		mov	ax,3
		int	10h
		; ����������� ���������� ��������
		push PM_seg
		pop ds

		; ��������� ���� ��� ���� ������������ ������������ ���������
		xor	eax,eax
		mov	ax,RM_seg
		shl	eax,4	; �������� ��������� ��� PARA, ����� �������� �� 4 ���� ��� ������������ �� ������� ���������
		mov	word ptr gdt_16bitCS+2,ax ; ����� 16bitCS ����� RM_seg (base_l)
		shr	eax,16
		mov	byte ptr gdt_16bitCS+4,al ; (base_m)
		mov	ax,PM_seg
		shl	eax,4
		push eax ; ��� ���������� ������ idt
		push eax ; ��� ���������� ������ gdt
		mov	word ptr GDT_32bitCS+2,ax ; ����� ���� 32bit ����� (base_l)
		mov	word ptr GDT_32bitSS+2,ax ; (base_l)
		mov	word ptr GDT_32bitDS+2,ax ; (base_l)
		shr	eax,16
		mov	byte ptr GDT_32bitCS+4,al ; (base_m)
		mov	byte ptr GDT_32bitSS+4,al ; (base_m)
		mov	byte ptr GDT_32bitDS+4,al ; (base_m)

		; �������� �������� ����� GDT
		pop eax
		add	eax,offset GDT ; � eax ����� ������ �������� ����� GDT (����� �������� + �������� GDT ������������ ����)
		mov	dword ptr gdtr+2,eax	; ����� ������ �������� ����� � ������� 4 ����� ���������� gdtr
		mov word ptr gdtr, gdt_size-1; � ������� 2 ����� ������� ������ gdt, ��-�� ����������� gdt_size (����� $) ��������� ������ �� 1 ���� ������
		; �������� GDT
		lgdt fword ptr gdtr

		; ���������� �������� �������� ����� IDT
		pop	eax
		add	eax,offset IDT
		mov	dword ptr idtr+2,eax
		mov word ptr idtr,idt_size-1

		; �������� �������� � ������������ ����������
		mov	eax,offset int08_handler ; ���������� ���������� �������
		mov	int08.offs_l,ax
		shr	eax,16
		mov	int08.offs_h,ax
		mov	eax,offset int09_handler ; ���������� ����������
		mov	int09.offs_l,ax
		shr	eax,16
		mov	int09.offs_h,ax

		; C������� ����� ���������� ������������
		in al,21h ; ��������, 21h - ����� ����, in �� �� ���� ��� ����� ����� (������)
		mov	master,al ; ��������� � ���������� master
		in al,A1h ; �������� - ����������, in ��� ����� ����� ��� ��������
		mov	slave,al

		; ����������������� ������� ���������� ����������
		; ������� ������ 32=20h
		mov dx,20h ; ���� �����������
		mov	al,11h ; ��������� ����� ������������� �������� ����������� (��� ��� 2 ����������� ����������)
		out	dx,al	; ������ ����� � ������ ���� - 20h, ��������� ����� �� 2
		inc dx ; ����������� ����� ������������� ����������� ������������ � ���� 21h
		mov	al,20h ; 2: ������� ������ (��������� �������� ��� �����������) ������ 32 (20h) (����� �� ��������� ����������)
		out	dx,al	; ���������, ��� ���������� ���������� ����� �������������� ������� � 32�� (20h)
		mov	al,4 ; 3: ����������� ������ �����, � �������� ��������� ������� ����������(� ������ 2, ���������� 2 ���)
		out	dx,al
		mov	al,1 ; 4: ���������, ��� ����� ����� �������� ������� ���������� ����������� ���������� (EOI)
		out	dx, al
		; �������� ��� ���������� � ������� �����������, ����� IRQ0 (������) � IRQ1(����������)
		mov	al,0FCh ; ����� ���������� 11111100
		out	dx,al

		; �������� ��� ���������� � ������� �����������
		; � ��������� ������ ��������� ���������� - ����� ������ ����������, ��� �������� � ��� �� ������� ����������
		mov dx,0A1h
		mov	al,0FFh
		out	dx,al

		; �������� IDT
		lidt fword ptr idtr

		; ���� �� ���������� �������� � 32-������ �������, ����� ������� A20
		; �20 - �������� ����� ("����"), ����� ������� �������������� ������ �� ���� ������ �� ��������� ������� ���������
		; ���� �� ������� �� ������ �����
		in al,92h ; ��������� ���� 2 � ���� �20
		or al,2
		out 92h,al

		; ��������� ����������� ����������
		cli
		; � ������������� ����������
		in al,70h
		or al,80h
		out	70h,al

		; ������� � ��������������� ���������� ����� ���������� ���������������� ���� �������� cr0
		mov	eax,cr0
		or al,1
		mov	cr0,eax

		; ��������� SEL_32bitCS � ������� CS
		db 66h
		db 0EAh
		dd offset PM_entry
		dw SEL_32bitCS
		; ������� � ���� �������, ����� ����������� ��� ����������� ������ PM_entry

	RM_return:
		; ������� � �������� �����
		mov	eax,cr0
		and	al,0FEh ; ���������� ���� ����������� ������
		mov	cr0,eax

		; �������� ������� � ��������� CS �������� ������
		db 0EAh
		dw $+4
		dw RM_seg

		; ������������ �������� ��� ������ � �������� ������
		mov	ax,PM_seg	; ��������� � ���������� �������� �������� ��������
		mov	ds,ax
		mov	es,ax
		mov	ax,stack_seg
		mov	bx,stack_l
		mov	ss,ax
		mov	sp,bx

		; ��������������� ����� ������������ ����������, ��������������� ����������� ���������� ��������� �� ���������
		mov	al,master
		out	21h,al
		mov	al,slave
		out	0A1h,al

		; ��������� ������� ������������ ���������� ��������� ������
		lidt fword ptr idtr_real

		; ��������� ������������� ����������
		in al,70h
		and	al,07FH
		out 70h,al

	  ; � �����������
		sti

		; �������� �����
		mov	ax,3
		int	10h

		; ������ ��������� � ������ �� �����������
		mov ah, 09h
		mov edx, offset msgreal
		int 21h

		; ��������� ��������� ����� int 21h �� ������� 4Ch
		mov	ah,4Ch
		int	21h
	RM_seg_size = $-start ;��������� �������, ��������� ����� ������ ��� ��������
RM_seg ENDS

stack_seg	segment para stack 'stack'
	stack_start	db 100h dup(?)
	stack_l = $-stack_start	; ����� ����� ��� ������������� ESP
stack_seg ENDS

END start
