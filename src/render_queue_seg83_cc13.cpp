#include "bsp/render_queue_seg83_cc13.hpp"

#include <intrin.h>

#include <cstring>
#include <stdexcept>

// Segment 83 (packet cc13-seg83). Evidence and ABIs: docs/CC13_SEG83.md and the declarations
// in include/bsp/render_queue_seg83_cc13.hpp.

namespace bsp::cc13_seg83 {
namespace {

template <typename T>
T rd(const void* base, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <typename T>
void wr(void* base, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(base) + offset, &value, sizeof value);
}

std::uint8_t* at(void* base, std::uint32_t offset) { return static_cast<std::uint8_t*>(base) + offset; }

void copy_text(void* dst, const void* src, TerrainHost& host) {
    if (dst != src) host.native_string_assign_0041dd40(dst, rd<const void*>(src, 4), rd<std::uint32_t>(src, 0));
}

}  // namespace

void* render_command_camera_00b1bf40(const void* command) { return rd<void*>(rd<const void*>(command, 0x28), 8); }

std::uint32_t render_queue_mode_00b1bf90(const void* queue) { return rd<std::uint32_t>(queue, 0x20); }

void* render_batch_pool_allocate_slot_00b1bfc0(void* pool) {
    const long n = _InterlockedIncrement(reinterpret_cast<volatile long*>(at(pool, 4)));
    return rd<std::uint8_t*>(pool, 0) + (n - 1) * 0x28;
}

void* pooled_render_batch_construct_00b1c090(void* batch, RenderQueueHost& host) {
    host.render_batch_construct(batch);
    wr<std::uint32_t>(batch, 0, 0x00D5E5AC);
    return batch;
}

void render_queue_set_mode_00b1c460(void* queue, std::uint32_t mode, RenderQueueHost& host) {
    if (rd<std::uint32_t>(queue, 0x20) == 2 && mode == 0) host.renderer_slot11c();
    wr<std::uint32_t>(queue, 0x20, mode);
}

void render_queue_start_worker_00b1c4a0(std::uint32_t a, std::uint32_t b, std::uint32_t c, RenderQueueHost& host) {
    host.renderer_slot118(a, b, c);
}

void render_queue_reset_mode_and_cycle_frame_00b1c4b0(void* queue, RenderQueueHost& host) {
    if (rd<std::uint32_t>(queue, 0x20) == 2) host.renderer_slot11c();
    wr<std::uint32_t>(queue, 0x20, 0);
    host.renderer_slot0c();
    host.renderer_slot14();
}

void* render_label_lock_construct_00b1ca90(void* lock, RenderQueueHost& host) {
    wr<std::uint32_t>(lock, 0, 0x00D5E5D8);
    wr<void*>(lock, 4, host.critical_section_create());
    return lock;
}

void* render_label_lock_get_00b1ccd0(TerrainHost& host, RenderQueueHost& queue) {
    if (host.read_global(0x00F8D43C) == nullptr) {
        host.lifetime_lock(true);
        if (host.read_global(0x00F8D43C) == nullptr) {
            void* memory = host.operator_new(8);
            host.write_global(0x00F8D43C, memory == nullptr ? nullptr : render_label_lock_construct_00b1ca90(memory, queue));
            host.lifetime_register_00bd0c30(host.read_global(0x00F8D43C));
        }
        host.lifetime_unlock(true);
    }
    return host.read_global(0x00F8D43C);
}

void render_lock_destruct(std::uint32_t address, void* lock, TerrainHost& host, RenderQueueHost& queue) {
    std::uint32_t vtable = 0, global = 0;
    switch (address) {
    case 0x00B1C610:
        host.write_global(0x00F8D43C, nullptr);
        wr<std::uint32_t>(lock, 0, bsp::cc13_terrain::kSingletonBaseVTable);
        return;
    case 0x00B1D0B0: vtable = 0x00D5E5D8; global = 0x00F8D43C; break;
    case 0x00B1D500: vtable = 0x00D5E5D4; global = 0x0109DBBC; break;
    default: throw std::invalid_argument("not a render lock destructor");
    }
    wr<std::uint32_t>(lock, 0, vtable);
    queue.critical_section_destroy_owned(lock);
    host.write_global(global, nullptr);
    wr<std::uint32_t>(lock, 0, bsp::cc13_terrain::kSingletonBaseVTable);
}

void render_batch_pool_tidy_00b1d680(void* pool, TerrainHost& host, RenderQueueHost& queue) {
    queue.render_batch_pool_resize_slots(pool, 0);
    host.free(rd<void*>(pool, 0));
}

void render_queue_label_destroy_00b1d830(void* label, TerrainHost& host) {
    bsp::cc13_terrain::release_native_string(label, host);
}

void* render_queue_label_copy_00b1dad0(void* label, const void* source, TerrainHost& host) {
    wr<std::uint32_t>(label, 0, 0);
    wr<std::uint32_t>(label, 4, 0);
    copy_text(label, source, host);
    for (std::uint32_t off : {8u, 0xCu, 0x10u}) wr<std::uint32_t>(label, off, rd<std::uint32_t>(source, off));
    return label;
}

void instance_group_ptr_delete_00b1dcb0(void** slot, TerrainHost& host, RenderQueueHost& queue) {
    void* group = *slot;
    if (group == nullptr) return;
    queue.instance_group_destroy(group);
    host.free(group);
    *slot = nullptr;
}

void render_command_list_delete_all_00b1e720(void* queue, TerrainHost& host, RenderQueueHost& rq) {
    for (std::int32_t i = 0; i < rd<std::int32_t>(queue, 0x18); ++i) {
        void* command = rd<void**>(queue, 0x14)[i];
        if (command == nullptr) continue;
        rq.render_command_destroy(command);
        host.free(command);
    }
    rq.render_command_pointers_resize_cleared(at(queue, 0x14), 0);
}

void render_queue_labels_push_back_00b1e770(void* rows, const void* label, TerrainHost& host, RenderQueueHost& rq) {
    if (rd<std::int32_t>(rows, 4) == rd<std::int32_t>(rows, 8)) {
        std::int32_t n = rd<std::int32_t>(rows, 8) * 2;
        if (n < 2) n = 1;
        rq.render_queue_rows_reserve(rows, n);
    }
    render_queue_label_copy_00b1dad0(rd<std::uint8_t*>(rows, 0) + rd<std::int32_t>(rows, 4) * 0x14, label, host);
    wr<std::int32_t>(rows, 4, rd<std::int32_t>(rows, 4) + 1);
}

void render_queue_add_world_label_00b1eca0(void* queue, const float pos[3], const void* text, TerrainHost& host,
    RenderQueueHost& rq) {
    void* cs = rd<void*>(render_label_lock_get_00b1ccd0(host, rq), 4);
    if (cs != nullptr) {
        rq.cs_enter(cs);
        wr<std::int32_t>(cs, 0x18, rd<std::int32_t>(cs, 0x18) + 1);
    }
    alignas(4) std::uint8_t label[0x14] = {};
    std::memcpy(label + 8, pos, 12);
    copy_text(label, text, host);
    render_queue_labels_push_back_00b1e770(at(queue, 0x24), label, host, rq);
    bsp::cc13_terrain::release_native_string(label, host);
    if (cs != nullptr) {
        wr<std::int32_t>(cs, 0x18, rd<std::int32_t>(cs, 0x18) - 1);
        rq.cs_leave(cs);
    }
}

void** com_ptr_release(std::uint32_t address, void** slot, RenderQueueHost& host) {
    if (address != 0x00B1F7F0 && address != 0x00B20F20 && address != 0x00B20F40)
        throw std::invalid_argument("not a COM release helper");
    if (*slot != nullptr) {
        host.com_release(*slot);
        *slot = nullptr;
    }
    return slot;
}

std::uint32_t com_object_ref_count(std::uint32_t address, void* object, RenderQueueHost& host) {
    switch (address) {
    case 0x00B20E90: case 0x00B20EC0: case 0x00B20EF0: case 0x00B20F60: case 0x00B21070: case 0x00B210A0: break;
    default: throw std::invalid_argument("not a COM ref-count helper");
    }
    if (object == nullptr) return 0;
    const std::uint32_t count = host.com_add_ref(object);
    host.com_release(object);
    return count - 1;
}

void* renderer_shared_vertex_buffer_00b1feb0(const void* renderer) { return rd<void*>(renderer, 0x1974); }

void renderer_show_cursor_00b1ff20(void* renderer, std::uint8_t show, RenderQueueHost& host) {
    host.device_show_cursor(rd<void*>(renderer, 0x1A10), show);
}

void renderer_get_render_target_data_00b1ff70(void* renderer, void* source, void* destination, RenderQueueHost& host) {
    host.device_get_render_target_data(rd<void*>(renderer, 0x1A10), rd<void*>(source, 0x2C), rd<void*>(destination, 0x2C));
}

void renderer_color_fill_00b201d0(void* renderer, void* surface, const std::uint32_t* color, RenderQueueHost& host) {
    host.device_color_fill(rd<void*>(renderer, 0x1A10), rd<void*>(surface, 0x2C), nullptr, *color);
}

void renderer_start_worker_00b21740(void* renderer, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    RenderQueueHost& host) {
    void* worker = rd<void*>(renderer, 0x1970);
    if (worker == nullptr) return;
    host.renderer_set_optional_synchronization(true);
    host.render_worker_start_00b33bc0(worker, a, b, c);
}

void binding_state_reset_00b24160(void* state, TerrainHost& host) {
    for (std::uint32_t off = 0; off < 0x20; off += 4) wr<std::uint32_t>(state, off, 0);
    wr<std::uint8_t>(state, 0x20, 0);
    bsp::cc13_terrain::release_ref_slot(reinterpret_cast<void**>(at(state, 0xA8)), host);
}

void renderer_for_each_resource_slot18_00b24ff0(void* renderer, RenderQueueHost& host) {
    for (std::uint8_t* r = rd<std::uint8_t*>(renderer, 0x1A9C);
         r != rd<std::uint8_t*>(renderer, 0x1A9C) + rd<std::int32_t>(renderer, 0x1AA0) * 0x2C; r += 0x2C)
        host.resource_slot18(rd<void*>(r, 0x28));
}

void* layout_iterator_step(std::uint32_t address, void* iterator, RenderQueueHost& host) {
    if (address == 0x00B23070 || address == 0x00B25CD0) host.layout_tree_advance(iterator);
    else if (address == 0x00B23080 || address == 0x00B25CE0) host.layout_tree_decrement(iterator);
    else throw std::invalid_argument("not a layout iterator thunk");
    return iterator;
}

bool renderer_attach_back_buffer_00b214e0(void* renderer, void* target, SurfaceHost& host) {
    const bool guarded = host.guard_enabled_0108d6dc();
    std::uint32_t token = 0;
    if (guarded) token = host.guard_enter_00b33ad0(renderer);
    void* surface = host.device_get_back_buffer(rd<void*>(renderer, 0x1A10));
    host.target_attach_surface(target, surface);
    if (host.guard_enabled_0108d6dc()) host.guard_leave_00b33b00(token);
    (void)guarded;
    return true;
}

void* renderer_create_system_memory_surface_00b215d0(void* renderer, std::uint32_t a, std::uint32_t b,
    std::uint32_t c, std::uint32_t, SurfaceHost& host) {
    void* surface = nullptr;
    if (host.device_create_offscreen_plain_surface(rd<void*>(renderer, 0x1A10), a, b, c, &surface) != 0) return nullptr;
    void* slot = host.surface_pool_allocate_00b3f2a0();
    void* wrapper = slot == nullptr ? nullptr : host.surface_construct_00b3f630(slot, surface, a);
    host.com_release(surface);
    return wrapper;
}

std::uint32_t record_push_back_element_size(std::uint32_t address) noexcept {
    switch (address) {
    case 0x00B25210: case 0x00B25250: case 0x00B254C0: case 0x00B25500: case 0x00B25540:
    case 0x00B255F0: case 0x00B25900: case 0x00B25990: case 0x00B25A20: case 0x00B25A60:
        return 4;
    case 0x00B1FA50: case 0x00B256A0: return 0x10;
    case 0x00B256F0: return 0x14;
    case 0x00B25830: return 0xC;
    default: return 0;
    }
}

void record_push_back(std::uint32_t address, void* array, const void* element, const RecordReserve& reserve) {
    const std::uint32_t size = record_push_back_element_size(address);
    if (size == 0) throw std::invalid_argument("not a record push-back");
    if (rd<std::int32_t>(array, 4) == rd<std::int32_t>(array, 8)) {
        std::int32_t n = rd<std::int32_t>(array, 8) * 2;
        if (n < 2) n = 1;
        reserve(array, n);
    }
    std::memcpy(rd<std::uint8_t*>(array, 0) + rd<std::int32_t>(array, 4) * size, element, size);
    wr<std::int32_t>(array, 4, rd<std::int32_t>(array, 4) + 1);
}

} // namespace bsp::cc13_seg83
