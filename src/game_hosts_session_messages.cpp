// cc15-sm1: host-owned image memory for session messages. See the header.
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/wire_goal4_chain527_switches.hpp"
#include "bsp/native_plane_session_message_lifetime.hpp"
#include "bsp/wire_goal4_chain506_switches.hpp"
#include "bsp/wire_goal4_chain514_switches.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/wire_goal4_chain415_switches.hpp"
#include "bsp/wire_goal4_chain428_switches.hpp"
#include "bsp/wire_goal4_chain440_switches.hpp"
#include "bsp/wire_goal4_chain450_switches.hpp"
#include "bsp/wire_goal4_chain478_switches.hpp"
#include "bsp/wire_goal4_chain511_switches.hpp"
#include "bsp/wire_goal4_chain509_switches.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/wire_goal4_chain373_switches.hpp"
#include "bsp/cc13_sweepC.hpp"
#include "bsp/wire_goal4_chain350_switches.hpp"
#include "bsp/actions_seg5_cc13.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/wire_goal4_chain294_switches.hpp"
#include "bsp/wire_goal4_chain298_switches.hpp"
#include "bsp/wire_goal4_chain322_switches.hpp"
#include "bsp/wire_goal4_chain326_switches.hpp"
#include "bsp/wire_goal4_chain335_switches.hpp"
#include "bsp/wire_goal4_chain343_switches.hpp"
#include "bsp/session_message_dispatch.hpp"
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/cc13_seg41_land.hpp"
#include "bsp/native_bit_cursor_write.hpp"
#include "bsp/native_session_message_tags_111_112.hpp"
#include "bsp/native_session_message_tags_107_108.hpp"
#include "bsp/native_session_message_tags_103_to106.hpp"
#include "bsp/cc13_seg33b.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/camera_position_modes.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/session_message_cc13_seg42.hpp"
#include <stdexcept>
#include "bsp/wire_goal4_chain260_switches.hpp"
#include "bsp/wire_goal4_chain283_switches.hpp"
#include "bsp/gameplay_seg38_cc13.hpp"
#include "bsp/band_006b_cc13.hpp"
#include "bsp/cc13_front3.hpp"
#include "bsp/cc13_seg33a.hpp"
#include "bsp/wire_goal4_chain356_switches.hpp"
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include <type_traits>
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
    // Goal 4 chain 260: the kind 5Ah profile (vtable 00CFD9C4).
    NativeSessionMessage90Profile profile90{context};
    // Goal 4 chain 260: the 5Dh profile (vtable 00CFD9D8).
    NativeSessionMessage93Profile profile93{context};
    NativeSessionMessage88Profile profile88{target};
    // Goal 4 chain 335: the 70h class (session message 112). Its codec reads
    // the session, the numeric constants and 00D7A248 (FLT_MAX, the raw
    // 32-bit float scale) through this context.
    NativeSessionMessage111To112Context context112{&session, &numeric, &maximum_00d7a248};
    NativeSessionMessage112Profile profile112{context112};
    // Goal 4 chain 335: the scalar codec of 6Bh / 6Ch (sessions 107 / 108).
    // bsp.py ghidra bytes, bsp.gpr's /battlestationspacific.exe: 00CF8B3C
    // 43800000h (256.0f, the default +20h), 00D7A3A0 3FB99999A0000000h (0.1f
    // widened, the presence threshold), 00D7A328 4010000000000000h (4.0, the
    // write scale), 00D7A348 3FD0000000000000h (0.25, the read scale).
    volatile float default_00cf8b3c{256.0f};
    volatile double threshold_00d7a3a0{static_cast<double>(0.1f)};
    volatile double write_scale_00d7a328{4.0};
    volatile double read_scale_00d7a348{0.25};
    NativeSessionMessage99To102Context context98{&session, &numeric, &default_00cf8b3c,
                                                 &threshold_00d7a3a0, &write_scale_00d7a328,
                                                 &read_scale_00d7a348};
    NativeSessionMessage107Profile profile107{context98};
    NativeSessionMessage108Profile profile108{context98};
    // Goal 4 chain 356: 68h / 69h (sessions 104 / 105) share that scalar
    // codec (vtables 00CFDE64 / 00CFDE78: 00729050 / 00729070 delete, 006D2F20
    // write, 006D1550 read, 00728F70 / 00728FF0 IsType, 004499C0); their
    // constructors take the 103..106 context, the same scalar context plus
    // 00D7A248.
    NativeSessionMessage103To106Context context104{context98, &maximum_00d7a248};
    NativeSessionMessage104Profile profile104{context104};
    NativeSessionMessage105Profile profile105{context104};

    // Goal 4 chain 450. Message 95 (5Fh) uses the existing session owner,
    // numeric codec and binary constant 00D7A248 (FLT_MAX, raw float mode).
    NativeSessionMessage94To97Context context95{&session, &numeric, &maximum_00d7a248};
    NativeSessionMessage95Profile profile95{context95};

    // The 79h class's vtable 00D032D0, its four slots read from the PE:
    // 007605F0 scalar delete, 0075FFF0 write, 007600D0 read, 0075FFD0 IsType.
    // The only table these bodies store here.
    // Goal 4 chain 260: the 76h, 77h and 9Fh tables, five slots each read from
    // the PE (scalar delete, write, read, IsType, 004499C0 always true):
    // 00D02D30 0075CA20 0075C7C0 0075C810 0075A310; 00D02D44 0075CA40 0075C860
    // 0075C8B0 0075A360; 00D03360 00760BF0 00760AE0 00760B30 00760AB0.
    struct Seg42Vtables final : seg42::VtableBinding {
        // bsp.py ghidra bytes 00D032E8 --length 20, bsp.gpr's
        // /battlestationspacific.exe: delete, write, read, IsType, true.
        std::uint32_t slots_00d032e8[5]{0x00760610u, 0x007602f0u, 0x00760340u,
                                        0x007602c0u, 0x004499c0u};
        std::uint32_t slots_00d032d0[4]{0x007605f0u, 0x0075fff0u, 0x007600d0u, 0x0075ffd0u};
        std::uint32_t slots_00d02d30[5]{0x0075ca20u, 0x0075c7c0u, 0x0075c810u, 0x0075a310u,
                                        0x004499c0u};
        std::uint32_t slots_00d02d44[5]{0x0075ca40u, 0x0075c860u, 0x0075c8b0u, 0x0075a360u,
                                        0x004499c0u};
        std::uint32_t slots_00d03360[5]{0x00760bf0u, 0x00760ae0u, 0x00760b30u, 0x00760ab0u,
                                        0x004499c0u};
        // 90h 00D034A0 and 8Fh 00D034DC, read from the PE the same way.
        std::uint32_t slots_00d034a0[5]{0x00762660u, 0x00761ff0u, 0x00762090u, 0x00761fc0u,
                                        0x004499c0u};
        std::uint32_t slots_00d034dc[5]{0x007626c0u, 0x00762460u, 0x007624b0u, 0x00762430u,
                                        0x004499c0u};
        // 83h 00D03554.
        std::uint32_t slots_00d03554[5]{0x00763240u, 0x00763260u, 0x007632c0u, 0x00763210u,
                                        0x004499c0u};
        // C3h 00D03504: native destructor, write, read, IsType, always true.
        // bsp.py ghidra bytes 00d03504 --length 20 (existing bsp.gpr program).
        std::uint32_t slots_00d03504[5]{0x00762920u, 0x00762940u, 0x007629a0u, 0x007628f0u,
                                        0x004499c0u};
        // bsp.py ghidra bytes 00D03518 --length 20, /battlestationspacific.exe:
        // scalar delete, shared bool writer/reader, IsType, always true.
        std::uint32_t slots_00d03518[5]{0x00762b00u, 0x004645b0u, 0x004645d0u, 0x00762ad0u,
                                        0x004499c0u};
        // bsp.py ghidra bytes 00CFAA20 --length 20, bsp.gpr's
        // /battlestationspacific.exe: delete, write, read, IsType, true.
        std::uint32_t slots_00cfaa20[5]{0x006ebb70u, 0x006ebb10u, 0x006ebb40u, 0x006ebae0u,
                                        0x004499c0u};
        // Goal 4 chain 326. bsp.py ghidra bytes 00CFADBC --length 60, bsp.gpr's
        // /battlestationspacific.exe: D3h 00CFADBC and D4h 00CFADD0, each
        // scalar delete, write, read, IsType, 004499C0 always true.
        std::uint32_t slots_00cfadbc[5]{0x006f3210u, 0x006f2520u, 0x006f2550u, 0x006f2500u,
                                        0x004499c0u};
        std::uint32_t slots_00cfadd0[5]{0x006f3230u, 0x006f25e0u, 0x006f2610u, 0x006f25c0u,
                                        0x004499c0u};
        // Goal 4 chain 335. bsp.py ghidra bytes 00D034C8 --length 20 and
        // 00D0334C --length 20, bsp.gpr's /battlestationspacific.exe: 93h
        // (007626A0 scalar delete, 007622B0 write, 00762340 read, 00762280
        // IsType, 004499C0) and 9Eh (00760BD0, 00760970, 007609E0, 00760940,
        // 004499C0).
        std::uint32_t slots_00d034c8[5]{0x007626a0u, 0x007622b0u, 0x00762340u, 0x00762280u,
                                        0x004499c0u};
        std::uint32_t slots_00d0334c[5]{0x00760bd0u, 0x00760970u, 0x007609e0u, 0x00760940u,
                                        0x004499c0u};
        // Goal 4 chain 343. bsp.py ghidra bytes 00D033C4 --length 40 and
        // 00D033EC --length 40, bsp.gpr's /battlestationspacific.exe: 98h
        // 00D033C4 (007625C0 scalar delete, 004645B0 write, 004645D0 read,
        // 00761370 IsType, 004499C0), 9Ah 00D033D8 (007625E0, 004645B0,
        // 004645D0, 00761410, 004499C0) and 99h 00D033EC (00762600, 007614F0,
        // 00761540, 007614B0, 004499C0).
        std::uint32_t slots_00d033c4[5]{0x007625c0u, 0x004645b0u, 0x004645d0u, 0x00761370u,
                                        0x004499c0u};
        std::uint32_t slots_00d033d8[5]{0x007625e0u, 0x004645b0u, 0x004645d0u, 0x00761410u,
                                        0x004499c0u};
        std::uint32_t slots_00d033ec[5]{0x00762600u, 0x007614f0u, 0x00761540u, 0x007614b0u,
                                        0x004499c0u};
        // 92h 00D00518 (bsp.py ghidra bytes 00D00518 --length 20): 0074EDA0
        // scalar delete, 0074EB20 write, 0074EB80 read, 0074EB00 IsType,
        // 004499C0. 00762130 (seg42) and 0074EAB0 (seg41) both store it.
        std::uint32_t slots_00d00518[5]{0x0074eda0u, 0x0074eb20u, 0x0074eb80u, 0x0074eb00u,
                                        0x004499c0u};
        // 7Ah 00D03310 (bsp.py ghidra bytes 00D03310 --length 20): 00760650
        // scalar delete, 00760550 write, 007605A0 read, 00760520 IsType,
        // 004499C0.
        std::uint32_t slots_00d03310[5]{0x00760650u, 0x00760550u, 0x007605a0u, 0x00760520u,
                                        0x004499c0u};
        // Goal 4 chain 356. C7h 00D0352C (bsp.py ghidra bytes 00D0352C --length
        // 24, bsp.gpr's /battlestationspacific.exe): 00762BC0 scalar delete,
        // 00762BE0 write, 00762C30 read, 00762B90 IsType, 004499C0.
        std::uint32_t slots_00d0352c[5]{0x00762bc0u, 0x00762be0u, 0x00762c30u, 0x00762b90u,
                                        0x004499c0u};
        const std::uint32_t* vtable(std::uint32_t image_address) const override {
            // Only queue_plane_disappear_c7 constructs a C7h message.
            if (image_address == 0x00d0352cu) return slots_00d0352c;
            if constexpr (kWireGoal4Chain343Bound) {
                if (image_address == 0x00d033c4u) return slots_00d033c4;
                if (image_address == 0x00d033d8u) return slots_00d033d8;
                if (image_address == 0x00d033ecu) return slots_00d033ec;
                if (image_address == 0x00d00518u) return slots_00d00518;
                if (image_address == 0x00d03310u) return slots_00d03310;
            } else {
                // The previous binding had no 98h / 9Ah / 99h profile.
            }
            if constexpr (kWireGoal4Chain335Bound) {
                if (image_address == 0x00d034c8u) return slots_00d034c8;
                if (image_address == 0x00d0334cu) return slots_00d0334c;
            } else {
                // The previous binding had no 93h / 9Eh profile.
            }
            if constexpr (kWireGoal4Chain326Bound) {
                if (image_address == 0x00cfadbcu) return slots_00cfadbc;
                if (image_address == 0x00cfadd0u) return slots_00cfadd0;
            } else {
                // The previous binding had no D3h / D4h profile.
            }
            if constexpr (kWireGoal4Chain322Bound) {
                if (image_address == 0x00cfaa20u) return slots_00cfaa20;
            } else {
                // The previous binding had no C6h profile.
            }
            if constexpr (kWireGoal4Chain298Bound) {
                if (image_address == 0x00d032e8u) return slots_00d032e8;
            } else {
                // The previous binding had no 7Fh profile.
            }
            if (image_address == 0x00d032d0u) return slots_00d032d0;
            if (image_address == 0x00d03518u) {
                if constexpr (kWireGoal4Chain294Bound) {
                    return slots_00d03518;
                } else {
                    return nullptr;
                }
            }
            if constexpr (kWireGoal4Chain283Bound) {
                if (image_address == 0x00d03504u) return slots_00d03504;
            }
            if constexpr (kWireGoal4Chain260Bound) {
                if (image_address == 0x00d02d30u) return slots_00d02d30;
                if (image_address == 0x00d02d44u) return slots_00d02d44;
                if (image_address == 0x00d03360u) return slots_00d03360;
                if (image_address == 0x00d034a0u) return slots_00d034a0;
                if (image_address == 0x00d034dcu) return slots_00d034dc;
                if (image_address == 0x00d03554u) return slots_00d03554;
            }
            return nullptr;
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
// 00768530 allocates each decoded message (58h: 3Ch, 5Ah/5Dh: 28h,
// 6Bh/6Ch: 24h),
// unlike the sender's stack object. Retain that ownership until this bridge
// projects the decoded payload to its existing value consumer. bsp.py show
// 00764D20 / 0071E370 / 0071E390: root-profile stamp, then 00BF65AC iff
// flags bit 0. singleton_lifetime_allocate uses the matching malloc/free pair.
// The RAII owner also releases a partially decoded message on exceptions;
// these message types have no owned payload subobjects.
template<class Message, Message* (*Delete)(Message*, std::uint32_t)>
class OwnedDecodedMessage {
public:
    OwnedDecodedMessage()
        : message_(::new (singleton_lifetime_allocate({SingletonAllocationKind::object,
              sizeof(Message), sizeof(Message)})) Message{}) {
        static_assert(std::is_trivially_destructible_v<Message>);
    }
    ~OwnedDecodedMessage() { Delete(message_, 1u); }
    OwnedDecodedMessage(const OwnedDecodedMessage&) = delete;
    OwnedDecodedMessage& operator=(const OwnedDecodedMessage&) = delete;
    Message& get() noexcept { return *message_; }
private:
    Message* message_;
};

// The real session owner and selected player record remain those of Image.
// bsp.py show 0080FA50 / 0080FF30 / 0080FF80 / 0080FFD0: each body calls
// 0075B430 once, then stores only the header, its native vptr and its payload.
class UnitMessageConstructionBinding final : public cc13_sweepC::UnitMessageConstructionHost {
public:
    explicit UnitMessageConstructionBinding(const NativeSessionMessageContext& session)
        : session_(session) {}
    void session_message_construct_base_0075b430(void* message, std::uint32_t kind) override {
        construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), kind, session_);
    }
private:
    const NativeSessionMessageContext& session_;
};

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

