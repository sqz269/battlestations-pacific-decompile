#include "bsp/game_native_texture_atlas_application.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/native_texture_atlas_manager_delete_attempt.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_vfs_application.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_raw_cstring_exact.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <list>
#include <mutex>
#include <new>
#include <stdexcept>

namespace bsp::game {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word bits(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
struct ProcessRoleSlot final {
    std::mutex mutex;
    bool attempted{};
    GameNativeTextureAtlasApplication* actual{}; // never deleted
    std::exception_ptr failure;
};
ProcessRoleSlot& process_role_slot() {
    // Actual born bootstrap metadata in aligned static backing. Placement
    // construction allocates nothing and no destructor is registered, so late
    // CRT/atexit borrowers never encounter an already-destroyed mutex/slot.
    alignas(ProcessRoleSlot) static unsigned char backing[sizeof(ProcessRoleSlot)];
    static auto* const slot = ::new (static_cast<void*>(backing)) ProcessRoleSlot;
    return *slot;
}
} // namespace

struct GameNativeTextureAtlasApplication::Impl final {
    GameNativeTextureAtlasApplication& process;
    GameSingletonHost& host;
    GameNativeReadOnlyData& data;
    GameNativeVfsApplication& vfs;
    GameNativeRendererApplication& renderer;
    const volatile Word& feature;
    NativePathCanonicalizerRawContext& canonical;
    NativeStringRawPoolContext& strings;
    GameNativeVfsRawServices raw;
    NativeTextureCacheContext& cache;
    NativeTextureAtlasItemStorageContext items;
    NativeTextureAtlasRegistryStorageContext registry;
    NativeTextureAtlasParserContext parser;
    NativeTextureAtlasLoaderContext loader;
    NativeSingletonDeletionBindings& deletion;
    NativeTextureAtlasManagerDeleteAttempt* manager_attempt{};
    bool source_fence_returned{};
    const void* const common_literal;

    NativeTextureAtlasRegistryConstructFrame construction;
    std::list<GameNativeTextureAtlasLoadCall> loads;
    std::list<GameNativeTextureAtlasDeleteCall> deletes;
    GameNativeTextureAtlasLoadCall* common{};
    GameNativeTextureAtlasDeleteCall* scalar{};
    void* allocation{};
    void* constructor_return{};
    void* normal_birth{};
    bool binding_installed{}, busy{}, failed{};
    bool allocation_started{}, allocation_returned{}, constructor_started{}, constructor_returned{};
    bool retirement_armed{}, scalar_started{}, scalar_returned{};
    std::exception_ptr failure;

    Impl(GameNativeTextureAtlasApplication& p, GameSingletonHost& h,
        GameNativeReadOnlyData& d, GameNativeVfsApplication& v,
        GameNativeRendererApplication& r, const volatile Word& f,
        NativePathCanonicalizerRawContext& c,
        NativeTextureAtlasRegistrySectionCall const volatile& enter,
        NativeTextureAtlasRegistrySectionCall const volatile& leave,
        NativeTextureAtlasRegistryDecrement const volatile& decrement)
        : process(p), host(h), data(d), vfs(v), renderer(r), feature(f), canonical(c),
          strings(v.raw_strings()), raw(v.borrow_raw_services()), cache(r.texture_cache()),
          items(d, strings, f),
          registry(d, h.manager_publication_01090aa0(), p.atlas_publication_,
              items, r.actual_owners(), cache.textures.owners, enter, leave, decrement),
          parser(d, strings, f, p.scratch_.get(), c, r),
          loader(d, strings, f, raw, p.atlas_publication_, p.empty_name_, parser,
              p.source_line_scratch_extent()),
          deletion(h.native_deletion_bindings()),
          common_literal(d.data_at(0x00ce81c4u, sizeof("interface/textures/common.ats"))) {}

