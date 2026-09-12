#pragma once

#include <cstddef>
#include <cstdint>

// Row 5 of the fixed-step fan-out: the per-step callback list, 00874DE0, and the
// registration that fills it, 00875A80.
//
// docs/FIXED_STEP_CALLBACKS.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol.
//
// bsp/fixed_step_fanout.hpp declares the fan-out's host method
// run_fixed_step_callbacks_00874de0 and leaves the list itself unread. This header is
// that method's body.

namespace bsp {

// ---------------------------------------------------------------------------
// The node
//
// An intrusive doubly-linked list node embedded in whatever object registers. The
// offsets are the ones 00874DE0 and 00875A80 touch; nothing this packet read says what
// +04h or +08h hold.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kFixedStepCallbackVtableOffset = 0x00;     // 00874DF0
inline constexpr std::size_t kFixedStepCallbackPrevOffset = 0x0c;       // 00874E0C
inline constexpr std::size_t kFixedStepCallbackNextOffset = 0x10;       // 00874E07
inline constexpr std::size_t kFixedStepCallbackLinkedOffset = 0x18;     // 00874E0F
inline constexpr std::size_t kFixedStepCallbackRepeatingOffset = 0x19;  // 00874E03
// The vtable slot the run walks call: [[node]+10h], i.e. slot index 4.
inline constexpr std::size_t kFixedStepCallbackStepSlotOffset = 0x10;   // 00874DF8

// The three image addresses the list lives at. The head sentinel's `next` field is
// 00E0B748 and the tail sentinel is the node at 00E0B76C, whose `prev` field is
// therefore 00E0B778 -- which is exactly the slot 00875A80 reads and writes as the
// tail. The two sentinels make the unlink at 00874E13/00874E1C total: a node that is
// first or last patches a sentinel rather than a null.
inline constexpr std::uint32_t kFixedStepCallbackHeadNextSlot = 0x00e0b748;
inline constexpr std::uint32_t kFixedStepCallbackTailSentinel = 0x00e0b76c;
inline constexpr std::uint32_t kFixedStepCallbackTailPrevSlot = 0x00e0b778;

struct FixedStepCallbackNode {
    FixedStepCallbackNode* prev{nullptr};  // node+0Ch
    FixedStepCallbackNode* next{nullptr};  // node+10h
    bool linked{false};                    // node+18h
    bool repeating{false};                 // node+19h
    // Not a native field: the image address of the function [[node]+10h] resolves to,
    // carried so a report can name which callback a node stands for.
    std::uint32_t step_slot_target{0};
};

// Two sentinels, exactly as the image lays them out.
struct FixedStepCallbackList {
    FixedStepCallbackNode head{};
    FixedStepCallbackNode tail{};
};

void fixed_step_callback_list_init(FixedStepCallbackList& list) noexcept;

// ---------------------------------------------------------------------------
// 00875A80, the registration
//
// __thiscall void(node, bool repeating), RET 4 at 00875ADB, body 00875A80..00875ADD,
// under the critical section the singleton at 00875340 owns (ECX = node, the bool at
// [ESP+0Ch] read into DL at 00875AC1). The linked test is 00875A9B, the append
// 00875AA1..00875AB9, and the two byte writes 00875AC5 and 00875AC8 happen whether or
// not the node was already linked, so re-registering only changes the flag.
//
// Two call sites, which is the whole of the contract:
//   006F5767  ECX = ESI+310h (the node is embedded at +310h of the object 006F5610
//             constructs, vtable 00CFAFE0 whose slot 4 is 006F7360), repeating = 1
//             (PUSH 1 at 006F5735).
//   007AC095  ECX = ESI+3E4h, repeating = 0 (PUSH 0 at 007AC086). 007AC000 is slot 3
//             of the vtable at 00D051E8 and registers the node only on the branch that
//             found something at 006F2C30; the node's own vtable is written by that
//             object's constructor, which this packet did not find.
// So both the repeating and the one-shot form are real, and the run has to handle both.
// ---------------------------------------------------------------------------
void fixed_step_callback_register_00875a80(FixedStepCallbackList& list,
                                           FixedStepCallbackNode& node,
                                           bool repeating) noexcept;

// ---------------------------------------------------------------------------
// 00874DE0, the run
//
// __cdecl void(void), RET at 00874E29, body 00874DE0..00874E29. Sole caller 00875E3F,
// which pushes nothing: the body loads the fixed step from 00D0DE84 itself at
// 00874DF2 and pushes it for each callback. The call is __thiscall with the node in
// ECX and the float on the stack (00874DFB..00874E01), and no cleanup follows, so the
// callback is RET 4.
//
// The walk reads `next` AFTER the callback returns (00874E07), so a callback may
// safely unlink itself, and a non-repeating node is unlinked right after it fires. The
// walk takes no lock, unlike the registration.
// ---------------------------------------------------------------------------
struct FixedStepCallbackHost {
    virtual ~FixedStepCallbackHost() = default;
    // 00874E01, CALL EDX through [[node]+10h], __thiscall(node, float), RET 4.
    virtual void invoke_step_callback(FixedStepCallbackNode& node, float step) = 0;
};

// Returns how many callbacks fired.
std::int32_t fixed_step_callback_list_run_00874de0(FixedStepCallbackList& list, float step,
                                                   FixedStepCallbackHost& host);

// ---------------------------------------------------------------------------
// The two registrants, for a report that wants to name them
// ---------------------------------------------------------------------------
struct FixedStepCallbackRegistrant {
    std::uint32_t call_site;       // the 00875A80 call site
    std::uint32_t registrant;      // the function containing it
    std::uint32_t node_offset;     // the ECX the site sets, as owner+offset
    bool repeating;                // the byte the site pushes
    std::uint32_t node_vtable;     // 0 when this packet did not find it
    std::uint32_t step_slot_target;  // [node_vtable+10h], 0 when unknown
};

inline constexpr std::size_t kFixedStepCallbackRegistrantCount = 2;
const FixedStepCallbackRegistrant& fixed_step_callback_registrant(std::size_t index) noexcept;

}  // namespace bsp
