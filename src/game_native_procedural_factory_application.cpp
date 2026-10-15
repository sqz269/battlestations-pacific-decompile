#include "bsp/game_native_procedural_factory_application.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_resource_registry_scalar_delete.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp::game {
namespace {
using Word = std::uint32_t;
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
Word current_word(const void* base, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const unsigned char*>(base) + offset);
}
Word pointer_word(const void* pointer) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
const void* checked_profile(GameNativeReadOnlyData& data, std::uintptr_t address,
    Word scalar, Word next) {
    const void* view = data.data_at(address, 8);
    require(current_word(view) == scalar && current_word(view, 4) == next,
        "procedural factory numeric profile differs from the admitted domain");
    return view;
}
template<std::size_t N>
const char* checked_text(GameNativeReadOnlyData& data, std::uintptr_t address,
    const char (&expected)[N]) {
    const auto* text = static_cast<const char*>(data.data_at(address, N));
    require(std::memcmp(text, expected, N) == 0,
        "procedural factory literal differs from the admitted domain");
    return text;
}

// All native cells have typed, initialized, stable storage before entry. These
// structures copy required Source values, including untouched bytes; they do
// not infer native caller preimages or preserve aliases between input frames.
struct NodeBacking {
    volatile Word incoming[5], constructor[5], allocation_cleanup, placement_cleanup;
    NativeResourceRegistryNodeAllocationFrame frame;
    explicit NodeBacking(const NativeResourceRegistryNodeAllocationFrame& source)
        : incoming{source.incoming.left, source.incoming.parent, source.incoming.right,
              source.incoming.pair, source.incoming.color},
          constructor{source.constructor.left, source.constructor.parent,
              source.constructor.right, source.constructor.pair, source.constructor.color},
          allocation_cleanup(source.allocation_cleanup), placement_cleanup(source.placement_cleanup),
          frame{{incoming[0], incoming[1], incoming[2], incoming[3], incoming[4]},
              {constructor[0], constructor[1], constructor[2], constructor[3], constructor[4]},
              allocation_cleanup, placement_cleanup} {}
};
struct LinkBacking {
    volatile Word output, left, parent, pair;
    NativeLegacySboStringStorage message;
    NodeBacking allocation;
    NativeResourceRegistryLinkFrame frame;
    explicit LinkBacking(const NativeResourceRegistryLinkFrame& source)
        : output(source.output_argument), left(source.left_argument),
          parent(source.parent_argument), pair(source.pair_argument),
          message(source.length_message), allocation(source.allocation),
          frame{output, left, parent, pair, message, allocation.frame} {}
};
struct PairBacking {
    volatile Word output, pair, direction;
    NativeResourceRegistryTreeIterator cursor;
    LinkBacking link;
    NativeResourceRegistryPairInsertFrame frame;
    explicit PairBacking(const NativeResourceRegistryPairInsertFrame& source)
        : output(source.output_argument), pair(source.pair_argument), direction(source.direction),
          cursor(source.cursor), link(source.link), frame{output, pair, direction, cursor, link.frame} {}
};
struct HintBacking {
    volatile Word output;
    NativeResourceRegistryTreeIterator hint;
    volatile Word pair;
    NativeResourceRegistryInsertResult local;
    LinkBacking link;
    PairBacking fallback;
    NativeResourceRegistryHintInsertFrame frame;
    explicit HintBacking(const NativeResourceRegistryHintInsertFrame& source)
        : output(source.output_argument), hint(source.incoming_hint), pair(source.pair_argument),
          local(source.local), link(source.link), fallback(source.fallback),
          frame{output, hint, pair, local, link.frame, fallback.frame} {}
};
struct ValueBacking {
    volatile Word name;
    NativeResourceRegistryTreeIterator result;
    volatile Word temporary_pair[3];
    HintBacking hint;
    NativeResourceRegistryValueFrame frame;
    explicit ValueBacking(const NativeResourceRegistryValueFrame& source)
        : name(source.name_argument), result(source.result),
          temporary_pair{source.temporary_pair[0], source.temporary_pair[1], source.temporary_pair[2]},
          hint(source.hint), frame{name, result, temporary_pair, hint.frame} {}
};
struct RegistrationBacking {
    volatile Word name, factory;
    NativeResourceRegistryTreeIterator result;
    ValueBacking value;
    NativeResourceRegistryRegistrationFrame frame;
    explicit RegistrationBacking(const NativeResourceRegistryRegistrationFrame& source)
        : name(source.name_argument), factory(source.factory_argument), result(source.result),
          value(source.value), frame{name, factory, result, value.frame} {}
};
struct FactoryBacking {
    volatile Word name, self;
    RegistrationBacking registration;
    NativeProceduralFactoryStorageFrame frame;
    explicit FactoryBacking(const NativeProceduralFactoryStorageFrame& source)
        : name(source.name_argument), self(source.cleanup_self), registration(source.registration),
          frame{name, self, registration.frame} {}
};
struct ConstructionBacking {
    volatile Word locals[5], eh_state;
    FactoryBacking factory;
    volatile Word flags[4];
    NativeProceduralFactorySlotFrame cleanup[4];
    NativeProceduralFactoryTableConstructionFrame frame;
    explicit ConstructionBacking(const NativeProceduralFactoryTableConstructionFrame& source)
        : locals{source.locals[0], source.locals[1], source.locals[2], source.locals[3], source.locals[4]},
          eh_state(source.eh_state), factory(source.factory),
          flags{source.cleanup[0].flags_argument, source.cleanup[1].flags_argument,
              source.cleanup[2].flags_argument, source.cleanup[3].flags_argument},
          cleanup{{flags[0]}, {flags[1]}, {flags[2]}, {flags[3]}},
          frame{locals, eh_state, factory.frame, cleanup} {}
};
struct SetupBacking {
    const Word incoming_ecx;
    volatile Word allocation_cleanup, eh_state;
    ConstructionBacking constructor;
    NativeProceduralFactoryTablePublicationFrame frame;
    explicit SetupBacking(const NativeProceduralFactoryTablePublicationFrame& source)
        : incoming_ecx(source.incoming_ecx), allocation_cleanup(source.cleanup_allocation),
          eh_state(source.eh_state), constructor(source.constructor),
          frame{incoming_ecx, allocation_cleanup, eh_state, constructor.frame} {}
    SetupBacking(const SetupBacking&) = delete;
    SetupBacking& operator=(const SetupBacking&) = delete;
};
struct RetirementBacking {
    volatile Word self, normal_flags, cleanup_flags[3];
    NativeProceduralFactorySlotFrame normal, cleanup[3];
    NativeProceduralFactoryTableFrame frame;
    explicit RetirementBacking(const NativeProceduralFactoryTableFrame& source)
        : self(source.cleanup_self), normal_flags(source.normal.flags_argument),
          cleanup_flags{source.cleanup[0].flags_argument, source.cleanup[1].flags_argument,
              source.cleanup[2].flags_argument},
          normal{normal_flags}, cleanup{{cleanup_flags[0]}, {cleanup_flags[1]}, {cleanup_flags[2]}},
          frame{self, normal, cleanup} {}
    RetirementBacking(const RetirementBacking&) = delete;
    RetirementBacking& operator=(const RetirementBacking&) = delete;
};
} // namespace

