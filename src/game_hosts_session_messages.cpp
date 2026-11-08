// cc15-sm1: host-owned image memory for session messages. See the header.
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/native_session_message_participant_owner.hpp"
#include "bsp/native_plane_session_message_lifetime.hpp"
#include "bsp/native_gun_control_message_lifetime.hpp"
#include "bsp/native_session_message_tags_66_67_68.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/cc13_powerup.hpp"
#include "bsp/powerup_seg58_argon.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/cc13_sweepC.hpp"
#include "bsp/actions_seg5_cc13.hpp"
#include "bsp/native_bit_cursor_fields.hpp"
#include "bsp/session_message_dispatch.hpp"
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/cc13_seg41_land.hpp"
#include "bsp/band_0071_cc13.hpp"
#include "bsp/gameplay_seg41_cc13.hpp"
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
#include "bsp/gameplay_seg38_cc13.hpp"
#include "bsp/band_006b_cc13.hpp"
#include "bsp/cc13_seg29U_trap_host.hpp"
#include "bsp/cc13_sweepC_trap_host.hpp"
#include "bsp/cc13_front3.hpp"
#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_seg47.hpp"
#include "bsp/gameplay_seg47_cc13.hpp"
#include "bsp/seg27_cc13.hpp"
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
    // bsp.gpr /battlestationspacific.exe, 00E17669: initial NUL byte.
    // The power-up profiles borrow this fallback; a zero-peer send runs no codec.
    const char powerup_empty_00e17669{};
    const SessionParticipantPools* vehicle_race_participants{};
    const std::size_t* vehicle_race_local_slot{};

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
    // Goal 5 chain 641: the plane failure classes 63h..67h (sessions 99..103),
    // the same scalar codec (103 with the 103..106 context).
    NativeSessionMessage99Profile profile99{context98};
    NativeSessionMessage100Profile profile100{context98};
    NativeSessionMessage101Profile profile101{context98};
    NativeSessionMessage102Profile profile102{context98};
    NativeSessionMessage103Profile profile103{context104};

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
        // Goal 5 chain 628. 86h 00CF864C (bsp.py ghidra bytes 00cf864c --length
        // 24, bsp.gpr's /battlestationspacific.exe): 006BEDF0 scalar delete,
        // 006BDB30 write, 006BDB60 read, 006BDB00 IsType, 006BDB90 (the +20h
        // handle-table test in place of 004499C0).
        std::uint32_t slots_00cf864c[5]{0x006bedf0u, 0x006bdb30u, 0x006bdb60u, 0x006bdb00u,
                                        0x006bdb90u};
        // Goal 5 chain 628. 87h 00D01870 (bsp.py ghidra bytes 00d01870 --length
        // 20): 00758CD0 scalar delete, 00758C20 write, 00758C70 read, 00758BF0
        // IsType, 004499C0.
        std::uint32_t slots_00d01870[5]{0x00758cd0u, 0x00758c20u, 0x00758c70u, 0x00758bf0u,
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
        // Goal 5 chain 636. 82h 00CF64C4 (bsp.py ghidra bytes 00CF64C4 --length
        // 24): 00656420 scalar delete, 00656320 write, 006563A0 read, 006562F0
        // IsType, 004499C0. 00763320 (seg42) and 00656280 (seg27) store it.
        std::uint32_t slots_00cf64c4[5]{0x00656420u, 0x00656320u, 0x006563a0u, 0x006562f0u,
                                        0x004499c0u};
        // Goal 5 chain 632. 7Bh 00CE53AC (bsp.py ghidra bytes 00CE53AC --length
        // 20, bsp.gpr's /battlestationspacific.exe): 00464FE0 scalar delete,
        // 004647D0 write, 00464810 read, 004647B0 IsType, 004499C0.
        std::uint32_t slots_00ce53ac[5]{0x00464fe0u, 0x004647d0u, 0x00464810u, 0x004647b0u,
                                        0x004499c0u};
        // Goal 5 chain 632. 7Ch 00CFAA0C (bsp.py ghidra bytes 00CFAA0C --length
        // 20): 006EC100 scalar delete, 006EBA20 write, 006EBA50 read, 006EBA00
        // IsType, 004499C0.
        std::uint32_t slots_00cfaa0c[5]{0x006ec100u, 0x006eba20u, 0x006eba50u, 0x006eba00u,
                                        0x004499c0u};
        // Goal 5 chain 775: bsp.py ghidra bytes 00D032FC --length 20,
        // bsp.gpr /battlestationspacific.exe. SetRace's complete five-slot
        // profile: scalar delete, writer, reader, IsType, always-true.
        std::uint32_t slots_00d032fc[5]{0x00760630u, 0x00760420u, 0x00760470u, 0x007603f0u,
                                        0x004499c0u};
        const std::uint32_t* vtable(std::uint32_t image_address) const override {
            if (image_address == 0x00d032fcu) return slots_00d032fc;
            if (image_address == 0x00cf64c4u) return slots_00cf64c4;
            if (image_address == 0x00ce53acu) return slots_00ce53ac;
            if (image_address == 0x00cfaa0cu) return slots_00cfaa0c;
            // Only queue_plane_disappear_c7 constructs a C7h message.
            if (image_address == 0x00d0352cu) return slots_00d0352c;
            if (image_address == 0x00d033c4u) return slots_00d033c4;
            if (image_address == 0x00d033d8u) return slots_00d033d8;
            if (image_address == 0x00d033ecu) return slots_00d033ec;
            if (image_address == 0x00d00518u) return slots_00d00518;
            if (image_address == 0x00d03310u) return slots_00d03310;
            if (image_address == 0x00d034c8u) return slots_00d034c8;
            if (image_address == 0x00d0334cu) return slots_00d0334c;
            if (image_address == 0x00cfadbcu) return slots_00cfadbc;
            if (image_address == 0x00cfadd0u) return slots_00cfadd0;
            if (image_address == 0x00cfaa20u) return slots_00cfaa20;
            if (image_address == 0x00d032e8u) return slots_00d032e8;
            if (image_address == 0x00d032d0u) return slots_00d032d0;
            if (image_address == 0x00d03518u) {
                return slots_00d03518;
            }
            if (image_address == 0x00d03504u) return slots_00d03504;
            if (image_address == 0x00d02d30u) return slots_00d02d30;
            if (image_address == 0x00d02d44u) return slots_00d02d44;
            if (image_address == 0x00d03360u) return slots_00d03360;
            if (image_address == 0x00d034a0u) return slots_00d034a0;
            if (image_address == 0x00d034dcu) return slots_00d034dc;
            if (image_address == 0x00d03554u) return slots_00d03554;
            if (image_address == 0x00cf864cu) return slots_00cf864c;
            if (image_address == 0x00d01870u) return slots_00d01870;
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

// 00780120 asks the decoded object's vtable[0Ch] in this order. Keep the
// predicate's first match as the receiver decision, rather than assuming the
// receiver from the sender API. 00780636's 59h arm enters 00721A40, whose
// mutable +10h kind then selects the director payload arm.
template <class IsType>
EntityMessageReceiver local_message_receiver(IsType is_type) {
    static constexpr std::uint8_t kLadder[] = {0x4B, 0x4C, 0x57, 0x4E, 0x4F, 0x50,
                                               0x51, 0x52, 0x97, 0x58, 0x59};
    MessageIsATest is_a{};
    for (const std::uint8_t id : kLadder) {
        if (!is_type(id)) continue;
        is_a.ids[is_a.count++] = id;
        break;
    }
    return receiver_for_message_00780120(is_a);
}

template <class IsType>
void require_local_director_route(std::uint8_t decoded_type, std::uint8_t expected_type,
    IsType is_type) {
    if (local_message_receiver(is_type) != EntityMessageReceiver::kWeaponDirector
        || decoded_type != expected_type)
        throw std::logic_error("decoded message did not select its weapon-director arm");
}
}  // namespace

HostSessionMessages::HostSessionMessages() : image_(std::make_unique<Image>()) {}
HostSessionMessages::~HostSessionMessages() = default;

namespace {
// Read-only views of the sender's item/binding fields, not another inventory.
// The host's kNoBinding sentinel represents the native null item+8h pointer.
// Non-null bindings require a real unit+174h and are rejected by these senders.
struct PowerupPacketItemView {
    std::uint32_t word_00;
    const PowerupClassConfig* cls_04;
    const void* binding_08;
    std::int32_t use_limit_0c;
};
static_assert(sizeof(PowerupPacketItemView) == 0x10);
static_assert(offsetof(PowerupPacketItemView, cls_04) == 4);
static_assert(offsetof(PowerupPacketItemView, binding_08) == 8);
static_assert(offsetof(PowerupPacketItemView, use_limit_0c) == 0x0c);
static_assert(offsetof(PowerupClassConfig, name_00) == 0);

struct PowerupPacketModifierView {
    const PowerupPacketItemView* item_00;
    std::byte payload_04[0x24];
};
static_assert(sizeof(PowerupPacketModifierView) == 0x28);
static_assert(offsetof(PowerupPacketModifierView, payload_04) == 4);
static_assert(offsetof(game_powerups::PowerupModifier, cls) == sizeof(PowerupPacketModifierView));

class PowerupMessageConstructionBinding final
    : public powerup_seg58_argon::PowerupMessageConstructionHost {
public:
    PowerupMessageConstructionBinding(const NativeSessionMessageContext& session,
        NativeStringRawPoolContext& strings, std::uint32_t address,
        const std::uint32_t* profile) noexcept
        : session_(session), strings_(strings), address_(address), profile_(profile) {}

    void* image_address(std::uint32_t address) override {
        if (address != address_) throw std::logic_error("power-up sender profile");
        return const_cast<std::uint32_t*>(profile_);
    }
    void* session_message_construct_base_0075b430(void* message, std::uint8_t type) override {
        return construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), type, session_);
    }
    void native_string_resize_0041dd40(void* header, std::uint32_t length,
        std::uint8_t preserve) override {
        // Each producer zeroes its header before this first potentially throwing
        // operation. Its native EH cleanup must also run if allocation fails.
        string_started_ = true;
        resize_native_string_header_0041dd40(header, strings_, length, preserve != 0);
    }
    void* memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) override {
        return std::memcpy(dst, src, bytes);
    }
    bool string_started() const noexcept { return string_started_; }
private:
    const NativeSessionMessageContext& session_;
    NativeStringRawPoolContext& strings_;
    std::uint32_t address_;
    const std::uint32_t* profile_;
    bool string_started_{};
};
}

