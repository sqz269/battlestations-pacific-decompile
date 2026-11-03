"""One Win32 case for the AI group query views and their x87/ECX bindings."""
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "local/output/wire_g4_c304_probe"
SOURCE = r'''
#include "bsp/game_ai_group_queries.hpp"
#include <cassert>
#include <float.h>
int main() {
    using namespace bsp;
    using namespace bsp::game;
    unsigned control;
    _controlfp_s(&control, _PC_24, _MCW_PC);
    const auto handle = [](std::size_t unit) { return reinterpret_cast<void*>(unit + 1); };
    const std::vector<std::size_t> members{2, 7};
    AiGroupListQuery own(members, handle), other(members, handle), empty({}, handle);
    assert(oct18_tail_c::ai_group_contains_entity_00a2c720(own.data(), handle(2), ai_group_query_invalid_iterator));
    assert(oct18_tail_c::ai_group_contains_entity_00a2c720(own.data(), handle(7), ai_group_query_invalid_iterator));
    assert(!oct18_tail_c::ai_group_contains_entity_00a2c720(own.data(), handle(3), ai_group_query_invalid_iterator));
    assert(!oct18_tail_c::ai_group_contains_entity_00a2c720(empty.data(), handle(2), ai_group_query_invalid_iterator));

    const float origin[3] = {0, 0, 0}, nearby[3] = {649, 900, 0};
    AiGroupLeaderQuery own_leader(origin, 2), other_leader(nearby, 3);
    own.bind_first_entity(&own_leader);
    other.bind_first_entity(&other_leader);
    AiCommandGroupQuery command;
    command.owner_group = own.data();
    AiTuningBlock tuning;
    tuning.field[kAiTuningAutoMergeMergeDist / 4] = 650;
    {
        AiGroupMergeQueryBinding query(tuning);
        assert(oct18_tail_c::ai_merge_strength_00a10d50(&command, other.data(), query.bindings()));
        assert(oct18_tail_c::ai_merge_distance_00a10c60(&command, other.data(), query.bindings()));
        other_leader.class_weight = 1;
        assert(!oct18_tail_c::ai_merge_strength_00a10d50(&command, other.data(), query.bindings()));
        other_leader.class_weight = 2;
        assert(oct18_tail_c::ai_merge_strength_00a10d50(&command, other.data(), query.bindings()));
        other_leader.position[0] = 650;
        assert(!oct18_tail_c::ai_merge_distance_00a10c60(&command, other.data(), query.bindings()));
        // The empty-list path must borrow the image's origin fallback, never
        // dereference the sentinel as a leader. Test the actual body too.
        assert(oct18_tail_c::ai_merge_distance_00a10c60(&command, empty.data(), query.bindings()));
        AiTuningBlock inner = tuning;
        inner.field[kAiTuningAutoMergeMergeDist / 4] = 651;
        {
            AiGroupMergeQueryBinding nested(inner);
            assert(oct18_tail_c::ai_merge_distance_00a10c60(&command, other.data(), nested.bindings()));
        }
        assert(!oct18_tail_c::ai_merge_distance_00a10c60(&command, other.data(), query.bindings()));
    }
    assert(g_ai_group_query_tuning == nullptr);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = OUT / "ai304_probe.cpp"
    source.write_text(SOURCE)
    vswhere = Path(os.environ["ProgramFiles(x86)"]) / "Microsoft Visual Studio/Installer/vswhere.exe"
    vs = subprocess.check_output([str(vswhere), "-latest", "-products", "*", "-requires",
        "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"], text=True).strip()
    vcvars = Path(vs) / "VC/Auxiliary/Build/vcvarsall.bat"
    libraries = [ROOT / "build/win32/Release" / name for name in (
        "bsp_core.lib", "bsp_lua511.lib", "bsp_zlib121.lib")]
    executable = OUT / "ai304_probe.exe"
    command = subprocess.list2cmdline(["cl.exe", "/nologo", "/std:c++17", "/EHsc", "/O2", "/MD",
        "/I" + str(ROOT / "include"), str(source), "/Fe" + str(executable),
        "/Fo" + str(OUT) + "/", "/link", "/MANIFEST:EMBED",
        *(str(p) for p in libraries), "user32.lib", "d3d9.lib", "PowrProf.lib"])
    batch = OUT / "build_probe.cmd"
    batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
    subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True)
    inputs = [Path(__file__), source, ROOT / "include/bsp/game_ai_group_queries.hpp",
        ROOT / "src/engine_seg11_35_66_cc13.cpp", *libraries]
    result = {"status": "passed", "scope": "synthetic Win32 group-query views; no gameplay proof",
        "checks": "member identity/order and empty sentinel, ECX/ST0 class-weight bridge, strict XZ radius boundary, origin fallback, nested tuning lifetime; x87 PC24",
        "inputs": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}}
    (OUT / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result))


if __name__ == "__main__":
    main()