struct GameNativeProceduralFactoryApplication::Impl {
    GameVfsHost& vfs;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    void* volatile& publication;
    NativeVfsOwnerServices& services;
    NativeStringRawPoolContext& strings;
    void* volatile& manager;
    void* volatile& registry;
    NativeResourceRegistryInsertionContext insertion;
    NativeResourceRegistryLookupContext profiles;
    NativeProceduralFactoryStorageContext factories;
    NativeProceduralFactoryTableConstructionContext construction;
    SetupBacking setup;
    RetirementBacking retirement;
    NativeProceduralFactoryTablePublicationAcquired acquired;
    NativeProceduralFactoryTableShutdownAcquired shutdown;
    Phase state{Phase::prepared};
    void* owned_table{};
    void* owned_factory[4]{};
    void* owned_node[4]{};
    void* settled_manager{};
    void* settled_registry{};

    Impl(GameVfsHost& vfs_owner, GameSingletonHost& singleton_owner,
        GameNativeReadOnlyData& read_only, void* volatile& actual_publication,
        const GameNativeProceduralFactorySourceInputs& inputs)
        : vfs(vfs_owner), singletons(singleton_owner), data(read_only), publication(actual_publication),
          services(vfs.native_owners()), strings(vfs.raw_strings()),
          manager(singletons.manager_publication_01090aa0()),
          registry(singletons.resource_registry_publication_00f8d41c()),
          insertion{strings, services.physical().invalid_parameters,
              checked_text(data, 0x00ce47bc, "map/set<T> too long")},
          profiles{checked_profile(data, 0x00d64470, 0x00bbc650, 0x00bbc6f0),
              checked_profile(data, 0x00d644ac, 0x00bbc7a0, 0x00bbc810),
              checked_profile(data, 0x00d644e8, 0x00bbc890, 0x00bbc6f0),
              checked_profile(data, 0x00d644f0, 0x00bbc8e0, 0x00bbc810),
              services.physical().invalid_parameters},
          factories{manager, registry, insertion,
              checked_profile(data, 0x00d5e594, 0x00b1b660, 0x00b1b3a0),
              checked_profile(data, 0x00d5e59c, 0x00b1b710, 0x00b1b3a0)},
          construction{factories, profiles, {
              checked_text(data, 0x00ce7dec, "CausticsTextureSource"),
              checked_text(data, 0x00ce7dc8, "ShoreWaveTextureSource0"),
              checked_text(data, 0x00ce7da4, "ShoreWaveTextureSource1"),
              checked_text(data, 0x00ce7d80, "ShoreWaveTextureSource2")}},
          setup(inputs.setup), retirement(inputs.retirement) {
        validate_domain();
    }
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    void validate_domain() {
        auto& process = game_native_string_process();
        const auto& deletion = singletons.native_deletion_bindings();
        require(&publication != &manager && &publication != &registry &&
            static_cast<const volatile void*>(&publication) != &strings.actual_published_01090aa8 &&
            static_cast<const volatile void*>(&publication) != &strings.actual_small_returns_disabled_01090aa4,
            "procedural factory publication aliases a shared parent cell");
        require(&vfs.native_owners() == &services && &vfs.raw_strings() == &strings,
            "procedural factory VFS owner services changed");
        require(&manager == &process.manager_01090aa0() &&
            &strings.actual_manager_publication_01090aa0 == &manager &&
            services.lifetime().borrows_same_domain(manager),
            "procedural factory manager domain differs");
        require(&strings.actual_published_01090aa8 == &services.string_pool_publication_01090aa8() &&
            &strings.actual_published_01090aa8 == &process.pool_01090aa8() &&
            &strings.actual_small_returns_disabled_01090aa4 == &services.string_returns_disabled_01090aa4() &&
            &strings.actual_small_returns_disabled_01090aa4 == &process.returns_disabled_01090aa4(),
            "procedural factory raw string publication domain differs");
        require(deletion.actual_string_pool_publication_01090aa8 == &strings.actual_published_01090aa8 &&
            deletion.actual_string_returns_disabled_01090aa4 == &strings.actual_small_returns_disabled_01090aa4,
            "procedural factory string deletion domain differs");
        require(deletion.resource_registry &&
            &deletion.resource_registry->actual_publication_00f8d41c == &registry &&
            &deletion.resource_registry->strings == &services.strings() &&
            &deletion.resource_registry->invalid_parameters == &services.physical().invalid_parameters,
            "procedural factory registry deletion domain differs");
        require(&singletons.resource_registry_publication_00f8d41c() == &registry,
            "procedural factory registry publication changed");
    }

