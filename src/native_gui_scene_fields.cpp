#include "bsp/native_gui_scene_fields.hpp"
#include "bsp/seg3_4_cc13.hpp"
#include <stdexcept>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
void* pointer(const void* p, Word offset) noexcept {
    void* value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(reinterpret_cast<Word>(p) + offset), 4);
    _ReadWriteBarrier();
    return value;
}
void put(void* p, Word offset, void* value) noexcept {
    _ReadWriteBarrier(); std::memcpy(at(p, offset), &value, 4); _ReadWriteBarrier();
}

class SceneResourceRelease final : public seg3_4_cc13::ReferenceSlotHost {
public:
    explicit SceneResourceRelease(NativeGuiSceneFieldsContext& context) : context_(context) {}
    std::int32_t interlocked_decrement_ce2220(void* counter) override {
        return context_.decrement_00ce2220(static_cast<volatile long*>(counter));
    }
    void object_destroy_00(void* object) override {
        auto& reference = context_.owners.resolve_actual(object);
        if (static_cast<void*>(&reference.reference_count) != at(object, 4))
            throw std::logic_error("scene field terminal requires captured owner's actual count");
        reference.release_zero_references();
    }
private:
    NativeGuiSceneFieldsContext& context_;
};
}

void set_native_gui_scene_resource_00b723f0(void* scene,
    const volatile Word& argument, NativeGuiSceneFieldsContext& context) {
    void* const requested = reinterpret_cast<void*>(argument); // B723F0
    void* old = pointer(scene, 0x1c); // B723F5
    if (old == requested) return;
    put(scene, 0x1c, requested);
    if (requested)
        context.increment_00ce221c(static_cast<volatile long*>(at(requested, 4)));
    // B7240D..B72425 inline the same old+4 decrement / virtual0 sequence as
    // 00440A30 (bsp.py show 00440a30). Reuse that body on the captured former
    // scene reference. Its final null store affects this local slot only:
    // scene+1C already owns requested and terminal callbacks may change it.
    SceneResourceRelease release(context);
    seg3_4_cc13::ref_slot_release_00440a30(&old, release);
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
