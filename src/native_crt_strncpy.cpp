#include "bsp/native_crt_strncpy.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw-storage implementation requires MSVC Win32 x86.
#endif

static_assert(sizeof(void*) == 4, "The target has 32-bit pointers.");
static_assert(sizeof(unsigned int) == 4, "The target has unsigned32 counts.");

namespace bsp {
namespace {

__declspec(noinline) unsigned int __cdecl load_raw_dword(const void* address)
{
    unsigned int value = 0U;
    __asm {
        mov edx, address
        mov eax, dword ptr [edx]
        mov value, eax
    }
    return value;
}

__declspec(noinline) void __cdecl store_raw_dword(void* address, unsigned int value)
{
    __asm {
        mov edx, address
        mov ecx, value
        mov dword ptr [edx], ecx
    }
}

} // namespace

void* __cdecl copy_native_crt_padded_bytes_00bf9280(
    void* actual_destination, const void* actual_source, unsigned int count)
{
    void* const result = actual_destination;
    auto* destination = static_cast<unsigned char*>(actual_destination);
    auto* source = static_cast<const unsigned char*>(actual_source);
    unsigned int remaining = count;
    if (remaining == 0U) {
        return result;
    }

    while ((reinterpret_cast<unsigned int>(source) & 3U) != 0U) {
        const unsigned char value = *static_cast<const volatile unsigned char*>(source);
        ++source;
        *static_cast<volatile unsigned char*>(destination) = value;
        ++destination;
        --remaining;
        if (remaining == 0U) {
            return result;
        }
        if (value == 0U) {
            while ((reinterpret_cast<unsigned int>(destination) & 3U) != 0U) {
                *static_cast<volatile unsigned char*>(destination) = 0U;
                ++destination;
                --remaining;
                if (remaining == 0U) {
                    return result;
                }
            }
            unsigned int zero_words = remaining >> 2U;
            while (zero_words != 0U) {
                store_raw_dword(destination, 0U);
                destination += 4U;
                --zero_words;
            }
            unsigned int zero_tail = remaining & 3U;
            while (zero_tail != 0U) {
                *static_cast<volatile unsigned char*>(destination) = 0U;
                ++destination;
                --zero_tail;
            }
            return result;
        }
    }

    unsigned int words = remaining >> 2U;
    unsigned int tail = remaining & 3U;
    bool padding = false;
    while (words != 0U) {
        const unsigned int first = load_raw_dword(source);
        volatile unsigned int candidate =
            ((first + 0x7efefeffU) ^ ~first) & 0x81010100U;
        const unsigned int second = load_raw_dword(source);
        source += 4U;
        unsigned int copied = second;
        bool found_zero = false;
        if (candidate != 0U) {
            if ((second & 0xffU) == 0U) {
                copied = 0U;
                found_zero = true;
            } else if ((second & 0xff00U) == 0U) {
                copied = second & 0xffU;
                found_zero = true;
            } else if ((second & 0xff0000U) == 0U) {
                copied = second & 0xffffU;
                found_zero = true;
            } else if ((second & 0xff000000U) == 0U) {
                found_zero = true;
            }
        }
        store_raw_dword(destination, copied);
        destination += 4U;
        --words;
        if (found_zero) {
            padding = true;
            break;
        }
    }

    if (padding) {
        while (words != 0U) {
            store_raw_dword(destination, 0U);
            destination += 4U;
            --words;
        }
        while (tail != 0U) {
            *static_cast<volatile unsigned char*>(destination) = 0U;
            ++destination;
            --tail;
        }
        return result;
    }

    while (tail != 0U) {
        const unsigned char value = *static_cast<const volatile unsigned char*>(source);
        ++source;
        *static_cast<volatile unsigned char*>(destination) = value;
        ++destination;
        if (value == 0U) {
            --tail;
            while (tail != 0U) {
                *static_cast<volatile unsigned char*>(destination) = 0U;
                ++destination;
                --tail;
            }
            return result;
        }
        --tail;
    }
    return result;
}

} // namespace bsp
