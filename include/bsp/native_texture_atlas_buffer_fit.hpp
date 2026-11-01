#pragma once
#include "bsp/native_particle_text_loader.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
enum class NativeAtlasBufferFitPhase { fresh, entered, complete, failed };
enum class NativeAtlasBufferFitFailure {
    none, missing_load_record, different_buffer, different_allocation,
    released_allocation, invalid_logical_range, extent_exceeds_allocation,
    extent_exceeds_completed_read, wrapped_range, insufficient_shared_scratch,
    signed_terminal_cursor_wrap
};
// Ordinary Source admission history, no Native ownership/cleanup capability.
// Retain with the actual descriptor/load/allocator/provider graph on escape.
struct NativeTextureAtlasBufferFitFrame final {
    NativeTextureAtlasBufferFitFrame() = default;
    NativeTextureAtlasBufferFitFrame(const NativeTextureAtlasBufferFitFrame&) = delete;
    NativeTextureAtlasBufferFitFrame& operator=(const NativeTextureAtlasBufferFitFrame&) = delete;
    NativeAtlasBufferFitPhase phase{NativeAtlasBufferFitPhase::fresh};
    NativeAtlasBufferFitFailure reason{NativeAtlasBufferFitFailure::none};
    const void* actual_buffer{};
    const void* current_data{};
    std::uint32_t current_cursor{}, logical_extent{}, allocation_extent{}, completed_read{};
    std::size_t physical_shared_scratch_extent{}, required_scratch_extent{};
    std::uint32_t line_count{}, peak_normalized_bytes{};
    bool no_data{};
    std::exception_ptr failure;
};

// Source-only pre-parser admission. The retained REAL AF5850 call supplies its
// own allocation/read history; no loose-file name, strlen, numeric profile or
// caller-fabricated extent substitutes for it. The separate scratch extent MUST
// come from the same process owner's actual allocated scratch used by parser.
// Scan the WHOLE current logical byte buffer conservatively, without changing
// cursor/data/output, invoking Native helpers/callbacks, or materializing text.
// LF only; signed leading <=20h trimmed, signed interior >=20h copied; +1 NUL.
// No input NUL is required. Stable valid extents/backing, same live graph and
// externally serialized/nonreentrant use remain caller obligations. A later
// DATA/extent/payload mutation requires fresh admission, not reuse of this record.
// Failure throws BEFORE parser entry; it never frees, retries, clamps or resizes.
void require_native_texture_atlas_buffer_scratch_fit(const void* actual_buffer1c,
    const NativeParticleTextBufferReadFrame& actual_completed_load,
    std::size_t actual_process_shared_scratch_extent,
    NativeTextureAtlasBufferFitFrame&);
} // namespace bsp
