#include "bsp/game_native_resource_application.hpp"

#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_resource_extra_parser_singletons.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <stdexcept>

namespace bsp::game {
namespace {
static_assert(sizeof(void*) == 4);
void current_resource_invalid_parameter(void*) {
    // Same source-CRT service used by the raw singleton vector bindings.
    // An installed returning handler is allowed to return to the native site.
    _invalid_parameter_noinfo();
}
}

struct GameNativeResourceApplication::Impl {
    void* volatile manager_010901c4{};
    void* volatile group_params_0109033c{};
    void* volatile mesh_0109047c{};
    void* volatile skined_mesh_01090480{};
    void* volatile matrix_mesh_01090484{};
    void* volatile camera_010902a0{};
    void* volatile skined_animation_0109043c{};
    void* volatile animation_channels_01090298{};
    void* volatile bone_0109029c{};
    SingletonLifetimeCallbacks validation{nullptr, nullptr, &current_resource_invalid_parameter};
    NativeDefaultResourceParserNameCalls default_names;
    NativeResourceExtraParserNameCalls names;
    NativeDefaultResourceManagerFactoryCalls factories;
    NativeResourceManagerContext manager;
    NativeResourceExtraParserContexts extra;
    std::uint32_t failed_entry{};

    Impl(GameSingletonHost& host, NativeStringRawPoolContext& strings,
        GameNativeReadOnlyData& mapped)
        : default_names(strings),
          names(strings, default_names),
          manager{host.manager_publication_01090aa0(), manager_010901c4,
              {group_params_0109033c, mesh_0109047c, skined_mesh_01090480,
               matrix_mesh_01090484, camera_010902a0, skined_animation_0109043c},
              strings, validation, names, factories},
          extra{{host.manager_publication_01090aa0(), animation_channels_01090298},
                {host.manager_publication_01090aa0(), bone_0109029c}} {
        if (&strings.actual_manager_publication_01090aa0 != &host.manager_publication_01090aa0())
            throw std::invalid_argument("resource application requires the shared string/lifetime domain");
        // A one-byte request verifies its already-mapped complete 64-KB band.
        (void)mapped.data_at(0x00ce2000, 1);
        (void)mapped.data_at(0x00cf0000, 1);
        (void)mapped.data_at(0x00d60000, 1);
        auto& deletion = host.native_deletion_bindings();
        if (deletion.resource_manager || deletion.resource_extra_parsers)
            throw std::logic_error("resource application deletion bindings are already installed");
        // No native owner has registered yet. All cells and contexts above
        // are stable before either actual getter can publish/register an owner.
        deletion.resource_manager = &manager;
        deletion.resource_extra_parsers = &extra;
    }

    // Metadata-only destruction. GameStartupHost can delete its singleton
    // host after the shared drain and before this VFS-owned context, so no
    // borrowed publication or deletion-table reference is accessed here.

    void require_operable() const {
        if (failed_entry)
            throw std::logic_error("resource application has an interrupted native operation");
    }
    template<class Operation> decltype(auto) invoke(std::uint32_t entry, Operation&& operation) {
        require_operable();
        try { return operation(); }
        catch (...) { failed_entry = entry; throw; }
    }
};

GameNativeResourceApplication::GameNativeResourceApplication(GameSingletonHost& host,
    NativeStringRawPoolContext& strings, GameNativeReadOnlyData& mapped)
    : impl_(std::make_unique<Impl>(host, strings, mapped)) {}
GameNativeResourceApplication::~GameNativeResourceApplication() = default;

void* GameNativeResourceApplication::manager_004c1400() {
    return impl_->invoke(0x004c1400, [&] { return get_native_resource_manager_004c1400(impl_->manager); });
}
void* GameNativeResourceApplication::animation_channels_parser_00736dd0() {
    return impl_->invoke(0x00736dd0, [&] {
        return get_native_animation_channels_parser_00736dd0(impl_->extra.animation_channels);
    });
}
void* GameNativeResourceApplication::bone_parser_00736ea0() {
    return impl_->invoke(0x00736ea0, [&] { return get_native_bone_parser_00736ea0(impl_->extra.bone); });
}
bool GameNativeResourceApplication::register_parser_00b80a50(void* manager, void* parser) {
    return impl_->invoke(0x00b80a50, [&] {
        return register_native_resource_type_parser_00b80a50(manager, parser,
            impl_->manager.strings, impl_->validation, impl_->names) != 0;
    });
}
NativeResourceManagerContext& GameNativeResourceApplication::raw_manager_context() {
    impl_->require_operable();
    return impl_->manager;
}
void* GameNativeResourceApplication::published_manager() const noexcept {
    return impl_->manager_010901c4;
}
std::size_t GameNativeResourceApplication::registered_parsers() const noexcept {
    const void* const manager = impl_->manager_010901c4;
    if (!manager) return 0;
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(manager) + 0x10);
}
std::uint32_t GameNativeResourceApplication::failure_entry() const noexcept { return impl_->failed_entry; }

} // namespace bsp::game
