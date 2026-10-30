#include "bsp/game_native_locale_manager.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_locale_pool.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_locale_manager_base_scalar.hpp"
#include "bsp/native_locale_manager_constructor.hpp"
#include "bsp/native_locale_manager_retirement.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Canonical raw locale manager requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
unsigned char low_byte(const volatile std::uint32_t& flags) noexcept {
    _ReadWriteBarrier();
    const unsigned char result =
        *reinterpret_cast<const volatile unsigned char*>(&flags);
    _ReadWriteBarrier();
    return result;
}
bool overlaps(const volatile void* a, std::size_t an,
    const volatile void* b, std::size_t bn) noexcept {
    const auto av = reinterpret_cast<std::uintptr_t>(a);
    const auto bv = reinterpret_cast<std::uintptr_t>(b);
    return av >= bv ? av - bv < bn : bv - av < an;
}
} // namespace

struct GameNativeLocaleManagerProcess::Impl {
    enum class Phase { fresh, binding, bound, initializing, initialized,
        draining, deleting, retired, ended_allocation_retained, detached, failed };

    GameNativeStringProcess& process;
    NativeStringRawPoolContext& strings;
    const NativeLocaleManagerConstructorContext publication;
    GameNativeLocaleManagerProcess& owner;
    std::recursive_mutex mutex;
    Phase phase{Phase::fresh};
    bool failed{};
    GameSingletonHost* host{};
    NativeSingletonDeletionBindings* bindings{};
    bool claim_consumed{};
    bool binding_installed{};
    bool initialization_entered{};
    GameNativeLocalePoolProcess* pool_process{};
    bool pool_startup_entered{};
    bool pool_startup_returned{};
    int pool_status{};
    void* pool{};
    std::optional<NativeLocaleManagerRetirementContext> retirement;
    bool allocation_entered{};
    bool allocation_returned{};
    void* allocation{};
    bool constructor_entered{};
    bool constructor_returned{};
    void* constructor_result{};
    bool drain_entered{};
    bool drain_had_receiver{};
    bool scalar_entered{};
    bool scalar_returned{};
    std::uint32_t captured_profile{};
    const volatile std::uint32_t* flags_identity{}; // diagnostic identity only after call
    void* scalar_result{};
    bool flags_sampled_after_return{};
    unsigned char flags_after_return{};
    bool allocation_released{};

    explicit Impl(GameNativeLocaleManagerProcess& actual_owner)
        : process(game_native_string_process()), strings(process.raw_context()),
          publication{process.manager_01090aa0(), process.locale_manager_00f8bc4c()},
          owner(actual_owner) {}

    void poison() noexcept { failed = true; phase = Phase::failed; }
    [[noreturn]] void reject(const char* text) {
        poison(); // sticky before diagnostic allocation/provider reentry
        throw std::logic_error(text);
    }
    void require_phase(Phase expected) {
        if (failed || phase != expected)
            reject("locale owner cannot resume after reentry/failure/retirement");
    }
    void require_cell_identity() {
        if (&publication.actual_manager_01090aa0 != &process.manager_01090aa0() ||
            &publication.actual_locale_00f8bc4c != &process.locale_manager_00f8bc4c() ||
            &strings != &process.raw_context() ||
            &strings.actual_manager_publication_01090aa0 != &publication.actual_manager_01090aa0 ||
            &strings.actual_published_01090aa8 != &process.pool_01090aa8() ||
            &strings.actual_small_returns_disabled_01090aa4 != &process.returns_disabled_01090aa4())
            reject("locale owner requires the exact permanent cell/context identities");
    }
    void require_host(GameSingletonHost& actual_host, bool installed) {
        if (failed || host != &actual_host ||
            bindings != &actual_host.native_deletion_bindings() ||
            &actual_host.manager_publication_01090aa0() != &publication.actual_manager_01090aa0)
            reject("locale owner requires its original live host/table");
        require_cell_identity();
        if (bindings->actual_string_pool_publication_01090aa8 != &strings.actual_published_01090aa8 ||
            bindings->actual_string_returns_disabled_01090aa4 != &strings.actual_small_returns_disabled_01090aa4 ||
            (installed && (!binding_installed || bindings->locale_manager != &owner)))
            reject("locale owner requires the actual raw string and typed deletion bindings");
    }
    void require_fixed_bound() {
        if (failed || !host || !bindings || !claim_consumed || !binding_installed)
            reject("locale owner requires its retained binding before native use");
        require_host(*host, true);
    }
    void require_initializing() {
        require_phase(Phase::initializing);
        require_fixed_bound();
    }
    void require_constructor_history() {
        if (!initialization_entered || !pool_startup_entered || !pool_startup_returned ||
            pool_status != 0 || !pool_process || !pool || !retirement ||
            &retirement->publication != &publication || &retirement->strings != &strings ||
            retirement->actual_initialized_locale_pool_00e1768c != pool ||
            !allocation_entered || !allocation_returned || !allocation ||
            !constructor_entered || !constructor_returned || constructor_result != allocation)
            reject("locale owner requires its actual complete constructor/provider history");
    }
    bool unused() const noexcept {
        return !initialization_entered && !pool_startup_entered && !pool_startup_returned &&
            !pool_process && !pool && !retirement && !allocation_entered &&
            !allocation_returned && !allocation && !constructor_entered &&
            !constructor_returned && !constructor_result && !scalar_entered;
    }
    void require_flags_storage(const volatile std::uint32_t& flags) {
        // Address-only checks; never sample a flag before the actual scalar.
        // Other opaque provider/backing disjointness remains a caller proof.
        if (overlaps(&flags, sizeof flags, allocation, 0x4040) ||
            overlaps(&flags, sizeof flags, this, sizeof *this) ||
            overlaps(&flags, sizeof flags, &process, sizeof process) ||
            overlaps(&flags, sizeof flags, bindings, sizeof *bindings) ||
            overlaps(&flags, sizeof flags, pool, 0x38))
            reject("locale flags must be actual disjoint caller control storage");
    }
};

