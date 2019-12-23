#include <stdio.h>
#include <windows.h>
#include <iostream>
using namespace std;

#define OK 0
#define ERROR 1

#define WRITERS 3
#define READERS 5
#define ITERS 5

int val = 0;
bool activewriter = false;
int readercount = 0;
HANDLE writers[WRITERS];
HANDLE readers[READERS];
HANDLE can_write;
HANDLE can_read;

void start_read() {
	if (true == activewriter || WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0) {
		WaitForSingleObject(can_read, INFINITE);
	}
	readercount++;
	SetEvent(can_read);
}

void stop_read() {
	readercount--;
	if (0 == readercount) {
		SetEvent(can_write);
	}
}

void start_write() {
	if (readercount > 0 || true == activewriter) {
		WaitForSingleObject(can_write, INFINITE);
	}
	activewriter = true;
}

void stop_write() {
	activewriter = false;
	ResetEvent(can_write);
	if (WaitForSingleObject(can_read, 0) == WAIT_OBJECT_0) {
		SetEvent(can_read);
	} else {
		SetEvent(can_write);
	}
}

DWORD WINAPI writer(LPVOID) {
	for (int i = 0; i < ITERS; i++) {
		start_write();
		val++;
		cout << "Writer" << GetCurrentThreadId() << " write " << val << endl;
		stop_write();
		Sleep(100);
	}
	return OK;
}

DWORD WINAPI reader(LPVOID mutex) {
	for (int i = 0; i < ITERS + 6; i++) {
		start_read();
		WaitForSingleObject(mutex, INFINITE);
		cout << "Reader" << GetCurrentThreadId() << " read " << val << endl;
		ReleaseMutex(mutex);
		stop_read();
		Sleep(100);
	}
	return OK;
}

int create_mutex_threads() {
	HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
	if (NULL == mutex) {
		cout << "Can't create mutex\n";
		return ERROR;
	}
	// создание писателей
	for (int i = 0; i < WRITERS; i++) {
		// данный аргумент определяет, может ли создаваемый поток быть унаследован дочерним процессом
		// размер стека в байтах. Если передать 0, то будет использоваться значение по - умолчанию (1 мегабайт)
		// адрес функции, которая будет выполняться потоком
		// указатель на переменную, которая будет передана в поток
		// флаги создания
		// указатель на переменную, куда будет сохранён идентификатор потока

		writers[i] = CreateThread(NULL, 0, &writer, NULL, 0, NULL);
		if (NULL == writers[i]) {
			cout << "Can't create threads\n";
			return ERROR;
		}
	}
	// создание читателей
	for (int i = 0; i < READERS; i++) {
		readers[i] = CreateThread(NULL, 0, &reader, mutex, 0, NULL);
		if (NULL == readers[i]) {
			cout << "Can't create threads\n";
			return ERROR;
		}
	}
	
	return OK;
}

int create_events() {
	// атрибут защиты
	// тип сброса TRUE - ручной
	// начальное состояние TRUE - сигнальное
	// имя обьекта

	// с автосбросом
	can_read = CreateEvent(NULL, FALSE, FALSE, TEXT("ReadEvent"));
	if (can_read == NULL) {
		cout << "Can't create event\n";
		return ERROR;
	}

	// с ручным сбросом
	can_write = CreateEvent(NULL, TRUE, FALSE, TEXT("WriteEvent"));
	if (can_write == NULL) {
		cout << "Can't create event\n";
		return ERROR;
	}
	return OK;
}

int main() {

	if (create_events() != OK) {
		return ERROR;
	}
	if (create_mutex_threads() != OK) {
		return ERROR;
	}

	WaitForMultipleObjects(WRITERS, writers, TRUE, INFINITE);
	WaitForMultipleObjects(READERS, readers, TRUE, INFINITE);

	cout << "OK";
	return OK;
}