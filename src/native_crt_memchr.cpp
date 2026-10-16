#include "bsp/native_crt_memchr.hpp"

namespace bsp {
namespace {
using Byte = unsigned char;
using Uint32 = unsigned int;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(int) == 4);
static_assert(sizeof(Uint32) == 4);
static_assert(static_cast<Byte>(0x100U) == 0U);

// Addressed MOV32 over actual byte storage: no invented Uint32 object or
// aliasing/lifetime precondition. The caller supplies four aligned in-range
// bytes. This private target mechanism must be checked in each actual object.
__forceinline Uint32 read_aligned_dword(const volatile Byte* address) {
    Uint32 value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
} // namespace

void* __cdecl find_native_crt_byte_00bfa700(
    void* actual_buffer, int value, unsigned int count) {
    if (count == 0U) {
        return nullptr;
    }

    Byte* current = static_cast<Byte*>(actual_buffer);
    const Byte target = static_cast<Byte>(value);
    while ((reinterpret_cast<Uint32>(current) & 3U) != 0U) {
        const Byte current_byte = *static_cast<const volatile Byte*>(current);
        if (current_byte == target) {
            return current;
        }
        ++current;
        --count;
        if (count == 0U) {
            return nullptr;
        }
    }

    const Uint32 repeated_target = static_cast<Uint32>(target) * 0x01010101U;
    while (count >= 4U) {
        const Uint32 word = read_aligned_dword(current);
        const Uint32 x = word ^ repeated_target;
        if ((((x + 0x7efefeffU) ^ ~x) & 0x81010100U) != 0U) {
            const Uint32 candidate = read_aligned_dword(current);
            if (static_cast<Byte>(candidate) == target) {
                return current;
            }
            if (static_cast<Byte>(candidate >> 8U) == target) {
                return current + 1U;
            }
            if (static_cast<Byte>(candidate >> 16U) == target) {
                return current + 2U;
            }
            if (static_cast<Byte>(candidate >> 24U) == target) {
                return current + 3U;
            }
        }
        current += 4U;
        count -= 4U;
    }

    while (count != 0U) {
        const Byte current_byte = *static_cast<const volatile Byte*>(current);
        if (current_byte == target) {
            return current;
        }
        ++current;
        --count;
    }
    return nullptr;
}

} // namespace bsp
