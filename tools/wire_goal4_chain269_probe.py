"""Check the chain269 adapter against the original GPU table and a Win32 fixture.

Runs no game or hardware survey. Extracts the actual host adapter/methods; tests
one synthetic owner with known/unknown GPUs and the switch OFF. Requires MSVC.
"""
from pathlib import Path
import hashlib
import json
import os
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'local/output/wire_g4_c269_review_probe'
OUT.mkdir(parents=True, exist_ok=True)
binary = Path(json.loads((ROOT / 'config/target.json').read_text())['binary'])
image = binary.read_bytes()
pe = struct.unpack_from('<I', image, 0x3c)[0]
optional = pe + 24
base = struct.unpack_from('<I', image, optional + 28)[0]
section_start = optional + struct.unpack_from('<H', image, pe + 20)[0]
sections = [struct.unpack_from('<IIII', image, section_start + i * 40 + 8)
            for i in range(struct.unpack_from('<H', image, pe + 6)[0])]


def dword(address):
    for size, rva, raw_size, raw in sections:
        if rva <= address - base < rva + max(size, raw_size):
            return struct.unpack_from('<I', image, raw + address - base - rva)[0]
    raise ValueError(f'unmapped original address {address:08x}')


catalog = (ROOT / 'include/bsp/wire_goal2_chain46_gpu_catalog.inc').read_text()
rows = re.findall(r'\{0x([0-9a-f]+)u, "([^"]+)", 0x([0-9a-f]+)u\}', catalog)
scores = [dword(0x00e0cff0 + row * 12) for row in range(len(rows))]
assert rows and all(int(row[0], 16) == dword(0x00e0cff4 + i * 12) for i, row in enumerate(rows))
assert scores[0] == 1
assert scores == [1] + [int(row[2], 16) for row in rows[:-1]]
old_mismatches = sum(int(row[2], 16) != score for row, score in zip(rows, scores))
print(f'original sha256={hashlib.sha256(image).hexdigest()} rows={len(rows)} '
      f'old_score_mismatches={old_mismatches}')

source = (ROOT / 'src/game_hosts_init_tail.cpp').read_text()
helpers = source[source.index('const hardware::GpuTableRow* probe_gpu_catalog()'):
                 source.index('class HardwareProbeMessages final')]
owner = source[source.index('struct ProbeHardwareImage {'):source.index('// Adapts the Windows survey')]


def method(anchor):
    start = source.index(anchor)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


