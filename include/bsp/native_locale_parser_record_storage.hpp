#pragma once

#include "bsp/native_locale_map_insertion.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Actual completed-record operands, NOT a branch-ready flag or scanner.
// Caller proves genuine AA0130 entry: mode3, just-stored0D00 and prior0A00.
// Context preparation reads no buffer/key/map payload and creates no owner.
struct NativeLocaleParserRecordContext final {
    void* const manager;
    void* const parser_mode_dword;
    void* const index_dword;
    void* const first_token;
    const void* const second_token;
    void* const wide_buffer;
    void* const current_wide_slot;
    NativeStringRawPoolContext& strings;
    game::GameNativeLocalePoolProcess& pool_owner;
    const void* const actual_dot_word_00ce3a70;

    NativeLocaleParserRecordContext(void* actual_manager, void* mode, void* index,
        void* token1, const void* token2, void* wide, void* delimiter_slot,
        NativeStringRawPoolContext&, game::GameNativeLocalePoolProcess&,
        const game::GameNativeReadOnlyData& same_retained_data);
    NativeLocaleParserRecordContext(const NativeLocaleParserRecordContext&) = delete;
    NativeLocaleParserRecordContext& operator=(const NativeLocaleParserRecordContext&) = delete;
    NativeLocaleParserRecordContext(NativeLocaleParserRecordContext&&) = delete;
    NativeLocaleParserRecordContext& operator=(NativeLocaleParserRecordContext&&) = delete;
};

enum class NativeLocaleParserRecordPhase { prepared, entered, returned, failed };
struct NativeLocaleParserRecordProgress {
    NativeLocaleParserRecordPhase phase{NativeLocaleParserRecordPhase::prepared};
    const NativeLocaleParserRecordContext* context{};
    void* key_header{};
    void* early_key_data{};
    std::uint32_t early_key_length{};
    void* value_header{};
    std::uint32_t requested_wide_length{};
    bool attempted{}, delimiter_stores_returned{}, tokens_joined{}, key_zeroed{};
    bool key_resize_entered{}, key_resize_returned{}, key_copy_returned{};
    bool native_key_armed{}, map_entered{}, map_returned{};
    bool wide_resize_entered{}, wide_resize_returned{}, wide_copy_returned{};
    bool selected_key_return_entered{}, selected_key_return_returned{};
    bool normal_key_edge_consumed{}, normal_key_return_entered{};
    bool normal_key_return_returned{}, normal_key_null_skip{}, key_return_completed{};
    bool normal_returned{}, failed{};
};

class NativeLocaleParserRecordFrame final {
public:
    explicit NativeLocaleParserRecordFrame(const NativeLocaleParserRecordContext&) noexcept;
    ~NativeLocaleParserRecordFrame() = default; // NO native cleanup/stop/recovery
    NativeLocaleParserRecordFrame(const NativeLocaleParserRecordFrame&) = delete;
    NativeLocaleParserRecordFrame& operator=(const NativeLocaleParserRecordFrame&) = delete;
    NativeLocaleParserRecordFrame(NativeLocaleParserRecordFrame&&) = delete;
    NativeLocaleParserRecordFrame& operator=(NativeLocaleParserRecordFrame&&) = delete;

    NativeLocaleParserRecordProgress progress() const noexcept { return progress_; }
    NativeLocaleMapInsertionProgress map_progress() const noexcept { return map_frame_.progress(); }
    NativeLocaleNodeAllocationProgress allocation_progress() const noexcept {
        return map_frame_.allocation_progress();
    }
    std::exception_ptr failure() const noexcept { return failure_; }
    std::exception_ptr selected_return_failure() const noexcept { return selected_return_failure_; }
    bool has_normal_result() const noexcept;
    void* normal_result() const noexcept;

private:
    friend void* store_native_locale_parser_record_00aa0130(
        const NativeLocaleParserRecordContext&, NativeLocaleParserRecordFrame&);
    const NativeLocaleParserRecordContext& context_;
    alignas(4) std::byte key8_[8]; // uninitialized until AA0192/196
    NativeLocaleMapInsertionContext map_context_;
    NativeLocaleMapInsertionFrame map_frame_;
    NativeLocaleParserRecordProgress progress_{};
    std::exception_ptr failure_, selected_return_failure_;
    std::uint32_t early_key_data_{}, early_key_length_{};
    bool key_owned_{};
    [[noreturn]] void reject(const char*);
    void require_entered();
    void fail(std::exception_ptr) noexcept;
    [[noreturn]] void selected_key_failure(std::exception_ptr);
};

// Ordinary Source fragment [AA0130,AA0257), NOT full AA0020/parser. ALL raw
// operands/proven entry must come from a genuine caller. Install and retain
// this frame/context/key/nested map attempt BEFORE effects and through any
// unresolved failure; never discard stack-local failure custody. Only actual
// escaping map/wide callee calls select CURRENT key cleanup. Source metadata
// fences retain instead; normal footer uses EARLY data/len and never retries.
// No stream/provider synthesis, rollback, value destruction, loading/startup,
// Native ABI, full parser/raw16 or runtime permission is supplied.
void* store_native_locale_parser_record_00aa0130(
    const NativeLocaleParserRecordContext&, NativeLocaleParserRecordFrame&);
} // namespace bsp
