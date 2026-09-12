# Actual physical stream conversion leaves

This packet supplies the physical stream's type-query and seek bodies for the
actual-storage conversion path. It uses the existing raw20h physical stream
produced by `construct_native_physical_stream_00bf50d0` and described in
`include/bsp/native_physical_stream_open.hpp`. It introduces no typed stream,
shadow owner, vtable replacement, context publication or dispatcher. Parent
integration of BEF750 and the native runtime binding is a separate change.

Interfaces are in `include/bsp/native_physical_stream_conversion.hpp`, source in
`src/native_physical_stream_conversion.cpp`, and machine evidence and artifact
hashes in `reports/native_physical_stream_conversion.json`. Descriptive names are
hypotheses. These C++ service interfaces are not drop-in binary ABI replacements.

| Original span, inclusive | Bytes | Original ABI and result | Coverage |
|---|---:|---|---|
| BF4FF0..BF5017 | 40 | ECX ignored; one stack DWORD token; AL Boolean; RET4 | complete raw body; saved Ghidra definition pending |
| BF4F20..BF4F3F | 32 | ECX actual stream; distance low/high and origin stack DWORDs; full BOOL in EAX; RET0C | complete |

BF4FF0 has no containing saved Ghidra function at capture. Its exclusive end is
BF5018, not the next aligned entry BF5020. The report preserves all 40 bytes,
their installed-PE match and raw instruction listing; the primary integrator
owns the missing definition and annotation. This worker makes no Ghidra writes.
BF4F20 has a saved body ending BF4F3F. Each live CLI query independently verifies
project `bsp`, program `/battlestationspacific.exe`, language and image base.

## Current type storage

`query_native_physical_stream_type_00bf4ff0(token, ids_0109dc30)` requires the
application's actual three contiguous volatile DWORDs corresponding to
0109DC30, 0109DC34 and 0109DC38. It compares in that order and returns at the
first match. All three are ordinary current data, including zero; the source
does not snapshot them, replace them with token constants, or treat zero as a
sentinel. The caller owns the publication and supplies readable storage.

The complete listing establishes register provenance: BF4FF0 overwrites ECX
with `[ESP+4]`, BF4FF4 loads EAX=0109DC30, BF5000 reads `[EAX]`, BF5004 advances
by four, and BF5007 compares against exclusive-end address 0109DC3C. Only AL is
defined as Boolean on the two exits, BF5010/BF5015 `RET4`; upper EAX bits are
incidental address residue. The incoming stream pointer in ECX is unused.

The D691B0 physical table's +0C word at D691BC is BF4FF0. Direct incoming xrefs
to the leaf are this data slot only, so the saved direct-call graph does not
enumerate virtual consumers. The reviewed conversion consumer BEF750 passes
the current memory token `[0109DBA0]` at BEF78C, restores ECX=ESI, invokes
current source+0C at BEF78F, and tests AL at BEF791. That is a qualified physical
profile observation, not a claim that every indirect caller has been recovered.

## Real Win32 seek

`seek_native_physical_stream_00bf4f20(actual_stream, distance_low, distance_high,
origin)` forwards all three original DWORDs without normalization. It reads
the current HANDLE from actual storage+8 and passes actual storage+10 directly
as SetFilePointerEx's output pointer. No temporary cursor is copied back, so OS
success and failure effects occur on the original field. The method returns
the entire BOOL unchanged and performs no subsequent API call or last-error
translation. Cached size+18/+1C, reference count, identity and untouched+C are
not written by the leaf.

The BF50D0 producer establishes the same raw20h layout: numeric D691B0, refs+4,
HANDLE+8, position+10/+14 and size+18/+1C, leaving +C untouched. No new record
layout is declared. BF4F20 consumes that exact storage and uses no services from
`NativePhysicalStreamOpenContext`, so it does not add a redundant context.

| Call site | Containing body | Native callee | Verified arguments and cleanup |
|---|---|---|---|
| BF4F37 | BF4F20..BF4F3F | KERNEL32!SetFilePointerEx via IAT CE22D8 | HANDLE, signed64 distance bits, direct position+10, full origin; stdcall pops20; BF4F3D RET0C |
| BEF7BC | BEF750..BEF83F | current source+1C; physical D691CC resolves BF4F20 | three zero DWORDs pushed BEF7B4/B6/B8, ECX=ESI; callee RET0C |

Full register/stack provenance for BF4F20: entry ESP+4=distance-low,
ESP+8=distance-high, ESP+C=origin. BF4F20 loads origin to EAX and BF4F24 pushes
it. BF4F25 then reads high at the shifted ESP+C. BF4F29 computes EDX=ECX+10 and
BF4F2C pushes it. BF4F2D reads low at the twice-shifted ESP+C; BF4F31 pushes
high, BF4F32 loads the handle into EAX, and BF4F35/BF4F36 push low/handle. ECX
remains the original stream throughout; no nonvolatile register is modified.
The imported target is independently confirmed by the installed PE import
directory and live Ghidra callee name. There is no internal callee body to
reconstruct for the operating-system import.

## Verification and limits

The accompanying report records the strict MSVC Win32 Release build, existing
CTest and seed-verification outcomes, and the focused ignored probe results.
The probe loads the exact verified 72 original bytes into its own process and
binds CE22D8 to the real SetFilePointerEx. It compares both implementations using
the same current type words and independent real file handles with raw owners
initialized by the existing BF50D0 source constructor. Its checks cover each ID
slot, zero, miss, changed and duplicate IDs; seek-from-begin/current/end, negative
distance, a nonzero high displacement DWORD, invalid origin, invalid handle and
negative-begin failure. It compares full return values, last error, actual owner
bytes, cached-size preservation and the resulting OS cursor. The manifest is
embedded in the probe executable; the installed game is opened read-only only
for evidence extraction.

No permanent test case is added. The report-call checker cannot mechanically
resolve virtual dispatch or operating-system import rows; the raw listing,
vtable bytes and installed import directory provide that evidence. Passing this
fixture establishes these bounded leaf behaviors, not game execution, complete
stream conversion, full ABI replacement or a recovered ID initialization policy.

## Integration correction from docs/NATIVE_PHYSICAL_MEMORY_BINDING.md

The primary defined BF4FF0 from its verified native byte range and saved the reviewed names and evidence. BEF750 now consumes these type/seek methods through the actual numeric D691B0 profile. Four paired conversions exercise original converter and physical method instructions with real handles; reference allocation, backing and wrapper construction use reconstructed dependencies. Final cached and OS cursors agree. The translated error callback and FH3 handler are not installed, so these paired runs do not validate failure or exception paths.

Evidence: reports/native_ao_integration.json; reports/native_physical_memory_binding.json.