writer = method('void write_default_options_file(const std::string& path) override {\n        if constexpr')
writer = writer.replace(' override', '').replace('kWireGoal4Chain269Bound', 'Enabled')
formatter = method('const char* build_default_options_text_0098e910() override {')
formatter = formatter.replace('{', '{ ++format_builds;', 1)  # Count entry, leave adapter unchanged.
cpp = r'''
#define _CRT_SECURE_NO_WARNINGS
#include "bsp/band_0090_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/oct18_tail_c.hpp"
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
namespace bsp::game {
namespace hardware = engine_seg11_35_66_cc13;
''' + helpers + owner + r'''
struct ProbeLog {
    void implemented(const char*, const char*) {}
    void notef(const char*, ...) {}
};
struct OldWriter {
    int calls{};
    void write_default_options_file(const std::string&) { ++calls; }
};
template <bool Enabled>
struct MachineFixture final : hardware::OptionsTextBuilder {
    ProbeHardwareImage image_{};
    OldWriter owner_;
    ProbeLog log_;
    std::uint32_t sound_count_{1};
    int format_builds{};
''' + writer + '\n' + formatter + r'''
};
}
int main() {
    using namespace bsp;
    const std::vector<std::uint32_t> expected{NATIVE_SCORES};
    const auto actual = game::probe_gpu_scores();
    if (actual != expected) return 1;
    const auto* rows = game::probe_gpu_catalog();
    std::int32_t high = 0, quadro = 0;
    while (rows[high].name && expected[high] != 3) ++high;
    while (rows[quadro].name && std::strcmp(rows[quadro].name, "Quadro2 Pro")) ++quadro;
    if (!rows[high].name || !rows[quadro].name || actual[quadro] != 0) return 2;
    game::MachineFixture<true> on;
    auto& image = on.image_;
    image.fields_28.gpu_index_28 = high;
    image.fields_28.memory_mb_2c = 1000;
    image.fields_28.cpu_mhz_30 = 3200;
    image.shader_model_24 = 2;
    image.language_c38 = reinterpret_cast<void*>(0x407);
    std::pair<std::uint32_t, std::uint32_t> modes[]{{1280, 1024}};
    std::uint32_t samples[]{4};
    image.vectors_00.words[1] = reinterpret_cast<std::uint32_t>(modes);
    image.vectors_00.words[2] = image.vectors_00.words[1] + sizeof modes;
    image.vectors_00.words[5] = reinterpret_cast<std::uint32_t>(samples);
    image.vectors_00.words[6] = image.vectors_00.words[5] + sizeof samples;
    on.write_default_options_file("on_options.txt");
    std::ifstream file("on_options.txt", std::ios::binary);
    const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (on.format_builds != 2 || on.owner_.calls || text != image.default_options_c3c) return 3;
    for (const char* s : {"Resolution 1280 1024\n", "Antialias 4\n", "SoundEnabled 1\n",
                         "Language deutsch\n", "EAX 0\nSoundSW 0\nLog 0\nCheatsOnAtStartup 0\n"})
        if (text.find(s) == std::string::npos) return 4;
    image.fields_28.gpu_index_28 = quadro;
    if (image.options_detail() != 0) return 5;
    std::remove("unknown_options.txt");
    image.fields_28.gpu_index_28 = -1;
    for (std::uint32_t unproduced : {0u, ~0u}) {
        image.unused_20 = unproduced;
        on.write_default_options_file("unknown_options.txt");
        if (image.fields_28.gpu_index_28 != -1 || on.format_builds != 2) return 6;
    }
    if (on.owner_.calls != 2 || std::ifstream("unknown_options.txt").good()) return 7;
    std::remove("off_options.txt");
    game::MachineFixture<false> off;
    off.image_.fields_28.gpu_index_28 = high;
    off.write_default_options_file("off_options.txt");
    if (off.owner_.calls != 1 || off.format_builds || std::ifstream("off_options.txt").good()) return 8;
    std::printf("PASS: %zu original scores, Quadro2 Pro, real callbacks/writer, unknown old path, OFF identity\n", actual.size());
}
'''
cpp = cpp.replace('NATIVE_SCORES', ','.join(f'{score}u' for score in scores))
unit = OUT / 'options_probe.cpp'
unit.write_text(cpp)
vswhere = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
vs = subprocess.check_output([str(vswhere), '-latest', '-products', '*', '-requires',
    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
vcvars = Path(vs) / 'VC/Auxiliary/Build/vcvarsall.bat'
command = subprocess.list2cmdline(['cl', '/nologo', '/std:c++17', '/EHsc', '/MD', '/O2', '/Gy', '/W4', '/WX',
    '/fp:strict', f'/I{ROOT / "include"}', str(unit), str(ROOT / 'src/band_0090_cc13.cpp'),
    f'/Fe{OUT / "options_probe.exe"}', f'/Fo{OUT}/', '/link', '/MANIFEST:EMBED', '/OPT:REF',
    str(ROOT / 'build/win32/Release/bsp_core.lib'), str(ROOT / 'build/win32/Release/bsp_lua511.lib'),
    str(ROOT / 'build/win32/Release/bsp_zlib121.lib'), 'd3d9.lib', 'user32.lib', 'gdi32.lib',
    'ole32.lib', 'advapi32.lib', 'shell32.lib', 'PowrProf.lib'])
batch = OUT / 'compile.cmd'
batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
built = subprocess.run(['cmd', '/d', '/c', str(batch)], cwd=OUT, text=True, capture_output=True)
(OUT / 'build.txt').write_text(built.stdout + built.stderr)
if built.returncode:
    print((built.stdout + built.stderr)[-5000:])
    raise SystemExit(built.returncode)
run = subprocess.run([str(OUT / 'options_probe.exe')], cwd=OUT, text=True, capture_output=True)
(OUT / 'run.txt').write_text(run.stdout + run.stderr)
print(run.stdout + run.stderr, end='')
raise SystemExit(run.returncode)
