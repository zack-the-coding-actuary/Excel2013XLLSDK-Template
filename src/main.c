#include <windows.h>
#include "XLCALL.H"
#include "helper.h"

/* Wrappers ----------------------------------------------------------------- */

static wchar_t *xloper12_sum_strs[] = {
L"xloper12_sum",
L"QQ",
L"SAFE_SUM",
L"range",
L"Safe sum of an Excel range",
L"Range to sum over"
};

static size_t xloper12_sum_len = sizeof(xloper12_sum_strs) / sizeof(xloper12_sum_strs[0]);

__declspec(dllexport) LPXLOPER12 WINAPI xloper12_sum(LPXLOPER12 range)
{
    double acc = 0;
    if (range->xltype != xltypeMulti) {
        return LStrToXL(L"Error: passed argument must be a valid range");
    }
    for (size_t i = 0; i < range->val.array.rows * range->val.array.columns; ++i) {

        if (range->val.array.lparray[i].xltype != xltypeNum)
            return LStrToXL(L"Error: passed argument had a non-number type");

        acc += range->val.array.lparray[i].val.num;
    }
    return DoubleToXL(acc);
}

static wchar_t *fp12_sum_strs[] = {
L"fp12_sum",
L"BK%",
L"UNSAFE_SUM",
L"range",
L"Unsafe sum of an Excel range",
L"Range to sum over"
};

static size_t fp12_sum_len = sizeof(fp12_sum_strs) / sizeof(fp12_sum_strs[0]);

__declspec(dllexport) double WINAPI fp12_sum(FP12 *range)
{
    double acc = 0;
    for (size_t i = 0; i < range->rows * range->columns; ++i) {
        acc += range->array[i];
    }
    return acc;
}

static wchar_t *triple_sum_strs[] = {
L"triple_sum",
L"BBBB",
L"TRIP_SUM",
L"X, Y, Z",
L"Sum of three doubles",
L"1st double",
L"2nd double",
L"3rd double"
};

static size_t triple_sum_len = sizeof(triple_sum_strs) / sizeof(triple_sum_strs[0]);

__declspec(dllexport) double WINAPI triple_sum(double a, double b, double c) {
    return a + b + c;
}

/* Required XLL entry points ------------------------------------------------ */

__declspec(dllexport) int WINAPI xlAutoOpen(void)
{
    if (!AllocateRegistrationBuf()) return 0;

    RegisterFunc(xloper12_sum_strs, xloper12_sum_len);
    RegisterFunc(fp12_sum_strs, fp12_sum_len);
    RegisterFunc(triple_sum_strs, triple_sum_len);

    FreeRegistrationBuf();

    return 1;
}

__declspec(dllexport) int WINAPI xlAutoClose(void)
{
    return 1;
}


// Don't need AutoAdd or AutoRemove
// AutoOpen and AutoClose cover the same surface area and then some

// __declspec(dllexport) int __cdecl xlAutoAdd(void)
// __declspec(dllexport) int __cdecl xlAutoRemove(void)

// TODO: Determine if AutoFree method needed depending on how static buffers are handled

__declspec(dllexport) void WINAPI xlAutoFree12(LPXLOPER12 pxFree)
{
    (void)pxFree;
}

// Only update the Add In name here, need to add length prefix, do NOT use MakePascal
__declspec(dllexport) LPXLOPER12 WINAPI xlAddInManagerInfo12(LPXLOPER12 pxAction)
{
    static XLOPER12 xInfo;
    static XLOPER12 xIntAction;
    static XCHAR szName[] = L"\x08" L"Template";
    XLOPER12 xCoerceType;

    xCoerceType.xltype = xltypeInt;
    xCoerceType.val.w  = xltypeInt;
    Excel12(xlCoerce, &xIntAction, 2, pxAction, &xCoerceType);

    if (xIntAction.val.w == 1)
    {
        xInfo.xltype  = xltypeStr;
        xInfo.val.str = szName;
    }
    else
    {
        xInfo.xltype  = xltypeErr;
        xInfo.val.err = xlerrValue;
    }

    return &xInfo;
}