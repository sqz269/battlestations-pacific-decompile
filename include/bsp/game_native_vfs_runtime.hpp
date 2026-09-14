#pragma once

#include "bsp/native_path_canonicalizer.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace bsp {
class NativeVfsOwnerServices;
struct NativeSingletonDeletionBindings;
struct NativePhysicalProviderPoolContext;
struct NativeRetainedMemoryOwnerContext;
struct NativeStreamTypeIdStorage;
struct NativeFileAccessLogLifetimeBindings;
struct SingletonLifetimeCallbacks;
class NativeVfsEnumerationDuplicateLog;
}
namespace bsp::game {
class GameNativeReadOnlyData;

// Borrow one initialized raw lifetime, physical-provider pool, type-ID set and
// immutable original table image. actual_vfs_storage points to caller-owned A0h
// bytes; the shared singleton drain frees it through the registered owner.
struct GameNativeVfsRuntimeInputs {
    NativeVfsOwnerServices& owners;
    GameNativeReadOnlyData& data;
    void* volatile& actual_manager_publication_01090aa0;
    NativeSingletonDeletionBindings& deletion_bindings;
    NativePhysicalProviderPoolContext& physical_provider_pool;
    NativeRetainedMemoryOwnerContext& retained_memory;
    NativeStreamTypeIdStorage& type_ids;
    const SingletonLifetimeCallbacks& invalid_parameters;
    NativePathCanonicalizerRuntimeServices::Lowercase lowercase_00bf9611;
    void* actual_vfs_storage_a0;
    // These archive bytes are outside GameNativeReadOnlyData's verified .rdata.
    // Supply genuine retained source storage; null is rejected before startup.
    const volatile std::uint8_t* actual_mpkg_xor_key_00e144f0;
    const char* actual_mpak_null_pattern_00e17bf0;
    NativeFileAccessLogLifetimeBindings* file_log_lifetime;
    // Real diagnostic sink and caller-owned readable bytes for the BE1130
    // enumeration visitor. The original 0109CEF0 starts as zero-filled .data;
    // this input does not recreate its fixed address or later mutations.
    // Both inputs are retained through the shared manager drain.
    NativeVfsEnumerationDuplicateLog* enumeration_duplicates;
    const char* actual_empty_name_0109cef0;
};

// Source composition for one raw VFS manager. The caller drains the SAME
// 01090AA0 singleton domain while this object and all borrowed inputs live.
class GameNativeVfsRuntime final {
public:
    explicit GameNativeVfsRuntime(const GameNativeVfsRuntimeInputs&);
    ~GameNativeVfsRuntime();
    GameNativeVfsRuntime(const GameNativeVfsRuntime&) = delete;
    GameNativeVfsRuntime& operator=(const GameNativeVfsRuntime&) = delete;
    GameNativeVfsRuntime(GameNativeVfsRuntime&&) = delete;
    GameNativeVfsRuntime& operator=(GameNativeVfsRuntime&&) = delete;

    void* actual_manager() const noexcept;
    // Keep this object alive if startup throws: drain the shared singleton
    // manager before any bound context or publication cell is destroyed.
    void construct_and_register_core();
    // Call at the native factory tail, after the caller's package/search phase.
    void register_archive_factory_tail(bool cached_load);
    void* mount(const char* system_path, const char* virtual_path,
        std::uint32_t priority, std::uint32_t flags, std::uint32_t device_id);
    void mount_phase2_loose_paths(const char* current_directory_with_separator);
    void scan_phase2_packages();
    void register_phase2_search_defaults();
    // Original order: three loose mounts, two fresh package scans, then the
    // full 00738360 search registrations. Archive factory tail follows later.
    void run_phase2_mount_scan_search(const char* current_directory_with_separator);
    bool exists(const char* path);
    // Read up to capacity bytes; report the source's actual byte count. A null
    // open returns false; unsupported profiles propagate as source errors.
    bool read(const char* path, void* output, std::uint32_t capacity,
        std::uint32_t& bytes_read);
    // Open through this runtime's raw manager and own the complete file bytes.
    // nullopt means no stream opened; an opened empty file is an engaged empty
    // vector. Length, allocation, read, and stream-release failures throw.
    // Partial reads are retried; reaching zero before length is a short read.
    std::optional<std::vector<std::uint8_t>> read_all(const char* path);
    // Call only after the shared 01090AA0 drain. Remove this bundle's borrowed
    // dispatch bindings and clear its matching VFS publication before teardown.
    void retire_after_shared_drain() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
