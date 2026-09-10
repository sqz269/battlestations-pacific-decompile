#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <fpieee.h>
#include "bsp/legacy_crt_math.hpp"
#include <atomic>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The recovered CRT math environment requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
std::atomic<const LegacyCrtMathRuntime*> runtime_binding{nullptr};

// 00C13048 uses the VS2005 FPIEEE layout, including the 16-byte union and
// 32-byte value stride. Check the host header instead of assuming ABI drift.
static_assert(sizeof(_FPIEEE_RECORD) == 112);
static_assert(offsetof(_FPIEEE_RECORD, Operand1) == 16);
static_assert(offsetof(_FPIEEE_RECORD, Operand2) == 48);
static_assert(offsetof(_FPIEEE_RECORD, Result) == 80);
static_assert(sizeof(CameraAxesCrtException) == 32);
static_assert(offsetof(CameraAxesCrtException, result) == 24);

struct alignas(16) IeeeValue {
    double number;
    std::uint64_t unused;
    std::uint32_t flags;
    std::uint32_t padding[3];
};
struct alignas(16) IeeeRecord {
    std::uint32_t mode, cause, enable, status;
    IeeeValue operand1, operand2, result;
};
static_assert(sizeof(IeeeRecord) == sizeof(_FPIEEE_RECORD));

