
#include "RobboLito.h"
#include <string.h>

#ifdef LINUX_LARGE_PAGES
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#ifdef LINUX_LARGE_PAGES /* acclaim to WINDOWS ? */
void SETUP_PRIVILEGES () {}
void CREATE_MEM
(void** A, int align, uint64 size, int* w, boolean* use, char* string)
{
  if (TRY_LARGE_PAGES)
    {
      *w = shmget (IPC_PRIVATE, size, IPC_CREAT | SHM_R | SHM_W | SHM_HUGETLB);
      if ((*w) == -1)
	MEMALIGN ((*A), align, size);
      else
	{
	  (*A) = shmat ((*w), NULL, 0x0);
	  (*use) = TRUE;
	  SEND ("info string %s as HUGETLB %d\n", string, size >> 20);
	}
    }
  else
    {
      (*w) = -1;
      MEMALIGN ((*A), align, size);
    }
  if ((*w) == -1)
    (*use) = FALSE;
}

void FREE_MEM (void* A, int* w, boolean* use)
{
  if (!A)
    return;
  if ((*w) == -1)
    {
      ALIGNED_FREE (A);
      return;
    }
  shmdt (A);
  shmctl ((*w), IPC_RMID, NULL);
  (*w) = -1;
  (*use) = FALSE;
}
#endif

#ifdef WINDOWS_LARGE_PAGES /* for the testings */ /* in the OK! */
/* http://msdn.microsoft.com/en-us/library/aa366720%28VS.85%29.aspx */
void CREATE_MEM
(void** A, int align, uint64 size, int* w, boolean* use, char* string)
{
  if (TRY_LARGE_PAGES)
    {
      (*A) = VirtualAlloc /* Vlad0 */
	(NULL, size, MEM_LARGE_PAGES|MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
      if ((*A)) /* HACK */
	{
	  (*use) = TRUE;
	  SEND ("info string %s LargePages size %d\n", string, size >> 20);
	}
      else
	{
	  (*use) = FALSE;
	  MEMALIGN ((*A), align, size);
	}
    }
  else
    {
      (*use) = FALSE;
      MEMALIGN ((*A), align, size);
    }
}

void FREE_MEM (void* A, int* w, boolean* use)
{
  if (!A)
    return;
  if (!(*use))
    {
      ALIGNED_FREE (A);
      return;
    }
  VirtualFree (A, 0, MEM_RELEASE);
}

void SETUP_PRIVILEGES ()
 /* http://msdn.microsoft.com/en-us/library/aa366543%28VS.85%29.aspx */
{
  HANDLE token_handle;
  TOKEN_PRIVILEGES tp;
  OpenProcessToken
    (GetCurrentProcess (), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token_handle);
  LookupPrivilegeValue (NULL, TEXT ("SeLockMemoryPrivilege"), &tp.Privileges[0].Luid);
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  AdjustTokenPrivileges (token_handle, FALSE, &tp, 0, NULL, 0);
  CloseHandle (token_handle);
}
#endif

#if !defined(LINUX_LARGE_PAGES) && !defined(WINDOWS_LARGE_PAGES)
void FREE_MEM (void* A, int* w, boolean* use) { ALIGNED_FREE (A); }
void CREATE_MEM
(void** A, int align, uint64 size, int* w, boolean* use, char* string)
{
  MEMALIGN ((*A), align, size);
}
void SETUP_PRIVILEGES () {}
#endif
