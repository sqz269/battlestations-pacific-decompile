#pragma once
// Partition segment 83 (00B1BF40..00B25D90): the render command queue, its label lock and
// batch pool, the D3D9 renderer's small device wrappers and the renderer's record arrays
// (packet cc13-seg83). Evidence, original ABIs and uncertainty: docs/CC13_SEG83.md.
//
// New C++ interfaces over host data in image layout (byte offsets on void*, Win32 pointers),
// not drop-in binary replacements. Image routines and COM calls are virtuals of
// RenderQueueHost named by address or by the IDirect3DDevice9 / IDirect3D9 slot; nothing is
// stubbed and no global is invented. Names are hypotheses. Status: reconstructed and
// build-tested; not fixture-tested, ABI-compatible or game-validated.

#include <cstdint>
#include <functional>

#include "bsp/terrain_particles_cc13.hpp"

namespace bsp::cc13_seg83 {

using bsp::cc13_terrain::TerrainHost;

struct RenderQueueHost {
    virtual ~RenderQueueHost() = default;
    // The renderer singleton [00F8D394]: vtable +0Ch (BeginFrame), +14h (EndFrame), +118h
    // (00B21740 StartWorker, three arguments), +11Ch (leave mode 2).
    virtual void renderer_slot0c() = 0;
    virtual void renderer_slot14() = 0;
    virtual void renderer_slot118(std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void renderer_slot11c() = 0;
    // BSP_RenderBatch_Construct (ECX = batch); BSP_CriticalSection_Create;
    // BSP_CriticalSection_DestroyOwned (ECX = owner of the section at +04).
    virtual void render_batch_construct(void* batch) = 0;
    virtual void* critical_section_create() = 0;
    virtual void critical_section_destroy_owned(void* owner) = 0;
    // Win32 Enter/LeaveCriticalSection on an image section.
    virtual void cs_enter(void* cs) = 0;
    virtual void cs_leave(void* cs) = 0;
    // BSP_RenderBatchPool_ResizeSlots(ECX = pool, n); BSP_InstanceGroup_Destroy;
    // BSP_RenderCommand_Destroy; BSP_RenderCommandPointers_ResizeCleared(ECX = array, n);
    // BSP_RenderQueueRows_Reserve(ECX = rows, n).
    virtual void render_batch_pool_resize_slots(void* pool, std::int32_t n) = 0;
    virtual void instance_group_destroy(void* group) = 0;
    virtual void render_command_destroy(void* command) = 0;
    virtual void render_command_pointers_resize_cleared(void* array, std::int32_t n) = 0;
    virtual void render_queue_rows_reserve(void* rows, std::int32_t n) = 0;
    // COM: IUnknown AddRef (+04h) / Release (+08h); IDirect3DDevice9 ShowCursor (+30h),
    // GetRenderTargetData (+80h), ColorFill (+8Ch); RenderWorker start and the optional
    // synchronisation switch (BSP_Renderer_SetOptionalSynchronization(1),
    // BSP_RenderWorker_Start = 00B33BC0 on the worker).
    virtual std::uint32_t com_add_ref(void* object) = 0;
    virtual std::uint32_t com_release(void* object) = 0;
    virtual void device_show_cursor(void* device, std::uint8_t show) = 0;
    virtual void device_get_render_target_data(void* device, void* source, void* destination) = 0;
    virtual void device_color_fill(void* device, void* surface, const void* rect, std::uint32_t color) = 0;
    virtual void renderer_set_optional_synchronization(bool enabled) = 0;
    virtual void render_worker_start_00b33bc0(void* worker, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    // Tree iterator steps (ECX = iterator).
    virtual void layout_tree_advance(void* iterator) = 0;
    virtual void layout_tree_decrement(void* iterator) = 0;
    // A resource record's +28h object, vtable +18h.
    virtual void resource_slot18(void* object) = 0;
};

// 00B1BF40 BSP_RenderCommand_GetCamera, __fastcall(command) -> EAX: [[command+28h]+08].
void* render_command_camera_00b1bf40(const void* command);
// 00B1BF90, __fastcall(queue) -> EAX: +20h (the queue mode / retention control).
std::uint32_t render_queue_mode_00b1bf90(const void* queue);
// 00B1BFC0 BSP_RenderBatchPool_AllocateSlot, __fastcall(pool) -> EAX: n =
// InterlockedIncrement(+04); returns data + (n - 1) * 28h (no bounds check).
void* render_batch_pool_allocate_slot_00b1bfc0(void* pool);
// 00B1C090, __fastcall(batch) -> batch: BSP_RenderBatch_Construct, vtable 00D5E5AC.
void* pooled_render_batch_construct_00b1c090(void* batch, RenderQueueHost& host);
// 00B1C460 BSP_RenderCommandQueue_SetMode, __thiscall(queue, mode), RET 4: when +20h == 2 and
// mode == 0 the renderer leaves mode 2 (+11Ch) first; +20h = mode.
void render_queue_set_mode_00b1c460(void* queue, std::uint32_t mode, RenderQueueHost& host);
// 00B1C4A0 BSP_RenderCommandQueue_StartWorker: a tail jump to the renderer's +118h with the
// caller's three stack arguments (cleaned up by the target, RET 0Ch).
void render_queue_start_worker_00b1c4a0(std::uint32_t a, std::uint32_t b, std::uint32_t c, RenderQueueHost& host);
// 00B1C4B0, __fastcall(queue): mode 2 -> renderer +11Ch; +20h = 0; renderer +0Ch then +14h.
void render_queue_reset_mode_and_cycle_frame_00b1c4b0(void* queue, RenderQueueHost& host);

// The label lock (8 bytes {vtable 00D5E5D8, critical section*}) published at [00F8D43C].
// 00B1CA90 construct (__fastcall): vtable, +04 = BSP_CriticalSection_Create().
// 00B1CCD0 get: double-checked under the lifetime lock (depth counter bumped), operator
// new(8) + 00B1CA90, register; returns the global.
// 00B1D0B0 destruct (__fastcall): vtable, destroy the owned section, global = 0, vtable
// 00CE3818. 00B1C610 is that tail alone (global = 0, vtable 00CE3818).
// 00B1D500 is the same destructor for the batch lock (vtable 00D5E5D4, global 0109DBBC).
void* render_label_lock_construct_00b1ca90(void* lock, RenderQueueHost& host);
void* render_label_lock_get_00b1ccd0(TerrainHost& host, RenderQueueHost& queue);
void render_lock_destruct(std::uint32_t address, void* lock, TerrainHost& host, RenderQueueHost& queue);

// 00B1D680, __fastcall(pool): ResizeSlots(0), then _free(data).
void render_batch_pool_tidy_00b1d680(void* pool, TerrainHost& host, RenderQueueHost& queue);
// Labels (14h): NativeString text +00/+04, float position +08..+10.
// 00B1D830 destroy text (__fastcall); 00B1DAD0 copy (__thiscall(dst, const src*), RET 4).
void render_queue_label_destroy_00b1d830(void* label, TerrainHost& host);
void* render_queue_label_copy_00b1dad0(void* label, const void* source, TerrainHost& host);
// 00B1DCB0, __fastcall(slot): when *slot: BSP_InstanceGroup_Destroy, _free, *slot = 0.
void instance_group_ptr_delete_00b1dcb0(void** slot, TerrainHost& host, RenderQueueHost& queue);
// 00B1E720, __fastcall(queue): each command of +14h (count +18h) destroyed and freed; the
// array resized to 0 (BSP_RenderCommandPointers_ResizeCleared).
void render_command_list_delete_all_00b1e720(void* queue, TerrainHost& host, RenderQueueHost& rq);
// 00B1E770, __thiscall(rows {data, count, capacity}, const label*), RET 4: when full
// RenderQueueRows_Reserve(2 * capacity, or 1); copy-construct at data + count * 14h; count++.
void render_queue_labels_push_back_00b1e770(void* rows, const void* label, TerrainHost& host, RenderQueueHost& rq);
// 00B1ECA0 BSP_RenderCommandQueue_AddWorldLabel, __thiscall(queue, const float pos[3], const
// NativeString* text), RET 8: under the label lock's section (entered and left directly;
// the depth counter at cs+18h is bumped around it), a local label {text copy, pos} is pushed
// into the +24h rows and its text released.
void render_queue_add_world_label_00b1eca0(void* queue, const float pos[3], const void* text, TerrainHost& host,
    RenderQueueHost& rq);

// COM helpers. 00B1F7F0 / 00B20F20 / 00B20F40, __fastcall(slot) -> slot: Release and clear.
// 00B20E90 / 00B20EC0 / 00B20EF0 / 00B20F60 / 00B21070 / 00B210A0, __fastcall(object) -> EAX:
// 0 for null, else AddRef(), Release(), returns the AddRef result - 1 (the reference count).
void** com_ptr_release(std::uint32_t address, void** slot, RenderQueueHost& host);
std::uint32_t com_object_ref_count(std::uint32_t address, void* object, RenderQueueHost& host);

// D3D9 renderer wrappers (device at renderer+1A10h; resources keep their D3D object at +2Ch).
// 00B1FEB0, __fastcall(renderer) -> EAX: +1974h (borrowed, no AddRef).
// 00B1FF20, __thiscall(renderer, byte), RET 4: IDirect3DDevice9::ShowCursor(byte).
// 00B1FF70, __thiscall(renderer, source, destination), RET 8: GetRenderTargetData.
// 00B201D0, __thiscall(renderer, surface, const D3DCOLOR*), RET 8: ColorFill(surface+2Ch,
// null rect, *color).
// 00B21740, __thiscall(renderer, a, b, c), RET 0Ch: when the worker +1970h is set, optional
// synchronisation on and 00B33BC0(a, b, c) on the worker; else nothing.
void* renderer_shared_vertex_buffer_00b1feb0(const void* renderer);
void renderer_show_cursor_00b1ff20(void* renderer, std::uint8_t show, RenderQueueHost& host);
void renderer_get_render_target_data_00b1ff70(void* renderer, void* source, void* destination, RenderQueueHost& host);
void renderer_color_fill_00b201d0(void* renderer, void* surface, const std::uint32_t* color, RenderQueueHost& host);
void renderer_start_worker_00b21740(void* renderer, std::uint32_t a, std::uint32_t b, std::uint32_t c,
    RenderQueueHost& host);
// 00B24160 BSP_D3D9BindingState_Reset, __fastcall(state): eight dwords and byte +20h zeroed;
// ref slot +A8h released.
void binding_state_reset_00b24160(void* state, TerrainHost& host);
// 00B24FF0, __fastcall(renderer): for each 2Ch record of +1A9Ch (count +1AA0h, re-read each
// step) the record's +28h object vtable +18h.
void renderer_for_each_resource_slot18_00b24ff0(void* renderer, RenderQueueHost& host);
// Iterator thunks, __fastcall(iterator) -> iterator: 00B23070 / 00B25CD0 advance,
// 00B23080 / 00B25CE0 decrement.
void* layout_iterator_step(std::uint32_t address, void* iterator, RenderQueueHost& host);

struct SurfaceHost {
    virtual ~SurfaceHost() = default;
    // Optional render guard when the byte [0108D6DC] is set: 00B33AD0 (ECX = renderer)
    // enters and returns a token; 00B33B00(token) leaves.
    virtual bool guard_enabled_0108d6dc() = 0;
    virtual std::uint32_t guard_enter_00b33ad0(void* renderer) = 0;
    virtual void guard_leave_00b33b00(std::uint32_t token) = 0;
    // IDirect3DDevice9::GetBackBuffer (+48h)(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) and
    // CreateOffscreenPlainSurface (+90h)(w, h, format, D3DPOOL_SYSTEMMEM, &surface, null) ->
    // HRESULT.
    virtual void* device_get_back_buffer(void* device) = 0;
    virtual std::int32_t device_create_offscreen_plain_surface(void* device, std::uint32_t w, std::uint32_t h,
        std::uint32_t format, void** surface) = 0;
    // A render target's vtable +14h(surface).
    virtual void target_attach_surface(void* target, void* surface) = 0;
    // D3D9SurfacePool_AllocateCanonicalSlot (00B3F2A0, ECX = 34h) and BSP_D3D9Surface_Construct
    // (00B3F630, ECX = slot, surface, a, 0) -> wrapper.
    virtual void* surface_pool_allocate_00b3f2a0() = 0;
    virtual void* surface_construct_00b3f630(void* slot, void* surface, std::uint32_t a) = 0;
    virtual void com_release(void* object) = 0;
};
// 00B214E0 (renderer vtable 00D5F0A8 slot +7Ch), __thiscall(renderer, target), RET 4 -> AL =
// 1: inside the optional guard, GetBackBuffer into a local and target +14h(surface); the
// local reference is not released here.
bool renderer_attach_back_buffer_00b214e0(void* renderer, void* target, SurfaceHost& host);
// 00B215D0 (slot +84h), __thiscall(renderer, a, b, c, d), RET 10h -> EAX:
// CreateOffscreenPlainSurface(a, b, c, SYSTEMMEM); on S_OK a 34h canonical slot is
// constructed around the surface with (surface, a, 0) (the bytes push the FIRST argument,
// 00B2163B; d is not read), the creation reference is released and the wrapper (or null
// when the slot allocation failed) returned; on failure null.
void* renderer_create_system_memory_surface_00b215d0(void* renderer, std::uint32_t a, std::uint32_t b,
    std::uint32_t c, std::uint32_t d, SurfaceHost& host);

// Record-array push backs {data, count, capacity}, __thiscall(array, const T*), RET 4:
// when full, the named reserve with 2 * capacity (1 below 2); the element is copied as
// dwords. Element size per address: 4 for 00B25210, 00B25250, 00B254C0, 00B25500, 00B25540,
// 00B255F0, 00B25900, 00B25990, 00B25A20, 00B25A60; 10h for 00B1FA50 and 00B256A0; 14h for
// 00B256F0; 0Ch for 00B25830.
using RecordReserve = std::function<void(void* array, std::int32_t n)>;
std::uint32_t record_push_back_element_size(std::uint32_t address) noexcept;  // 0 when unknown
void record_push_back(std::uint32_t address, void* array, const void* element, const RecordReserve& reserve);

} // namespace bsp::cc13_seg83
