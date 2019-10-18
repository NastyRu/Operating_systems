.386p ; Разрешение трансляции всех, в том
			; числе привилегированных команд МП 386
			; и 486

descr struc  ; Cтруктура для описания дескриптора сегмента
		limit 	dw 0	; Граница (биты 0..15)
		base_l 	dw 0	; База, биты 0..15
		base_m 	db 0	; База, биты 16..23
		attr_1 	db 0	; Байт атрибутов 1
		arrt_2 	db 0	; Граница(биты 16..19) и атрибуты 2
		base_h 	db 0	; База, биты 24..31
descr ends

intr struc ; Структура для описания дескрипторов прерываний
	  offs_l 	dw 0  ; Смещение обработчика, нижняя часть (биты 0..15)
	  sel	    dw 0	; Селектор сегмента команд
	  rsrv    db 0  ; Зарезервировано
	  attr	  db 0  ; Атрибуты
	  offs_h 	dw 0  ; Смещение обработчика, верхняя часть (биты 16..31)
intr ends

; Защищенный режим
PM_seg segment para public 'code' use32
		assume CS:PM_seg

		GDT	label	byte ; Таблица глобальных дескрипторов
		gdt_null descr <> ; Нулевой дескриптор
		gdt_flatDS descr <0FFFFh,0,0,92h,0CFh,0> ; 32-битный 4-гигабайтный сегмент с базой = 0
		gdt_16bitCS	descr <RM_seg_size-1,0,0,98h,0,0> ; 16-битный 64-килобайтный сегмент кода с базой RM_seg
		gdt_32bitCS	descr <PM_seg_size-1,0,0,98h,0CFh,0> ; 32-битный 4-гигабайтный сегмент кода с базой PM_seg
		gdt_32bitDS	descr <PM_seg_size-1,0,0,92h,0CFh,0> ; 32-битный 4-гигабайтный сегмент данных с базой PM_seg
		gdt_32bitSS	descr <stack_l-1,0,0,92h,0CFh,0> ; 32-битный 4-гигабайтный сегмент данных с базой stack_seg
		gdt_size = $-GDT ; размер нашей таблицы GDT+1байт (на саму метку)

		gdtr dw gdt_size-1 ; Лимит GDT
				 dd ? ; Линейный адрес GDT

		; Имена для селекторов
		SEL_flatDS equ 8
		SEL_16bitCS equ 16
		SEL_32bitCS equ 24
		SEL_32bitDS equ 32
		SEL_32bitSS equ 40

		IDT	label	byte ; Таблица дескрипторов прерываний IDT
		intr 32 dup (<0, SEL_32bitCS,0, 8Eh, 0>) ; Первые 32 элемента таблицы (в программе не используются)
		int08 intr <0, SEL_32bitCS,0, 8Eh, 0> ; Дескриптор прерывания от таймера
		int09 intr <0, SEL_32bitCS,0, 8Eh, 0> ; Дескриптор прерывания от клавиатуры
		idt_size = $-IDT ; Размер IDT

		idtr dw idt_size-1 ; Лимит IDT
				 dd ? ; Линейный адрес IDT

		idtr_real dw 3FFh,0,0 ; содержимое регистра IDTR в реальном режиме

		msgprotected db 'Protected mode$'
		msgreal db 'Real mode$'
		msgtime db 'Timer:$'
		msgmemory db 'Memory:$'

		; Таблица символов ASCII для перевода из скан кода в код ASCII.
		; Номер скан кода = номеру соответствующего элемента в таблице:
		scan2ascii db	0,1Bh,'1','2','3','4','5','6','7','8','9','0','-','=',8
							 db ' ','q','w','e','r','t','y','u','i','o','p','[',']','$'
							 db ' ','a','s','d','f','g','h','j','k','l',';','""',0
							 db '\','z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0,0
							 db 0,0,0,0,0,0,0,0,0,0,0,0

		screen_addr	dd 1E0h ; Позиция печати вводимого текста
		time_08 dd 0 ; Счетчик прошедших тиков таймера

		master db 0 ; Маска прерываний ведущего контроллера
    slave	db 0 ; Ведомого

	; Макрос печати строки
	print_string macro msg
	local screen
	screen:
		mov al,word ptr msg[si] ; Символ для вывода со смещением SI
		mov es:[ebp],al ; Вывод в видеобуфер
		add ebp,2 ; Смещаемся в видеобуфере
		inc si ; Следующий символ строки
		loop screen ; Цикл вывода на экран
	endm

	; Макрос печати числа
	print_number macro
	local cycle, number, print
	cycle:
		mov dl,al	; Кладём в DL текущее значение AL (самый младший байт ЕАХ)
		and dl,0Fh ; Оставляем от него одно 16ричное число (последняя цифра, последние 4 бита)
		cmp dl,10
		jl number
		sub dl, 10 ; Превращаем число в букву
		add dl,'A'
		jmp print
	number:
		add dl,'0' ; Превращаем это цифру в символ
	print:
		mov es:[ebp],dl	; Выводим в видеобуфер
		ror eax,4	; Циклически двигаем биты в ЕАХ - таким образом, после всех перестановок,
							; ЕАХ окажется тем же что и в начале, нет необходимости на PUSH; POP
		sub ebp,2	; Смещаем позицию вывода
		loop cycle	; Цикл
	endm

	; Точка входа в 32-битный защищенный режим
	PM_entry:
		; Установить 32-х битный стек и другие регистры (загруженные в дескрипторы)
		mov	ax,SEL_flatDS
		mov	ds,ax
		mov	es,ax
		mov	ax,SEL_32bitSS
		mov	ebx,stack_l
		mov	ss,ax
		mov	esp,ebx

		; Разрешить прерывания
		sti

		; Protected mode - вывод на экран
		push ebp
		xor eax,eax
		mov ebp,0
		add ebp,0B8000h ; Начальное смещение на экране
		mov ecx,14 ; Число выводимых символов
		mov si,0 ; Итератор (смещение от начала)
		print_string msgprotected
		pop ebp

		; Timer - вывод на экран
		push ebp
		xor eax,eax
		mov ebp,160 ; Одна строка 160 символов
		add ebp,0B8000h ; Начальное смещение на экране
		mov ecx,6 ; Число выводимых символов
		mov si,0 ; Итератор (смещение от начала)
		print_string msgtime
		pop ebp

		; Memory - вывод на экран
		push ebp
		xor eax,eax
		mov ebp,1600 ; Одна строка 160 символов
		add ebp,0B8000h ; Начальное смещение на экране
		mov ecx,7 ; Число выводимых символов
		mov si,0 ; Итератор (смещение от начала)
		print_string msgmemory
		pop ebp

		; Cчитаем количество доступной памяти и печатаем его на экран
		call compute_memory

		; Rрутимся в бесконечном цикле, периодически натыкаясь на прерывания клавиатуры и таймера
		; Выход из цикла - по нажатию ESC
		jmp short $

	dummy_exc proc
		iret
	dummy_exc endp

	; Обработчик прерывания таймера
	int08_handler:
		push eax ; Это аппаратное прерывание, сохраняем регистры
		push ebp
		push ecx
		push dx
		mov eax,time_08

		mov ebp,190 ; Ещё 30h - для слова timer и поскольку число печатается справа-налево
		mov ecx,8	; Число выводимых символов
		add ebp,0B8000h ; Начальное смещение на экране
										; 0B8000h - смещение видеобуффера относительно начала сегмента.
										cycle1:
											mov dl,al	; Кладём в DL текущее значение AL (самый младший байт ЕАХ)
											and dl,0Fh ; Оставляем от него одно 16ричное число (последняя цифра)
											cmp dl,10
											jl number1
											sub dl, 10 ; Превращаем число в букву
											add dl,'A'
											jmp print1
										number1:
											add dl,'0' ; Превращаем это цифру в символ
										print1:
											mov es:[ebp],dl	; Выводим в видеобуфер
											ror eax,4	; Циклически двигаем биты в ЕАХ - таким образом, после всех перестановок,
																; ЕАХ окажется тем же что и в начале, нет необходимости на PUSH; POP
											sub ebp,2	; Смещаем позицию вывода
											loop cycle1	; Цикл
		; Инкремент
		inc eax
		mov time_08,eax

		; Посылаем сигнал EOI контроллеру прерываний и восстанавливаем регистры
		mov	al,20h
		out	20h,al
		pop dx
		pop ecx
		pop ebp
		pop eax
		; Выходим из прерывания
		iretd

	; Обработчик прерывания клавиатуры
	int09_handler:
		push eax ; Это аппаратное прерывание, сохраняем регистры
		push ebx
		push es
		push ds

		in	al,60h ; Прочитать скан-код нажатой клавиши из порта клавиатуры
		cmp	al,01h ; Сравниваем с кодом ESC
		je esc_pressed ; Если ESC, выходим в реальный режим
		cmp al,39h ; Сравним какой скан-код пришел: обслуживаемой клавиши или нет?
		ja skip_translate ; Если нет, то ничего не выводим
		mov bx,SEL_32bitDS ; Иначе
		mov ds,bx ; DS:EBX - таблица для перевода скан-кода
		mov ebx,offset scan2ascii ; в ASCII
		xlatb ; Преобразовать
		mov bx,SEL_flatDS
		mov es,bx ; ES:EBX - адрес текущей
		mov ebx,screen_addr ; позиции на экране
		cmp al,8 ; Если не была нажата Backspace
		je bs_pressed
		mov es:[ebx+0B8000h],al ; Вывести символ на экран
		add dword ptr screen_addr,2 ; Увеличить адрес позиции на 2
		jmp short skip_translate
	bs_pressed: ; Иначе
		mov al,' ' ; нарисовать пробел
		sub ebx,2 ; в позиции предыдущего символа
		mov es:[ebx+0B8000h],al ; Вывести символ на экран
		mov screen_addr,ebx ; и сохранить адрес предыдущего символа как текущий
	skip_translate:
		; Разрешить работу клавиатуры
		in	al,61h
		or	al,80h
		out	61h,al
		; Посылаем сигнал EOI контроллеру прерываний
		mov	al,20h
		out	20h,al
		; Восстановить регистры и выйти
		pop ds
		pop es
		pop ebx
		pop	eax
		iretd

	; Сюда передается управление из обработчика int09h при нажатии ESC
	esc_pressed:
		; Разрешить работу клавиатуры, послать EOI и восстановить регистры.
		in	al,61h
		or	al,80h
		out	61h,al
		mov	al,20h
		out	20h,al
		pop ds
		pop es
		pop ebx
		pop	eax
		; Запрет прерываний
		cli

		; Возврат в реальный режим
		db	0EAh
		dd	offset RM_return
		dw	SEL_16bitCS

