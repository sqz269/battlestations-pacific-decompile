#include "bsp/native_locale_parser_record_storage.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstring>
#include <intrin.h>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale completed-record storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::uint16_t) == 2);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
template<class T> T read(Word location) noexcept {
    T value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(location), sizeof(value));
    _ReadWriteBarrier();
    return value;
}
template<class T> void store(Word location, T value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer(location), &value, sizeof(value));
    _ReadWriteBarrier();
}
Word cstring_end(Word current) noexcept {
    while (read<std::uint8_t>(current) != 0u) ++current;
    return current;
}
Word wide_length(Word begin) noexcept {
    Word after = begin;
    std::uint16_t unit;
    do {
        unit = read<std::uint16_t>(after);
        after += 2u;
    } while (unit != 0u);
    const Word bytes = after - (begin + 2u);
    return (bytes >> 1u) | (bytes & 0x80000000u); // native SAR32(bytes,1)
}
} // namespace

NativeLocaleParserRecordContext::NativeLocaleParserRecordContext(void* actual_manager,
    void* mode, void* index, void* token1, const void* token2, void* wide,
    void* delimiter_slot, NativeStringRawPoolContext& raw_strings,
    game::GameNativeLocalePoolProcess& owner, const game::GameNativeReadOnlyData& data)
    : manager(actual_manager), parser_mode_dword(mode), index_dword(index),
      first_token(token1), second_token(token2), wide_buffer(wide),
      current_wide_slot(delimiter_slot), strings(raw_strings), pool_owner(owner),
      actual_dot_word_00ce3a70(data.data_at(0x00ce3a70u, 2u)) {}

NativeLocaleParserRecordFrame::NativeLocaleParserRecordFrame(
    const NativeLocaleParserRecordContext& context) noexcept
    : context_(context),
      map_context_(pointer(address(context.manager) + 0x14u), key8_,
          context.strings, context.pool_owner), map_frame_(map_context_) {
    progress_.context = &context;
    progress_.key_header = key8_; // identity only; no Native header read/init
}
bool NativeLocaleParserRecordFrame::has_normal_result() const noexcept {
    return progress_.phase == NativeLocaleParserRecordPhase::returned &&
        progress_.normal_returned && !progress_.failed && !failure_;
}
void* NativeLocaleParserRecordFrame::normal_result() const noexcept {
    return has_normal_result() ? progress_.value_header : nullptr;
}
[[noreturn]] void NativeLocaleParserRecordFrame::reject(const char* message) {
    progress_.failed = true;
    progress_.phase = NativeLocaleParserRecordPhase::failed;
    throw std::logic_error(message);
}
void NativeLocaleParserRecordFrame::require_entered() {
    if (progress_.phase != NativeLocaleParserRecordPhase::entered || progress_.failed || failure_)
        reject("locale record cannot resume after caught reentry/failure");
}
void NativeLocaleParserRecordFrame::fail(std::exception_ptr failure) noexcept {
    progress_.failed = true;
    progress_.phase = NativeLocaleParserRecordPhase::failed;
    if (!failure_) failure_ = failure;
}
[[noreturn]] void NativeLocaleParserRecordFrame::selected_key_failure(std::exception_ptr original) {
    fail(original);
    key_owned_ = false; // consume state2 ->1 BEFORE actual CURRENT header cleanup
    progress_.native_key_armed = false;
    progress_.selected_key_return_entered = true;
    try {
        destroy_native_string_header_0041dd20(key8_, context_.strings);
        progress_.selected_key_return_returned = true;
    } catch (...) {
        selected_return_failure_ = std::current_exception();
        std::terminate(); // ordinary true-unwind second-failure policy; no retry
    }
    std::rethrow_exception(original);
}