// Only the four boundaries the existing shared codec actually invokes.
// Object/stream retain their native offsets; these adapters neither allocate
// unrelated scene objects nor replace any bitstream arithmetic.
struct SessionBoolCodec final : cc13_seg5::SessionBoolMessageHost {
    using Object = cc13_seg5::Object;
    void session_message_write_extended_header_0075b480(Object message, Object stream) override {
        write_native_session_message_header_0075b480(message,
            reinterpret_cast<NativeBitCursor*>(stream));
    }
    void session_message_read_extended_header_0075b4c0(Object message, Object stream) override {
        read_native_session_message_header_0075b4c0(message,
            reinterpret_cast<NativeSessionReadStream*>(stream));
    }
    void bit_cursor_write_boolean_004290b0(Object cursor, std::uint8_t value) override {
        write_native_bool_bit_004290b0(reinterpret_cast<NativeBitCursor*>(cursor), value);
    }
    void bit_cursor_read_boolean_00428d70(Object cursor, Object out_byte) override {
        read_native_bool_bit_00428d70(reinterpret_cast<NativeBitCursor*>(cursor),
            reinterpret_cast<bool*>(out_byte));
    }
};

// Goal 4 chain 326: the ten boundaries the command-building codecs
// (006F2520..006F2700) call, bound to the process's bit-cursor bodies. Write
// calls receive the 10h cursor itself; read calls receive the 18h stream for
// the header and stream+4 (its cursor) for the fields, as the codecs pass them.
struct CommandBuildingCodec final : cc13_seg33b::CommandBuildingMessageCodecHost {
    using Object = cc13_seg33b::Object;
    explicit CommandBuildingCodec(const NativeBitNumericContext& numeric) : numeric_(numeric) {}
    void session_message_write_header_0075b480(Object message, Object stream) override {
        write_native_session_message_header_0075b480(message,
            reinterpret_cast<NativeBitCursor*>(stream));
    }
    void session_message_read_header_0075b4c0(Object message, Object stream) override {
        read_native_session_message_header_0075b4c0(message,
            reinterpret_cast<NativeSessionReadStream*>(stream));
    }
    void bit_write_unsigned_00429070(Object cursor, std::uint32_t value, std::int32_t bits) override {
        write_native_unsigned_dword_bits_00429070(reinterpret_cast<NativeBitCursor*>(cursor),
            value, static_cast<std::uint32_t>(bits));
    }
    void bit_read_unsigned_00428d10(Object cursor, std::uint32_t* value, std::int32_t bits) override {
        read_native_u32_bits_00428d10(reinterpret_cast<NativeBitCursor*>(cursor), value,
            static_cast<std::uint32_t>(bits));
    }
    void bit_write_signed_00429090(Object cursor, std::int32_t value, std::int32_t bits) override {
        write_native_signed_dword_bits_00429090(reinterpret_cast<NativeBitCursor*>(cursor),
            static_cast<std::uint32_t>(value), static_cast<std::uint32_t>(bits));
    }
    void bit_read_signed_00428d30(Object cursor, std::int32_t* value, std::int32_t bits) override {
        read_native_i32_bits_00428d30(reinterpret_cast<NativeBitCursor*>(cursor), value,
            static_cast<std::uint32_t>(bits));
    }
    void bit_write_boolean_004290b0(Object cursor, std::uint32_t value) override {
        write_native_bool_bit_004290b0(reinterpret_cast<NativeBitCursor*>(cursor), value);
    }
    void bit_read_boolean_00428d70(Object cursor, std::uint8_t* value) override {
        read_native_bool_bit_00428d70(reinterpret_cast<NativeBitCursor*>(cursor),
            reinterpret_cast<bool*>(value));
    }
    // D5h's numeric codec (004295C0 / 004293F0) on raw float bits, as the
    // shared bodies take them. D3h / D4h never reach these two.
    void bit_write_float_004295c0(Object cursor, float value, std::int32_t zero,
                                 std::int32_t is_signed, float scale, std::int32_t bits) override {
        std::uint32_t value_bits = 0;
        std::uint32_t scale_bits = 0;
        std::memcpy(&value_bits, &value, sizeof value_bits);
        std::memcpy(&scale_bits, &scale, sizeof scale_bits);
        write_native_numeric_float_004295c0(reinterpret_cast<NativeBitCursor*>(cursor), value_bits,
            static_cast<std::uint32_t>(zero), static_cast<std::uint32_t>(is_signed), scale_bits,
            static_cast<std::uint32_t>(bits), numeric_);
    }
    void bit_read_float_004293f0(Object cursor, float* value, std::int32_t zero,
                                std::int32_t is_signed, float scale, std::int32_t bits) override {
        std::uint32_t value_bits = 0;
        std::uint32_t scale_bits = 0;
        std::memcpy(&scale_bits, &scale, sizeof scale_bits);
        read_native_numeric_float_004293f0(reinterpret_cast<NativeBitCursor*>(cursor), &value_bits,
            static_cast<std::uint32_t>(zero), static_cast<std::uint32_t>(is_signed), scale_bits,
            static_cast<std::uint32_t>(bits), numeric_);
        std::memcpy(value, &value_bits, sizeof value_bits);
    }

private:
    const NativeBitNumericContext& numeric_;
};

// D3h / D4h: 00768530 allocates 24h bytes for either (00769F45 / 00769F65):
// the 1Ch extended header, +1Ch, and +20h (D3h's codec moves only its byte).
struct CommandBuildingMessage24 {
    seg42::HeaderedMessage h;
    std::uint32_t value_1c;
    std::uint32_t value_20;
};
static_assert(sizeof(CommandBuildingMessage24) == 0x24);
static_assert(offsetof(CommandBuildingMessage24, value_20) == 0x20);

cc13_seg33b::Object command_building_object(CommandBuildingMessage24& message) noexcept {
    return reinterpret_cast<cc13_seg33b::Object>(&message);
}
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

