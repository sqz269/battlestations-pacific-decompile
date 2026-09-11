#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Audio and online phase of cSkeletonAppMidway::Init (BSP_Application_Initialize,
// 0073d410).  Three routines are modelled here:
//
//   00a88770  sound system construction and FMOD bring-up   (called at 0073dafd)
//   00a79230  streamed dialog manager construction          (called at 0073db2b)
//   00a87060  sound/streamed_dialogs.def parser             (called from 00a79230)
//   00a40df0  XenonSystemManager / Games for Windows LIVE   (called at 0073dc7c)
//
// FMOD Ex, FMOD Event and xlive.dll are dynamically linked in the original image,
// so the recovered game code is a wrapper.  Every foreign call is routed through
// an injected host interface: nothing is initialised for real when this module is
// exercised.  No global or type of the original binary is invented; addresses are
// carried as constants so a reviewer can walk straight back to the listing.
//
// Evidence: docs/APP_INIT_AUDIO_ONLINE.md, reports/app_init_audio_online.json.

namespace bsp {

// ---------------------------------------------------------------------------
// 00a88770 - sound system construction
// ---------------------------------------------------------------------------

// Only FMOD_OK and the one value the recovered code tests are named.  Every FMOD
// call in 00a88770 is compared against 0x2b and nothing else, so any other
// failure passes through unnoticed.
enum class FmodResult : std::uint32_t {
    ok = 0u,
    err_memory = 0x2bu,
};

// FMOD_OUTPUTTYPE.  Only the value the recovered code passes is proven; the
// other two are the documented neighbours and are kept for readability.
enum class FmodOutputType : std::int32_t {
    autodetect = 0,
    unknown = 1,
    nosound = 2,
};

// FMOD_SPEAKERMODE.  The switch at 00a88a19 accepts 0..7 inclusive, which fixes
// the upper bound of the set the original build knew about.
enum class FmodSpeakerMode : std::int32_t {
    raw = 0,
    mono = 1,
    stereo = 2,
    quad = 3,
    surround = 4,
    five_point_one = 5,
    seven_point_one = 6,
    srs5_1_matrix = 7,
};

// Value stored at sound system +0x170 by the jump table at 00a88aac.  It is a
// game-side enum, not an FMOD one: 5.1 and the SRS 5.1 matrix mode collapse onto
// the same entry, and FMOD_SPEAKERMODE_RAW leaves the field untouched.
enum class SoundSpeakerLayout : std::uint32_t {
    unset = 0u,
    mono = 1u,
    stereo = 2u,
    quad = 3u,
    surround = 4u,
    five_point_one = 5u,
    seven_point_one = 6u,
};

// 00a88a1e, table of eight entries at 00a88aac.
SoundSpeakerLayout speaker_layout_from_mode(FmodSpeakerMode mode);

// Out parameters of FMOD::System::getDriverCaps, in the order they are pushed at
// 00a888a6-00a888b5.
struct FmodDriverCaps {
    std::uint32_t caps = 0u;
    std::int32_t min_frequency = 0;
    std::int32_t max_frequency = 0;
    FmodSpeakerMode control_panel_speaker_mode = FmodSpeakerMode::raw;
};

// FMOD::System::setFileSystem arguments at 00a8894a-00a88960.  The four hooks are
// image addresses, not reconstructed functions, so they stay integers.
struct FmodFileSystemHooks {
    std::uint32_t user_open = 0u;
    std::uint32_t user_close = 0u;
    std::uint32_t user_read = 0u;
    std::uint32_t user_seek = 0u;
    std::int32_t block_align = 0;
};

// FMOD::System::set3DSettings arguments at 00a8897b-00a88994.
struct Fmod3DSettings {
    float doppler_scale = 0.0f;
    float distance_factor = 0.0f;
    float rolloff_scale = 0.0f;
};

// Arguments of the four argument slot-0 call at 00a88923-00a88931.
struct FmodEventSystemInitArgs {
    std::int32_t max_channels = 0;
    std::uint32_t init_flags = 0u;
    void* extra_driver_data = nullptr;
    std::uint32_t event_init_flags = 0u;
};

// Every foreign entry point 00a88770 reaches.  The two event system entries are
// dispatched through the handle's own function table rather than through an
// import, so they carry their slot index in the comment.
class FmodStartupHost {
public:
    virtual ~FmodStartupHost() = default;

