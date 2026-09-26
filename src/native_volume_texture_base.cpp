#include "bsp/native_volume_texture_base.hpp"

#include <atomic>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unnamed volume texture base requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(alignof(std::atomic<std::int32_t>) == 4);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);
void* at(void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
Word read(const void* storage) noexcept {
    Word value;
    __asm { mov eax, storage }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void put(void* storage, Word value) noexcept {
    __asm { mov eax, storage }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
} // namespace

void* construct_native_unnamed_volume_texture_base_00b340a0(
    void* owner, void* borrowed_com, Word flags, Word& serial) {
    put(owner, 0x00ceb130);
    put(owner, 0x00d5f1f4);
    ::new (at(owner, 4)) std::atomic<std::int32_t>(1);
    put(at(owner, 8), 0);
    put(at(owner, 0x0c), 0);
    put(at(owner, 0x14), 0);
    put(at(owner, 0x10), reinterpret_cast<Word>(borrowed_com));
    put(at(owner, 0x1c), flags);
    put(at(owner, 0x20), read(&serial));
    put(&serial, read(&serial) + 1u);
    put(owner, 0x00d5f2c0);
    return owner;
}

} // namespace bsp
