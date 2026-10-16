# Shared GUI white getter: whole Original-body native comparison 42

Packet `cc12_gui_white_getter_native_42` passed against unchanged Main
`7bbd1e4042fe9e74c634a5a6aeadcea819021345`. Published receipt:
`reports/cc12_gui_white_getter_native_42.json`. The ignored probe and its exact
artifacts remain in `local/cc12_gui_white_getter_native_42/` in the worker tree.
No production Source, CMake, Ghidra, game installation or audio setting changed.

The probe executes **all 65 Original bytes** of 00AB7060..00AB70A0, after
relocating only its eight named absolute data operands. It compares that leaf
with the real current `GameNativeGuiTextState::white_color_00ab7060` compiled
from `src/game_native_gui_text_state.cpp` into `current_state.obj`; there is no
replacement implementation in the probe. This is native whole-body behavioral
evidence over controlled backing. The C++ member still has its new typed API,
including its explicit one-value reference, and is not a binary drop-in or
Original fixed-VA global binding.

## Exact Original and complete relocation audit

The installed Original executable is 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Python checks this complete hash before decoding. The probe independently reads
and hashes the full installed file at execution, extracts the leaf from its
validated I386/PE32 section table, and compares every byte with the decoded
audit. A final full-file hash confirms the installation is unchanged.
The Original 65-byte body SHA-256 is
`02f917d1afea24dd3e91ec91546b8d638a43ca2f7768a76058fdf329eb2a4c1c`.

Capstone decodes exactly 11 instructions, consuming every byte: MOV EAX,1;
TEST mask/AL; JNZ; MOVSS one; OR mask/EAX; four MOVSS white stores;
MOV EAX,white; RET. There are no calls, imports or opaque operands. The PE has
relocations stripped and an empty base-relocation directory. The copy therefore
uses the independently decoded data-operand fixups below, rather than assuming
that an empty PE directory makes the copied code position-independent.

| Byte offset | Original absolute target | Exact replacement destination |
| --- | --- | --- |
| 7 | F8BE48, TEST displacement | `&state.default_color_mask_00f8be48()` |
| 17 | D7A24C, MOVSS displacement | Dedicated probe-owned live const float, exact Original word 3F800000. |
| 23 | F8BE48, OR displacement | Same canonical mask reference. |
| 31 | F8BE38, MOVSS displacement | `&state.default_color_00f8be38()[0]` |
| 39 | F8BE3C, MOVSS displacement | Same canonical array, element 1. |
| 47 | F8BE40, MOVSS displacement | Same canonical array, element 2. |
| 55 | F8BE44, MOVSS displacement | Same canonical array, element 3. |
| 60 | F8BE38, MOV EAX immediate | Same canonical array, element 0: returned cache identity. |

Only these eight four-byte fields change. The receipt records all original and
replacement words, all 31 actual changed bytes, and the complete Original and
patched 65-byte hex strings and hashes. The eight operand fields cover 32 bytes;
one operand byte happens to retain its original value in this run. All 33 bytes
outside those fields remain identical, for 34 identical bytes in total.
In particular, JNZ bytes `75 2E` at offsets 11..12 still target offset 59 inside
the copied body; the initial EAX=1 and RET at offset 64 are unchanged. No code
is appended or replaced. The full copy is prepared RW, sealed RX, and followed
by `FlushInstructionCache` before the raw zero-argument/plain-RET invocation.

## One concrete case, two arms, immediate sticky repeats

Both arms use the **same current canonical Source singleton** and the same
one-value object. The probe checks that its 12 compared member words cover the
entire 48-byte current Source object, without padding gaps. All comparisons and
restores go through its real public member references. This is a statement
about that compiled Source layout, not the Original global layout.

The initial Source words match packet 38's verified image/loader preimages.
The one incoming case then sets mask **A5A500A0** (bit 0 clear, other bits set)
and four unequal stale white words **3E800000,C0200000,41140000,BF000000**.
Expected results are formed independently: preserve all other member words,
OR the mask with 1, and copy the Original D7A24C word to all four channels.
The expected path does not call the Source method.

The Original arm initializes and immediately repeats. The full incoming Source
state is then restored, and the compiled Source arm initializes and immediately
repeats. All four returned pointers equal the same canonical white-array
address. Both initialization results are **mask A5A500A1 and four 3F800000
words**. Both sticky results are unchanged. Aspect, vertical scale, every shadow
channel and adjacent blend remain bit-identical throughout. The receipt retains
the complete 48-byte initial/incoming/after/sticky tuples, not just a success flag.

For each sticky call, the one-input page is temporarily **PAGE_NOACCESS**.
Its const-float lifetime begins by placement new in a dedicated aligned
VirtualAlloc page while readable. The const-volatile reference is bound before
protection changes; the allocation and live object remain present throughout.
Forwarding that existing reference to the Source parameter requires no value
read. VirtualProtect changes access permissions, not object lifetime. Completing
each sticky call without an access violation proves it did not reload one.
PAGE_READONLY is restored after each call; old-protection results verify each
transition and a final VirtualQuery confirms READONLY before lifetime ends.
The word is checked again, and only after both arms finish is storage released.
No access-violation handler masks
a failed read, and no dangling or fabricated reference is supplied.

## Build, results and limits

The ignored `compile.cmd` calls the x86 VS 2026 environment, compiles the actual
current Source and probe separately with `/std:c++17 /O2 /EHsc /MD /W4 /WX`,
and links `white_probe.exe` with `/link /MANIFEST:EMBED /DYNAMICBASE /NXCOMPAT`.
The embedded asInvoker manifest and I386 machine type were checked.
The Source object's `DEFAULTLIB:MSVCRT` and the executable's dynamic
CRT imports confirm `/MD`. The receipt records Source/header/probe/code,
object, executable, manifest and log SHA-256 values.

Commands, from the recorded worktree:

```powershell
python local/cc12_gui_white_getter_native_42/audit.py
cmd.exe /d /s /c local\cc12_gui_white_getter_native_42\compile.cmd
& ./local/cc12_gui_white_getter_native_42/white_probe.exe 'I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe' 'local/cc12_gui_white_getter_native_42/native_result.json'
python local/cc12_gui_white_getter_native_42/write_report.py
```

Audit, strict compilation/linking and native execution all exited 0. The report
builder independently rechecks the complete byte diff, allowed fields, expected
results, adjacent state, source cleanliness and final Original hash. This is one
supported scalar case and its sticky repeat, not exhaustive input/concurrency
coverage. The probe does not execute Original startup or surrounding routines,
prove Source binary ABI compatibility, start the game, change audio output,
or validate native Text drawing/gameplay.