std::shared_ptr<NativeSessionMessage92> HostSessionMessages::queue_owned_set_command_92(
    int command_ordinal, const SceneCommandTarget& target, std::uint8_t flag) const {
    const Image& image = *image_;
    const std::uint32_t command[2] = {0u, static_cast<std::uint32_t>(command_ordinal)};
    NativeSessionMessage92 sent{};
    construct_native_session_message92_value_0071c830(
        &sent, command_ordinal >= 0 ? command : nullptr, &target, flag, image.context,
        image.profile92);
    std::uint8_t bytes[0x400]{};
    NativeBitCursor out{bytes, sizeof bytes, bytes, 0};
    write_native_session_message92_0071c930(&sent, &out, image.context);
    destroy_native_session_message92_0071cb40(&sent);

    NativeSessionReadStream in{nullptr, NativeBitCursor{bytes, sizeof bytes, bytes, 0}, 0, {}};
    std::uint8_t type = 0;
    read_native_u8_bits_00428c70(&in.cursor_04, &type, 8);
    rewind_native_bits_00428b80(&in.cursor_04, 8);
    static_cast<void>(type);
    // 00768530 case 5Ch (00769D10): allocate 3Ch, construct 00764F90, then
    // 0076A27B invokes 0071CA30 on that same allocation. Keep the live decoded
    // object in the command queue, as the existing 5Fh owner does. Evidence:
    // bsp.py show 0071E3B0: root-profile stamp, 00BF65AC iff flags & 1, RET 4.
    // This allocator is the matching malloc/free pair. The body owns no
    // payload subobjects; its target descriptor borrows the game's entity.
    std::shared_ptr<NativeSessionMessage92> queued(
        ::new (singleton_lifetime_allocate({SingletonAllocationKind::object,
            sizeof(NativeSessionMessage92), sizeof(NativeSessionMessage92)})) NativeSessionMessage92{},
        [](NativeSessionMessage92* message) { delete_native_session_message92_0071e3b0(message, 1u); });
    construct_native_session_message92_00764f90(queued.get(), image.context, image.profile92);
    read_native_session_message92_0071ca30(queued.get(), &in, image.context);
    return queued;
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
    if constexpr (kWireGoal4Chain428Bound) {
        OwnedDecodedMessage<NativeSessionMessage88, delete_native_session_message88_00764d20> owner;
        NativeSessionMessage88& queued = owner.get();
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
        return result; // owner runs 00764D20(1) once, after extracting the payload.
    } else {
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
}

std::shared_ptr<NativeSessionMessage95> HostSessionMessages::queue_user_path_point_5f(
    const float point[3]) const {
    Image& image = *image_;
    NativeSessionMessage95 sent{};
    // Evidence: bsp.py show 0071D340 --asm: MOVSS copies at 0071D36A..3A4,
    // presence 1, then route flags 7. Use that existing sender body once;
    // 0071CC20's FLD/FSTP copies would change signaling-NaN/FP-status behavior.
    const auto posted = gameplay_seg38_cc13::add_user_path_point_message_0071d340(
        {point[0], point[1], point[2]});
    // The native header fields the semantic sender leaves to its transport:
    // 0071D361 base construct, 0071D38B..39C extended header and profile.
    construct_native_session_message_0075b430(&sent.header.base, posted.type, image.session);
    sent.header.sender_18 = 0;
    sent.header.relay_1a = 0;
    sent.header.flag_1c = 0;
    sent.header.base.delivery_04 = 1;
    sent.header.base.profile_00 = image.profile95.slots;
    const float payload[3]{posted.point.x, posted.point.y, posted.point.z};
    std::memcpy(sent.vector_20, payload, sizeof payload);
    sent.present_2c = static_cast<std::uint8_t>(posted.add_2c);
    LocalStream stream;
    write_native_session_message95_0071cca0(&sent, &stream.out, image.context95);
    destroy_native_session_message95_0071cdc0(&sent);
    stream.begin_read();

    // 0076E520's local route decodes a separate object; 00768530 case 5Fh
    // uses 0075B100. Keep that object in the host's real loopback entry,
    // rather than copying the posted XYZ straight to the receiver. The
    // matching scalar delete stamps the root profile and frees iff bit 0.
    std::shared_ptr<NativeSessionMessage95> queued(
        ::new (singleton_lifetime_allocate({SingletonAllocationKind::object,
            sizeof(NativeSessionMessage95), sizeof(NativeSessionMessage95)})) NativeSessionMessage95{},
        [](NativeSessionMessage95* message) { delete_native_session_message95_0071e3f0(message, 1u); });
    construct_native_session_message95_0075b100(queued.get(), image.context95, image.profile95);
    read_native_session_message95_0071cd30(queued.get(), &stream.in, image.context95);
    return queued;
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

std::uint16_t HostSessionMessages::queue_formation_member_76_77(std::uint8_t type,
                                                               std::uint16_t member_id) const {
    const Image& image = *image_;
    const bool join = type == 0x76;
    // 0077C920..0077C951 (join) / 0077C9F5..0077CA2F (leave): 0075B430(type)
    // on the stack message, then +18h word 0, +1Ah byte 0, +4h 1, +1Ch byte 0,
    // the class vtable, and +20h the other entity's +174h word.
    seg42::FormationMemberMessage sent{};
    construct_native_session_message_0075b430(&sent.u.h.base, type, image.session);
    sent.u.h.id_18 = 0;
    sent.u.h.relay_1a = 0;
    sent.u.h.base.delivery_04 = 1;
    sent.u.flag_1c = 0;
    sent.u.h.base.profile_00 = image.seg42_vtables.vtable(join ? 0x00d02d30u : 0x00d02d44u);
    sent.member_id_20 = member_id;

    // 0076E575: vtable[4], the class writer: type byte, 12-bit id, relay and
    // +1Ch bits (0075B480 + 004290B0), then +20h in 12 bits.
    LocalStream stream;
    if (join) {
        seg42::write_formation_join_0075c7c0(&sent, &stream.out);
    } else {
        seg42::write_formation_leave_0075c860(&sent, &stream.out);
    }
    // 0076E5A4 -> 00768530: the type read and rewind, then operator new(24h)
    // and the case's ConstructForRead (007690E6 / 00769106), then 0076A27B
    // vtable[8], the class reader.
    stream.begin_read();
    seg42::FormationMemberMessage queued{};
    if (join) {
        seg42::construct_for_read_formation_join_0075a2e0(&queued, image.seg42);
        seg42::read_formation_join_0075c810(&queued, &stream.in);
    } else {
        seg42::construct_for_read_formation_leave_0075a330(&queued, image.seg42);
        seg42::read_formation_leave_0075c8b0(&queued, &stream.in);
    }
    // 0076E5B2 then stores the sender's +174h at the copy's +18h; the hosts
    // deliver to the posting unit directly, so that store is not repeated.
    // The drain's scalar delete (0075CA20 / 0075CA40) has no owned storage.
    return queued.member_id_20;
}

std::uint32_t HostSessionMessages::queue_vehicle_party_7f(std::uint32_t party) const {
    const Image& image = *image_;
    // bsp.py show 008a8930: 008A8A8D..008A8AC4 builds 7Fh with the
    // requested party at +1Ch, zero id/relay, delivery 1 and vtable 00D032E8.
    // 00760260 supplies that same header; the body is reused for the factory
    // copy as at 007691C6, not run twice on the sender.
    seg42::VehicleValueMessage sent{};
    seg42::construct_vehicle_set_party_00760260(&sent, image.seg42);
    sent.value_1c = party;
    LocalStream stream;
    seg42::write_vehicle_set_party_007602f0(&sent, &stream.out);
    stream.begin_read();
    seg42::VehicleValueMessage queued{};
    seg42::construct_vehicle_set_party_00760260(&queued, image.seg42);
    seg42::read_vehicle_set_party_00760340(&queued, &stream.in);
    // Both message lifetimes own no heap members. The native scalar delete
    // only restamps the vtable before freeing its allocation.
    return queued.value_1c;
}

namespace {
template <class IsType>
bool copy_goes_to_unit_handler(IsType is_type);
}

std::uint32_t HostSessionMessages::queue_vehicle_race_80(
    const VehicleRaceFields& posted) const {
    // Review round 1: selected-player ownership and the 00D032FC profile are
    // not bound, so do not run or claim the native constructor. These plain
    // records provide storage only for the codec's fields, taken from the
    // actual descriptor produced by 008A8720 (008A8879..008A88B7).
    // bsp.py show 00760420/00760470: only type +10h, id +18h, relay +1Ah and
    // race +1Ch are read/written. Profile +0h and owner +14h are never used.
    seg42::VehicleValueMessage sent{};
    sent.h.base.type_10 = posted.type_10;
    sent.h.id_18 = posted.id_18;
    sent.h.relay_1a = posted.relay_1a;
    sent.value_1c = posted.race_1c;
    LocalStream stream;
    seg42::write_vehicle_set_race_00760420(&sent, &stream.out);
    stream.begin_read();
    seg42::VehicleValueMessage queued{};
    seg42::read_vehicle_set_race_00760470(&queued, &stream.in);
    // 00780120's real IsType ladder must select the session kind switch,
    // whose 80h arm consumes this payload (0095ABE0 / unit_message_arms.cpp).
    if (!copy_goes_to_unit_handler([](std::uint32_t type) {
            return seg42::is_vehicle_set_race_007603f0(type);
        })
        || queued.h.base.type_10 != 0x80)
        throw std::logic_error("SetRace: decoded copy did not reach the unit race arm");
    // Codec storage owns no heap members and claims no native object lifetime.
    return queued.value_1c;
}

bool HostSessionMessages::queue_repair_body_9f(bool enabled) const {
    const Image& image = *image_;
    // 008AD48B..008AD4B7: 0075B430(9Fh), +18h = EBP (0), +4h = 1, +1Ah = 0,
    // vtable 00D03360, +1Ch = the Lua flag byte. 00760A50 makes the same
    // stores (0075B430, the zero header, delivery 1, vtable 00D03360).
    seg42::RepairBodyMessage sent{};
    seg42::construct_shiprepair_bodyrepair_00760a50(&sent, image.seg42);
    sent.flag_1c = enabled ? 1u : 0u;

    LocalStream stream;
    seg42::write_shiprepair_bodyrepair_00760ae0(&sent, &stream.out);
    // 00768530 case 9Fh: operator new(20h), 00760A50 at 0076960E; 0076A27B
    // vtable[8] = 00760B30.
    stream.begin_read();
    seg42::RepairBodyMessage queued{};
    seg42::construct_shiprepair_bodyrepair_00760a50(&queued, image.seg42);
    seg42::read_shiprepair_bodyrepair_00760b30(&queued, &stream.in);
    return queued.flag_1c != 0;
}

HostSessionMessages::ShipLeakCopy HostSessionMessages::queue_ship_leak_90(
    std::uint32_t count, const float (&point)[3]) const {
    const Image& image = *image_;
    // 0080FF80: 0075B430(90h), +18h word 0, +1Ah 0, +1Ch = value, +4h = 1,
    // vtable 00D034A0, +20h..+28h the point; 00761F60 is those stores
    // without the two arguments.
    seg42::ShipLeakMessage sent{};
    if constexpr (kWireGoal4Chain350Bound) {
        UnitMessageConstructionBinding binding(image.session);
        cc13_sweepC::unit_message_hull_impact_construct_0080ff80(binding, &sent, count, point);
        // Relocate only the body's original vptr 00D034A0 to the existing host profile.
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d034a0u);
    } else {
    seg42::construct_ship_leak_00761f60(&sent, image.seg42);
    sent.leak_1c = count;
    for (int i = 0; i < 3; ++i) sent.vector_20[i] = point[i];
    }

    LocalStream stream;
    seg42::write_ship_leak_00761ff0(&sent, &stream.out, image.numeric);
    // 00768530 case 90h: operator new(2Ch), 00761F60 at 0076946E; 0076A27B
    // vtable[8] = 00762090.
    stream.begin_read();
    seg42::ShipLeakMessage queued{};
    seg42::construct_ship_leak_00761f60(&queued, image.seg42);
    seg42::read_ship_leak_00762090(&queued, &stream.in, image.numeric);
    ShipLeakCopy copy;
    copy.count = queued.leak_1c;
    for (int i = 0; i < 3; ++i) copy.point[i] = queued.vector_20[i];
    return copy;
}

HostSessionMessages::ShipAvoidSideCopy HostSessionMessages::queue_ship_avoid_side_8f(
    std::uint16_t other_id, std::int32_t side) const {
    const Image& image = *image_;
    // 009D66B0..009D66E6: 0075B430(8Fh), +18h word 0, +1Ah 0, +4h = 1, vtable
    // 00D034DC (007623D0's stores), +1Ch = observed+174h, +20h = side.
    seg42::ShipAvoidSideMessage sent{};
    seg42::construct_ship_avoidside_007623d0(&sent, image.seg42);
    sent.other_id_1c = other_id;
    sent.side_20 = side;

    LocalStream stream;
    seg42::write_ship_avoidside_00762460(&sent, &stream.out);
    // 00768530 case 8Fh: operator new(24h), 007623D0 at 0076944E; 0076A27B
    // vtable[8] = 007624B0 (00428D30 sign-extends the 3-bit side).
    stream.begin_read();
    seg42::ShipAvoidSideMessage queued{};
    seg42::construct_ship_avoidside_007623d0(&queued, image.seg42);
    seg42::read_ship_avoidside_007624b0(&queued, &stream.in);
    return ShipAvoidSideCopy{queued.other_id_1c, queued.side_20};
}

HostSessionMessages::AirbaseLaunchSlotCopy HostSessionMessages::queue_airbase_launch_slot_83(
    std::uint32_t slot, bool now) const {
    const Image& image = *image_;
    // 00896915..0089694C: 0075B430(83h), +18h word 0, +1Ah 0, +4h = 1, +1Ch 0,
    // vtable 00D03554 (007631A0's stores), +20h = EDI (slot - 1), +24h = CL.
    seg42::AirbaseLaunchSlotMessage sent{};
    seg42::construct_airbase_launchslot_007631a0(&sent, image.seg42);
    sent.slot_20 = slot;
    sent.flag_24 = now ? 1u : 0u;

    LocalStream stream;
    seg42::write_airbase_launchslot_00763260(&sent, &stream.out);
    // 00768530 case 83h: operator new(28h), 007631A0 at 00769246; 0076A27B
    // vtable[8] = 007632C0.
    stream.begin_read();
    seg42::AirbaseLaunchSlotMessage queued{};
    seg42::construct_airbase_launchslot_007631a0(&queued, image.seg42);
    seg42::read_airbase_launchslot_007632c0(&queued, &stream.in);
    return AirbaseLaunchSlotCopy{queued.slot_20, queued.flag_24 != 0};
}

HostSessionMessages::PlaneChangeStateCopy HostSessionMessages::queue_plane_change_state_c3(
    std::uint8_t current, std::uint8_t requested) const {
    const Image& image = *image_;
    // Evidence: bsp.py show 007C17D0 and 00762880. The sender inlines the
    // constructor's header stores; its two payload bytes come from the caller.
    seg42::PlaneChangeStateMessage sent{};
    seg42::construct_plane_change_state_00762880(&sent, image.seg42);
    sent.state_20 = current;
    sent.state_21 = requested;

    LocalStream stream;
    seg42::write_plane_change_state_00762940(&sent, &stream.out);
    // 00768530 case C3h allocates 24h bytes, calls 00762880, then dispatches
    // vtable[8] at 0076A27B to 007629A0. These message lifetimes own no
    // references; the host keeps the two bytes read by 007CCFA0's C3h arm.
    stream.begin_read();
    if constexpr (kWireGoal4Chain506Bound) {
        // 00769B70..00769B84 allocates 24h, then runs the same constructor.
        // Keep that real decoded object until its payload has been copied;
        // 00762920(1) then frees it once. The stack sender takes flags=0.
        OwnedDecodedMessage<seg42::PlaneChangeStateMessage,
            delete_plane_change_state_message_00762920> owner;
        auto& queued = owner.get();
        seg42::construct_plane_change_state_00762880(&queued, image.seg42);
        seg42::read_plane_change_state_007629a0(&queued, &stream.in);
        const PlaneChangeStateCopy copy{queued.state_20, queued.state_21};
        delete_plane_change_state_message_00762920(&sent, 0u);
        return copy;
    } else {
    seg42::PlaneChangeStateMessage queued{};
    seg42::construct_plane_change_state_00762880(&queued, image.seg42);
    seg42::read_plane_change_state_007629a0(&queued, &stream.in);
    return PlaneChangeStateCopy{queued.state_20, queued.state_21};
    }
}

std::uint8_t HostSessionMessages::queue_plane_touchdown_c5() const {
    const Image& image = *image_;
    static_assert(sizeof(seg42::UnitMessage) == 0x20);
    // bsp.py show 00762A60: type C5h, +18h/+1Ah/+1Ch zero, delivery 1,
    // and the selected owner from the existing live session context.
    seg42::UnitMessage sent{};
    seg42::construct_plane_touchdown_00762a60(&sent, image.seg42);
    SessionBoolCodec codec;
    LocalStream stream;
    cc13_seg5::session_bool_message_write_004645b0(codec,
        reinterpret_cast<cc13_seg5::Object>(&sent),
        reinterpret_cast<cc13_seg5::Object>(&stream.out));
    stream.begin_read();
    // 00768530's C5h factory allocates 20h at 00769BD0 and calls 00762A60 at
    // 00769BE4 (bsp.py show 00768530 --asm --start 1576 --lines 19).
    // The reader reloads
    // the serialized type, sender and flag. The direct delivery retains the
    // posting plane's registry identity, as the other local message routes do.
    if constexpr (kWireGoal4Chain506Bound) {
        OwnedDecodedMessage<seg42::UnitMessage,
            delete_plane_touchdown_message_00762b00> owner;
        auto& queued = owner.get();
        seg42::construct_plane_touchdown_00762a60(&queued, image.seg42);
        cc13_seg5::session_bool_message_read_004645d0(codec,
            reinterpret_cast<cc13_seg5::Object>(&queued),
            reinterpret_cast<cc13_seg5::Object>(&stream.in));
        const auto type = queued.h.base.type_10;
        delete_plane_touchdown_message_00762b00(&sent, 0u);
        return type; // 00762B00(1) releases the factory's 20h allocation.
    } else {
    seg42::UnitMessage queued{};
    seg42::construct_plane_touchdown_00762a60(&queued, image.seg42);
    cc13_seg5::session_bool_message_read_004645d0(codec,
        reinterpret_cast<cc13_seg5::Object>(&queued),
        reinterpret_cast<cc13_seg5::Object>(&stream.in));
    return queued.h.base.type_10;
    }
}

std::uint8_t HostSessionMessages::make_plane_takeoff_c6() const {
    // 00762A00..00762A58 (ECX = destination, RET): the native stores build
    // the 20h-byte header, including type C6h, delivery 1, and the selected
    // session owner. Neither this object nor the constructor supplies +20h.
    // Keep the established immediate-delivery path until that flag's producer
    // is recovered, rather than invoking 006EBB10 on an invented tail byte.
    seg42::UnitMessage sent{};
    return seg42::construct_plane_takeoff_00762a00(&sent, image_->seg42)->h.base.type_10;
}

HostSessionMessages::DirectorSubKindCopy HostSessionMessages::queue_director_sub_kind_5a(
    std::uint32_t sub_kind, std::uint32_t value) const {
    const Image& image = *image_;
    // 0071DD30's message: type 5Ah, +20h the sub-kind, +24h the value.
    const gameplay_seg38_cc13::RoutedDirectorMessage posted =
        gameplay_seg38_cc13::director_sub_kind_message(sub_kind, value);
    // 0071DD4B..0071DD81: 0075B430 on the stack message, then +18h word 0,
    // +1Ah 0, +4h = 1, +1Ch 0, the vtable 00CFD9C4 (profile90), +20h, +24h.
    NativeSessionMessage90 sent{};
    construct_native_session_message_0075b430(&sent.header.base, posted.type, image.session);
    sent.header.sender_18 = 0;
    sent.header.relay_1a = 0;
    sent.header.base.delivery_04 = 1;
    sent.header.flag_1c = 0;
    sent.header.base.profile_00 = image.profile90.slots;
    sent.selector_20 = posted.field_20;
    sent.value_24 = posted.field_24;

    LocalStream stream;
    write_native_session_message90_0071c670(&sent, &stream.out, image.context);
    // 00768530 case 5Ah: operator new(28h), 0075AF60 at 00769CE4; 0076A27B
    // vtable[8] = 0071C6C0. 0071E370 frees the decoded allocation at this
    // bridge's value-consumer boundary; the OFF arm retains its stack copy.
    stream.begin_read();
    if constexpr (kWireGoal4Chain428Bound) {
        OwnedDecodedMessage<NativeSessionMessage90, delete_native_session_message90_0071e370> owner;
        NativeSessionMessage90& queued = owner.get();
        construct_native_session_message90_0075af60(&queued, image.context, image.profile90);
        read_native_session_message90_0071c6c0(&queued, &stream.in, image.context);
        const DirectorSubKindCopy copy{queued.selector_20, queued.value_24};
        destroy_native_session_message90_0071c720(&sent);
        return copy; // owner runs 0071E370(1); the sender stays a stack object.
    } else {
    NativeSessionMessage90 queued{};
    construct_native_session_message90_0075af60(&queued, image.context, image.profile90);
    read_native_session_message90_0071c6c0(&queued, &stream.in, image.context);
    if constexpr (kWireGoal4Chain373Bound) {
        const DirectorSubKindCopy copy{queued.selector_20, queued.value_24};
        // bsp.py show 0071C720: ECX=this, stamp CE4974, RET. The sender's
        // cleanup funclet C85180 belongs to 0071DD30 (bsp.py callers 0071C720).
        // Finish both real loopback objects after their payload was consumed.
        // These host objects use stack storage, so use the non-deleting body.
        destroy_native_session_message90_0071c720(&queued);
        destroy_native_session_message90_0071c720(&sent);
        return copy;
    } else {
        return DirectorSubKindCopy{queued.selector_20, queued.value_24};
    }
    }
}

HostSessionMessages::ClearCommandCopy HostSessionMessages::queue_clear_command_5d(
    std::uint8_t arm, std::int32_t index) const {
    const Image& image = *image_;
    // 0071C730..0071C75E: 0075B430(5Dh), +18h word 0, +1Ah 0, +1Ch 0, +20h =
    // the arm byte, +4h = 1, vtable 00CFD9D8 (profile93), +24h = the index.
    NativeSessionMessage93 sent{};
    if constexpr (kWireGoal4Chain527Bound) {
        // bsp.py show 0071C730: the existing semantic body produces the 5Dh
        // type/arm/index; this transport supplies its recovered base/header
        // stores. Serialize that result, rather than repeating its payload
        // construction. Copy the DWORD bits so index -1 retains FFFFFFFFh.
        const auto posted = gameplay_seg38_cc13::clearcmd_construct_0071c730(
            arm, static_cast<std::uint32_t>(index));
        construct_native_session_message_0075b430(&sent.header.base, posted.type, image.session);
        sent.header.sender_18 = 0;
        sent.header.relay_1a = 0;
        sent.header.flag_1c = 0;
        sent.flag_20 = static_cast<std::uint8_t>(posted.field_20);
        sent.header.base.delivery_04 = 1;
        sent.header.base.profile_00 = image.profile93.slots;
        std::memcpy(&sent.value_24, &posted.field_24, sizeof sent.value_24);
    } else {
    construct_native_session_message_0075b430(&sent.header.base, 0x5d, image.session);
    sent.header.sender_18 = 0;
    sent.header.relay_1a = 0;
    sent.header.flag_1c = 0;
    sent.flag_20 = arm;
    sent.header.base.delivery_04 = 1;
    sent.header.base.profile_00 = image.profile93.slots;
    sent.value_24 = index;
    }

    LocalStream stream;
    write_native_session_message93_0071c7a0(&sent, &stream.out, image.context);
    // 00768530 case 5Dh: operator new(28h), 0075B030 at 00769D44; 0076A27B
    // vtable[8] = 0071C7E0 (00428D30 sign-extends the 5-bit index).
    stream.begin_read();
    if constexpr (kWireGoal4Chain428Bound) {
        OwnedDecodedMessage<NativeSessionMessage93, delete_native_session_message93_0071e390> owner;
        NativeSessionMessage93& queued = owner.get();
        construct_native_session_message93_0075b030(&queued, image.context, image.profile93);
        read_native_session_message93_0071c7e0(&queued, &stream.in, image.context);
        const ClearCommandCopy copy{queued.flag_20, queued.value_24};
        destroy_native_session_message93_0071c820(&sent);
        return copy; // owner runs 0071E390(1); no second queued-object teardown.
    } else {
    NativeSessionMessage93 queued{};
    construct_native_session_message93_0075b030(&queued, image.context, image.profile93);
    read_native_session_message93_0071c7e0(&queued, &stream.in, image.context);
    if constexpr (kWireGoal4Chain373Bound) {
        const ClearCommandCopy copy{queued.flag_20, queued.value_24};
        // bsp.py show 0071C820: root-profile store only; C850C0/C850E0 are
        // the clear-command senders' cleanup funclets. No payload is read
        // after teardown, and stack storage must not take scalar-delete bit 0.
        destroy_native_session_message93_0071c820(&queued);
        destroy_native_session_message93_0071c820(&sent);
        return copy;
    } else {
        return ClearCommandCopy{queued.flag_20, queued.value_24};
    }
    }
}

HostSessionMessages::CommandBuildingCopy HostSessionMessages::queue_command_building_set_owner_d3(
    std::uint32_t player, bool flag) const {
    const Image& image = *image_;
    // 006F727A..006F72C1 / 008A8EE7..008A8F13: 0075B430(D3h), +18h word 0, +4h
    // = 1, +1Ah 0, vtable 00CFADBC (00764220's stores), +1Ch = the slot, byte
    // +20h = the flag. Bytes +21h..+23h are never written by a sender.
    CommandBuildingMessage24 sent{};
    seg42::construct_commandbuilding_setowner_00764220(&sent.h, image.seg42);
    sent.value_1c = player;
    sent.value_20 = flag ? 1u : 0u;

    CommandBuildingCodec codec(image.numeric);
    LocalStream stream;
    cc13_seg33b::command_building_set_owner_message_write_006f2520(codec,
        command_building_object(sent), reinterpret_cast<cc13_seg33b::Object>(&stream.out));
    // 00768530 case D3h: operator new(24h) at 00769F47, 00764220 at 00769F59;
    // 0076A27B vtable[8] = 006F2550 (6-bit player, then the bool).
    stream.begin_read();
    CommandBuildingMessage24 queued{};
    seg42::construct_commandbuilding_setowner_00764220(&queued.h, image.seg42);
    cc13_seg33b::command_building_set_owner_message_read_006f2550(codec,
        command_building_object(queued), reinterpret_cast<cc13_seg33b::Object>(&stream.in));
    // Neither message owns a reference; 006F3210's scalar delete only restamps.
    // 006F5460 hands 006F4D10 the copy's +1Ch and byte +20h (006F549E..006F54A9).
    return CommandBuildingCopy{queued.h.base.type_10, queued.value_1c, queued.value_20 & 0xFFu};
}

HostSessionMessages::CommandBuildingCopy HostSessionMessages::queue_command_building_set_level_d4(
    std::uint32_t level, std::uint32_t argument) const {
    const Image& image = *image_;
    // 006F759F..006F75D0 / 006F5362..006F538E: 0075B430(D4h), +18h word 0, +4h
    // = 1, +1Ah 0, vtable 00CFADD0 (00764280's stores), +1Ch = the level, +20h
    // = the argument.
    CommandBuildingMessage24 sent{};
    seg42::construct_commandbuilding_setlevel_00764280(&sent.h, image.seg42);
    sent.value_1c = level;
    sent.value_20 = argument;

    CommandBuildingCodec codec(image.numeric);
    LocalStream stream;
    cc13_seg33b::command_building_set_level_message_write_006f25e0(codec,
        command_building_object(sent), reinterpret_cast<cc13_seg33b::Object>(&stream.out));
    // 00768530 case D4h: operator new(24h) at 00769F67, 00764280 at 00769F79;
    // 0076A27B vtable[8] = 006F2610 (3-bit level, 4-bit argument).
    stream.begin_read();
    CommandBuildingMessage24 queued{};
    seg42::construct_commandbuilding_setlevel_00764280(&queued.h, image.seg42);
    cc13_seg33b::command_building_set_level_message_read_006f2610(codec,
        command_building_object(queued), reinterpret_cast<cc13_seg33b::Object>(&stream.in));
    // 006F5460 hands 006F38E0 the copy's +1Ch and +20h.
    return CommandBuildingCopy{queued.h.base.type_10, queued.value_1c, queued.value_20};
}

HostSessionMessages::ShipAddTorqueCopy HostSessionMessages::queue_ship_add_torque_93(
    const float (&torque)[3]) const {
    const Image& image = *image_;
    // 0080FFD0 (called at 00827312): 0075B430(93h), +18h word 0, +1Ah 0,
    // +1Ch/+20h/+24h = the three torque dwords, +4h = 1, vtable 00D034C8.
    // 00762220 makes the same header stores, so the sent message is 00762220
    // with the torque; 0077C2A0 at 00827329 (flags 7) takes the local route.
    seg42::ShipAddTorqueMessage sent{};
    if constexpr (kWireGoal4Chain350Bound) {
        UnitMessageConstructionBinding binding(image.session);
        // 0080FFD0 copies stack DWORDs; preserve the caller's three float bit patterns.
        std::uint32_t bits[3];
        std::memcpy(bits, torque, sizeof bits);
        cc13_sweepC::unit_message_add_hull_torque_construct_0080ffd0(
            binding, &sent, bits[0], bits[1], bits[2]);
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d034c8u);
    } else {
    seg42::construct_ship_addtorque_00762220(&sent.h, image.seg42);
    for (int i = 0; i < 3; ++i) sent.torque_1c[i] = torque[i];
    }

    LocalStream stream;
    seg42::write_ship_addtorque_007622b0(&sent, &stream.out, image.numeric);
    // 00768530 case 93h: operator new(28h), 00762220 at 0076942E; 0076A27B
    // vtable[8] = 00762340.
    stream.begin_read();
    if constexpr (kWireGoal4Chain511Bound) {
        // 00768530 case 93h allocates the decoded 28h message. Keep that same
        // allocation through consumption of the real serialized hull torque;
        // 007626A0 then stamps the root profile and frees it once (flags=1).
        // The shared RAII owner also covers a reader exception. The sender
        // remains the existing stack object and never enters this deleting path.
        OwnedDecodedMessage<seg42::ShipAddTorqueMessage,
            seg42::delete_ship_addtorque_007626a0> owner;
        auto& queued = owner.get();
        seg42::construct_ship_addtorque_00762220(&queued.h, image.seg42);
        seg42::read_ship_addtorque_00762340(&queued, &stream.in, image.numeric);
        ShipAddTorqueCopy copy;
        for (int i = 0; i < 3; ++i) copy.torque[i] = queued.torque_1c[i];
        return copy;
    } else {
    seg42::ShipAddTorqueMessage queued{};
    seg42::construct_ship_addtorque_00762220(&queued.h, image.seg42);
    seg42::read_ship_addtorque_00762340(&queued, &stream.in, image.numeric);
    // Neither message owns storage; 007626A0's scalar delete only restamps.
    ShipAddTorqueCopy copy;
    for (int i = 0; i < 3; ++i) copy.torque[i] = queued.torque_1c[i];
    return copy;
    }
}

