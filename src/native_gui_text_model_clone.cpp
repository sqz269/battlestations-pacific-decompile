#include "bsp/native_gui_text_model_clone.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Text model clone fragments require MSVC Win32.
#endif

namespace bsp {
namespace {
void require_pair(NativeModelOwner& source, NativeModelOwner& destination) {
    if (&source == &destination || &source.environment != &destination.environment ||
        source.phase != NativeModelOwner::Phase::live ||
        destination.phase != NativeModelOwner::Phase::live)
        throw std::logic_error("model clone requires distinct live Models in the same native environment");
    auto& scenes = source.environment.nodes.scenes;
    if (&scenes.resolve(source.node.transform) != &source.node.scene_attachment ||
        &scenes.resolve(destination.node.transform) != &destination.node.scene_attachment)
        throw std::logic_error("model clone requires the same canonical scene bindings");
}
void require_model_slot(NativeModelOwner& owner, std::uint32_t byte_offset,
    std::uint32_t expected) {
    const auto* const profile = owner.environment.vtable_00d62de8;
    if (owner.storage.node.vtable_00 != 0x00d62de8u || !profile ||
        profile[byte_offset / 4u] != expected)
        throw std::logic_error("model clone has no implementation for the current native virtual slot");
}
void retain_actual(void* identity) noexcept {
    auto* const count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(identity) + 4));
    count->fetch_add(1, std::memory_order_seq_cst);
}
void copy_x87_word(const void* source, void* destination, std::uint32_t byte_offset) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        mov ecx, byte_offset
        fld dword ptr [eax + ecx]
        fstp dword ptr [edx + ecx]
    }
}
void assign_node_retained130(NativeModelOwner& source, NativeModelOwner& destination,
    bool source_first) {
    void* incoming;
    void* previous;
    if (source_first) { // B6F270 source load precedes destination load.
        incoming = source.storage.node.retained_130;
        previous = destination.storage.node.retained_130;
    } else { // B6F1E4 destination load precedes source load.
        previous = destination.storage.node.retained_130;
        incoming = source.storage.node.retained_130;
    }
    if (incoming == previous) return;
    destination.storage.node.retained_130 = incoming;
    if (incoming) retain_actual(incoming);
    if (previous) destination.environment.nodes.release_retained_owner(previous);
}
} // namespace

NativeGuiTextModelBaseCopyResult copy_native_gui_text_model_base_00b6f150_fragment(
    NativeModelOwner& source, NativeModelOwner& destination) {
    require_pair(source, destination);
    auto& src = source.storage.node;
    auto& dst = destination.storage.node;
    // Native signed comparison atB6F15C. Existing generated point-light
    // vectors cannot stand in for the physical source/reverse-link owners.
    if (src.point_lights_164.count > 0)
        return NativeGuiTextModelBaseCopyResult::point_light_owners_required;

    // Current50 is captured before the scalar stores; no native callback
    // occurs between these loads and its invocation.
    require_model_slot(destination, 0x50, 0x00b6ed80u);
    const auto attach = destination.node.scene_attachment.attach_scene;
    if (!attach) throw std::logic_error("model clone requires actual current scene attachment");
    copy_x87_word(&src, &dst, 0xac);
    copy_x87_word(&src, &dst, 0x4c);
    attach(destination.environment.nodes.scenes, destination.node.scene_attachment,
        src.scene_170, true);
    assign_node_retained130(source, destination, false);

    // Text's flags26h include20h; B6F223..246 skips all source-child clones.
    set_native_node_parent_null_00b6e680(destination.environment.nodes,
        destination.node.transform);
    propagate_native_node_root_00b6d890(destination.environment.nodes,
        destination.node.transform, src.root_list_a4);

    require_model_slot(destination, 0x38, 0x00b6db10u);
    // The shared matrix helper needs the real existing+A0 notification route.
    // Missing host bindings are diagnosed, never silently treated as no-op.
    if ((dst.valid_flags_5c & 0xau) && dst.notification_context_a0 &&
        !destination.node.transform.notify_changed)
        throw std::logic_error("model local-matrix copy requires actual attached-object virtual3C");
    set_transform_local_matrix_00b6db10(destination.node.transform, src.local_b0);
    assign_node_retained130(source, destination, true);

    // Requested parent is the caller's saved null, even if callbacks changed
    // the destination hierarchy. Clear only+A0, not an invented backlink.
    dst.notification_context_a0 = nullptr;
    destination.node.transform.notify_changed = nullptr; // SAME binding metadata.
    dst.auxiliary_flags_138 = src.auxiliary_flags_138;
    copy_x87_word(&src, &dst, 0x13c);
    copy_x87_word(&src, &dst, 0x140);
    copy_x87_word(&src, &dst, 0x144);
    copy_x87_word(&src, &dst, 0x148);
    dst.mask_48 = src.mask_48;
    return NativeGuiTextModelBaseCopyResult::copied;
}

void associate_native_gui_text_model_clone_geometry_00b752b0_fragment(
    NativeModelOwner& source, NativeModelOwner& destination,
    NativeMeshStorage*& acquired_geometry) {
    require_pair(source, destination);
    auto* const captured = acquired_geometry;
    if (!captured) throw std::invalid_argument("model clone geometry requires its actual acquired mesh");
    auto& owners = destination.environment.retained_owners;
    auto* const reference = dynamic_cast<NativeMeshReference*>(&owners.resolve_actual(captured));
    if (!reference || &reference->storage() != captured)
        throw std::logic_error("model clone geometry does not match the canonical native mesh owner");
    float scalar_17c;
    float scalar_178;
    const void* const raw_source = &source.storage.node;
    __asm {
        mov eax, raw_source
        fld dword ptr [eax + 17ch]
        fstp scalar_17c
        fld dword ptr [eax + 178h]
        fstp scalar_178
    }
    set_native_model_geometry_00b75170(destination, 0, captured, scalar_178, scalar_17c);
    acquired_geometry = nullptr;
    release_native_render_actual_owner(owners, captured);
}

void finish_native_gui_text_model_clone_00b752b0_fragment(
    NativeModelOwner& source, NativeModelOwner& destination) {
    require_pair(source, destination);
    void* const incoming = source.storage.model.retained_174;
    void* const previous = destination.storage.model.retained_174;
    if (incoming != previous) {
        destination.storage.model.retained_174 = incoming;
        if (incoming) retain_actual(incoming);
        if (previous)
            release_native_render_actual_owner(destination.environment.retained_owners, previous);
    }
    for (std::uint32_t byte_offset = 8; byte_offset <= 0x2c; byte_offset += 4)
        copy_x87_word(&source.storage.node, &destination.storage.node, byte_offset);
}
} // namespace bsp
