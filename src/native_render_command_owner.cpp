#include "bsp/native_render_command_owner.hpp"
#include "bsp/storage_pool.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(NativeRenderCommandStorage) == 0x44);
static_assert(offsetof(NativeRenderCommandStorage, scene_04) == 4);
static_assert(offsetof(NativeRenderCommandStorage, batches_0c) == 0x0c);
static_assert(offsetof(NativeRenderCommandStorage, diagnostic_length_14) == 0x14);
static_assert(offsetof(NativeRenderCommandStorage, diagnostic_data_18) == 0x18);
static_assert(offsetof(NativeRenderCommandStorage, context_28) == 0x28);
static_assert(offsetof(NativeRenderCommandStorage, indexed_groups_2c) == 0x2c);
static_assert(offsetof(NativeRenderCommandStorage, ordered_groups_38) == 0x38);

namespace {
char* allocate_diagnostic(SizedStoragePool& strings, std::uint32_t size) {
    return static_cast<char*>(strings.allocate_00bd1120(size));
}
char* allocate_diagnostic(ActualNativeStringPoolStorage& strings, std::uint32_t size) {
    return strings.allocate(size);
}
void return_diagnostic(SizedStoragePool& strings, char* data, std::uint32_t size) {
    strings.release_00bd1510(data, size);
}
void return_diagnostic(ActualNativeStringPoolStorage& strings, char* data,
    std::uint32_t size) noexcept {
    strings.release(data, size);
}
template<class StringStorage>
class DiagnosticCleanup final {
public:
    DiagnosticCleanup(NativeRenderCommandStorage& command, StringStorage& strings) noexcept
        : command_(command), strings_(strings) {}
    ~DiagnosticCleanup() { if (armed_) run(); }
    void disarm() noexcept { armed_ = false; }
    void run() noexcept {
        char* const captured = command_.diagnostic_data_18;
        armed_ = false;
        if (captured) return_diagnostic(strings_, captured, command_.diagnostic_length_14 + 1u);
    }
private:
    NativeRenderCommandStorage& command_;
    StringStorage& strings_;
    bool armed_{true};
};
class ArrayCleanup final {
public:
    ArrayCleanup(NativeRenderPointerArrayStorage& array, bool indexed) noexcept
        : array_(array), indexed_(indexed) {}
    ~ArrayCleanup() { if (armed_) run(); }
    void disarm() noexcept { armed_ = false; }
    void run() {
        armed_ = false;
        if (indexed_) destroy_native_indexed_group_pointers_00b1d260(array_);
        else destroy_native_ordered_group_pointers_00b1d1f0(array_);
    }
private:
    NativeRenderPointerArrayStorage& array_;
    bool indexed_;
    bool armed_{true};
};
template<class T> void release_then_clear(T*& field, NativeRenderActualOwners& owners) {
    T* const captured = field;
    if (captured) {
        release_native_render_actual_owner(owners, captured);
        field = nullptr;
    }
}
std::uintptr_t current_indexed_end(const NativeRenderCommandStorage& command) noexcept {
    return reinterpret_cast<std::uintptr_t>(command.indexed_groups_2c.data_00)
        + static_cast<std::uint32_t>(command.indexed_groups_2c.count_04) * 4u;
}
template<class Environment>
void initialize_command(NativeRenderCommandStorage&, Environment&,
    NativeRenderCommandAssociations&, void*, void*, void*, void*);
template<class Environment>
NativeRenderCommandStorage* construct_command(void* raw,
    Environment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    auto* command = ::new (raw) NativeRenderCommandStorage;
    command->native_vtable_00 = 0x00d5e5e0u;
    command->scene_04 = nullptr;
    command->diagnostic_length_14 = 0;
    command->diagnostic_data_18 = nullptr;
    command->metadata_1c[0] = 0; command->metadata_1c[1] = 0; command->metadata_1c[2] = 0;
    command->context_28 = nullptr;
    command->indexed_groups_2c.data_00 = nullptr;
    command->indexed_groups_2c.count_04 = 0;
    command->indexed_groups_2c.capacity_08 = 0;
    command->ordered_groups_38.data_00 = nullptr;
    command->ordered_groups_38.count_04 = 0;
    command->ordered_groups_38.capacity_08 = 0;
    DiagnosticCleanup diagnostic(*command, environment.strings);
    ArrayCleanup indexed(command->indexed_groups_2c, true);
    ArrayCleanup ordered(command->ordered_groups_38, false);
    initialize_command(*command, environment, associations,
        scene, camera, second_owner, target);
    ordered.disarm(); indexed.disarm(); diagnostic.disarm();
    return command;
}
template<class Environment>
void initialize_command(NativeRenderCommandStorage& command,
    Environment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    auto* context = initialize_native_render_context_00b1edc0_fragment(
        singleton_lifetime_allocate({SingletonAllocationKind::object, 0x18, sizeof(NativeRenderContextStorage)}));
    auto& context_reference = associations.bind_initialized_context(*context);
    if (&context_reference.storage() != context || &context_reference.reference_count != &context->references_04)
        std::terminate();
    command.context_28 = context;
    assign_native_instance_binding_00b1ca50(command.scene_04, &scene, environment.owners);
    assign_native_instance_binding_00b1ca50(command.context_28->camera_08, &camera, environment.owners);
    assign_native_instance_binding_00b1ca50(command.context_28->second_owner_0c, &second_owner, environment.owners);
    assign_native_instance_binding_00b1ca50(command.context_28->target_14, &target, environment.owners);
    command.context_28->borrowed_command_10 = &command;
    for (unsigned i = 0; i < 2; ++i) {
        auto* const pool = environment.batches.pool_00b1e870();
        auto* const batch = environment.batches.acquire_00b1d5b0(pool->slots_04);
        auto& reference = associations.bind_acquired_batch(*batch);
        if (&reference.storage() != batch || &reference.reference_count != &batch->references_04)
            std::terminate();
        command.batches_0c[i] = batch;
    }
    if (command.diagnostic_length_14 != 1u) {
        auto* const replacement = allocate_diagnostic(environment.strings, 2);
        char* const old = command.diagnostic_data_18;
        if (old) return_diagnostic(environment.strings, old, command.diagnostic_length_14 + 1u);
        command.diagnostic_data_18 = replacement;
        command.diagnostic_length_14 = 1;
        replacement[1] = '\0';
    }
    if (char* const current = command.diagnostic_data_18)
        std::memcpy(current, environment.default_diagnostic_00ce9a38, command.diagnostic_length_14);
}
template<class Environment>
void destroy_command(NativeRenderCommandStorage& command,
    Environment& environment) {
    command.native_vtable_00 = 0x00d5e5e0u;
    DiagnosticCleanup diagnostic(command, environment.strings);
    ArrayCleanup indexed(command.indexed_groups_2c, true);
    ArrayCleanup ordered(command.ordered_groups_38, false);
    for (unsigned i = 0; i < 2; ++i) {
        auto* const batch = command.batches_0c[i];
        release_native_render_batch_reference(environment.batch_references, *batch);
    }
    auto cursor = reinterpret_cast<std::uintptr_t>(command.indexed_groups_2c.data_00);
    while (cursor != current_indexed_end(command)) {
        auto** const cell = reinterpret_cast<void**>(cursor);
        auto* const group = static_cast<NativeRenderGroupStorage*>(*cell);
        if (group) {
            destroy_native_render_group_00b1d760(*group, environment.owners, environment.models, environment.strings);
            singleton_lifetime_free(group);
            *cell = nullptr;
        }
        cursor += 4;
    }
    release_then_clear(command.scene_04, environment.owners);
    release_then_clear(command.context_28, environment.owners);
    ordered.run(); indexed.run(); diagnostic.run();
}
} // namespace

