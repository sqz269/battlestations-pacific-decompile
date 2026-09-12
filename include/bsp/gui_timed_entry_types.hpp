#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
class GuiWidgetOwner;
struct GuiLayoutWidget;

inline constexpr std::uint32_t kGuiTimedEntryBaseProfile = 0x00d5d204;
inline constexpr std::uint32_t kGuiTimedEntryAlphaProfile = 0x00d5ca74;
inline constexpr std::uint32_t kGuiTimedEntrySectionProfile = 0x00d5d210;

// Newly owned Win32 14h storage. profile_00 is an identity, NEVER a callable
// C++ vtable. widget_08 borrows the SAME canonical layout, resolved through its
// existing owner runtime. It is not an original executable object pointer.
struct GuiTimedEntryStorage {
    std::uint32_t profile_00;
    float remaining_04;
    GuiLayoutWidget* widget_08;
    float target_0c;
    float rate_10;
};
static_assert(sizeof(void*) == 4, "Timed entries require MSVC Win32");
static_assert(sizeof(GuiTimedEntryStorage) == 0x14);
static_assert(offsetof(GuiTimedEntryStorage, widget_08) == 8);
static_assert(offsetof(GuiTimedEntryStorage, rate_10) == 0x10);

struct GuiTimedEntryConstants {
    const volatile float& one_00d7a24c;
    const volatile float& zero_00d7a208;
};

// AD3970/AC2ED0: ECX destination, borrowed widget stack, EAX destination,
// RET4. The base leaves +C/+10 unwritten. Alpha reads D7A24C only once.
GuiTimedEntryStorage& construct_gui_timed_entry_base_00ad3970(
    GuiTimedEntryStorage&, GuiLayoutWidget&) noexcept;
GuiTimedEntryStorage& construct_gui_timed_entry_alpha_00ac2ed0(
    GuiTimedEntryStorage&, GuiLayoutWidget&, const volatile float& one_00d7a24c) noexcept;
void reset_gui_timed_entry_base_00ad3990(GuiTimedEntryStorage&) noexcept;

// AC2FE0/AD3A40 destructor PHASE ONLY: reset the current derived profile to
// D5D204. Original virtual0 takes flags and RET4; its conditional BF65AC free
// belongs to the allocation owner. No pointer here is freed or adopted.
void destroy_gui_timed_entry_profile(GuiTimedEntryStorage&);

// AC2F40: complete arithmetic and established canonical current54/current4C
// dispatch; ECX entry, delta stack, AL changed, RET4. Native x87 control state
// and unordered comparisons are preserved. No default profile callback.
bool update_gui_timed_entry_alpha_00ac2f40(
    GuiTimedEntryStorage&, GuiWidgetOwner& domain_owner, float delta);

// ABE7B0 PARTIAL: current5C !=17 returns false. Type17 requires the missing
// canonical Section fields +F4/+F8/+100 and ABE6E0/current7C owner, and throws
// explicitly. The numeric fragment below covers ABE7CC..ABE84B separately;
// it does not complete the missing Section consumer or emit geometry.
bool update_gui_timed_entry_section_00abe7b0(
    GuiTimedEntryStorage&, GuiWidgetOwner& domain_owner, float delta);
bool approach_gui_timed_entry_section_00abe7b0(const float& current,
    const float& target, const float& rate, float delta, float& output) noexcept;

// AD39A0: ECX entry, delta then unused borrowed widget stack, AL result, RET8.
// Subtract/spill +4; negative countdown dispatches current8 with SSE-computed
// D7A208-countdown, then clears +4 only AFTER successful return. A thrown
// owner boundary leaves the negative countdown, not a completed update.
bool advance_gui_timed_entry_00ad39a0(GuiTimedEntryStorage&,
    GuiWidgetOwner& domain_owner, float delta, GuiLayoutWidget* unused_widget,
    const GuiTimedEntryConstants&);
} // namespace bsp
