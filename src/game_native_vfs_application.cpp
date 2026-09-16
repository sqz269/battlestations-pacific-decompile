#include "bsp/game_native_vfs_application.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_vfs_constants.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/native_vfs_enumeration.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cctype>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <utility>

namespace bsp::game {
namespace {
void reject_native_vfs_parameter(void*) {
    throw std::invalid_argument("native VFS validation failed");
}
}

struct GameNativeVfsApplication::Impl {
    struct DuplicateLog final : NativeVfsEnumerationDuplicateLog {
        explicit DuplicateLog(GameHostLog& sink) noexcept : log(sink) {}
        void rejected_duplicate(const char* name) override {
            log.notef("native VFS duplicate: %s", name ? name : "(null)");
        }
        GameHostLog& log;
    };

    GameSingletonHost& singleton_host;
    GameNativeReadOnlyData& data;
    GameNativeVfsConstants constants;

    // These are application cells. The manager cell is the singleton host's
    // existing 01090AA0 publication, never a second raw manager domain.
    void* volatile vfs_0109ceec{};
    void* volatile stream_pool_0109dc28{};
    NativeRenderBatchPoolStorage* volatile batch_pool_0108fe8c{};
    NativeRenderBatchLockOwner* volatile batch_lock_0109dbbc{};
    NativeStringPoolStorage* volatile string_pool_01090aa8{};
    volatile std::uint32_t string_returns_disabled_01090aa4{};
    TypeIdCounterStorage* volatile type_counter_0109db7c{};
    NativeVfsPublicationCells cells;
    SingletonLifetimeCallbacks validation{nullptr, nullptr, &reject_native_vfs_parameter};
    NativeVfsOwnerServices owner_services;
    NativeStringRawPoolContext raw_strings;
    GameNativeTypeStorage type_storage;
    LightTypeBootstrap common_types;

    volatile std::uint32_t retained_objects_0109db98{};
    volatile std::uint32_t retained_bytes_0109db9c{};
    NativeRetainedMemoryOwnerContext retained_memory;
    DuplicateLog duplicates;
    char empty_name_0109cef0{};

    // The A0h allocation passes to the shared manager's deleting destructor
    // after registration begins. An unknown partial native state is retained.
    void* manager_storage_a0{};
    std::unique_ptr<GameNativeVfsRuntime> vfs_runtime;
    bool initialization_attempted{};
    std::optional<int> pool_registration_status;

    Impl(GameHostLog& log, GameSingletonHost& host, GameNativeReadOnlyData& mapped,
        const std::filesystem::path& original_executable)
        : singleton_host(host), data(mapped), constants(original_executable),
          cells{host.manager_publication_01090aa0(), vfs_0109ceec,
              stream_pool_0109dc28, batch_pool_0108fe8c, batch_lock_0109dbbc,
              string_pool_01090aa8, string_returns_disabled_01090aa4,
              type_counter_0109db7c},
          owner_services(cells,
              static_cast<const volatile std::uint32_t*>(
                  mapped.data_at(0x00d5e5ac, sizeof(std::uint32_t))),
              validation),
          raw_strings{owner_services.string_pool_publication_01090aa8(),
              owner_services.string_returns_disabled_01090aa4(),
              host.manager_publication_01090aa0()},
          common_types(owner_services.types(), type_storage.light_types()),
          retained_memory{retained_objects_0109db98, retained_bytes_0109db9c,
              static_cast<const volatile std::uint32_t*>(
                  mapped.data_at(0x00d15ad8, sizeof(std::uint32_t))),
              static_cast<const volatile std::uint32_t*>(
                  mapped.data_at(0x00d642c0, sizeof(std::uint32_t)))},
          duplicates(log) {
        // Borrowed raw string services are available before VFS core startup.
        // Bind their actual shared-drain contexts before any such consumer can
        // create the string pool or another owner through those services.
        owner_services.bind_deletion(singleton_host.native_deletion_bindings());
    }

    void initialize_core() {
        if (initialization_attempted)
            throw std::logic_error("native VFS application initialization cannot be retried");
        initialization_attempted = true;

        auto& deletion = singleton_host.native_deletion_bindings();
        type_storage.initialize_resource_types(owner_services.types(), common_types);
        type_storage.initialize_memory_00cd8fc0(owner_services.types(), common_types);
        auto& process_pool = game_native_physical_pool_process();
        pool_registration_status = process_pool.initialize_once_00cd9010();
        type_storage.initialize_physical_00cd9030(owner_services.types(), common_types);
        auto& physical_pool = process_pool.physical_provider_pool_context_0109dbf0();

        manager_storage_a0 = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0xa0, 0xa0});
        if (!manager_storage_a0) throw std::bad_alloc();

        GameNativeVfsRuntimeInputs inputs{owner_services, data,
            singleton_host.manager_publication_01090aa0(), deletion, physical_pool,
            retained_memory, type_storage.stream_types(), validation, &std::tolower,
            manager_storage_a0, constants.mpkg_xor_key_00e144f0(),
            constants.null_pattern_00e17bf0(), nullptr, &duplicates,
            &empty_name_0109cef0};
        try {
            // No native constructor or registration has touched A0h yet. The
            // runtime constructor validates data and installs source bindings.
            vfs_runtime = std::make_unique<GameNativeVfsRuntime>(inputs);
        } catch (...) {
            singleton_lifetime_free(manager_storage_a0);
            manager_storage_a0 = nullptr;
            throw;
        }

        singleton_host.bind_vfs_runtime(vfs_runtime.get());
        // If this throws, neither the runtime nor A0h can be unwound here:
        // registration may have partially published or registered native state.
        vfs_runtime->construct_and_register_core();
    }
};

GameNativeVfsApplication::GameNativeVfsApplication(GameHostLog& log,
    GameSingletonHost& singleton_host, GameNativeReadOnlyData& data,
    const std::filesystem::path& original_executable)
    : impl_(std::make_unique<Impl>(log, singleton_host, data, original_executable)) {}

GameNativeVfsApplication::~GameNativeVfsApplication() = default;

void GameNativeVfsApplication::initialize_core() { impl_->initialize_core(); }

GameNativeVfsRuntime& GameNativeVfsApplication::runtime() {
    if (!impl_->vfs_runtime)
        throw std::logic_error("native VFS runtime has not been retained");
    return *impl_->vfs_runtime;
}

NativeStringRawPoolContext& GameNativeVfsApplication::raw_strings() noexcept {
    return impl_->raw_strings;
}

GameNativeVfsRawServices GameNativeVfsApplication::borrow_raw_services() {
    return runtime().borrow_raw_services();
}

NativeVfsOwnerServices& GameNativeVfsApplication::owners() noexcept {
    return impl_->owner_services;
}

GameNativeTypeStorage& GameNativeVfsApplication::types() noexcept {
    return impl_->type_storage;
}

std::optional<int> GameNativeVfsApplication::physical_pool_registration_status() const noexcept {
    return impl_->pool_registration_status;
}
} // namespace bsp::game
