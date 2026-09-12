#pragma once
#include "bsp/main_menu_command_listener.hpp"
#include "bsp/mission_progress.hpp"
#include "bsp/locale_tables.hpp"

namespace bsp {

// Resolve the CURRENT actual game+6B4 score owner after selected mission5806A0.
// ProfileResetState at game+650 is not read by594B60. Do not copy its unlock
// completion projection or construct a second MissionProgress for this view.
struct MainMenuMedalServices {
    virtual ~MainMenuMedalServices() = default;
    virtual MissionProgress& mission_progress_00e188a8_6b4() = 0;
};
struct MainMenuMedalBindings {
    MainMenuWidgetListenerBindings& widget;
    MainMenuCommandServices& command;
    MainMenuMedalServices& scores;
};
// Complete9052D0..9052D7: ECX score record; EAX=0/1, RET. Tests +00 !=0.
bool mission_score_completed_009052d0(const MissionScoreRecord&) noexcept;
// Complete normal594B60..594BE4: ECX screen, RET. Mutates the SAME score map
// through its established594A70 operator[] and actual screen2F8 Icon. Uses the
// current widget slot again after visibility callbacks. Semantic record/map
// interface; native tree/record/vtable/SEH ABI and gameplay are not claimed.
void update_main_menu_medal_00594b60(MainMenuMedalBindings&);

// Distinct from the18h pooled log builder: native851E90 constructs this16h
// fixed-capacity owner using array operator_new;851EC0 releases with CRT free.
// The buffer is uninitialized until the first successful append (even empty).
struct NativeFixedStringBuilder {
    std::uint32_t vtable_00;
    char* buffer_04;
    std::uint32_t capacity_08;
    std::uint32_t used_0c;
};
static_assert(sizeof(NativeFixedStringBuilder) == 0x10, "MSVC Win32 required");
struct NativeFixedStringBuilderMemory {
    virtual ~NativeFixedStringBuilderMemory() = default;
    virtual char* allocate_array_00bf55be(std::uint32_t bytes) = 0;
    virtual void free_00bf6989(char*) noexcept = 0;
};
// Source CRT allocation domain. No native heap/new-handler ABI claim.
NativeFixedStringBuilderMemory& crt_fixed_string_builder_memory() noexcept;
// Full normal851E90/851EC0/F50/FA0; ECX builder, RET4 for ctor/appends, RET
// for destructor. Append returns builder and skips an oversized item whole.
NativeFixedStringBuilder& construct_fixed_string_builder_00851e90(
    NativeFixedStringBuilder&, std::uint32_t capacity, NativeFixedStringBuilderMemory&);
void destroy_fixed_string_builder_00851ec0(
    NativeFixedStringBuilder&, NativeFixedStringBuilderMemory&) noexcept;
NativeFixedStringBuilder& append_fixed_string_00851f50(
    NativeFixedStringBuilder&, const char*);
NativeFixedStringBuilder& append_fixed_integer_00851fa0(
    NativeFixedStringBuilder&, std::int32_t);

struct MainMenuDateBindings {
    // Canonical language catalog and actual settings+4, borrowed and reread
    // after the129-byte builder allocation. Native index must be valid.
    const std::vector<LanguageEntry>& languages_00f88974;
    const volatile std::uint32_t& language_index_00f88984;
    // Borrow the real13-entry table: null, globals.date_jan ... date_dec.
    // No zero-month correction, bounds clamp, or eager localization is added.
    const char* const volatile* month_keys_00e08100;
    NativeFixedStringBuilderMemory& builder_memory;
};
// Complete normal43BC30..43BDE5. ECX fresh output8h header, EDX year(+60),
// stack month(+64),day(+68), EAX output, RET8. Preserves language/output pooled
// allocations and independent129-byte builder lifetime; all five native
// language arms emit the original localization markup. Valid native month
// pointers/allocation domains required. No SEH/native ABI/rendering claim.
NativeString& build_main_menu_date_0043bc30(NativeString& output,
    std::uint32_t date_60, std::uint32_t date_64, std::uint32_t date_68,
    MainMenuDateBindings&, NativeStringStorage&);
} // namespace bsp
