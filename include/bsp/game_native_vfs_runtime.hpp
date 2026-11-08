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
class ActualNativeStringPoolStorage;
struct NativeSingletonDeletionBindings;
struct NativePhysicalProviderPoolContext;
struct NativeRetainedMemoryOwnerContext;
struct NativeStreamTypeIdStorage;
struct NativeFileAccessLogLifetimeBindings;
struct SingletonLifetimeCallbacks;
class NativeVfsEnumerationDuplicateLog;
class NativeFileStoreCompletionDispatch;
class NativeGuiIconDelayedRequestFileInvocation;
class NativeGuiIconDelayedDeliveryFileInvocation;
class NativeGuiIconDelayedLoads;
struct NativeGuiIconDelayedDeliveryStorageContext;
class NativeVfsRuntimeBindings;
struct NativeVfsNameResolutionContext;
struct NativeVfsDateRouteContext;
struct NativeVfsOpenRouteContext;
struct NativeStoredStreamConversionContext;
struct NativeVfsEnumerationContext;
struct NativeVfsFileBlockScopeContext;
struct NativeLoadingQueueOwnerStorage;
struct NativeStringRawPoolContext;
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
    // The same retained routes used by the runtime's own opens and stored
    // stream conversion. Callers keep their acquired frames and native owner
    // obligations; borrowing these references performs no I/O or cleanup.
    NativeVfsOpenRouteContext& opens;
    NativeStoredStreamConversionContext& conversion;
    // The SAME application pool wrapper and retained-memory counters/profiles
    // used by these routes. Texture owners borrow them through shared drain;
    // this view creates no second pool, backing owner or accounting domain.
    ActualNativeStringPoolStorage& strings;
    NativeRetainedMemoryOwnerContext& retained_memory;
    NativeVfsEnumerationContext& enumeration;
    const SingletonLifetimeCallbacks& invalid_parameters;
    // SAME retained caller-supplied 0109CEF0 role used by existing VFS routes.
    // Distinct from E17654; borrowing creates no buffer or fallback provider.
    const char* actual_empty_name_0109cef0;
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

// Narrow borrowed metadata for one raw BEE390 context. These references name
// the runtime's EXISTING null-source role and current-locale library binding;
// this view owns no context, raw bytes, string pool or Native operation.
struct GameNativeVfsRawCanonicalizerProviders final {
    const char& actual_null_source_0109db91;
    const NativePathCanonicalizerRuntimeServices::Lowercase& current_locale_lowercase_00bf9611;
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
    // After actual core registration, before retirement. Admission reads the
    // CURRENT Native VFS publication cell and compares the retained allocation;
    // no raw owner-prefix read, getter, callback, write, I/O or cleanup occurs.
    GameNativeVfsRawCanonicalizerProviders borrow_raw_canonicalizer_providers() const;
    // Borrow the actual scope domain only after the complete archive tail.
    // Checks the current retained manager/registry/lock and cached-load state;
    // performs no allocation, publication, callback, I/O or cleanup. Keep this
    // runtime and its borrowed inputs alive until all blocks close, before drain.
    NativeVfsFileBlockScopeContext& borrow_fileblock_scopes();
    // 00509190's stopped-front branch, using this runtime's existing factory,
    // provider, FileBlock scope and SAME raw string domain. Retain this runtime
    // and caller's queue on escape; no job is created or worker started here.
    void retire_loading_queue_front(NativeLoadingQueueOwnerStorage*,
        NativeStringRawPoolContext&);
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
    // Manager vtable+0Ch: only mounts whose actual payload+0Ch byte is set.
    bool exists_in_flagged_mounts(const char* path);
    // BDB0B0 visits the live mount tree and dispatches each current provider+28.
    void pump_pending();
    // Request through BE7CD0 against the CURRENT existing factory cache.
    // Requires a bound completion dispatcher and a previously created FileStore.
    // Resolve/request frames are retained together if a nested call fails.
    bool request_file_store(const char* path, std::uint32_t native_callback);
    // Whole BE7AB0 over the SAME mounted factory cache and raw streams.
    // An escaping operation retains its name/frame until the caller stops;
    // no retry or destructor cleanup replays the interrupted native releases.
    void cache_file_store(const char* path, std::uint32_t flags);
    // Cold Source-only preparation for the whole raw Icon request. Fresh
    // Runtime-owned node/context/resolver/request backing, no Native call or
    // name copy. Caller retains this Runtime through actual I/O/callback last-
    // use and shared drain; normal phase is not a quiescence/owner certificate.
    NativeGuiIconDelayedRequestFileInvocation& prepare_icon_delayed_request_file_call();
    // Cold once-bound Source dispatcher for actual AB58D0 only. The const
    // input's borrowed dispatcher is never overwritten/replaced. Retain the
    // SAME service/context/feature/renderer/opaque-owner graph through all
    // actual pending/callback/queued/worker last use and shared drain.
    void bind_icon_delayed_completion(NativeGuiIconDelayedLoads&,
        NativeGuiIconDelayedDeliveryStorageContext&);
    // Fresh opaque THIS-Runtime footer call; only the actual inline callback
    // child may enter. No Native work, name snapshot, legacy/request reuse.
    NativeGuiIconDelayedDeliveryFileInvocation& prepare_icon_delayed_delivery_file_call();
    std::uint32_t file_store_request_failure_site() const noexcept;
    // BDD600 normalizes the actual mutable header, probes with D683E8 and
    // replaces it on success. Copy its final spelling back on TRUE and FALSE.
    // Exceptions propagate; this is not BDD6E0 or candidate/extension search.
    bool probe_and_replace_file_name(std::string& mutable_name);
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
    friend class GameNativeVfsApplication;
    // Application-only opt-in: its completed-initialization gate must precede
    // the runtime/owner/preimage checks. Borrowing extends no lifetime. Callers
    // exclude concurrent publication/callback changes and finish uses before
    // shared shutdown starts; keep the existing owners and code through drain.
    void* publish_and_borrow_raw_failure_manager();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
