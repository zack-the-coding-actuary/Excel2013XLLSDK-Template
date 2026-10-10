#include "tls.h"

static DWORD TlsIndex; // Module scope only if all TLS access in this module

// Fallback for broken thread-local allocations
static const XLOPER12 CatchAllErr = {.val.err = xlerrNull, .xltype = xltypeErr};
LPXLOPER12 Null_XLOPER12(void) { return (LPXLOPER12)&CatchAllErr; }

BOOL TLS_Action(DWORD DllMainCallReason)
{
    switch (DllMainCallReason)
    {
    case DLL_PROCESS_ATTACH: // The DLL is being loaded.
        if((TlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        break;

    case DLL_PROCESS_DETACH: // The DLL is being unloaded.
        TlsFree(TlsIndex); // Release the TLS index.
        break;
    case DLL_THREAD_DETACH: { // Free TLS buffer
        void *pTLS = TlsGetValue(TlsIndex);
        if (pTLS) VirtualFree(pTLS, 0, MEM_RELEASE);
        break;
        }
    }

    return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hDll, DWORD Reason, void *Reserved)
{
    // Silence trifilng compiler
    (void)hDll;
    (void)Reserved;

    return TLS_Action(Reason);
}

// Public API, the important stuff ----------------------------------------------------------------

TLS_data *get_TLS_data(void)
{
// Get a pointer to this thread's static memory.
    void *pTLS = TlsGetValue(TlsIndex);
    if(!pTLS) // No TLS memory for this thread yet
    {
        if((pTLS = VirtualAlloc(NULL, TLS_BUF_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)) == NULL)
        // Display some error message (omitted).
            return NULL;
        TlsSetValue(TlsIndex, pTLS); // Associate with this thread
    }
    return (TLS_data *)pTLS;
}

// Must cast to proper pointer type at call site, i.e. LPXLOPER12 or XCHAR*
void* get_tl_data(enum TL_data_field field)
{
    TLS_data *pTLS = get_TLS_data();
    if(pTLS) {
        switch (field) {
            case RET_VAL:
                return &(pTLS->ret_val);
            case XLBUF:
                return &(pTLS->buf.xlBuf);
            case STRLEN:
                return &(pTLS->buf.xlStr.len);
            case STRBUF:
                return &(pTLS->buf.xlStr.strBuf);
            default:
                return NULL;
        }
    }
    return NULL;
}