HostSessionMessages::KamikazeDetonateCopy HostSessionMessages::queue_kamikaze_detonate_70(
    std::uint16_t other_id, const float (&point)[3]) const {
    const Image& image = *image_;
    // 0080FF30: 0075B430(70h), +18h word 0, +1Ah 0, +4h = 1, vtable 00D0348C,
    // +1Ch word = other+174h, +20h..+28h = the point (FLD/FSTP copies).
    // 00761D90 makes the header stores (+1Ch cleared); the sent message is
    // 00761D90 with the id and the point.
    NativeSessionMessage112 sent{};
    if constexpr (kWireGoal4Chain350Bound) {
        UnitMessageConstructionBinding binding(image.session);
        // Only entity+174h is read (bsp.py show 0080FF30). Project the supplied
        // real target handle; no other entity bytes are initialized or consumed.
        alignas(4) std::uint8_t entity_id_view[0x176];
        std::memcpy(entity_id_view + 0x174, &other_id, sizeof other_id);
        cc13_sweepC::unit_message_kamikaze_detonate_construct_0080ff30(
            binding, &sent, entity_id_view, point);
        sent.base.profile_00 = image.profile112.slots;
    } else {
    construct_native_session_message112_00761d90(&sent, image.context112, image.profile112);
    sent.value_1c = other_id;
    for (int i = 0; i < 3; ++i) std::memcpy(&sent.float_bits_20[i], &point[i], sizeof point[i]);
    }

    LocalStream stream;
    write_native_session_message112_00761e20(&sent, &stream.out, image.context112);
    // 00768530 case 70h: 00761D90 at 00769026; 0076A27B vtable[8] = 00761EC0.
    stream.begin_read();
    if constexpr (kWireGoal4Chain478Bound) {
        // 00768530 allocates the decoded 2Ch message; keep that allocation
        // through payload consumption. bsp.py show 00762640: stamp CE4974,
        // free with 00BF65AC iff flags&1. The existing owner uses its matching
        // allocator and runs that body once, including on a decode exception.
        OwnedDecodedMessage<NativeSessionMessage112,
            delete_native_session_message112_00762640> owner;
        NativeSessionMessage112& queued = owner.get();
        construct_native_session_message112_00761d90(&queued, image.context112, image.profile112);
        read_native_session_message112_00761ec0(&queued, &stream.in, image.context112);
        KamikazeDetonateCopy copy;
        copy.other_id = queued.value_1c;
        for (int i = 0; i < 3; ++i) {
            std::memcpy(&copy.point[i], &queued.float_bits_20[i], sizeof copy.point[i]);
        }
        if constexpr (kWireGoal4Chain373Bound) {
            destroy_native_session_message112_00761e10(&sent);
        }
        return copy; // owner frees only the decoded object; sender remains on stack.
    } else {
    NativeSessionMessage112 queued{};
    construct_native_session_message112_00761d90(&queued, image.context112, image.profile112);
    read_native_session_message112_00761ec0(&queued, &stream.in, image.context112);
    KamikazeDetonateCopy copy;
    copy.other_id = queued.value_1c;
    for (int i = 0; i < 3; ++i) std::memcpy(&copy.point[i], &queued.float_bits_20[i], sizeof copy.point[i]);
    if constexpr (kWireGoal4Chain373Bound) {
        // bsp.py show 00761E10: root-profile store only. The original
        // collision sender 008145B0 has two cleanup sites, C90C33/C90C3B.
        // Run that same non-deleting teardown on the consumed stack objects.
        destroy_native_session_message112_00761e10(&queued);
        destroy_native_session_message112_00761e10(&sent);
    } else {
        // Neither message owns storage; the drain's 00762640 scalar delete only
        // restamps the root profile.
    }
    return copy;
    }
}

