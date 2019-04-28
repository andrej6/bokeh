#ifndef THREADS_H_
#define THREADS_H_

#include <cstdint>

#if defined UNIX
#include <pthread.h>
typedef pthread_t thread_t;
typedef pthread_mutex_t* mutex_t;
#elif defined WINDOWS
#include <Windows.h>
typedef HANDLE thread_t;
typedef HANDLE mutex_t;
#else
#error "Threads API unsupported on this system"
#endif

typedef void (*thread_func)(void*);
typedef uint32_t thread_id;

thread_id create_thread(thread_func func, void *arg);
void join_thread(thread_id thread);
void join_all_threads();
bool try_join_thread(thread_id thread);

mutex_t create_mutex();
bool lock_mutex(mutex_t mutex);
bool unlock_mutex(mutex_t mutex);
void destroy_mutex(mutex_t mutex);

#endif /* THREADS_H_ */
