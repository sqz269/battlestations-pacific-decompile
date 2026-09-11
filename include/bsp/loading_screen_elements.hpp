#pragma once
// The loading screen's GUI elements, its localised hint rotation and the render
// worker callback that animates it at 25 Hz.
// Addresses: 0057C560, 0057C990, 0057C4C0, 0057C360, 0057CA00, 00ABAED0.
//
// Every name below is a hypothesis, not a recovered symbol. The call-by-call
// evidence is in docs/LOADING_SCREEN_ELEMENTS.md. Nothing here is binary
// compatible with the original: the widget vtables, the pooled native strings
// and the GUI page objects are not reproduced, and the widget handles are
// opaque host pointers rather than the native class hierarchy.
//
// This header completes include/bsp/frontend_entry.hpp rather than repeating it.
// begin_loading_screen there drives the mode configuration through
// LoadingScreenHost; the operations that host hides behind one method are
// recovered here: prepare_extra_elements() is reset_loading_hints_0057c990,
// element_set_rect() is loading_plate_size plus widget vtable +58h, and the
// int element argument it passes is LoadingScreenElement below.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/frame_clock.hpp"
#include "bsp/frontend_entry.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The GUI page and the widgets 0057C560 binds
// ---------------------------------------------------------------------------

// Returned by vtable slot +0h of the loading screen (0057C230 is MOV EAX,59h;
// RET). BSP_FrontEndScreen_Register 004F71D0 publishes the screen at
// 00E18B60 + id*4, so this is the screen's slot in the front-end registry.
inline constexpr int kLoadingScreenId = 0x59;

// 0057C58D..0057C5A3: the GUI manager factory 00AA5840 is called with this name
// and the two zero arguments, and the handle is stored at screen+0Ch. That same
// handle is argument 1 of BSP_RenderMode_PrepareAndStartWorker, so the page is
// what the render worker draws.
inline constexpr std::string_view kLoadingPageName = "FE_loading"; // 00CEF2B8 area

// Every widget 0057C560 resolves, in body order, with the field it lands in.
// The lookup is FUN_00AA7E00(&name, 1) with ECX = the page handle; the trailing
// 1 is the recursive-search flag (00AA7E00 walks children with __stricmp).
enum class LoadingScreenElement : int {
    HintText = 0, // +18h, "hint_Text": the rotating localised loading tip
    RadarGroup = 1, // +10h, "radar_Group": the worker callback's context (ECX)
    WaveIcon = 2, // +14h, "wave_Icon": the animated sweep inside radar_Group
    BackgroundGroup = 3, // "bg_Group": looked up and discarded, stored nowhere
    TitleLogoIcon = 4, // +20h, "titleLogo_Icon": shown in mode 0 only
    FrameFlagIcon = 5, // +1Ch, "frameFlag_Icon": the background image element
    TitleText = 6, // +24h, "title_Text": the mode 1 caption
    LoadingLogoFrameBox = 7, // +28h, "loadingLogo_FrameBox": the caption's plate
};

struct LoadingScreenElementBinding {
    std::string_view name;
    LoadingScreenElement element;
    // Offset in the 0x58-byte singleton at 00E194B4. 0xFFFF marks the one
    // lookup whose result 0057C609 discards (see BackgroundGroup above).
    std::uint16_t field_offset;
};

// 0057C560 body order. "FE_loading" is the page, so it is not in this table.
inline constexpr std::array<LoadingScreenElementBinding, 7> kLoadingScreenElementBindings{{
    {"hint_Text", LoadingScreenElement::HintText, 0x18},
    {"radar_Group", LoadingScreenElement::RadarGroup, 0x10},
    {"wave_Icon", LoadingScreenElement::WaveIcon, 0x14},
    {"bg_Group", LoadingScreenElement::BackgroundGroup, 0xFFFF},
    {"titleLogo_Icon", LoadingScreenElement::TitleLogoIcon, 0x20},
    {"frameFlag_Icon", LoadingScreenElement::FrameFlagIcon, 0x1C},
    {"title_Text", LoadingScreenElement::TitleText, 0x24},
}};
// "loadingLogo_FrameBox" is the eighth lookup and lands at +28h. It is kept out
// of the array above only because its name is 0x14 bytes and the native code
// builds it from a third string local; the order is still page, hint_Text,
// radar_Group, wave_Icon, bg_Group, titleLogo_Icon, frameFlag_Icon, title_Text,
// loadingLogo_FrameBox.
inline constexpr std::string_view kLoadingPlateName = "loadingLogo_FrameBox";
inline constexpr std::uint16_t kLoadingPlateFieldOffset = 0x28;

