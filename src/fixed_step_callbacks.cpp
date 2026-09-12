#include "bsp/fixed_step_callbacks.hpp"

// 00874DE0 and 00875A80, the fixed step's callback list.
// docs/FIXED_STEP_CALLBACKS.md carries the addresses, the original ABI and the
// uncertainty.

namespace bsp {
namespace {

constexpr FixedStepCallbackRegistrant kRegistrants[kFixedStepCallbackRegistrantCount] = {
    // 006F5767 inside 006F5610. EBP = ESI+310h (006F5676) and the node's vtable is
    // written there at 006F567C as 00CFAFE0, whose slot 4 ([vtable+10h]) is 006F7360.
    // PUSH 1 at 006F5735, so the node stays linked for the life of the object.
    {0x006f5767, 0x006f5610, 0x310, true, 0x00cfafe0, 0x006f7360},
    // 007AC095 inside 007AC000, which is slot 3 of the vtable at 00D051E8. ECX =
    // ESI+3E4h (007AC088) and PUSH 0 at 007AC086, so this registration fires once and
    // unlinks itself. The node's vtable is written by the owner's constructor, which
    // this packet did not find, so the target is unknown.
    {0x007ac095, 0x007ac000, 0x3e4, false, 0, 0},
};

}  // namespace

const FixedStepCallbackRegistrant& fixed_step_callback_registrant(std::size_t index) noexcept {
    if (index >= kFixedStepCallbackRegistrantCount) {
        index = 0;
    }
    return kRegistrants[index];
}

void fixed_step_callback_list_init(FixedStepCallbackList& list) noexcept {
    // The image's two sentinels: the head's next slot is 00E0B748 and the tail's prev
    // slot is 00E0B778, both of which start out pointing at the other sentinel.
    list.head.next = &list.tail;
    list.head.prev = nullptr;
    list.tail.prev = &list.head;
    list.tail.next = nullptr;
    list.head.linked = false;
    list.tail.linked = false;
}

void fixed_step_callback_register_00875a80(FixedStepCallbackList& list,
                                           FixedStepCallbackNode& node,
                                           bool repeating) noexcept {
    // 00875A84..00875A97 and 00875ABF..00875AD3: the critical section the singleton at
    // 00875340 owns is taken around the whole body through the imports at 00CE2218 and
    // 00CE2210, and a depth counter next to it is bumped. The list walk at 00874DE0 does
    // not take it.
    if (!node.linked) {
        // 00875AA1..00875AB9: append at the tail. node->prev = the current tail,
        // node->next = the tail sentinel, tail->next = node, tail slot = node.
        FixedStepCallbackNode* const tail = list.tail.prev;
        node.prev = tail;
        node.next = &list.tail;
        if (tail != nullptr) {
            tail->next = &node;
        }
        list.tail.prev = &node;
    }
    // 00875AC5 and 00875AC8, outside the `linked` test: re-registering an already
    // linked node only rewrites its repeating flag.
    node.repeating = repeating;
    node.linked = true;
}

std::int32_t fixed_step_callback_list_run_00874de0(FixedStepCallbackList& list, float step,
                                                   FixedStepCallbackHost& host) {
    std::int32_t fired = 0;

    // 00874DE1: the walk starts from the head sentinel's next slot and stops at the
    // tail sentinel.
    FixedStepCallbackNode* node = list.head.next;
    while (node != nullptr && node != &list.tail) {
        // 00874DF0..00874E01. The step is the image float at 00D0DE84, loaded by the
        // callee and not passed by the fan-out site.
        host.invoke_step_callback(*node, step);

        // 00874E07: `next` is read after the callback returns, so a callback that
        // registers or unlinks something is seen by this same walk.
        FixedStepCallbackNode* const next = node->next;

        // 00874E03..00874E1C: a node whose repeating byte is clear is unlinked
        // immediately after it fires. Both sentinels make this total.
        if (!node->repeating) {
            node->linked = false;
            if (node->prev != nullptr) {
                node->prev->next = next;
            }
            if (next != nullptr) {
                next->prev = node->prev;
            }
        }

        fired += 1;
        node = next;
    }

    return fired;
}

}  // namespace bsp
