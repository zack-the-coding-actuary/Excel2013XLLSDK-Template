#include "helper.h"

// XLOPER12 buffer API internals ------------------------------------------------------------------

// redundant since XCHAR is literally a typedef of wchar_t but whatevs
_Static_assert(sizeof(XCHAR) == sizeof(wchar_t), "XCHAR must match wchar_t size");

// Copies over a null-terminated wide string and makes it Excel-Pascal friendly in a global buffer
// NOW THREAD SAFE!! Silently truncates at strings over STR_BUF_LEN characters. Do I care? No, no I do not.
static void MakePascal(const wchar_t* str) {

    TLS_data *tls_data = get_TLS_data();

    size_t len = wcslen(str);
    // Cap max length, ok to use full length since accounted for in struct layout
    len = (len > STR_BUF_LEN) ? STR_BUF_LEN : len;

    tls_data->buf.xlStr.len = (XCHAR)len;
    memcpy(tls_data->buf.xlStr.strBuf, str, len * sizeof(XCHAR));
}

// Registration API internals ---------------------------------------------------------------------

static LPXLOPER12 xlRegisterBuf = NULL;
static wchar_t* regStrBuf = NULL;
static size_t bump_ptr = 0;

// Public API, NOW THREAD SAFE!! ------------------------------------------------------------------

// Double to Excel XLOPER12
LPXLOPER12 DoubleToXL(double d) {
    TLS_data *tls_data = get_TLS_data();
    if (!tls_data) return Null_XLOPER12();

    tls_data->ret_val.xltype = xltypeNum;
    tls_data->ret_val.val.num = d;
    return &(tls_data->ret_val);
}

// Wide string (wchar_t*) to Excel XLOPER12
LPXLOPER12 LStrToXL(const wchar_t* str) {
    TLS_data *tls_data = get_TLS_data();
    if (!tls_data) return Null_XLOPER12();

    MakePascal(str);
    tls_data->ret_val.xltype = xltypeStr;
    tls_data->ret_val.val.str = &(tls_data->buf.xlStr.len);
    return &(tls_data->ret_val);
}

// Boolean or truthy/falsy value to Excel XLOPER12
LPXLOPER12 BoolToXL(BOOL b) {
    TLS_data *tls_data = get_TLS_data();
    if (!tls_data) return Null_XLOPER12();

    tls_data->ret_val.xltype = xltypeBool;
    tls_data->ret_val.val.xbool = (b) ? 1 : 0;
    return &(tls_data->ret_val);
}

// Error code to Excel XLOPER12
LPXLOPER12 ErrCodeToXL(int errCode) {

    // All error codes defined in xlcall.h
    // xlerrNull         0
    // xlerrDiv0         7
    // xlerrValue        15
    // xlerrRef          23
    // xlerrName         29
    // xlerrNum          36
    // xlerrNA           42
    // xlerrGettingData  43

    TLS_data *tls_data = get_TLS_data();
    if (!tls_data) return Null_XLOPER12();

    tls_data->ret_val.xltype = xltypeErr;
    tls_data->ret_val.val.err = errCode;
    return &(tls_data->ret_val);
}

// 2D Array/Range to Excel XLOPER12. Assume caller populates tls_data->xlBuf
LPXLOPER12 RangeToXL(int rows, int cols) {
    TLS_data *tls_data = get_TLS_data();
    if (!tls_data) return Null_XLOPER12();

    tls_data->ret_val.xltype = xltypeMulti;
    tls_data->ret_val.val.array.rows = rows;
    tls_data->ret_val.val.array.columns = cols;
    tls_data->ret_val.val.array.lparray = tls_data->buf.xlBuf;
    return &(tls_data->ret_val);
}

// Allocate XLOPER12 buffer for registering UDFs at program start
LPXLOPER12 AllocateRegistrationBuf() {
    xlRegisterBuf = malloc(sizeof(XLOPER12) * MAX_ARG_LEN);

    // Hopefully this will never fire, will add silent fallback to registration func
    if (!xlRegisterBuf) return xlRegisterBuf;

    // Get a MB of space for strings
    regStrBuf = malloc(STR_ARENA_SIZE);

    if (!regStrBuf) {
        free(xlRegisterBuf);
        xlRegisterBuf = NULL;
        return xlRegisterBuf;
    }

    // First entry always points to xlGetName, free at the end
    Excel12(xlGetName, &xlRegisterBuf[0], 0);

    // Second through fifth entries always strings
    for (size_t i = 1; i <= 4; ++i) {
        xlRegisterBuf[i].xltype = xltypeStr;
    }

    // Sixth entry always is xMacroType, which should always be 1 in val.w
    xlRegisterBuf[5].val.w = 1;
    xlRegisterBuf[5].xltype = xltypeInt;

    // 7th, 8th, and 9th entries are for category, keyboard shortcut, and help file. Assume not needed.
    xlRegisterBuf[6].xltype = xltypeMissing;
    xlRegisterBuf[7].xltype = xltypeMissing;
    xlRegisterBuf[8].xltype = xltypeMissing;

    // Set rest of XLOPER12 buffer to type string
    for (size_t i = 9; i < MAX_ARG_LEN; ++i) {
        xlRegisterBuf[i].xltype = xltypeStr;
    }

    return xlRegisterBuf;
}

