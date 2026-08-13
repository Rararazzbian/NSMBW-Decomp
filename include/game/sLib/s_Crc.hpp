#pragma once

#include <types.h>

/// @brief CRC helpers.
/// @unofficial The class name comes from the mangled symbol
/// `calcCRC32__4sCrcFPCvUl`; its defining TU is not yet decompiled.
class sCrc {
public:
    /// @note `unsigned long`, not `u32`. u32 is `unsigned int` here and would
    /// mangle `Ui`; the symbol is `calcCRC32__4sCrcFPCvUl`.
    static unsigned long calcCRC32(const void *p, unsigned long len);
};
