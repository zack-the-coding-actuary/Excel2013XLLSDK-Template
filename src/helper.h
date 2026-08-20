#include <windows.h>
#include "XLCALL.H"
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

LPXLOPER12 DoubleToXL(double d);
LPXLOPER12 LStrToXL(const wchar_t* str);
LPXLOPER12 BoolToXL(BOOL b);
LPXLOPER12 ErrCodeToXL(int errCode);
LPXLOPER12 RangeToXL(int rows, int cols, LPXLOPER12 range_ptr);