GameNativeLocaleManagerProcess::GameNativeLocaleManagerProcess()
    : impl_(std::make_unique<Impl>(*this)) {}
GameNativeLocaleManagerProcess::~GameNativeLocaleManagerProcess() = default;

GameNativeLocaleManagerProcess& game_native_locale_manager_process() {
    static auto* const process = new GameNativeLocaleManagerProcess;
    return *process; // intentionally retained; no Native or CRT-exit teardown
}

void GameNativeLocaleManagerProcess::bind_once(GameSingletonHost& host) {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        if (!p.failed && (p.phase == Impl::Phase::bound || p.phase == Impl::Phase::initialized)) {
            p.require_host(host, true); // healthy SAME graph repeat, no claim/effect replay
            return;
        }
        p.require_phase(Impl::Phase::fresh);
        p.phase = Impl::Phase::binding;
        p.host = &host;
        p.bindings = &host.native_deletion_bindings();
        if (!p.process.claim_locale_manager_domain())
            p.reject("locale process domain was already consumed");
        p.claim_consumed = true;
        p.require_host(host, false);
        if (p.bindings->locale_manager != nullptr || p.publication.actual_locale_00f8bc4c != nullptr)
            p.reject("locale binding cannot adopt an existing owner/publication");
        p.bindings->locale_manager = this;
        p.binding_installed = true;
        p.require_phase(Impl::Phase::binding);
        p.phase = Impl::Phase::bound;
    } catch (...) { p.poison(); throw; }
}

void GameNativeLocaleManagerProcess::initialize_once() {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        if (!p.failed && p.phase == Impl::Phase::initialized) {
            p.require_fixed_bound();
            p.require_constructor_history();
            if (p.publication.actual_locale_00f8bc4c != p.allocation ||
                p.publication.actual_manager_01090aa0 == nullptr)
                p.reject("initialized locale publication changed");
            return;
        }
        p.require_phase(Impl::Phase::bound);
        p.require_fixed_bound();
        if (!p.unused() || p.publication.actual_locale_00f8bc4c != nullptr)
            p.reject("locale construction requires unused private history");
        p.initialization_entered = true;
        p.phase = Impl::Phase::initializing;
        p.pool_process = &game_native_locale_pool_process();
        p.require_initializing();
        p.pool_startup_entered = true;
        p.pool_status = p.pool_process->initialize_once_00cc8d30();
        p.pool_startup_returned = true;
        p.require_initializing();
        if (p.pool_status != 0)
            p.reject("locale pool actual CRT registration did not return status zero");
        p.pool = p.pool_process->pool_storage_00e1768c();
        p.require_initializing();
        if (!p.pool) p.reject("locale requires the real initialized canonical pool");
        p.retirement.emplace(NativeLocaleManagerRetirementContext{
            p.publication, p.strings, p.pool});
        p.allocation_entered = true;
        p.allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x4040, 0x4040});
        p.allocation_returned = true; // capture normal result even after caught reentry
        p.require_initializing();
        if (!p.allocation) p.reject("locale allocation returned null");
        p.constructor_entered = true;
        p.constructor_result = construct_native_locale_manager_0073c960(p.allocation, p.publication);
        p.constructor_returned = true;
        p.require_initializing();
        p.require_constructor_history();
        if (p.publication.actual_locale_00f8bc4c != p.allocation ||
            p.publication.actual_manager_01090aa0 == nullptr)
            p.reject("locale constructor did not leave its same actual publication");
        p.phase = Impl::Phase::initialized;
    } catch (...) { p.poison(); throw; } // retain all actual effects; never free/repair/retry
}