bool HostSessionMessages::broadcast_powerup_item(std::int32_t slot,
    const cc13_powerup::PowerupItem& item, std::int32_t operation,
    const SessionParticipantPools& participants, const std::size_t& local_slot) const {
    if (item.cls == nullptr || item.binding != cc13_powerup::PowerupItem::kNoBinding)
        return false;
    Image& image = *image_;
    NativeSessionMessageParticipantOwner selected(participants, local_slot);
    const NativeSessionMessageContext session{image.current_game, &selected};
    auto& strings = game::game_native_string_process().raw_context();
    // The invalid-parameter callback belongs to message 66's vector accessors;
    // neither this sender nor either of its profile codecs uses those accessors.
    const NativeSessionMessage66To68Context context{&session, &strings,
        &image.powerup_empty_00e17669, &image.numeric, &image.maximum_00d7a248, nullptr};
    const NativeSessionMessage67Profile profile(context);
    PowerupMessageConstructionBinding construction(session, strings, 0x00d0366cu, profile.slots);
    const PowerupPacketItemView view{item.word_00, item.cls, nullptr, item.use_limit};
    NativeSessionMessage67 message;
    try {
        // 008EDDC0 @008EDFB7..008EDFF7 and 008E9FD0 @008EA06B..008EA079:
        // the real slot, item name/use limit and operation, with both units null.
        // 008E6E00 owns the +1Ch DWORD length and +20h DWORD pool pointer.
        powerup_seg58_argon::session_message67_construct_from_class_008e6e00(
            construction, &message, static_cast<std::uint32_t>(slot), &view,
            static_cast<std::uint32_t>(operation), nullptr, nullptr);
        // The retained host-only session has no peers. 0076A9F0/00784790
        // therefore performs no serialization or delivery; do not loop back.
    } catch (...) {
        if (construction.string_started())
            destroy_native_session_message67_007657d0(&message, context, profile);
        throw;
    }
    destroy_native_session_message67_007657d0(&message, context, profile);
    return true;
}

bool HostSessionMessages::broadcast_powerup_modifier(const game_powerups::PowerupModifier& record,
    std::int32_t operation, const SessionParticipantPools& participants,
    const std::size_t& local_slot) const {
    // 008E6FB0 reads item->binding->unit+174h only for operation zero.
    // Expiration (operation two) never dereferences that graph.
    if (operation == 0 && record.item != nullptr
        && record.item->binding != cc13_powerup::PowerupItem::kNoBinding) return false;
    Image& image = *image_;
    NativeSessionMessageParticipantOwner selected(participants, local_slot);
    const NativeSessionMessageContext session{image.current_game, &selected};
    auto& strings = game::game_native_string_process().raw_context();
    const NativeSessionMessage66To68Context context{&session, &strings,
        &image.powerup_empty_00e17669, &image.numeric, &image.maximum_00d7a248, nullptr};
    const NativeSessionMessage68Profile profile(context);
    PowerupMessageConstructionBinding construction(session, strings, 0x00d03680u, profile.slots);
    PowerupPacketItemView item_view;
    PowerupPacketModifierView view;
    std::memcpy(&view, &record, sizeof(view)); // borrow the real +04h..+27h fields and name
    if (operation == 0 && record.item != nullptr) {
        item_view = {record.item->word_00, record.item->cls, nullptr, record.item->use_limit};
        view.item_00 = &item_view; // translate the authoritative absent-binding sentinel
    }
    NativeSessionMessage68 message;
    try {
        // 008E8C30 @008E8CE6..008E8D46 / 008EA250 @008EA466..008EA4A9:
        // 008E6FB0 copies all real modifier fields and owns name+3Ch/+40h.
        powerup_seg58_argon::session_message68_construct_from_modifier_008e6fb0(
            construction, &message, &view, static_cast<std::uint32_t>(operation));
        // Same zero-peer terminal as the item sender; no receiving mutation.
    } catch (...) {
        if (construction.string_started())
            destroy_native_session_message68_007658c0(&message, context, profile);
        throw;
    }
    destroy_native_session_message68_007658c0(&message, context, profile);
    return true;
}

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
}

std::shared_ptr<NativeSessionMessage95> HostSessionMessages::queue_user_path_point_5f(
    const float point[3]) const {
    Image& image = *image_;
    NativeSessionMessage95 sent{};
    // Evidence: bsp.py show 0071D340 --asm: MOVSS copies at 0071D36A..3A4,
    // presence 1, then route flags 7. Use that existing sender body once;
    // exceptional encodings keep its MOVSS payload construction below.
    const auto posted = gameplay_seg38_cc13::add_user_path_point_message_0071d340(
        {point[0], point[1], point[2]});
    {
        const float payload[3]{posted.point.x, posted.point.y, posted.point.z};
        // Evidence: bsp.py show 0071CC20 --asm, CC47..CC59, versus
        // 0071D340 --asm, D36A..D3A4. Both produce the same real 5Fh message,
        // but the shared constructor uses FLD/FSTP and the inlined sender
        // uses MOVSS. Admit only normal binary32 values and signed zeros:
        // their copies are exact in every x87 rounding/precision mode and
        // raise no FP exceptions. Test the bits without inspecting an sNaN
        // through floating-point instructions. Other encodings retain the
        // existing transport construction below.
        bool exact_x87_copy = true;
        for (const float& component : payload) {
            std::uint32_t bits;
            std::memcpy(&bits, &component, sizeof bits);
            const auto magnitude = bits & 0x7fffffffu;
            const auto exponent = magnitude & 0x7f800000u;
            if (magnitude != 0 && (exponent == 0 || exponent == 0x7f800000u))
                exact_x87_copy = false;
        }
        if (exact_x87_copy) {
            construct_native_session_message95_value_0071cc20(&sent, payload,
                static_cast<std::uint8_t>(posted.add_2c), image.context95, image.profile95);
        } else {
            construct_native_session_message_0075b430(&sent.header.base, posted.type, image.session);
            sent.header.sender_18 = 0;
            sent.header.relay_1a = 0;
            sent.header.flag_1c = 0;
            sent.header.base.delivery_04 = 1;
            sent.header.base.profile_00 = image.profile95.slots;
            std::memcpy(sent.vector_20, payload, sizeof payload);
            sent.present_2c = static_cast<std::uint8_t>(posted.add_2c);
        }
    }
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
    // Evidence: bsp.py show 0071CC70 and 00780120. The actual 5Fh class
    // accepts the ladder's 59h; only that receiver may consume its +2Ch/XYZ.
    require_local_director_route(queued->header.base.type_10, 0x5f, [](std::uint32_t type) {
        return native_session_message_is95_0071cc70(type);
    });
    return queued;
}

namespace {
template <class IsType>
bool copy_goes_to_unit_handler(IsType is_type);

// Goal 5 chain 719. bsp.py show 00780120: the decoded object's slot +0Ch
// selects the receiver before 0077FE80 switches on its actual +10h kind.
// The local codecs already retain the real receive allocation; admit its
// payload only through that same ladder and the expected kind-switch arm.
// Each borrowed predicate below is the existing whole address body (receiver
// unused), queried with the binary's ladder IDs, not an invented class tag.
template <class IsType>
void require_local_unit_route(std::uint8_t decoded_type, std::uint8_t expected_type,
    IsType is_type, const char* failure) {
    if (!copy_goes_to_unit_handler(is_type) || decoded_type != expected_type)
        throw std::logic_error(failure);
}
}  // namespace

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
    // 00769132..0076914D: allocate 3Ch, construct 0075FF60, then dispatch
    // vtable+8 (007600D0) at 0076A27B. Retain that allocation until the real
    // gun-control payload has been consumed below. Profile D032D0 selects
    // 007605F0: reset CE4974 and free iff flags&1, including on decode failure.
    OwnedDecodedMessage<seg42::VehicleGunControlMessage,
        delete_vehicle_gun_control_message_007605f0> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_gun_control_0075ff60(&queued, image.seg42);
    seg42::read_vehicle_gun_control_007600d0(&queued, &stream.in, image.numeric);
    require_local_unit_route(queued.h.base.type_10, 0x79, [](std::uint32_t type) {
        return seg42::is_vehicle_gun_control_0075ffd0(type);
    }, "GunControl: decoded copy did not reach the unit control arm");

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
    delete_vehicle_gun_control_message_007605f0(&sent, 0u);
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
    // Chain 626: retain the factory's actual 24h allocation through decode
    // and member-id consumption. The profile's scalar delete owns that block;
    // the stack sender takes the same delete with flags=0.
    const auto is_type = [join](std::uint32_t id) {
        return join ? seg42::is_formation_join_0075a310(id)
                    : seg42::is_formation_leave_0075a360(id);
    };
    auto decode = [&](seg42::FormationMemberMessage& queued) {
        if (join) {
            seg42::construct_for_read_formation_join_0075a2e0(&queued, image.seg42);
            seg42::read_formation_join_0075c810(&queued, &stream.in);
        } else {
            seg42::construct_for_read_formation_leave_0075a330(&queued, image.seg42);
            seg42::read_formation_leave_0075c8b0(&queued, &stream.in);
        }
        // 00780120 queries this message's actual IsType slot before the
        // session-kind switch consumes its member id. Both formation classes
        // answer their own tag, 49h and 46h (bsp.py show 0075A310/0075A360).
        if (!copy_goes_to_unit_handler(is_type) || !is_type(queued.u.h.base.type_10))
            throw std::logic_error("Formation: decoded copy did not reach the unit formation arm");
        return queued.member_id_20;
    };
    std::uint16_t member;
    if (join) {
        OwnedDecodedMessage<seg42::FormationMemberMessage,
            delete_formation_join_message_0075ca20> owner;
        member = decode(owner.get());
        delete_formation_join_message_0075ca20(&sent, 0u);
    } else {
        OwnedDecodedMessage<seg42::FormationMemberMessage,
            delete_formation_leave_message_0075ca40> owner;
        member = decode(owner.get());
        delete_formation_leave_message_0075ca40(&sent, 0u);
    }
    // 0076E5B2 then stores the sender's +174h at the copy's +18h; the hosts
    // deliver to the posting unit directly, so that store is not repeated.
    return member;
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
    // 00768530 allocates 20h at 007691B2 before 00760260. Release that
    // decoded allocation with 00760610(1), after retaining the copied party.
    OwnedDecodedMessage<seg42::VehicleValueMessage,
        delete_vehicle_set_party_message_00760610> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_set_party_00760260(&queued, image.seg42);
    seg42::read_vehicle_set_party_00760340(&queued, &stream.in);
    require_local_unit_route(queued.h.base.type_10, 0x7f, [](std::uint32_t type) {
        return seg42::is_vehicle_set_party_007602c0(type);
    }, "SetParty: decoded copy did not reach the unit party arm");
    const auto copied_party = queued.value_1c;
    delete_vehicle_set_party_message_00760610(&sent, 0u);
    return copied_party;
}

