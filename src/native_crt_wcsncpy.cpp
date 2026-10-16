#include "bsp/native_crt_wcsncpy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw-storage implementation requires MSVC Win32 x86.
#endif

static_assert(sizeof(void*) == 4, "The target has 32-bit pointers.");
static_assert(sizeof(unsigned int) == 4, "The target has unsigned32 counts.");
static_assert(sizeof(unsigned short) == 2, "The target has WORD16 units.");

namespace bsp {
namespace {

__declspec(noinline) unsigned int __cdecl load_raw_word(const void* address)
{
    unsigned int value = 0U;
    __asm {
        mov edx, address
        movzx eax, word ptr [edx]
        mov value, eax
    }
    return value;
}

__declspec(noinline) void __cdecl store_raw_word(void* address, unsigned int value)
{
    __asm {
        mov edx, address
        mov ecx, value
        mov word ptr [edx], cx
    }
}

__declspec(noinline) void __cdecl store_raw_zero_dword(void* address)
{
    __asm {
        mov edx, address
        xor eax, eax
        mov dword ptr [edx], eax
    }
}

} // namespace

void* __cdecl copy_native_crt_wide_units_00bfaeef(
    void* actual_destination, const void* actual_source, unsigned int count)
{
    void* const result = actual_destination;
    auto* destination = static_cast<unsigned char*>(actual_destination);
    auto* source = static_cast<const unsigned char*>(actual_source);
    unsigned int remaining = count;
    if (remaining == 0U) {
        return result;
    }

    for (;;) {
        const unsigned int value = load_raw_word(source);
        store_raw_word(destination, value);
        ++destination;
        ++destination;
        ++source;
        ++source;
        if (value == 0U) {
            --remaining;
            break;
        }
        --remaining;
        if (remaining == 0U) {
            return result;
        }
    }

    if (remaining == 0U) {
        return result;
    }
    unsigned int pairs = remaining >> 1U;
    while (pairs != 0U) {
        store_raw_zero_dword(destination);
        destination += 4U;
        --pairs;
    }
    if ((remaining & 1U) != 0U) {
        store_raw_word(destination, 0U);
        ++destination;
        ++destination;
    }
    return result;
}

} // namespace bsp
