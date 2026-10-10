#include <windows.h>
#include "XLCALL.H"

// Flags for which field to return from get_tl_data
enum TL_data_field {
    RET_VAL,
    XLBUF,
    STRLEN,
    STRBUF
};

// Default 8KB allocated per thread, change as needed
#define TLS_BUF_SIZE (8192)

// TLS_Data structured to fit into 8KB, one xloper12 followed by a buffer held inline, change as needed
#define XL_BUF_LEN 255

// Same principle, strings longer than this are silently truncated
#define STR_BUF_LEN 4079

typedef struct
{
    XLOPER12 ret_val;
    union {
        XLOPER12 xlBuf[XL_BUF_LEN];
        struct {
            XCHAR len;
            XCHAR strBuf[STR_BUF_LEN];
        } xlStr;
    } buf;
} TLS_data;

// Size checks at compile time
_Static_assert(TLS_BUF_SIZE == sizeof(TLS_data), "TLS_BUF_SIZE and TLS_data must be same size");
_Static_assert(TLS_BUF_SIZE == sizeof(XLOPER12) * (1 + XL_BUF_LEN), "XL_BUF_LEN does not match up with TLS_BUF_SIZE");
_Static_assert(TLS_BUF_SIZE == sizeof(XLOPER12) + sizeof(XCHAR) * (1 + STR_BUF_LEN), "STR_BUF_LEN does not match up with TLS_BUF_SIZE");

LPXLOPER12 Null_XLOPER12(void);
TLS_data *get_TLS_data(void);
void* get_tl_data(enum TL_data_field field);