HostSessionMessages::RepairAddDamageCopy HostSessionMessages::queue_repair_add_damage_9e(
    std::int32_t channel, float seconds, bool add) const {
    const Image& image = *image_;
    // 0080FA50 (00827372 water, 0082740F fire): 0075B430(9Eh), +18h word 0,
    // +1Ah 0, +20h = the flag byte, +4h = 1, vtable 00D0334C, +1Ch = the
    // seconds, +24h = the channel. 007608E0 is the header stores.
    seg42::RepairAddDamageMessage sent{};
    if constexpr (kWireGoal4Chain350Bound) {
        UnitMessageConstructionBinding binding(image.session);
        cc13_sweepC::unit_message_fire_or_water_damage_construct_0080fa50(
            binding, &sent, channel, seconds, add ? 1u : 0u);
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d0334cu);
    } else {
    seg42::construct_shiprepair_adddamage_007608e0(&sent, image.seg42);
    sent.flag_20 = add ? 1u : 0u;
    sent.amount_1c = seconds;
    sent.section_24 = channel;
    }

    LocalStream stream;
    seg42::write_shiprepair_adddamage_00760970(&sent, &stream.out, image.numeric);
    // 00768530 case 9Eh: 007608E0 at 007695EE; 0076A27B vtable[8] = 007609E0
    // (00428D30 sign-extends the 3-bit channel).
    stream.begin_read();
    if constexpr (kWireGoal4Chain509Bound) {
        // 007695DA..007695F5: factory case 9Eh allocates 28h and constructs
        // 007608E0, then joins the real decoder at 0076A27B. Keep this owned
        // receive object through projection and release it through its actual
        // scalar-delete slot (00D0334C -> 00760BD0), flags=1. The sender stays
        // stack-owned. Both paths read the same stream and return the same copy.
        static_assert(sizeof(seg42::RepairAddDamageMessage) == 0x28);
        OwnedDecodedMessage<seg42::RepairAddDamageMessage,
            seg42::delete_shiprepair_adddamage_00760bd0> owner;
        auto& queued = owner.get();
        seg42::construct_shiprepair_adddamage_007608e0(&queued, image.seg42);
        seg42::read_shiprepair_adddamage_007609e0(&queued, &stream.in, image.numeric);
        return RepairAddDamageCopy{queued.section_24, queued.amount_1c, queued.flag_20 != 0};
    } else {
    seg42::RepairAddDamageMessage queued{};
    seg42::construct_shiprepair_adddamage_007608e0(&queued, image.seg42);
    seg42::read_shiprepair_adddamage_007609e0(&queued, &stream.in, image.numeric);
    // Neither message owns storage; 00760BD0's scalar delete only restamps.
    return RepairAddDamageCopy{queued.section_24, queued.amount_1c, queued.flag_20 != 0};
    }
}

