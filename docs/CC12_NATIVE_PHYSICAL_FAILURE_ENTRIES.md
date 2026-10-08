# Native physical failure entries

Two additional raw Source entries now have complete production COFF bodies exactly equal to Original: callback `00530620` is **1 byte**, and notifier `00BD9E30` is **11 bytes**. Equality is unconditional for the entire bodies, with **zero code relocations and zero normalized bytes**. The normal Release/Win32 build and all three existing checks pass.

This is static Source/object/archive evidence. Neither new entry nor its corresponding Original body was executed. No linked/loaded-body claim is made for the uncalled COMDATs. A genuine callable-domain manager remains unavailable, and raw BF5030 or a connected read-failure path is not admitted.

## Exact bodies and ABI

| Original | Additional Source entry | Complete emitted bytes |
| --- | --- | --- |
| `00530620` | `raw_ignore_native_vfs_mount_failure_00530620` | `C3` |
| `00BD9E30` | `raw_notify_native_vfs_request_failure_00bd9e30` | `8B 81 90 00 00 00 FF D0 C2 04 00` |

Names and C++ types are descriptive hypotheses, not recovered symbols. The notifier's explicit `void __fastcall` signature models actual manager in ECX, forwarded DWORD in EDX, and one discarded stack DWORD. There is no recovered semantic return value.

The callback is a naked plain near return. It has no semantic inputs or result, needs only a valid normal call stack, consumes only the return address, and leaves non-stack registers and arithmetic flags unchanged. `_emit 0xC3` fixes the exact opcode: the first normal build showed MSVC encodes plain inline-asm `ret` in this declaration as `C2 00 00`. That failed object/archive gate is retained. The genuine Original function really is C3; this is not an invented empty failure fallback.

The notifier is exactly `MOV EAX,[ECX+90h]; CALL EAX; RET4`. It reads neither global `0109CEEC` nor manager+18. It forwards incoming ECX and EDX to the actual target without pushing callback arguments; RET4 discards one caller DWORD. It does not require EDX and that word to be equal. BF5030's separate field18 loads are a caller contract.

If notifier entry ESP is S, the callback return address is pushed at S-4. The callback must return without popping arguments; then the notifier's RET4 leaves its caller at S+8. For the genuine C3 target only, EAX retains the loaded actual target address, ECX/EDX remain unchanged, and other non-stack registers and flags are unchanged. A `void` Source declaration does not clear or assign semantic meaning to that observable EAX state.

## Genuine borrowed-manager requirement

The caller must lend a genuine already-produced initialized actual manager whose lifetime spans the call. Its readable +90 DWORD must hold a directly callable executable address for the genuine C3 leaf, either Original or a separately qualified exact Source counterpart, in the same process/code domain. The callback must have the zero-pop return ABI. A numeric Original identity or a fabricated readable byte buffer with a convenient field value does not establish this input.

No manager was produced, no callback pointer was installed, no global was substituted, and no notifier or native/SDK probe was run. Existing Source startup still stores numeric identity `00530620`; its typed virtual identity dispatch remains distinct from a direct callable pointer. The new functions do not change those production routes. A connected raw read additionally needs actual `0109CEEC` publication and field18 provenance.

## Production artifacts and preservation

The actual normal-build object is **912 bytes**, SHA-256 `e10015963caa24ee220fbe6557482eda4ebfabca35ab5cf8b3d1df21497c242b`. Its two complete COMDAT code sections are 1 and 11 bytes, each with zero code relocations. The whole object occurs exactly once inside the whole actual `bsp_core.lib`, SHA-256 `6f8cd8b48cb62eb5e8a952b6ba80c5813a0ad1448ae34ce1e7d316df7f997a55`. The member starts at archive offset `15638694` and matches all 912 object bytes, not only an instruction prefix.

The five old provider objects remain byte-for-byte identical; all **702** complete function bodies and ordered relocations also agree, and each whole object remains an exact unique archive member. All ten old CPP/header files remain unchanged. This covers the typed notifier, ordinary RET0 callback, numeric installer, runtime identity bindings, and typed physical read. Only the new header/source/document/report and one appended CMake source line belong to this change.

## Build and retained evidence

`scripts/build.ps1 -Diagnostic` passed the normal Release/Win32 build and the three existing checks: `reconstructed_math`, `native_math_differential`, and `tool_tests`. No permanent test was added. These checks keep their normal scope; they do not exercise the new entries.

Evidence family: `local/cc12_native_physical_failure_entries_source_20261008a/run02`. Manifest SHA-256: `401692a00fc845be2347f4a4ca7353dee78fcc3f6843447cae9a9fbed73b3a91`. Whole COFF/archive proof SHA-256: `e3a2829fbd6040c4de14fa4215e48b0672b74529d476bad0bebef26d3058d507`.

There are **315 physically retained rows**: 296 captured before the normal build and 19 after it. The pre-frozen Source, header, recipe, and selected tool files remained equal afterward. Nine actual module include inputs were selected from 203 physically copied header candidates; the other 194 candidates are retained copies, not claimed consumed inputs. The CPP itself is separately retained and named by the actual normal compiler command.

The compiler read log also recorded `SORTDEFAULT.NLS` and `TZRES.DLL`. They were discovered and copied after compilation, with no pre-build hash claim. This qualification avoids an exhaustive compiler/OS closure claim. Both static gate receipts remain available: run01's rejected C20000 leaf and run02's missing ambient-file preimage check. The final static proof uses the corrected successful run02 object and archive; no third build or target execution was performed.

Machine-readable details: [cc12_native_physical_failure_entries.json](../reports/cc12_native_physical_failure_entries.json). Further notifier execution requires the missing genuine callable-domain owner; body identity and a successful build do not supply it.
