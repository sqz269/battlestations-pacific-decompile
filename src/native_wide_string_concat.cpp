#include "bsp/native_wide_string_concat.hpp"
#include "bsp/native_wide_string_raw_copy.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wide concat requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(Word) == 4);

Word word(const void* header, Word offset) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(header) + offset;
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    _ReadWriteBarrier();
    return value;
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

NativeWideStringConcatContext::NativeWideStringConcatContext(void* output,
    const void* left, const void* right, NativeStringRawPoolContext& strings) noexcept
    : output_(output), left_(left), right_(right), strings_(strings) {}

NativeWideStringConcatFrame::NativeWideStringConcatFrame(
    const NativeWideStringConcatContext& context) noexcept : context_(context) {}

void NativeWideStringConcatFrame::fail() noexcept {
    phase_ = Phase::failed;
    normal_result_ = false;
    progress_.failed = true;
}
[[noreturn]] void NativeWideStringConcatFrame::reject(const char* message) {
    fail(); // latch before diagnostic allocation/reentry
    throw std::logic_error(message);
}
void NativeWideStringConcatFrame::require_active() {
    if (phase_ != Phase::active || progress_.failed)
        reject("raw wide concat cannot resume after failure/reentry");
}
void NativeWideStringConcatFrame::unwind_selected() noexcept {
    if (!output_owed_) return;
    output_owed_ = false; // consume BEFORE the actual CURRENT-header action
    progress_.selected_edge_consumed = true;
    progress_.selected_destroy_entered = true;
    try {
        destroy_native_wide_string_header_00436430(context_.output_, context_.strings_);
        progress_.selected_destroy_returned = true;
    } catch (...) {
        std::terminate(); // true secondary exception on selected unwind
    }
}
void NativeWideStringConcatFrame::complete_and_transfer() {
    require_active();
    output_owed_ = false; // actual output transfers; no native cleanup
    progress_.normal_transfer = true;
    normal_result_ = true;
    progress_.normal_completed = true;
    phase_ = Phase::returned;
}

void* concatenate_native_wide_string_header_005cf570(
    const NativeWideStringConcatContext& c, NativeWideStringConcatFrame& f) {
    // Rejected inner invocation never enters the active outer cleanup try.
    if (&c != &f.context_ || f.phase_ != NativeWideStringConcatFrame::Phase::fresh)
        f.reject("raw wide concat requires its exact fresh Context/Frame");
    f.phase_ = NativeWideStringConcatFrame::Phase::active;
    f.progress_.attempted = true;
    try {
        f.progress_.copy_entered = true;
        f.copy_result_ = copy_construct_native_wide_string_header_004c8dd0(
            c.output_, c.left_, c.strings_);
        f.progress_.copy_returned = true;
        f.output_owed_ = true; // genuine private authority BEFORE any Source fence
        f.progress_.constructed_authority = true;
        // Native's physical bit1 is written AFTER right-length load. This
        // own-metadata-only fence is added Source policy; no physical stack/
        // hardware/FH3 parity is asserted for its extra reentry-failure path.
        f.require_active();
        f.right_length_ = word(c.right_, 0u); //5CF5A4, CURRENT after copy
        f.progress_.right_length_read = true;
        f.progress_.native_arm_epoch_passed = true; // diagnostic, NOT authority
        if (f.right_length_ != 0u) {
            f.old_output_length_ = word(c.output_, 0u); //5CF5B7
            f.progress_.old_output_length_read = true;
            const Word requested = f.old_output_length_ + f.right_length_;
            f.progress_.resize_entered = true;
            resize_native_wide_string_header_004c53e0(c.output_, c.strings_, requested, true);
            f.progress_.resize_returned = true;
            f.require_active();
            const Word right_data = word(c.right_, 4u); //5CF5C6 BEFORE output
            f.progress_.right_data_read = true;
            const Word output_data = word(c.output_, 4u); //5CF5C9
            f.progress_.output_data_read = true;
            const Word bytes = f.right_length_ * 2u;
            const Word output = output_data + f.old_output_length_ * 2u;
            if (bytes != 0u) {
                f.progress_.append_copy_entered = true;
                std::memmove(pointer(output), pointer(right_data), bytes);
                f.progress_.append_copy_returned = true;
            } else {
                // Qualified zero-byte omission AFTER all Native operand reads.
                f.progress_.zero_copy_omitted = true;
            }
            f.require_active();
        }
        f.complete_and_transfer();
        return c.output_;
    } catch (...) {
        f.fail();
        f.unwind_selected(); // constructor escape has output_owed=false
        throw;
    }
}
} // namespace bsp