void HostSessionMessages::bind_vehicle_race_owner(const SessionParticipantPools& participants,
    const std::size_t& local_slot) const noexcept {
    image_->vehicle_race_participants = &participants;
    image_->vehicle_race_local_slot = &local_slot;
}

std::uint32_t HostSessionMessages::queue_vehicle_race_80(
    const VehicleRaceFields& posted) const {
    Image& image = *image_;
    if (!image.vehicle_race_participants || !image.vehicle_race_local_slot)
        throw std::logic_error("SetRace: no live selected-participant source");
    if (posted.type_10 != 0x80 || posted.id_18 != 0 || posted.relay_1a != 0)
        throw std::logic_error("SetRace: posted header disagrees with 008A8720");
    NativeSessionMessageParticipantOwner selected(*image.vehicle_race_participants,
        *image.vehicle_race_local_slot);
    const seg42::Context live{image.current_game, image.seg42_vtables, &selected};
    // 008A8879..008A88B7 inlines 00760390's stores on the sender, then
    // supplies the requested race. Its +14h now comes from the real mission
    // pool/slot, without filling the legacy Image::game with copied records.
    seg42::VehicleValueMessage sent{};
    seg42::construct_vehicle_set_race_00760390(&sent, live);
    sent.value_1c = posted.race_1c;
    LocalStream stream;
    seg42::write_vehicle_set_race_00760420(&sent, &stream.out);
    stream.begin_read();
    // 00768530 case 80h: 007691D2 allocates 20h, 007691E6 calls 00760390,
    // then 0076A27B dispatches the reader. Keep that actual allocation until
    // the race has been copied for the unit callback; 00760630(1) frees it.
    static_assert(sizeof(seg42::VehicleValueMessage) == 0x20);
    OwnedDecodedMessage<seg42::VehicleValueMessage,
        delete_vehicle_set_race_message_00760630> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_set_race_00760390(&queued, live);
    seg42::read_vehicle_set_race_00760470(&queued, &stream.in);
    // 00780120's real IsType ladder must select the session kind switch,
    // whose 80h arm consumes this payload (0095ABE0 / unit_message_arms.cpp).
    if (!copy_goes_to_unit_handler([](std::uint32_t type) {
            return seg42::is_vehicle_set_race_007603f0(type);
        })
        || queued.h.base.type_10 != 0x80)
        throw std::logic_error("SetRace: decoded copy did not reach the unit race arm");
    const auto copied_race = queued.value_1c;
    delete_vehicle_set_race_message_00760630(&sent, 0u);
    return copied_race;
}

namespace {
// Goal 5 chain 630. The A5h class table, outside Image's seg42 set: bsp.py
// ghidra bytes 00CFF908 --length 20 (bsp.gpr, /battlestationspacific.exe)
// reads 0074A1B0 scalar delete, 00749E10 write, 00749E40 read, 00749DF0
// IsType, 004499C0. The sender 00749D90 and the factory's 0075AF00 both store
// it; neither runs a slot, so the table is only the stored profile.
constexpr std::uint32_t kLandAtPadSlots_00cff908[5]{0x0074a1b0u, 0x00749e10u, 0x00749e40u,
                                                    0x00749df0u, 0x004499c0u};
// bsp.py ghidra bytes 00CFF8D4 --length 20, bsp.gpr /battlestationspacific.exe:
// scalar delete, base header write/read, A6h IsType, the common final slot.
constexpr std::uint32_t kLandingShipOpenSlots_00cff8d4[5]{0x0074a1d0u, 0x0075b480u,
    0x0075b4c0u, 0x00749ad0u, 0x004499c0u};
struct LandingShipSenderProfile final : cc13_seg41::SessionMessageHost {
    const std::uint32_t* vtable(std::uint32_t image_address) override {
        switch (image_address) {
            case 0x00cff908u: return kLandAtPadSlots_00cff908;
            case 0x00cff8d4u: return kLandingShipOpenSlots_00cff8d4;
            default: throw std::logic_error("Landing ship sender stored an unexpected profile");
        }
    }
    // Neither sender constructor runs a header codec. A5h's 00749E10 /
    // 00749E40 call it directly; the local flags-4 A6h send never reaches it.
    void write_header_0075b480(const void*, NativeBitCursor*) override { unused(); }
    void read_header_0075b4c0(void*, NativeSessionReadStream&) override { unused(); }
    [[noreturn]] static void unused() {
        throw std::logic_error("Landing ship constructor has no header codec call");
    }
};
// bsp.py ghidra bytes 00CFF37C --length 20, bsp.gpr /battlestationspacific.exe:
// scalar delete, writer, reader, IsType, always true. Only the constructor
// profile binding is used: the flags-4 local route returns before a codec.
constexpr std::uint32_t kLandFortEffectSlots_00cff37c[5]{0x00744a00u, 0x00744920u,
    0x00744990u, 0x00744900u, 0x004499c0u};
struct LandFortEffectSenderProfile final : cc13_seg41::SessionMessageHost {
    const std::uint32_t* vtable(std::uint32_t image_address) override {
        if (image_address != 0x00cff37cu)
            throw std::logic_error("LandFort effect constructor stored an unexpected profile");
        return kLandFortEffectSlots_00cff37c;
    }
    void write_header_0075b480(const void*, NativeBitCursor*) override { unused(); }
    void read_header_0075b4c0(void*, NativeSessionReadStream&) override { unused(); }
    [[noreturn]] static void unused() {
        throw std::logic_error("LandFort effect sender constructor has no header codec call");
    }
};
struct LandAtPadReadProfile final : seg42::VtableBinding {
    const std::uint32_t* vtable(std::uint32_t image_address) const override {
        if (image_address != 0x00cff908u)
            throw std::logic_error("0075AF00 stored an unexpected profile");
        return kLandAtPadSlots_00cff908;
    }
};
// 00749D90 reads one word of each non-null entity, +174h (00749DBB / 00749DD2
// MOVZX word [EAX+174h]); nothing else of the object.
struct EntityId174View {
    std::uint8_t unread_000[0x174]{};
    std::uint16_t id_174{0};
};
static_assert(offsetof(EntityId174View, id_174) == 0x174);
}  // namespace

bool HostSessionMessages::deliver_land_at_pad_a5(std::uint16_t pad_id, std::uint16_t building_id,
    gameplay_seg41_cc13::LandingShipMessageHost& receiver) const {
    Image& image = *image_;
    // 009F2304..009F2311 / 0074A55F..0074A56B: 00749D90(stack message, pad,
    // building): 0075B430(A5h), +18h word 0, +4h = 1, +1Ah byte 0, vtable
    // 00CFF908, +1Ch / +1Eh the two +174h words.
    EntityId174View pad;
    pad.id_174 = pad_id;
    EntityId174View building;
    building.id_174 = building_id;
    LandingShipSenderProfile sender_profile;
    seg42::StartLandingMessage sent{};
    static_assert(sizeof(seg42::StartLandingMessage) == 0x20);
    cc13_seg41::land_at_pad_construct_00749d90(reinterpret_cast<std::uint8_t*>(&sent),
        reinterpret_cast<std::uint8_t*>(&pad), reinterpret_cast<std::uint8_t*>(&building),
        image.session, sender_profile);

    // 0076E575: vtable[4], 00749E10.
    LocalStream stream;
    cc13_seg41::land_at_pad_write_00749e10(reinterpret_cast<const std::uint8_t*>(&sent),
                                           &stream.out);
    // 0076E5A4 -> 00768530: the type read and rewind, then case A5h
    // (operator new(20h), 0075AF00 at 007696E8) and 0076A27B vtable[8],
    // 00749E40. The decoded copy is host storage here: its scalar delete
    // 0074A1B0 is the compiler's (library, out of scope) and the class owns
    // no payload subobjects.
    stream.begin_read();
    LandAtPadReadProfile read_profile;
    const seg42::Context read_context{image.current_game, read_profile};
    seg42::StartLandingMessage queued{};
    seg42::construct_for_read_landingship_startlanding_0075af00(&queued, read_context);
    cc13_seg41::land_at_pad_read_00749e40(reinterpret_cast<std::uint8_t*>(&queued), stream.in);

    // 00780120 over the copy's IsType, 00749DF0 {A5h, 46h}.
    const band_006b_cc13::TypeIdRule* const is_land_at_pad =
        band_0071_cc13::find_type_id_rule_0071(0x00749df0u);
    if (is_land_at_pad == nullptr || !copy_goes_to_unit_handler([is_land_at_pad](std::uint32_t type) {
            return band_006b_cc13::type_id_rule_accepts(*is_land_at_pad,
                static_cast<std::int32_t>(type));
        })) {
        return false;
    }
    // The session-kind switch: the ship's vtable[164h], 0074B570, over the
    // copy (+10h kind, +1Ch pad handle, +1Eh building handle; 0075AF00 names
    // them zero_1c / zero_1e for its zeroing stores).
    return gameplay_seg41_cc13::handle_message_0074b570(queued.h.base.type_10, queued.zero_1c,
                                                        queued.zero_1e, receiver);
}

bool HostSessionMessages::post_landing_ship_open_a6(std::int32_t game_state,
    std::int32_t session_mode, const SessionParticipantPools& participants,
    const std::size_t& local_slot) const {
    // Only the single-player sender is bound here. Preserve the existing
    // record for host/client transport rather than manufacture a peer send.
    if (session_mode != static_cast<std::int32_t>(SessionMode::kLocal)) return false;
    Image& image = *image_;
    seg42::HeaderedMessage sent{};
    LandingShipSenderProfile profile;
    NativeSessionMessageParticipantOwner selected(participants, local_slot);
    // The selected-participant view bypasses the legacy raw-game reference
    // in 0075B430. Only the canonical pool identity and the borrowed current
    // slot are read. No state is copied into Image::game.
    const NativeSessionMessageContext live_session{image.current_game, &selected};
    // bsp.py show 0074AF50: 0074B080 lowers the ramp BEFORE 00749AA0 and the
    // flags-4 route at 0074B0A0. The native 1Ch message has no payload or owned
    // subobjects. Keep its real constructor/context, not a synthesized header.
    cc13_seg41::landing_ship_open_construct_00749aa0(
        reinterpret_cast<std::uint8_t*>(&sent), live_session, profile);

    SessionRouteInputs route;
    route.session_object_present = true;  // The caller supplies its retained mission owner.
    route.game_state = game_state;
    route.mode = static_cast<SessionMode>(session_mode);
    route.flags_override = kRouteFlagToPeers;  // PUSH 4 at 0074B090
    route.default_flags = default_route_flags_for_mode(route.mode);
    // 0077C30D..0077C323 returns before any codec, queue, audit or transport
    // mutation. Its earlier discarded IsType(D3h) query cannot mutate this A6h
    // message. In particular, do not loop it back to 0074B570/0074A420.
    return !route_session_message_0077c2a0(route).routed;
}

