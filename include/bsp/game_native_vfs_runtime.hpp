#pragma once

#include "bsp/native_path_canonicalizer.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
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
class NativeFileStoreCompletionDispatch;
class NativeVfsRuntimeBindings;
struct NativeVfsNameResolutionContext;
struct NativeVfsDateRouteContext;
}
namespace bsp::game {
class GameNativeReadOnlyData;

// Borrow existing raw services for reconstructed callers such as AF5850.
// This view owns no native storage or invocation frames. Keep the runtime and
// its inputs alive through every consumer and the shared singleton drain;
// failed resolution frames retain their existing process-lifetime obligation.
struct GameNativeVfsRawServices {
    void* volatile& actual_vfs_publication_0109ceec;
    NativeVfsRuntimeBindings& bindings;
    NativeVfsNameResolutionContext& name_resolution;
    NativeVfsDateRouteContext& dates;
};

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
    // Optional borrowed source dispatcher for original FileStore completion
    // callback identities. Retain it through all pending reads and shared drain.
    // Production loading-queue binding is a separate reconstruction packet.
    NativeFileStoreCompletionDispatch* filestore_completions{};
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
    // The publication remains a live reference, not a cached manager pointer.
    // Consumers supply the SAME raw string cells used by inputs.owners, and
    // retain their own actual headers and acquired frames. Borrowing performs
    // no startup, I/O, retry or cleanup and is valid before core registration.
    GameNativeVfsRawServices borrow_raw_services() noexcept;
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
    // BDB0B0 visits the live mount tree and dispatches each current provider+28.
    void pump_pending();
    // Request through BE7CD0 against the CURRENT existing factory cache.
    // Requires a bound completion dispatcher and a previously created FileStore.
    // Resolve/request frames are retained together if a nested call fails.
    bool request_file_store(const char* path, std::uint32_t native_callback);
    std::uint32_t file_store_request_failure_site() const noexcept;
    // BDF4C0 mutates the pooled caller name even on a normal false result.
    // Copy that final spelling back before releasing the complete native frame.
    // An interrupted native call retains its frame/header in this runtime;
    // retain this runtime and all borrowed inputs through process exit then.
    bool resolve_existing(std::string& mutable_name);
    // Diagnostics cover both existing-name and direct-name resolution.
    bool has_failed_name_resolution() const noexcept;
    std::uint32_t name_resolution_failure_site() const noexcept;
    // BDD6E0 tests the normalized input directly against current mounts.
    // Unlike candidate resolution, a normal false leaves output unchanged.
    bool direct_resolve(const std::string& input, std::string& output);
    // BDD990 traverses actual mounts and builds an actual pooled intrusive
    // result list. Copy its ordered, deduplicated names before releasing it.
    std::vector<std::string> enumerate(const char* directory, const char* extension,
        std::uint32_t flags);
    // Preserve BDD340's five raw date words, including its all-zero result.
    std::array<std::uint32_t, 5> file_date(const char* path);
    // Read up to capacity bytes; report the source's actual byte count. A null
    // open returns false; unsupported profiles propagate as source errors.
    bool read(const char* path, void* output, std::uint32_t capacity,
        std::uint32_t& bytes_read);
    // Open through this runtime's raw manager and own the complete file bytes.
    // nullopt means no stream opened; an opened empty file is an engaged empty
    // vector. Length, allocation, read, and stream-release failures throw.
    // Partial reads are retried; reaching zero before length is a short read.
    std::optional<std::vector<std::uint8_t>> read_all(const char* path);
    // Pass the complete original flags to the raw open route. The one-argument
    // overload retains mode 2; this overload includes descriptor mode 32h.
    std::optional<std::vector<std::uint8_t>> read_all(const char* path,
        std::uint32_t flags);
    // Call only after the shared 01090AA0 drain. Remove this bundle's borrowed
    // dispatch bindings and clear its matching VFS publication before teardown.
    void retire_after_shared_drain() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
