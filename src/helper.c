#include "helper.h"

// redundant since XCHAR is literally a typedef of wchar_t but whatevs
_Static_assert(sizeof(XCHAR) == sizeof(wchar_t), "XCHAR must match wchar_t size");

// Set Pascal string max length here:
#define MAX_STR_LEN 32768

// Max Pascal string length, INCLUDING first len character. 64kb is fine.
// Access only through helper
static XCHAR buf[MAX_STR_LEN];

// XLOPER12 buffer, just needed to pass strings to Excel
// Access only through helper
static XLOPER12 xlOperBuf;

// Copies over a null-terminated wide string and makes it Excel-Pascal friendly in a global buffer
// NOT THREAD SAFE!! Buffer gets clobbered on every call to MakePascal.
// Silently truncates at strings over 32,767 characters. Do I care? No, no I do not.
static XCHAR *MakePascal(const wchar_t* str) {

    size_t len = wcslen(str);
    // Cap max length
    len = (len > MAX_STR_LEN - 1) ? MAX_STR_LEN - 1 : len;

    buf[0] = (XCHAR)len;
    memcpy(&buf[1], str, len * sizeof(XCHAR));

    return buf;
}

// Public API, NOT THREAD SAFE ------------------------------------------------------

// Double to Excel XLOPER12
LPXLOPER12 DoubleToXL(double d) {
    xlOperBuf.xltype = xltypeNum;
    xlOperBuf.val.num = d;
    return &xlOperBuf;
}

// Wide string (wchar_t*) to Excel XLOPER12
LPXLOPER12 LStrToXL(const wchar_t* str) {
    XCHAR* buf_ptr = MakePascal(str);
    xlOperBuf.xltype = xltypeStr;
    xlOperBuf.val.str = buf_ptr;
    return &xlOperBuf;
}

// Boolean or truthy/falsy value to Excel XLOPER12
LPXLOPER12 BoolToXL(BOOL b) {
    xlOperBuf.xltype = xltypeBool;
    xlOperBuf.val.xbool = (b) ? 1 : 0;
    return &xlOperBuf;
}

// Error code to Excel XLOPER12
LPXLOPER12 ErrCodeToXL(int errCode) {

    // xlerrNull         0
    // xlerrDiv0         7
    // xlerrValue        15
    // xlerrRef          23
    // xlerrName         29
    // xlerrNum          36
    // xlerrNA           42
    // xlerrGettingData  43

    xlOperBuf.xltype = xltypeErr;
    xlOperBuf.val.err = errCode;
    return &xlOperBuf;
}

// 2D Array/Range to Excel XLOPER12
LPXLOPER12 RangeToXL(int rows, int cols, LPXLOPER12 range_ptr) {
    xlOperBuf.xltype = xltypeMulti;
    xlOperBuf.val.array.rows = rows;
    xlOperBuf.val.array.columns = cols;
    xlOperBuf.val.array.lparray = range_ptr;
    return &xlOperBuf;
}