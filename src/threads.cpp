#include "threads.h"

#include <unordered_map>

static thread_id _next_thread_id = 1;
static std::unordered_map<thread_id, thread_t> _threads;

struct _thread_args {
  thread_func func;
  void *arg;
};

#ifdef UNIX
static void *_thread_driver(void *argstruct)
#else
static DWORD WINAPI _thread_driver(LPVOID argstruct)
#endif
{
  _thread_args *ta = (_thread_args*) argstruct;
  ta->func(ta->arg);
  delete ta;
  return 0;
}

thread_id create_thread(thread_func func, void *arg) {
  _thread_args *argstruct = new _thread_args;
  argstruct->func = func;
  argstruct->arg = arg;

  thread_id tid = _next_thread_id++;
  thread_t thread;

# ifdef UNIX
  pthread_create(&thread, NULL, _thread_driver, (void*) argstruct);
# else
  thread = CreateThread(NULL, 0, _thread_driver, (void*) argstruct, 0, NULL);
# endif

  _threads.insert(std::make_pair(tid, thread));

  return tid;
}

void join_thread(thread_id tid) {
  auto itr = _threads.find(tid);
  if (itr == _threads.end()) {
    return;
  }

  thread_t thread = itr->second;

# ifdef UNIX
  pthread_join(thread, NULL);
# else
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
# endif

  _threads.erase(itr);
}

void join_all_threads() {
  for (auto itr = _threads.begin(); itr != _threads.end(); ++itr) {
#   ifdef UNIX
    pthread_join(itr->second, NULL);
#   else
    WaitForSingleObject(itr->second, INFINITE);
    CloseHandle(itr->second);
#   endif
  }

  _threads.clear();
  _next_thread_id = 1;
}

bool try_join_thread(thread_id tid) {
  auto itr = _threads.find(tid);
  if (itr == _threads.end()) {
    return true;
  }

  thread_t thread = itr->second;

# ifdef UNIX
  return pthread_tryjoin_np(thread, NULL) == 0;
# else
  return WaitForSingleObject(thread, 0) == WAIT_OBJECT_0;
# endif
}

mutex_t create_mutex() {
  mutex_t mutex;

# ifdef UNIX
  mutex = new pthread_mutex_t;
  pthread_mutex_init(mutex, NULL);
# else
  mutex = CreateMutexA(NULL, FALSE, NULL);
# endif

  return mutex;
}

bool lock_mutex(mutex_t mutex) {
# ifdef UNIX
  return pthread_mutex_lock(mutex) == 0;
# else
  return WaitForSingleObject(mutex, INFINITE) != WAIT_FAILED;
# endif
}

bool unlock_mutex(mutex_t mutex) {
# ifdef UNIX
  return pthread_mutex_unlock(mutex) == 0;
# else
  return ReleaseMutex(mutex);
# endif
}

void destroy_mutex(mutex_t mutex) {
# ifdef UNIX
  pthread_mutex_destroy(mutex);
  delete mutex;
# else
  CloseHandle(mutex);
# endif
}
