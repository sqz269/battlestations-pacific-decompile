#include "bsp/game_native_vfs_runtime.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_filestore_factory.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_filestore_completion.hpp"
#include "bsp/native_filestore_request.hpp"
#include "bsp/native_mpak_storage_services.hpp"
#include "bsp/native_mpkg_runtime.hpp"
#include "bsp/native_pak_registry.hpp"
#include "bsp/native_physical_enumeration.hpp"
#include "bsp/native_physical_factory.hpp"
#include "bsp/native_physical_provider.hpp"
#include "bsp/native_physical_provider_pool.hpp"
#include "bsp/native_physical_pending_io.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_stream_type_ids.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_derived_manager.hpp"
#include "bsp/native_vfs_date_route.hpp"
#include "bsp/native_vfs_enumeration.hpp"
#include "bsp/native_vfs_factory_registration.hpp"
#include "bsp/native_vfs_lookup_routes.hpp"
#include "bsp/native_vfs_mount_registration.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/native_vfs_open_route.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include "bsp/native_vfs_package_scan.hpp"
#include "bsp/native_vfs_provider_enumeration_bindings.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_vfs_search_defaults.hpp"
#include "bsp/native_vfs_startup_callbacks.hpp"
#include "bsp/platform_window.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <stdexcept>
#include <limits>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {
std::uint32_t word(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(p) + offset);
}
void put_word(void* p, std::size_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(p) + offset) = value;
}
void put_byte(void* p, std::size_t offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(
        static_cast<std::byte*>(p) + offset) = value;
}
const void* required(const GameNativeReadOnlyData& data,
    std::uintptr_t address, std::size_t bytes) {
    return data.data_at(address, bytes);
}
NativeVfsEnumerationDuplicateLog& required_log(
    NativeVfsEnumerationDuplicateLog* logger) {
    if (!logger) throw std::invalid_argument("Native VFS duplicate logger is required");
    return *logger;
}
struct PooledHeader {
    NativeString value{};
    ActualNativeStringPoolStorage& strings;
    PooledHeader(ActualNativeStringPoolStorage& storage, const char* text)
        : strings(storage) {
        if (!text) throw std::invalid_argument("Native VFS path is null");
        try {
            value.assign_0041e870(strings, text);
        } catch (...) {
            value.release_to(strings);
            throw;
        }
    }
    ~PooledHeader() { value.release_to(strings); }
    PooledHeader(const PooledHeader&) = delete;
    PooledHeader& operator=(const PooledHeader&) = delete;
};
struct NameResolutionInvocation {
    // Declaration order keeps the mutable caller alive until after its frame.
    PooledHeader name;
    PooledHeader input;
    NativeVfsNameResolutionAcquired acquired;
    NameResolutionInvocation(ActualNativeStringPoolStorage& strings, const char* text,
        const char* original = "") : name(strings, text), input(strings, original) {}
};
struct FileStoreRequestInvocation {
    // The nested resolver borrows the request frame's actual mutable header.
    // Both acquired owners must outlive all interrupted native work.
    PooledHeader original;
    NativeFileStoreRequestAcquired acquired;
    std::unique_ptr<NativeVfsNameResolutionAcquired> resolution;
    FileStoreRequestInvocation(ActualNativeStringPoolStorage& strings, const char* text)
        : original(strings, text) {}
};
struct PooledNameList {
    ActualNativeStringPoolStorage& strings;
    // BDD990's list has an unwritten +0 preimage, sentinel at +4, count at +8.
    alignas(4) std::byte value[0x0c];
    explicit PooledNameList(ActualNativeStringPoolStorage& storage) : strings(storage) {
        put_word(value, 4, reinterpret_cast<std::uint32_t>(
            allocate_native_render_alias_sentinel_004c3020()));
        put_word(value, 8, 0);
    }
    ~PooledNameList() {
        destroy_native_render_alias_list_004d0a10(value, strings);
    }
    NativeRenderResourceAliasNode* sentinel() const noexcept {
        return reinterpret_cast<NativeRenderResourceAliasNode*>(word(value, 4));
    }
    PooledNameList(const PooledNameList&) = delete;
    PooledNameList& operator=(const PooledNameList&) = delete;
};
// The stream returned by the manager carries one caller reference. Keep its
// release paired with that open even when length, allocation, or read throws.
struct OpenedStream {
    NativeVfsRuntimeBindings& bindings;
    void* stream;
    std::uintptr_t table;
    OpenedStream(NativeVfsRuntimeBindings& source, void* value)
        : bindings(source), stream(value), table(word(value)) {}
    OpenedStream(const OpenedStream&) = delete;
    OpenedStream& operator=(const OpenedStream&) = delete;
    ~OpenedStream() {
        // During unwinding, preserve the original failure. The successful
        // path calls release explicitly so a release failure still propagates.
        if (stream) {
            try { release(); } catch (...) {}
        }
    }
    void release() {
        if (!stream) return;
        void* const owned = std::exchange(stream, nullptr);
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
                static_cast<std::byte*>(owned) + 4)) == 0)
            bindings.zero_reference(table, owned);
    }
};
}

