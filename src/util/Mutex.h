#ifndef ABYSS_MUTEX_H
#define ABYSS_MUTEX_H

typedef struct Mutex Mutex;

struct Mutex *Mutex_Create(void);
void          Mutex_Destroy(Mutex **mutex);
void          Mutex_Lock(Mutex *mutex);
void          Mutex_Unlock(Mutex *mutex);

#endif // ABYSS_MUTEX_H