bool HostSessionMessages::post_land_fort_death_smoke_d1(float duration,
    std::int32_t game_state, std::int32_t session_mode,
    const SessionParticipantPools& participants, const std::size_t& local_slot) const {
    if (session_mode != static_cast<std::int32_t>(SessionMode::kLocal)) return false;
    Image& image = *image_;
    NativeSessionMessageParticipantOwner selected(participants, local_slot);
    const NativeSessionMessageContext live_session{image.current_game, &selected};
    LandFortEffectSenderProfile profile;
    // bsp.py show 007465E0: the smoke duration written to fort+748h is passed
    // unchanged to 007448B0(7, duration) at 0074675A. The integer variant's
    // +20h is deliberately untouched by this constructor and never read here.
    cc13_seg41::LandFortEffectMessage sent;
    cc13_seg41::land_fort_effect_construct_point_007448b0(
        sent, 7u, duration, live_session, profile);

    SessionRouteInputs route;
    route.session_object_present = true; // Caller retains the bound mission/session owner.
    route.game_state = game_state;
    route.mode = static_cast<SessionMode>(session_mode);
    route.flags_override = kRouteFlagToPeers; // 00746761 PUSH 4, route at 0074676E.
    route.default_flags = default_route_flags_for_mode(route.mode);
    // 0077C30D..0077C323 returns before serialization, entity-id assignment,
    // the queue or a receiver. Re-applying kind 7 locally would start smoke twice.
    return !route_session_message_0077c2a0(route).routed;
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
    // 00768530 case 9Fh owns the received 20h allocation. Its actual
    // 00D03360 slot0 (00760BF0, flags1) releases it once after projection.
    static_assert(sizeof(seg42::RepairBodyMessage) == 0x20);
    OwnedDecodedMessage<seg42::RepairBodyMessage,
        seg42::delete_shiprepair_bodyrepair_00760bf0> owner;
    auto& queued = owner.get();
    seg42::construct_shiprepair_bodyrepair_00760a50(&queued, image.seg42);
    seg42::read_shiprepair_bodyrepair_00760b30(&queued, &stream.in);
    require_local_unit_route(queued.h.base.type_10, 0x9f, [](std::uint32_t type) {
        return seg42::is_shiprepair_bodyrepair_00760ab0(type);
    }, "BodyRepair: decoded copy did not reach the unit repair arm");
    return queued.flag_1c != 0;
}

HostSessionMessages::ShipLeakCopy HostSessionMessages::queue_ship_leak_90(
    std::uint32_t count, const float (&point)[3]) const {
    const Image& image = *image_;
    // 0080FF80: 0075B430(90h), +18h word 0, +1Ah 0, +1Ch = value, +4h = 1,
    // vtable 00D034A0, +20h..+28h the point; 00761F60 is those stores
    // without the two arguments.
    seg42::ShipLeakMessage sent{};
    {
        UnitMessageConstructionBinding binding(image.session);
        cc13_sweepC::unit_message_hull_impact_construct_0080ff80(binding, &sent, count, point);
        // Relocate only the body's original vptr 00D034A0 to the existing host profile.
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d034a0u);
    }

    LocalStream stream;
    seg42::write_ship_leak_00761ff0(&sent, &stream.out, image.numeric);
    // 00768530 case 90h: operator new(2Ch), 00761F60 at 0076946E; 0076A27B
    // vtable[8] = 00762090.
    stream.begin_read();
    // The decoder owns this 2Ch block through extraction of the live leak
    // count and point. 00D034A0 slot0 00762660 frees that same block once.
    static_assert(sizeof(seg42::ShipLeakMessage) == 0x2c);
    OwnedDecodedMessage<seg42::ShipLeakMessage, seg42::delete_ship_leak_00762660> owner;
    auto& queued = owner.get();
    seg42::construct_ship_leak_00761f60(&queued, image.seg42);
    seg42::read_ship_leak_00762090(&queued, &stream.in, image.numeric);
    require_local_unit_route(queued.h.base.type_10, 0x90, [](std::uint32_t type) {
        return seg42::is_ship_leak_00761fc0(type);
    }, "ShipLeak: decoded copy did not reach the unit leak arm");
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
    // 00768530's 8Fh receive allocation is 24h. Retain it through the
    // signed-side projection, then release with its 00D034DC slot0.
    static_assert(sizeof(seg42::ShipAvoidSideMessage) == 0x24);
    OwnedDecodedMessage<seg42::ShipAvoidSideMessage,
        seg42::delete_ship_avoidside_007626c0> owner;
    auto& queued = owner.get();
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
    // Chain 682: retain 00768530's actual 28h allocation through decode and
    // dispatch, then release it with this class's 00763240(1), exactly once.
    static_assert(sizeof(seg42::AirbaseLaunchSlotMessage) == 0x28);
    OwnedDecodedMessage<seg42::AirbaseLaunchSlotMessage,
        seg42::delete_airbase_launchslot_00763240> owner;
    auto& queued = owner.get();
    seg42::construct_airbase_launchslot_007631a0(&queued, image.seg42);
    seg42::read_airbase_launchslot_007632c0(&queued, &stream.in);
    // 00780120 uses the decoded message's +0Ch predicate against its
    // receiver ladder before 0077FE80 forwards 83h to the air-ops switch.
    if (!copy_goes_to_unit_handler(&seg42::is_airbase_launchslot_00763210)
        || !seg42::is_airbase_launchslot_00763210(queued.u.h.base.type_10)) {
        throw std::logic_error("AirbaseLaunchSlot: decoded copy did not reach the kind switch");
    }
    seg42::delete_airbase_launchslot_00763240(&sent, 0u);
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
    // 00769B70..00769B84 allocates 24h, then runs the same constructor.
    // Keep that real decoded object until its payload has been copied;
    // 00762920(1) then frees it once. The stack sender takes flags=0.
    OwnedDecodedMessage<seg42::PlaneChangeStateMessage,
        delete_plane_change_state_message_00762920> owner;
    auto& queued = owner.get();
    seg42::construct_plane_change_state_00762880(&queued, image.seg42);
    seg42::read_plane_change_state_007629a0(&queued, &stream.in);
    require_local_unit_route(queued.u.h.base.type_10, 0xc3, [](std::uint32_t type) {
        return seg42::is_plane_change_state_007628f0(type);
    }, "PlaneChangeState: decoded copy did not reach the plane state arm");
    const PlaneChangeStateCopy copy{queued.state_20, queued.state_21};
    delete_plane_change_state_message_00762920(&sent, 0u);
    return copy;
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
    OwnedDecodedMessage<seg42::UnitMessage,
        delete_plane_touchdown_message_00762b00> owner;
    auto& queued = owner.get();
    seg42::construct_plane_touchdown_00762a60(&queued, image.seg42);
    cc13_seg5::session_bool_message_read_004645d0(codec,
        reinterpret_cast<cc13_seg5::Object>(&queued),
        reinterpret_cast<cc13_seg5::Object>(&stream.in));
    require_local_unit_route(queued.h.base.type_10, 0xc5, [](std::uint32_t type) {
        return seg42::is_plane_touchdown_00762ad0(type);
    }, "PlaneTouchdown: decoded copy did not reach the plane touchdown arm");
    const auto type = queued.h.base.type_10;
    delete_plane_touchdown_message_00762b00(&sent, 0u);
    return type; // 00762B00(1) releases the factory's 20h allocation.
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
    OwnedDecodedMessage<NativeSessionMessage90, delete_native_session_message90_0071e370> owner;
    NativeSessionMessage90& queued = owner.get();
    construct_native_session_message90_0075af60(&queued, image.context, image.profile90);
    read_native_session_message90_0071c6c0(&queued, &stream.in, image.context);
    // 0071C640's class predicate selects 00780120's 59h director receiver.
    require_local_director_route(queued.header.base.type_10, 0x5a, [](std::uint32_t type) {
        return native_session_message_is90_0071c640(type);
    });
    const DirectorSubKindCopy copy{queued.selector_20, queued.value_24};
    destroy_native_session_message90_0071c720(&sent);
    return copy; // owner runs 0071E370(1); the sender stays a stack object.
}

HostSessionMessages::ClearCommandCopy HostSessionMessages::queue_clear_command_5d(
    std::uint8_t arm, std::int32_t index) const {
    const Image& image = *image_;
    // 0071C730..0071C75E: 0075B430(5Dh), +18h word 0, +1Ah 0, +1Ch 0, +20h =
    // the arm byte, +4h = 1, vtable 00CFD9D8 (profile93), +24h = the index.
    NativeSessionMessage93 sent{};
    {
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
    }

    LocalStream stream;
    write_native_session_message93_0071c7a0(&sent, &stream.out, image.context);
    // 00768530 case 5Dh: operator new(28h), 0075B030 at 00769D44; 0076A27B
    // vtable[8] = 0071C7E0 (00428D30 sign-extends the 5-bit index).
    stream.begin_read();
    OwnedDecodedMessage<NativeSessionMessage93, delete_native_session_message93_0071e390> owner;
    NativeSessionMessage93& queued = owner.get();
    construct_native_session_message93_0075b030(&queued, image.context, image.profile93);
    read_native_session_message93_0071c7e0(&queued, &stream.in, image.context);
    // 0071C770's class predicate selects 00780120's 59h director receiver.
    require_local_director_route(queued.header.base.type_10, 0x5d, [](std::uint32_t type) {
        return native_session_message_is93_0071c770(type);
    });
    const ClearCommandCopy copy{queued.flag_20, queued.value_24};
    destroy_native_session_message93_0071c820(&sent);
    return copy; // owner runs 0071E390(1); no second queued-object teardown.
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
    {
        UnitMessageConstructionBinding binding(image.session);
        // 0080FFD0 copies stack DWORDs; preserve the caller's three float bit patterns.
        std::uint32_t bits[3];
        std::memcpy(bits, torque, sizeof bits);
        cc13_sweepC::unit_message_add_hull_torque_construct_0080ffd0(
            binding, &sent, bits[0], bits[1], bits[2]);
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d034c8u);
    }

    LocalStream stream;
    seg42::write_ship_addtorque_007622b0(&sent, &stream.out, image.numeric);
    // 00768530 case 93h: operator new(28h), 00762220 at 0076942E; 0076A27B
    // vtable[8] = 00762340.
    stream.begin_read();
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
    require_local_unit_route(queued.h.base.type_10, 0x93, [](std::uint32_t type) {
        return seg42::is_ship_addtorque_00762280(type);
    }, "ShipAddTorque: decoded copy did not reach the unit torque arm");
    ShipAddTorqueCopy copy;
    for (int i = 0; i < 3; ++i) copy.torque[i] = queued.torque_1c[i];
    return copy;
}

