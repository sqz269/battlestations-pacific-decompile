# Native material texture queries (BY)

This packet reconstructs two complete leaves over existing actual material and
effect-owner storage. Their descriptive source names are hypotheses. The source
uses the same `NativeMaterialStorage` and `NativeMaterialEffectBaseStorage` as
the owner implementations; it creates no private copy or lifetime companion.

The installed `battlestationspacific.exe` at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/` has SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live Ghidra was project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` (64,015 functions). The PE and live listing agree
on the complete native spans:

| Address | Length | Native SHA-256 | Original behavior |
| --- | ---: | --- | --- |
| `00B17320..00B17326` | 7 | `b6072d4fcefdd088e536843289a7c138f74a202b7213f420ff155a3cb220ec2e` | `MOV EAX,[ECX+104h]; RET` |
| `00B17D90..00B17DC0` | 49 | `c54d6769cf94eeab47078b3a800bc340475c1dd8ca0c6373fafd0c49847f913e` | Signed selector and fallback retain |

`00B17320` takes material in ECX, returns its raw DWORD `+104h` in EAX, and
uses plain RET. Its caller is `BSP_MaterialPass_ApplyStateAndDraw` at
`00B43442`; that caller treats the word as a signed alpha-reference state.
The native getter has no null guard or side effect. The new naked fastcall leaf
preserves the entire seven-byte COMDAT without relocations.

`00B17D90` takes effect owner in ECX and a **signed** stacked index, returns a
pointer in EAX, preserves ESI, and uses RET4. It sign-extends the short at +38h,
then compares the index signed. A below-count index loads
`owner+0Ch+index*4`; a nonnull hit returns with **no** increment. There is no
negative-index guard. An out-of-range or null slot captures pointer +98h,
passes captured+4h to the current target of IAT cell `00CE221C`, and **reloads**
owner+98h after that call. The installed PE maps this IAT cell to
`KERNEL32.dll!InterlockedIncrement`. A null fallback still passes address 4h
and retains native fault behavior. The only direct caller is
`BSP_MaterialPass_ApplyStateAndDraw` at `00B434D0`; it decodes a negative
selector with `-1-selector`, pushes the resulting index, then calls this leaf.
That route hands the selected texture to renderer virtual +130h. It does not
balance a fallback retain at the call site; whole-route ownership remains
unproven.

The source's selector retains original ECX, EAX, stacked index and RET4 while
using new EDX for a reference to the actual IAT cell. EDX therefore carries the
cell address; `CALL [EDX]` reads its *current* target only on the fallback path.
The saved ESI holds the actual owner across that external call. Its raw assembly
preserves direct-hit no-retain and post-call +98h reload without bounds repair,
null substitution, target snapshot, or callback stand-in. The additional EDX
input makes this a new source ABI, not a binary drop-in for the original game.
No native C++ EH frame is present in either leaf. The Release MSVC Win32 build
and existing CTest `reconstructed_math` passed. `dumpbin /HEADERS` found the
getter in a seven-byte `.text$mn` section with zero relocations; `/DISASM:BYTES`
showed exactly `8B 81 04 01 00 00 C3`, identical to the installed PE. The
selector occupies a 45-byte `.text$mn` section with zero relocations. Its
disassembly has `MOVSX` and `JGE` for the signed comparison, a raw indexed
load, `JNE` for the borrowed direct hit, `CALL [EDX]` only on fallback, a
second read of `owner+98h` after that call, and RET4. Build and object
inspection establish code properties only; game runtime and renderer lifetime
behavior are not validated here.

The machine-readable evidence and verification status are in
`reports/native_material_texture_queries_by.json`.

## Primary integration at7da09ce9

The combined library passes strict MSVC Win32 compilation, eight native seeds, both CTests and32 numeric call rows. Fresh Ghidra and installed-PE captures match52 spans/12295 bytes. Independent review of all1125 native body lines resolved descriptor count/base capture, world-flag versus width ordering, and the live-one read between decoded component1 FLD/FSTP. Seventeen names/comments were saved and read back, including four new descriptive names; previous comments were preserved.

The existing full original/source fixture compares both4096-byte banks in fallback and optimized skinning variants. It executes27 original bodies/6704 bytes, with shared source B48CE0 and host memmove/invalid-parameter boundaries. Actual texture-source providers and source retained-failure/replay checks also pass. Nonempty renderer/shadow binding, full compiler/pass/debug parents, private ABI/FH3/SEH and game execution remain unvalidated. Worker source6336f486 remains historical; final corrected source is7da09ce9.

Immutable checkpoint: local/checkpoints/7da09ce9/native-material-constant-build-wave/validation.json; SHA256 40e2d9986cf8c3e53cf84d715a1d9aa87081c7df36d8dde6639c4359e8e69b76; 3516 artifacts and7 actual mapped runtime modules. It retains source/build/probe/native-byte/annotation/review inputs and outputs, compiler dependencies, searched libraries and tool/runtime closure.
