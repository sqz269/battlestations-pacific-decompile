// Focused fixture for the real marker-name boundary: a forced level changes
// visibility without changing the sensor level, and the name virtual is lazy.
// Link against the built Win32 bsp_core.lib with /OPT:REF /MANIFEST:EMBED.
#include "bsp/cc13_seg49.hpp"
#include "bsp/gunnery_recon_detection.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
struct NameHost final : bsp::cc13_seg49::ReconDisplayNameHost {
    bsp::ReconDetectionRecord record;
    const char* name = "fixture.class.first";
    int name_calls = 0;

    int display_name_detection_level(int party, void* unit) override {
        if (party != 1 || unit != &record) std::abort();
        return static_cast<int>(bsp::recon_record_effective_level(record));
    }
    const char* unit_name_14(void* unit) override {
        if (unit != &record) std::abort();
        ++name_calls;
        return name;
    }
};
void require(bool condition) { if (!condition) std::abort(); }
}

int main() {
    NameHost host;
    const auto read = [&] {
        return bsp::cc13_seg49::recon_display_name_for_party_00803dc0(host, 1, &host.record);
    };
    require(std::strcmp(read(), "globals.unknown") == 0 && host.name_calls == 0);
    host.record.forced = true;
    host.record.forced_level = static_cast<bsp::ReconDetectionLevel>(4); // scripts may force 4
    require(read() == host.name && host.name_calls == 1);
    host.record.forced = false;
    require(std::strcmp(read(), "globals.unknown") == 0 && host.name_calls == 1);
    host.record.level = bsp::ReconDetectionLevel::identified;
    host.name = "fixture.class.reused";
    require(read() == host.name && host.name_calls == 2);
    std::puts("recon marker name: zero / forced / cleared / identified passed");
}