;функция подсчета доступной памяти
	compute_memory proc
		push ds ; Сохранение регистров
		mov	ax,SEL_flatDS ; Кладем в него сегмент на 4 ГБ - все доступное виртуальное адресное пространство
		mov	ds,ax
		mov	ebx,100001h	; Пропускаем первый мегабайт сегмента (потому, что в противном случае может произойти
																													; попытка редактирования процедуры собственного кода)
		mov	dl,11111111b ; Проверочный байт

		mov	ecx,0FFEFFFFFh ; В ECX кладём количество оставшейся памяти - чтобы не было переполнения
											 ; лимит в 4 Гб = 4096 Мб, без одного Мб = 4293918719 байт

		; В цикле считаем память
	check:
		mov	dh,ds:[ebx]	; Сохраняем в DH текущее значение по некоторому байту памяти
		mov	ds:[ebx],dl ; Кладём некоторое значение в этот байт
		cmp	ds:[ebx],dl ; Проверяем - считается обратно то же DL, если свопали, то перед нами памяти
		jnz	end_of_memory	; если не совпали - то мы достигли конца памяти, выходим из цикла
		mov	ds:[ebx], dh ; если не достигли - кладём обратно сохранённое значение
		inc	ebx	; Проверяем следующий байт и тд (размер памяти можно задать в настройках DOSBOX по умолчанию 16 Мб)
		loop check

	end_of_memory: ; Достигли конца памяти
		pop	ds ; Восстанавливаем регистры
		xor	edx,edx
		mov	eax,ebx ; В EBX лежит количество посчитанной памяти в байтах; кладём его в EAX,
		mov	ebx,100000h ; делим на 1 Мб, чтобы получить результат в Мб
		div	ebx

		push ecx ; Сохранение регистров
		push dx
		push ebp

		mov ebp,1630 ; Указываем смещение в видеопамяти относительно начала экрана
								 ; (30h на memory и поскольку число печатается справа-налево)
		mov ecx,8	; Число выводимых символов
		add ebp,0B8000h ; Начальное смещение на экране
		                ; 0B8000h - смещение видеобуффера относительно начала сегмента
										cycle2:
											mov dl,al	; Кладём в DL текущее значение AL (самый младший байт ЕАХ)
											and dl,0Fh ; Оставляем от него одно 16ричное число (последняя цифра)
											cmp dl,10
											jl number2
											sub dl, 10 ; Превращаем число в букву
											add dl,'A'
											jmp print2
										number2:
											add dl,'0' ; Превращаем это цифру в символ
										print2:
											mov es:[ebp],dl	; Выводим в видеобуфер
											ror eax,4	; Циклически двигаем биты в ЕАХ - таким образом, после всех перестановок,
																; ЕАХ окажется тем же что и в начале, нет необходимости на PUSH; POP
											sub ebp,2	; Смещаем позицию вывода
											loop cycle2	; Цикл
		sub ebp,0B8000h
		pop ebp ; Восстанавливаем регистры
		pop dx
		pop ecx

		ret
	compute_memory	endp

	PM_seg_size = $-GDT
