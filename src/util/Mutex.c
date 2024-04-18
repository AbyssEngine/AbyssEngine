#include "Mutex.h"

#include <stdlib.h>
#include <string.h>

struct Mutex *mutex_create(void) {
    struct Mutex *result = malloc(sizeof(struct Mutex));
    memset(result, 0, sizeof(struct Mutex));
#ifdef _WIN32
    result->mutex = CreateMutex(NULL, FALSE, NULL);
#endif
    return result;
}
void mutex_destroy(struct Mutex **mutex) {
#ifdef _WIN32
    CloseHandle((*mutex)->mutex);
#endif
    free(*mutex);
    *mutex = NULL;
}

void mutex_lock(struct Mutex *mutex) {
#if _WIN32
    WaitForSingleObject(mutex->mutex, INFINITE);
#else  // _WIN32
    pthread_mutex_lock(&mutex->mutex);
#endif // _WIN32
}

void mutex_unlock(struct Mutex *mutex) {
#if _WIN32
    ReleaseMutex(mutex->mutex);
#else  // _WIN32
    pthread_mutex_unlock(&mutex->mutex);
#endif // _WIN32
}

int mutex_try_lock(struct Mutex *mutex) {
#if _WIN32
#else  // _WIN32
    return pthread_mutex_trylock(&mutex->mutex);
#endif // _WIN32
}