// The handles 0057C560 stores. Opaque: the widget classes are the GUI hierarchy
// that docs/APP_INIT_FONTS_GUI.md and docs/GUI_GEOMETRY_DISPATCH.md cover, and
// this packet neither reproduces nor owns them.
struct LoadingScreenElements {
    void* page{nullptr}; // +0Ch, "FE_loading"; the render worker's descriptor
    void* radar_group{nullptr}; // +10h; the worker callback's ECX
    void* wave_icon{nullptr}; // +14h; no reader was found, see the doc
    void* hint_text{nullptr}; // +18h
    void* frame_flag_icon{nullptr}; // +1Ch
    void* title_logo_icon{nullptr}; // +20h
    void* title_text{nullptr}; // +24h
    void* logo_frame_box{nullptr}; // +28h
};

// ---------------------------------------------------------------------------
// The hint rotation (screen +2Ch..+50h)
// ---------------------------------------------------------------------------

// The published image list at 00E08784..00E0878C is a vector of 8-byte native
// strings, and 0057C360 hands each entry to the localisation resolver
// 00A9FAD0, so the mp.loading_NN entries are localisation keys for the tip line
// rather than texture names. The index cursor is the separate global 00E0877C.
struct LoadingHintList {
    std::vector<std::string> keys{}; // 00E08784/88/8C, the same vector
                                     // LoadingScreenConfig::images publishes
    std::int32_t current{-1}; // 00E0877C; -1 before the first advance
};

// Fields of the same 0x58-byte singleton, all owned by the rotation.
struct LoadingHintRotation {
    std::int32_t progress_units{-1}; // +2Ch, signed max of converted incoming*128
    float progress{0.0f}; // +30h, comparison-selected progress; see0057BEC0
    ClockTimestamp shown_at{}; // +38h..+47h, when the current hint went up
    float hold_seconds{0.0f}; // +48h, its minimum time on screen
    float progress_step{0.0f}; // +4Ch, the progress each hint is worth
    float advanced_at_progress{0.0f}; // +50h, progress at the last advance
};

// The part of the singleton this packet recovers. +34h and +54h were not
// observed being read or written by any routine reached from here.
struct LoadingScreenElementState {
    LoadingScreenElements elements{};
    LoadingHintRotation rotation{};
};

// ---------------------------------------------------------------------------
// Constants, all byte-checked in the shipped image
// ---------------------------------------------------------------------------

// 00CE4D70, the step 0057C9D5 installs when the key list is empty. Progress
// never travels 200.0, so an empty list pins the rotation on hint -1 forever.
inline constexpr double kLoadingHintStepWithoutHints = 200.0;
// 00CE3978, the 2^32 addend of the signed-FILD-to-unsigned fixup at 0057C9C9
// and 0057C433: both counts are converted as unsigned.
inline constexpr float kUnsignedFixup = 4294967296.0f;
// 00CEF290, multiplied by the resolved UTF-16 length at 0057C439. One sixteenth
// of a second per character, so a 40-character tip holds for 2.5 s.
inline constexpr double kLoadingHintSecondsPerCharacter = 0.0625;

// 00CEC380, the divisor applied to the caption's +114h at 0057CDE4.
inline constexpr double kLoadingPlateReferenceWidth = 960.0;
// 00CEC3E8, both the comparison threshold at 0057CDF8 and the addend at
// 0057CE13. Exactly the double nearest 4/15.
inline constexpr double kLoadingPlatePadding = 0.26666666666666666;
// 00CEC3E4, the width the short-caption arm at 0057CE40 uses instead. Exactly
// the float nearest 8/15, one padding unit of text plus one of padding.
inline constexpr float kLoadingPlateMinimumWidth = 0.5333333611488342f;

// 00CE47A0, milliseconds per second: the worker callback keeps its own clock in
// milliseconds at 00E194BC and divides back down before using the delta.
inline constexpr double kMillisecondsPerSecond = 1000.0;
// 00D7A208, the -0.0f that 0057CAE9 subtracts the delta from. This is the
// native negation idiom, so a delta of exactly 0.0f comes back as -0.0f.
inline constexpr float kNegativeZero = -0.0f;