/* EXAMPLE: wchar_t*[] to pass into registration function

wchar_t *myFunc_strs[] = {

L"my_func",                     // Name of function in C to register
L"QQ",                          // Return type, followed by argument types, see documentation under xlfRegister12
L"MYFUNC",                      // How you want the function to look in Excel
L"a, b",                        // Names of arguments, separated by commas
L"A function that is useful",   // Description of function to pop up in function wizard in Excel
L"Argument a",                  // Description of first argument
L"Argument b"                   // Description of second argument, etc.
                                // Can do up to 244 argument descriptions, dummy slot inserted at end
};
*/

void RegisterFunc(wchar_t *metadata[], size_t len) {

    // malloc fail guard, silently refuse to register
    if (!xlRegisterBuf) return;

    // We NEED at least 3 args to register func properly
    // Cannot register more than 249 total strings (func, types, excel_func, arg_names, func_desc, arg_desc...)
    if (len < 3 || len > 249) return;

    size_t i = 0;
    size_t currlen;

    // Copy string into arena, pass pointer into arena to xlRegisterBuf
    // Silently truncates strings over 32767 long
    for (; i < 3; ++i) {
        currlen = wcslen(metadata[i]);
        regStrBuf[bump_ptr] = (currlen > MAX_STR_LEN) ? MAX_STR_LEN : (wchar_t)currlen;
        wcscpy_s(&regStrBuf[bump_ptr + 1], STR_ARENA_SIZE / sizeof(wchar_t) - bump_ptr - 1, metadata[i]);
        xlRegisterBuf[i + 1].val.str = &regStrBuf[bump_ptr];
        bump_ptr += regStrBuf[bump_ptr] + 1;
    }

    // Set xArgText to empty string if not given by metadata
    if (len == 3) {
        regStrBuf[bump_ptr] = 0;
        xlRegisterBuf[4].val.str = &regStrBuf[bump_ptr];
    }
    // Otherwise, keep going and increment i to 4
    else {
        currlen = wcslen(metadata[i]);
        regStrBuf[bump_ptr] = (currlen > MAX_STR_LEN) ? MAX_STR_LEN : (wchar_t)currlen;
        wcscpy_s(&regStrBuf[bump_ptr + 1], STR_ARENA_SIZE / sizeof(wchar_t) - bump_ptr - 1, metadata[i++]);
        xlRegisterBuf[4].val.str = &regStrBuf[bump_ptr];
        bump_ptr += regStrBuf[bump_ptr] + 1;

        // We now skip 5 entries in xlRegisterBuf
        for (; i < len; ++i) {
            currlen = wcslen(metadata[i]);
            regStrBuf[bump_ptr] = (currlen > MAX_STR_LEN) ? MAX_STR_LEN : (wchar_t)currlen;
            wcscpy_s(&regStrBuf[bump_ptr + 1], STR_ARENA_SIZE / sizeof(wchar_t) - bump_ptr - 1, metadata[i]);
            xlRegisterBuf[i + 5].val.str = &regStrBuf[bump_ptr];
            bump_ptr += regStrBuf[bump_ptr] + 1;
        }

        // Set dummy empty arg, safe if set to xFunctionHelp when len == 4
        // Need to do this to avoid the "last arg docstring gets slightly truncated" niche bug
        xlRegisterBuf[len + 5].val.str = L"\x00";
    }

    int send_count = ((int)len >= 5) ? (int)len + 6 : 9;

    // Excel wants an array of pointers instead of one pointer to an array of XLOPER12 so that's super fun
    LPXLOPER12 ptr_buff[256];
    for (int j = 0; j < send_count; ++j) {
        ptr_buff[j] = &xlRegisterBuf[j];
    }

    // Register function via Excel12v callback
    Excel12v(xlfRegister, NULL, send_count, ptr_buff);

    // Reset bump pointer after sending everything to Excel
    bump_ptr = 0;
    return;
}

void FreeRegistrationBuf() {
    if (!xlRegisterBuf) return;
    Excel12(xlFree, 0, 1, &xlRegisterBuf[0]);
    free(regStrBuf);
    free(xlRegisterBuf);
    xlRegisterBuf = NULL;
}