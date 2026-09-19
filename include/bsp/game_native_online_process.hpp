#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>

namespace bsp {
struct NativeOnlineIpcRuntime;
class ReconstructedXLivePipeServices;
}
namespace bsp::game {
// Explicit source values for the two native IPC stack locals. Protocol heap,
// framing stack and CryptGenRandom output bytes use the existing machine-byte
// capture adapter instead. These values do not claim original stack identity.
struct GameNativeOnlineIpcStackPolicy {
    std::uint32_t capacity;
    std::uint32_t sent;
};
class GameNativeOnlineProcess final {
public:
    // Startup-thread-only. Run this represented CRT subset once, preserving the
    // six real atexit return values. Repeated access does not register again.
    std::array<int, 6> initialize_once();
    const NativeOnlineIpcRuntime& ipc() const;
    ReconstructedXLivePipeServices& pipes() const;
    GameNativeOnlineProcess(const GameNativeOnlineProcess&) = delete;
    GameNativeOnlineProcess& operator=(const GameNativeOnlineProcess&) = delete;
private:
    friend GameNativeOnlineProcess& game_native_online_process(
        const std::filesystem::path&, GameNativeOnlineIpcStackPolicy);
    GameNativeOnlineProcess(const std::filesystem::path&, GameNativeOnlineIpcStackPolicy);
    ~GameNativeOnlineProcess();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// One permanent process owner. Validates original data before CRT mutations and
// rejects a different image or stack policy. Owns no endpoint: applications must
// close their IPC owners before the real CRT callbacks destroy global locks.
// Native timeout/free behavior is unchanged and is not a guaranteed worker join.
GameNativeOnlineProcess& game_native_online_process(
    const std::filesystem::path& original_executable, GameNativeOnlineIpcStackPolicy);
} // namespace bsp::game