    // 00c2df80, the FMOD_EventSystem_Create import.
    virtual FmodResult event_system_create(void** out_event_system) = 0;
    // Handle table slot 7 (+0x1c), called at 00a88844.
    virtual FmodResult event_system_get_system_object(void* event_system, void** out_system) = 0;
    // Handle table slot 0, called at 00a88931.
    virtual FmodResult event_system_init(void* event_system, const FmodEventSystemInitArgs& args) = 0;

    virtual FmodResult system_get_num_drivers(void* system, std::int32_t* out_count) = 0;
    virtual FmodResult system_get_driver_caps(void* system, std::int32_t driver_index,
                                              FmodDriverCaps* out_caps) = 0;
    virtual FmodResult system_set_speaker_mode(void* system, FmodSpeakerMode mode) = 0;
    virtual FmodResult system_set_output(void* system, FmodOutputType type) = 0;
    virtual FmodResult system_set_file_system(void* system, const FmodFileSystemHooks& hooks) = 0;
    virtual FmodResult system_set_3d_settings(void* system, const Fmod3DSettings& settings) = 0;
    virtual FmodResult system_get_driver(void* system, std::int32_t* out_driver) = 0;
    virtual FmodResult system_get_output(void* system, FmodOutputType* out_type) = 0;
    virtual FmodResult system_get_speaker_mode(void* system, FmodSpeakerMode* out_mode) = 0;

    // 00a7a460.  In this build the handler ignores its argument and only calls
    // FMOD_Memory_GetStats into two discarded locals, so the message never
    // reaches a log.  It is passed here so the call sites stay reviewable.
    virtual void on_out_of_sound_memory(const char* message) = 0;
};

// Fields of the 0x178 byte sound system object that 00a88770 writes.  Offsets are
// the ones proven in the listing; the rest of the object belongs to the base
// constructor at 00a81480 and to the follow-on stage at 00a7ff80.
struct SoundSystemState {
    void* system = nullptr;                                        // +0x044
    void* event_system = nullptr;                                  // +0x048
    bool sound_enabled = false;                                    // +0x070
    std::int32_t min_frequency = 0;                                // +0x158
    std::int32_t max_frequency = 0;                                // +0x15c
    SoundSpeakerLayout speaker_layout = SoundSpeakerLayout::unset;  // +0x170
    std::uint32_t field_174 = 0u;                                  // +0x174

    // Locals of the routine.  getDriver and getOutput results are never read back
    // by the original code; they are kept so the queries stay observable.
    std::int32_t driver_count = 0;
    std::int32_t queried_driver = 0;
    FmodOutputType queried_output = FmodOutputType::autodetect;
    FmodSpeakerMode queried_speaker_mode = FmodSpeakerMode::raw;
    // Stack slot initialised to 1 at 00a88890 and overwritten by each
    // getDriverCaps pass, so the value that reaches setSpeakerMode is driver 0's.
    FmodSpeakerMode requested_speaker_mode = FmodSpeakerMode::mono;
    bool speaker_mode_requested = false;
    bool output_forced_to_nosound = false;
    std::uint32_t memory_failures = 0u;
};

// Literals recovered from the listing.
extern const char kOutOfSoundMemoryMessage[];  // 00d5ab94, misspelt in the original
constexpr std::uint32_t kSoundSystemObjectSize = 0x178u;   // 0073dac8
constexpr std::int32_t kSoundSystemMaxChannels = 0x200;    // 00a88929
constexpr std::uint32_t kSoundSystemInitFlags = 0x10u;     // 00a88927
constexpr std::int32_t kSoundSystemMinFrequency = 1;       // 00a888eb
constexpr std::int32_t kSoundSystemMaxFrequency = 440000;  // 00a888f1, 0x6b6c0
constexpr float kSoundSystemDopplerScale = 0.3f;           // DAT_00ce69c8
constexpr float kSoundSystemDistanceFactor = 1.0f;         // 00a8897b FLD1
constexpr float kSoundSystemRolloffScale = 1.0f;           // 00a8897b FLD1

FmodFileSystemHooks sound_system_file_system_hooks();
Fmod3DSettings sound_system_3d_settings();

// Legacy library-stage entry with fresh projected fields. Full owner construction,
// lifetime ordering and Lua configuration are provided by sound_startup.hpp.
// sound_disabled is the byte argument the
// caller computes at 0073daee as (DAT_00f889a4 == 0); the routine stores its
// negation into +0x70.
void start_audio(FmodStartupHost& host, bool sound_disabled, SoundSystemState& state);

// 00A8881E..00A88A63 library stage, with base/derived fields already initialized.
// Preserves fields not written by these instructions, including the raw/default
// speaker layout when getSpeakerMode returns an unhandled mode.
void initialize_sound_library_00a8881e_fragment(FmodStartupHost&, SoundSystemState&);

// ---------------------------------------------------------------------------
// 00a79230 and 00a87060 - streamed dialog definition table
// ---------------------------------------------------------------------------

// Index written to record +0x10 and used by 00a77ef0 to look up the channel count
// in the four entry table at 00e12ef0.
enum class DialogChannelFormat : std::int32_t {
    unspecified = 0,
    mono = 1,
    stereo = 2,
    five_point_one = 3,
};

// DAT_00e12ef0[format]: {?, 1, 2, 6}.  Entry 0 is never indexed by the parser.
std::int32_t dialog_format_channel_count(DialogChannelFormat format);

// One 0x14 byte record of the vector at table +0x08.
struct DialogChannelDefinition {
    std::string name;                                              // +0x00 length, +0x04 pointer
    std::int32_t channel_count = 0;                                // +0x08
    std::int32_t first_channel = 0;                                // +0x0c
    DialogChannelFormat format = DialogChannelFormat::unspecified;  // +0x10
};

// The 0x20 byte reference counted object created at 00a792f8 and stored at dialog
// manager +0x22c.  00a87060 fills it in.
struct DialogStreamTable {
    std::vector<DialogChannelDefinition> channels;  // +0x08 data, +0x0c count, +0x10 capacity
    float volume = 1.0f;                            // +0x14, DAT_00d7a24c
    std::int32_t total_channels = 0;                // +0x18
    bool loop = false;                              // +0x1c
};

// The shared text tokenizer 00a87060 drives (a 0x828 byte object; token text at
// +0x001, end of file flag at +0x806, source stream at +0x824).  peek is
// idempotent because 00bee8e0 returns early while the cached-token flag at +0x801
// is set; advance is 00bee800, which clears that flag.
class DialogDefinitionTokenizer {
public:
    virtual ~DialogDefinitionTokenizer() = default;

