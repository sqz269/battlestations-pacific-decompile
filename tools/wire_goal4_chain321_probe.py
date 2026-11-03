"""One Win32 BIN parser/merge ownership case; no gameplay claim."""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "local/output/wire_g4_c321_probe"
SOURCE = r'''
#include "bsp/scene_numeric_property_merge.hpp"
#include "bsp/wire_goal4_chain321_switches.hpp"
#include <cassert>
#include <cstdio>
int main() {
    using namespace bsp;
    const auto document = parse_scene_document(R"(
      entity "destination" (Ship) {
        localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
        properties { Payload = BIN 2 ff 00; }
      }
      entity "source" (Ship) {
        localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
        properties {
          Payload = BIN 5 0f A 12ff g1 1g; Added = bin 1 fF;
          Short = BIN 2 00; Empty = BIN; Zero = BIN 0; Negative = BIN -1;
        }
      }
    )");
    assert(document.errors.empty() && document.entities.size() == 2);
    const auto& destination = document.entities[0].properties;
    const auto& source = document.entities[1].properties;
    const std::vector<std::uint8_t> decoded{0x0f, 0x0a, 0x12, 0x01, 0x10};
    assert(source.find("Payload")->has_byte_array == kWireGoal4Chain321Bound);
    if constexpr (kWireGoal4Chain321Bound) {
        assert(source.find("Payload")->byte_array == decoded);
        assert(source.find("Added")->byte_array == std::vector<std::uint8_t>{0xff});
    }
    for (const char* key : {"Short", "Empty", "Zero", "Negative"}) {
        assert(!source.find(key)->has_byte_array);
    }
    std::vector<SceneProperty> retained;
    for (const bool overwrite : {false, true}) {
        game::SceneNumericPropertyMerge merge(destination, source, overwrite);
        for (std::size_t i = 0; i < source.values.size(); ++i) {
            assert(merge.handles(i) == (kWireGoal4Chain321Bound && i < 2));
            if (!merge.handles(i)) continue;
            const auto& incoming = source.values[i];
            const auto* prior = destination.find(incoming.key);
            auto metadata = prior != nullptr && !overwrite ? *prior : incoming;
            metadata.byte_array.clear(); // Prove result reads the native merged owner.
            auto result = merge.result(metadata);
            assert(result.has_byte_array);
            const auto want = i == 1 ? std::vector<std::uint8_t>{0xff}
                : overwrite ? decoded : std::vector<std::uint8_t>{0xff, 0x00};
            assert(result.byte_array == want);
            retained.push_back(std::move(result));
        }
    }
    // Source/destination native bags and their adopted/clone allocations retired.
    if constexpr (kWireGoal4Chain321Bound) {
        assert(retained.size() == 4 && retained[0].byte_array.size() == 2);
        assert(retained[2].byte_array == decoded && retained[3].byte_array[0] == 0xff);
    }
    std::puts("chain321 BIN parser/merge ownership passed");
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = OUT / "bin321_probe.cpp"
    source.write_text(SOURCE)
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    executable = OUT / "bin321_probe.exe"
    command = subprocess.list2cmdline(["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/MD",
        "/I" + str(ROOT / "include"), str(source), "/Fe" + str(executable),
        "/Fo" + str(OUT) + "/", "/link", "/MANIFEST:EMBED",
        *(str(ROOT / "build/win32/Release" / name) for name in
          ("bsp_core.lib", "bsp_lua511.lib", "bsp_zlib121.lib")),
        "user32.lib", "d3d9.lib", "PowrProf.lib"])
    batch = OUT / "build_probe.cmd"
    batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
    subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True)


if __name__ == "__main__":
    main()