void initialize_native_render_command_00b1edc0(NativeRenderCommandStorage& command,
    NativeRenderCommandEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    initialize_command(command, environment, associations, scene, camera, second_owner, target);
}
NativeRenderCommandStorage* construct_native_render_command_00b1f1f0(void* raw,
    NativeRenderCommandEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    return construct_command(raw, environment, associations, scene, camera, second_owner, target);
}
NativeRenderCommandStorage* construct_native_render_command_00b1f170(void* raw,
    NativeRenderCommandEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* target) {
    return construct_command(raw, environment, associations, scene, camera, camera, target);
}
void destroy_native_render_command_00b1ddd0(NativeRenderCommandStorage& command,
    NativeRenderCommandEnvironment& environment) {
    destroy_command(command, environment);
}
NativeRenderCommandStorage* delete_native_render_command_00b1e6b0(
    NativeRenderCommandStorage* command, NativeRenderCommandEnvironment& environment,
    std::uint32_t flags) {
    destroy_command(*command, environment);
    if (flags & 1u) singleton_lifetime_free(command);
    return command;
}

void initialize_native_render_command_00b1edc0(NativeRenderCommandStorage& command,
    NativeRenderCommandActualEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    initialize_command(command, environment, associations, scene, camera, second_owner, target);
}
NativeRenderCommandStorage* construct_native_render_command_00b1f1f0(void* raw,
    NativeRenderCommandActualEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* second_owner, void* target) {
    return construct_command(raw, environment, associations, scene, camera, second_owner, target);
}
NativeRenderCommandStorage* construct_native_render_command_00b1f170(void* raw,
    NativeRenderCommandActualEnvironment& environment, NativeRenderCommandAssociations& associations,
    void* scene, void* camera, void* target) {
    return construct_command(raw, environment, associations, scene, camera, camera, target);
}
void destroy_native_render_command_00b1ddd0(NativeRenderCommandStorage& command,
    NativeRenderCommandActualEnvironment& environment) {
    destroy_command(command, environment);
}
NativeRenderCommandStorage* delete_native_render_command_00b1e6b0(
    NativeRenderCommandStorage* command, NativeRenderCommandActualEnvironment& environment,
    std::uint32_t flags) {
    destroy_command(*command, environment);
    if (flags & 1u) singleton_lifetime_free(command);
    return command;
}
} // namespace bsp