void* store_native_locale_parser_record_00aa0130(
    const NativeLocaleParserRecordContext& context, NativeLocaleParserRecordFrame& frame) {
    auto& p = frame.progress_;
    try {
        if (&context != &frame.context_ || p.phase != NativeLocaleParserRecordPhase::prepared ||
            p.attempted || p.failed || frame.failure_)
            frame.reject("locale record requires its exact fresh prepared context/frame");
        p.attempted = true;
        p.phase = NativeLocaleParserRecordPhase::entered;
        if (!context.manager || !context.parser_mode_dword || !context.index_dword ||
            !context.first_token || !context.second_token || !context.wide_buffer ||
            !context.current_wide_slot)
            frame.reject("locale record requires actual completed-record backing");

        // AA0130..013F: eligibility was established by the REAL caller branch.
        store(address(context.parser_mode_dword), Word{1});
        store(address(context.current_wide_slot), std::uint16_t{0});
        store(address(context.current_wide_slot) - 2u, std::uint16_t{0});
        store(address(context.index_dword), Word{0});
        p.delimiter_stores_returned = true;

        Word destination = cstring_end(address(context.first_token));
        store(destination, read<std::uint16_t>(address(context.actual_dot_word_00ce3a70)));
        Word source = address(context.second_token);
        const Word second_end = cstring_end(source);
        const Word count = second_end + 1u - source; // INCLUDING NUL
        destination = cstring_end(address(context.first_token)); // genuine rescan
        for (Word i = 0u; i < (count >> 2u); ++i) {
            const Word value = read<Word>(source);
            store(destination, value); // forward REP MOVSD group ordering
            source += 4u;
            destination += 4u;
        }
        for (Word i = 0u; i < (count & 3u); ++i) {
            const auto value = read<std::uint8_t>(source);
            store(destination, value); // then forward REP MOVSB remainder
            ++source;
            ++destination;
        }
        p.tokens_joined = true;
        store(address(frame.key8_), Word{0});
        store(address(frame.key8_) + 4u, Word{0});
        p.key_zeroed = true;
        const Word key_length = cstring_end(address(context.first_token)) - address(context.first_token);
        p.key_resize_entered = true;
        resize_native_string_header_0041dd40(frame.key8_, context.strings, key_length, true);
        p.key_resize_returned = true;
        frame.early_key_data_ = read<Word>(address(frame.key8_) + 4u);
        frame.early_key_length_ = read<Word>(address(frame.key8_));
        p.early_key_data = pointer(frame.early_key_data_);
        p.early_key_length = frame.early_key_length_;
        frame.require_entered(); // state1; Source metadata failure retains
        if (frame.early_key_data_ != 0u) {
            const Word bytes = frame.early_key_length_ + 1u;
            if (bytes != 0u)
                std::memmove(pointer(frame.early_key_data_), context.first_token, bytes);
        }
        p.key_copy_returned = true;
        frame.require_entered(); // OUTSIDE any selected map/wide boundary

        frame.key_owned_ = true; // AA01E1: state2 ONLY immediately before map
        p.native_key_armed = true;
        p.map_entered = true;
        void* value;
        try {
            value = insert_or_get_native_locale_value_00a9fc30(frame.map_context_, frame.map_frame_);
        } catch (...) {
            frame.selected_key_failure(std::current_exception());
        }
        p.value_header = value;
        p.map_returned = true;
        frame.require_entered(); // metadata/reentry fences NEVER select cleanup
        if (!frame.map_frame_.has_normal_result() || frame.map_frame_.normal_result() != value)
            frame.reject("locale record requires the exact map normal-result borrow");

        const Word requested = wide_length(address(context.wide_buffer)); // AFTER map
        p.requested_wide_length = requested;
        p.wide_resize_entered = true;
        try {
            resize_native_wide_string_header_004c53e0(value, context.strings, requested, true);
        } catch (...) {
            frame.selected_key_failure(std::current_exception());
        }
        p.wide_resize_returned = true;
        frame.require_entered(); // OUTSIDE actual callee catch
        Word output = read<Word>(address(value) + 4u); // CURRENT once AFTER resize
        if (output != 0u) {
            Word input = address(context.wide_buffer);
            std::uint16_t unit;
            do {
                unit = read<std::uint16_t>(input);
                store(output, unit);
                input += 2u;
                output += 2u;
            } while (unit != 0u); // real streaming WORD copy INCLUDING NUL
        }
        p.wide_copy_returned = true;
        frame.require_entered();
        frame.key_owned_ = false; // AA023A state2 ->1 BEFORE captured footer
        p.native_key_armed = false;
        p.normal_key_edge_consumed = true;
        if (frame.early_key_data_ != 0u) {
            const Word bytes = frame.early_key_length_ + 1u; // EARLY, not header
            p.normal_key_return_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                context.strings.actual_published_01090aa8,
                context.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, pointer(frame.early_key_data_), bytes,
                context.strings.actual_small_returns_disabled_01090aa4);
            p.normal_key_return_returned = true;
        } else {
            p.normal_key_null_skip = true;
        }
        p.key_return_completed = true;
        frame.require_entered(); // no ended key/value/header reads
        p.normal_returned = true;
        p.phase = NativeLocaleParserRecordPhase::returned;
        return value; // borrowed map value, never caller-destroyed
    } catch (...) {
        frame.fail(std::current_exception());
        throw; // metadata ONLY; no generic native cleanup/rollback/retry
    }
}
} // namespace bsp
