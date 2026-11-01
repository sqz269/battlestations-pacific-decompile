#pragma once
#include <cstdint>
#include <exception>

namespace bsp {
struct NativeStringRawPoolContext;
enum class NativeStringCopyConcatPhase { fresh, entered, complete, failed };
// Retained nonmoving Source frames. Only explicit reached stores initialize
// native DWORD homes. Private history grants no Native cleanup/Origin power.
struct NativeStringHeaderCopyExactFrame final {
    NativeStringHeaderCopyExactFrame() {}
    NativeStringHeaderCopyExactFrame(const NativeStringHeaderCopyExactFrame&)=delete;
    NativeStringHeaderCopyExactFrame& operator=(const NativeStringHeaderCopyExactFrame&)=delete;
    std::uint32_t preserve_home;
    NativeStringCopyConcatPhase phase{NativeStringCopyConcatPhase::fresh};
    std::uint32_t active_site{};
    std::exception_ptr failure;
};
struct NativeStringHeaderConcatExactFrame final {
    NativeStringHeaderConcatExactFrame() {}
    NativeStringHeaderConcatExactFrame(const NativeStringHeaderConcatExactFrame&)=delete;
    NativeStringHeaderConcatExactFrame& operator=(const NativeStringHeaderConcatExactFrame&)=delete;
    std::uint32_t preserve_home;
    NativeStringCopyConcatPhase phase{NativeStringCopyConcatPhase::fresh};
    std::uint32_t active_site{}, native_output_owned{};
    std::int32_t native_state{-1};
    std::exception_ptr failure;
};
// Whole ordinary NORMAL 426060, ECX output8 / source8 stack / EAX output / RET4.
// Identity captured before LEN0/DATA0. Otherwise exact41DD40 with live preserve
// HOME and actual feature; then CURRENT source LEN guard, CURRENT output LEN,
// source DATA, output DATA, and genuine BF7680 EVEN for copied count0.
void* copy_construct_native_string_header_00426060_exact(void* output8,
    const void* source8, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4,
    NativeStringHeaderCopyExactFrame&);
// Whole ordinary NORMAL 4261A0, ECX left8 / stack output8,right8 / EAX output /
// RET8. Initial copy reads output DATA before left DATA. Append length captured
// before ownership arm; post-resize reads right DATA then output DATA. Preserve
// wrapped lengths, actual overlap/access ordering, all reached zero-copy calls.
// Both APIs require SAME actual pool/feature, valid backing/CPU/DF/provider
// domains and disjoint private controls. Fresh retained frames cannot replay.
// Escaping C++ records failure and barethrows, retaining partial output/helper
// debt WITHOUT new outer cleanup. Native FH3/SEH/private-stack/fault/ABI and
// selected unwind behavior remain outside these NORMAL-only interfaces.
void* concatenate_native_string_headers_004261a0_exact(const void* left8,
    void* output8, const void* right8, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_0109eea4,
    NativeStringHeaderConcatExactFrame&);
} // namespace bsp