    // Admission reads only. No additional native getter, insertion, retain or
    // rollback is substituted for the genuine construction's recorded effects.
    void capture_completed_setup() {
        require(acquired.completed && acquired.constructor_returned && acquired.publication_written &&
            acquired.captured_allocation && acquired.captured_return == acquired.captured_allocation &&
            publication == acquired.captured_return && acquired.constructor.completed,
            "procedural factory setup did not produce a complete owned table");
        owned_table = acquired.captured_return;
        settled_manager = manager;
        settled_registry = registry;
        require(settled_manager && settled_registry, "procedural factory shared parents are absent");
        for (unsigned i = 0; i != 4; ++i) {
            const auto& factory = acquired.constructor.factory[i];
            const auto& value = factory.registration.value;
            const auto& direct = value.hint.direct;
            const auto& fallback = value.hint.fallback.link;
            const auto& link = direct.started ? direct : fallback;
            require(acquired.constructor.slot_published[i] && acquired.constructor.final_profile_written[i] &&
                factory.constructor_completed && factory.returned_registry == settled_registry &&
                factory.registration.started && value.started && link.linked && link.result_published &&
                link.node.constructor_completed && link.node.captured_allocation &&
                !link.node.catch_free_completed,
                "procedural factory registration did not insert a fresh borrowed factory node");
            owned_factory[i] = acquired.constructor.captured_allocation[i];
            owned_node[i] = link.node.captured_allocation;
            require(owned_factory[i] != nullptr, "procedural factory slot has no owned allocation");
        }
        validate_live();
    }
    void validate_live() {
        validate_domain();
        require(publication == owned_table && owned_table && manager == settled_manager &&
            registry == settled_registry && settled_manager && settled_registry,
            "procedural factory publication or parent identity changed");
        for (unsigned i = 0; i != 4; ++i) {
            require(current_word(owned_table, i * 4) == pointer_word(owned_factory[i]) &&
                current_word(owned_factory[i]) == (i == 0 ? 0x00d644e8u : 0x00d644f0u) &&
                current_word(owned_node[i], 0x14) == pointer_word(owned_factory[i]),
                "procedural factory table or borrowed registry identity changed");
        }
    }
};

