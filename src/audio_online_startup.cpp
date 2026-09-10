#include "bsp/audio_online_startup.hpp"

#include <utility>

// Recovered bodies of 00a88770, 00a87060 (driven by 00a79230) and 00a40df0.
// Control flow follows the listing, not the decompiler output: the pseudocode for
// 00a88770 carries register inputs (unaff_ESI, unaff_retaddr), reuses the first
// argument slot as scratch and turns the speaker-mode jump table into a switch on
// the wrong value, so every branch below is taken from the disassembly.

namespace bsp {

// Literals as they appear in the image.  The sound failure message is misspelt in
// the shipped binary and is reproduced verbatim.
const char kOutOfSoundMemoryMessage[] = "Out of sounjd memory:";           // 00d5ab94
const char kDialogStreamTablePath[] = "sound/streamed_dialogs.def";        // 00d58f88
const char kXenonStartInitializationMessage[] =                            // 00d2435c
    " ...  XenonSystemManager Start Initialization  ...";
const char kXenonXLiveInitializedMessage[] =                               // 00d2432c
    " ...  XenonSystemManager XLiveInitialized  ...";
const char kXenonNotifyListenerMessage[] =                                 // 00d242ec
    " ...  XenonSystemManager XNotifyCreateListener  ... SUCCEED ";
const char kXenonChangeStateMessage[] =                                    // 00a4004d
    " ...   XenonSystemManager  ChangeState To %d";

namespace {

// The parser compares with _stricmp at every keyword site (00a871b6 and its
// siblings), so the comparison is case insensitive and the length is implicit.
bool equals_ignore_case(const char* token, const char* keyword)
{
    if (token == nullptr) {
        return false;
    }
    for (;; ++token, ++keyword) {
        const unsigned char left = static_cast<unsigned char>(*token);
        const unsigned char right = static_cast<unsigned char>(*keyword);
        const unsigned char lower_left =
            (left >= 'A' && left <= 'Z') ? static_cast<unsigned char>(left + 32) : left;
        const unsigned char lower_right =
            (right >= 'A' && right <= 'Z') ? static_cast<unsigned char>(right + 32) : right;
        if (lower_left != lower_right) {
            return false;
        }
        if (lower_left == 0u) {
            return true;
        }
    }
}

// Every FMOD call site in 00a88770 is followed by CMP EAX,0x2b and, on a match,
// by the handler at 00a7a460 with 00d5ab94 pushed.  No other result is tested and
// the routine never returns early.
void check_fmod(FmodStartupHost& host, FmodResult result, SoundSystemState& state)
{
    if (result == FmodResult::err_memory) {
        host.on_out_of_sound_memory(kOutOfSoundMemoryMessage);
        ++state.memory_failures;
    }
}

// 00a872a7-00a872c8.  The original grows the vector at manager +0x08 through
// 00a78dc0 with a doubling policy (new capacity max(1, capacity * 2)) before
// placement-constructing the record with 00a77ef0; std::vector covers both.
void append_channel(DialogStreamTable& table, std::string name, DialogChannelFormat format,
                    std::int32_t channel_count)
{
    DialogChannelDefinition record;
    record.name = std::move(name);
    record.channel_count = channel_count;
    record.format = format;
    // Written after the count is bumped, from the running total as it stands
    // before this entry is accounted for.
    record.first_channel = table.total_channels;
    table.channels.push_back(std::move(record));
    table.total_channels += channel_count;
}

// 00a87149-00a872f9, the body of the Channels block.
void parse_channel_block(DialogDefinitionTokenizer& tokens, DialogStreamTable& table)
{
    for (;;) {
        if (equals_ignore_case(tokens.peek(), "end")) {
            break;
        }

        std::string name;
        const char* text = tokens.read_string();
        if (text != nullptr) {
            name.assign(text);
        }

        // The format keyword is optional in the recovered code: when none of the
        // three matches, nothing is consumed and the record is appended with a
        // zero channel count, which makes the next iteration read the format
        // keyword as the next channel's name.
        DialogChannelFormat format = DialogChannelFormat::unspecified;
        std::int32_t channel_count = 0;
        if (equals_ignore_case(tokens.peek(), "mono")) {
            tokens.advance();
            format = DialogChannelFormat::mono;
        } else if (equals_ignore_case(tokens.peek(), "stereo")) {
            tokens.advance();
            format = DialogChannelFormat::stereo;
        } else if (equals_ignore_case(tokens.peek(), "51")) {
            tokens.advance();
            format = DialogChannelFormat::five_point_one;
        }
        if (format != DialogChannelFormat::unspecified) {
            channel_count = dialog_format_channel_count(format);
        }

        append_channel(table, std::move(name), format, channel_count);
    }
    // 00a872fb, consumes "end".
    tokens.advance();
}

}  // namespace

// The path literal is 26 characters, which is the length 00a79365 passes to the
// string allocator before the copy at 00a79394.
static_assert(sizeof(kDialogStreamTablePath) == 27u,
              "the streamed dialog definition path is 0x1a characters plus a terminator");

// ---------------------------------------------------------------------------
// 00a88770
// ---------------------------------------------------------------------------

SoundSpeakerLayout speaker_layout_from_mode(FmodSpeakerMode mode)
{
    // 00a88a19: CMP EAX,0x7 / JA default, then JMP [EAX*4 + 0xa88aac].  The eight
    // table entries are 00a88a63 (default), 00a88a25, 00a88a2d, 00a88a35,
    // 00a88a41, 00a88a59, 00a88a4d, 00a88a59 in index order.
    switch (mode) {
    case FmodSpeakerMode::mono:
        return SoundSpeakerLayout::mono;
    case FmodSpeakerMode::stereo:
        return SoundSpeakerLayout::stereo;
    case FmodSpeakerMode::quad:
        return SoundSpeakerLayout::quad;
    case FmodSpeakerMode::surround:
        return SoundSpeakerLayout::surround;
    case FmodSpeakerMode::five_point_one:
    case FmodSpeakerMode::srs5_1_matrix:
        // Index 5 and index 7 share the block at 00a88a59.
        return SoundSpeakerLayout::five_point_one;
    case FmodSpeakerMode::seven_point_one:
        return SoundSpeakerLayout::seven_point_one;
    case FmodSpeakerMode::raw:
    default:
        // RAW and anything above 7 fall through the jump table without a store,
        // so the field keeps whatever the constructor left in it.
        return SoundSpeakerLayout::unset;
    }
}

FmodFileSystemHooks sound_system_file_system_hooks()
{
    // 00a8894a-00a88960, pushed right to left: blockalign, userseek, userread,
    // userclose, useropen.
    FmodFileSystemHooks hooks;
    hooks.user_open = 0x00a7d410u;
    hooks.user_close = 0x00a7b750u;
    hooks.user_read = 0x00a79930u;
    hooks.user_seek = 0x00a79970u;
    hooks.block_align = 0;
    return hooks;
}

Fmod3DSettings sound_system_3d_settings()
{
    // 00a8897b: FLD1 stored into both of the upper two slots, DAT_00ce69c8 into
    // the lowest, so dopplerscale is 0.3f and the other two are 1.0f.
    Fmod3DSettings settings;
    settings.doppler_scale = kSoundSystemDopplerScale;
    settings.distance_factor = kSoundSystemDistanceFactor;
    settings.rolloff_scale = kSoundSystemRolloffScale;
    return settings;
}

void start_audio(FmodStartupHost& host, bool sound_disabled, SoundSystemState& state)
{
    state = SoundSystemState{};

    // 00a88798-00a887b1: SETZ on the byte argument, so the stored flag is the
    // negation of what the caller computed at 0073daee.
    state.sound_enabled = !sound_disabled;
    state.field_174 = 0u;

    // 00a8881e, FMOD_EventSystem_Create into +0x48.
    check_fmod(host, host.event_system_create(&state.event_system), state);
    // 00a88844, handle table slot 7, writes +0x44.
    check_fmod(host, host.event_system_get_system_object(state.event_system, &state.system), state);

    if (state.sound_enabled) {
        // 00a8886d.
        check_fmod(host, host.system_get_num_drivers(state.system, &state.driver_count), state);

        // 00a88890 seeds the control-panel speaker mode slot with 1 so the value
        // survives a machine with no drivers.
        state.requested_speaker_mode = FmodSpeakerMode::mono;
        state.speaker_mode_requested = true;

        // 00a888a0-00a888be counts down from driver_count - 1 to 0 inclusive, so
        // the values that survive belong to driver 0.  The result of each call is
        // never tested.
        for (std::int32_t index = state.driver_count - 1; index >= 0; --index) {
            FmodDriverCaps caps;
            caps.min_frequency = state.min_frequency;
            caps.max_frequency = state.max_frequency;
            caps.control_panel_speaker_mode = state.requested_speaker_mode;
            host.system_get_driver_caps(state.system, index, &caps);
            // The two frequency out-pointers address +0x158 and +0x15c directly.
            state.min_frequency = caps.min_frequency;
            state.max_frequency = caps.max_frequency;
            state.requested_speaker_mode = caps.control_panel_speaker_mode;
        }

        // 00a888c8.
        check_fmod(host, host.system_set_speaker_mode(state.system, state.requested_speaker_mode),
                   state);
    }

    // 00a888eb and 00a888f1, unconditional and after the driver scan, so the
    // frequencies read back from getDriverCaps are discarded.
    state.min_frequency = kSoundSystemMinFrequency;
    state.max_frequency = kSoundSystemMaxFrequency;

    if (!state.sound_enabled) {
        // 00a88904, the disabled-sound path: FMOD_OUTPUTTYPE_NOSOUND.
        state.output_forced_to_nosound = true;
        check_fmod(host, host.system_set_output(state.system, FmodOutputType::nosound), state);
    }

    // 00a88931, handle table slot 0.  The System is fully configured first, which
    // is what makes this the EventSystem::init call.
    FmodEventSystemInitArgs init_args;
    init_args.max_channels = kSoundSystemMaxChannels;
    init_args.init_flags = kSoundSystemInitFlags;
    init_args.extra_driver_data = nullptr;
    init_args.event_init_flags = 0u;
    check_fmod(host, host.event_system_init(state.event_system, init_args), state);

    // 00a88961 and 00a88994.
    check_fmod(host, host.system_set_file_system(state.system, sound_system_file_system_hooks()),
               state);
    check_fmod(host, host.system_set_3d_settings(state.system, sound_system_3d_settings()), state);

    // 00a889b6, 00a889d8 and 00a889fa.  The driver index and output type are read
    // into stack slots the routine never looks at again; only the speaker mode
    // feeds the jump table.  There is no setDriver call anywhere in the routine.
    check_fmod(host, host.system_get_driver(state.system, &state.queried_driver), state);
    check_fmod(host, host.system_get_output(state.system, &state.queried_output), state);
    check_fmod(host, host.system_get_speaker_mode(state.system, &state.queried_speaker_mode), state);

    state.speaker_layout = speaker_layout_from_mode(state.queried_speaker_mode);

    // 00a88a63 allocates a 0x18 byte object into +0x54 and 00a88a8e calls the
    // Lua-driven second stage 00a7ff80, which is where the master channel group
    // and the FMOD advanced settings are handled.  Both are outside this packet.
}

// ---------------------------------------------------------------------------
// 00a79230 and 00a87060
// ---------------------------------------------------------------------------

std::int32_t dialog_format_channel_count(DialogChannelFormat format)
{
    // DAT_00e12ef0 + index * 4, read at 00a77f0b and at the three parser sites.
    switch (format) {
    case DialogChannelFormat::mono:
        return 1;
    case DialogChannelFormat::stereo:
        return 2;
    case DialogChannelFormat::five_point_one:
        return 6;
    case DialogChannelFormat::unspecified:
    default:
        return 0;
    }
}

DialogParseOutcome load_dialog_stream_table(DialogDefinitionTokenizer& tokens,
                                            DialogStreamTable& table)
{
    // The table starts from the values 00a79307-00a79333 leave in the 0x20 byte
    // object: volume 1.0f, no channels, no loop.
    table = DialogStreamTable{};

    for (;;) {
        const char* token = tokens.peek();  // 00a870f2
        if (tokens.at_end()) {              // 00a870f7, tokenizer +0x806
            return DialogParseOutcome::end_of_file;
        }
        if (token == nullptr || token[0] == '\0') {  // 00a87104-00a8711f
            return DialogParseOutcome::empty_token;
        }

        if (equals_ignore_case(tokens.peek(), "Channels")) {
            tokens.advance();
            parse_channel_block(tokens, table);
        } else if (equals_ignore_case(tokens.peek(), "Volume")) {
            tokens.advance();
            table.volume = static_cast<float>(tokens.read_number());
        } else if (equals_ignore_case(tokens.peek(), "Loop")) {
            tokens.advance();
            table.loop = true;
        } else {
            // Divergence, recorded in docs/APP_INIT_AUDIO_ONLINE.md: the original
            // consumes nothing on this path and spins.  Returning keeps the
            // recovered structure without reproducing the hang.
            return DialogParseOutcome::unrecognized_token;
        }
    }
}

// ---------------------------------------------------------------------------
// 00a40df0
// ---------------------------------------------------------------------------

void reset_online_signin_state(XLiveStartupHost& host, OnlineSystemState& state)
{
    // 00a40028: the stored stub is called only when both the slot and the
    // connected flag at +0x8c are set.
    if (state.callback_20 != nullptr && state.connected_flag != 0u) {
        host.invoke_state_callback(state.callback_20);
    }

    // The three sign-in fields are only reset while +0x3bd is clear.
    if (!state.flag_3bd) {
        state.signin_flag_119 = false;
        state.signin_state_11c = 1u;
        state.signin_flag_11a = false;
    }

    state.signin_slot_124 = -1;
    state.pending_notifications.clear();
    state.state = 0u;
    host.log_state_change(state.state);
    state.field_3b4 = 1u;
    state.field_3b8 = 1u;
}

void start_online(XLiveStartupHost& host, const void* callback_20, const void* callback_24,
                  OnlineSystemState& state)
{
    state = OnlineSystemState{};

    // Constructor prologue of 00a40df0: the object is zeroed, +0x10 is set to 1,
    // and the two caller-supplied stubs land in +0x20 and +0x24.
    state.field_10 = 1u;
    state.callback_20 = callback_20;
    state.callback_24 = callback_24;

    host.log(kXenonStartInitializationMessage);  // 00a40eae

    // 00a40ef1-00a40f41.  cbSize is written first, the whole block is zeroed, then
    // three fields are filled: the D3D9 device, the present parameters held at
    // renderer + 0x1a28, and the user default language id as a WORD.
    XLiveInitializeInfo info;
    info.size_bytes = 0x1cu;
    info.field_04 = nullptr;
    info.d3d_device = host.d3d9_device();
    info.d3d_present_parameters = host.d3d9_present_parameters();
    info.language_id = host.user_default_lang_id();
    info.field_12 = 0u;
    info.field_14 = 0u;
    info.field_18 = 0u;

    // 00a40f46.  The HRESULT is discarded by the original: nothing branches on it
    // and there is no path for a missing xlive.dll, which is a plain import.
    state.xlive_initialize_result = host.xlive_initialize_ex(info, kXLiveInitializeVersion);
    host.log(kXenonXLiveInitializedMessage);  // 00a40f50

    // 00a40f5e, on the sub-object at manager +0x3ac.
    state.subsystem_3ac_result = host.init_subsystem_3ac();
    host.x_online_startup();  // 00a40f63

    // 00a40f72: XWSAStartup(0x0202, &wsadata), then the low word of WSADATA is
    // checked byte by byte at 00a40f77-00a40f82 and anything but 2.2 is undone.
    std::uint16_t negotiated = 0u;
    host.x_wsa_startup(kXLiveWinsockVersion, &negotiated);
    const std::uint16_t major = static_cast<std::uint16_t>(negotiated & 0x00ffu);
    const std::uint16_t minor = static_cast<std::uint16_t>((negotiated >> 8) & 0x00ffu);
    state.winsock_started = (major == 0x02u) && (minor == major);
    if (!state.winsock_started) {
        host.x_wsa_cleanup();  // 00a40f84
    }

    // 00a40f8e: the literal 0x0c02 is 3074, the Xbox LIVE system link port, and
    // the byte swap turns it into network order before it is handed on.
    state.system_link_port = host.x_socket_ntohs(kSystemLinkPortLiteral);
    host.x_net_set_system_link_port(state.system_link_port);  // 00a40f94

    // 00a40fc5: PUSH 0 / PUSH 0x2f, one ULONGLONG argument.
    state.connected_flag = 0u;
    state.notification_listener = host.x_notify_create_listener(kNotifyListenerAreas);

    // 00a40fd9 and 00a40fe0 reject both zero and -1 before logging.
    void* const invalid = reinterpret_cast<void*>(~static_cast<std::uintptr_t>(0));
    state.notification_listener_valid =
        state.notification_listener != nullptr && state.notification_listener != invalid;
    if (state.notification_listener_valid) {
        host.log(kXenonNotifyListenerMessage);  // 00a40fea
    }

    // 00a40ff4.
    reset_online_signin_state(host, state);

    // 00a40ffb calls 00a409f0, one pump of the manager's state machine.  It reads
    // the platform timer through DAT_01090ab0 and is not modelled here.
}

}  // namespace bsp