HostSessionMessages::ShipJamCopy HostSessionMessages::queue_ship_jam_6b_6c(bool steering) const {
    const Image& image = *image_;
    LocalStream stream;
    ShipJamCopy copy;
    // 008198A0: vtable[0Ch](6Ah) at 008198C6, then (6Bh) at 0081999D, then
    // (6Ch); the first true answer takes its arm.
    const auto answer = [&copy](bool (*is_type)(std::uint32_t)) {
        copy.explosion_6a = is_type(0x6A);
        copy.steering_jam_6b = !copy.explosion_6a && is_type(0x6B);
        copy.engine_jam_6c = !copy.explosion_6a && !copy.steering_jam_6b && is_type(0x6C);
    };
    if (steering) {
        // 00827D09..00827D37 / 008134C4: 0075B430(6Bh) and 007618E0's stores.
        NativeSessionMessage107 sent{};
        construct_native_session_message107_007618e0(&sent, image.context98, image.profile107);
        // 0076E575 vtable[4] = 006D2F20 (vtable 00D03428 read from the PE:
        // 00761990, 006D2F20, 006D1550, 00761960, 004499C0).
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        if constexpr (kWireGoal4Chain440Bound) {
            // 00768530 case 6Bh allocates 24h, constructs 007618E0 and reads
            // the real stream. Retain that receive object until projection;
            // 00761990 flags=1 stamps CE4974 and frees this same allocation.
            OwnedDecodedMessage<NativeSessionMessage107,
                delete_native_session_message107_00761990> owner;
            auto& queued = owner.get();
            construct_native_session_message107_007618e0(&queued, image.context98, image.profile107);
            read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
            copy.type = queued.header.base.type_10;
            answer(&native_session_message_is107_00761960);
            if constexpr (kWireGoal4Chain373Bound) {
                // The sender remains stack-owned; the decoded owner alone
                // runs the scalar-deleting path. Neither object is run twice.
                destroy_native_session_message107_00761950(&sent);
            }
        } else {
        NativeSessionMessage107 queued{};
        construct_native_session_message107_007618e0(&queued, image.context98, image.profile107);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        copy.type = queued.header.base.type_10;
        answer(&native_session_message_is107_00761960);
        if constexpr (kWireGoal4Chain373Bound) {
            // bsp.py show 00761950: root-profile store only; C90B56/C91930
            // clean up the 6Bh messages of the real failure senders.
            destroy_native_session_message107_00761950(&queued);
            destroy_native_session_message107_00761950(&sent);
        } else {
            // The previous host ended these trivial storage objects directly.
        }
        }
    } else {
        // 00827D65 / 00813530: 007619B0 itself.
        NativeSessionMessage108 sent{};
        construct_native_session_message108_007619b0(&sent, image.context98, image.profile108);
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        if constexpr (kWireGoal4Chain440Bound) {
            // 00768530 case 6Ch likewise allocates 24h for 007619B0;
            // 00761A60 flags=1 releases the actual decoded message once.
            OwnedDecodedMessage<NativeSessionMessage108,
                delete_native_session_message108_00761a60> owner;
            auto& queued = owner.get();
            construct_native_session_message108_007619b0(&queued, image.context98, image.profile108);
            read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
            copy.type = queued.header.base.type_10;
            answer(&native_session_message_is108_00761a30);
            if constexpr (kWireGoal4Chain373Bound) {
                destroy_native_session_message108_00761a20(&sent);
            }
        } else {
        NativeSessionMessage108 queued{};
        construct_native_session_message108_007619b0(&queued, image.context98, image.profile108);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        copy.type = queued.header.base.type_10;
        answer(&native_session_message_is108_00761a30);
        if constexpr (kWireGoal4Chain373Bound) {
            // bsp.py show 00761A20: same non-deleting teardown for 6Ch;
            // its failure-sender cleanup funclets are C90B61/C91938.
            destroy_native_session_message108_00761a20(&queued);
            destroy_native_session_message108_00761a20(&sent);
        } else {
            // The previous host ended these trivial storage objects directly.
        }
        }
    }
    // Payloads own no subobjects. With chain440, only the receive allocations
    // are freed; the sender storage still belongs to this stack frame.
    return copy;
}

namespace {
// Goal 4 chain 343. 00780120 (the queued message's receiver ladder) calls the
// message's vtable[0Ch] with 4Bh first (0078015D..00780166) and then each id
// of its ladder in turn; the answered ids are the is-a set its first match
// is chosen from (receiver_for_message_00780120). The ladder ids are that
// function's, in its order.
template <class IsType>
bool copy_goes_to_unit_handler(IsType is_type) {
    static constexpr std::uint8_t kLadder[] = {0x4B, 0x4C, 0x57, 0x4E, 0x4F, 0x50,
                                               0x51, 0x52, 0x97, 0x58, 0x59};
    MessageIsATest is_a{};
    for (const std::uint8_t id : kLadder) {
        if (!is_type(id)) continue;
        if (is_a.count < static_cast<int>(sizeof is_a.ids)) is_a.ids[is_a.count++] = id;
    }
    return receiver_for_message_00780120(is_a) == EntityMessageReceiver::kSessionKindSwitch;
}
}  // namespace

HostSessionMessages::ShipDeathCopy HostSessionMessages::queue_ship_death_98_9a(
    bool explode_to_parts) const {
    const Image& image = *image_;
    static_assert(sizeof(seg42::UnitMessage) == 0x20);
    SessionBoolCodec codec;
    LocalStream stream;
    ShipDeathCopy copy;
    // 00827ADC..00827B01 inline 0075B430(9Ah) and 007613B0's stores; 00827B63
    // calls 00761310 for 98h. Neither sender writes the +1Ch bool.
    using Construct = seg42::UnitMessage* (*)(seg42::UnitMessage*, const seg42::Context&);
    const Construct construct = explode_to_parts ? &seg42::construct_ship_explodetoparts_007613b0
                                                 : &seg42::construct_ship_wrecked_00761310;
    seg42::UnitMessage sent{};
    construct(&sent, image.seg42);
    // 0076E575 vtable[4]: 004645B0 in both tables.
    cc13_seg5::session_bool_message_write_004645b0(codec,
        reinterpret_cast<cc13_seg5::Object>(&sent),
        reinterpret_cast<cc13_seg5::Object>(&stream.out));
    stream.begin_read();
    // 00768530 case 98h: 00761310 at 0076952E; case 9Ah: 007613B0. 0076A27B
    // vtable[8] = 004645D0.
    if constexpr (kWireGoal4Chain514Bound) {
        // The factory allocates a real 20h-byte message for either arm.
        // Decode and route that object before releasing it through the
        // profile's scalar delete. The sender is a stack object (flags=0).
        // Evidence: bsp.py show 00768530, 007625C0 / 007625E0 --asm;
        // profiles 00D033C4 / 00D033D8 above retain their original slot 0.
        auto decode = [&](seg42::UnitMessage& queued) {
            construct(&queued, image.seg42);
            cc13_seg5::session_bool_message_read_004645d0(codec,
                reinterpret_cast<cc13_seg5::Object>(&queued),
                reinterpret_cast<cc13_seg5::Object>(&stream.in));
            copy.type = queued.h.base.type_10;
            copy.to_unit = copy_goes_to_unit_handler(explode_to_parts
                ? &seg42::is_ship_explodetoparts_00761410 : &seg42::is_ship_wrecked_00761370);
            copy.wrecked_98 = copy.to_unit && copy.type == 0x98;
            copy.explode_to_parts_9a = copy.to_unit && copy.type == 0x9A;
        };
        if (explode_to_parts) {
            OwnedDecodedMessage<seg42::UnitMessage,
                delete_ship_explode_to_parts_message_007625e0> owner;
            decode(owner.get());
            delete_ship_explode_to_parts_message_007625e0(&sent, 0u);
        } else {
            OwnedDecodedMessage<seg42::UnitMessage,
                delete_ship_wrecked_message_007625c0> owner;
            decode(owner.get());
            delete_ship_wrecked_message_007625c0(&sent, 0u);
        }
        return copy;
    } else {
    seg42::UnitMessage queued{};
    construct(&queued, image.seg42);
    cc13_seg5::session_bool_message_read_004645d0(codec,
        reinterpret_cast<cc13_seg5::Object>(&queued),
        reinterpret_cast<cc13_seg5::Object>(&stream.in));
    copy.type = queued.h.base.type_10;
    // The copy's own IsType answers the ladder (slot 3 of its table).
    copy.to_unit = copy_goes_to_unit_handler(explode_to_parts
        ? &seg42::is_ship_explodetoparts_00761410 : &seg42::is_ship_wrecked_00761370);
    // 00821E80 switches on the copy's +10h.
    copy.wrecked_98 = copy.to_unit && copy.type == 0x98;
    copy.explode_to_parts_9a = copy.to_unit && copy.type == 0x9A;
    // Neither message owns storage; 007625C0 / 007625E0 only restamp.
    return copy;
    }
}

