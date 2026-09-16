#pragma once
#include <cstdint>

namespace bsp {
struct NativeRenderCommandQueueStorage;
struct NativeRenderCommandExecutionContext;
struct NativeRenderCommandExecutionFrame;
struct NativeRenderCommandRawEnvironment;
class NativeRenderActualOwners;

// Same actual command execution, canonical context/owner associations and raw
// lifetime/string domain. The command profile view contains original numeric
// targets; it is never invoked as a host table. Lifetime is needed only when
// current control0 reaches destruction of a nonnull current command.
struct NativeRenderQueueExecutionContext {
    NativeRenderCommandExecutionContext& actual_commands;
    NativeRenderActualOwners& actual_owners;
    const volatile std::uint32_t* actual_command_profile_00d5e5e0;
    NativeRenderCommandRawEnvironment* actual_lifetime{};
};

// Prepared persistent frame slots, one per reached command invocation. Null
// outer frame/slot is allowed only for readiness-false command paths. Nothing
// here prepares, owns, retires, cleans or replays a command/system/pass frame.
struct NativeRenderQueueExecutionFrame {
    NativeRenderCommandExecutionFrame* const* commands{};
    std::uint32_t capacity{};
    std::uint32_t used{};
};

// Full B1EBE0[187], native ECX actual34h queue, RET. Signed count loop,
// current list/count/control reloads, publish context before retain/release,
// current command virtual0 -> genuine B1D950; release CURRENT queue context
// after execution and clear after its callback. Control0 destroys/frees the
// CURRENT indexed command, then final currentcontrol0 shrinks list to zero.
// No outer exception cleanup or rollback. Null context is valid only when
// the initial signed command count is nonpositive.
void execute_native_render_queue_00b1ebe0(NativeRenderCommandQueueStorage&,
    NativeRenderQueueExecutionContext*, NativeRenderQueueExecutionFrame*);

// Source API adds borrowed contexts/frames and canonical source profile
// dispatch. It is not a binary ABI replacement; native FH3/SEH/register/fault,
// arbitrary concurrent mutation and active application/gameplay remain open.
} // namespace bsp
