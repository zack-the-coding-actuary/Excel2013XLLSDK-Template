#include <windows.h>
#include "XLCALL.H"
#include "tls.h"
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

// Max # of args for Excel12v for xlfRegister, per documentation
#define MAX_ARG_LEN 255

// Amount to allocate for per-UDF docstring registration, using 1MB as a ceiling
#define STR_ARENA_SIZE (1024 * 512 * sizeof(wchar_t))

LPXLOPER12 DoubleToXL(double d);
LPXLOPER12 LStrToXL(const wchar_t* str);
LPXLOPER12 BoolToXL(BOOL b);
LPXLOPER12 ErrCodeToXL(int errCode);
LPXLOPER12 RangeToXL(int rows, int cols);

LPXLOPER12 AllocateRegistrationBuf();
void RegisterFunc(wchar_t *metadata[], size_t len);
void FreeRegistrationBuf();