    virtual const char* peek() = 0;         // 00bee8e0
    virtual bool at_end() = 0;              // tokenizer +0x806, tested at 00a870f7
    virtual void advance() = 0;             // 00bee800
    virtual const char* read_string() = 0;  // 00bef020, consumes the token
    virtual double read_number() = 0;       // 00bef170, consumes the token
};

// Why the recovered loop stopped.  unrecognized_token has no counterpart in the
// original: see the divergence note in docs/APP_INIT_AUDIO_ONLINE.md.
enum class DialogParseOutcome : std::uint32_t {
    end_of_file,
    empty_token,
    unrecognized_token,
};

extern const char kDialogStreamTablePath[];  // 00d58f88
constexpr std::uint32_t kDialogManagerObjectSize = 0x234u;  // 0073db02
constexpr std::uint32_t kDialogManagerSlotCount = 0x12u;    // 00a79260 and 00a79283

DialogParseOutcome load_dialog_stream_table(DialogDefinitionTokenizer& tokens,
                                            DialogStreamTable& table);

// ---------------------------------------------------------------------------
// 00a40df0 - XenonSystemManager and Games for Windows LIVE
// ---------------------------------------------------------------------------

// 28 byte block built on the stack at 00a40ef1-00a40f41 and handed to
// XLiveInitializeEx.  cbSize and the three fields the routine fills are proven by
// the stores; the names of the remaining slots follow the published
// XLIVE_INITIALIZE_INFO layout and are provisional.
struct XLiveInitializeInfo {
    std::uint32_t size_bytes = 0u;           // +0x00, written as 0x1c
    void* field_04 = nullptr;                // +0x04, zero
    void* d3d_device = nullptr;              // +0x08
    void* d3d_present_parameters = nullptr;  // +0x0c
    std::uint16_t language_id = 0u;          // +0x10
    std::uint16_t field_12 = 0u;             // +0x12, left zero
    std::uint32_t field_14 = 0u;             // +0x14, zero
    std::uint32_t field_18 = 0u;             // +0x18, zero
};

class XLiveStartupHost {
public:
    virtual ~XLiveStartupHost() = default;

    // 00b1fef0 on the renderer singleton DAT_00f8d394.
    virtual void* d3d9_device() = 0;
    // DAT_00f8d394 + 0x1a28, the present parameters held inside the renderer.
    virtual void* d3d9_present_parameters() = 0;
    // GetUserDefaultLangID through 00ce22a8.
    virtual std::uint16_t user_default_lang_id() = 0;