void copy_x87(double& destination, const double& source) {
    __asm {
        mov eax, source
        mov edx, destination
        fld qword ptr [eax]
        fstp qword ptr [edx]
    }
}
std::uint16_t compare_zero(const double& value) {
    std::uint16_t status;
    __asm {
        mov eax, value
        fldz
        fcomp qword ptr [eax]
        fnstsw ax
        mov status, ax
    }
    return status;
}
void negate_x87(double& value) {
    __asm {
        mov eax, value
        fld qword ptr [eax]
        fchs
        fstp qword ptr [eax]
    }
}
void multiply_zero_x87(double& value) {
    const double zero = 0.0; // Original 00D7A258.
    __asm {
        mov eax, value
        fld qword ptr [eax]
        fmul zero
        fstp qword ptr [eax]
    }
}
std::uint64_t bits(const double& value) {
    std::uint64_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
void set_bits(double& value, std::uint64_t representation) {
    std::memcpy(&value, &representation, sizeof(value));
}

// 00C138CE: cause real x87 exceptions with the original 80-bit constants.
// Merely changing a software flag or MXCSR does not reproduce this function.
void set_status_00c138ce(std::uint32_t mask) {
    static const unsigned char large[10] = {0,0,0,0,0,0,0,0x80,0x10,0x44};
    static const unsigned char tiny[10] = {1,0,0,0,0,0,0,0x80,0,0x30};
    double scratch;
    int integer_scratch;
    __asm {
        mov ecx, mask
        test cl, 1
        jz no_invalid
        fld tbyte ptr large
        fistp integer_scratch
        fwait
    no_invalid:
        test cl, 8
        jz no_overflow
        fstsw ax
        fld tbyte ptr large
        fstp scratch
        fwait
        fstsw ax
    no_overflow:
        test cl, 10h
        jz no_underflow
        fld tbyte ptr tiny
        fstp scratch
        fwait
    no_underflow:
        test cl, 4
        jz no_zero_divide
        fldz
        fld1
        fdivrp st(1), st(0)
        fstp st(0)
        fwait
    no_zero_divide:
        test cl, 20h
        jz finished
        fldpi
        fstp scratch
        fwait
    finished:
    }
}

// 00C12F99 plus __set_exp00C12EB0. The original subnormal loop and binary64
// spills are significant; no modern frexp or floating-point classification.
void decompose_00c12f99(double& value, int& exponent) {
    const auto zero_status = compare_zero(value);
    if ((zero_status & 0x4400) == 0x4000) {
        value = 0.0;
        exponent = 0;
        return;
    }
    auto representation = bits(value);
    if ((representation & 0x7ff0000000000000ull) == 0 &&
            (representation & 0x000fffffffffffffull) != 0) {
        const bool negative = (compare_zero(value) & 0x4100) == 0;
        exponent = -1021;
        while ((representation & 0x0010000000000000ull) == 0) {
            representation <<= 1;
            --exponent;
        }
        representation &= ~0x0010000000000000ull;
        if (negative) representation |= 0x8000000000000000ull;
    } else {
        exponent = static_cast<int>((representation >> 52) & 0x7ff) - 1022;
    }
    set_bits(value, representation);
    copy_x87(value, value); // __set_exp's input spill precedes exponent rewrite.
    representation = (bits(value) & 0x800fffffffffffffull) | 0x3fe0000000000000ull;
    set_bits(value, representation);
    copy_x87(value, value);
}

bool handle_00c13364(std::uint32_t mask, double& result, std::uint32_t control) {
    std::uint32_t pending = mask & 0x1f;
    if ((mask & 8) && (control & 1)) {
        set_status_00c138ce(1);
        pending &= ~8u;
    } else if ((mask & 4) && (control & 4)) {
        set_status_00c138ce(4);
        pending &= ~4u;
    } else if ((mask & 1) && (control & 8)) {
        set_status_00c138ce(8);
        const auto comparison = compare_zero(result);
        // FCOMP 0,result; TEST AH,5; JP handles <=0 and unordered alike.
        const bool positive = ((comparison & 0x0500) == 0x0100) ||
                              ((comparison & 0x0500) == 0x0400);
        const auto rounding = control & 0xc00;
        const bool infinity = rounding == 0 ||
            (rounding == 0x400 && !positive) || (rounding == 0x800 && positive);
        set_bits(result, infinity ? 0x7ff0000000000000ull : 0x7fefffffffffffffull);
        if (!positive) negate_x87(result);
        else copy_x87(result, result);
        pending &= ~1u;
    } else if ((mask & 2) && (control & 0x10)) {
        bool inexact = (mask & 0x10) != 0;
        const auto comparison = compare_zero(result);
        if ((comparison & 0x4400) == 0x4000) {
            inexact = true;
        } else {
            double fraction;
            copy_x87(fraction, result);
            int exponent;
            decompose_00c12f99(fraction, exponent);
            exponent -= 1536;
            if (exponent < -1074) {
                multiply_zero_x87(fraction);
                inexact = true;
            } else {
                const bool negative = (compare_zero(fraction) & 0x4100) == 0;
                auto representation = (bits(fraction) & 0x000fffffffffffffull) |
                                      0x0010000000000000ull;
                while (exponent < -1021) {
                    if (representation & 1) inexact = true;
                    representation >>= 1;
                    ++exponent;
                }
                set_bits(fraction, representation);
                if (negative) negate_x87(fraction);
            }
            copy_x87(result, fraction);
        }
        if (inexact) set_status_00c138ce(0x10);
        pending &= ~2u;
    }
    if ((mask & 0x10) && (control & 0x20)) {
        set_status_00c138ce(0x20);
        pending &= ~0x10u;
    }
    return pending == 0;
}

std::uint32_t ieee_flags(std::uint32_t x87) {
    return ((x87 << 4) & 16) | ((x87 << 1) & 8) | ((x87 >> 1) & 4) |
           ((x87 >> 3) & 2) | ((x87 >> 5) & 1);
}

// __raise_exc00C13322 fixes the last __raise_exc_ex argument to zero (binary64).
void raise_00c13322(IeeeRecord& record, std::uint32_t& control,
        std::uint32_t mask, std::int32_t operation, CameraAxesCrtException& exception) {
    DWORD code = mask;
    record.cause = 0;
    if (mask & 0x10) { record.cause |= 1; code = 0xc000008f; }
    if (mask & 2) { record.cause |= 2; code = 0xc0000093; }
    if (mask & 1) { record.cause |= 4; code = 0xc0000091; }
    if (mask & 4) { record.cause |= 8; code = 0xc000008e; }
    if (mask & 8) { record.cause |= 16; code = 0xc0000090; }
    record.enable = ieee_flags(~control);
    std::uint16_t status;
    __asm { fstsw status }
    record.status = ieee_flags(status);
    record.mode = (record.mode & ~3u) | ((control >> 10) & 3);
    switch (control & 0x300) {
    case 0: record.mode = (record.mode & 0xffffffeb) | 8; break;
    case 0x200: record.mode = (record.mode & 0xffffffe7) | 4; break;
    case 0x300: record.mode &= 0xffffffe3; break;
    }
    record.mode = (record.mode & ~0x1ffe0u) |
                  ((static_cast<std::uint32_t>(operation) << 5) & 0x1ffe0);
    record.operand1.flags = (record.operand1.flags & 0xffffffe3) | 3;
    copy_x87(record.operand1.number, exception.argument1);
    record.result.flags = (record.result.flags & 0xffffffe3) | 3;
    copy_x87(record.result.number, exception.result);
    __asm { fnclex }
    const ULONG_PTR argument = reinterpret_cast<ULONG_PTR>(&record);
    ::RaiseException(code, 0, 1, &argument);
    // A continuing SEH/VEH handler may change all documented FPIEEE fields.
    if (record.enable & 16) control &= ~1u;
    if (record.enable & 8) control &= ~4u;
    if (record.enable & 4) control &= ~8u;
    if (record.enable & 2) control &= ~0x10u;
    if (record.enable & 1) control &= ~0x20u;
    control = (control & ~0xc00u) | ((record.mode & 3) << 10);
    // Native 00C132A8 loads EDX=FFFFF3FF and reuses it at 00C132F2..00C13308.
    // This clears rounding bits, not precision bits. Preserve the native quirk.
    switch ((record.mode >> 2) & 7) {
    case 0: control = (control & 0xfffff3ff) | 0x300; break;
    case 1: control = (control & 0xfffff3ff) | 0x200; break;
    case 2: control &= 0xfffff3ff; break;
    }
    copy_x87(exception.result, record.result.number);
}
}

