#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <list>

#include "bsp/frame_clock.hpp"
#include "bsp/frontend_states.hpp"
#include "bsp/native_string.hpp"

// Names are hypotheses, not recovered symbols. Native ABI and evidence:
// docs/FRONTEND_PROMPT_SCREEN.md. These are host interfaces, not binary hooks.
namespace bsp {

enum class PromptKind : std::int32_t {
    Empty = 0, YesNo = 1, Accept = 2, Busy = 3, YesNoRestart = 4,
};

// 00530f40, ECX=this, EAX=this, RET. The padding is not initialized by native.
// Callback is a native address: the host invokes it with the result in ECX.
struct FrontEndPromptRecord {
    NativeString message;                       // +00h
    PromptKind kind{PromptKind::Empty};           // +08h
    std::uint32_t callback{};                     // +0Ch
    NativeString title;                         // +10h
    bool timed{};                                // +18h
    std::array<std::byte, 7> padding_19;
    ClockTimestamp started{};                    // +20h, frequency starts at 1
    float timeout{};                             // +30h
    std::int32_t timeout_result;                  // +34h (NOT initialized natively)
    NativeString countdown_key;                 // +38h
    bool dismissible{true};                      // +40h
    std::array<std::byte, 3> padding_41;
    std::int32_t slot{7};                         // +44h, 7 means unused
};
#if INTPTR_MAX == INT32_MAX
static_assert(sizeof(FrontEndPromptRecord) == 0x48);
static_assert(offsetof(FrontEndPromptRecord, started) == 0x20);
static_assert(offsetof(FrontEndPromptRecord, countdown_key) == 0x38);
#endif

enum class PromptWidget { Background, Title, Message, Navigation, Yes, No, Accept, Restart, Group };

struct FrontEndPromptScreen {
    // Reuse the frame/pause/title screen projection. +188h and +218h mirror
    // records[4].kind and records[6].kind on every native mutation below.
    MenuCommandScreen menu;
    std::array<std::array<NativeString, 7>, 2> labels; // native vectors +40h/+50h
    std::array<FrontEndPromptRecord, 7> records;      // native +60h
    std::int32_t current{-1};                        // +258h, host default only
    std::array<bool, 4> buttons{};                  // +25Dh..+260h: yes,no,accept,restart
    bool saved_cinematic{};                        // +261h, initialized on refresh
    std::int32_t input_mode{};                     // +264h, supplied by register/enter
    std::list<FrontEndPromptRecord> pending;         // +278h STL list, size at +280h
    std::int32_t requested_focus{-1};               // +284h
    bool field_288{};
    bool background_visible{true};                 // +3Ch
};

// One method per concrete native call/virtual boundary. GUI construction,
// rendering/navigation, localization substitutions and input remain engine
// contracts; all prompt selection, queueing, result and timeout policy is here.
struct FrontEndPromptHost {
    virtual ~FrontEndPromptHost() = default;
    virtual NativeStringStorage& strings() = 0;
    virtual FrontEndScreenHost& screens() = 0; // existing close helper 004b6e50
    virtual bool cinematic_mode() = 0;         // game+634h
    virtual int local_player_count() = 0;      // game+1FE4h
    virtual bool mission_present() = 0;       // 00E198C4
    virtual void set_cinematic_mode(bool value, bool second, bool third) = 0; //004cd0f0
    virtual void set_input_mapping(int action, int mapping) = 0; //00a933f0
    virtual void enter_screen() = 0;          // screen virtual +18h, 00531380
    virtual ClockTimestamp sample_clock() = 0; //01090AB0 virtual +20h
    virtual void invoke_callback(std::uint32_t address, int result) = 0;
    virtual void input_update(float seconds) = 0; //00a92c40
    virtual bool any_dynamic_device_button() = 0; //00a91020
    virtual void set_consumed_button(bool value) = 0; //00F8BBF4+DDh
    virtual bool input_action(int action) = 0; //004c43c0
    virtual int game_state() = 0;              //game+5D4h
    virtual bool title_primary_device_button_zero() = 0; //004ba6d0(1,0), virtual+20h(0)
    virtual void finish_input_dispatch(int channel) = 0; //004c1e90/00427190
    virtual bool widget_visible(PromptWidget widget) = 0; //virtual+38h
    virtual void set_widget_flag(PromptWidget widget, int vtable_slot, bool value) = 0;
    virtual void clear_glyphs(PromptWidget widget) = 0; //00ab80c0
    virtual void set_literal_text(PromptWidget widget, const char* text, bool flag) = 0; //00abbe50
    virtual void set_localized_text(PromptWidget widget, const NativeString& text, bool flag) = 0; //00abaed0
    virtual void navigation_owner(bool this_plus_8) = 0; //00a9ac40, nullptr or screen+8
    virtual void clear_navigation() = 0;       //00a9bf90
    virtual void prepare_button(PromptWidget widget) = 0; //00532110
    virtual void add_navigation_button(PromptWidget widget, int a, int b) = 0; //00a9d750
    virtual void erase_navigation_scratch() = 0; //004954f0(begin,end) at0053294e
    virtual int navigation_count() = 0;        //00a9ac50 result+8h
    virtual void set_navigation_focus(int index) = 0; //00a9c7c0
    virtual void set_navigation_callback(std::uint32_t callback) = 0; //00a9ac70
    virtual void layout_buttons_00530a60() = 0; //tail layout; rendering contract
    virtual void update_widget(PromptWidget widget, float seconds) = 0; //virtual+40h
    virtual int widget_pressed_field(PromptWidget widget) = 0; //widget+D8h
    virtual void dispatch_widget(PromptWidget widget) = 0; //screen+10h virtual+4h
    virtual int event_type(std::uintptr_t event) = 0; //event virtual+5Ch
    virtual void event_strength(std::uintptr_t event, float strength) = 0; //virtual+4Ch
    virtual std::uint8_t event_code(std::uintptr_t event) = 0; //event+F0h first byte, null=>00E18E74
    virtual float countdown_round_00bf85b0(float value) = 0; //unresolved CRT rounding ABI
    virtual std::uintptr_t begin_localization_scope() = 0; //00b67980(game+1A0Ch)
    virtual void set_localization_integer(std::uintptr_t scope, const NativeString& key, int value) = 0; //00b67460
    virtual void end_localization_scope(std::uintptr_t scope) = 0; //00b67700
};

// Fresh object only; initializes owned projections of 00533120's two string
// vectors, seven record constructors and queue. GUI pointers/register remain
// the 005311c0 / 00531380 contracts. Native constructor does not set +258h..264h.
void construct_prompt_screen_00533120(FrontEndPromptScreen& screen, NativeStringStorage& storage);
// Host cleanup, NOT a claim to reconstruct the screen destructor.
void release_prompt_screen_strings(FrontEndPromptScreen& screen, NativeStringStorage& storage) noexcept;
void destroy_prompt_record_00530f90(FrontEndPromptRecord& record, NativeStringStorage& storage) noexcept;

// Native ECX=screen, ten logical stack arguments / RET 2Ch. The ninth is an
// owned 8-byte NativeString value, released before returning. Fifth is unread.
// Precondition slot in [0,6]. Passing strings within records preserves native
// alias order: all seven dismiss attempts precede the source copies.
void raise_prompt_00531b00(FrontEndPromptScreen&, FrontEndPromptHost&, int slot,
    const NativeString& message, PromptKind kind, std::uint32_t callback,
    std::uint32_t unused_flag, const NativeString& title, float timeout,
    int timeout_result, NativeString countdown_key, bool dismissible);
void dismiss_prompt_00532a20(FrontEndPromptScreen&, FrontEndPromptHost&, int slot); //ECX screen, RET4
void dismiss_all_prompts_00530650(FrontEndPromptScreen&, FrontEndPromptHost&); //ECX screen, RET
// Complete normal00530670/00530C20 (ECX prompt screen, no stack args, RET).
// Only input_mode+264==0 changes navigation. The last helper reads the current
// navigation count, then selects its native DWORD count-minus-one (empty =>-1).
// Host operations resolve the SAME screen's current navigation widget+24;
// they retain actual Listbox current80 effects. No requested_focus+284 store.
void focus_first_prompt_navigation_00530670(FrontEndPromptScreen&, FrontEndPromptHost&);
void focus_last_prompt_navigation_00530c20(FrontEndPromptScreen&, FrontEndPromptHost&);
void refresh_prompt_screen_00532360(FrontEndPromptScreen&, FrontEndPromptHost&); //ECX screen, tail00530a60
void complete_prompt_00532c50(FrontEndPromptScreen&, FrontEndPromptHost&, int result); //ECX screen, RET4
void prompt_widget_event_00532cb0(FrontEndPromptScreen&, FrontEndPromptHost&, std::uintptr_t event); //ECX screen+10h, RET4
void update_prompt_screen_00532dc0(FrontEndPromptScreen&, FrontEndPromptHost&, float seconds); //ECX screen, RET4
}