namespace {
class PartDestroyedMessageBinding final
    : public entity_mission_seg60_argon::UnitMessageConstructionHost {
public:
    PartDestroyedMessageBinding(const NativeSessionMessageContext& session,
        const seg42::VtableBinding& vtables) : session_(session), vtables_(vtables) {}

    void* image_address(std::uint32_t address) override {
        return const_cast<std::uint32_t*>(vtables_.vtable(address));
    }
    void* session_message_construct_base_0075b430(void* message, std::uint8_t type) override {
        return construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), type, session_);
    }

private:
    const NativeSessionMessageContext& session_;
    const seg42::VtableBinding& vtables_;
};
}

HostSessionMessages::ExplodeOnePartCopy HostSessionMessages::queue_ship_explode_one_part_99(
    std::uint32_t part) const {
    const Image& image = *image_;
    // 0092CED0: 0075B430(99h), +18h word 0, +1Ah 0, +1Ch 0, +20h = the index,
    // +4h = 1, vtable 00D033EC; 00761450 makes the same header stores.
    seg42::ExplodeOnePartMessage sent{};
    if constexpr (kWireGoal4Chain415Bound) {
        // Saved-program 0092CED0..0092CF02 (bsp.py show 0092ced0): the
        // sender constructor takes the destroyed segment index itself.
        // Gunnery supplies its live segment index; the session and 00D033EC
        // binding are the same owners the existing local codec uses.
        static_assert(sizeof(part) == sizeof(std::int32_t));
        std::int32_t native_part;
        std::memcpy(&native_part, &part, sizeof(native_part));
        PartDestroyedMessageBinding binding(image.session, image.seg42_vtables);
        entity_mission_seg60_argon::unit_message_part_destroyed_construct_0092ced0(
            binding, &sent, native_part);
    } else {
        seg42::construct_ship_explodeonepart_00761450(&sent, image.seg42);
        sent.part_20 = part;
    }

    LocalStream stream;
    seg42::write_ship_explodeonepart_007614f0(&sent, &stream.out);
    // 00768530 case 99h: 00761450 at 0076954E; 0076A27B vtable[8] = 00761540.
    stream.begin_read();
    seg42::ExplodeOnePartMessage queued{};
    seg42::construct_ship_explodeonepart_00761450(&queued, image.seg42);
    seg42::read_ship_explodeonepart_00761540(&queued, &stream.in);
    ExplodeOnePartCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.to_unit = copy_goes_to_unit_handler(&seg42::is_ship_explodeonepart_007614b0);
    copy.explode_one_part_99 = copy.to_unit && copy.type == 0x99;
    copy.part = queued.part_20;
    // Neither message owns storage; 00762600 only restamps.
    return copy;
}

namespace {
// Goal 4 chain 343. The boundaries of the table-driven segment 26/27 message
// classes (ingame_gui_cc13), bound to the process's header and bit-cursor
// bodies. Write calls receive the cursor itself, read calls stream+4.
struct SessionMessageClassCodec final : ingame_gui_cc13::SessionMessageHost {
    explicit SessionMessageClassCodec(const NativeSessionMessageContext& session)
        : session_(session) {}
    using Raw = ingame_gui_cc13::Raw;
    void construct_base_0075b430(Raw message, std::uint8_t type) override {
        construct_native_session_message_0075b430(
            reinterpret_cast<NativeSessionMessageStorage*>(message.p), type, session_);
    }
    void write_header_0075b480(Raw message, void* cursor) override {
        write_native_session_message_header_0075b480(message.p,
            static_cast<NativeBitCursor*>(cursor));
    }
    void read_header_0075b4c0(Raw message, void* cursor) override {
        read_native_session_message_header_0075b4c0(message.p,
            static_cast<NativeSessionReadStream*>(cursor));
    }
    void write_bool_004290b0(void* cursor, std::uint8_t value) override {
        write_native_bool_bit_004290b0(static_cast<NativeBitCursor*>(cursor), value);
    }
    void write_signed_00429090(void* cursor, std::int32_t value, int bits) override {
        write_native_signed_dword_bits_00429090(static_cast<NativeBitCursor*>(cursor),
            static_cast<std::uint32_t>(value), static_cast<std::uint32_t>(bits));
    }
    void write_unsigned_00429070(void* cursor, std::uint32_t value, int bits) override {
        write_native_unsigned_dword_bits_00429070(static_cast<NativeBitCursor*>(cursor), value,
            static_cast<std::uint32_t>(bits));
    }
    void read_bool_00428d70(void* cursor, std::uint8_t& out) override {
        bool value = false;
        read_native_bool_bit_00428d70(static_cast<NativeBitCursor*>(cursor), &value);
        out = value ? 1u : 0u;
    }
    void read_signed_00428d30(void* cursor, std::int32_t& out, int bits) override {
        read_native_i32_bits_00428d30(static_cast<NativeBitCursor*>(cursor), &out,
            static_cast<std::uint32_t>(bits));
    }
    void read_unsigned_00428d10(void* cursor, std::uint32_t& out, int bits) override {
        read_native_u32_bits_00428d10(static_cast<NativeBitCursor*>(cursor), &out,
            static_cast<std::uint32_t>(bits));
    }
    void read_word_bits_00428e30(void* cursor, std::uint16_t& out, int bits) override {
        read_native_word_bits_00428e30(static_cast<NativeBitCursor*>(cursor), &out,
            static_cast<std::uint32_t>(bits));
    }
    // Only the scalar deleting destructors free a block; no local copy calls one.
    void free_block(void*) override { std::abort(); }

private:
    const NativeSessionMessageContext& session_;
};
}  // namespace

HostSessionMessages::StartLandingCopy HostSessionMessages::queue_start_landing_94() const {
    const Image& image = *image_;
    const ingame_gui_cc13::SessionMessageClass* const cls =
        ingame_gui_cc13::find_session_message_class(0x94);
    StartLandingCopy copy;
    if (cls == nullptr) return copy;
    SessionMessageClassCodec construct_host(image.session);
    SessionBoolCodec codec;
    // 009F3598 0064A820: the 20h-byte message (header, then the +1Ch bool).
    seg42::UnitMessage sent{};
    ingame_gui_cc13::session_message_construct(*cls,
        ingame_gui_cc13::Raw{reinterpret_cast<std::uint8_t*>(&sent)}, 0, 0, construct_host);
    LocalStream stream;
    cc13_seg5::session_bool_message_write_004645b0(codec,
        reinterpret_cast<cc13_seg5::Object>(&sent),
        reinterpret_cast<cc13_seg5::Object>(&stream.out));
    // 00768530 case 94h: 0064A820 at 007694AE; 0076A27B vtable[8] = 004645D0.
    stream.begin_read();
    seg42::UnitMessage queued{};
    ingame_gui_cc13::session_message_construct(*cls,
        ingame_gui_cc13::Raw{reinterpret_cast<std::uint8_t*>(&queued)}, 0, 0, construct_host);
    cc13_seg5::session_bool_message_read_004645d0(codec,
        reinterpret_cast<cc13_seg5::Object>(&queued),
        reinterpret_cast<cc13_seg5::Object>(&stream.in));
    copy.type = queued.h.base.type_10;
    // 00780120's ladder through 0064A850, session_message_is_type over the 94h
    // row (bases 49h, 46h).
    copy.to_unit = copy_goes_to_unit_handler([cls](std::uint32_t type) {
        return ingame_gui_cc13::session_message_is_type(*cls, static_cast<std::int32_t>(type));
    });
    copy.start_landing_94 = copy.to_unit && copy.type == 0x94;
    // Neither message owns storage; 0064AE00 is not run on these.
    return copy;
}

namespace {
// Goal 4 chain 343. The seg41 message bodies' boundaries: the profile for
// an image vtable (the same binding seg42 uses) and the extended header.
struct Seg41MessageHost final : cc13_seg41::SessionMessageHost {
    explicit Seg41MessageHost(const seg42::VtableBinding& vtables) : vtables_(vtables) {}
    const std::uint32_t* vtable(std::uint32_t image_address) override {
        return vtables_.vtable(image_address);
    }
    void write_header_0075b480(const void* message, NativeBitCursor* cursor) override {
        write_native_session_message_header_0075b480(message, cursor);
    }
    void read_header_0075b4c0(void* message, NativeSessionReadStream& stream) override {
        read_native_session_message_header_0075b4c0(message, &stream);
    }

private:
    const seg42::VtableBinding& vtables_;
};
}  // namespace

HostSessionMessages::ShipSinkCopy HostSessionMessages::queue_ship_sink_92(
    const float* rates, std::uint32_t count) const {
    const Image& image = *image_;
    ShipSinkCopy copy;
    // Both blocks are the factory's E8h; a count past its capacity would
    // overrun the image's block too, so it is not posted.
    if (count > kShipSinkCapacity) return copy;
    Seg41MessageHost host(image.seg42_vtables);
    // 0074ED18..0074ED62: 0074EAB0's stores, inline.
    alignas(4) std::uint8_t sent[0xE8]{};
    cc13_seg41::ship_sink_construct_0074eab0(sent, count, rates, image.session, host);

    LocalStream stream;
    cc13_seg41::ship_sink_write_0074eb20(sent, &stream.out, image.maximum_00d7a248, image.numeric);
    // 00768530 case 92h: operator new(E8h) at 007693F7, 00762130 at 0076940E;
    // 0076A27B vtable[8] = 0074EB80.
    stream.begin_read();
    alignas(4) std::uint8_t queued[0xE8]{};
    seg42::construct_ship_sink_00762130(reinterpret_cast<seg42::HeaderedMessage*>(queued),
                                        image.seg42);
    cc13_seg41::ship_sink_read_0074eb80(queued, stream.in, image.maximum_00d7a248, image.numeric);
    copy.type = reinterpret_cast<const NativeSessionMessageStorage*>(queued)->type_10;
    copy.to_unit = copy_goes_to_unit_handler(&cc13_seg41::ship_sink_is_type_0074eb00);
    copy.sink_92 = copy.to_unit && copy.type == 0x92;
    std::memcpy(&copy.count, queued + 0x1C, sizeof copy.count);
    if (copy.count > kShipSinkCapacity) copy.count = kShipSinkCapacity;
    std::memcpy(copy.rates, queued + 0x20, copy.count * sizeof(float));
    // Neither message owns storage; 0074EDA0 only restamps.
    return copy;
}

HostSessionMessages::ShipyardLaunchCopy HostSessionMessages::queue_shipyard_launch_7a(
    std::uint8_t flag) const {
    const Image& image = *image_;
    // 009CFBFB..009CFC23: 0080F960's stores inline; 007604C0 makes the same
    // header, and the sender adds +20h.
    seg42::VehicleShipyardLaunchMessage sent{};
    seg42::construct_vehicle_shipyard_launch_007604c0(&sent, image.seg42);
    sent.flag_20 = flag;

    LocalStream stream;
    seg42::write_vehicle_shipyard_launch_00760550(&sent, &stream.out);
    // 00768530 case 7Ah: 007604C0 at 00769166; 0076A27B vtable[8] = 007605A0.
    stream.begin_read();
    seg42::VehicleShipyardLaunchMessage queued{};
    seg42::construct_vehicle_shipyard_launch_007604c0(&queued, image.seg42);
    seg42::read_vehicle_shipyard_launch_007605a0(&queued, &stream.in);
    ShipyardLaunchCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.to_unit = copy_goes_to_unit_handler(&seg42::is_vehicle_shipyard_launch_00760520);
    copy.shipyard_launch_7a = copy.to_unit && copy.type == 0x7A;
    copy.flag = queued.flag_20;
    // Neither message owns storage; 00760650 only restamps.
    return copy;
}

