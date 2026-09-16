#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeResourceManagerContext;
struct NativeStringRawPoolContext;
}

namespace bsp::game {
class GameSingletonHost;
class GameNativeReadOnlyData;

// Actual resource-manager and eight parser publication cells, using the
// application's existing raw string pool and singleton lifetime manager.
// Retain this owner, strings, mapped data and GameSingletonHost through the
// complete shared drain. Destruction does not independently drain or replay it.
class GameNativeResourceApplication final {
public:
    GameNativeResourceApplication(GameSingletonHost&, NativeStringRawPoolContext&,
        GameNativeReadOnlyData&);
    ~GameNativeResourceApplication();
    GameNativeResourceApplication(const GameNativeResourceApplication&) = delete;
    GameNativeResourceApplication& operator=(const GameNativeResourceApplication&) = delete;

    void* manager_004c1400();
    void* animation_channels_parser_00736dd0();
    void* bone_parser_00736ea0();
    bool register_parser_00b80a50(void* actual_manager, void* actual_parser);

    // Borrow the actual context for subsequent native resource loading. No
    // semantic registry, extra manager or private string storage is supplied.
    NativeResourceManagerContext& raw_manager_context();
    void* published_manager() const noexcept;
    std::size_t registered_parsers() const noexcept;
    std::uint32_t failure_entry() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