PM_seg ENDS


RM_seg segment para public 'CODE' use16
		assume cs:RM_seg, ds:PM_seg, ss:stack_seg
	start:
		; Очистить экран
		mov	ax,3
		int	10h
		; Подготовить сегментные регистры
		push PM_seg
		pop ds

		; Вычислить базы для всех используемых дескрипторов сегментов
		xor	eax,eax
		mov	ax,RM_seg
		shl	eax,4	; Сегменты объявлены как PARA, нужно сдвинуть на 4 бита для выравнивания по границе параграфа
		mov	word ptr gdt_16bitCS+2,ax ; Базой 16bitCS будет RM_seg (base_l)
		shr	eax,16
		mov	byte ptr gdt_16bitCS+4,al ; (base_m)
		mov	ax,PM_seg
		shl	eax,4
		push eax ; Для вычисления адреса idt
		push eax ; Для вычисления адреса gdt
		mov	word ptr GDT_32bitCS+2,ax ; Базой всех 32bit будет (base_l)
		mov	word ptr GDT_32bitSS+2,ax ; (base_l)
		mov	word ptr GDT_32bitDS+2,ax ; (base_l)
		shr	eax,16
		mov	byte ptr GDT_32bitCS+4,al ; (base_m)
		mov	byte ptr GDT_32bitSS+4,al ; (base_m)
		mov	byte ptr GDT_32bitDS+4,al ; (base_m)

		; Вычислим линейный адрес GDT
		pop eax
		add	eax,offset GDT ; В eax будет полный линейный адрес GDT (адрес сегмента + смещение GDT относительно него)
		mov	dword ptr gdtr+2,eax ; Кладём полный линейный адрес в младшие 4 байта переменной gdtr
		mov word ptr gdtr, gdt_size-1 ; В старшие 2 байта заносим размер gdt, из-за определения gdt_size (через $)
																  ; настоящий размер на 1 байт меньше
		; загрузим GDT
		lgdt fword ptr gdtr

		; Аналогично вычислим линейный адрес IDT
		pop	eax
		add	eax,offset IDT
		mov	dword ptr idtr+2,eax
		mov word ptr idtr,idt_size-1

		; Заполним смещение в дескрипторах прерываний
		mov	eax,offset int08_handler ; Прерывание системного таймера
		mov	int08.offs_l,ax
		shr	eax,16
		mov	int08.offs_h,ax
		mov	eax,offset int09_handler ; Прерывание клавиатуры
		mov	int09.offs_l,ax
		shr	eax,16
		mov	int09.offs_h,ax

		; Cохраним маски прерываний контроллеров
		in al,21h ; ведущего, 21h - номер шины, in на неё даст нам набор масок (флагов)
		mov	master,al ; сохраняем в переменной master
		in al,0A1h ; ведомого - аналогично, in даёт набор масок для ведомого
		mov	slave,al

		; Перепрограммируем ведущий контроллер прерываний
		; Базовый теперь 32=20h
		mov dx,20h ; Порт контроллера
		mov	al,11h ; Командное слово инициализации ведущего контроллера (так как 2 контроллера прерываний)
		out	dx,al	; Первое слово в первый порт - 20h, остальные слова во 2
		inc dx ; Последующие слова инициализации контроллера записываются в порт 21h
		mov	al,20h ; 2: Базовый вектор (начальное смещение для обработчика) теперь 32 (20h) (вслед за векторами исключений)
		out	dx,al	; Указываем, что аппаратные прерывания будут обрабатываться начиная с 32го (20h)
		mov	al,4 ; 3: Определение номера входа, к которому подключен ведомый контроллер(к уровню 2, установить 2 бит)
		out	dx,al
		mov	al,1 ; 4: Указываем, что нужно будет посылать команду завершения обработчика прерывания (EOI)
		out	dx, al
		; Запретим все прерывания в ведущем контроллере, кроме IRQ0 (таймер) и IRQ1(клавиатура)
		mov	al,0FCh ; Маска прерываний 11111100
		out	dx,al

		; Запретим все прерывания в ведомом контроллере
		; В противном случае возникнет исключение - может прийти прерывание
		; для которого у нас не написан обработчик
		mov dx,0A1h
		mov	al,0FFh
		out	dx,al

		; Загрузим IDT
		lidt fword ptr idtr

		; Если мы собираемся работать с 32-битной памятью, стоит открыть A20
		; А20 - адресная линия ("шина")
		; через которую осуществляется доступ ко всей памяти за пределами 1 Мб
		; если не открыть то память битая
		in al,92h ; Установка бита 2 в порт А20
		or al,2
		out 92h,al

		; Отключить маскируемые прерывания
		cli
		; и немаскируемые прерывания
		in al,70h
		or al,80h
		out	70h,al

		; Перейти в непосредственно защищенный режим установкой соответствующего бита регистра cr0
		mov	eax,cr0
		or al,1
		mov	cr0,eax

		; Загрузить SEL_32bitCS в регистр CS
		db 66h
		db 0EAh
		dd offset PM_entry
		dw SEL_32bitCS
		; Начиная с этой строчки, будет выполняться код защищенного режима PM_entry

	RM_return:
		; Переход в реальный режим
		mov	eax,cr0
		and	al,0FEh ; сбрасываем флаг защищенного режима
		mov	cr0,eax

		; Сбросить очередь и загрузить CS реальным числом
		db 0EAh
		dw $+4
		dw RM_seg

		; Восстановить регистры для работы в реальном режиме
		mov	ax,PM_seg	; Загружаем в сегментные регистры реальные смещения
		mov	ds,ax
		mov	es,ax
		mov	ax,stack_seg
		mov	bx,stack_l
		mov	ss,ax
		mov	sp,bx

		; Восстанавливаем маски контроллеров прерываний
		; реинициализацию контроллера прерываний проводить не требуется
		mov	al,master
		out	21h,al
		mov	al,slave
		out	0A1h,al

		; Загружаем таблицу дескрипторов прерываний реального режима
		lidt fword ptr idtr_real

		; Разрешаем немаскируемые прерывания
		in al,70h
		and	al,07FH
		out 70h,al

	  ; и маскируемые
		sti

		; Очистить экран
		mov	ax,3
		int	10h

		; Печать сообщения о выходе из защищенного
		mov ah, 09h
		mov edx, offset msgreal
		int 21h

		; Завершаем программу через int 21h по команде 4Ch
		mov	ah,4Ch
		int	21h
	RM_seg_size = $-start ;завершаем сегмент, указываем метку начала для сегмента
RM_seg ENDS

stack_seg	segment para stack 'stack'
		stack_start	db 100h dup(?)
		stack_l = $-stack_start	; длина стека для инициализации ESP
stack_seg ENDS

END start
