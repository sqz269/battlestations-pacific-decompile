#include "bsp/native_damageable_section.hpp"

#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable section construction requires MSVC Win32 x87.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}
using ReleaseOwner = void (__thiscall*)(void*);
} // namespace

void release_native_ref_counted_handle_0041de40(void* slot) {
    void* const owner = field<void*>(slot);
    if (owner == nullptr) return;
    if (_InterlockedDecrement(&field<long>(owner, 4)) == 0) {
        void* const table = field<void*>(owner);
        field<ReleaseOwner>(table)(owner);
    }
    field<void*>(slot) = nullptr;
}

void destroy_native_damageable_section_00878ef0(void* section, Word actual_vtable) {
    field<Word>(section) = actual_vtable;
    release_native_ref_counted_handle_0041de40(at(section, 0x24));
}

void* copy_construct_native_damageable_section_00878b40(void* destination,
    const void* source, Word actual_vtable) {
    field<Word>(destination) = actual_vtable;
    field<Word>(destination, 4) = field<Word>(source, 4);
    field<Word>(destination, 8) = field<Word>(source, 8);
    // 00878B5A..7B: FLD/FSTP pairs, not integer/SSE loads or a staged copy.
    // In particular a masked signaling NaN is quieted and x87 flags survive.
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx+0ch]
        fstp dword ptr [eax+0ch]
        fld dword ptr [edx+10h]
        fstp dword ptr [eax+10h]
        fld dword ptr [edx+14h]
        fstp dword ptr [eax+14h]
        fld dword ptr [edx+18h]
        fstp dword ptr [eax+18h]
        fld dword ptr [edx+1ch]
        fstp dword ptr [eax+1ch]
        fld dword ptr [edx+20h]
        fstp dword ptr [eax+20h]
    }
    field<void*>(destination, 0x24) = nullptr;
    void* const owner = field<void*>(source, 0x24);
    if (owner != nullptr) {
        field<void*>(destination, 0x24) = owner;
        _InterlockedIncrement(&field<long>(owner, 4));
    }
    // 00878B99..A5 occur after publication and the retain, including aliases.
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx+28h]
        fstp dword ptr [eax+28h]
        fld dword ptr [edx+2ch]
        fstp dword ptr [eax+2ch]
    }
    return destination;
}

} // namespace bsp
