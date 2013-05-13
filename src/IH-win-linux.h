
#ifdef WINDOWS
#include <windows.h>
#pragma warning(disable : 4005)
// macro redefinition
#pragma warning(disable : 4018)
// signed/unsigned mismatch
#pragma warning(disable : 4090)
// different 'const' qualifiers
#pragma warning(disable : 4098)
// 'void' function returning a value
#pragma warning(disable : 4101)
// unreferenced local variable
#pragma warning(disable : 4102)
// unreferenced label
#pragma warning(disable : 4113)
// 'int (__cdecl *)()' differs in parameter list
#pragma warning(disable : 4146)
// unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable : 4244)
// conversion from 'int' to 'short', possible loss of data
#pragma warning(disable : 4267)
//'initializing' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4311)
// pointer truncation from 'void *' to 'DWORD'
#pragma warning(disable : 4334)
// '<<' : result of 32-bit shift implicitly converted to 64 bits
#pragma warning(disable : 4761)
// integral size mismatch in argument; conversion supplied
#pragma warning(disable : 4996)
// 'sscanf': This function or variable may be unsafe
#define NOME_WINDOWS
/* typedef ? */
#define sint8 __int8
#define sint16 __int16
#define sint32 __int32
#define sint64 __int64
#define uint8 unsigned __int8
#define uint16 unsigned __int16
#define uint32 unsigned __int32
#define uint64 unsigned __int64
#define INLINE _inline
#define atoll _atoi64
#define TYPE_64_BIT "%I64d"
#if 1
#define MEMALIGN(a, b, c) a = _aligned_malloc (c, b)
#define ALIGNED_FREE(x) _aligned_free (x);
#else
#define MEMALIGN(a, b, c) a = malloc (c)
#define ALIGNED_FREE(x) free (x);
#endif
#define __builtin_prefetch /* */
#ifdef WINDOWS_X64 /* 64 bits */
#include "win64bits.h"
#else
#include "win32bits.h"
#endif
#define MUTEX_TYPE CRITICAL_SECTION
#define COND_TYPE HANDLE
#define LOCK(x) EnterCriticalSection (x)
#define UNLOCK(x) LeaveCriticalSection (x)
#define WAIT_CON_LOCK(x, y) WaitForSingleObject (x, INFINITE)
#define SIGNAL_CON_LOCK(x, y) SetEvent (x)
#define LOCK_INIT(x) InitializeCriticalSection (x)
#define LOCK_DESTROY DeleteCriticalSection
#define COND_INIT(x, y) (x) = CreateEvent (0, FALSE, FALSE, 0)
#if 0
#define TRY_LOCK(x)  WaitForSingleObject (x, 0) /* HACK */
#else /* LeonardoVinci */
#define TRY_LOCK(x)  (!TryEnterCriticalSection (x)) /* HACK */
#endif
#define PTHREAD_CREATE(N, b, thr, d) \
  (*(N)) = CreateThread (NULL, 0, thr, (LPVOID) (d), 0, NULL);
#define PTHREAD_JOIN(x) \
  { DWORD w; do { GetExitCodeThread (x, &w); } while (w == STILL_ACTIVE); }
HANDLE PTHREAD[MAX_CPUS], PTHREAD_IO;
#define IVAN_THREAD(A) DWORD WINAPI ivan_thread (LPVOID A)
#define IO_THREAD(A) DWORD WINAPI io_thread (LPVOID A)
#define VOID_STAR_TYPE DWORD
#define DL_ERROR GetLastError
#define DL_SYM (int (__cdecl *) (void)) GetProcAddress
#define DL_OPEN(x, y) LoadLibrary (x)
#define DllExport __declspec (dllexport)
#define LIB_NAME0 "./RobboBaseLib.dll"
#define LIB_NAME1 "../RobboBaseLib.dll"

#else /* LINUX */

#define sint8 signed char
#define sint16 signed short int
#define sint32 int
#define sint64 long long int
#define uint8 unsigned char
#define uint16 unsigned short int
#define uint32 unsigned int
#define uint64 unsigned long long int
#define INLINE inline
#define TYPE_64_BIT "%lld"
#define MEMALIGN(a, b, c) posix_memalign ((void*) &(a), b, c)
#define ALIGNED_FREE(x) free (x);
#include "bits.h"
#include <pthread.h> /* for what when no SMP ? */
#define MUTEX_TYPE pthread_mutex_t
#define COND_TYPE pthread_cond_t
#define LOCK(x) pthread_mutex_lock (x)
#define UNLOCK(x) pthread_mutex_unlock (x)
#define WAIT(x, y) pthread_cond_wait (x, y)
#define SIGNAL(x) pthread_cond_signal (x)
#define WAIT_CON_LOCK(x, y) { LOCK (&(y)); WAIT (&(x), &(y)); UNLOCK (&(y)); }
#define SIGNAL_CON_LOCK(x, y) { LOCK (&(y)); SIGNAL (&(x)); UNLOCK (&(y)); }
#define LOCK_INIT(x) pthread_mutex_init ((x), NULL)
#define LOCK_DESTROY pthread_mutex_destroy /* new */
#define TRY_LOCK(x) pthread_mutex_trylock (x)
#define COND_INIT(x, y)	\
 { pthread_cond_init (&(x), NULL); pthread_mutex_init (&(y), NULL); }
#define PTHREAD_CREATE(N, b, thr, d) pthread_create (N, NULL, thr, (void*) (d))
#define PTHREAD_JOIN(x) pthread_join (x, NULL)
pthread_t PTHREAD[MAX_CPUS], PTHREAD_IO;
#define IVAN_THREAD(A) void* ivan_thread (void* A)
#define IO_THREAD(A) void* io_thread (void* A)
#define VOID_STAR_TYPE void*
#define LINUX_LARGE_PAGES TRUE /* NEW */ /* HACK */

#define DL_ERROR dlerror
#define DL_SYM dlsym
#define DL_OPEN(x, y) dlopen (x, y)
#define HMODULE void *
#define DllExport /* */
#define LIB_NAME0 "./RobboBaseLib.so"
#define LIB_NAME1 "../RobboBaseLib.so"

#endif
