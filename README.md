# Operating_systems

Learning course at 5th semester of BMSTU (2019)

## Lab 1
**Дизассемблирование INT 8h**

1. Получение начального адреса прерывания Int 8h. 
2. Используя sourser получить дизассемблерный код обработчика аппаратного прерывания от системного таймера Int 8h.
3. На основе полученного кода составить алгоритм работы обработчика Int 8h. 

**Функции обработчика прерываний от системного таймера**

1. Перечисление и исследование. 
2. Указать функции, которые выполняются по тику, главному тику и кванту. 
3. Отдельно для Windows, Unix/Linux. 
4. Исследование вопроса подсчета динамических приоритетов (только для приложений (пользовательских процессов))

*Источники литературы:*
1. Руссинович, Соломон "Внутренне устройство Microsoft Windows"
2. Вахалия "Unix изнутри"

## Lab 2
**Защищенный режим**

1. Организовать преход в защищенный и реальный режимы. 
2. Работа с клавиатурой и таймером - прерывания. 
3. Определить объем доступного адресного пространства. 

*Источники литературы:*
1. Рудаков, Финогенов "Прогарммируем на языке ассемблера"
2. Зубков "Ассемблер для Dos, Windows и Unix"

## Lab 3
**1 лабораторная по линуксу**

1. Напишите программу, в которой создается дочерний процесс и организуйте как в предке, так и в потомке бесконечные циклы. 
2. Запустите программу и посмотрите идентификаторы созданных процессов: предка и потомка;
3. Для получения процесса зомби (отобраны все ресурсы кроме последнего, строки в таблице процессов, необходимо чтобы предок мог получить статус завершения своего потомка) выполните следующие действия: 
  - удалите командой kill потомка и посмотрите с помощью команды ps его новый статус – Z;
  - удалите предка.
4. Для получения «осиротевшего» процесса запустите программу еще раз, но в этот раз удалите предка и посмотрите с помощью команды ps идентификатор предка у продолжающего выполняться потомка – идентификатор предка будет изменен на 1, так как процесс был «усыновлен» процессом с идентификатором 1 , процессом «открывшим» терминал.
5. Создать soft link(ссылка на файл, а именно это специальный файл, в котором только одна строка содержащая путь по которому нужно перейти при обращении к этой ссылке), hard link (еще одно имя файла). 
6. Изменить приоритет. renice -n 5 -p process

Можем только увеличить приоритет (без sudo - суперпользователя (доступны привелигированные команды и доступ к структуре ядра). Нельзя уменьшить, так как для этого требуется пересчет приоритетов. 

7. Именованный программный канал - специальный файл, в который можно писать информация и из которого можно читать. 
  -	создаем именованный программный канал командой mknod с именем pipe;
  -	направляем  текст в программный канал: echo [текст]  >  pipe
  -	меняем консоль;
  -	получаем через канал текст и используя команду tee, выводим на экран: tee < pipe  

Значения флагов:

- 0 - был fork и был exec (родительский процесс с флагом 0, так как была вызвана была вызвана команда запуска программы, она не принадлежит языку bash, поэтому создается процесс, котором выполняется сначала fork, потом exec)
- 1 - был fork но не было exec 
- 4 - суперюзер

Типы файлов:
- -- обычный
- d директория
- p именованный программный канал
- l soft link
- s socket
- c специальный файл символьного устройства (символьный файл)
- b специальный файл блочного устройства (блочный файл)