HostSessionMessages::KamikazeDetonateCopy HostSessionMessages::queue_kamikaze_detonate_70(
    std::uint16_t other_id, const float (&point)[3]) const {
    const Image& image = *image_;
    // 0080FF30: 0075B430(70h), +18h word 0, +1Ah 0, +4h = 1, vtable 00D0348C,
    // +1Ch word = other+174h, +20h..+28h = the point (FLD/FSTP copies).
    // 00761D90 makes the header stores (+1Ch cleared); the sent message is
    // 00761D90 with the id and the point.
    NativeSessionMessage112 sent{};
    {
        UnitMessageConstructionBinding binding(image.session);
        // Only entity+174h is read (bsp.py show 0080FF30). Project the supplied
        // real target handle; no other entity bytes are initialized or consumed.
        alignas(4) std::uint8_t entity_id_view[0x176];
        std::memcpy(entity_id_view + 0x174, &other_id, sizeof other_id);
        cc13_sweepC::unit_message_kamikaze_detonate_construct_0080ff30(
            binding, &sent, entity_id_view, point);
        sent.base.profile_00 = image.profile112.slots;
    }

    LocalStream stream;
    write_native_session_message112_00761e20(&sent, &stream.out, image.context112);
    // 00768530 case 70h: 00761D90 at 00769026; 0076A27B vtable[8] = 00761EC0.
    stream.begin_read();
    // 00768530 allocates the decoded 2Ch message; keep that allocation
    // through payload consumption. bsp.py show 00762640: stamp CE4974,
    // free with 00BF65AC iff flags&1. The existing owner uses its matching
    // allocator and runs that body once, including on a decode exception.
    OwnedDecodedMessage<NativeSessionMessage112,
        delete_native_session_message112_00762640> owner;
    NativeSessionMessage112& queued = owner.get();
    construct_native_session_message112_00761d90(&queued, image.context112, image.profile112);
    read_native_session_message112_00761ec0(&queued, &stream.in, image.context112);
    require_local_unit_route(queued.base.type_10, 0x70, [](std::uint32_t type) {
        return native_session_message_is112_00761df0(type);
    }, "KamikazeDetonate: decoded copy did not reach the unit detonation arm");
    KamikazeDetonateCopy copy;
    copy.other_id = queued.value_1c;
    for (int i = 0; i < 3; ++i) {
        std::memcpy(&copy.point[i], &queued.float_bits_20[i], sizeof copy.point[i]);
    }
    destroy_native_session_message112_00761e10(&sent);
    return copy; // owner frees only the decoded object; sender remains on stack.
}

HostSessionMessages::RepairAddDamageCopy HostSessionMessages::queue_repair_add_damage_9e(
    std::int32_t channel, float seconds, bool add) const {
    const Image& image = *image_;
    // 0080FA50 (00827372 water, 0082740F fire): 0075B430(9Eh), +18h word 0,
    // +1Ah 0, +20h = the flag byte, +4h = 1, vtable 00D0334C, +1Ch = the
    // seconds, +24h = the channel. 007608E0 is the header stores.
    seg42::RepairAddDamageMessage sent{};
    {
        UnitMessageConstructionBinding binding(image.session);
        cc13_sweepC::unit_message_fire_or_water_damage_construct_0080fa50(
            binding, &sent, channel, seconds, add ? 1u : 0u);
        sent.h.base.profile_00 = image.seg42.vtables.vtable(0x00d0334cu);
    }

    LocalStream stream;
    seg42::write_shiprepair_adddamage_00760970(&sent, &stream.out, image.numeric);
    // 00768530 case 9Eh: 007608E0 at 007695EE; 0076A27B vtable[8] = 007609E0
    // (00428D30 sign-extends the 3-bit channel).
    stream.begin_read();
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
    require_local_unit_route(queued.h.base.type_10, 0x9e, [](std::uint32_t type) {
        return seg42::is_shiprepair_adddamage_00760940(type);
    }, "RepairAddDamage: decoded copy did not reach the unit repair-damage arm");
    return RepairAddDamageCopy{queued.section_24, queued.amount_1c, queued.flag_20 != 0};
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
        // The sender remains stack-owned; the decoded owner alone
        // runs the scalar-deleting path. Neither object is run twice.
        destroy_native_session_message107_00761950(&sent);
    } else {
        // 00827D65 / 00813530: 007619B0 itself.
        NativeSessionMessage108 sent{};
        construct_native_session_message108_007619b0(&sent, image.context98, image.profile108);
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        // 00768530 case 6Ch likewise allocates 24h for 007619B0;
        // 00761A60 flags=1 releases the actual decoded message once.
        OwnedDecodedMessage<NativeSessionMessage108,
            delete_native_session_message108_00761a60> owner;
        auto& queued = owner.get();
        construct_native_session_message108_007619b0(&queued, image.context98, image.profile108);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        copy.type = queued.header.base.type_10;
        answer(&native_session_message_is108_00761a30);
        destroy_native_session_message108_00761a20(&sent);
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
    return local_message_receiver(is_type) == EntityMessageReceiver::kSessionKindSwitch;
}
}  // namespace

namespace {
// Goal 5 chain 628. The 86h class: the 20h-byte unit message and the plane's
// handle word at +20h, 24h bytes as 00768530 case 86h allocates it
// (include/bsp/oct18_tail_a_factory.inc: {0x86, 0x24, 0x00763440, ...}).
struct AppearReadyPlaneMessage86 {
    seg42::UnitMessage u;
    std::uint16_t plane_id_20;
    std::uint16_t pad_22;
};
static_assert(sizeof(AppearReadyPlaneMessage86) == 0x24);
static_assert(offsetof(AppearReadyPlaneMessage86, plane_id_20) == 0x20);

// 006BDAC0 reads one word of the plane (bsp.py show 006bdac0 --asm: 006BDAEA
// MOV CX,[EAX+174h], the handle-table ordinal); nothing else of the plane is
// read, so this is that word's view.
struct PlaneHandleView174 {
    std::uint8_t unread_000[0x174];
    std::uint16_t handle_174;
};
static_assert(offsetof(PlaneHandleView174, handle_174) == 0x174);

// 006BDAC0's only service is the inlined base constructor 0075B430
// (006BDAC8), over the session owner the other local codecs use; every other
// segment-29 service traps.
class AppearReadyPlaneConstructionBinding final : public cc13_seg29U::Seg29UTrapHost {
public:
    explicit AppearReadyPlaneConstructionBinding(const NativeSessionMessageContext& session)
        : session_(session) {}
    void session_message_construct_base_0075b430(void* message, std::uint32_t type) override {
        construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), type, session_);
    }
private:
    const NativeSessionMessageContext& session_;
};
}  // namespace

std::uint16_t HostSessionMessages::queue_airbase_appear_ready_plane_86(
    std::uint16_t plane_id) const {
    const Image& image = *image_;
    // 006CF9AD..006CF9B2: 006BDAC0(plane) on the stack message at [ESP+8].
    AppearReadyPlaneMessage86 sent{};
    {
        PlaneHandleView174 plane{};
        plane.handle_174 = plane_id;
        AppearReadyPlaneConstructionBinding binding(image.session);
        cc13_seg29U::appear_ready_plane_message_construct_006bdac0(&sent, &plane, binding);
    }

    // 0076E575: vtable[4] = 006BDB30, the header, the +1Ch bit, +20h in 12 bits.
    LocalStream stream;
    band_006b_cc13::airbase_type86_write_006bdb30(&sent, &stream.out);
    // 00768530 case 86h: operator new(24h), 00763440 at 007692A6; 0076A27B
    // vtable[8] = 006BDB60.
    stream.begin_read();
    // Chain 682: 00768530's 24h allocation remains alive until its decoded
    // payload has been copied. Slot 0, 006BEDF0(1), owns final release.
    void* const block = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, sizeof(AppearReadyPlaneMessage86),
         sizeof(AppearReadyPlaneMessage86)});
    auto* const decoded = ::new (block) AppearReadyPlaneMessage86{};
    struct Release {
        AppearReadyPlaneMessage86* message;
        ~Release() {
            band_006b_cc13::airbase_appear_ready_plane_delete_006bedf0(
                &message->u.h.base, 1u);
        }
    } release{decoded};
    auto& queued = *decoded;
    seg42::construct_airbase_appearreadyplane_00763440(&queued.u, image.seg42);
    band_006b_cc13::airbase_type86_read_006bdb60(&queued, &stream.in);
    // 00780120 asks the copy's IsType 006BDB00 (86h, 49h, 46h): none is on its
    // ladder, so the copy takes the shared tail to the entity's session-kind
    // switch, whose air-ops arm 006CD788 reads +20h.
    if (!copy_goes_to_unit_handler(&band_006b_cc13::airbase_type86_is_type_006bdb00)
        || !band_006b_cc13::airbase_type86_is_type_006bdb00(queued.u.h.base.type_10)) {
        throw std::logic_error("AppearReadyPlane: decoded copy did not reach the kind switch");
    }
    // The sender belongs to this stack frame; only the decoded copy is freed.
    band_006b_cc13::airbase_appear_ready_plane_delete_006bedf0(&sent.u.h.base, 0u);
    return queued.plane_id_20;
}

