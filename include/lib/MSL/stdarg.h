#ifndef MSL_STDARG_H
#define MSL_STDARG_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __CWCC__

typedef enum _va_arg_type {
    arg_ARGPOINTER,
    arg_WORD,
    arg_DOUBLEWORD,
    arg_ARGREAL
} _va_arg_type;

typedef struct __va_list_struct {
    char gpr;
    char fpr;
    char* input_arg_area;
    char* reg_save_area;
} __va_list_struct;

/// @brief An array of one, so that it decays to a pointer when passed on.
/// Declaring this as a plain struct makes MWCC copy all 16 bytes to the stack
/// before each `__va_arg` call; the original passes the pointer.
typedef __va_list_struct va_list[1];

void* __va_arg(va_list argp, int type);

#define va_start(VA_LIST, ARG) ((void)ARG, __builtin_va_info(&VA_LIST))
#define va_end(VA_LIST) ((void)VA_LIST)
#define va_arg(VA_LIST, ARG_TYPE)                                              \
    (*(ARG_TYPE*)__va_arg(VA_LIST, _var_arg_typeof(ARG_TYPE)))

#else

typedef __builtin_va_list va_list;

#define va_start(VA_LIST, ARG) __builtin_va_start(VA_LIST, ARG)
#define va_end(VA_LIST) ((void)VA_LIST)
#define va_arg(VA_LIST, ARG_TYPE)                                              \
    (*(ARG_TYPE*)__va_arg(VA_LIST, _var_arg_typeof(ARG_TYPE)))

#endif

#ifdef __cplusplus
}
#endif
#endif
