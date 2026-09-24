#include "bsp/platform_renderer_activation.hpp"
#include "bsp/native_vfs_date_route.hpp"

#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);
static_assert(offsetof(NativeRenderResourceRecord, payload_14_24) == 0x14);
static_assert(offsetof(NativeRenderResourceRecord, resource_28) == 0x28);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
volatile unsigned char& byte(void* p, Word offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, offset));
}
void refresh_registry_dates(void* registry, NativeRendererActivationContext& c) {
    const Word count = word(registry, 8);
    Word current = word(registry, 4);
    const Word end = current + count * 0x2cu;
    while (current != end) {
        auto* const record = reinterpret_cast<NativeRenderResourceRecord*>(current);
        Word date[5]; // BDD340 writes all five words; no fabricated date.
        c.calls.file_date_00bdd340(c.actual_vfs_0109ceec, date, record);
        bool newer = false;
        for (Word index = 0; index != 5; ++index) {
            const Word previous = word(record, 0x14 + index * 4);
            if (previous < date[index]) { newer = true; break; }
            if (previous > date[index]) break;
        }
        if (newer) {
            for (Word index = 0; index != 5; ++index)
                word(record, 0x14 + index * 4) = date[index];
            void* const resource = pointer(record, 0x28);
            void* const table = pointer(resource, 0);
            c.calls.resource_reload_slot_08(resource, word(table, 8));
        }
        current += 0x2c;
    }
}
} // namespace

NativeRendererActivationVfsBindings::NativeRendererActivationVfsBindings(
    NativeVfsDateRouteContext& dates, NativeStringRawPoolContext& strings) noexcept
    : dates_(dates), strings_(strings) {}

void NativeRendererActivationVfsBindings::file_date_00bdd340(void* manager,
    Word* output, const void* name) {
    query_native_vfs_file_date_00bdd340(manager, output, name, dates_, strings_);
}

void refresh_native_effect_registry_dates_00b22030(void* registry,
    NativeRendererActivationContext& c) { refresh_registry_dates(registry, c); }

void refresh_native_texture_registry_dates_00b21f70(void* registry,
    NativeRendererActivationContext& c) { refresh_registry_dates(registry, c); }

void refresh_native_renderer_material_textures_00b24dd0(void* renderer,
    NativeRendererActivationBindings& calls) {
    Word count = word(renderer, 0x1aa0);
    Word current = word(renderer, 0x1a9c);
    Word end = current + count * 0x2cu;
    while (current != end) {
        calls.refresh_material_textures_00b19000(pointer(
            reinterpret_cast<void*>(current), 0x28));
        count = word(renderer, 0x1aa0);
        end = word(renderer, 0x1a9c) + count * 0x2cu;
        current += 0x2c;
    }
}

void refresh_native_renderer_activation_00b24fb0(void* renderer,
    NativeRendererActivationContext& c) {
    if (c.modes.reload_resources_0108d4bb == 0) return;
    refresh_native_effect_registry_dates_00b22030(at(renderer, 0x1a98), c);
    const Word target = word(pointer(renderer, 0), 0x120);
    if (target != 0x00b24dd0)
        throw std::invalid_argument("B24FD1 unsupported current renderer slot120 target");
    refresh_native_renderer_material_textures_00b24dd0(renderer, c.calls);
    refresh_native_texture_registry_dates_00b21f70(at(renderer, 0x1a74), c);
}

void unlink_native_raw_root_node_00b72220(void* root, void* node) noexcept {
    void* const next = pointer(node, 0x3c);
    if (next) word(next, 0x40) = word(node, 0x40);
    void* const previous = pointer(node, 0x40);
    if (previous) word(previous, 0x3c) = word(node, 0x3c);
    else word(root, 0x0c) = word(node, 0x3c);
}

void clear_native_raw_node_root_00b6d890_null(void* node) noexcept {
    void* const captured_root = pointer(node, 0xa4);
    if (!captured_root && pointer(node, 0x30)) return;
    if (captured_root && !pointer(node, 0x30))
        unlink_native_raw_root_node_00b72220(captured_root, node);
    word(node, 0xa4) = 0;
    void* child = pointer(node, 0x34);
    while (child) {
        clear_native_raw_node_root_00b6d890_null(child);
        child = pointer(child, 0x3c);
    }
}

void mark_native_render_batch_dirty_00b50010(void* batch) noexcept {
    byte(batch, 0x24c) = 1;
}

void invalidate_native_render_root_chain_00b4ecc0(void* owner) noexcept {
    void* captured_root = pointer(owner, 0x3c);
    byte(owner, 0x250) = 1;
    while (pointer(captured_root, 0x0c)) {
        void* const current_root = pointer(owner, 0x3c);
        clear_native_raw_node_root_00b6d890_null(pointer(current_root, 0x0c));
        captured_root = pointer(owner, 0x3c);
    }
    byte(owner, 0x251) = 1;
}

void refresh_native_render_service_focus_00b0d1e0(void* service) noexcept {
    if (byte(service, 0x1c4) == 0) return;
    void* const batch = pointer(service, 0x20);
    if (batch) mark_native_render_batch_dirty_00b50010(batch);
    if (byte(service, 0x1c4) == 0) return;
    void* const owner = pointer(service, 0x30);
    if (owner) invalidate_native_render_root_chain_00b4ecc0(owner);
}
} // namespace bsp
