#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This recovered x87 kernel requires MSVC Win32.
#endif

namespace bsp {
struct GameTuningBlock;
// Native 009BE150: ECX=destination, stack=source, EAX=destination, RET4.
// The declared fastcall bridge reserves unused EDX so source remains on stack.
// Each block must expose at least A4h live bytes; exact self alias and disjoint
// spans are admitted. Partial overlap is outside the validated source domain.
// Forty ordered FLD/FSTP binary32 lanes at 00h..64h and 6Ch..A0h, with byte
// copies at 68h/69h between them. Bytes6Ah/6Bh are untouched. Self alias must
// still execute: signaling NaNs and ambient x87 status can change.
// Requires masked x87 exceptions, no pending unmasked exception, and a free
// x87 stack slot. Preserves the control word and existing stack values; status
// follows the actual loads/stores. No default block, pointer replacement,
// allocation, singleton binding, fault recovery or game lifetime is supplied.
void* __fastcall copy_native_land_follow_parameters_009be150(
    void* destination, const void* unused_edx, const void* source) noexcept;

// Owns the parameter payload reached through follow-state+6Ch. The gameplay
// host detects entry by a gap in its pilot-think sequence; repeated uses within
// one think and consecutive follow thinks retain the same payload. This is the
// host's existing entry approximation, not the native state-machine lifecycle.
class FollowParameterSnapshot {
public:
    bool observe_follow_think(const GameTuningBlock&, std::uint64_t think) noexcept;
    float safe_alt() const;
private:
    alignas(4) std::array<std::byte, 0xa4> parameters_;
    std::uint64_t last_think_{};
    bool initialized_{};
};
} // namespace bsp

