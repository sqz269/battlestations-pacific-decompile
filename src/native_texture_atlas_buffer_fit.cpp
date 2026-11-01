#include "bsp/native_texture_atlas_buffer_fit.hpp"
#include <limits>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual atlas buffer admission requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word raw_word(const void* raw, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov eax, raw
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
[[noreturn]] void reject(NativeTextureAtlasBufferFitFrame& f,
    NativeAtlasBufferFitFailure reason, const char* message) {
    f.reason = reason;
    throw std::runtime_error(message);
}
}
void require_native_texture_atlas_buffer_scratch_fit(const void* buffer,
    const NativeParticleTextBufferReadFrame& load, std::size_t scratch_extent,
    NativeTextureAtlasBufferFitFrame& f) {
    using Phase = NativeAtlasBufferFitPhase;
    using Failure = NativeAtlasBufferFitFailure;
    if (f.phase != Phase::fresh)
        throw std::logic_error("atlas buffer fit record cannot replay");
    f.phase = Phase::entered;
    f.actual_buffer = buffer;
    f.physical_shared_scratch_extent = scratch_extent;
    try {
        if (scratch_extent == 0)
            reject(f, Failure::insufficient_shared_scratch, "atlas fit requires an explicit nonzero process scratch extent");
        if (!load.normal_returned())
            reject(f, Failure::missing_load_record, "atlas fit requires its completed actual AF5850 record");
        if (load.actual_buffer() != buffer)
            reject(f, Failure::different_buffer, "atlas fit received a different actual TextBuffer");
        const Word data_bits = raw_word(buffer, 0x14);
        f.current_data = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(data_bits));
        // AF5740 returns false before reading cursor/extent if DATA is null.
        // In particular preserve the existing AF5850 mismatch/free/null arm.
        if (data_bits == 0) {
            f.no_data = true;
            f.phase = Phase::complete;
            return;
        }
        if (!load.allocation_returned() || !load.read_returned() ||
            f.current_data != load.actual_allocation())
            reject(f, Failure::different_allocation, "atlas fit has no current actual allocated/read byte range");
        if (load.allocation_released())
            reject(f, Failure::released_allocation, "atlas fit allocation was already released");
        f.allocation_extent = load.allocation_extent();
        f.completed_read = load.completed_read_count();
        f.current_cursor = raw_word(buffer, 4);
        f.logical_extent = raw_word(buffer, 8);
        // The real newly loaded buffer starts at0. Negative logical fields are
        // outside this Source admission, rather than an unsigned huge scan.
        if ((f.current_cursor & 0x80000000u) || (f.logical_extent & 0x80000000u))
            reject(f, Failure::invalid_logical_range, "atlas fit requires nonnegative logical buffer fields");
        if (f.logical_extent > f.allocation_extent)
            reject(f, Failure::extent_exceeds_allocation, "atlas logical extent exceeds its actual allocation");
        if (f.logical_extent > f.completed_read)
            reject(f, Failure::extent_exceeds_completed_read, "atlas logical extent exceeds the completed actual read");
        if (f.allocation_extent > (std::numeric_limits<std::uintptr_t>::max)() - data_bits)
            reject(f, Failure::wrapped_range, "atlas allocation range wraps the Source address domain");
        const auto* const bytes = static_cast<const unsigned char*>(f.current_data);
        // AF57D6 always increments the terminal cursor. At signed INT_MAX,
        // an unterminated final line would advance it to80000000; AF5778's
        // next SIGNED comparison can then re-enter beyond the input range.
        // The final BYTE read is covered by both physical/read range proofs.
        // LF termination leaves cursor INT_MAX (last LF index INT_MAX-1).
        if (f.logical_extent == 0x7fffffffu && bytes[f.logical_extent - 1u] != 0x0au)
            reject(f, Failure::signed_terminal_cursor_wrap,
                "atlas final unterminated line would wrap the signed terminal cursor");
        // Whole-buffer bound also covers later parser cursor rewinds/line starts.
        // It assumes the already-required stable payload/DATA/extent between
        // this admission and the genuine line calls; no future-epoch theorem.
        Word cursor = 0;
        while (cursor < f.logical_extent) {
            Word start = cursor;
            while (cursor < f.logical_extent && bytes[cursor] != 0x0au) ++cursor;
            const Word end = cursor;
            while (start < end) {
                const unsigned char value = bytes[start];
                if (value >= 0x21u && value < 0x80u) break;
                ++start; // signed BYTE <=20h, including80..FF.
            }
            Word copied = 0;
            while (start < end) {
                const unsigned char value = bytes[start++];
                if (value >= 0x20u && value < 0x80u) ++copied;
            }
            ++f.line_count;
            if (copied > f.peak_normalized_bytes) f.peak_normalized_bytes = copied;
            const std::size_t required = static_cast<std::size_t>(copied) + 1u;
            if (required > f.required_scratch_extent) f.required_scratch_extent = required;
            ++cursor; // Also one past extent on a final unterminated line.
        }
        if (f.required_scratch_extent > scratch_extent)
            reject(f, Failure::insufficient_shared_scratch, "atlas normalized line plus NUL exceeds actual process scratch");
        f.phase = Phase::complete;
    } catch (...) {
        f.phase = Phase::failed;
        f.failure = std::current_exception();
        throw;
    }
}
} // namespace bsp
