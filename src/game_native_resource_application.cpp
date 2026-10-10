#include "bsp/game_native_resource_application.hpp"

#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_shader_process.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_resource_extra_parser_singletons.hpp"
#include "bsp/native_game_resource_parsers.hpp"
#include "bsp/native_resource_hierarchy_parser.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_resource_root_owner.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
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

struct HierarchyMappedData {
    const volatile std::uint32_t* node_profile;
    const volatile std::uint32_t* negative_bound;
    const volatile std::uint32_t* positive_bound;
};

HierarchyMappedData hierarchy_mapped_data(GameNativeReadOnlyData& data) {
    const HierarchyMappedData mapped{
        static_cast<const volatile std::uint32_t*>(data.data_at(0x00d68bb4, 8)),
        static_cast<const volatile std::uint32_t*>(data.data_at(0x00ce4adc, 4)),
        static_cast<const volatile std::uint32_t*>(data.data_at(0x00ce4970, 4))};
    // Retained BS/BV evidence: actual BD30E0/BE9FC0 prefix and bound words.
    // The contexts below borrow these cells; these comparisons copy no defaults.
    if (mapped.node_profile[0] != 0x00bd30e0u ||
            mapped.node_profile[1] != 0x00be9fc0u ||
            *mapped.negative_bound != 0xd01502f9u ||
            *mapped.positive_bound != 0x501502f9u)
        throw std::logic_error("hierarchy mapped data differs from the supported profile/bounds");
    return mapped;
}

struct HierarchyBinding {
    NativeVfsRuntimeBindings& vfs;
    GameNativeReadOnlyData& data;
    const volatile std::uint32_t* const node_profile;
    NativeResourceRootDispatch nodes;
    NativeResourceStreamReadContext reads;
    NativeResourceHierarchyParserContext hierarchy;
    GameNativeHierarchyServices services;

    HierarchyBinding(NativeVfsRuntimeBindings& actual_vfs,
        NativeStringRawPoolContext& strings, GameNativeReadOnlyData& actual_data,
        HierarchyMappedData mapped, NativeMaterialParameterPool& pool, void* empty)
        : vfs(actual_vfs), data(actual_data), node_profile(mapped.node_profile),
          nodes(actual_vfs, strings), reads{strings, nodes, empty},
          hierarchy{reads, *mapped.negative_bound, *mapped.positive_bound},
          services{reads, hierarchy, pool} {}
    HierarchyBinding(const HierarchyBinding&) = delete;
    HierarchyBinding& operator=(const HierarchyBinding&) = delete;

    void require_same_domain(NativeVfsRuntimeBindings& actual_vfs,
        NativeStringRawPoolContext& strings, GameNativeReadOnlyData& actual_data,
        HierarchyMappedData mapped, NativeMaterialParameterPool& pool, void* empty) const {
        if (&vfs != &actual_vfs || &data != &actual_data ||
                node_profile != mapped.node_profile || &reads.strings != &strings ||
                &reads.streams != &nodes || reads.actual_empty_string_storage_0109db64 != empty ||
                &hierarchy.reads != &reads ||
                &hierarchy.negative_bound_00ce4adc != mapped.negative_bound ||
                &hierarchy.positive_bound_00ce4970 != mapped.positive_bound ||
                &services.reads != &reads || &services.hierarchy != &hierarchy ||
                &services.hierarchy_pool_0109022c != &pool)
            throw std::logic_error("hierarchy services cannot be rebound to another domain");
    }
};
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
    void* volatile geom_mesh_00e19bd0{};
    void* volatile convex_object_00e19a90{};
    void* volatile note_00e19b84{};
    void* volatile zone_desc_00e19b8c{};
    void* volatile aux_00e19b88{};
    SingletonLifetimeCallbacks validation{nullptr, nullptr, &current_resource_invalid_parameter};
    NativeDefaultResourceParserNameCalls default_names;
    NativeResourceExtraParserNameCalls extra_names;
    NativeGameResourceParserNameCalls names;
    NativeDefaultResourceManagerFactoryCalls factories;
    NativeResourceManagerContext manager;
    NativeResourceExtraParserContexts extra;
    NativeGameResourceParserContexts game_parsers;
    NativeGameResourceParsersContext game_context;
    std::uint32_t failed_entry{};
    // Append metadata so earlier publication cells and contexts retain their
    // offsets. Neither member accesses the borrowed host during destruction.
    GameNativeReadOnlyData& hierarchy_data;
    std::unique_ptr<HierarchyBinding> hierarchy_binding;

    Impl(GameSingletonHost& host, NativeStringRawPoolContext& strings,
        GameNativeReadOnlyData& mapped)
        : default_names(strings),
          extra_names(strings, default_names),
          names(strings, extra_names),
          manager{host.manager_publication_01090aa0(), manager_010901c4,
              {group_params_0109033c, mesh_0109047c, skined_mesh_01090480,
               matrix_mesh_01090484, camera_010902a0, skined_animation_0109043c},
              strings, validation, names, factories},
          extra{{host.manager_publication_01090aa0(), animation_channels_01090298},
                {host.manager_publication_01090aa0(), bone_0109029c}},
          game_parsers{{host.manager_publication_01090aa0(), geom_mesh_00e19bd0},
              {host.manager_publication_01090aa0(), convex_object_00e19a90},
              {host.manager_publication_01090aa0(), note_00e19b84},
              {host.manager_publication_01090aa0(), zone_desc_00e19b8c},
              {host.manager_publication_01090aa0(), aux_00e19b88}},
          game_context{manager, game_parsers}, hierarchy_data(mapped) {
        if (&strings.actual_manager_publication_01090aa0 != &host.manager_publication_01090aa0())
            throw std::invalid_argument("resource application requires the shared string/lifetime domain");
        // A one-byte request verifies its already-mapped complete 64-KB band.
        (void)mapped.data_at(0x00ce2000, 1);
        (void)mapped.data_at(0x00cf0000, 1);
        (void)mapped.data_at(0x00d60000, 1);
        auto& deletion = host.native_deletion_bindings();
        if (deletion.resource_manager || deletion.resource_extra_parsers || deletion.game_resource_parsers)
            throw std::logic_error("resource application deletion bindings are already installed");
        // No native owner has registered yet. All cells and contexts above
        // are stable before either actual getter can publish/register an owner.
        deletion.resource_manager = &manager;
        deletion.resource_extra_parsers = &extra;
        deletion.game_resource_parsers = &game_parsers;
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

const GameNativeHierarchyServices& GameNativeResourceApplication::borrow_hierarchy_services(
    NativeVfsRuntimeBindings& vfs, NativeStringRawPoolContext& strings) {
    auto& state = *impl_;
    state.require_operable();
    if (&strings != &state.manager.strings)
        throw std::invalid_argument("hierarchy services require the resource application's raw strings");
    const auto mapped = hierarchy_mapped_data(state.hierarchy_data);
    auto& pool = game_native_resource_pool_process().hierarchy_pool_0109022c();
    void* const empty = game_native_shader_process().stream_empty_0109db64();
    if (!state.hierarchy_binding)
        state.hierarchy_binding = std::make_unique<HierarchyBinding>(
            vfs, strings, state.hierarchy_data, mapped, pool, empty);
    else
        state.hierarchy_binding->require_same_domain(
            vfs, strings, state.hierarchy_data, mapped, pool, empty);
    return state.hierarchy_binding->services;
}

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
NativeGameResourceParsersContext& GameNativeResourceApplication::raw_game_parsers_context() {
    impl_->require_operable();
    return impl_->game_context;
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
