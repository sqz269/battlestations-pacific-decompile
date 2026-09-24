#include "bsp/native_gui_scene_fields.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
void* pointer(const void* p, Word offset) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        reinterpret_cast<Word>(p) + offset);
}
void put(void* p, Word offset, void* value) noexcept {
    *reinterpret_cast<void* volatile*>(at(p, offset)) = value;
}
}

void set_native_gui_scene_resource_00b723f0(void* scene,
    const volatile Word& argument, NativeGuiSceneFieldsContext& context) {
    void* const requested = reinterpret_cast<void*>(argument); // B723F0
    void* const old = pointer(scene, 0x1c); // B723F5
    if (old == requested) return;
    put(scene, 0x1c, requested);
    if (requested)
        context.increment_00ce221c(static_cast<volatile long*>(at(requested, 4)));
    if (old) {
        const long count = context.decrement_00ce2220(
            static_cast<volatile long*>(at(old, 4)));
        if (count == 0) {
            auto& reference = context.owners.resolve_actual(old);
            if (static_cast<void*>(&reference.reference_count) != at(old, 4))
                throw std::logic_error("scene field terminal requires captured owner's actual count");
            reference.release_zero_references();
        }
    }
}

void prepend_native_gui_scene_node_00b721f0(void* scene, void* node) noexcept {
    void* const old = pointer(scene, 0x0c);
    put(node, 0x3c, old);
    put(node, 0x40, nullptr);
    void* const current = pointer(scene, 0x0c);
    if (current) put(current, 0x40, node);
    put(scene, 0x0c, node);
}
} // namespace bsp
