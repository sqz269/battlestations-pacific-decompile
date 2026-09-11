#include "bsp/unit_orders.hpp"

namespace bsp {

void publish_unit_order_0080dad0(UnitOrderQueue& queue, const UnitOrderRecord& record) noexcept
{
    // 0080DAD3: the index is reloaded from unit+97Ch before every one of the four
    // slot writes, and again for the mirror. It is not advanced here.
    const int index = queue.slot_index;
    if (index < 0 || static_cast<std::size_t>(index) >= UnitOrderQueue::kMaxSlots) {
        return; // the native code has no bound check; this projection needs one
    }
    const std::size_t i = static_cast<std::size_t>(index);

    queue.slot[i].param_a = record.param_a; // 0080DAE6
    queue.slot[i].param_b = record.param_b; // 0080DAF3
    queue.slot[i].kind = record.kind;       // 0080DB0A
    queue.slot_active[i] = false;           // 0080DB1B, stores a zero byte

    // 0080DB2E..0080DB4A: the mirror is written from the slot, in the order
    // param_b, param_a, kind.
    queue.current_param_b = queue.slot[i].param_b;
    queue.current_param_a = queue.slot[i].param_a;
    queue.current_kind = queue.slot[i].kind;
}

void issue_unit_order_00816a40(UnitOrderIssueHost& host, UnitOrderQueue& queue,
                               std::uint32_t a, std::uint32_t b, std::uint32_t c)
{
    const UnitOrderRecord record = host.build_order_record(a, b, c); // 00815440
    publish_unit_order_0080dad0(queue, record);                      // 0080DAD0

    if (host.session_mode() == kUnitOrderSessionMode) { // 00816A69
        host.send_order_message(kUnitOrderSessionMessageId, record);
    }
}

float raise_unit_load_latch_009d4fb0(float current, float request) noexcept
{
    // fcomip on (request, current) then jbe: the store runs only when the
    // request is strictly greater. An unordered compare sets CF and takes the
    // jump, so a NaN request leaves the field alone.
    if (request > current) {
        return request;
    }
    return current;
}

float latch_unit_load_to_level(float current, double gate, float level) noexcept
{
    // 009F45F8..009F4606 and the five sites like it: the field is compared
    // against the gate and, when it is below, replaced by the constant level.
    if (static_cast<double>(current) < gate) {
        return level;
    }
    return current;
}

float clamp_unit_control_override_0099d300(float value) noexcept
{
    // 0099D57C: comiss against -1.0f, then comiss against 1.0f only on the
    // taken branch. The else arm is reached for an unordered compare too.
    if (kUnitControlClampLow <= value) {
        if (kUnitControlClampHigh < value) {
            return kUnitControlClampHigh;
        }
        return value;
    }
    return kUnitControlClampLow;
}

void seed_unit_plan_slot_0099b450(UnitPlanSlot& slot, float unit_value) noexcept
{
    // 0099B456..0099B46E: one movss load feeds both stores, pending first.
    slot.pending = unit_value;
    slot.committed = unit_value;
    slot.has_pending = false;
}

float unit_plan_slot_value(const UnitPlanSlot& slot) noexcept
{
    return slot.has_pending ? slot.pending : slot.committed;
}

bool apply_unit_control_override_0099d300(UnitPlanSlot& slot, float control) noexcept
{
    // 0099D5A8: the test is an exact compare against zero, so a negative zero
    // control word is treated as absent.
    if (control == 0.0f) {
        return false;
    }
    slot.pending = clamp_unit_control_override_0099d300(control);
    slot.has_pending = true;
    return true;
}

} // namespace bsp