// The image index 0057CD3C pushes for mode 0. Modes and the published
// visibility byte pick 2, 1 or 0; the atlas that supplies them is loaded one
// instruction earlier.
inline constexpr int kMenuBackgroundImageIndex = 2;
inline constexpr std::string_view kMenuAtlasPath = "interface/textures/menu.ats"; // 00CEF30C

// ---------------------------------------------------------------------------
// Pure recovered arithmetic
// ---------------------------------------------------------------------------

// 0057C9A7..0057C9DB. The count is the byte span of the key vector shifted by 3
// and converted as unsigned; an empty or null vector takes the 200.0 arm.
float loading_hint_step(const LoadingHintList& hints) noexcept;
float loading_hint_step_for_count(std::int32_t hint_count) noexcept;

// 0057C427..0057C43F. length is the UTF-16 length the localisation resolver
// reports for the key, converted as unsigned before the multiply.
float loading_hint_hold_seconds(std::int32_t utf16_length) noexcept;

// The two-float pair 0057CDD2..0057CE5E hands to widget vtable +58h. Native
// reads the caption's laid-out width from title_text+114h and the plate's own
// current height from loadingLogo_FrameBox+24h (00AA6740 is LEA EAX,[ECX+20h],
// so it returns the widget's size pair and only .y is used).
struct LoadingPlateSize {
    float width{0.0f};
    float height{0.0f};
};
LoadingPlateSize loading_plate_size(float caption_width, float plate_height) noexcept;

// 0057CD2E..0057CD3E for mode 0 and 0057CD8B..0057CDA5 for mode 1. The third
// argument of widget vtable +88h is always FLD1, so the alpha is 1.0f.
int loading_background_image_index(LoadingScreenMode mode, bool image_visible) noexcept;

// 0057CAA7..0057CB01. now_ms and last_ms are the scaled clock samples; the
// result is the seconds the worker callback passes to wave_Icon. The absolute
// value uses the native -0.0f subtraction, so a zero delta yields -0.0f/1000.
float worker_tick_delta_seconds(float now_ms, float last_ms) noexcept;

// 0057CAA7 and 0057CB16 both scale the same sampled timestamp by 1000.0.
float clock_sample_milliseconds(const ClockTimestamp& sample) noexcept;

// ---------------------------------------------------------------------------
// The host: one method per native call site
// ---------------------------------------------------------------------------

// Nothing here has a default implementation; none of it stands in for
// unrecovered behaviour. The widget vtable slots named in the comments belong
// to the GUI hierarchy this packet does not own.
struct LoadingScreenElementHost {
    virtual ~LoadingScreenElementHost() = default;

    // 004F71D0 at 0057C566, ECX = the screen. Stores it at 00E18B60 + id*4.
    virtual void register_front_end_screen(int screen_id) = 0;
    // 004C12B0 then 00AA5840(&name, 0, 0) at 0057C58D. Returns the page handle.
    virtual void* create_gui_page(std::string_view name) = 0;
    // 00AA7E00(&name, 1) with ECX = parent. Recursive case-insensitive search.
    virtual void* find_element(void* parent, std::string_view name) = 0;

    // Clock singleton 01090AB0, vtable +20h: ECX = the singleton, one stack
    // destination, EAX = destination, RET 4. Reconstructed as
    // sample_frame_clock_00BEE080 in include/bsp/frame_clock.hpp.
    virtual ClockTimestamp sample_clock() = 0;

    // 00A9FAD0 with ECX = 00F8BC4C, out = {length, buffer}. Only the UTF-16
    // length is consumed here; the buffer is returned to the pool immediately.
    virtual std::int32_t resolve_localised_length(std::string_view key) = 0;
    // 00ABAED0(element, &key, localise), RET 8. Compares the key against the
    // widget's cached string at +F4h and returns early when equal; otherwise it
    // stores the key, rebuilds the UTF-16 geometry through 00ABA8D0 and calls
    // widget vtable +50h. localise is 1 at both native call sites, which routes
    // the string through the localisation resolver instead of 004C5E60.
    virtual void set_element_text(void* element, std::string_view key, bool localise) = 0;