    virtual std::int32_t xlive_initialize_ex(const XLiveInitializeInfo& info,
                                            std::uint32_t version) = 0;
    // 00a4c250 on the sub-object at manager +0x3ac.  Returns E_INVALIDARG for a
    // null target, otherwise the result of 00a4c030.  Purpose is provisional.
    virtual std::int32_t init_subsystem_3ac() = 0;
    virtual void x_online_startup() = 0;
    // XWSAStartup; out_version receives the low word of the WSADATA block.
    virtual std::int32_t x_wsa_startup(std::uint16_t requested_version,
                                       std::uint16_t* out_version) = 0;
    virtual void x_wsa_cleanup() = 0;
    virtual std::uint16_t x_socket_ntohs(std::uint16_t value) = 0;
    virtual void x_net_set_system_link_port(std::uint16_t port) = 0;
    virtual void* x_notify_create_listener(std::uint64_t areas) = 0;
    // 004254b0, the trace sink the three progress lines go to.
    virtual void log(const char* message) = 0;
    // 00a4004d, the same sink with the "ChangeState To %d" format.
    virtual void log_state_change(std::uint32_t state) = 0;
    // Indirect call through manager +0x20 at 00a40036.  The stored value is a
    // 14 byte stub in the image (0x00735510), so the call is handed back to the
    // host instead of being reconstructed.
    virtual void invoke_state_callback(const void* stub) = 0;
};

// Fields of the 0x3f0 byte XenonSystemManager that 00a40df0 writes, plus the
// two follow-on calls it makes.  Sign-in slots are named after their offset
// because the meaning of each is only inferred from 00a40020 and 00a409f0.
struct OnlineSystemState {
    void* notification_listener = nullptr;  // +0x1c
    bool notification_listener_valid = false;
    const void* callback_20 = nullptr;  // +0x20, the caller's first argument
    const void* callback_24 = nullptr;  // +0x24, the caller's second argument
    std::int32_t xlive_initialize_result = 0;
    std::int32_t subsystem_3ac_result = 0;
    bool winsock_started = false;   // XWSAStartup returned version 2.2
    std::uint16_t system_link_port = 0u;
    // +10 overlaps the low word of the debounce timestamp frequency (+8..+17).
    std::uint32_t field_10 = 0u;    // +0x10, set to 1 by the constructor
    std::uint32_t connected_flag = 0u;   // +0x8c
    std::uint32_t state = 0u;       // +0x3b0, logged as "ChangeState To %d"
    std::uint32_t field_3b4 = 0u;   // +0x3b4
    std::uint32_t field_3b8 = 0u;   // +0x3b8
    std::uint32_t signin_state_11c = 0u;  // +0x11c
    std::int32_t signin_slot_124 = 0;     // +0x124
    bool signin_flag_119 = false;   // +0x119
    bool signin_flag_11a = false;   // +0x11a
    bool flag_3bd = false;          // +0x3bd, gate on the sign-in reset
    // Achievement-ID vector, begin/end +364/+368, cleared by 00A40020.
    // 00A3FA70 builds {user,id} SDK batches from these DWORD IDs.
    std::vector<std::uint32_t> pending_notifications;
};

// Progress lines at 00d2435c, 00d2432c and 00d242ec.
extern const char kXenonStartInitializationMessage[];
extern const char kXenonXLiveInitializedMessage[];
extern const char kXenonNotifyListenerMessage[];
extern const char kXenonChangeStateMessage[];

constexpr std::uint32_t kOnlineManagerObjectSize = 0x3f0u;      // 0073dc50
constexpr std::uint32_t kXLiveInitializeVersion = 0x20029900u;  // 00a40f37
constexpr std::uint16_t kXLiveWinsockVersion = 0x0202u;         // 00a40f6d
constexpr std::uint16_t kSystemLinkPortLiteral = 0x0c02u;       // 00a40f89, 3074
constexpr std::uint64_t kNotifyListenerAreas = 0x2fu;           // 00a40fcc
constexpr std::uint32_t kOnlineCallbackStubA = 0x00735510u;     // 0073dc75
constexpr std::uint32_t kOnlineCallbackStubB = 0x00735520u;     // 0073dc70

// 00a40df0 up to and including the two calls it ends with, 00a40020 (the sign-in
// reset that logs "ChangeState To 0") and 00a409f0 (one pump of the state
// machine, which is not modelled).
void start_online(XLiveStartupHost& host, const void* callback_20, const void* callback_24,
                  OnlineSystemState& state);

// 00a40020, split out because it is also the manager's reset path.
void reset_online_signin_state(XLiveStartupHost& host, OnlineSystemState& state);

}  // namespace bsp