void bind_legacy_crt_math_runtime(const LegacyCrtMathRuntime& runtime) {
    if (!runtime.matherr_bypass_00e16bd0 || !runtime.errno_location_00bffb8b)
        throw std::invalid_argument("Legacy CRT math requires its global and errno accessor");
    runtime_binding.store(&runtime, std::memory_order_release);
}

void __cdecl legacy_crt_87except_00c27489(std::int32_t operation,
        CameraAxesCrtException* exception, std::uint16_t* saved_control_word) {
    const auto* runtime = runtime_binding.load(std::memory_order_acquire);
    if (!runtime) throw std::logic_error("Legacy CRT math runtime has not been bound");
    std::uint32_t control = *saved_control_word;
    std::uint32_t mask = 0;
    switch (exception->type) {
    case 1: case 5: mask = 8; break;
    case 2: mask = 4; break;
    case 3: mask = 0x11; break;
    case 4: mask = 0x12; break;
    case 7: exception->type = 1; break;
    case 8: mask = 0x10; break;
    }
    if (mask && !handle_00c13364(mask, exception->result, control)) {
        // Native reserved bytes are unspecified stack storage. Initialize them
        // here; only documented FPIEEE fields carry a compatibility contract.
        IeeeRecord record{};
        if (operation == 0x10 || operation == 0x16 || operation == 0x1d) {
            copy_x87(record.operand2.number, exception->argument2);
            record.operand2.flags = 3;
        }
        raise_00c13322(record, control, mask, operation, *exception);
    }
    // __ctrlfp00C138A7(control, FFFF): waiting control read, then raw FLDCW.
    std::uint16_t previous;
    __asm {
        fstcw previous
        fldcw word ptr control
    }
    if (exception->type != 8 && *runtime->matherr_bypass_00e16bd0 == 0) {
        // _matherr00C28545 is exactly XOR EAX,EAX; RET. There is no callback
        // dispatch or result mutation in this installed library implementation.
    }
    if (exception->type == 1) *runtime->errno_location_00bffb8b() = 33;
    else if (exception->type == 2 || exception->type == 3)
        *runtime->errno_location_00bffb8b() = 34;
}
}