struct GameNativeVfsRuntime::Impl : NativePhysicalPendingCompletionDispatch,
    NativeFileStoreCompletionDispatch, NativeFileStoreRequestDispatch {
    const GameNativeVfsRuntimeInputs inputs;
    void* volatile file_log_0109cee8{};
    void* volatile physical_factory_0109dbe8{};
    void* volatile filestore_factory_0109db68{};
    void* volatile mpkg_factory_010904f4{};
    void* volatile mpak_factory_010904d4{};
    void* volatile pak_registry_010904d8{};
    void* volatile mpak_lock_010904e0{};
    void* volatile mpak_cached_provider_010904dc{};
    SharedLock mpak_lock_storage_{};
    char null_source_0109db91{};

    NativePathCanonicalizerRuntimeServices path_services;
    NativePathCanonicalizerContext canonicalizer;
    NativeVfsLookupRouteContext lookup;
    NativeVfsOpenRouteContext open;
    NativeVfsOpenLoggingContext logging;
    NativeStoredStreamConversionContext conversion;
    NativePhysicalProviderContext physical_provider;
    NativePhysicalPendingIoContext physical_pending;
    NativeVfsRuntimeBindings bindings;
    NativeVfsPendingRouteContext pending;
    NativeFileStoreCompletionContext filestore_completion;
    NativeFileStoreRequestContext filestore_request_context;
    std::unique_ptr<FileStoreRequestInvocation> filestore_request;

    NativeMpkgRuntimeServices mpkg_services;
    NativeMpkgDirectoryContext mpkg_directory;
    NativeMpkgArchiveContext mpkg_archive;
    NativeMpkgArchiveRuntimeOperations mpkg_archive_operations;
    NativeMpkgProviderContext mpkg_provider;
    NativeMpkgProviderContext* previous_mpkg{};

    NativePakRegistryContext pak_registry_context;
    NativeMpakStorageServices mpak_storage;
    NativeMpakStorageRuntimeDependencies mpak_dependencies;
    NativeMpakRuntimeInputs mpak_inputs;
    NativeMpakRuntime mpak_runtime;
    NativeVfsNameResolutionContext name_resolution_context;
    std::unique_ptr<NameResolutionInvocation> name_resolution;
    NativeVfsDateRouteContext date_context;

    NativePhysicalFactoryContext physical_factory_context;
    NativeFileStoreFactoryContext filestore_factory_context;
    NativeMpkgFactoryContext mpkg_factory_context;
    NativeMpakFactoryContext mpak_factory_context;
    NativeVfsManagerLifetimeContext manager_context;
    NativeVfsStartupCallbacks failure;
    NativeVfsMountRegistrationContext mount_context;
    NativePhysicalEnumerationContext physical_enumeration;
    NativeVfsProviderEnumerationInputs enumeration_provider_inputs;
    NativeVfsProviderEnumerationBindings enumeration_providers;
    NativeVfsEnumerationContext enumeration_context;
    NativeVfsPackageScanContext package_scan_context;
    bool core_started{};
    bool core_registered{};
    bool archive_tail_registered{};
    bool retired{};

    explicit Impl(const GameNativeVfsRuntimeInputs& source)
        : inputs(source),
          path_services(inputs.owners.string_pool_publication_01090aa8(),
              inputs.owners.string_returns_disabled_01090aa4(),
              inputs.actual_manager_publication_01090aa0,
              inputs.lowercase_00bf9611),
          canonicalizer{inputs.owners.strings(), path_services, &null_source_0109db91},
          lookup{inputs.owners.physical(), required(inputs.data, 0x00d68398, 12),
              required(inputs.data, 0x00d683e8, 12), nullptr},
          open{inputs.owners.physical(), required(inputs.data, 0x00d6838c, 12),
              nullptr, nullptr},
          logging{file_log_0109cee8, inputs.owners.strings()},
          conversion{inputs.retained_memory,
              reinterpret_cast<const volatile std::uint32_t*>(
                  &inputs.type_ids.memory_0109dba0),
              &inputs.owners.streams(),
              reinterpret_cast<const volatile std::uint32_t*>(
                  &inputs.type_ids.physical_0109dc30),
              nullptr,
              reinterpret_cast<const volatile std::uint32_t*>(
                  &inputs.type_ids.file_0109db58)},
          physical_provider{inputs.physical_provider_pool, inputs.owners.strings(),
              inputs.invalid_parameters,
              static_cast<const char*>(required(inputs.data, 0x00cff208, 1))},
          physical_pending{inputs.owners.physical(), inputs.retained_memory, *this},
          bindings(open, lookup, inputs.owners.streams(), conversion, logging,
              inputs.retained_memory, &physical_provider, inputs.file_log_lifetime, &physical_pending),
          pending{lookup, required(inputs.data, 0x00d68478, 12), bindings},
          filestore_completion{inputs.owners.strings(), inputs.invalid_parameters, bindings, *this},
          filestore_request_context{inputs.owners.strings(), inputs.invalid_parameters,
              inputs.owners.vfs_publication_0109ceec(), *this},
          mpkg_services(bindings, path_services, inputs.retained_memory),
          mpkg_directory{inputs.owners.strings(), bindings, mpkg_services},
          mpkg_archive{inputs.owners.vfs_publication_0109ceec(), conversion,
              mpkg_directory, mpkg_services, inputs.actual_mpkg_xor_key_00e144f0},
          mpkg_archive_operations(mpkg_archive),
          mpkg_provider{inputs.owners.strings(), mpkg_archive_operations},
          pak_registry_context{inputs.actual_manager_publication_01090aa0,
              pak_registry_010904d8},
          mpak_storage(inputs.owners.strings(), bindings, path_services,
              inputs.invalid_parameters),
          mpak_dependencies{lookup, required(inputs.data, 0x00d683f4, 12),
              pak_registry_context, conversion,
              inputs.owners.vfs_publication_0109ceec(), mpak_lock_010904e0,
              mpak_cached_provider_010904dc,
              inputs.actual_mpak_null_pattern_00e17bf0},
          mpak_inputs(mpak_storage.runtime_inputs(mpak_dependencies)),
          mpak_runtime(bindings, mpak_inputs),
          name_resolution_context{mpak_runtime.device_context(), logging,
              inputs.actual_empty_name_0109cef0},
          date_context{inputs.owners.physical(), required(inputs.data, 0x00d683b0, 12)},
          physical_factory_context{inputs.actual_manager_publication_01090aa0,
              physical_factory_0109dbe8},
          filestore_factory_context{inputs.actual_manager_publication_01090aa0,
              filestore_factory_0109db68},
          mpkg_factory_context{inputs.actual_manager_publication_01090aa0,
              mpkg_factory_010904f4},
          mpak_factory_context{inputs.actual_manager_publication_01090aa0,
              mpak_factory_010904d4},
          manager_context{inputs.owners.vfs_publication_0109ceec(), file_log_0109cee8,
              inputs.owners.lifetime(), inputs.owners.strings(), canonicalizer,
              {static_cast<const char*>(required(inputs.data, 0x00d68464, 1)),
               static_cast<const char*>(required(inputs.data, 0x00d68454, 1)),
               static_cast<const char*>(required(inputs.data, 0x00d68444, 1)),
               static_cast<const char*>(required(inputs.data, 0x00d68438, 1)),
               static_cast<const char*>(required(inputs.data, 0x00d68414, 1)),
               static_cast<const char*>(required(inputs.data, 0x00d68400, 1))},
              inputs.invalid_parameters, bindings},
          mount_context{inputs.owners.vfs_publication_0109ceec(), canonicalizer,
              inputs.invalid_parameters, bindings, failure},
          physical_enumeration{inputs.owners.physical(), canonicalizer},
          enumeration_provider_inputs{physical_enumeration, inputs.owners.strings(),
              inputs.invalid_parameters, mpkg_directory,
              inputs.actual_mpak_null_pattern_00e17bf0, mpak_runtime},
          enumeration_providers(enumeration_provider_inputs),
          enumeration_context{inputs.owners.strings(), inputs.invalid_parameters,
              enumeration_providers, required_log(inputs.enumeration_duplicates),
              inputs.actual_empty_name_0109cef0,
              required(inputs.data, 0x00d6846c, 12)},
          package_scan_context{inputs.owners.vfs_publication_0109ceec(),
              inputs.owners.strings(), inputs.invalid_parameters,
              enumeration_context, mount_context} {
        if (!inputs.actual_vfs_storage_a0 || !inputs.lowercase_00bf9611 ||
            !inputs.actual_mpkg_xor_key_00e144f0 ||
            !inputs.actual_mpak_null_pattern_00e17bf0 ||
            !inputs.enumeration_duplicates || !inputs.actual_empty_name_0109cef0)
            throw std::invalid_argument("Native VFS requires actual owner and archive services");
        required(inputs.data, 0x00d68d04, 0x38);
        required(inputs.data, 0x00cfea14, 0x10);
        required(inputs.data, 0x00d68cfc, 0x10);
        required(inputs.data, 0x00d688b4, 0x10);
        lookup.pending = &pending;

        previous_mpkg = bindings.bind_mpkg_provider(&mpkg_provider);
        inputs.owners.bind_deletion(inputs.deletion_bindings);
        inputs.deletion_bindings.physical_factory = &physical_factory_context;
        inputs.deletion_bindings.filestore_factory = &filestore_factory_context;
        inputs.deletion_bindings.mpkg_factory = &mpkg_factory_context;
        inputs.deletion_bindings.mpak_factory = &mpak_factory_context;
        inputs.deletion_bindings.pak_registry = &pak_registry_context;
        inputs.deletion_bindings.vfs_manager = &manager_context;

    }
    void invoke_00bf476d(std::uint32_t callback, void* stream,
        const void* first, const void* second) override {
        if (callback != 0x00be7b20)
            throw std::invalid_argument("Unimplemented native physical completion callback");
        dispatch_native_file_store_completion_00be7b20(stream, first, second,
            filestore_factory_context, filestore_completion);
    }
    void invoke_00be7942(std::uint32_t callback, const void* first,
        const void* second) override {
        if (!inputs.filestore_completions)
            throw std::invalid_argument("Native FileStore completion dispatcher is not bound");
        inputs.filestore_completions->invoke_00be7942(callback, first, second);
    }
    bool resolve_existing_name_00bdf4c0(void* manager, void* name) override {
        if (!filestore_request || filestore_request->resolution)
            throw std::logic_error("Native FileStore resolver retains an invocation");
        filestore_request->resolution = std::make_unique<NativeVfsNameResolutionAcquired>();
        const bool found = resolve_native_vfs_existing_name_00bdf4c0(manager, name,
            name_resolution_context, *filestore_request->resolution);
        filestore_request->resolution.reset();
        return found;
    }
    bool open_file_overlapped_00bdda10(void* manager, const void* first,
        const void* second, std::uint32_t callback, std::uint32_t flags) override {
        return submit_native_vfs_pending_00bdda10(manager, first, second, callback, flags, pending) != 0;
    }
    void invoke_manager_failure_callback(std::uintptr_t target, void* manager) override {
        bindings.open_failure_entry(target, manager);
    }
    void register_core() {
        if (core_started)
            throw std::logic_error("Native VFS core construction cannot be retried");
        core_started = true;
        // BEDA60 appends physical. The startup's next two getter/register pairs
        // append FileStore then MPKG; MPAK belongs to the later factory tail.
        construct_native_vfs_derived_manager_00beda60(
            inputs.actual_vfs_storage_a0, manager_context, physical_factory_context);
        inputs.owners.vfs_publication_0109ceec() = inputs.actual_vfs_storage_a0;
        install_native_vfs_startup_callbacks_0073d63c(
            inputs.owners.vfs_publication_0109ceec());
        register_native_vfs_provider_factory_00be0660(inputs.actual_vfs_storage_a0,
            nullptr, get_native_filestore_factory_004fc150(filestore_factory_context));
        register_native_vfs_provider_factory_00be0660(inputs.actual_vfs_storage_a0,
            nullptr, get_native_mpkg_factory_00736a90(mpkg_factory_context));
        core_registered = true;
    }
    ~Impl() {
        bindings.bind_mpkg_provider(previous_mpkg);
        if (archive_tail_registered) {
            mpak_lock_010904e0 = nullptr;
            DeleteCriticalSection(&mpak_lock_storage_.section);
        }
    }
    void retire_after_drain() noexcept {
        if (retired) return;
        auto& deletion = inputs.deletion_bindings;
        if (deletion.physical_factory == &physical_factory_context)
            deletion.physical_factory = nullptr;
        if (deletion.filestore_factory == &filestore_factory_context)
            deletion.filestore_factory = nullptr;
        if (deletion.mpkg_factory == &mpkg_factory_context)
            deletion.mpkg_factory = nullptr;
        if (deletion.mpak_factory == &mpak_factory_context)
            deletion.mpak_factory = nullptr;
        if (deletion.pak_registry == &pak_registry_context)
            deletion.pak_registry = nullptr;
        if (deletion.vfs_manager == &manager_context)
            deletion.vfs_manager = nullptr;
        if (inputs.owners.vfs_publication_0109ceec() == inputs.actual_vfs_storage_a0)
            inputs.owners.vfs_publication_0109ceec() = nullptr;
        retired = true;
    }
    void register_archive_tail(bool cached_load) {
        if (!core_registered)
            throw std::logic_error("Native VFS core must be registered first");
        if (archive_tail_registered) return;
        register_native_vfs_provider_factory_00be0660(inputs.actual_vfs_storage_a0,
            nullptr, get_native_mpak_factory_00736b60(mpak_factory_context));
        void* const registry = get_native_pak_registry_00736c30(pak_registry_context);
        put_word(inputs.actual_vfs_storage_a0, 0x88,
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(registry)));
        put_byte(inputs.actual_vfs_storage_a0, 0x78,
            static_cast<std::uint8_t>(cached_load));
        auto* const lock = reinterpret_cast<SharedLock*>(&mpak_lock_storage_);
        SharedLock* published = nullptr;
        if (!create_shared_lock_00bb40b0(published, *lock))
            throw std::runtime_error("Cannot create actual MPAK registry lock");
        mpak_lock_010904e0 = published;
        archive_tail_registered = true;
    }
};

