// cc15-sm1: host-owned image memory for session messages. See the header.
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/camera_position_modes.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/session_message_cc13_seg42.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace bsp {

struct HostSessionMessages::Image {
    // [00E188A8], the game object 0075B430 reads at 0075B454: the local slot at
    // +18ECh over the eight player records at +18CCh. This process keeps no
    // player records, so the slot is 0 over null records and the message's
    // +14h owner is null, which is what the commands host's
    // session_player_slot already answers.
    alignas(4) std::uint8_t game[0x18f0]{};
    void* volatile current_game{game};
    NativeSessionMessageContext session{current_game};

    // The codec's image constants, read from the PE: 00D7A218 0.0f, 00D7A260
    // -1.0f, 00D7A24C 1.0f, 00CE3978 4294967296.0f, 00D7A278 the double
    // 47EFFFFFE0000000h (FLT_MAX widened), 00D7A280 0.5.
    volatile float zero_00d7a218{0.0f};
    volatile float minus_one_00d7a260{-1.0f};
    volatile float plus_one_00d7a24c{1.0f};
    volatile float uint32_bias_00ce3978{4294967296.0f};
    volatile double unscaled_00d7a278{static_cast<double>(FLT_MAX)};
    volatile double half_00d7a280{0.5};
    // 0109EEA4 is the CRT's SSE2 mode word, written by the initializer at
    // 00C27B81 from the processor's capability; bound the way GameStartupHost
    // binds it.
    const volatile std::uint32_t sse2_0109eea4{
        IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE ? 1u : 0u};
    NativeBitNumericContext numeric{zero_00d7a218, minus_one_00d7a260, plus_one_00d7a24c,
                                    uint32_bias_00ce3978, unscaled_00d7a278, half_00d7a280,
                                    sse2_0109eea4};

    // The target codec: 00F87574 a zero vector (BSS, no writer), 00D7A248
    // FLT_MAX, and the trailing threshold 00D7A24C, the same 1.0f. 00CF9360 is
    // the position scale, 22000.0f (46ABE000h).
    volatile float default_vector_00f87574[3]{0.0f, 0.0f, 0.0f};
    volatile float maximum_00d7a248{FLT_MAX};
    volatile float position_scale_00cf9360{22000.0f};
    NativeSessionMessage88Context target{&numeric, default_vector_00f87574, &maximum_00d7a248,
                                         &plus_one_00d7a24c};
    NativeSessionMessage90To93Context context{&session, target, &position_scale_00cf9360};
    NativeSessionMessage92Profile profile92{context};
    NativeSessionMessage88Profile profile88{target};

    // The 79h class's vtable 00D032D0, its four slots read from the PE:
    // 007605F0 scalar delete, 0075FFF0 write, 007600D0 read, 0075FFD0 IsType.
    // The only table these bodies store here.
    struct Seg42Vtables final : seg42::VtableBinding {
        std::uint32_t slots_00d032d0[4]{0x007605f0u, 0x0075fff0u, 0x007600d0u, 0x0075ffd0u};
        const std::uint32_t* vtable(std::uint32_t image_address) const override {
            return image_address == 0x00d032d0u ? slots_00d032d0 : nullptr;
        }
    } seg42_vtables;
    seg42::Context seg42{current_game, seg42_vtables};
    // 00954A10's pitch clamp: 00D1A640 / 00D1A638 the doubles of -1.57f and
    // 1.57f compared, 00CF0AAC / 00CF0B40 the floats stored.
    volatile double pitch_low_00d1a640{static_cast<double>(-1.57f)};
    volatile double pitch_high_00d1a638{static_cast<double>(1.57f)};
    volatile float pitch_low_00cf0aac{-1.57f};
    volatile float pitch_high_00cf0b40{1.57f};
};

namespace {
// 0076E520's stream over a cleared 400h buffer, then 00768530's type read and
// rewind (00768558 / 00768561). See queue_set_command_92 for the clearing.
struct LocalStream {
    std::uint8_t bytes[0x400]{};
    NativeBitCursor out{bytes, sizeof bytes, bytes, 0};
    NativeSessionReadStream in{nullptr, NativeBitCursor{bytes, sizeof bytes, bytes, 0}, 0, {}};
    void begin_read() {
        std::uint8_t type = 0;
        read_native_u8_bits_00428c70(&in.cursor_04, &type, 8);
        rewind_native_bits_00428b80(&in.cursor_04, 8);
    }
};
}  // namespace

HostSessionMessages::HostSessionMessages() : image_(std::make_unique<Image>()) {}
HostSessionMessages::~HostSessionMessages() = default;

NativeSessionMessage92 HostSessionMessages::queue_set_command_92(
    int command_ordinal, const SceneCommandTarget& target, std::uint8_t flag) const {
    const Image& image = *image_;
    // 0071C830 reads only the command object's ordinal at +4h (0071C893).
    const std::uint32_t command[2] = {0u, static_cast<std::uint32_t>(command_ordinal)};
    NativeSessionMessage92 sent{};
    construct_native_session_message92_value_0071c830(
        &sent, command_ordinal >= 0 ? command : nullptr, &target, flag, image.context,
        image.profile92);

    // 0076E520: the 400h-byte stream at ESP+38h. The image does not clear it;
    // the writer ORs its first byte into what is there and assigns each byte
    // after, so the host's cleared buffer is the deterministic reading.
    std::uint8_t bytes[0x400]{};
    NativeBitCursor out{bytes, sizeof bytes, bytes, 0};
    write_native_session_message92_0071c930(&sent, &out, image.context);
    // The stack message ends with 0071ECF0's frame.
    destroy_native_session_message92_0071cb40(&sent);

    // 0076E577..0076E5A4: the read stream over the same bytes, then 00768530
    // reads the 8-bit type, rewinds it (00768558 / 00768561) and dispatches.
    NativeSessionReadStream in{nullptr, NativeBitCursor{bytes, sizeof bytes, bytes, 0}, 0, {}};
    std::uint8_t type = 0;
    read_native_u8_bits_00428c70(&in.cursor_04, &type, 8);
    rewind_native_bits_00428b80(&in.cursor_04, 8);
    // Type 92 by construction: the jump table entry 0076A298 + 5Bh*4 is
    // 00769D10, operator new(3Ch) then 00764F90; 0076A27B then calls read.
    static_cast<void>(type);
    NativeSessionMessage92 queued{};
    construct_native_session_message92_00764f90(&queued, image.context, image.profile92);
    read_native_session_message92_0071ca30(&queued, &in, image.context);
    return queued;
}

void HostSessionMessages::free_set_command_92(NativeSessionMessage92& message) noexcept {
    destroy_native_session_message92_0071cb40(&message);
}

EntityOrderMessage HostSessionMessages::queue_command_88(const EntityOrderMessage& posted) const {
    const Image& image = *image_;
    // The record is 007798D0's message field for field (entity_orders.hpp), so
    // this is the same 3Ch object with the image's profile in +0h.
    NativeSessionMessage88 sent{};
    sent.header.base.profile_00 = image.profile88.slots;
    sent.header.base.delivery_04 = static_cast<std::uint32_t>(posted.kind);
    sent.header.base.field_08 = posted.reserved_08;
    sent.header.base.field_0c = posted.send_tick;
    sent.header.base.type_10 = posted.type;
    sent.header.base.selected_owner_14 =
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(posted.player_slot));
    sent.header.sender_18 = posted.sender_id;
    sent.header.relay_1a = posted.relay_flag;
    sent.header.flag_1c = posted.audit_flag;
    sent.command_20 = posted.command_ordinal;
    sent.flags_21 = posted.flags;
    std::memcpy(&sent.target_24.kind, &posted.target_kind, sizeof posted.target_kind);
    sent.target_24.object_id = posted.target_id;
    sent.target_24.object = const_cast<void*>(posted.target_object);
    std::memcpy(sent.target_24.position, posted.position, sizeof posted.position);
    std::memcpy(&sent.target_24.trailing, &posted.trailing, sizeof posted.trailing);

    LocalStream stream;
    write_native_session_message88_00764d40(&sent, &stream.out, image.target);
    destroy_native_session_message88_00764cf0(&sent);
    stream.begin_read();
    NativeSessionMessage88 queued{};
    construct_native_session_message88_00764c80(&queued, image.target, image.profile88);
    read_native_session_message88_00764e60(&queued, &stream.in, image.target);

    EntityOrderMessage result = posted;
    result.command_ordinal = queued.command_20;
    result.flags = queued.flags_21;
    std::memcpy(&result.target_kind, &queued.target_24.kind, sizeof result.target_kind);
    result.target_id = queued.target_24.object_id;
    result.target_object = queued.target_24.object;
    std::memcpy(result.position, queued.target_24.position, sizeof result.position);
    std::memcpy(&result.trailing, &queued.target_24.trailing, sizeof result.trailing);
    // The drain's delete (00764D20 -> 00764CF0) only restamps +0h, which the
    // record does not carry, so ending the copy here is not observable.
    destroy_native_session_message88_00764cf0(&queued);
    return result;
}