namespace {
// Goal 5 chain 628. The 89h class: the 20h-byte unit message and the slot
// dword at +20h, 24h bytes (006BC8F5 SUB ESP,24h; 00768530 case 89h
// allocates 24h, include/bsp/oct18_tail_a_factory.inc).
struct DoLaunchSlotMessage89 {
    seg42::UnitMessage u;
    std::uint32_t slot_20;
};
static_assert(sizeof(DoLaunchSlotMessage89) == 0x24);
static_assert(offsetof(DoLaunchSlotMessage89, slot_20) == 0x20);

// 006BC8E0 reads one word of the block, the router at +7Ch (006BC937 MOV
// ECX,[ESI+7Ch], the owner entity 0077C2A0 routes to). LABELLED: this process
// delivers to the deck its caller served, so the router is not read past the
// route below and the view carries none.
struct AirOpsBlockRouterView {
    std::uint8_t unread_00[0x7c];
    void* router_7c;
};
static_assert(offsetof(AirOpsBlockRouterView, router_7c) == 0x7c);

// 006BC8E0's two services: 0075B430 (006BC904) and 0077C2A0 (006BC93F). The
// route runs 0077C44D's local copy, as 0076E520 does: the class writer, the
// factory's 89h constructor 006BC3E0 (which takes the same 0075B430) and the
// class reader. Every other segment-29 service traps.
class DoLaunchSlotRouteBinding final : public cc13_seg29U::Seg29UTrapHost {
public:
    explicit DoLaunchSlotRouteBinding(const NativeSessionMessageContext& session)
        : session_(session) {}
    void session_message_construct_base_0075b430(void* message, std::uint32_t type) override {
        construct_native_session_message_0075b430(
            static_cast<NativeSessionMessageStorage*>(message), type, session_);
    }
    void session_route_message_0077c2a0(void*, void* message, std::uint32_t flags,
                                        std::uint32_t) override {
        // 006BC92D PUSH 1: the local bit, so 0077C43A..0077C45A take 0076E520.
        if ((flags & 1u) == 0u) {
            throw std::logic_error("DoLaunchSlot: 006BC8E0 routed without the local bit");
        }
        // 0076E575: vtable[4] = 006BC440, the header, +1Ch bit, +20h in 8 bits.
        LocalStream stream;
        band_006b_cc13::airbase_do_launch_slot_write_006bc440(message, &stream.out);
        // 00768530 case 89h: operator new(24h), 006BC3E0 at 00769306; 0076A27B
        // vtable[8] = 006BC470.
        stream.begin_read();
        DoLaunchSlotMessage89 queued{};
        cc13_seg29U::do_launch_slot_message_construct_006bc3e0(&queued, *this);
        band_006b_cc13::airbase_do_launch_slot_read_006bc470(&queued, &stream.in);
        copy_type = queued.u.h.base.type_10;
        copy_slot = queued.slot_20;
        routed = true;
        // 006BEDB0, slot 0 of 00CF85F0 for both, only restamps the root
        // profile and frees for flags bit 0: neither message owns storage.
    }
    bool routed{false};
    std::uint8_t copy_type{0};
    std::uint32_t copy_slot{0};
private:
    const NativeSessionMessageContext& session_;
};
}  // namespace

std::uint32_t HostSessionMessages::queue_airbase_do_launch_slot_89(std::uint32_t slot) const {
    const Image& image = *image_;
    AirOpsBlockRouterView block{};
    DoLaunchSlotRouteBinding binding(image.session);
    cc13_seg29U::air_ops_route_do_launch_slot_006bc8e0(&block, slot, binding);
    // 00780120 asks the copy's IsType 006BC410 (89h, 49h, 46h; the table rule
    // band_006b_cc13 keeps for it): none is on its ladder, so the copy takes
    // the shared tail to the owner's session-kind switch, case 8 of 006CD6C0.
    const band_006b_cc13::TypeIdRule* const is_type =
        band_006b_cc13::find_type_id_rule(0x006BC410u);
    if (!binding.routed || binding.copy_type != 0x89 || is_type == nullptr
        || !band_006b_cc13::type_id_rule_accepts(*is_type, binding.copy_type)
        || !copy_goes_to_unit_handler([is_type](std::uint8_t id) {
               return band_006b_cc13::type_id_rule_accepts(*is_type, id);
           })) {
        throw std::logic_error("DoLaunchSlot: decoded copy did not reach the kind switch");
    }
    return binding.copy_slot;
}

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
    {
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
    }

    LocalStream stream;
    seg42::write_ship_explodeonepart_007614f0(&sent, &stream.out);
    // 00768530 case 99h: 00761450 at 0076954E; 0076A27B vtable[8] = 00761540.
    stream.begin_read();
    // 00768530 case 99h allocates 24h. The receive owner keeps that block
    // through routing and segment-index extraction, then calls 00762600(1).
    static_assert(sizeof(seg42::ExplodeOnePartMessage) == 0x24);
    OwnedDecodedMessage<seg42::ExplodeOnePartMessage,
        seg42::delete_ship_explodeonepart_00762600> owner;
    auto& queued = owner.get();
    seg42::construct_ship_explodeonepart_00761450(&queued, image.seg42);
    seg42::read_ship_explodeonepart_00761540(&queued, &stream.in);
    ExplodeOnePartCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.to_unit = copy_goes_to_unit_handler(&seg42::is_ship_explodeonepart_007614b0);
    copy.explode_one_part_99 = copy.to_unit && copy.type == 0x99;
    copy.part = queued.part_20;
    return copy;
}

namespace {
// Goal 4 chain 343. The boundaries of the table-driven segment 26/27 message
// classes (ingame_gui_cc13), bound to the process's header and bit-cursor
// bodies. Write calls receive the cursor itself, read calls stream+4. Goal 5
// chain 636: also the 82h class's (seg27_cc13), which adds the 12-bit word
// write 00429120.
struct SessionMessageClassCodec final : seg27_cc13::SessionMessage130Host {
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
    void write_word_bits_00429120(void* cursor, std::uint16_t value, int bits) override {
        write_native_word_bits_00429120(static_cast<NativeBitCursor*>(cursor), value,
            static_cast<std::uint32_t>(bits));
    }
    // Only the scalar deleting destructors free a block (flags bit 0, 00BF65AC);
    // the one that does here is 00656420 on the 82h copy OwnedDecodedMessage
    // took from singleton_lifetime_allocate, the matching pair.
    void free_block(void* block) override { singleton_lifetime_free(block); }

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
// Goal 5 chain 636. The 82h class: the 20h-byte unit message (header, +1Ch
// bool) and its four payload fields, 30h bytes as 00768530 case 82h allocates
// it (00769212 PUSH 30h). 00656280 / 00656320 / 006563A0 name the offsets.
struct AirbaseSlotOrderMessage82 {
    seg42::UnitMessage u;
    std::uint32_t slot_20{0};
    std::uint32_t order_24{0};
    std::uint16_t target_28{0};
    std::uint16_t unread_2a{0};
    std::uint32_t player_2c{0};
};
static_assert(sizeof(seg42::UnitMessage) == 0x20);
static_assert(offsetof(AirbaseSlotOrderMessage82, slot_20) == 0x20);
static_assert(offsetof(AirbaseSlotOrderMessage82, target_28) == 0x28);
static_assert(offsetof(AirbaseSlotOrderMessage82, player_2c) == 0x2c);
static_assert(sizeof(AirbaseSlotOrderMessage82) == 0x30);
static_assert(offsetof(NativeSessionReadStream, cursor_04) == 4);
}  // namespace

HostSessionMessages::AirbaseSlotOrderCopy HostSessionMessages::queue_airbase_slot_order_82(
    std::uint32_t slot, std::uint32_t order, std::uint16_t target_id) const {
    Image& image = *image_;
    SessionMessageClassCodec codec(image.session);
    // The sender: 00656280 on the support manager's stack storage (006748B9
    // LEA ECX,[ESP+164h]), with the target entity, of whose object only +174h
    // is read (006562BC MOVZX word [ECX+174h], 0 for null), and the local
    // player slot [[00E188A8]+18ECh] (0067489A..0067489F), which this image's
    // game object holds. The first stack argument is unread.
    EntityId174View target;
    target.id_174 = target_id;
    std::uint32_t player = 0;
    std::memcpy(&player, image.game + 0x18ec, sizeof player);
    AirbaseSlotOrderMessage82 sent{};
    seg27_cc13::session_message130_construct_00656280(
        seg27_cc13::Raw{reinterpret_cast<std::uint8_t*>(&sent)}, 0u, slot, order,
        target_id != 0 ? &target : nullptr, player, codec);

    // 0077C2A0's local bit (see the header): 0076E520 writes the message
    // through its vtable[4], 00656320, into the cleared stream.
    LocalStream stream;
    seg27_cc13::session_message130_write_00656320(
        seg27_cc13::Raw{reinterpret_cast<std::uint8_t*>(&sent)}, &stream.out, codec);
    stream.begin_read();

    // 00768530 case 82h: operator new(30h), 00763320 at 00769226, then
    // 0076A27B vtable[8] = 006563A0 over the stream (fields at stream+4h).
    // The receive side releases the copy with its vtable[0](1), 00656420.
    void* const block = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, sizeof(AirbaseSlotOrderMessage82),
         sizeof(AirbaseSlotOrderMessage82)});
    auto* const queued = ::new (block) AirbaseSlotOrderMessage82{};
    struct Release {
        AirbaseSlotOrderMessage82* message;
        SessionMessageClassCodec& codec;
        ~Release() {
            ingame_gui_cc13::session_message_scalar_delete(
                ingame_gui_cc13::Raw{reinterpret_cast<std::uint8_t*>(message)}, 1u, codec);
        }
    } release{queued, codec};
    seg42::construct_airbase_changeslotstatus_00763320(&queued->u, image.seg42);
    ingame_gui_cc13::session_message130_read_006563a0(
        ingame_gui_cc13::Raw{reinterpret_cast<std::uint8_t*>(queued)}, &stream.in, codec);

    AirbaseSlotOrderCopy copy;
    copy.type = queued->u.h.base.type_10;
    copy.slot = queued->slot_20;
    copy.order = queued->order_24;
    copy.target_id = queued->target_28;
    copy.player = queued->player_2c;
    // 00780120's ladder over the copy's IsType 006562F0 {82h, 49h, 46h}: none
    // is on the ladder, so the copy takes the owner's session-kind switch
    // (006CD6C0 for an air-ops block; its 82h arm is 006CD160).
    const ingame_gui_cc13::SessionMessageClass* const cls =
        ingame_gui_cc13::find_session_message_class(0x82);
    copy.to_unit = cls != nullptr && copy.type == 0x82
        && copy_goes_to_unit_handler([cls](std::uint32_t type) {
               return ingame_gui_cc13::session_message_is_type(*cls,
                   static_cast<std::int32_t>(type));
           });
    // The sender is the support manager's stack object; 00673A10 / 00675C40
    // run no destructor on it (pool_pool21_cc13.cpp launch_from_deck).
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

