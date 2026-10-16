# Preset-tree predecessor 0069AEC0

The complete ordinary leaf `0069AEC0..0069AF48` is reconstructed directly in
`src/native_input_preset_tree_predecessor.cpp`. The established analysis name
`STL_TreeIter_Dec_0069aec0` is preserved; the C++ name is descriptive.
The typed body is 137 bytes and 48 instructions, including the two seven-byte
alignment LEAs at `0069AEF9` and `0069AF19`. Source does not imitate their ESP use.
Only this whole leaf is Source-owned. Family, wrapper and diagnostic bytes below
are read-only support and do not promote hint/insertion/map-index callers.

Native receives the actual eight-byte iterator in ECX, consumes no incoming EDX
or stacked argument, and uses plain RET. Its path-dependent EAX is incidental:
the left maximum path leaves a nil next-link there, while publishing a different
candidate. Both inspected wrappers replace EAX with their iterator after the call.
The Source API is `void __fastcall decrement_native_input_preset_tree_iterator_0069aec0`
with the actual `NativeInputPresetTreeIterator*` and an unused EDX word.
It reuses the published real owner0/node4 carrier; the same no-stack footprint
does not establish drop-in ABI compatibility. The stored unspecified Ghidra
prototype is preserved.

All public DWORD and BYTE accesses are volatile. Links are left0/parent4/right8
and nil is literally byte29. Owner0 is only tested for nullness; a returning
initial diagnostic is followed by a fresh CURRENT node4 load. A nil current node
captures its right link, publishes it to node4, then tests that captured link.
If it is still nil, the diagnostic returns directly, preserving handler changes.

For a nonempty left subtree, the Source carries each captured right link while
finding the maximum and publishes the retained candidate once. Otherwise it
captures the original node's parent. Each climb reloads CURRENT iterator.node
before reading CURRENT parent.left; if equal, it publishes parent before reading
that captured parent's parent. The new parent's nil byte is tested each time.
After climbing, it reloads CURRENT iterator.node and tests its nil byte before
any final retained-parent publication. A diagnostic at this point returns with
no later store. A nonnil current node publishes the retained parent even if that
parent is nil. There is no endpoint repair, owner-equality check or extra guard.

These are the three actual Native transfers in the owned whole body:

| Actual Native instruction | Scope |
| --- | --- |
| `0069AEC8 CALL 00BF6713` | Owned whole leaf |
| `0069AEE3 JMP 00BF6713` | Owned whole leaf, diagnostic tail |
| `0069AF3F JMP 00BF6713` | Owned whole leaf, diagnostic tail |

The existing `invoke_native_invalid_parameter_00bf6713` supplies the qualified
current Windows SDK diagnostic domain. It may return, throw or terminate. Its
Source policy does not reproduce Original BF66EF encoded-global selection,
debugger hooks, dynamic handlers, Watson delivery or fault/return-slot identity.
No callback interface, RawCtx, CRT string service, payload/heap work, local guard,
catch, cleanup, revalidation or rollback is introduced. Only iterator.node4 is
written. Earlier publications remain if the diagnostic throws. Reached storage
must be valid and walks finite; Original faults and SEH are not emulated.

The published nil19 predecessor `00B7CEF0` and its existing callback Source are
read-only comparison evidence. The complete two Native 137-byte bodies differ
only in eight nil-displacement bytes and three four-byte relative transfer
operands; the other 117 bytes match. Both transfer sets reach BF6713. The preset
Source does not call that family or consume its callback interface. Two 12-byte
wrappers and the 16-byte diagnostic are also read-only support. Six provider LF
hashes match the published Main snapshot recorded in the report.

The fresh proof matches all 137 owned bytes and 48 starts against PE and live
Ghidra. Including support, it covers 314 bytes/117 decoded starts, all 29 encoded
edges and 118 current contexts. Project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` were verified. The frozen readiness receipt remains
unchanged. All 82 prior frozen Source-packet artifacts were checked unchanged.

The default `./scripts/build.ps1` MSVC Win32 build exited zero; the three existing
suites passed in 7.78 seconds. No new tests, probes or Native execution were added.
The entire actual CMake object was read: 175 code bytes/75 instructions, comprising
the public function's 121 bytes/48 instructions and four helpers' 54 bytes/27
instructions. Its single alignment NOP is at public offset4F; all returns are
plain RET. Both Native diagnostic tails share generated JMP offset6F. The two
code REL32 relocations at offsets0A/70 bind the existing diagnostic provider.
The third total relocation is debug DIR32NB, not a code call. All directives,
debug/checksum data and relocations were reviewed; MSVCRT/OLDNAMES names remain.
There are no EH sections, literal/data references or additional cleanup paths.

Generated public order retains owner diagnostic before node reload, right
publication before its nil test, CURRENT iterator-node before parent-left load,
parent publication before parent-parent load, and final CURRENT-node nil test
before final store. Compiler registers, flags, private frames, canonical unused
EDX, current SDK policy, FH3/SEH/RTTI/fault/heap/Original ABI and game behavior remain
qualified. Static/build evidence is not runtime, binary-replacement or game proof.
