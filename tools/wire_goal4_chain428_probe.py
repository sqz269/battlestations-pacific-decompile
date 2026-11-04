"""One Win32 ON/OFF fixture of the production command queue lifetime; no game run."""
import os
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
out = root / 'local/output/g4_c428_probe'
out.mkdir(exist_ok=True)
source = (root / 'src/game_hosts_session_messages.cpp').read_text()
probe = out / 'lifetime428_probe.cpp'
probe.write_text(r'''
#include "bsp/game_hosts_session_messages.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <malloc.h>

int main() {
    bsp::HostSessionMessages messages;
    std::uint32_t digest = 2166136261u;
    const auto add = [&](std::uint32_t value) { digest = (digest ^ value) * 16777619u; };
    // Repeated synchronous deliveries exercise all three allocated record sizes
    // and payload consumers. Compare the same codec's OFF arm below.
    for (int i = 0; i < 256; ++i) {
        const auto sub = messages.queue_director_sub_kind_5a(i & 7, i);
        const auto clear = messages.queue_clear_command_5d(i & 1, (i & 31) - 16);
        assert(clear.index == (i & 31) - 16 && clear.arm == (i & 1));
        add(sub.sub_kind); add(sub.value); add(clear.arm); add(clear.index);

        bsp::EntityOrderMessage posted{};
        posted.command_ordinal = static_cast<std::uint8_t>(i % 26);
        posted.flags = static_cast<std::uint8_t>(i & 3);
        posted.target_kind = 0; // the image's no-target command variant
        posted.position[0] = static_cast<float>(i);
        posted.position[1] = 0.0f;
        posted.position[2] = -static_cast<float>(i);
        posted.trailing = 1.0f;
        const auto command = messages.queue_command_88(posted);
        assert(command.command_ordinal == posted.command_ordinal);
        add(command.command_ordinal); add(command.flags); add(command.target_kind);
        add(command.target_id);
        for (float position : command.position) {
            std::uint32_t bits;
            std::memcpy(&bits, &position, sizeof bits);
            add(bits);
        }
        assert(_heapchk() == _HEAPOK);
    }
    std::printf("256 command queue deliveries per type; payload digest %08x; heap valid\n", digest);
}
''')
vswhere = Path(os.environ['ProgramFiles(x86)']) / 'Microsoft Visual Studio/Installer/vswhere.exe'
vs = subprocess.check_output([str(vswhere), '-latest', '-products', '*', '-requires',
    'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath'], text=True).strip()
vcvars = Path(vs) / 'VC/Auxiliary/Build/vcvarsall.bat'
results = []
for mode in ('on', 'off'):
    host = out / ('host_' + mode + '.cpp')
    host.write_text(source if mode == 'on' else source.replace(
        'if constexpr (kWireGoal4Chain428Bound)', 'if constexpr (false)'))
    exe = out / ('lifetime428_' + mode + '_probe.exe')
    args = ['cl.exe', '/nologo', '/std:c++17', '/EHsc', '/O2', '/MD', '/fp:strict',
        '/I'+str(root/'include'), str(probe), str(host), '/Fe'+str(exe), '/Fo'+str(out)+'/',
        '/link', '/MANIFEST:EMBED',
        *(str(root/'build/win32/bsp_game.dir/Release'/name) for name in
          ('gameplay_seg38_cc13.obj', 'band_0060_cc13.obj', 'cmake_pch.obj')),
        *(str(root/'build/win32/Release'/name) for name in
          ('bsp_core.lib', 'bsp_lua511.lib', 'bsp_zlib121.lib')),
        'user32.lib', 'd3d9.lib', 'PowrProf.lib']
    batch = out / 'build_probe.cmd'
    command = subprocess.list2cmdline(args)
    batch.write_text(f'@echo off\ncall "{vcvars}" x86 >nul\nif errorlevel 1 exit /b 1\n{command}\nexit /b %errorlevel%\n')
    subprocess.run(['cmd.exe', '/d', '/c', str(batch)], cwd=root, check=True)
    results.append(subprocess.check_output([str(exe)], cwd=root, text=True))
assert results[0] == results[1], (results[0], results[1])
print('ON/OFF identical: ' + results[0].strip())
