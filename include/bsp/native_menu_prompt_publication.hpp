#pragma once

#include "bsp/native_menu_prompt_construction.hpp"

namespace bsp {
// Nonowning prepared frame, retained separately for each call/reentry. The
// genuine eight-byte CE37FC/section guard is initialized at its native epoch.
// It has no destructor-driven native effects and never retries construction.
struct NativeMenuPromptPublicationOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    std::uint32_t failure_native_site{};
    std::int32_t failure_unwind_state{-1};
    std::uint32_t guard_words[2];
    void* captured_allocation{};
    NativeMenuPromptConstructionOperation construction;
    NativeMenuPromptPublicationOperation() noexcept = default;
    NativeMenuPromptPublicationOperation(const NativeMenuPromptPublicationOperation&) = delete;
    NativeMenuPromptPublicationOperation& operator=(const NativeMenuPromptPublicationOperation&) = delete;
};

// Whole00425D10..00425DD9,202B/56ops. Native no-input getter/EAX/plainRET;
// this Source ABI borrows context and a fresh retained operation. Fast return
// is the first captured E18E6C. Slow path captures SAME manager+10 section,
// enters/increments its physical+18, rechecks, allocates real290h, calls the
// whole raw constructor, then publishes before singleton registration.
// Capture CURRENT publication+0C/null BEFORE the second manager lookup and
// realBD0C30. Normal section release precedes final CURRENT publication read.
void* get_native_menu_prompt_screen_00425d10(NativeMenuPromptConstructionContext&,
    NativeMenuPromptPublicationOperation&);

// A covered constructor failure frees only the captured raw allocation after
// constructor cleanup. Registration failure retains publication/allocation;
// full411EE0 runs over the captured guard, then the exception escapes. No raw95
// registration/current10 call is implicit here. No activation is performed.
// The caller must already retain genuine CED3AC/+0->532340 adjust-minus-C,
// current scalar532A00 and full existing5321F0 member destruction
// deletion and member providers in the SAME manager/application graph before
// this getter may register actual+0C. No successful fallback is supplied.
// Original no-input ABI/FH3/SEH/private-stack aliases and secondary faults are
// qualified as in the constructor; this is not full frontend Source readiness.
} // namespace bsp
