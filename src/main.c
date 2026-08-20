#include <windows.h>
#include "XLCALL.H"
#include "helper.h"

/* Wrappers ----------------------------------------------------------------- */

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

__declspec(dllexport) double WINAPI fp12_sum(FP12 *range)
{
    double acc = 0;
    for (size_t i = 0; i < range->rows * range->columns; ++i) {
        acc += range->array[i];
    }
    return acc;
}

/* Required XLL entry points ------------------------------------------------ */

__declspec(dllexport) int WINAPI xlAutoOpen(void)
{
    XLOPER12 xDLL;
    XLOPER12 xProcName;
    XLOPER12 xTypeText;
    XLOPER12 xFuncText;
    XLOPER12 xArgText;
    XLOPER12 xMacroType;
    XLOPER12 xFunctionHelp;
    XLOPER12 xArgumentHelp1;

    Excel12(xlGetName, &xDLL, 0);

    xProcName.xltype      = xltypeStr; xProcName.val.str      = (XCHAR *)L"\x0Cxloper12_sum";
    xTypeText.xltype      = xltypeStr; xTypeText.val.str      = (XCHAR *)L"\x02QQ";
    xFuncText.xltype      = xltypeStr; xFuncText.val.str      = (XCHAR *)L"\x08SAFE_SUM";
    xArgText.xltype       = xltypeStr; xArgText.val.str       = (XCHAR *)L"\x07rangeee";
    xMacroType.xltype     = xltypeInt; xMacroType.val.w       = 1;
    xFunctionHelp.xltype  = xltypeStr; xFunctionHelp.val.str  = (XCHAR *)L"\x1ASafe sum of an Excel range";
    xArgumentHelp1.xltype = xltypeStr; xArgumentHelp1.val.str = (XCHAR *)L"\x11Range to sum over";

    Excel12(xlfRegister, 0, 11,
        &xDLL, &xProcName, &xTypeText, &xFuncText, &xArgText, &xMacroType, NULL, NULL, NULL, &xFunctionHelp, &xArgumentHelp1);

    xProcName.xltype  = xltypeStr; xProcName.val.str  = (XCHAR *)L"\x08" L"fp12_sum";
    xTypeText.xltype  = xltypeStr; xTypeText.val.str  = (XCHAR *)L"\x03" L"BK%";
    xFuncText.xltype  = xltypeStr; xFuncText.val.str  = (XCHAR *)L"\x0AUNSAFE_SUM";
    xArgText.xltype   = xltypeStr; xArgText.val.str   = (XCHAR *)L"\x05range";
    xMacroType.xltype = xltypeInt; xMacroType.val.w   = 1;
    xFunctionHelp.xltype  = xltypeStr; xFunctionHelp.val.str  = (XCHAR *)L"\x1CUnsafe sum of an Excel range";
    xArgumentHelp1.xltype = xltypeStr; xArgumentHelp1.val.str = (XCHAR *)L"\x11Range to sum over";

    Excel12(xlfRegister, 0, 11,
        &xDLL, &xProcName, &xTypeText, &xFuncText, &xArgText, &xMacroType, NULL, NULL, NULL, &xFunctionHelp, &xArgumentHelp1);

    Excel12(xlFree, 0, 1, &xDLL);

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
    static XCHAR szName[] = L"\x08Template";
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

/* -------------------------------------------------------------------------- */

// More no touchy
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}