void* GameNativeLocaleManagerProcess::initialized_receiver() {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        p.require_phase(Impl::Phase::initialized);
        p.require_fixed_bound();
        p.require_constructor_history();
        if (p.scalar_entered || p.publication.actual_locale_00f8bc4c != p.allocation ||
            p.publication.actual_manager_01090aa0 == nullptr)
            p.reject("locale receiver borrow requires its healthy initialized publication");
        return p.allocation;
    } catch (...) { p.poison(); throw; }
}

void GameNativeLocaleManagerProcess::before_shared_drain(GameSingletonHost& host) {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        if (!p.failed && p.phase == Impl::Phase::detached && p.host == &host) return;
        if (p.failed || (p.phase != Impl::Phase::bound && p.phase != Impl::Phase::initialized))
            p.reject("locale cannot enter shared drain from active/failed/ended history");
        p.require_host(host, true);
        p.drain_had_receiver = p.phase == Impl::Phase::initialized;
        if (p.drain_had_receiver) {
            p.require_constructor_history();
            if (p.scalar_entered || p.publication.actual_locale_00f8bc4c != p.allocation ||
                p.publication.actual_manager_01090aa0 == nullptr)
                p.reject("locale shared drain requires the same live initialized owner");
        } else if (!p.unused() || p.publication.actual_locale_00f8bc4c != nullptr) {
            p.reject("unused locale binding acquired unrecorded native debt");
        }
        p.drain_entered = true;
        p.phase = Impl::Phase::draining;
    } catch (...) { p.poison(); throw; }
}

void* GameNativeLocaleManagerProcess::delete_registered_current_profile(void* captured,
    std::uint32_t profile, const volatile std::uint32_t& flags) {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        p.require_phase(Impl::Phase::draining);
        p.require_fixed_bound();
        p.require_constructor_history();
        if (!p.drain_entered || !p.drain_had_receiver || p.scalar_entered ||
            p.publication.actual_manager_01090aa0 == nullptr ||
            captured != p.allocation || (profile != 0x00cff140u && profile != 0x00cfea5cu))
            p.reject("locale deletion requires the exact captured owner/current admitted profile");
        p.require_flags_storage(flags);
        p.scalar_entered = true;
        p.captured_profile = profile;
        p.flags_identity = &flags;
        p.phase = Impl::Phase::deleting;
        // No profile reread or flag value read before the genuine target.
        p.scalar_result = profile == 0x00cff140u
            ? delete_native_locale_manager_0073caf0(captured, flags, *p.retirement)
            : delete_native_locale_manager_base_00737010(captured, flags, p.publication);
        p.scalar_returned = true;
        p.require_phase(Impl::Phase::deleting); // caught recursion cannot complete
        p.require_fixed_bound();
        if (p.scalar_result != captured)
            p.reject("locale scalar did not return its captured address identity");
        // Under the actual PUSH1/stable disjoint control-cell contract, this
        // late control observation matches the real scalar's branch. It is
        // not inferred from returned bits, and reads no retired native bytes.
        p.flags_after_return = low_byte(flags);
        p.flags_sampled_after_return = true;
        p.allocation_released = (p.flags_after_return & 1u) != 0;
        p.phase = p.allocation_released ? Impl::Phase::retired
            : Impl::Phase::ended_allocation_retained;
        return p.scalar_result;
    } catch (...) { p.poison(); throw; }
}

void GameNativeLocaleManagerProcess::after_shared_drain(GameSingletonHost& host) {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    try {
        if (!p.failed && p.phase == Impl::Phase::detached && p.host == &host) return;
        if (p.failed || !p.drain_entered ||
            (p.phase != Impl::Phase::draining && p.phase != Impl::Phase::retired))
            p.reject("locale shared drain did not complete its supported custody");
        p.require_host(host, true);
        if (p.drain_had_receiver) {
            p.require_constructor_history(); // retained C++ metadata only
            if (p.phase != Impl::Phase::retired || !p.scalar_entered || !p.scalar_returned ||
                !p.flags_sampled_after_return || !p.allocation_released ||
                (p.flags_after_return & 1u) == 0 || p.scalar_result != p.allocation)
                p.reject("locale receiver has incomplete or flags-zero allocation debt");
        } else if (!p.unused()) {
            p.reject("unused locale graph gained native debt during drain");
        }
        if (p.publication.actual_locale_00f8bc4c != nullptr ||
            p.publication.actual_manager_01090aa0 != nullptr)
            p.reject("locale detach requires actual publication/parent closure");
        p.bindings->locale_manager = nullptr;
        p.binding_installed = false;
        p.phase = Impl::Phase::detached;
    } catch (...) { p.poison(); throw; }
}
} // namespace bsp::game
