#pragma once

#include <array>
#include <cstdint>

#include "bsp/unit_orders.hpp"

namespace bsp {

// Descriptive hypotheses, MSVC Win32 semantic interfaces. See
// docs/UNIT_ORDER_RECORD.md for native ABI, boundaries and uncertainties.
using UnitOrderRecordStorage = std::array<std::uint8_t, kUnitOrderSlotStride>;
using UnitOrderMessageStorage = std::array<std::uint8_t, 0x3c>;

// 00815440: record* __thiscall(record*, float a, float b, byte kind), RET 0Ch.
// Writes the two parameters clamped to [-2,+2], four bound floats and kind.
// The caller supplies all prior bytes; +08..0B and +1D..1F remain untouched.
// The return is the existing compact semantic projection, not native EAX.
UnitOrderRecord build_unit_order_record_00815440(
    UnitOrderRecordStorage& storage, float param_a, float param_b,
    std::uint8_t kind) noexcept;

struct UnitOrderRecordIssueHost {
    virtual ~UnitOrderRecordIssueHost() = default;
    virtual int session_mode() = 0; // DAT_00E188A8 + 1FE4h, after publication
    // Exactly one method per unresolved native call. The constructor supplies
    // the base message's prior bytes; the issue routine patches proven fields.
    virtual UnitOrderMessageStorage construct_message_0075b430(int message_id) = 0;
    virtual void send_message_0077c2a0(const UnitOrderMessageStorage& message,
                                     std::uint32_t arg2, std::uint32_t arg3) = 0;
};

// 00816A40: void __thiscall(unit*, float a, float b, byte kind), RET 0Ch.
// Explicit scratch bytes replace the original uninitialized stack record.
// Uses the existing publish projection, including its documented slot bound.
void issue_unit_order_record_00816a40(
    UnitOrderRecordIssueHost& host, UnitOrderQueue& queue,
    UnitOrderRecordStorage& scratch, float param_a, float param_b,
    std::uint8_t kind);

// HUD fragments start AFTER input integration, clamps and ownership gates.
// All inputs below are values at those boundaries. The native CRT helper
// 00BF85B0 stays external (including its x87/error behavior). Separate methods
// retain every native call site, even two calls to the same CRT function.
struct UnitOrderHud0064b870Host {
    virtual ~UnitOrderHud0064b870Host() = default;
    virtual double quantize_turn_0064bab5(double input) = 0;
    virtual double quantize_thrust_0064baee(double input) = 0;
    virtual void issue_0064bb12(const UnitOrderRecord& arguments) = 0;
};

// 0064BA97..0064BB16: hud+24h thrust, hud+28h turn, kind=0.
void issue_hud_order_fragment_0064b870(UnitOrderHud0064b870Host& host,
                                      float thrust, float turn);

struct UnitOrderHud00651800Host {
    virtual ~UnitOrderHud00651800Host() = default;
    virtual double quantize_turn_00651a32(double input) = 0;
    virtual double quantize_thrust_00651a5c(double input) = 0;
    virtual void issue_00651aa3(const UnitOrderRecord& arguments) = 0;
};

// 00651A15..00651AA7: hud+54h thrust, hud+58h turn, kind=0.
// The enclosing 00651800 has no Ghidra function; 00651760 is NOT its entry.
void issue_hud_order_fragment_00651800(UnitOrderHud00651800Host& host,
                                      float thrust, float turn);

struct UnitOrderHud0067c4f0Host {
    virtual ~UnitOrderHud0067c4f0Host() = default;
    virtual double quantize_thrust_0067c7a7(double input) = 0;
    virtual void issue_0067c7cb(const UnitOrderRecord& arguments) = 0;
};

// 0067C772..0067C7CF: hud+1Ch thrust, negative clamped steering input,
// kind from hud+20h (the upstream input predicate normally writes 0 or 1).
void issue_hud_order_fragment_0067c4f0(UnitOrderHud0067c4f0Host& host,
                                      float thrust, float steering_input,
                                      std::uint8_t kind);

} // namespace bsp
