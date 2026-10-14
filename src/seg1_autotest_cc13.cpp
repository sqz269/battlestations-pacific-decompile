// Segment 1 automation driver pieces. See include/bsp/seg1_autotest_cc13.hpp
// and docs/CC13_SEG1_AUTOTEST.md.
#include "bsp/seg1_autotest_cc13.hpp"

#include <cstdlib>
#include <cstring>

namespace bsp::seg1_cc13 {

namespace {

// 00BD20A0: split on a byte, keeping only non-empty pieces.
std::vector<std::string> split_nonempty(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string piece;
    for (char ch : s) {
        if (ch == sep) {
            if (!piece.empty()) out.push_back(piece);
            piece.clear();
        } else {
            piece.push_back(ch);
        }
    }
    if (!piece.empty()) out.push_back(piece);
    return out;
}

// __stricmp / 00425850 BSP_NativeString_EqualsCStringInsensitive.
bool equals_insensitive(const std::string& a, const char* b) {
    std::size_t i = 0;
    for (; i < a.size() && b[i] != '\0'; ++i) {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z') x = static_cast<char>(x - 'A' + 'a');
        if (y >= 'A' && y <= 'Z') y = static_cast<char>(y - 'A' + 'a');
        if (x != y) return false;
    }
    return i == a.size() && b[i] == '\0';
}

}  // namespace

void autotest_select_mpak_classes_00425560(AutoTestDriver& d) noexcept {
    d.mode_08 = 1;
    d.countdown_0c = 200;
}

void autotest_select_mpak_scenes_00425570(AutoTestDriver& d, bool flag_00e17659) noexcept {
    d.mode_08 = 2;
    d.campaign_18 = 0;
    d.mission_1c = 0;
    d.f_28 = 0;
    d.word_14 = 0;
    d.byte_16 = 0;
    if (flag_00e17659) d.f_24 = 1;
}

ClockTimestamp add_timestamp_to_004256f0(const ClockTimestamp& left, const ClockTimestamp& right) noexcept {
    ClockTimestamp out;
    out.frequency = left.frequency;
    if (left.frequency == right.frequency) {
        out.ticks = static_cast<std::int64_t>(static_cast<std::uint64_t>(right.ticks) +
                                              static_cast<std::uint64_t>(left.ticks));
    } else {
        // __allmul then __alldiv (low 64 bits; a zero right frequency faults natively).
        const std::int64_t scaled = static_cast<std::int64_t>(static_cast<std::uint64_t>(right.ticks) *
                                                              static_cast<std::uint64_t>(left.frequency)) /
                                    right.frequency;
        out.ticks = static_cast<std::int64_t>(static_cast<std::uint64_t>(scaled) +
                                              static_cast<std::uint64_t>(left.ticks));
    }
    return out;
}

bool timestamp_less_00425780(const ClockTimestamp& left, const ClockTimestamp& right) noexcept {
    if (left.frequency == right.frequency) return left.ticks < right.ticks;
    const std::int64_t right_seconds = right.ticks / right.frequency;   // __alldiv
    const std::int64_t left_seconds = left.ticks / left.frequency;
    return left_seconds < right_seconds;
}

void autotest_parse_test_004278a0(AutoTestDriver& d, const std::string& name, AutoTestGlobals& g,
                                  AutoTestHost& host) {
    host.install_fatal_hook_00425b80();             // 004278C2: [01090AB8] = 00425B80
    g.start_00e08060 = host.clock_now();            // 004278D6..004278F5
    g.main_thread_00e17660 = host.current_thread_id();
    d.watchdog_thread_04 = host.create_watchdog_thread_004258d0();
    const std::vector<std::string> parts = split_nonempty(name, ':');
    if (parts.empty() || !equals_insensitive(parts[0], "LoadWaitQuit")) return;   // 00CE3AD8
    const std::vector<std::string> args =
        parts.size() > 1 ? split_nonempty(parts[1], ',') : std::vector<std::string>{};
    const std::string mission = args.size() > 1 ? args[1] : std::string();
    d.mission_1c = static_cast<std::int32_t>(std::atol(mission.c_str())) - 1;
    const std::string campaign = args.empty() ? std::string() : args[0];
    if (equals_insensitive(campaign, "usa")) d.campaign_18 = 0;
    else if (equals_insensitive(campaign, "japan")) d.campaign_18 = 1;
    else if (equals_insensitive(campaign, "multi")) d.campaign_18 = 2;
    else if (equals_insensitive(campaign, "training")) d.campaign_18 = 3;
    d.mode_08 = 3;
}

std::uint32_t autotest_watchdog_004258d0(const AutoTestGlobals& g, AutoTestHost& host) {
    const ClockTimestamp budget{900, 1};   // 004258EC..00425900
    while (timestamp_less_00425780(host.clock_now(), add_timestamp_to_004256f0(g.start_00e08060, budget))) {
        host.sleep_ms(1000);
        if (host.finished_00e1ae75()) return 0;
    }
    host.write_file("crash.log", "Timed out.\n");
    host.terminate_process(3);
    return 0;
}

void autotest_fatal_hook_00425b80(const std::string& message, AutoTestHost& host) {
    host.write_file("crash.log", message + "\n");   // "%s\n"
    host.terminate_process(2);
}

void autolog_group_00426e60(const std::string& name, AutoTestHost& host) {
    host.log_line("GROUP " + name);
}

void autolog_end_group_00426ed0(AutoTestHost& host) {
    host.log_line("ENDGROUP");
}

void autolog_value_00426f30(const std::string& name, const std::string& value, AutoTestHost& host) {
    std::string key = name;
    for (char& ch : key)
        if (ch == ' ') ch = '_';
    host.log_line("LOG " + key + " " + value);
}

void autolog_memory_00427040(AutoTestHost& host) {
    autolog_group_00426e60("Memory", host);
    autolog_end_group_00426ed0(host);
}

std::uint32_t count_flagged_records_00425b20(const std::vector<FlaggedRecord>& records) noexcept {
    std::uint32_t n = 0;
    for (const FlaggedRecord& r : records) n += r.bytes[0x11] != 0 ? 1u : 0u;
    return n;
}

bool autotest_ensure_screen_00426350(std::int32_t screen, ScreenHost& host) {
    void* menu = host.main_menu_e198ac_58();
    // 00426355..00426365: menu ? menu + 8 : 0, the listener sub-object.
    void* listener = menu != nullptr ? static_cast<void*>(static_cast<std::uint8_t*>(menu) + 8) : nullptr;
    void* listbox = host.menu_listbox_1b8(menu);                       // 0042636A
    if (host.listbox_selected_data_00a9c990(listbox) == screen) {      // 00426372
        // 0042637F..00426390: PUSH list box; 00425E50(list box); PUSH EAX; listener->vtable[4].
        host.listener_activate_vslot04(listener, host.listbox_current_row_00425e50(listbox), listbox);
        return true;
    }
    host.listbox_select_data_00a9c860(listbox, screen);                // 0042639D
    return false;
}

}  // namespace bsp::seg1_cc13
