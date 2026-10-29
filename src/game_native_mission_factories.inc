// File-scope include in game_native_renderer_application.cpp. No CMake source
// mutation; the existing procedural factory providers remain authoritative.
#include "bsp/game_native_mission_factories.hpp"
#include "bsp/game_native_mission_factory_frames.hpp"
#include "bsp/game_native_procedural_factory_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace bsp::game {
namespace mission_factory_detail {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void require(bool condition, const char* text) {
    if (!condition) throw std::logic_error(text);
}
Word word(const void* object, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const unsigned char*>(object) + offset);
}
void require_empty_registry(void* registry) {
    if (!registry) return; // Genuine B1B730 creates the absent registry.
    require(word(registry) == 0x00d5e59cu && word(registry, 0x0c) == 0,
        "mission factory setup requires the absent or empty canonical registry");
    const auto head = word(registry, 8);
    require(head != 0, "empty canonical factory registry has no sentinel");
    const auto* const node = reinterpret_cast<const unsigned char*>(head);
    require(node[0x19] != 0 && word(node) == head && word(node, 4) == head && word(node, 8) == head,
        "empty canonical factory registry has nonempty sentinel links");
}
} // namespace mission_factory_detail

struct GameNativeMissionFactories::Impl {
    enum class Phase { prepared, creating, live, retiring, retired, drained, failed };
    GameHostLog& log;
    GameSingletonHost& singletons;
    // Designated Source-owned cell required by the existing wrapper. This is
    // not an invented Original mutable-global mapping or a second registry.
    void* volatile publication{};
    GameNativeMissionFactoryFrames frames;
    GameNativeProceduralFactoryApplication application;
    Phase phase{Phase::prepared};

    Impl(GameHostLog& l, GameVfsHost& files, GameSingletonHost& host,
        GameNativeReadOnlyData& data)
        : log(l), singletons(host),
          // Explicit Source caller identity, not a recovered incoming ECX.
          // Original BBCB55 saves that word while EH is -1. BBCB60 replaces it
          // with allocator result before BBCB65 arms state0. No reached cleanup
          // reads the Source receiver bits, including allocator failure.
          frames(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this))),
          application(files, host, data, publication,
              GameNativeProceduralFactoryBorrowedFrames{frames.setup(), frames.retirement()}) {}
};

GameNativeMissionFactories::GameNativeMissionFactories(GameHostLog& log,
    GameVfsHost& files, GameSingletonHost& singletons, GameNativeReadOnlyData& data)
    : impl_(std::make_unique<Impl>(log, files, singletons, data)) {}

GameNativeMissionFactories::~GameNativeMissionFactories() noexcept {
    if (impl_ && impl_->phase != Impl::Phase::prepared && impl_->phase != Impl::Phase::drained)
        impl_.release(); // Interrupted native ownership retains its entire frame graph.
}

void GameNativeMissionFactories::create() {
    auto& p = *impl_;
    using mission_factory_detail::require;
    require(p.phase == Impl::Phase::prepared, "mission factory setup cannot replay");
    p.phase = Impl::Phase::creating;
    try {
        mission_factory_detail::require_empty_registry(
            p.singletons.resource_registry_publication_00f8d41c());
        // Exact checked native names ascend from CausticsTextureSource through
        // ShoreWaveTextureSource0/1/2. Empty entry plus serialized setup makes
        // every hint an end insertion; B1AF90's unresolved direction is never
        // entered. The borrowed frames do not read/copy its dormant payload.
        p.application.setup_00bbcb40();
        p.application.require_lookup_ready();
        const auto& evidence = p.application.setup_evidence();
        for (unsigned i = 0; i != 4; ++i) {
            const auto& value = evidence.constructor.factory[i].registration.value;
            require(value.hint.direct.started && !value.hint.fallback.started,
                "mission factory setup left its proved direct-insertion domain");
        }
        p.phase = Impl::Phase::live;
        p.log.notef("native mission factories: setup=BBCB40/BBC900 table=%p keys=4 "
            "registry=%p source_receiver=%p scratch=first_native_writes",
            p.publication, p.singletons.resource_registry_publication_00f8d41c(), &p);
    } catch (...) {
        p.phase = Impl::Phase::failed;
        throw;
    }
}

void GameNativeMissionFactories::require_lookup_ready() {
    auto& p = *impl_;
    mission_factory_detail::require(p.phase == Impl::Phase::live,
        "mission factory lookup admission requires its completed live setup");
    try {
        p.application.require_lookup_ready();
    } catch (...) {
        p.phase = Impl::Phase::failed;
        throw;
    }
}

bool GameNativeMissionFactories::requires_process_retention() const noexcept {
    const auto phase = impl_->phase;
    return phase == Impl::Phase::creating || phase == Impl::Phase::retiring || phase == Impl::Phase::failed;
}

void GameNativeMissionFactories::retire_before_native_drain() {
    auto& p = *impl_;
    mission_factory_detail::require(p.phase == Impl::Phase::live,
        "mission factory retirement requires its completed setup and quiescent lookups");
    p.phase = Impl::Phase::retiring;
    try {
        p.application.retire_00bbc5d0(true);
        p.phase = Impl::Phase::retired;
        p.log.note("native mission factories: retired via BBC5D0 before shared registry drain");
    } catch (...) {
        p.phase = Impl::Phase::failed;
        throw;
    }
}

void GameNativeMissionFactories::after_native_drain() {
    auto& p = *impl_;
    mission_factory_detail::require(p.phase == Impl::Phase::retired,
        "mission factory metadata closure requires its native retirement");
    try {
        p.application.after_parent_drain();
        p.phase = Impl::Phase::drained;
    } catch (...) {
        p.phase = Impl::Phase::failed;
        throw;
    }
}
} // namespace bsp::game