    void require_graph() const {
        require(!source_fence_returned, "atlas graph cannot be used after its one actual Host fence");
        // Object/reference identities are checked, never reconstructed from a
        // profile, count, phase, name lookup or independently created provider.
        require(binding_installed && deletion.texture_atlas == &process &&
            &host.native_deletion_bindings() == &deletion && manager_attempt != nullptr,
            "atlas requires its retained actual singleton deletion binding");
        require(vfs.borrows_graph(host, data) && &vfs.raw_strings() == &strings &&
            &strings.actual_manager_publication_01090aa0 ==
                &host.manager_publication_01090aa0() &&
            &canonical.strings == &strings &&
            &canonical.actual_feature_word_0109eea4 == &feature,
            "atlas requires the same genuine VFS/pool/feature/canonical graph");
        require(&renderer.texture_cache() == &cache &&
            &cache.strings == &raw.strings &&
            &cache.textures.strings == &raw.strings &&
            &cache.textures.streams == &raw.bindings &&
            &cache.textures.opens == &raw.opens &&
            &cache.textures.conversion == &raw.conversion &&
            &cache.textures.current_vfs_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &cache.textures.owners == &registry.textures &&
            &renderer.actual_owners() == &registry.owners &&
            &registry.atlas_publication == &process.atlas_publication_,
            "atlas requires the renderer's same real loader/cache/creator/registry domain");
    }
    void require_ready_providers() const {
        require_graph();
        require(vfs.runtime().actual_manager() != nullptr &&
            raw.actual_vfs_publication_0109ceec == vfs.runtime().actual_manager(),
            "atlas requires the actual retained published VFS owner");
        // Root-owned ready-only supplier checks private normal B32410 birth,
        // SAME RO owner and ready provider graph without reading a replaced
        // publication's raw prefix or inventing captured dispatch observations.
        // Actual AEEAF0 alone captures/admits its receiver/profile/slot epochs.
        require(&renderer.atlas_process_texture_cache(data) == &cache,
            "atlas requires this genuine normal-born renderer loading graph");
    }
    void require_birth() const {
        require(allocation_started && allocation_returned && allocation != nullptr &&
            constructor_started && constructor_returned && constructor_return == allocation &&
            normal_birth == allocation && construction.actual_registry == allocation &&
            construction.phase == NativeTextureAtlasRegistryPhase::complete &&
            construction.base.actual_registry == allocation && construction.base.registered,
            "atlas requires its private actual1Ch allocation and whole normal constructor history");
    }
    void begin() {
        require(!failed && !busy, "atlas attempt cannot reenter, reset or retry an interrupted graph");
        require_graph();
        busy = true;
    }
    void still_running() const {
        require(busy && !failed,
            "atlas graph retains a caught nested failure and cannot complete");
        require_graph();
    }
    void poison_current() noexcept {
        failed = true;
        if (!failure) failure = std::current_exception();
        // Never clear busy here. No Native unlock/free/return/retry.
    }
};

GameNativeTextureAtlasApplication::GameNativeTextureAtlasApplication(std::size_t extent)
    : scratch_extent_(extent), scratch_(new char[extent]()) {}

GameNativeTextureAtlasApplication&
GameNativeTextureAtlasApplication::create_process_roles_once(std::size_t extent) {
    auto& slot = process_role_slot();
    std::lock_guard lock(slot.mutex); // bootstrap only; no Native work under it
    require(!slot.attempted, "atlas Source process-role creation is a one-attempt operation");
    slot.attempted = true;
    try {
        require(extent != 0, "atlas Source shared scratch requires an explicit nonzero extent");
        slot.actual = new GameNativeTextureAtlasApplication(extent);
        return *slot.actual;
    } catch (...) {
        slot.failure = std::current_exception();
        throw;
    }
}
GameNativeTextureAtlasApplication& GameNativeTextureAtlasApplication::process_roles() {
    auto& slot = process_role_slot();
    std::lock_guard lock(slot.mutex);
    require(slot.actual != nullptr, "atlas Source process roles have not been created");
    return *slot.actual;
}

void GameNativeTextureAtlasApplication::bind_graph_once(GameStartupHost& startup, GameSingletonHost& host,
    GameNativeReadOnlyData& data, GameNativeVfsApplication& vfs,
    GameNativeRendererApplication& renderer, const volatile Word& feature,
    NativePathCanonicalizerRawContext& canonical,
    NativeTextureAtlasRegistrySectionCall const volatile& enter,
    NativeTextureAtlasRegistrySectionCall const volatile& leave,
    NativeTextureAtlasRegistryDecrement const volatile& decrement) {
    try {
        // The ACTUAL owning Root records selection before any throwing admission.
        // Its private identity check proves ownership, not just matching metadata.
        startup.bind_native_texture_atlas_retention(*this, host, data, vfs, renderer);
        require(!bind_attempted_, "atlas graph binding is one attempt without graph replacement");
        bind_attempted_ = true;
        // Cold context constructors perform no Native allocation/acquisition.
        // Once retained, this pointer and its entire frame graph are permanent.
        graph_ = new Impl(*this, host, data, vfs, renderer, feature, canonical,
            enter, leave, decrement);
        require(graph_->deletion.texture_atlas == nullptr && atlas_publication_ == nullptr,
            "atlas graph cannot adopt an existing binding or unknown publication");
        graph_->deletion.texture_atlas = this;
        graph_->binding_installed = true;
        graph_->manager_attempt = &host.prepare_texture_atlas_delete_attempt(*this);
        graph_->require_graph();
    } catch (...) {
        bind_failure_ = std::current_exception();
        if (graph_) graph_->poison_current();
        throw;
    }
}

void GameNativeTextureAtlasApplication::construct_normal() {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    try {
        p.begin();
        require(!p.allocation_started && !p.constructor_started && !p.retirement_armed &&
            atlas_publication_ == nullptr, "atlas construction requires its untouched process birth");
        p.require_ready_providers();
        p.allocation_started = true;
        p.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x1c, 0x1c});
        p.allocation_returned = true; // adopt actual raw bytes BEFORE 737EE0
        p.still_running();
        require(p.allocation != nullptr, "atlas actual BF681B allocation returned null");
        p.constructor_started = true;
        p.constructor_return = construct_native_texture_atlas_registry_00737ee0(
            p.allocation, p.construction, p.registry);
        p.constructor_returned = true;
        p.still_running();
        require(p.constructor_return == p.allocation,
            "atlas normal constructor returned a different allocation identity");
        p.normal_birth = p.allocation;
        p.require_birth();
        // Private normal allocation/whole constructor/base registration history;
        // NEVER inferred from profile, count, CURRENT publication or public flags.
        p.manager_attempt->registered_receiver_ = p.normal_birth;
        p.manager_attempt->registered_receiver_bound_ = true;
        p.busy = false;
    } catch (...) { p.poison_current(); throw; }
}

