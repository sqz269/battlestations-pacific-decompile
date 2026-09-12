#include "bsp/main_menu_profile_display.hpp"
#include "bsp/gui_type_dispatch.hpp"
#include "bsp/native_string_compare.hpp"
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
GuiWidgetOwner& medal(MainMenuMedalBindings& b) {
    auto* layout = b.widget.layout.medal_2f8;
    if (!layout) throw std::logic_error("main-menu medal requires its bound2F8 widget");
    return b.widget.owners.owner(*layout);
}
class CrtBuilderMemory final : public NativeFixedStringBuilderMemory {
public:
    char* allocate_array_00bf55be(std::uint32_t bytes) override {
        auto* result = static_cast<char*>(std::malloc(bytes));
        if (!result) throw std::bad_alloc();
        return result;
    }
    void free_00bf6989(char* block) noexcept override { std::free(block); }
};
struct BuilderTemporary {
    NativeFixedStringBuilder value;
    NativeFixedStringBuilderMemory& memory;
    explicit BuilderTemporary(NativeFixedStringBuilderMemory& input) : memory(input) {
        construct_fixed_string_builder_00851e90(value, 0x80, memory);
    }
    ~BuilderTemporary() { destroy_fixed_string_builder_00851ec0(value, memory); }
};
struct LanguageTemporary {
    NativeString value;
    NativeStringStorage& storage;
    char* captured_data{};
    explicit LanguageTemporary(NativeStringStorage& input) : storage(input) {}
    void construct(const char* source) {
        value.assign_0041e870(storage, source);
        captured_data = value.data(); //43BC85 keeps ESI through output allocation.
    }
    ~LanguageTemporary() {
        if (captured_data) storage.release(captured_data, value.length() + 1u);
    }
};
} // namespace

bool mission_score_completed_009052d0(const MissionScoreRecord& record) noexcept {
    return record.mission_completed_00 != 0;
}
void update_main_menu_medal_00594b60(MainMenuMedalBindings& b) {
    //00594B68 captures selected mission BEFORE current game+6B4 at594B6D/73.
    const auto& mission = b.command.selected_mission_005806a0();
    auto& progress = b.scores.mission_progress_00e188a8_6b4();
    auto& record = mission_record_00594a70(progress, mission.screen.name);
    const bool completed = mission_score_completed_009052d0(record);
    medal(b).set_visible34(completed);
    if (completed) {
        //594BA4 rereads ranking AFTER current34. SUB/TEST/JG uses wrapped DWORD.
        const auto rank = signed_word(static_cast<std::uint32_t>(record.ranking_04) - 1u);
        const auto low = static_cast<std::uint16_t>(rank > 0 ? rank : 0);
        std::int16_t state;
        std::memcpy(&state, &low, sizeof(state));
        auto& actual = medal(b);
        auto* icon = dynamic_cast<GuiIconTypeImplementation*>(&actual.implementation());
        if (!icon) throw std::logic_error("main-menu medal requires its actual Icon companion");
        icon->runtime().select_state_00ab1710(state, 0, 1.0f);
    }
}

NativeFixedStringBuilderMemory& crt_fixed_string_builder_memory() noexcept {
    static CrtBuilderMemory memory;
    return memory;
}
NativeFixedStringBuilder& construct_fixed_string_builder_00851e90(
    NativeFixedStringBuilder& value, std::uint32_t capacity, NativeFixedStringBuilderMemory& memory) {
    value.vtable_00 = 0x00d0beac;
    char* buffer = memory.allocate_array_00bf55be(capacity + 1u);
    value.capacity_08 = capacity;
    value.buffer_04 = buffer;
    value.used_0c = 0;
    return value;
}
void destroy_fixed_string_builder_00851ec0(
    NativeFixedStringBuilder& value, NativeFixedStringBuilderMemory& memory) noexcept {
    char* buffer = value.buffer_04;
    value.vtable_00 = 0x00d0beac;
    memory.free_00bf6989(buffer);
}
NativeFixedStringBuilder& append_fixed_string_00851f50(
    NativeFixedStringBuilder& value, const char* text) {
    const auto length = static_cast<std::uint32_t>(std::strlen(text));
    if (value.used_0c + length <= value.capacity_08) {
        std::memcpy(value.buffer_04 + value.used_0c, text, length);
        value.used_0c += length;
        value.buffer_04[value.used_0c] = '\0';
    }
    return value;
}
NativeFixedStringBuilder& append_fixed_integer_00851fa0(
    NativeFixedStringBuilder& value, std::int32_t number) {
    char text[32];
#pragma warning(suppress: 4996) // Native_itoa; every signed DWORD fits32 bytes.
    _itoa(number, text, 10);
    //851FA0 inlines the same append algorithm; no pooled string is allocated.
    return append_fixed_string_00851f50(value, text);
}

NativeString& build_main_menu_date_0043bc30(NativeString& output,
    std::uint32_t year, std::uint32_t month, std::uint32_t day,
    MainMenuDateBindings& b, NativeStringStorage& strings) {
    BuilderTemporary temporary(b.builder_memory);
    LanguageTemporary language(strings);
    language.construct(
        language_name_008d4870(b.languages_00f88974, b.language_index_00f88984).c_str());
    // French is an inline native _stricmp using the captured data pointer.
    const char* captured_language = language.captured_data;
    const bool french = captured_language && _stricmp(captured_language, "french") == 0;
    const char* before_month;
    const char* before_year;
    bool day_first = true;
    if (french || equal_native_string_header_00425850(&language.value, "italian")) {
        before_month = "| |";      //00CE42F4
        before_year = "|. ";      //00CE42F8
    } else if (equal_native_string_header_00425850(&language.value, "spanish")) {
        before_month = "| |.de |"; //00CE4310
        before_year = "| |.de |";
    } else if (equal_native_string_header_00425850(&language.value, "german")) {
        before_month = ". |";      //00CE4300
        before_year = "| ";       //00CE4304
    } else {
        before_month = "| ";
        before_year = ", ";       //00CE42FC
        day_first = false;
    }
    const char* month_key = b.month_keys_00e08100[month];
    auto& builder = temporary.value;
    if (day_first) {
        append_fixed_integer_00851fa0(builder, signed_word(day));
        append_fixed_string_00851f50(builder, before_month);
        append_fixed_string_00851f50(builder, month_key);
    } else {
        append_fixed_string_00851f50(builder, month_key);
        append_fixed_string_00851f50(builder, before_month);
        append_fixed_integer_00851fa0(builder, signed_word(day));
    }
    append_fixed_string_00851f50(builder, before_year);
    append_fixed_integer_00851fa0(builder, signed_word(year));
    output.assign_0041e870(strings, builder.buffer_04);
    // Scope destruction returns the language block BEFORE freeing the builder.
    return output;
}
} // namespace bsp
