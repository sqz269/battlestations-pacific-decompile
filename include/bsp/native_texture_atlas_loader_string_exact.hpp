#pragma once
#include "bsp/native_string_copy_concat_exact.hpp"
#include <cstdint>
#include <exception>
#include <list>

namespace bsp {
struct NativeStringRawPoolContext;
enum class NativeAtlasLoaderStringPhase { fresh, entered, complete, failed };

// Source history is disjoint from all supplied native bytes/HOMEs. Only reached
// explicit stores initialize these raw native homes; default preimages are not
// fabricated. Frames do not move or replay, and remain with the whole partial
// graph after an escaping child. A destructor grants no native cleanup power.
struct NativeAtlasStringAssignFrame final {
    NativeAtlasStringAssignFrame() {}
    NativeAtlasStringAssignFrame(const NativeAtlasStringAssignFrame&)=delete;
    NativeAtlasStringAssignFrame& operator=(const NativeAtlasStringAssignFrame&)=delete;
    std::uint32_t preserve_home;
    NativeAtlasLoaderStringPhase phase{NativeAtlasLoaderStringPhase::fresh};
    std::uint32_t active_site{};
    std::exception_ptr failure;
};
struct NativeAtlasStringSubstringFrame final {
    NativeAtlasStringSubstringFrame() {}
    NativeAtlasStringSubstringFrame(const NativeAtlasStringSubstringFrame&)=delete;
    NativeAtlasStringSubstringFrame& operator=(const NativeAtlasStringSubstringFrame&)=delete;
    std::uint32_t native_temporary[2], native_output_flag, preserve_home;
    NativeStringHeaderCopyExactFrame output_copy;
    NativeAtlasLoaderStringPhase phase{NativeAtlasLoaderStringPhase::fresh};
    std::uint32_t active_site{}, captured_start{}, captured_count{};
    std::int32_t native_state{-1};
    void* captured_begin{};
    void* captured_temporary_data{};
    void* returned_output{};
    std::exception_ptr failure;
};
struct NativeAtlasStringReplaceIteration final {
    NativeAtlasStringReplaceIteration() {}
    NativeAtlasStringReplaceIteration(const NativeAtlasStringReplaceIteration&)=delete;
    NativeAtlasStringReplaceIteration& operator=(const NativeAtlasStringReplaceIteration&)=delete;
    std::uint32_t suffix_homes[3], prefix_homes[3], preserve_home;
    NativeAtlasStringSubstringFrame suffix, prefix;
    NativeStringHeaderConcatExactFrame first_concat, second_concat;
    NativeAtlasLoaderStringPhase phase{NativeAtlasLoaderStringPhase::fresh};
    std::uint32_t active_site{};
    std::exception_ptr failure;
};
struct NativeAtlasStringReplaceFrame final {
    NativeAtlasStringReplaceFrame() {}
    NativeAtlasStringReplaceFrame(const NativeAtlasStringReplaceFrame&)=delete;
    NativeAtlasStringReplaceFrame& operator=(const NativeAtlasStringReplaceFrame&)=delete;
    // Native locals S+10..S+2F after all four nonvolatile pushes: joined,
    // first concat, prefix, suffix. Reused on each ordinary native iteration.
    std::uint32_t native_headers[8];
    std::list<NativeAtlasStringReplaceIteration> iterations;
    NativeAtlasLoaderStringPhase phase{NativeAtlasLoaderStringPhase::fresh};
    std::uint32_t active_site{}, position{};
    std::int32_t native_state{-1};
    void* captured_replacement{};
    std::exception_ptr failure;
};

// Whole normal 425F40: ECX destination, stacked source, EAX destination/RET4.
// Identity skips all reads. Exact41DD40 consumes a reached BYTE1 HOME; later
// CURRENT source LEN gates output LEN/source DATA/output DATA and genuine
// BF7680, including a reached zero-count call. No automatic cleanup/rollback.
void* assign_native_atlas_string_header_00425f40_exact(void* destination8,
    const void* source8, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4,
    NativeAtlasStringAssignFrame&);

// Whole normal469840: ECX source; stacked output/start/count HOMEs, RET0C/EAX
// output. Incoming homes are ordinary initialized writable DWORD backing.
// Start/count captured in native order; output HOME is reloaded only at its
// actual reached point AFTER temp resize/BF9280. Reuse the existing whole
// concrete BF9280 Source provider, including its actual NUL-padding, overlap
// and reached BYTE/DWORD-read domain; no new vendor reconstruction credit.
// A captured temp DATA is returned with CURRENT temp LEN+1 after exact426060.
void* substring_native_atlas_string_header_00469840_exact(const void* source8,
    volatile std::uint32_t (&output_start_count_homes)[3],
    NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4,
    NativeAtlasStringSubstringFrame&);

// Whole normal4CAD40, ECX mutable8; stacked search/replacement/count HOMEs,
// RET0C. Initial search reads search DATA before value DATA. Capture replacement
// HOME once after initial match; reload search HOME on each reached iteration.
// Adopt distinct suffix/prefix/concat frames before first child effect; rebuild
// through exact469840/4261A0 plus the original inline assignment read schedule.
// Return joined/head/prefix/suffix in native order; compute next search BEFORE
// decrementing the live count HOME (including initial zero wrapping to FFFFFFFF).
void replace_native_atlas_string_substrings_004cad40_exact(void* value8,
    volatile std::uint32_t (&search_replacement_count_homes)[3],
    NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4,
    NativeAtlasStringReplaceFrame&);

// These NORMAL-only Source interfaces require the genuine same raw pool/live
// feature/canonical code and valid actual extents, CPU/FP/DF/CRT/provider graph.
// No original registers/private stack/FH3/SEH/fault-PC/NoThrow/Origin, outer
// unwind, binary compatibility, startup or game validation is asserted.
} // namespace bsp
