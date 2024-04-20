#include "Mutex.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#else // _WIN32
#include <pthread.h>
#endif // _WINDOWS_

struct Mutex {
#ifdef _WIN32
    HANDLE mutex;
#else  // _WIN32
    pthread_mutex_t mutex;
#endif // _WINDOWS_
};

struct Mutex *Mutex_Create(void) {
    struct Mutex *result = malloc(sizeof(Mutex));
    memset(result, 0, sizeof(Mutex));
#ifdef _WIN32
    result->mutex = CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&result->mutex, NULL);
#endif
    return result;
}
void Mutex_Destroy(Mutex **mutex) {
#ifdef _WIN32
    CloseHandle((*mutex)->mutex);
#else
    pthread_mutex_destroy(&(*mutex)->mutex);
#endif
    free(*mutex);
    *mutex = NULL;
}

void Mutex_Lock(Mutex *mutex) {
#if _WIN32
    WaitForSingleObject(mutex->mutex, INFINITE);
#else  // _WIN32
    pthread_mutex_lock(&mutex->mutex);
#endif // _WIN32
}

void Mutex_Unlock(Mutex *mutex) {
#if _WIN32
    ReleaseMutex(mutex->mutex);
#else  // _WIN32
    pthread_mutex_unlock(&mutex->mutex);
#endif // _WIN32
}