namespace {
// 006CFFF0 reads one word of the site, the carrier at +B8h (006D001E MOV
// ECX,[ESI+0B8h], the entity 0077C2A0 routes to). LABELLED: this process
// delivers to the deck its caller ticks, so the carrier is not read past the
// route below and the view carries none.
struct MotherShipSiteCarrierView {
    std::uint8_t unread_00[0xb8];
    void* carrier_b8;
};
static_assert(offsetof(MotherShipSiteCarrierView, carrier_b8) == 0xb8);

// 006CFFF0's two services: 00758B90 (006D0019) and 0077C2A0 (006D0031). The
// route runs 0077C44D's local copy, as 0076E520 does: the class writer, the
// factory's 87h constructor 00763500 and the class reader. Every other
// SweepCHost service traps.
class ElevatorMoveRouteBinding final : public cc13_sweepC::SweepCTrapHost {
public:
    ElevatorMoveRouteBinding(const NativeSessionMessageContext& session,
                             const seg42::Context& seg42, Seg41MessageHost& seg41)
        : session_(session), seg42_(seg42), seg41_(seg41) {}
    void* elevator_message_construct_00758b90(void* message, std::uint32_t flag,
                                              void* plane) override {
        // 006D0013..006D0019 PUSH EAX (flag), PUSH ECX (plane): 00758B90(plane, flag).
        return cc13_seg41::elevator_move_construct_00758b90(static_cast<std::uint8_t*>(message),
            static_cast<std::uint8_t*>(plane), static_cast<std::uint8_t>(flag), session_, seg41_);
    }
    void session_route_message_0077c2a0(void*, void* message, std::uint32_t channel,
                                        std::uint32_t) override {
        // 006D0026 PUSH 5: the local bit, so 0077C43A..0077C45A take 0076E520.
        if ((channel & 1u) == 0u) {
            throw std::logic_error("ElevatorMove: 006CFFF0 routed without the local bit");
        }
        // 0076E575: vtable[4] = 00758C20, the header, the +1Ch/+22h/+23h bits
        // and +20h in 12 bits when +23h.
        LocalStream stream;
        cc13_seg41::elevator_move_write_00758c20(static_cast<const std::uint8_t*>(message),
                                                 &stream.out, seg41_);
        // 00768530 case 87h: operator new(24h), 00763500 at 007692C6; 0076A27B
        // vtable[8] = 00758C70 (which zeroes +20h without +23h).
        stream.begin_read();
        // Retain the factory's actual 24h allocation until the local consumer
        // has copied its decoded fields. 00758CD0 stamps the root and frees
        // this allocation once; the sender is the non-owning frame buffer of
        // 006CFFF0, whose normal return has no destructor call.
        OwnedDecodedMessage<ElevatorMoveSessionMessage,
            delete_elevator_move_message_00758cd0> decoded;
        auto& queued = decoded.get();
        seg42::construct_mothership_elevator_move_00763500(
            &queued.unit, seg42_);
        cc13_seg41::elevator_move_read_00758c70(
            reinterpret_cast<std::uint8_t*>(&queued), stream.in, seg41_);
        type = queued.unit.h.base.type_10;
        copy.flag = queued.flag_22 != 0;
        copy.has_plane = queued.has_plane_23 != 0;
        copy.plane_id = queued.plane_id_20;
        routed = true;
    }
    bool routed{false};
    std::uint8_t type{0};
    HostSessionMessages::ElevatorMoveCopy copy;
private:
    const NativeSessionMessageContext& session_;
    const seg42::Context& seg42_;
    Seg41MessageHost& seg41_;
};
}  // namespace

HostSessionMessages::ElevatorMoveCopy HostSessionMessages::queue_mothership_elevator_move_87(
    bool flag, bool has_plane, std::uint16_t plane_id) const {
    const Image& image = *image_;
    Seg41MessageHost seg41(image.seg42_vtables);
    ElevatorMoveRouteBinding binding(image.session, image.seg42, seg41);
    // 00758B90 reads only the plane's +174h word (id_or_zero) and tests it for
    // null (the +23h presence byte).
    PlaneHandleView174 plane{};
    plane.handle_174 = plane_id;
    MotherShipSiteCarrierView site{};
    cc13_sweepC::mothership_site_route_elevator_message_006cfff0(binding, &site,
        flag ? 1u : 0u, has_plane ? static_cast<void*>(&plane) : nullptr);
    // 00780120 asks IsType 00758BF0 (87h, 49h, 46h): none is on its ladder,
    // so the copy reaches the carrier's kind switch, whose arm is 006D0050.
    if (!binding.routed || binding.type != 0x87
        || !copy_goes_to_unit_handler(&cc13_seg41::elevator_move_is_type_00758bf0)) {
        throw std::logic_error("ElevatorMove: decoded copy did not reach the kind switch");
    }
    return binding.copy;
}

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
    // 009CFBFB..009CFC23 inline the 0080F960 value constructor. Run that
    // existing body over the sender and this session's actual player state.
    // Evidence: bsp.py show 0080F960. Relocate only its original vptr.
    seg42::VehicleShipyardLaunchMessage sent{};
    UnitMessageConstructionBinding binding(image.session);
    cc13_sweepC::unit_message_shipyard_launch_construct_0080f960(binding, &sent, flag);
    sent.u.h.base.profile_00 = image.seg42.vtables.vtable(0x00d03310u);

    LocalStream stream;
    seg42::write_vehicle_shipyard_launch_00760550(&sent, &stream.out);
    // 00768530 case 7Ah: 007604C0 at 00769166; 0076A27B vtable[8] = 007605A0.
    stream.begin_read();
    // The 7Ah factory allocates 24h at 00769152, then calls 007604C0 at
    // 00769166. Its existing profile selects 00760650 for final release.
    OwnedDecodedMessage<seg42::VehicleShipyardLaunchMessage,
        delete_vehicle_shipyard_launch_message_00760650> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_shipyard_launch_007604c0(&queued, image.seg42);
    seg42::read_vehicle_shipyard_launch_007605a0(&queued, &stream.in);
    ShipyardLaunchCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.to_unit = copy_goes_to_unit_handler(&seg42::is_vehicle_shipyard_launch_00760520);
    copy.shipyard_launch_7a = copy.to_unit && copy.type == 0x7A;
    copy.flag = queued.flag_20;
    delete_vehicle_shipyard_launch_message_00760650(&sent, 0u);
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
        if (set) destroy_native_session_message104_00728f60(&sent);
        else destroy_native_session_message105_00728fe0(&sent);
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
}

