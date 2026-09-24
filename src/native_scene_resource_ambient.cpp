#include "bsp/native_scene_resource_ambient.hpp"
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
volatile Word& cell(void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(p) + offset);
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word identity(void* value) noexcept { return reinterpret_cast<Word>(value); }
SystemAmbientBacklinks& array(void* ambient) noexcept {
    return *reinterpret_cast<SystemAmbientBacklinks*>(static_cast<std::byte*>(ambient) + 8);
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
    auto& links = array(ambient);
    // Valid nonoverflowing spans make native begin<end equivalent to count>0.
    // No callback/write occurs before the leaf captures this opaque key.
    if (links.count_04 == 0) return false;
    SceneResource* const key = reinterpret_cast<SceneResource*>(scene_argument);
    return erase_system_ambient_backlink_00b7b620(links, &key); // B7BD58
}

void append_native_ambient_scene_00b7bf90(void* ambient, const volatile Word& scene_argument) {
    const Word capacity = cell(ambient, 0x10);
    if (cell(ambient, 0x0c) == capacity) {
        const Word doubled = capacity + capacity;
        std::int32_t minimum;
        std::memcpy(&minimum, &doubled, sizeof(minimum));
        if (minimum <= 1) minimum = 1;
        reserve_system_ambient_backlinks_00b7b390(array(ambient), minimum); // B7BFAB
    }
    const Word count = cell(ambient, 0x0c);
    const Word begin = cell(ambient, 8);
    const Word destination = begin + count * 4u;
    if (destination != 0) *reinterpret_cast<volatile Word*>(destination) = scene_argument;
    cell(ambient, 0x0c) = cell(ambient, 0x0c) + 1u;
}

void set_native_scene_resource_ambient_00b825d0(void* resource,
    const volatile Word& requested_argument, NativeSceneResourceAmbientFrame& frame,
    NativeSceneResourceAmbientContext& context) {
    if (void* initial = pointer(cell(resource, 0x10))) {
        frame.scene_argument = identity(resource);
        remove_native_ambient_scene_00b7bd50(initial, frame.scene_argument); // B825DB
    }
    void* const requested = pointer(requested_argument); // B825E0
    void* const old = pointer(cell(resource, 0x10)); // B825E5
    if (old != requested) {
        cell(resource, 0x10) = identity(requested);
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
    if (void* current = pointer(cell(resource, 0x10))) {
        frame.scene_argument = identity(resource);
        append_native_ambient_scene_00b7bf90(current, frame.scene_argument); // B82620
    }
}

} // namespace bsp
