# Operating_systems

Learning course at 6th semester of BMSTU (2020)

## Lab 1
**Демон**

1. Функция Demonize -- правила программирования демонов. 
2. Журналирование ошибок.
3. Демон сущесвует в единственном экземпляре, функция контроля единственности already_running. 

*Источники литературы:*
1. Стивен Раго "Unix. Профессиональное программирование", глава 13. 

## Lab 2
**Иерархическая структура каталогов в Unix**

1. В книге описан рекурсивный обход файлов. 
2. На экран выводи дерево каталогов.
3. Включаем в программу changedir, для использования коротких имен.
4. Переписываем рекурсию в итерации (2 вида программы). 

*Источники литературы:*
1. Стивен Раго "Unix. Профессиональное программирование", глава 4. 

## Lab 3
**Загружаемые модули ядра**

1. Задание 1. Реализовать загружаемый модуль ядра, выводящий информацию о процессах. 
2. Задание 2. Реализовать три загружаемых модуля ядра:
  - Вызываемый модуль md1
  - Вызывающий модуль md2
  - «Отладочный» модуль md3

*Источники литературы:*
1. https://www.ibm.com/developerworks/ru/library/l-linux_kernel_01/index.html 

## Lab 4
**Виртуальная файловая система /proc**

Написать программу – загружаемый модуль ядра (LKM) – которая  поддерживает чтение из пространства пользователя и запись в пространство пользователя.

*Источники литературы:*
1. https://www.ibm.com/developerworks/ru/library/l-proc/index.html 

## Lab 5
**Анализ особенностей работы функций ввода-вывода в UNIX/Linux**

*Источники литературы:*
1. https://www.usna.edu/Users/cs/wcbrown/courses/IC221/classes/L09/Class.html 

## Lab 6
**Сокеты**

Задание 1

Написать приложение по модели клиент-сервер, демонстрирующее взаимодействие параллельных процессов на отдельном компьютере с использованием сокетов в файловом пространстве имен: семейство - AF_UNIX, тип - SOCK_DGRAM. 

Задание 2

Написать приложение по модели клиент-сервер, осуществляющее взаимодействие параллельных процессов, которые выполняются на разных компьютерах. Для взаимодействия с клиентами сервер должен использовать мультиплексирование. Сервер должен обслуживать запросы параллельно запущенных клиентов. 

*Источники литературы:*
1. Стивенс У., UNIX: Разработка сетевых приложений, глава 6
2. http://citforum.ru/programming/unix/sockets/

## Lab 7
**Системный вызов open()**

Построить схему выполнения системного вызова open() в зависимости от значения основных флагов определяющих открытие файла на чтение, на запись, на выполнение и на создание нового файла. В схеме должны быть названия функций и кратко указаны выполняемые ими действия. 

*Источники литературы:*
1. https://elixir.bootlin.com/linux/v5.6.3/source/fs/open.c

## Lab 8
**Создание виртуальной файловой системы**

Cоздать виртуальную файловую систему. В лабораторной работе предлагается дополнительно создать слаб кэш для inode.

*Источники литературы:*
1. https://www.opennet.ru/base/dev/virtual_fs.txt.html
2. http://rus-linux.net/MyLDP/BOOKS/Moduli-yadra-Linux/06/kern-mod-06-05.html

## Lab 9
**Обработчики прерываний**

1. Написать загружаемый модуль ядра, в котором зарегистрировать обработчик аппаратного прерывания с флагом IRQF_SHARED.
2. Инициализировать очередь работ.
3. В обработчике прерывания запланировать очередь работ на выполнение.
4. Вывести информацию об очереди работ используя, или printk(), или seq_file interface

*Источники литературы:*
1. https://www.mashin.ru/files/2016/ao1016.pdf 
