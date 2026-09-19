#include "bsp/game_native_online_process.hpp"
#include "bsp/native_online_ipc.hpp"
#include "bsp/xlive_pipe_bootstrap.hpp"
#include "bsp/xlive_pipe_preimage.hpp"
#include "bsp/xlive_pipe_services.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace bsp::game {
namespace {
// Construct only after CD6E16 has established the actual acquisition section.
// All adapters borrow this process's one globals/data/preimage domain.
struct PipeServices final {
    XLivePipeProtocolGlobals protocol;
    XLivePipeFrameGlobals frame;
    CurrentProcessXLivePipePreimageHost preimages;
    Win32XLivePipeProtocolSystemHost protocol_system{preimages};
    Win32XLivePipeFrameSystemHost frame_system{preimages};
    Win32XLivePipeSystemHost transport;
    Win32XLivePipeIoHost io;
    ReconstructedXLivePipeServices pipes;
    const NativeOnlineIpcRuntime ipc;

    PipeServices(XLivePipeGlobalsOwner& globals, const XLivePipeProtocolTables& tables,
        GameNativeOnlineIpcStackPolicy stack)
        : protocol(globals.protocol_globals()),
          frame{globals.acquisition_lock_f8b778(), globals.value_f8b858,
              globals.value_f8b9a0, globals.value_f8bba0, globals.key_f8badc,
              globals.raw_f8b8f8},
          pipes(protocol, frame, tables, protocol_system, frame_system, transport, io),
          ipc(make_win32_native_online_ipc_runtime(pipes, stack.capacity, stack.sent)) {}
};
} // namespace

struct GameNativeOnlineProcess::Impl final {
    const std::filesystem::path image;
    const GameNativeOnlineIpcStackPolicy stack;
    XLivePipeOriginalData data;
    XLivePipeGlobalsOwner globals{original_xlive_pipe_loader_preimage()};
    CrtXLivePipeGlobalsStartupHost crt{globals};
    enum class Phase { fresh, initializing, ready } phase{Phase::fresh};
    std::array<int, 6> registrations{};
    std::unique_ptr<PipeServices> services;

    Impl(const std::filesystem::path& path, GameNativeOnlineIpcStackPolicy policy)
        : image(path), stack(policy), data(image.wstring()) {}

    PipeServices& require_services() const {
        if (phase != Phase::ready)
            throw std::logic_error("native online process CRT initialization is incomplete");
        return *services;
    }
};

GameNativeOnlineProcess::GameNativeOnlineProcess(const std::filesystem::path& image,
    GameNativeOnlineIpcStackPolicy stack) : impl_(std::make_unique<Impl>(image, stack)) {}
GameNativeOnlineProcess::~GameNativeOnlineProcess() = default;

GameNativeOnlineProcess& game_native_online_process(const std::filesystem::path& image,
    GameNativeOnlineIpcStackPolicy stack) {
    if (!image.is_absolute())
        throw std::invalid_argument("native online process requires an absolute original image path");
    const auto normalized = image.lexically_normal();
    // Deliberately process-lived: the CRT callbacks and immutable worker runtime
    // retain these exact addresses beyond GameStartupHost's source lifetime.
    static auto* const process = new GameNativeOnlineProcess(normalized, stack);
    if (process->impl_->image != normalized || process->impl_->stack.capacity != stack.capacity
        || process->impl_->stack.sent != stack.sent)
        throw std::logic_error("native online process cannot replace its image or stack policy");
    return *process;
}

std::array<int, 6> GameNativeOnlineProcess::initialize_once() {
    auto& state = *impl_;
    if (state.phase == Impl::Phase::ready) return state.registrations;
    if (state.phase != Impl::Phase::fresh)
        throw std::logic_error("native online process initialization cannot be replayed");
    state.phase = Impl::Phase::initializing;
    try {
        state.registrations = initialize_xlive_pipe_globals(state.globals,
            state.data.globals(), state.data.tables(), state.crt);
        state.services = std::make_unique<PipeServices>(state.globals, state.data.tables(), state.stack);
        bind_native_online_ipc_worker_runtime(state.services->ipc);
        state.phase = Impl::Phase::ready;
        return state.registrations;
    } catch (...) {
        // Retain partially registered globals. Running CRT cleanup or retrying
        // against an incomplete native graph is not an established FH3 policy.
        std::fputs("bsp_game: native online CRT initialization interrupted; retaining process graph\n", stderr);
        std::fflush(stderr);
        std::_Exit(1);
    }
}
const NativeOnlineIpcRuntime& GameNativeOnlineProcess::ipc() const {
    return impl_->require_services().ipc;
}
ReconstructedXLivePipeServices& GameNativeOnlineProcess::pipes() const {
    return impl_->require_services().pipes;
}
} // namespace bsp::game