GameNativeProceduralFactoryApplication::GameNativeProceduralFactoryApplication(
    GameVfsHost& vfs, GameSingletonHost& singletons, GameNativeReadOnlyData& data,
    void* volatile& actual_publication, const GameNativeProceduralFactorySourceInputs& inputs)
    : impl_(std::make_unique<Impl>(vfs, singletons, data, actual_publication, inputs)) {}

GameNativeProceduralFactoryApplication::~GameNativeProceduralFactoryApplication() noexcept {
    // No automatic native cleanup is safe after partial registration or table
    // retirement. Retain the complete graph until the explicit parent drain.
    if (requires_process_retention()) std::terminate();
}
void GameNativeProceduralFactoryApplication::setup_00bbcb40() {
    auto& d = *impl_;
    require(d.state == Phase::prepared, "procedural factory setup cannot be replayed");
    d.state = Phase::setting_up;
    try {
        d.validate_domain();
        require(d.publication == nullptr, "procedural factory publication is already occupied");
        publish_native_procedural_factory_table_00bbcb40(
            d.publication, d.setup.frame, d.construction, d.acquired);
        d.capture_completed_setup();
        d.state = Phase::live;
    } catch (...) {
        d.state = Phase::failed;
        throw;
    }
}
void GameNativeProceduralFactoryApplication::require_lookup_ready() {
    auto& d = *impl_;
    require(d.state == Phase::live, "procedural factory lookups are not admitted");
    try {
        d.validate_live();
    } catch (...) {
        d.state = Phase::failed;
        throw;
    }
}
void GameNativeProceduralFactoryApplication::retire_00bbc5d0(bool all_factory_lookups_quiescent) {
    auto& d = *impl_;
    require(d.state == Phase::live, "procedural factory retirement cannot be entered or replayed");
    d.state = Phase::retiring;
    try {
        require(all_factory_lookups_quiescent, "procedural factory retirement requires lookup quiescence");
        d.validate_live();
        shutdown_native_procedural_factory_table_00bbc5d0(
            d.publication, d.retirement.frame, d.profiles, d.shutdown);
        require(d.shutdown.completed && d.shutdown.table_returned && d.shutdown.table_freed &&
            d.shutdown.captured_table == d.owned_table && d.publication == d.owned_table,
            "procedural factory shutdown did not preserve the captured dangling publication");
        d.state = Phase::retired;
    } catch (...) {
        d.state = Phase::failed;
        throw;
    }
}
void GameNativeProceduralFactoryApplication::after_parent_drain() {
    auto& d = *impl_;
    require(d.state == Phase::retired, "procedural factory parent drain acknowledgment is out of order");
    try {
        require(d.publication == d.owned_table && d.manager == nullptr && d.registry == nullptr &&
            d.strings.actual_published_01090aa8 == nullptr,
            "procedural factory parent drain is incomplete or publication changed");
        d.state = Phase::drained;
    } catch (...) {
        d.state = Phase::failed;
        throw;
    }
}
GameNativeProceduralFactoryApplication::Phase GameNativeProceduralFactoryApplication::phase() const noexcept {
    return impl_->state;
}
bool GameNativeProceduralFactoryApplication::requires_process_retention() const noexcept {
    return impl_->state != Phase::prepared && impl_->state != Phase::drained;
}
const NativeProceduralFactoryTablePublicationAcquired&
GameNativeProceduralFactoryApplication::setup_evidence() const noexcept { return impl_->acquired; }
const NativeProceduralFactoryTableShutdownAcquired&
GameNativeProceduralFactoryApplication::retirement_evidence() const noexcept { return impl_->shutdown; }
} // namespace bsp::game