void GameNativeTextureAtlasApplication::load_common_normal() {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    GameNativeTextureAtlasLoadCall* reached = nullptr;
    try {
        p.begin();
        p.require_birth();
        require(!p.retirement_armed && p.common == nullptr,
            "common.ats requires its first reached load before normal retirement");
        p.require_ready_providers();
        require(atlas_publication_ == p.normal_birth,
            "common.ats requires the same actual normal-born registry publication");
        p.loads.emplace_back(); // stable frame adopted before string/loader effects
        reached = &p.loads.back();
        p.common = reached;
        reached->is_common = true;
        reached->actual_request_home = &reached->common_request_home;
        reached->request_constructor_started = true;
        construct_native_string_header_0041e870_exact(reached->common_request,
            p.common_literal, p.strings, p.feature);
        reached->request_constructor_returned = true;
        p.still_running();
        reached->common_request_home = bits(reached->common_request);
        reached->captured_manager = atlas_publication_;
        reached->loader_started = true;
        load_native_texture_atlas_storage_00af0060(reached->captured_manager,
            reached->common_request_home, reached->loader, p.loader);
        reached->loader_returned = true;
        p.still_running();
        reached->request_return_started = true;
        destroy_native_string_header_0041dd20(reached->common_request, p.strings);
        reached->request_returned = true;
        p.still_running();
        p.busy = false;
    } catch (...) {
        if (reached) reached->failure = std::current_exception();
        p.poison_current();
        throw;
    }
}

void GameNativeTextureAtlasApplication::load_normal(volatile Word& home) {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    GameNativeTextureAtlasLoadCall* reached = nullptr;
    try {
        p.begin();
        p.require_birth();
        require(!p.retirement_armed, "atlas cannot start a loader during normal retirement");
        p.require_ready_providers();
        require(atlas_publication_ == p.normal_birth,
            "atlas load requires its same actual normal-born registry publication");
        p.loads.emplace_back();
        reached = &p.loads.back();
        reached->actual_request_home = &home;
        reached->captured_manager = atlas_publication_;
        reached->loader_started = true;
        load_native_texture_atlas_storage_00af0060(reached->captured_manager,
            home, reached->loader, p.loader);
        reached->loader_returned = true;
        p.still_running();
        p.busy = false;
    } catch (...) {
        if (reached) reached->failure = std::current_exception();
        p.poison_current();
        throw;
    }
}