    // Widget vtable +40h, __thiscall(float), RET 4. The worker callback's only
    // per-tick widget call; the argument is the elapsed seconds.
    virtual void update_element(void* element, float delta_seconds) = 0;
    // Widget vtable +88h(index, 0, alpha). The middle argument is 0 at both
    // native call sites and its meaning is not recovered.
    virtual void set_element_image(void* element, int index, int slot, float alpha) = 0;
    // Widget vtable +34h(visible).
    virtual void set_element_visible(void* element, bool visible) = 0;
    // Widget vtable +58h(&size), the pair loading_plate_size builds.
    virtual void set_element_size(void* element, LoadingPlateSize size) = 0;
    // 00AA6740, LEA EAX,[ECX+20h]: the widget's own size pair.
    virtual LoadingPlateSize element_size(void* element) = 0;
    // The float at element+114h. On the caption class this is the laid-out text
    // width in the 960-unit reference space; see the doc for why that reading
    // is required by the divide and not shared with the +114h of the stateful
    // texture class in docs/GUI_GEOMETRY_DISPATCH.md.
    virtual float element_caption_width(void* element) = 0;

    // 004CAA90 with ECX = 00E188A8, the last call of the worker callback.
    virtual void update_online_stats() = 0;
};

// ---------------------------------------------------------------------------
// The recovered routines
// ---------------------------------------------------------------------------

// 0057C560, the vtable +10h initialiser called at 0057CCC2. __fastcall(this),
// RET 0. Registers the screen, builds the FE_loading page and resolves the
// eight named widgets. Each name is built into a pooled native string and
// released again, so the page is the only thing it keeps.
void bind_loading_screen_elements_0057c560(LoadingScreenElements& elements,
    LoadingScreenElementHost& host);

// 0057C360. __fastcall(this), RET 0. Steps the cursor to the next key when one
// is left, snapshots the clock into +38h, sets the hint's hold time from the
// resolved length and pushes the key into hint_Text. Returns false when the
// cursor is already on the last key, which is when the rotation stops.
bool advance_loading_hint_0057c360(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host);

// 0057C4C0. __fastcall(this), RET 0. Advances the hint only when the reported
// progress has travelled a whole step since the last advance AND the current
// hint has been up for its hold time. Both gates are strict >.
void tick_loading_hint_0057c4c0(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host);

// 0057C990, the prepare_extra_elements() of frontend_entry.hpp, called at
// 0057CE83 for every mode. __fastcall(this), RET 0, tail-jumping into
// 0057C4C0. Clears the progress fields, sizes the step from the key count,
// rewinds the cursor to -1 and shows the first hint.
void reset_loading_hints_0057c990(LoadingScreenElementState& screen,
    LoadingHintList& hints, LoadingScreenElementHost& host);

// The render worker callback's own state. The worker thread 00B33C20 loads the
// context into ECX at 00B33CFC before the call, and 0057CB60 passes
// *(screen+10h), so the context is the radar_Group widget, not the screen.
struct LoadingScreenWorkerContext {
    void* radar_group{nullptr}; // ECX
    float last_sample_ms{0.0f}; // 00E194BC, this callback's private clock
    // 00E194B4, null-checked at 0057CB1E. Null between screens, which is why
    // the callback keeps its own clock instead of using a field of the screen.
    LoadingScreenElementState* screen{nullptr};
};

// 0057CA00. __fastcall(context), RET 0. Runs on the render worker thread at the
// rate 0057CE9D passes, which gates the body on more than 1000/25 ms of elapsed
// time. It issues no render commands: it re-resolves wave_Icon by name inside
// its context, hands it the elapsed seconds through vtable +40h, ticks the hint
// rotation and updates the online stats. The frame around it is the worker's.
void loading_screen_worker_tick_0057ca00(LoadingScreenWorkerContext& context,
    LoadingHintList& hints, LoadingScreenElementHost& host);

// 0057CE9D pushes 19h. docs/RENDER_WORKER.md establishes that the worker runs
// its callback only when the elapsed milliseconds exceed 1000/rate, so this is
// a 40 ms period.
inline constexpr int kLoadingWorkerRate = 0x19;
inline constexpr double kLoadingWorkerPeriodMilliseconds
    = kMillisecondsPerSecond / static_cast<double>(kLoadingWorkerRate);

} // namespace bsp
