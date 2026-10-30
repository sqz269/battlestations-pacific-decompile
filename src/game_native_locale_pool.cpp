#include "bsp/game_native_locale_pool.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/pool_pool16_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Canonical locale pool requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
GameNativeLocalePoolProcess* canonical_process;
static_assert(sizeof(void*) == 4 && sizeof(NodePool1CStorage) == 0x38);
static_assert(offsetof(NodePool1CStorage, allocator_00) == 0);
static_assert(offsetof(NodePool1CStorage, critical_section_0c) == 0x0c);

struct LocalePoolHost final : NodePool1CHost {
    void* operator_new_00bf55be(std::uint32_t bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    }
    void free_00bf6989(void* block) noexcept override { singleton_lifetime_free(block); }
    void free_00bf65ac(void* block) noexcept override { singleton_lifetime_free(block); }
};

[[noreturn]] void crt_failed(const char* operation) noexcept {
    std::fprintf(stderr, "bsp_game: native locale pool CRT %s interrupted; retaining partial ownership\n", operation);
    std::fflush(stderr);
    std::_Exit(1);
}
} // namespace

struct GameNativeLocalePoolProcess::Impl {
    enum class Startup { fresh, initializing, returned, failed };
    enum class Exit { not_entered, entered, returned, failed };

    AllocatorListDomain& list;
    LocalePoolHost host;
    NodePool1CStorage pool{};
    std::recursive_mutex mutex;
    Startup startup{Startup::fresh};
    Exit exit{Exit::not_entered};
    bool failed{};
    bool constructor_returned{};
    bool registration_returned{};
    int registration_status{};

    Impl() : list(game_native_physical_pool_process().allocator_list_domain_00e188b4()) {}

    void poison() noexcept {
        failed = true;
        startup = Startup::failed;
        if (exit == Exit::entered) exit = Exit::failed;
    }

    [[noreturn]] void reject(const char* message) {
        poison(); // before constructing/throwing the diagnostic
        throw std::logic_error(message);
    }

    void require_initializing() {
        if (failed || startup != Startup::initializing || exit != Exit::not_entered)
            reject("locale pool startup cannot resume after caught reentry/failure/exit");
    }

    static void invoke_trim(void* context) {
        auto& p = *static_cast<Impl*>(context);
        // No state permission or no-op: this is the real CE4120 virtual0 body.
        node_pool_1c_trim_empty_slabs_004360e0(p.pool, p.host);
    }
};

GameNativeLocalePoolProcess::GameNativeLocalePoolProcess()
    : impl_(std::make_unique<Impl>()) {
    canonical_process = this;
}

GameNativeLocalePoolProcess::~GameNativeLocalePoolProcess() = default;

GameNativeLocalePoolProcess& game_native_locale_pool_process() {
    // Do not register a C++ destructor for this bookkeeping/host/context. In
    // particular, failed atexit registration leaves the native element linked.
    static auto* const process = new GameNativeLocalePoolProcess;
    return *process;
}

int GameNativeLocalePoolProcess::initialize_once_00cc8d30() {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    if (!p.failed && p.startup == Impl::Startup::returned && p.exit == Impl::Exit::not_entered)
        return p.registration_status;
    if (p.failed || p.startup != Impl::Startup::fresh || p.exit != Impl::Exit::not_entered)
        p.reject("locale pool startup cannot replay or reenter");

    p.startup = Impl::Startup::initializing; // consume attempt before any binding/publication
    try {
        p.list.bind_virtual0(p.pool.allocator_00,
            {kNodePool1CVtable, 0x004360e0u, &p, &Impl::invoke_trim});
        p.require_initializing();
        node_pool_1c_construct_00436230(p.pool, p.list, p.host);
        p.constructor_returned = true;
        p.require_initializing();
        p.registration_status = std::atexit(&destroy_at_exit_00cd9410);
        p.registration_returned = true;
        p.require_initializing();
        p.startup = Impl::Startup::returned;
        return p.registration_status;
    } catch (...) {
        p.poison();
        throw; // retain everything; existing constructor owns its declared cleanup only
    }
}

void* GameNativeLocalePoolProcess::pool_storage_00e1768c() {
    auto& p = *impl_;
    std::lock_guard lock(p.mutex);
    if (p.failed || p.startup != Impl::Startup::returned || !p.constructor_returned ||
        !p.registration_returned || p.exit != Impl::Exit::not_entered)
        p.reject("locale pool borrow requires completed healthy startup before exit");
    return &p.pool;
}

void GameNativeLocalePoolProcess::destroy_at_exit_00cd9410() noexcept {
    auto* const process = canonical_process; // installed context, never a lazy factory
    if (!process) crt_failed("missing canonical context");
    auto& p = *process->impl_;
    try {
        std::lock_guard lock(p.mutex);
        if (!p.constructor_returned || !p.registration_returned || p.registration_status != 0 ||
            p.exit != Impl::Exit::not_entered) {
            p.poison();
            crt_failed("invalid or repeated callback");
        }
        // Honor an actual registered obligation even if later Source startup
        // metadata was poisoned. It never grants a new borrow or healthy startup.
        p.exit = Impl::Exit::entered;
        node_pool_1c_destroy_00435a50(p.pool, p.list, p.host);
        if (p.exit != Impl::Exit::entered) {
            p.poison();
            crt_failed("caught callback reentry");
        }
        p.exit = Impl::Exit::returned;
    } catch (...) {
        p.poison();
        p.exit = Impl::Exit::failed;
        crt_failed("shutdown");
    }
}
} // namespace bsp::game
