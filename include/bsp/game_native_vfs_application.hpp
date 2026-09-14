#pragma once

#include <filesystem>
#include <memory>
#include <optional>

namespace bsp {
class NativeVfsOwnerServices;
}

namespace bsp::game {
class GameHostLog;
class GameSingletonHost;
class GameNativeReadOnlyData;
class GameNativeTypeStorage;
class GameNativeVfsRuntime;

// Retained source owner for one application's raw VFS graph. Borrow the same
// GameSingletonHost and mapped data throughout startup and the shared drain.
// The caller must invoke singleton_host.shutdown() before destroying this
// bundle, including when initialize_core() throws after a getter/register call.
class GameNativeVfsApplication final {
public:
    GameNativeVfsApplication(GameHostLog&, GameSingletonHost&,
        GameNativeReadOnlyData&, const std::filesystem::path& original_executable);
    ~GameNativeVfsApplication();
    GameNativeVfsApplication(const GameNativeVfsApplication&) = delete;
    GameNativeVfsApplication& operator=(const GameNativeVfsApplication&) = delete;
    GameNativeVfsApplication(GameNativeVfsApplication&&) = delete;
    GameNativeVfsApplication& operator=(GameNativeVfsApplication&&) = delete;

    // One attempt. Bind deletion before type getters; initialize memory type
    // CD8FC0, process pool CD9010, physical type CD9030, then retain and bind
    // the VFS runtime before BEDA60/registration. A nonzero CRT registration
    // status does not mean that the native pool construction failed.
    void initialize_core();
    GameNativeVfsRuntime& runtime();
    NativeVfsOwnerServices& owners() noexcept;
    GameNativeTypeStorage& types() noexcept;
    // Empty until CD9010 returns. Its status describes CRT registration only.
    std::optional<int> physical_pool_registration_status() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
