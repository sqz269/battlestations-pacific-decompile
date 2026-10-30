#pragma once

#include "bsp/native_lua_script_overrides_raw.hpp"
#include "bsp/native_locale_parser_file_storage.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
class NativeLocaleTextualParserFrame;

// Explicit trailing-option1 domain of AA0020, not an option0/full-parser entry.
// The actual completed owning input8 is supplied by the caller; its debt is
// transferred only at admitted invocation, not by Context/Frame construction.
class NativeLocaleTextualParserContext final {
public:
    NativeLocaleTextualParserContext(void* actual_locale_manager,
        void* actual_owning_input8, std::uint32_t opaque_other_argument,
        game::GameNativeVfsApplication&, game::GameNativeLocalePoolProcess&,
        const game::GameNativeReadOnlyData&,
        NativeCockpitInterlockedDecrement const volatile& actual_decrement_cell);
    NativeLocaleTextualParserContext(const NativeLocaleTextualParserContext&) = delete;
    NativeLocaleTextualParserContext& operator=(const NativeLocaleTextualParserContext&) = delete;
    NativeLocaleTextualParserContext(NativeLocaleTextualParserContext&&) = delete;
    NativeLocaleTextualParserContext& operator=(NativeLocaleTextualParserContext&&) = delete;
private:
    friend class NativeLocaleTextualParserFrame;
    friend void parse_native_locale_textual_option1_00aa0020(
        const NativeLocaleTextualParserContext&, NativeLocaleTextualParserFrame&);
    void* const manager_;
    void* const input_;
    const std::uint32_t other_;
    game::GameNativeVfsApplication& app_;
    NativeStringRawPoolContext& strings_;
    game::GameNativeVfsRawServices services_;
    game::GameNativeLocalePoolProcess& pool_;
    const game::GameNativeReadOnlyData& data_;
    NativeCockpitInterlockedDecrement const volatile& decrement_cell_;
    NativeLuaScriptOverridesRawContext overrides_; // existing callee Context, NO invented Frame
};

enum class NativeLocaleTextualParserPhase { prepared, entered, scanning, returning, returned, failed };
struct NativeLocaleTextualParserProgress {
    NativeLocaleTextualParserPhase phase{NativeLocaleTextualParserPhase::prepared};
    const NativeLocaleTextualParserContext* context{};
    std::uint32_t opaque_other_argument{}, captured_begin{}, captured_end{}, cursor{};
    std::size_t file_attempts{}, files_returned{};
    bool attempted{}, input_owned{}, vector_initialized{}, overrides_entered{}, overrides_returned{};
    bool original_append_entered{}, original_append_returned{};
    bool normal_vector_edge_consumed{}, normal_vector_shrink_entered{}, normal_vector_shrink_returned{};
    bool normal_vector_free_entered{}, normal_vector_free_returned{};
    bool normal_input_edge_consumed{}, normal_input_return_entered{}, normal_input_return_returned{}, normal_input_null_skip{};
    bool selected_vector_edge_consumed{}, selected_vector_entered{}, selected_vector_returned{};
    bool selected_input_edge_consumed{}, selected_input_entered{}, selected_input_returned{};
    bool normal_returned{}, failed{};
};

// Caller retains the entire nonmoving Frame/list and unresolved native graph on
// failure, even after outer cleanup ends filename/input headers. Historical
// status/destruction is metadata-only; no native cleanup occurs in destructors.
class NativeLocaleTextualParserFrame final {
public:
    explicit NativeLocaleTextualParserFrame(const NativeLocaleTextualParserContext&) noexcept;
    ~NativeLocaleTextualParserFrame() noexcept;
    NativeLocaleTextualParserFrame(const NativeLocaleTextualParserFrame&) = delete;
    NativeLocaleTextualParserFrame& operator=(const NativeLocaleTextualParserFrame&) = delete;
    NativeLocaleTextualParserFrame(NativeLocaleTextualParserFrame&&) = delete;
    NativeLocaleTextualParserFrame& operator=(NativeLocaleTextualParserFrame&&) = delete;
    NativeLocaleTextualParserProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept {
        return progress_.phase == NativeLocaleTextualParserPhase::returned && !progress_.failed;
    }
private:
    friend void parse_native_locale_textual_option1_00aa0020(
        const NativeLocaleTextualParserContext&, NativeLocaleTextualParserFrame&);
    struct FileAttempt;
    const NativeLocaleTextualParserContext& context_;
    alignas(std::uint32_t) unsigned char vector_[0x0c]; // uninitialized until actual entry
    std::uint32_t cursor_{}, end_{};
    FileAttempt* files_{};
    FileAttempt* last_{};
    int state_{-2}; // Source-unentered; actual entered states0/1, consumed=-1
    NativeLocaleTextualParserProgress progress_{};
    std::exception_ptr failure_{};
    void require_entered() const;
    void fail(std::exception_ptr) noexcept;
    [[noreturn]] void reject_entry();
    void unwind_owned() noexcept;
};

// New ordinary Source ABI; option1 is explicit, other DWORD remains opaque.
// Rejected invocation is outside its cleanup scope. Only the active admitted
// invocation consumes its own state0/input and state1/vector actions, never
// stream or record-key actions. No raw16/sidecar/full-AA/loading/ABI/runtime claim.
void parse_native_locale_textual_option1_00aa0020(
    const NativeLocaleTextualParserContext&, NativeLocaleTextualParserFrame&);
} // namespace bsp
