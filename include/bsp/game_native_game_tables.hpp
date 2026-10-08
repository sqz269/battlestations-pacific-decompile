#pragma once
#include "bsp/native_game_tables.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp::game {
// Stable Source storage for the constructor's unit/rank table domain. The
// retained construction context borrows this owner, which must outlive all
// consumers, including a failed constructor's retained operation/context.
class GameNativeGameTables final {
public:
    // The caller supplies the native local word's opaque preexisting bytes.
    // There is deliberately no default and no inferred native stack value.
    explicit GameNativeGameTables(std::uint32_t unit_frame_word_preimage) noexcept;
    GameNativeGameTables(const GameNativeGameTables&)=delete;
    GameNativeGameTables& operator=(const GameNativeGameTables&)=delete;
    GameNativeGameTables(GameNativeGameTables&&)=delete;
    GameNativeGameTables& operator=(GameNativeGameTables&&)=delete;

    // Issues the same stored context once, while its real append count is zero.
    // Neither construction nor borrowing invokes either native table builder.
    // No reset/reissue is permitted, even after a failed game construction.
    // Once borrowed, the raw context still requires one 23-row append only;
    // this owner cannot guard raw calls, context copies, or native ABI replays.
    NativeGameTablesContext& borrow_construction_context();

private:
    alignas(4) std::array<std::byte,23*16> unit_rows_{};
    volatile std::uint32_t unit_count_{0};
    std::array<std::uint32_t,12*97> rank_rows_{};
    NativeGameTablesContext context_;
    bool context_borrowed_{false};
};
} // namespace bsp::game
