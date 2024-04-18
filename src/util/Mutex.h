#ifndef ABYSS_MUTEX_H
#define ABYSS_MUTEX_H

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

struct Mutex *mutex_create(void);
void          mutex_destroy(struct Mutex **mutex);
void          mutex_lock(struct Mutex *mutex);
void          mutex_unlock(struct Mutex *mutex);
int           mutex_try_lock(struct Mutex *mutex);

#endif // ABYSS_MUTEX_H
