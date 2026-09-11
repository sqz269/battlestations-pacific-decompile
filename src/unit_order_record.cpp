#include "bsp/unit_order_record.hpp"

#include <cstring>

namespace bsp {
namespace {

// Exact values decoded from the verified image's double constants. Neither
// is the rounded source literal its printed appearance might suggest.
constexpr double kQuantizeBias = 0.4900000095367431640625; // 00CF5E58
constexpr double kTurnStep = 0.16666667163372039794921875; // 00CF5E60

template <std::size_t N, class T>
void store(std::array<std::uint8_t, N>& bytes, std::size_t offset, T value) noexcept
{
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}

float clamp_order_parameter(float value) noexcept
{
    // FCOMIP/JBE at 00815472/8A and 008154B0/DB. Unordered comparisons
    // take both JBE branches to the original MOVSS argument load.
    if (value < -2.0f) return -2.0f;
    if (value > 2.0f) return 2.0f;
    return value;
}

double thrust_quantizer_input(float value) noexcept
{
    // Native x87 evaluation spills once to float before passing a double.
    return static_cast<double>(static_cast<float>(
        static_cast<double>(value) * 4.0 + kQuantizeBias));
}

double turn_quantizer_input(float value) noexcept
{
    return static_cast<double>(static_cast<float>(
        static_cast<double>(value) / kTurnStep + kQuantizeBias));
}

float scale_thrust_result(double value) noexcept
{
    const float spilled = static_cast<float>(value);
    return static_cast<float>(static_cast<double>(spilled) * 0.25); // 00D7A348
}

float scale_turn_result(double value) noexcept
{
    const float spilled = static_cast<float>(value);
    return static_cast<float>(static_cast<double>(spilled) * kTurnStep);
}

} // namespace

UnitOrderRecord build_unit_order_record_00815440(
    UnitOrderRecordStorage& storage, float param_a, float param_b,
    std::uint8_t kind) noexcept
{
    static_assert(sizeof(float) == 4);
    store(storage, 0x14, 2.0f);  // 00815449
    store(storage, 0x0c, 2.0f);  // 0081544E
    store(storage, 0x18, -2.0f); // 0081545B
    store(storage, 0x10, -2.0f); // 00815460
    const float a = clamp_order_parameter(param_a);
    store(storage, kUnitOrderSlotParamA, a); // 0081549D
    const float b = clamp_order_parameter(param_b);
    storage[kUnitOrderSlotKind] = kind; // precedes B on every native exit
    store(storage, kUnitOrderSlotParamB, b);
    return UnitOrderRecord{a, b, kind};
}

void issue_unit_order_record_00816a40(
    UnitOrderRecordIssueHost& host, UnitOrderQueue& queue,
    UnitOrderRecordStorage& scratch, float param_a, float param_b,
    std::uint8_t kind)
{
    const auto record = build_unit_order_record_00815440(
        scratch, param_a, param_b, kind); // 00816A76
    publish_unit_order_0080dad0(queue, record); // 00816A82
    if (host.session_mode() != kUnitOrderSessionMode) return; // 00816A8D

    auto message = host.construct_message_0075b430(kUnitOrderSessionMessageId);
    store(message, 0x18, std::uint16_t{0}); // 00816AB5
    message[0x1a] = 0;                     // 00816ABA
    store(message, 0x04, std::uint32_t{0}); // 00816ABE
    // Observed native vtable identity; never dereferenced by this projection.
    store(message, 0x00, std::uint32_t{0x00d02da8}); // 00816AC2
    std::memcpy(message.data() + 0x1c, scratch.data(), scratch.size()); // REP MOVSD
    host.send_message_0077c2a0(message, 0, 0); // 00816AD9
}

void issue_hud_order_fragment_0064b870(UnitOrderHud0064b870Host& host,
                                      float thrust, float turn)
{
    const float b = scale_turn_result(
        host.quantize_turn_0064bab5(turn_quantizer_input(turn)));
    const float a = scale_thrust_result(
        host.quantize_thrust_0064baee(thrust_quantizer_input(thrust)));
    host.issue_0064bb12(UnitOrderRecord{a, b, 0});
}

void issue_hud_order_fragment_00651800(UnitOrderHud00651800Host& host,
                                      float thrust, float turn)
{
    // This caller postpones scaling B until BOTH CRT calls have returned.
    const float rounded_b = static_cast<float>(
        host.quantize_turn_00651a32(turn_quantizer_input(turn)));
    const float rounded_a = static_cast<float>(
        host.quantize_thrust_00651a5c(thrust_quantizer_input(thrust)));
    const float b = scale_turn_result(rounded_b);
    const float a = scale_thrust_result(rounded_a);
    host.issue_00651aa3(UnitOrderRecord{a, b, 0});
}

void issue_hud_order_fragment_0067c4f0(UnitOrderHud0067c4f0Host& host,
                                      float thrust, float steering_input,
                                      std::uint8_t kind)
{
    // Native SUBSS starts from negative zero (00D7A208 = 80000000h).
    // This is subtraction, not a sign-bit toggle; +0 input produces -0.
    const float b = -0.0f - steering_input;
    const float a = scale_thrust_result(
        host.quantize_thrust_0067c7a7(thrust_quantizer_input(thrust)));
    host.issue_0067c7cb(UnitOrderRecord{a, b, kind});
}

} // namespace bsp
