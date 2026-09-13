#pragma once
#include "bsp/native_render_batch_reference.hpp"
#include "bsp/native_render_group_lifetime.hpp"

namespace bsp {

// Actual44h command. No native count and no implicit field initialization.
// The only table entry at D5E5E0 is command execution B1D950.
struct NativeRenderCommandStorage {
    volatile std::uint32_t native_vtable_00;
    void* scene_04;
    std::uint32_t untouched_08;
    NativeRenderBatchStorage* batches_0c[2];
    std::uint32_t diagnostic_length_14;
    char* diagnostic_data_18;
    std::uint32_t metadata_1c[3];
    NativeRenderContextStorage* context_28;
    NativeRenderPointerArrayStorage indexed_groups_2c;
    NativeRenderPointerArrayStorage ordered_groups_38;
};

struct NativeRenderCommandEnvironment {
    NativeRenderBatchLifetime& batches;
    NativeRenderBatchReferences& batch_references;
    NativeRenderActualOwners& owners;
    NativeRenderGroupModels& models;
    SizedStoragePool& strings;
    const char* default_diagnostic_00ce9a38;
};

// The full command lifetime using the SAME actual419CC0 publication and pool
// as instance collection/group names. Every reached allocation/return reloads
// that publication through ActualNativeStringPoolStorage. No extra owner or
// native field; canonical associations and all other domains remain shared.
struct NativeRenderCommandActualEnvironment {
    NativeRenderBatchLifetime& batches;
    NativeRenderBatchReferences& batch_references;
    NativeRenderActualOwners& owners;
    NativeRenderGroupModels& models;
    ActualNativeStringPoolStorage& strings;
    const char* default_diagnostic_00ce9a38;
};

// Association only: prepare stable host storage/capacity BEFORE entering the
// native initializer. These calls cannot allocate, throw, retain, publish any
// native field, or run ownership behavior. Return the one concrete companion
// borrowing the supplied raw owner, already registered in environment.owners
// (context) or environment.batch_references (batch).
// Context association survives command destruction if another queue/reference
// holds that context. Retirement removes association only after actual cleanup.
class NativeRenderCommandAssociations {
public:
    virtual ~NativeRenderCommandAssociations() = default;
    virtual NativeRenderContextReference& bind_initialized_context(
        NativeRenderContextStorage&) noexcept = 0;
    virtual NativeRenderBatchReference& bind_acquired_batch(
        NativeRenderBatchStorage&) noexcept = 0;
};

// Full B1EDC0: allocate/init/publish actual18h context; live retained assignments
// scene, camera, second owner, target; reload context for each later field;
// acquire two actual pooled batches; set current pooled diagnostic to default.
// ECX=command, four stack pointers, RET10h. No native rollback on failure.
void initialize_native_render_command_00b1edc0(NativeRenderCommandStorage&,
    NativeRenderCommandEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* second_owner, void* target);
// Placement constructors preserve08 and batch0C/10 until their acquisitions.
// On initializer failure clean ordered/indexed arrays and diagnostic only;
// native EH does not release already-published scene/context/batches.
NativeRenderCommandStorage* construct_native_render_command_00b1f1f0(void* actual44h,
    NativeRenderCommandEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* second_owner, void* target);
NativeRenderCommandStorage* construct_native_render_command_00b1f170(void* actual44h,
    NativeRenderCommandEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* target);
// Full B1DDD0; two nonnull valid batches are required. Drop their actual04
// refs without clearing their slots, walk/live-recheck indexed group end,
// destroy/free each group then clear captured cell, release/clear scene/context,
// destroy ordered/indexed arrays then return current name. No command free.
void destroy_native_render_command_00b1ddd0(NativeRenderCommandStorage&,
    NativeRenderCommandEnvironment&);
// Full B1E6B0: destroy, ordinary-free iff flags&1, return original address; RET4.
NativeRenderCommandStorage* delete_native_render_command_00b1e6b0(
    NativeRenderCommandStorage*, NativeRenderCommandEnvironment&, std::uint32_t flags);

// Same complete native schedules/valid-storage domain, with actual string
// allocation and cleanup. Existing environment overloads retain their behavior.
// Actual pool release requires the returning/nonthrowing getter domain of the
// established NativeStringStorage interface; full native FH3/SEH is not implied.
void initialize_native_render_command_00b1edc0(NativeRenderCommandStorage&,
    NativeRenderCommandActualEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* second_owner, void* target);
NativeRenderCommandStorage* construct_native_render_command_00b1f1f0(void* actual44h,
    NativeRenderCommandActualEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* second_owner, void* target);
NativeRenderCommandStorage* construct_native_render_command_00b1f170(void* actual44h,
    NativeRenderCommandActualEnvironment&, NativeRenderCommandAssociations&,
    void* scene, void* camera, void* target);
void destroy_native_render_command_00b1ddd0(NativeRenderCommandStorage&,
    NativeRenderCommandActualEnvironment&);
NativeRenderCommandStorage* delete_native_render_command_00b1e6b0(
    NativeRenderCommandStorage*, NativeRenderCommandActualEnvironment&, std::uint32_t flags);

} // namespace bsp