namespace {
// Goal 4 chain 356. The seven boundaries 006E3940 / 006E39B0 / 006E39E0 call,
// bound to the process's own bodies: 0075B430 over Image's session, the
// extended header 0075B480 / 0075B4C0 and the bool / word bit helpers. The
// writer hands every call the 10h cursor; the reader hands the header the 18h
// stream and the fields stream+4, its cursor (late_read_bool_word).
class GunFireMessageBinding final : public cc13_seg33a::Seg33aGunFireMessageHost {
public:
    explicit GunFireMessageBinding(const NativeSessionMessageContext& session)
        : session_(session) {}
    void bit_cursor_write_boolean_004290b0(void* cursor, std::uint32_t value) override {
        write_native_bool_bit_004290b0(static_cast<NativeBitCursor*>(cursor), value);
    }
    void bit_cursor_read_boolean_00428d70(void* cursor, std::uint8_t* out) override {
        read_native_bool_bit_00428d70(static_cast<NativeBitCursor*>(cursor),
            reinterpret_cast<bool*>(out));
    }
    void bit_cursor_write_word_bits_00429120(void* cursor, std::uint32_t value,
                                            std::int32_t bits) override {
        write_native_word_bits_00429120(static_cast<NativeBitCursor*>(cursor), value,
            static_cast<std::uint32_t>(bits));
    }
    void bit_cursor_read_word_bits_00428e30(void* cursor, std::uint16_t* out,
                                           std::int32_t bits) override {
        read_native_word_bits_00428e30(static_cast<NativeBitCursor*>(cursor), out,
            static_cast<std::uint32_t>(bits));
    }
    void session_message_construct_base_0075b430(void* message, std::uint32_t type) override {
        construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), type, session_);
    }
    void session_message_write_header_0075b480(void* message, void* cursor) override {
        write_native_session_message_header_0075b480(message,
            static_cast<NativeBitCursor*>(cursor));
    }
    void session_message_read_header_0075b4c0(void* message, void* stream) override {
        read_native_session_message_header_0075b4c0(message,
            static_cast<NativeSessionReadStream*>(stream));
    }

private:
    const NativeSessionMessageContext& session_;
};

// The 24h-byte ADh message: the unit header with its +1Ch bool, then the
// 12-bit target word at +20h (006E3940's stores, 006E39B0's fields).
struct GunFireMessage24 {
    seg42::UnitMessage u;
    std::uint16_t target_20;
    std::uint16_t pad_22;
};
static_assert(sizeof(GunFireMessage24) == 0x24);
static_assert(offsetof(GunFireMessage24, target_20) == 0x20);

// Goal 4 chain 356. The receiving gun's switch 0072D830 over a copy. The
// dispatch reports the arm the copy reached; the gunnery host then runs that
// arm's body for the gun that sent the message: ADh vtable[1DCh] (FireIfReady),
// 68h vtable[1C8h] (0072B5A0) and 69h vtable[1CCh] (007280A0). ACh and the
// 0043F860 tail are counted, not run: none of these copies selects them.
class GunMessageDispatch final : public GunNetworkMessageHost {
public:
    void vcall_1c8(void*, void*) override { set_destroyed_arm = true; }
    void vcall_1cc(void*, void*) override { clear_destroyed_arm = true; }
    void vcall_1dc(void*) override { fire_if_ready = true; }
    bool entity_handle_messages_4b4c_0043f860(void*, void*) override {
        ++other_arms;
        return false;
    }
    void gun_select_fire_mode_0072d5b0(void*, std::uint32_t, std::uint32_t) override {
        ++other_arms;
    }
    bool fire_if_ready{false};
    bool set_destroyed_arm{false};
    bool clear_destroyed_arm{false};
    int other_arms{0};
};
}  // namespace

HostSessionMessages::GunFireCopy HostSessionMessages::queue_gun_fire_ad(
    std::uint16_t target) const {
    const Image& image = *image_;
    GunFireMessageBinding binding(image.session);
    // 0072D252..0072D27D: 006E3940's stores with the target word, so the
    // sender's message is built by 006E3940 itself.
    GunFireMessage24 sent{};
    cc13_seg33a::gun_fire_message_construct_006e3940(binding, &sent, target);
    LocalStream stream;
    cc13_seg33a::gun_fire_message_write_006e39b0(binding, &sent, &stream.out);
    // 00768530 case ADh: operator new(24h), 006E3940(0) at 0076982A; 0076A27B
    // vtable[8] = 006E39E0.
    stream.begin_read();
    GunFireMessage24 queued{};
    cc13_seg33a::gun_fire_message_construct_006e3940(binding, &queued, 0);
    cc13_seg33a::gun_fire_message_read_006e39e0(binding, &queued, &stream.in);
    GunFireCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.target = queued.target_20;
    // 00780120's ladder over 006E3940's IsType 006E3980 {ADh, 49h, 46h}.
    const band_006b_cc13::TypeIdRule* const is_gun_fire =
        band_006b_cc13::find_type_id_rule(0x006E3980u);
    copy.to_entity = is_gun_fire != nullptr && copy_goes_to_unit_handler(
        [is_gun_fire](std::uint32_t type) {
            return band_006b_cc13::type_id_rule_accepts(*is_gun_fire,
                static_cast<std::int32_t>(type));
        });
    if (copy.to_entity) {
        // The gun's slot 164h, 0072D830 (directly or through 0084C7E0 /
        // 00803480's jump), over the rebuilt copy. The gun object is the
        // gunnery host's; the dispatch reports the arm it reached.
        GunMessageDispatch dispatch;
        cc13_front3_gun_handle_network_message_0072d830(nullptr, &queued, dispatch);
        copy.fire_if_ready = dispatch.fire_if_ready;
    }
    // Neither message owns storage; 006E47C0 only restamps.
    return copy;
}

HostSessionMessages::GunDestroyedCopy HostSessionMessages::queue_gun_destroyed_68_69(
    bool set) const {
    const Image& image = *image_;
    // 0072A016..0072A04E / 0072A116..0072A14E: 0075B430(type) and the stores
    // 00728F20 / 00728FA0 make, +20h the default 256.0f included, so both
    // messages are built by the constructor 00768530 calls for the type.
    const auto construct = [&image, set](NativeSessionMessage104* message) {
        if (set) {
            construct_native_session_message104_00728f20(message, image.context104,
                image.profile104);
        } else {
            construct_native_session_message105_00728fa0(message, image.context104,
                image.profile105);
        }
    };
    bool (*const is_type)(std::uint32_t) = set ? &native_session_message_is104_00728f70
                                               : &native_session_message_is105_00728ff0;
    NativeSessionMessage104 sent{};
    construct(&sent);
    LocalStream stream;
    write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
    // 00768530 case 68h / 69h: operator new(24h), 00728F20 at 00768F26 /
    // 00728FA0 at 00768F46; 0076A27B vtable[8] = 006D1550.
    stream.begin_read();
    if constexpr (kWireGoal4Chain478Bound) {
        // Both messages borrow the same live session/codec owners as before.
        // Consume their decoded fields and receiver dispatch before releasing
        // the native allocation. bsp.py show 00729050 / 00729070: CE4974 store,
        // then 00BF65AC iff bit 0; neither payload owns further subobjects.
        const auto consume = [&](NativeSessionMessage104& queued) {
            construct(&queued);
            read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
            GunDestroyedCopy copy;
            copy.type = queued.header.base.type_10;
            copy.to_entity = copy_goes_to_unit_handler(is_type);
            if (copy.to_entity) {
                GunMessageDispatch dispatch;
                cc13_front3_gun_handle_network_message_0072d830(nullptr, &queued, dispatch);
                copy.set_arm_68 = dispatch.set_destroyed_arm && is_type(0x68);
                copy.clear_arm_69 = dispatch.clear_destroyed_arm && is_type(0x69);
            }
            if constexpr (kWireGoal4Chain373Bound) {
                if (set) destroy_native_session_message104_00728f60(&sent);
                else destroy_native_session_message105_00728fe0(&sent);
            }
            return copy;
        };
        if (set) {
            OwnedDecodedMessage<NativeSessionMessage104,
                delete_native_session_message104_00729050> owner;
            return consume(owner.get());
        } else {
            OwnedDecodedMessage<NativeSessionMessage105,
                delete_native_session_message105_00729070> owner;
            return consume(owner.get());
        }
    } else {
    NativeSessionMessage104 queued{};
    construct(&queued);
    read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
    GunDestroyedCopy copy;
    copy.type = queued.header.base.type_10;
    copy.to_entity = copy_goes_to_unit_handler(is_type);
    if (copy.to_entity) {
        // The gun's slot 164h, 0072D830, over the copy; then the arm body's own
        // IsType test on the request (0072B5A0 68h, 007280AC 69h).
        GunMessageDispatch dispatch;
        cc13_front3_gun_handle_network_message_0072d830(nullptr, &queued, dispatch);
        copy.set_arm_68 = dispatch.set_destroyed_arm && is_type(0x68);
        copy.clear_arm_69 = dispatch.clear_destroyed_arm && is_type(0x69);
    }
    if constexpr (kWireGoal4Chain373Bound) {
        // bsp.py show 00728F60/00728FE0: stamp CE4974, retaining every other
        // byte. Their cleanup funclets C85560/C85580 select the same class
        // as the constructor above. Consume dispatch results before teardown;
        // use non-deleting bodies because both host objects are on the stack.
        if (set) {
            destroy_native_session_message104_00728f60(&queued);
            destroy_native_session_message104_00728f60(&sent);
        } else {
            destroy_native_session_message105_00728fe0(&queued);
            destroy_native_session_message105_00728fe0(&sent);
        }
    } else {
        // Neither message owns storage; 00729050 / 00729070 only restamp.
    }
    return copy;
    }
}

HostSessionMessages::PlaneDisappearCopy HostSessionMessages::queue_plane_disappear_c7(
    std::uint16_t holder_id) const {
    const Image& image = *image_;
    // 007CE6FE..007CE735: 0075B430(C7h), +18h word 0, +1Ah 0, +4h = 1, +1Ch 0,
    // vtable 00D0352C, which are 00762B20's stores; then +20h the holder id.
    seg42::PlaneDisappearMessage sent{};
    seg42::construct_plane_disappear_00762b20(&sent, image.seg42);
    sent.id_20 = holder_id;
    LocalStream stream;
    seg42::write_plane_disappear_00762be0(&sent, &stream.out);
    // 00768530 case C7h: operator new(24h), 00762B20 at 00769C04; 0076A27B
    // vtable[8] = 00762C30.
    stream.begin_read();
    if constexpr (kWireGoal4Chain506Bound) {
        OwnedDecodedMessage<seg42::PlaneDisappearMessage,
            delete_plane_disappear_message_00762bc0> owner;
        auto& queued = owner.get();
        seg42::construct_plane_disappear_00762b20(&queued, image.seg42);
        seg42::read_plane_disappear_00762c30(&queued, &stream.in);
        PlaneDisappearCopy copy;
        copy.type = queued.u.h.base.type_10;
        copy.holder_id = queued.id_20;
        delete_plane_disappear_message_00762bc0(&sent, 0u);
        return copy; // 00762BC0(1) releases the factory's 24h allocation.
    } else {
    seg42::PlaneDisappearMessage queued{};
    seg42::construct_plane_disappear_00762b20(&queued, image.seg42);
    seg42::read_plane_disappear_00762c30(&queued, &stream.in);
    PlaneDisappearCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.holder_id = queued.id_20;
    // Neither message owns storage; 00762BC0 only restamps.
    return copy;
    }
}

} // namespace bsp