game::GunAimMessage79 HostSessionMessages::queue_gun_aim_79(
    const game::GunAimMessage79& posted) const {
    Image& image = *image_;
    // 0054884D: 00521370(ECX = the mover's forward row, EDX = &pitch, &yaw),
    // as cc13_sweepA reconstructs it: y clamped to [-1, 1] in double, then
    // 0042CF10 on the float; yaw = atan2(x, z) rounded to float.
    const double y = posted.forward[1];
    const double clamped = -1.0 > y ? -1.0 : (y > 1.0 ? 1.0 : y);
    float pitch = camera_asin_clamped_0042cf10(static_cast<float>(clamped));
    const float yaw = static_cast<float>(
        std::atan2(static_cast<double>(posted.forward[0]), static_cast<double>(posted.forward[2])));

    // 00954A10: 0075B430(79h), +18h/+1Ah/+4h zero, vtable 00D032D0, which are
    // 0075FF60's stores; then the arguments.
    seg42::VehicleGunControlMessage sent{};
    seg42::construct_vehicle_gun_control_0075ff60(&sent, image.seg42);
    sent.mode_1c = static_cast<std::uint32_t>(posted.group);
    for (int i = 0; i < 3; ++i) sent.vector_20[i] = posted.camera[i];
    sent.angle_2c = yaw;
    sent.flag_34 = posted.held_34 ? 1u : 0u;
    sent.flag_35 = posted.pressed_35 ? 1u : 0u;
    sent.has_target_36 = posted.has_target_36 ? 1u : 0u;
    sent.target_id_38 = posted.target_38;
    // 00954A88..00954AD9: FCOMIP against the doubles, NaN stored unchanged.
    if (image.pitch_low_00d1a640 > static_cast<double>(pitch)) {
        pitch = image.pitch_low_00cf0aac;
    } else if (static_cast<double>(pitch) > image.pitch_high_00d1a638) {
        pitch = image.pitch_high_00cf0b40;
    }
    sent.angle_30 = pitch;

    LocalStream stream;
    seg42::write_vehicle_gun_control_0075fff0(&sent, &stream.out, image.numeric);
    stream.begin_read();
    // 00769132: operator new(3Ch), 0075FF60; 0076A27B: vtable+8, 007600D0.
    seg42::VehicleGunControlMessage queued{};
    seg42::construct_vehicle_gun_control_0075ff60(&queued, image.seg42);
    seg42::read_vehicle_gun_control_007600d0(&queued, &stream.in, image.numeric);

    game::GunAimMessage79 result = posted;
    result.group = static_cast<int>(queued.mode_1c);
    for (int i = 0; i < 3; ++i) result.camera[i] = queued.vector_20[i];
    result.yaw_2c = queued.angle_2c;
    result.pitch_30 = queued.angle_30;
    result.held_34 = queued.flag_34 != 0;
    result.pressed_35 = queued.flag_35 != 0;
    result.has_target_36 = queued.has_target_36 != 0;
    result.target_38 = result.has_target_36 ? posted.target_38 : queued.target_id_38;
    result.angles_carried = true;
    return result;
}

} // namespace bsp
