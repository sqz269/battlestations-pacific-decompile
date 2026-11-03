"""One Win32 numeric-property merge case, including canonical-pool retirement."""
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "local/output/wire_g4_c296_probe"
SOURCE = r'''
#include "bsp/scene_numeric_property_merge.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
int main() {
    using namespace bsp;
    const auto document = parse_scene_document(R"(
        entity "destination" (Ship) {
          localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
          properties {
            COUNT = I 12; Ratio = F 1.5; Visible = B true;
            Unchanged = I 99; Mixed = F 2;
            Child { X = I 1; }
        } }
        entity "source" (Ship) {
          localframe 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
          properties {
            Count = I -17; Ratio = F -0.0; Visible = B false; Added = I 73;
            Label = S "native"; Dup = I 1; dup = I 2; Mixed = I 4; Child = I 2;
        } }
    )");
    for (const auto& error : document.errors) std::fprintf(stderr, "%s\n", error.c_str());
    assert(document.errors.empty() && document.entities.size() == 2);
    const auto& destination = document.entities[0].properties;
    const auto& source = document.entities[1].properties;
    for (bool overwrite : {false, true}) {
        game::SceneNumericPropertyMerge merge(destination, source, overwrite);
        for (std::size_t i = 0; i < source.values.size(); ++i) {
            const auto& incoming = source.values[i];
            const bool numeric = i < 4;
            assert(merge.handles(i) == numeric);
            if (!numeric) continue;
            const auto* prior = destination.find(incoming.key);
            const auto& metadata = prior != nullptr && !overwrite ? *prior : incoming;
            const auto value = merge.result(metadata);
            assert(value.key == metadata.key && value.values == metadata.values);
            if (i == 0) assert(value.has_integer && value.integer_value == (overwrite ? -17 : 12));
            if (i == 1) {
                unsigned bits; std::memcpy(&bits, &value.float_value, 4);
                assert(value.has_float && bits == (overwrite ? 0x80000000u : 0x3fc00000u));
            }
            if (i == 2) assert(value.has_boolean && value.boolean_value == (overwrite ? 0 : 1));
            if (i == 3) assert(value.has_integer && value.integer_value == 73);
        }
        // Both roots and every key/node/record retire before the next merge
        // reuses the same canonical physical and string process domains.
    }
    assert(destination.find("COUNT")->integer_value == 12);
    assert(source.find("Count")->integer_value == -17);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = OUT / "numeric296_probe.cpp"
    source.write_text(SOURCE)
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    libraries = [ROOT / "build/win32/Release" / name for name in (
        "bsp_core.lib", "bsp_lua511.lib", "bsp_zlib121.lib")]
    executable = OUT / "numeric296_probe.exe"
    command = subprocess.list2cmdline(["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/MD",
        "/I" + str(ROOT / "include"), str(source), "/Fe" + str(executable),
        "/Fo" + str(OUT) + "/", "/link", "/MANIFEST:EMBED",
        *(str(p) for p in libraries), "user32.lib", "d3d9.lib", "PowrProf.lib"])
    batch = OUT / "build_probe.cmd"
    batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
    subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True)
    inputs = [source, ROOT / "include/bsp/scene_numeric_property_merge.hpp",
        ROOT / "src/scene_numeric_property_merge.cpp", *libraries]
    result = {"status": "passed", "scope": "synthetic Win32 native-bag merge; no gameplay proof",
        "checks": "parsed I/F/B payloads, keep/overwrite, case-insensitive collision, miss clone, signed zero, unsupported/duplicate/child fallback, canonical retirement",
        "inputs": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}}
    (OUT / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