namespace {
// Goal 5 chain 641. 007BA170 / 007BA1C0 / 007BA200 call nothing but 0075B430
// (bsp.py show each); the session owner and selected record are Image's, as for
// UnitMessageConstructionBinding.
class Seg47MessageConstructionBinding final : public cc13_seg47::Seg47MessageHost {
public:
    explicit Seg47MessageConstructionBinding(const NativeSessionMessageContext& session)
        : session_(session) {}
    void session_message_construct_0075b430(cc13_seg47::Raw message, std::uint32_t type) override {
        construct_native_session_message_0075b430(
            reinterpret_cast<NativeSessionMessageStorage*>(message.p), type, session_);
    }
private:
    const NativeSessionMessageContext& session_;
};

std::uint32_t plane_failure_float_bits(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float plane_failure_bits_float(std::uint32_t bits) {
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

template <class Message>
cc13_seg47::Raw plane_failure_raw(Message& message) {
    return cc13_seg47::Raw{reinterpret_cast<std::uint8_t*>(&message)};
}
}  // namespace

HostSessionMessages::PlaneFailureCopy HostSessionMessages::queue_plane_failure_63_67(
    std::uint8_t type, float value_20, std::uint32_t arg_24) const {
    const Image& image = *image_;
    Seg47MessageConstructionBinding binding(image.session);
    LocalStream stream;
    // The sender's stack message is not destroyed on 007BBFA0's normal path (its
    // 0075C3F0 / 0075C4C0 / 0075A190 / 0075C5E0 / 0075A230 calls sit in the unwind
    // funclets); it goes out of scope here. The copy is 00768530's allocation,
    // read and then released by its scalar delete, as for queue_gun_destroyed_68_69.
    const auto scalar_copy = [](const NativeSessionMessage98ScalarStorage& queued,
                                bool (*is_type)(std::uint32_t)) {
        PlaneFailureCopy copy;
        copy.type = queued.header.base.type_10;
        copy.value_20 = plane_failure_bits_float(queued.value_20);
        // 00780120's ladder, then 007CCFA0's vtable[0Ch](62h) at 007CCFB4.
        copy.to_state_handler = copy_goes_to_unit_handler(is_type) && is_type(0x62);
        return copy;
    };
    switch (type) {
    case 0x63: {
        NativeSessionMessage99 sent{};
        construct_native_session_message99_0075c380(&sent, image.context98, image.profile99);
        sent.value_20 = plane_failure_float_bits(value_20);           // 007BC424
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        OwnedDecodedMessage<NativeSessionMessage99,
            delete_native_session_message99_0075c430> owner;
        NativeSessionMessage99& queued = owner.get();
        construct_native_session_message99_0075c380(&queued, image.context98, image.profile99);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        return scalar_copy(queued, &native_session_message_is99_0075c400);
    }
    case 0x64: {
        // 007BC06F: 0075C450 is the sender; its +20h is the 256.0f default.
        NativeSessionMessage100 sent{};
        construct_native_session_message100_0075c450(&sent, image.context98, image.profile100);
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        OwnedDecodedMessage<NativeSessionMessage100,
            delete_native_session_message100_0075c500> owner;
        NativeSessionMessage100& queued = owner.get();
        construct_native_session_message100_0075c450(&queued, image.context98, image.profile100);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        return scalar_copy(queued, &native_session_message_is100_0075c4d0);
    }
    case 0x65: {
        NativeSessionMessage101 sent{};
        cc13_seg47::session_message_enginefire_construct_007ba170(binding,
            plane_failure_raw(sent), static_cast<std::int32_t>(arg_24), value_20);   // 007BC11D
        // Relocate only the body's original vptr 00D02CF4 to the host profile.
        sent.scalar.header.base.profile_00 = image.profile101.slots;
        write_native_session_message101_0075c520(&sent, &stream.out, image.context98);
        stream.begin_read();
        OwnedDecodedMessage<NativeSessionMessage101,
            delete_native_session_message101_0075a1d0> owner;
        NativeSessionMessage101& queued = owner.get();
        construct_native_session_message101_0075a150(&queued, image.context98, image.profile101);
        read_native_session_message101_0075c540(&queued, &stream.in, image.context98);
        PlaneFailureCopy copy = scalar_copy(queued.scalar, &native_session_message_is101_0075a1a0);
        copy.arg_24 = queued.value_24;
        return copy;
    }
    case 0x66: {
        NativeSessionMessage102 sent{};
        cc13_seg47::session_message_engineslost_construct_007ba1c0(binding,
            plane_failure_raw(sent), value_20);                                       // 007BC162
        sent.header.base.profile_00 = image.profile102.slots;   // vptr 00D02F94
        write_native_session_scalar98_006d2f20(&sent, &stream.out, image.context98);
        stream.begin_read();
        OwnedDecodedMessage<NativeSessionMessage102,
            delete_native_session_message102_0075c620> owner;
        NativeSessionMessage102& queued = owner.get();
        construct_native_session_message102_0075c570(&queued, image.context98, image.profile102);
        read_native_session_scalar98_006d1550(&queued, &stream.in, image.context98);
        return scalar_copy(queued, &native_session_message_is102_0075c5f0);
    }
    case 0x67: {
        NativeSessionMessage103 sent{};
        cc13_seg47::session_message_spinning_construct_007ba200(binding,
            plane_failure_raw(sent), static_cast<std::uint8_t>(arg_24), value_20);   // 007BC220..
        sent.scalar.header.base.profile_00 = image.profile103.slots;   // vptr 00D02D08
        write_native_session_message103_0075c640(&sent, &stream.out, image.context104);
        stream.begin_read();
        OwnedDecodedMessage<NativeSessionMessage103,
            delete_native_session_message103_0075a270> owner;
        NativeSessionMessage103& queued = owner.get();
        construct_native_session_message103_0075a1f0(&queued, image.context104, image.profile103);
        read_native_session_message103_0075c660(&queued, &stream.in, image.context104);
        PlaneFailureCopy copy = scalar_copy(queued.scalar, &native_session_message_is103_0075a240);
        copy.arg_24 = queued.flag_24;
        return copy;
    }
    default:
        // C8h (007BA8D0) is routed only when [00E188A8]+1FE4h != 0 (multiplayer).
        throw std::logic_error("queue_plane_failure_63_67: not a single-player failure kind");
    }
}

bool HostSessionMessages::deliver_plane_failure_message(
    const gameplay_seg47_cc13::FailureMessage& routed, gameplay_seg47_cc13::PlaneFailureState& state,
    const gameplay_seg47_cc13::StateMessageEnv& env,
    const gameplay_seg47_cc13::StateMessageHost& host) const {
    const PlaneFailureCopy copy = queue_plane_failure_63_67(
        static_cast<std::uint8_t>(routed.kind), routed.value_20, routed.arg_24);
    if (!copy.to_state_handler) return false;
    // 007D0B80 reads the copy: its kind byte +10h, float +20h and +24h.
    gameplay_seg47_cc13::FailureMessage received;
    received.kind = static_cast<gameplay_seg47_cc13::FailureMessageKind>(copy.type);
    received.value_20 = copy.value_20;
    received.arg_24 = copy.arg_24;
    gameplay_seg47_cc13::state_message_007d0b80(state, received, env, host);
    return true;
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
    OwnedDecodedMessage<seg42::PlaneDisappearMessage,
        delete_plane_disappear_message_00762bc0> owner;
    auto& queued = owner.get();
    seg42::construct_plane_disappear_00762b20(&queued, image.seg42);
    seg42::read_plane_disappear_00762c30(&queued, &stream.in);
    require_local_unit_route(queued.u.h.base.type_10, 0xc7, [](std::uint32_t type) {
        return seg42::is_plane_disappear_00762b90(type);
    }, "PlaneDisappear: decoded copy did not reach the plane disappear arm");
    PlaneDisappearCopy copy;
    copy.type = queued.u.h.base.type_10;
    copy.holder_id = queued.id_20;
    delete_plane_disappear_message_00762bc0(&sent, 0u);
    return copy; // 00762BC0(1) releases the factory's 24h allocation.
}

namespace {
// Goal 5 chain 632. The 7Bh class: the 20h-byte unit message and the heading
// at +20h, 24h bytes as 00768530 case 7Bh allocates it
// (include/bsp/oct18_tail_a_factory.inc: {0x7b, 0x24, 0x007601a0, ...}).
static_assert(sizeof(NativeVehicleUnitLaunchMessage)
              == sizeof(HostSessionMessages::UnitKindSwitchCopy::bytes));

// The six boundaries 004647D0 / 00464810 call, bound to the process's
// bit-cursor bodies. The write receives the 10h cursor; the read receives the
// 18h stream for the header and stream+4 (its cursor) for the fields.
struct VehicleLaunchCodec final : cc13_seg5::VehicleLaunchWriteHost,
                                  cc13_seg5::VehicleLaunchReadHost {
    using Object = cc13_seg5::Object;
    explicit VehicleLaunchCodec(const NativeBitNumericContext& numeric) : numeric_(numeric) {}
    void write_extended_header_0075b480(Object message, Object cursor) override {
        write_native_session_message_header_0075b480(message,
            reinterpret_cast<NativeBitCursor*>(cursor));
    }
    void write_boolean_004290b0(Object cursor, std::uint8_t value) override {
        write_native_bool_bit_004290b0(reinterpret_cast<NativeBitCursor*>(cursor), value);
    }
    void write_numeric_float_004295c0(Object cursor, float value, std::uint32_t mode,
                                      std::uint32_t signed_mode, float maximum,
                                      std::uint32_t bits) override {
        std::uint32_t value_bits = 0;
        std::uint32_t scale_bits = 0;
        std::memcpy(&value_bits, &value, sizeof value_bits);
        std::memcpy(&scale_bits, &maximum, sizeof scale_bits);
        write_native_numeric_float_004295c0(reinterpret_cast<NativeBitCursor*>(cursor),
            value_bits, mode, signed_mode, scale_bits, bits, numeric_);
    }
    void read_extended_header_0075b4c0(Object message, Object stream) override {
        read_native_session_message_header_0075b4c0(message,
            reinterpret_cast<NativeSessionReadStream*>(stream));
    }
    void read_boolean_00428d70(Object cursor, Object out) override {
        read_native_bool_bit_00428d70(reinterpret_cast<NativeBitCursor*>(cursor),
            reinterpret_cast<bool*>(out));
    }
    void read_numeric_float_004293f0(Object cursor, Object out, std::uint32_t mode,
                                     std::uint32_t signed_mode, float maximum,
                                     std::uint32_t bits) override {
        // The destination is the message's own +20h cell, as 00464810 passes it.
        std::uint32_t value_bits = 0;
        std::uint32_t scale_bits = 0;
        std::memcpy(&value_bits, out, sizeof value_bits);
        std::memcpy(&scale_bits, &maximum, sizeof scale_bits);
        read_native_numeric_float_004293f0(reinterpret_cast<NativeBitCursor*>(cursor),
            &value_bits, mode, signed_mode, scale_bits, bits, numeric_);
        std::memcpy(out, &value_bits, sizeof value_bits);
    }

private:
    const NativeBitNumericContext& numeric_;
};
}  // namespace

HostSessionMessages::UnitKindSwitchCopy HostSessionMessages::queue_vehicle_unit_launch_7b(
    float heading) const {
    const Image& image = *image_;
    // 00891F05..00891F31: 007601A0's stores, then +20h the heading.
    NativeVehicleUnitLaunchMessage sent{};
    seg42::construct_vehicle_unit_launch_007601a0(&sent.u, image.seg42);
    sent.heading_20 = heading;

    VehicleLaunchCodec codec(image.numeric);
    LocalStream stream;
    // 0076E575: vtable[4] = 004647D0 (header, the +1Ch bit, +20h on 004295C0's
    // raw 32-bit path with the FLT_MAX scale).
    cc13_seg5::vehicle_launch_message_write_004647d0(reinterpret_cast<cc13_seg5::Object>(&sent),
        reinterpret_cast<cc13_seg5::Object>(&stream.out), codec);
    // 00768530 case 7Bh: operator new(24h), 007601A0 at 00769186; 0076A27B
    // vtable[8] = 00464810. Retain the original factory's 24h allocation
    // through decode/routing/projection, then scalar-delete it with bit 0 set.
    stream.begin_read();
    OwnedDecodedMessage<NativeVehicleUnitLaunchMessage,
        delete_vehicle_unit_launch_message_00464fe0> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_unit_launch_007601a0(&queued.u, image.seg42);
    cc13_seg5::vehicle_launch_message_read_00464810(reinterpret_cast<cc13_seg5::Object>(&queued),
        reinterpret_cast<cc13_seg5::Object>(&stream.in), codec);

    UnitKindSwitchCopy copy;
    // 00780120 over the copy's IsType, 004647B0 {7Bh, 49h, 46h}.
    copy.to_unit = copy_goes_to_unit_handler([](std::uint32_t type) {
        return cc13_seg5::session_message7b_is_kind_of_004647b0(static_cast<std::int32_t>(type));
    });
    std::memcpy(copy.bytes, &queued, sizeof queued);
    return copy;
}

HostSessionMessages::UnitKindSwitchCopy HostSessionMessages::queue_vehicle_add_launched_7c(
    std::uint16_t plane_id) const {
    const Image& image = *image_;
    // 006ED3E3..006ED3EF: 006EB9C0(stack message, plane). It reads only the
    // plane's +174h word (006EB9E7 MOV CX,[EAX+174h]); its other service is 0075B430.
    NativeVehicleAddLaunchedMessage sent{};
    GunFireMessageBinding codec(image.session);
    {
        PlaneHandleView174 plane{};
        plane.handle_174 = plane_id;
        cc13_seg33a::vehicle_add_launched_message_construct_006eb9c0(codec, &sent, &plane);
        // Relocate only the body's image vptr 00CFAA0C to the host profile.
        sent.u.h.base.profile_00 = image.seg42.vtables.vtable(0x00cfaa0cu);
    }

    LocalStream stream;
    // 0076E575: vtable[4] = 006EBA20.
    cc13_seg33a::vehicle_add_launched_message_write_006eba20(codec, &sent, &stream.out);
    // 00768530 case 7Ch: operator new(24h), 00760200 at 007691A6; 0076A27B
    // vtable[8] = 006EBA50. Keep the factory's real 24h allocation through
    // decode/routing/projection; 006EC100 then restamps and releases it.
    stream.begin_read();
    OwnedDecodedMessage<NativeVehicleAddLaunchedMessage,
        delete_vehicle_add_launched_message_006ec100> owner;
    auto& queued = owner.get();
    seg42::construct_vehicle_add_launched_00760200(&queued.u, image.seg42);
    cc13_seg33a::vehicle_launched_message_read_006eba50(codec, &queued, &stream.in);

    UnitKindSwitchCopy copy;
    // 00780120 over the copy's IsType, 006EBA00 {7Ch, 49h, 46h}.
    const band_006b_cc13::TypeIdRule* const is_add_launched =
        band_006b_cc13::find_type_id_rule(0x006eba00u);
    copy.to_unit = is_add_launched != nullptr
        && copy_goes_to_unit_handler([is_add_launched](std::uint32_t type) {
               return band_006b_cc13::type_id_rule_accepts(*is_add_launched,
                                                          static_cast<std::int32_t>(type));
           });
    std::memcpy(copy.bytes, &queued, sizeof queued);
    return copy;
}

} // namespace bsp
