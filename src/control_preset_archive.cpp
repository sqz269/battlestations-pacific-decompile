#include "bsp/control_preset_archive.hpp"
#include "bsp/keyboard_archive.hpp"

namespace bsp {

void write_control_preset_006a5640(const InputSettings& settings,
                                   SettingsWriter& writer) {
    // 006A564F..006A5665: string key 0, value 00CEE818 "Preset".
    writer.begin_section("Preset");
    // 006A566B..006A5A85 is the 006A51C0 traversal, register allocation aside.
    write_keyboard_setup_006a51c0(settings, writer);
    // 006A5A85: virtual +8h on the path that leaves the device loop.
    writer.end_section();
}

} // namespace bsp