void GameNativeTextureAtlasApplication::prepare_normal_retirement() {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    try {
        p.begin();
        p.require_birth();
        require(!p.retirement_armed && !p.scalar_started,
            "atlas normal retirement cannot be armed twice");
        p.require_ready_providers();
        // No requirement that CURRENT F8C26C still identifies the popped owner.
        // The complete7375E0 child independently unregisters/clears CURRENT cell.
        p.retirement_armed = true;
        p.still_running();
        p.busy = false;
    } catch (...) { p.poison_current(); throw; }
}
void GameNativeTextureAtlasApplication::retire_normal() {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    try {
        p.require_graph(); // FIRST rejects the consumed fence before any borrowed attempt access
        if (!p.retirement_armed) prepare_normal_retirement();
        // Outside manager wrapper, but retain the SAME actual registered receiver.
        auto& atlas = *p.manager_attempt;
        require(!atlas.scalar_entered_, "atlas scalar cannot replay explicit retirement");
        atlas.scalar_receiver_ = p.normal_birth;
        atlas.scalar_flags_ = 1u;
        atlas.scalar_entered_ = true;
        (void)delete_registered_00737f10(p.normal_birth, 1u);
        atlas.scalar_returned_ = true;
    } catch (...) { p.poison_current(); throw; }
}
void* GameNativeTextureAtlasApplication::delete_registered_00737f10(
    void* captured, Word flags) {
    require(graph_ != nullptr && !bind_failure_, "atlas requires its complete retained graph binding");
    auto& p = *graph_;
    GameNativeTextureAtlasDeleteCall* reached = nullptr;
    try {
        p.begin();
        p.require_birth();
        require(p.retirement_armed && !p.scalar_started && captured == p.normal_birth,
            "atlas deletion requires its private normal-born captured owner and explicit normal schedule");
        p.deletes.emplace_back(); // BEFORE first scalar Native effect
        reached = &p.deletes.back();
        p.scalar = reached;
        reached->flags_home = flags; // full fresh real DWORD HOME, retained/disjoint
        reached->captured_receiver = captured;
        reached->scalar_started = true;
        p.scalar_started = true;
        reached->opaque_return = delete_native_texture_atlas_registry_00737f10(captured,
            reached->flags_home, reached->scalar, p.registry);
        reached->scalar_returned = true;
        p.scalar_returned = true;
        p.still_running();
        // No raw-prefix/flag/profile reread, free or destructor after return.
        p.busy = false;
        return reached->opaque_return;
    } catch (...) {
        if (reached) reached->failure = std::current_exception();
        p.poison_current();
        throw;
    }
}

bool GameNativeTextureAtlasApplication::requires_host_retention() const noexcept {
    return bind_failure_ || (graph_ && (graph_->failed || graph_->busy ||
        (graph_->manager_attempt && graph_->manager_attempt->requires_source_retention())));
}
void GameNativeTextureAtlasApplication::note_host_exception() noexcept {
    if (graph_ && !graph_->source_fence_returned) graph_->poison_current();
    if (!bind_failure_) bind_failure_ = std::current_exception();
}
void GameNativeTextureAtlasApplication::after_host_shared_drain(GameSingletonHost& host) {
    require(graph_ != nullptr && !bind_failure_, "atlas Host fence requires its retained graph");
    auto& p = *graph_;
    try {
        require(&p.host == &host && !p.source_fence_returned && !p.failed && !p.busy &&
            p.manager_attempt != nullptr,
            "atlas Host fence requires the same complete actual drain");
        host.finish_texture_atlas_delete_attempt(*p.manager_attempt);
        p.source_fence_returned = true; // no more access through borrowed Root references
    } catch (...) { p.poison_current(); throw; }
}

const GameNativeTextureAtlasLoadCall*
GameNativeTextureAtlasApplication::common_load_record() const noexcept {
    return graph_ ? graph_->common : nullptr;
}
const NativeTextureAtlasRegistryConstructFrame*
GameNativeTextureAtlasApplication::construction_record() const noexcept {
    return graph_ ? &graph_->construction : nullptr;
}
const GameNativeTextureAtlasDeleteCall*
GameNativeTextureAtlasApplication::deletion_record() const noexcept {
    return graph_ ? graph_->scalar : nullptr;
}
} // namespace bsp::game
