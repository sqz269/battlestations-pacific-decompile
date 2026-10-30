#include "bsp/native_wide_string_substring.hpp"
#include "bsp/native_wide_string.hpp"
#include "bsp/native_wide_string_raw_copy.hpp"
#include "bsp/native_crt_wcsncpy.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide substring requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 &&
    sizeof(Word) == 4 && sizeof(std::int32_t) == 4 && sizeof(unsigned int) == 4);

Word word(const void* header, Word offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(header) + offset;
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
void put(void* header, Word offset, Word value) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(header) + offset;
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(address), &value, sizeof value);
    _ReadWriteBarrier();
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

NativeWideStringSubstringContext::NativeWideStringSubstringContext(
    const void* source, void* output, std::int32_t position, std::uint32_t count,
    NativeStringRawPoolContext& strings) noexcept
    : source_(source), output_(output), position_(position), count_(count), strings_(strings) {}

NativeWideStringSubstringFrame::NativeWideStringSubstringFrame(
    const NativeWideStringSubstringContext& context) noexcept : context_(context) {}

void NativeWideStringSubstringFrame::fail() noexcept {
    phase_ = Phase::failed;
    normal_result_ = false;
    progress_.failed = true;
}
[[noreturn]] void NativeWideStringSubstringFrame::reject(const char* message) {
    fail(); // sticky before diagnostic allocation can reenter
    throw std::logic_error(message);
}
void NativeWideStringSubstringFrame::require_active() {
    if (phase_ != Phase::active || progress_.failed)
        reject("raw wide substring cannot resume after failure/reentry");
}
void NativeWideStringSubstringFrame::construct_output(bool nonempty_path) {
    progress_.output_copy_entered = true;
    output_copy_result_ = copy_construct_native_wide_string_header_004c8dd0(
        context_.output_, temp_, context_.strings_);
    progress_.output_copy_returned = true;
    output_owed_ = true; // actual normal output return BEFORE any Source fence
    progress_.output_authority = true;
    if (nonempty_path) {
        // Native770EF4/FC: flag1/state1->0 BEFORE footer or Source fence.
        temp_owed_ = false;
        progress_.native_output_bit_set = true;
        progress_.native_state0_after_copy = true;
        progress_.temp_edge_consumed_normal = true;
    }
    // Empty Native arm leaves physical flag0/state1. This added fence's private
    // constructed-output cleanup policy is NOT physical stack/FH3/fault parity.
    require_active();
}
void NativeWideStringSubstringFrame::unwind_selected() noexcept {
    if (temp_owed_) {
        temp_owed_ = false; // state1->0 consumes BEFORE CURRENT temp cleanup
        progress_.temp_selected_consumed = true;
        progress_.temp_destroy_entered = true;
        try {
            destroy_native_wide_string_header_00436430(temp_, context_.strings_);
            progress_.temp_destroy_returned = true;
        } catch (...) { std::terminate(); }
    }
    if (output_owed_) {
        output_owed_ = false; // consume private constructed bit BEFORE output
        progress_.output_selected_consumed = true;
        progress_.output_destroy_entered = true;
        try {
            destroy_native_wide_string_header_00436430(context_.output_, context_.strings_);
            progress_.output_destroy_returned = true;
        } catch (...) { std::terminate(); }
    }
}
void NativeWideStringSubstringFrame::complete_and_transfer() {
    require_active();
    if (temp_owed_) progress_.empty_normal_discards_temp_edge = true;
    temp_owed_ = false; // empty NORMAL return has no temp-return action
    output_owed_ = false; // actual output transfers without destruction
    progress_.normal_transfer = true;
    normal_result_ = true;
    progress_.normal_completed = true;
    phase_ = Phase::returned;
}

void* substring_native_wide_string_header_00770e50(
    const NativeWideStringSubstringContext& c, NativeWideStringSubstringFrame& f) {
    // Rejected inner entry MUST NOT run an active outer invocation's actions.
    if (&c != &f.context_ || f.phase_ != NativeWideStringSubstringFrame::Phase::fresh)
        f.reject("raw wide substring requires its exact fresh Context/Frame");
    f.phase_ = NativeWideStringSubstringFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        put(f.temp_, 0u, 0u); //770E70
        f.progress_.temp_length_zeroed = true;
        put(f.temp_, 4u, 0u); //770E74
        f.progress_.temp_data_zeroed = true;
        f.temp_owed_ = true; // Native state1 before source-field work
        f.progress_.temp_edge_armed = true;
        f.position_ = static_cast<Word>(c.position_);
        f.count_ = c.count_;
        if (c.position_ < 0) {
            f.progress_.negative_position = true;
            f.count_ += f.position_; // ADD bits, no initial source-length read
            f.position_ = 0u;
        } else {
            const Word initial_length = word(c.source_, 0u);
            f.progress_.initial_length_read = true;
            if (f.position_ > initial_length) f.position_ = initial_length;
        }
        f.source_data_ = word(c.source_, 4u); //770E9A before later current length
        f.progress_.source_data_read = true;
        bool nonempty_path = false;
        if (f.source_data_ != 0u) {
            f.source_length_ = word(c.source_, 0u); //770EA5, distinct CURRENT epoch
            f.progress_.current_length_read = true;
            nonempty_path = f.source_length_ != 0u && f.position_ < f.source_length_;
        }
        if (nonempty_path) {
            const Word end = f.position_ + f.count_; // DWORD wrap, NOT widened min
            if (end > f.source_length_) {
                f.count_ = f.source_length_ - f.position_;
                f.progress_.count_clamped = true;
            }
            f.source_start_ = f.source_data_ + f.position_ * 2u; //BEFORE resize
            f.progress_.source_start_captured = true;
            f.progress_.resize_entered = true;
            resize_native_wide_string_header_004c53e0(f.temp_, c.strings_, f.count_, true);
            f.progress_.resize_returned = true;
            f.temp_data_ = word(f.temp_, 4u); //770ED1, unconditional including count0
            f.progress_.temp_data_captured = true;
            f.require_active();
            if (f.count_ != 0u) {
                f.progress_.crt_copy_entered = true;
                f.crt_result_ = copy_native_crt_wide_units_00bfaeef(
                    pointer(f.temp_data_), pointer(f.source_start_), f.count_);
                f.progress_.crt_copy_returned = true;
                f.require_active();
            }
            f.construct_output(true); // consumes temp edge BEFORE healthy/footer
            if (f.temp_data_ != 0u) {
                // EARLY DATA remains fixed; CURRENT temp length is AFTER output ctor.
                f.temp_return_bytes_ = word(f.temp_, 0u) * 2u + 2u;
                f.progress_.temp_getter_entered = true;
                auto* const pool = native_string_pool_get_or_create_00419cc0(
                    c.strings_.actual_published_01090aa8,
                    c.strings_.actual_manager_publication_01090aa0);
                f.progress_.temp_getter_returned = true;
                f.require_active();
                f.progress_.temp_return_entered = true;
                return_native_string_pool_00bd1510(pool, pointer(f.temp_data_), f.temp_return_bytes_,
                    c.strings_.actual_small_returns_disabled_01090aa4);
                f.progress_.temp_return_returned = true;
                f.require_active(); // no native temp read after return
            } else {
                f.progress_.temp_footer_null_skip = true;
            }
        } else {
            f.progress_.empty_path = true;
            f.construct_output(false); // genuine copy from actual zero temp, no synthetic result
        }
        f.complete_and_transfer();
        return c.output_;
    } catch (...) {
        f.fail();
        f.unwind_selected(); // TEMP first if armed; output only after actual ctor normal
        throw;
    }
}
} // namespace bsp