GameNativeVfsRuntime::GameNativeVfsRuntime(const GameNativeVfsRuntimeInputs& inputs)
    : impl_(std::make_unique<Impl>(inputs)) {}
GameNativeVfsRuntime::~GameNativeVfsRuntime() = default;
void* GameNativeVfsRuntime::actual_manager() const noexcept {
    return impl_->inputs.actual_vfs_storage_a0;
}
GameNativeVfsRawServices GameNativeVfsRuntime::borrow_raw_services() noexcept {
    return {impl_->inputs.owners.vfs_publication_0109ceec(), impl_->bindings,
        impl_->name_resolution_context};
}
void GameNativeVfsRuntime::construct_and_register_core() {
    impl_->register_core();
}
void GameNativeVfsRuntime::register_archive_factory_tail(bool cached_load) {
    impl_->register_archive_tail(cached_load);
}
void* GameNativeVfsRuntime::mount(const char* system, const char* virtual_path,
    std::uint32_t priority, std::uint32_t flags, std::uint32_t device_id) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    PooledHeader system_name(impl_->inputs.owners.strings(), system);
    PooledHeader virtual_name(impl_->inputs.owners.strings(), virtual_path);
    return mount_native_vfs_system_path_00be1890(actual_manager(), &system_name.value,
        &virtual_name.value, priority, flags, device_id, impl_->mount_context);
}
void GameNativeVfsRuntime::mount_phase2_loose_paths(const char* root) {
    // The three verified 0073D6A2..0073D881 requests.
    mount(root, ".", 0, 1, static_cast<std::uint32_t>(-1));
    mount(root, "persistent_data", 99, 1, static_cast<std::uint32_t>(-1));
    mount("filestore", ".", 300, 0, static_cast<std::uint32_t>(-1));
}
void GameNativeVfsRuntime::scan_phase2_packages() {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    scan_native_vfs_packages_0073cb10(impl_->package_scan_context);
}
void GameNativeVfsRuntime::register_phase2_search_defaults() {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    register_native_vfs_search_defaults_00738360(
        impl_->inputs.owners.vfs_publication_0109ceec(),
        impl_->inputs.owners.strings(), impl_->inputs.invalid_parameters);
}
void GameNativeVfsRuntime::run_phase2_mount_scan_search(const char* root) {
    mount_phase2_loose_paths(root);
    scan_phase2_packages();
    scan_phase2_packages();
    register_phase2_search_defaults();
}
bool GameNativeVfsRuntime::exists(const char* path) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    PooledHeader name(impl_->inputs.owners.strings(), path);
    return impl_->bindings.exists(word(actual_manager()), actual_manager(), name.value) != 0;
}
void GameNativeVfsRuntime::pump_pending() {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    if (file_store_request_failure_site())
        throw std::logic_error("Native FileStore retains an interrupted invocation");
    // 737B79 loads the publication afresh immediately before BDB0B0.
    pump_native_vfs_pending_00bdb0b0(impl_->inputs.owners.vfs_publication_0109ceec(), impl_->pending);
}
bool GameNativeVfsRuntime::request_file_store(const char* path, std::uint32_t callback) {
    if (!impl_->core_registered || !impl_->inputs.filestore_completions)
        throw std::logic_error("Native FileStore request requires core and completion binding");
    if (impl_->filestore_request)
        throw std::logic_error("Native FileStore retains an invocation");
    auto* const factory = get_native_filestore_factory_004fc150(impl_->filestore_factory_context);
    auto* const store = reinterpret_cast<void*>(word(factory, 8));
    if (!store) throw std::logic_error("Native FileStore factory has no current provider");
    impl_->filestore_request = std::make_unique<FileStoreRequestInvocation>(
        impl_->inputs.owners.strings(), path);
    auto& invocation = *impl_->filestore_request;
    try {
        const bool accepted = request_native_file_store_00be7cd0(store,
            &invocation.original.value, callback, impl_->filestore_request_context, invocation.acquired);
        impl_->filestore_request.reset();
        return accepted;
    } catch (...) {
        const auto phase = invocation.acquired.phase();
        if (phase == NativeFileStoreRequestPhase::fresh || phase == NativeFileStoreRequestPhase::complete)
            impl_->filestore_request.reset();
        throw;
    }
}
std::uint32_t GameNativeVfsRuntime::file_store_request_failure_site() const noexcept {
    return impl_->filestore_request ? impl_->filestore_request->acquired.failure_site() : 0;
}
bool GameNativeVfsRuntime::resolve_existing(std::string& mutable_name) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    if (impl_->name_resolution)
        throw std::logic_error("Native VFS name resolution retains an interrupted invocation");
    // Publish the complete caller/frame owner before entering native source.
    // Unknown provider targets throw with the failed frame still inspectable.
    impl_->name_resolution = std::make_unique<NameResolutionInvocation>(
        impl_->inputs.owners.strings(), mutable_name.c_str());
    auto& invocation = *impl_->name_resolution;
    try {
        const bool found = resolve_native_vfs_existing_name_00bdf4c0(actual_manager(),
            &invocation.name.value, impl_->name_resolution_context, invocation.acquired);
        const auto& name = invocation.name.value;
        mutable_name.assign(name.data() ? name.data() : "", name.length());
        impl_->name_resolution.reset();
        return found;
    } catch (...) {
        const auto phase = invocation.acquired.phase();
        if (phase == NativeVfsNameResolutionPhase::fresh ||
            phase == NativeVfsNameResolutionPhase::complete)
            impl_->name_resolution.reset();
        throw;
    }
}
bool GameNativeVfsRuntime::has_failed_name_resolution() const noexcept {
    return impl_->name_resolution &&
        impl_->name_resolution->acquired.phase() == NativeVfsNameResolutionPhase::failed;
}
std::uint32_t GameNativeVfsRuntime::name_resolution_failure_site() const noexcept {
    return impl_->name_resolution ? impl_->name_resolution->acquired.failure_site() : 0;
}
bool GameNativeVfsRuntime::direct_resolve(const std::string& input, std::string& output) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    if (impl_->name_resolution)
        throw std::logic_error("Native VFS name resolution retains an interrupted invocation");
    impl_->name_resolution = std::make_unique<NameResolutionInvocation>(
        impl_->inputs.owners.strings(), output.c_str(), input.c_str());
    auto& invocation = *impl_->name_resolution;
    try {
        const bool found = resolve_native_vfs_direct_name_00bdd6e0(actual_manager(),
            &invocation.input.value, &invocation.name.value,
            impl_->name_resolution_context, invocation.acquired);
        if (found)
            output.assign(invocation.name.value.data() ? invocation.name.value.data() : "",
                invocation.name.value.length());
        impl_->name_resolution.reset();
        return found;
    } catch (...) {
        const auto phase = invocation.acquired.phase();
        if (phase == NativeVfsNameResolutionPhase::fresh ||
            phase == NativeVfsNameResolutionPhase::complete)
            impl_->name_resolution.reset();
        throw;
    }
}
std::vector<std::string> GameNativeVfsRuntime::enumerate(const char* directory,
    const char* extension, std::uint32_t flags) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    auto& strings = impl_->inputs.owners.strings();
    PooledHeader directory_name(strings, directory), extension_name(strings, extension);
    PooledNameList names(strings);
    enumerate_native_vfs_resources_00bdd990(actual_manager(), &directory_name.value,
        &extension_name.value, flags, names.value, impl_->enumeration_context);
    std::vector<std::string> output;
    output.reserve(word(names.value, 8));
    auto* const head = names.sentinel();
    for (auto* node = head->next_00; node != head; node = node->next_00)
        output.emplace_back(node->string_data_0c ? node->string_data_0c : "",
            node->string_length_08);
    return output;
}
std::array<std::uint32_t, 5> GameNativeVfsRuntime::file_date(const char* path) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    PooledHeader name(impl_->inputs.owners.strings(), path);
    std::array<std::uint32_t, 5> date;
    query_native_vfs_file_date_00bdd340(actual_manager(), date.data(), &name.value,
        impl_->date_context);
    return date;
}
bool GameNativeVfsRuntime::read(const char* path, void* output,
    std::uint32_t capacity, std::uint32_t& bytes_read) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    if (!output && capacity)
        throw std::invalid_argument("Native VFS read output is null");
    PooledHeader name(impl_->inputs.owners.strings(), path);
    void* const stream = impl_->bindings.open(word(actual_manager()), actual_manager(),
        name.value, 2);
    bytes_read = 0;
    if (!stream) return false;
    OpenedStream opened(impl_->bindings, stream);
    impl_->bindings.read(opened.table, stream, output, capacity, &bytes_read);
    opened.release();
    return true;
}
std::optional<std::vector<std::uint8_t>> GameNativeVfsRuntime::read_all(
    const char* path) {
    return read_all(path, 2);
}
std::optional<std::vector<std::uint8_t>> GameNativeVfsRuntime::read_all(
    const char* path, std::uint32_t flags) {
    if (!impl_->core_registered)
        throw std::logic_error("Native VFS core is not registered");
    PooledHeader name(impl_->inputs.owners.strings(), path);
    void* const stream = impl_->bindings.open(word(actual_manager()), actual_manager(),
        name.value, flags);
    if (!stream) return std::nullopt;
    OpenedStream opened(impl_->bindings, stream);
    const std::uint64_t length = impl_->bindings.length(opened.table, stream);
    std::vector<std::uint8_t> bytes;
    const std::uint64_t win32_count_limit =
        (std::min)(static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)()),
            static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()));
    if (length > win32_count_limit || length > bytes.max_size())
        throw std::length_error("Native VFS file exceeds Win32 read/allocation limit");
    bytes.resize(static_cast<std::size_t>(length));
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const auto requested = static_cast<std::uint32_t>(bytes.size() - offset);
        std::uint32_t actual = 0;
        impl_->bindings.read(opened.table, stream, bytes.data() + offset,
            requested, &actual);
        if (actual == 0 || actual > requested)
            throw std::runtime_error("Native VFS full-file read was short or invalid");
        offset += actual;
    }
    opened.release();
    return std::move(bytes);
}
void GameNativeVfsRuntime::retire_after_shared_drain() noexcept {
    impl_->retire_after_drain();
}
} // namespace bsp::game
