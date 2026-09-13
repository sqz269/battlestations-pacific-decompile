#include "bsp/native_mpak_runtime.hpp"

#include "bsp/native_filestore_open.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"

#include <stdexcept>

namespace bsp {
namespace {
std::uint32_t word(const void* owner, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(owner) + offset);
}
[[noreturn]] void unsupported() {
    throw std::invalid_argument("Unimplemented captured native MPAK method");
}
} // namespace

NativeMpakRuntime::NativeMpakRuntime(NativeVfsRuntimeBindings& vfs,
    const NativeMpakRuntimeInputs& inputs)
    : vfs_(vfs), lookup_(inputs.lookup), conversion_(inputs.conversion),
      directory_search_(inputs.directory_search), null_pattern_(inputs.null_pattern_00e17bf0),
      directory_{inputs.strings, vfs, inputs.allocation_and_pool, inputs.containers},
      provider_{inputs.strings, inputs.registry, directory_,
          inputs.actual_manager_publication_0109ceec, *this},
      cache_{provider_, inputs.actual_lock_publication_010904e0,
          inputs.actual_cached_provider_010904dc},
      provider_operations_(cache_),
      create_{inputs.strings, provider_operations_, inputs.actual_lock_publication_010904e0,
          inputs.actual_cached_provider_010904dc},
      entry_{inputs.containers, vfs, inputs.conversion, *this},
      open_{inputs.strings, inputs.invalid_parameters, inputs.actual_manager_publication_0109ceec,
          inputs.open_library, *this},
      device_{inputs.lookup, inputs.actual_device_profile_00d683f4, *this},
      binding_{create_, provider_, open_},
      previous_mpak_(vfs.bind_mpak_provider(&binding_)),
      previous_device_(inputs.lookup.device),
      previous_substreams_(inputs.conversion.adopted_substreams) {
    lookup_.device = &device_;
    conversion_.adopted_substreams = &vfs_;
}
NativeMpakRuntime::~NativeMpakRuntime() {
    vfs_.restore_mpak_provider(&binding_, previous_mpak_);
    if (lookup_.device == &device_) lookup_.device = previous_device_;
    if (conversion_.adopted_substreams == &vfs_)
        conversion_.adopted_substreams = previous_substreams_;
}
void* NativeMpakRuntime::open_manager_00bb82c8(std::uintptr_t entry,
    void* manager, const void* name, std::uint32_t flags) {
    return vfs_.open_manager_entry(entry, manager, name, flags);
}
std::int32_t NativeMpakRuntime::select_device_00bdd850(void* manager,
    const void* name, const void* unused_name) {
    return select_native_vfs_device_00bdd850(manager, name, unused_name, device_);
}
void* NativeMpakRuntime::materialize_entry_00bb5080(void* provider, std::int32_t index) {
    return materialize_native_mpak_entry_00bb5080(provider, index, entry_);
}
std::uint64_t NativeMpakRuntime::source_position(std::uintptr_t entry, void* stream) {
    return vfs_.stream_position_entry(entry, stream);
}
std::uint8_t NativeMpakRuntime::invoke_resolve(std::uintptr_t entry, void* provider,
    const void* suffix, void* output) {
    switch (entry) {
    case 0x00bf0fb0:
        return resolve_native_provider_logical_name_00bf0fb0(provider, suffix, output, lookup_);
    case 0x00bb68f0:
        return select_native_mpak_member_directory_00bb68f0(provider, suffix, output,
            directory_, directory_search_);
    default: unsupported();
    }
}
void NativeMpakRuntime::enumerate_entry(std::uintptr_t entry, void* provider,
    const void* prefix, const void* extension, std::uint32_t flags,
    NativeStringVectorStorage& output) {
    if (entry != 0x00bb5f40) unsupported();
    append_native_mpak_names_00bb5f40(provider, prefix, extension, flags,
        output, directory_, null_pattern_);
}
bool NativeMpakRuntime::reject_operation_entry(std::uintptr_t entry,
    std::uint32_t first, std::uint32_t second, std::uint32_t third, std::uint32_t fourth) {
    if (entry != 0x00bb79e0) unsupported();
    return reject_native_mpak_operation_00bb79e0(first, second, third, fourth);
}
void* NativeMpakRuntime::clear_result_entry(std::uintptr_t entry, void* output,
    std::uint32_t ignored) {
    if (entry != 0x00bb7a00) unsupported();
    return clear_native_mpak_result_00bb7a00(output, ignored);
}
void NativeMpakRuntime::noop_entry(std::uintptr_t entry) {
    if (entry != 0x00bb79f0) unsupported();
    noop_native_mpak_default_00bb79f0();
}
std::uint64_t position_native_memory_stream_00bef580(const void* stream) noexcept {
    const auto* const backing = reinterpret_cast<const void*>(word(stream, 8));
    const auto cursor = word(stream, 0x10);
    const auto difference = cursor - word(backing, 8);
    return static_cast<std::uint64_t>(static_cast<std::int64_t>(
        static_cast<std::int32_t>(difference)));
}
std::uint64_t position_native_physical_stream_00bf4f40(const void* stream) noexcept {
    const auto low = word(stream, 0x10);
    const auto high = word(stream, 0x14);
    return (static_cast<std::uint64_t>(high) << 32) | low;
}
} // namespace bsp
