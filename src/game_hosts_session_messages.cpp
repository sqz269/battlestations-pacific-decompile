// cc15-sm1: host-owned image memory for session messages. See the header.
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include <cfloat>
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
};

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

} // namespace bsp
