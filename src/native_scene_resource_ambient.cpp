#include "bsp/native_scene_resource_ambient.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "native_scene_typed_storage.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene-resource ambient operations require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(SystemAmbientBacklinks) == 12);
static_assert(offsetof(SystemAmbientBacklinks, count_04) == 4);
static_assert(offsetof(SystemAmbientBacklinks, capacity_08) == 8);
void* at(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read(void* p, Word offset = 0) noexcept { return detail::scene_word(at(p, offset)); }
void write(void* p, Word offset, Word value) noexcept { detail::scene_word(at(p, offset), value); }
std::int32_t signed_word(Word bits) noexcept {
    std::int32_t value; std::memcpy(&value, &bits, 4); return value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word identity(void* value) noexcept { return reinterpret_cast<Word>(value); }
// Same complete game-owned B7B390 reserve algorithm. Only backing accesses
// differ: bytes/addresses, not fictitious C++ pointer-array objects. Allocation
// and free remain the existing shared singleton providers, including callbacks.
void reserve_backlinks(void* ambient, std::int32_t minimum) {
    if (minimum < 1) minimum = 1;
    if (minimum <= signed_word(read(ambient, 0x10))) return;
    const Word bytes = static_cast<Word>(minimum) * 4u;
    void* const allocated = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    for (std::int32_t i = 0; i < signed_word(read(ambient, 0x0c)); ++i) {
        const Word source = read(ambient, 8) + static_cast<Word>(i) * 4u;
        const Word value = read(pointer(source));
        write(allocated, static_cast<Word>(i) * 4u, value);
    }
    singleton_lifetime_free(pointer(read(ambient, 8)));
    write(ambient, 8, identity(allocated));
    write(ambient, 0x10, static_cast<Word>(minimum));
}
void terminal(void* old, NativeSceneResourceAmbientContext& context) {
    auto& reference = context.owners.resolve_actual(old);
    if (&reference.reference_count != reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(old) + 4))
        throw std::logic_error("ambient terminal requires its canonical actual count");
    reference.release_zero_references();
}
}

bool remove_native_ambient_scene_00b7bd50(void* ambient, const volatile Word& scene_argument) {
    // Preserve the B7BD50 empty gate and captured key, then the complete
    // B7B620 first-match/swap-last algorithm without typed pointer-slot access.
    if (read(ambient, 0x0c) == 0) return false;
    const Word key = scene_argument;
    for (std::int32_t i = 0; i < signed_word(read(ambient, 0x0c)); ++i) {
        if (read(pointer(read(ambient, 8) + static_cast<Word>(i) * 4u)) != key) continue;
        const auto last = signed_word(read(ambient, 0x0c)) - 1;
        if (i != last) {
            const Word value = read(pointer(read(ambient, 8) + static_cast<Word>(last) * 4u));
            write(pointer(read(ambient, 8)), static_cast<Word>(i) * 4u, value);
        }
        write(ambient, 0x0c, read(ambient, 0x0c) - 1u);
        return true;
    }
    return false;
}

void append_native_ambient_scene_00b7bf90(void* ambient, const volatile Word& scene_argument) {
    const Word capacity = read(ambient, 0x10);
    if (read(ambient, 0x0c) == capacity) {
        const Word doubled = capacity + capacity;
        std::int32_t minimum;
        std::memcpy(&minimum, &doubled, sizeof(minimum));
        if (minimum <= 1) minimum = 1;
        reserve_backlinks(ambient, minimum); // B7BFAB
    }
    const Word count = read(ambient, 0x0c);
    const Word begin = read(ambient, 8);
    const Word destination = begin + count * 4u;
    if (destination != 0) write(pointer(destination), 0, scene_argument);
    write(ambient, 0x0c, read(ambient, 0x0c) + 1u);
}

void resize_native_ambient_backlinks_00b7bc70(void* ambient, std::int32_t count) {
    if (signed_word(read(ambient, 0x10)) < count) reserve_backlinks(ambient, count);
    for (auto i = signed_word(read(ambient, 0x0c)); i < count; ++i)
        write(pointer(read(ambient, 8)), static_cast<Word>(i) * 4u, 0);
    while (count < signed_word(read(ambient, 0x0c)))
        write(ambient, 0x0c, read(ambient, 0x0c) - 1u);
    write(ambient, 0x0c, static_cast<Word>(count));
}

void set_native_scene_resource_ambient_00b825d0(void* resource,
    const volatile Word& requested_argument, NativeSceneResourceAmbientFrame& frame,
    NativeSceneResourceAmbientContext& context) {
    if (void* initial = pointer(read(resource, 0x10))) {
        frame.scene_argument = identity(resource);
        remove_native_ambient_scene_00b7bd50(initial, frame.scene_argument); // B825DB
    }
    void* const requested = pointer(requested_argument); // B825E0
    void* const old = pointer(read(resource, 0x10)); // B825E5
    if (old != requested) {
        write(resource, 0x10, identity(requested));
        if (requested) {
            const auto increment = context.increment_00ce221c;
            if (!increment) throw std::logic_error("missing current CE221C increment import");
            increment(reinterpret_cast<volatile long*>(static_cast<std::byte*>(requested) + 4));
        }
        if (old) {
            const auto decrement = context.decrement_00ce2220;
            if (!decrement) throw std::logic_error("missing current CE2220 decrement import");
            if (decrement(reinterpret_cast<volatile long*>(static_cast<std::byte*>(old) + 4)) == 0)
                terminal(old, context); // B82615: current actual virtual0
        }
    }
    if (void* current = pointer(read(resource, 0x10))) {
        frame.scene_argument = identity(resource);
        append_native_ambient_scene_00b7bf90(current, frame.scene_argument); // B82620
    }
}

} // namespace bsp
