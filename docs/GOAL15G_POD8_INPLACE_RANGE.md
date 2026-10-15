# Complete POD8 in-place range kernels

Addresses: `0041C680`, `0041DAC0`.

| Native extent, inclusive | Coverage | New Source entry | ABI/result |
|---|---|---|---|
| `0041C680..0041C6A3`, 36B | complete | `fill_native_pod8_range_0041c680` | cdecl three stack pointers, plain RET; Source void |
| `0041DAC0..0041DAFD`, 62B | complete | `copy_backward_native_pod8_range_0041dac0` | cdecl three stack pointers, plain RET; EAX captured destination endpoint |

Existing library names `STL_Pod8Range_Fill_0041C680` and
`STL_Pod8Range_CopyBackward_0041DAC0`, comments and inventory categories remain.
Both public APIs consume actual byte endpoints and pointed raw words. No private
pair object, new receiver layout, helper call, allocator or fake provider exists.
The literal MSVC Win32 naked bodies reconstruct these exact addresses rather
than substituting other-address range analogs.

## Fill schedule

Capture first into EAX and last into EDX from stack+4/+8. If equal, return before
even loading the value-pointer argument at stack+Ch, saving ESI or accessing
pointed storage. Otherwise capture the value pointer once into ECX and save ESI.
Each iteration reads source word0, writes destination word0, THEN freshly reads
source word1 and writes destination word1. The first store can change that
second source read. Source contents are reread on every iteration; they are not
snapshotted as a pair. ADD EAX,8 modulo DWORD, compare to captured last, repeat
by inequality. Restore ESI and plain RET; the caller cleans all three arguments.

No null destination skip, numeric ordering/count guard, memmove, typed assignment
or C++ pointer difference is added. Physical EAX equals captured last upon normal
return; Source returns void because no original semantic return is established.

## Backward-copy schedule

Capture last into ECX and destination-end into EDX before saving ESI, then load
first into ESI from the shifted stack slot. Compute the EAX result before the
empty test using literal DWORD instructions:

`destination_end - (SAR32(uint32(last-first),3)*8) modulo 2^32`.

The SAR is arithmetic; multiplication is three ADD-to-self operations. No signed
C++ pointer subtraction, division or rounded-up count replaces it. Save EDI;
the result remains captured even if the range is empty or later stores alias
argument/storage words. Empty returns this endpoint with no pointed access.

For a nonempty range, EDX becomes destination_end-original_last modulo DWORD.
Each iteration reads word0 at old ECX-8 BEFORE decrementing ECX. SUB ECX,8; CMP
ECX,first; write that word0 at EDX+ECX; freshly read word1 at current ECX+4 and
write it at EDX+ECX+4. JNZ uses the CMP flags set BEFORE the two stores; MOVs do
not replace them. A word0 store can change the following word1 read. Continue
by equality, then restore EDI/ESI and plain RET, leaving the captured EAX result.
There is no null skip, pair snapshot, memmove or external dependency.

Both loops use +/-8 and equality modulo DWORD, rather than ordered ranges.
The reached walk must terminate and every actually accessed source, destination,
stack and control cell must be backed. Existing C7E0/19960 null-skipping kernels
and byte-range/memmove helpers implement different schedules and are not used.

## Actual insertion consumers

The three consumers were checked in the live containing body
`00420F80..004211C0`, including their stack cleanups:

```text
00421157 CALL 0041C680
00421196 CALL 0041DAC0
004211A6 CALL 0041C680
```

At the first fill, push the actual local-pair address, saved old end after the
current-end count publication/reload, then insertion position; ADD ESP,0Ch at
`0042115C`. At backward-copy, push old end EBX, saved old-end-countBytes, then
insertion position. The preceding copy-dispatch EAX is published to owner+8 at
`00421193` before entering backward-copy. Its three arguments remain stacked. The final fill
pushes actual local-pair address, insertionPos+countBytes, insertionPos; ADD
ESP,18h at `004211AB` cleans BOTH this fill and preceding backward-copy arguments.
The fill EAX is unused at both sites; the copy endpoint remains physically defined.

Live xrefs and a physical call census find exactly two C680 calls and one DAC0
call. The nearest-start census/index owner can label them `Catch_All@004210E7`;
live containing-function queries identify `00420F80`. The former heuristic is
not used to invent a separate cleanup caller or rename a consumed address.
No reconstruction of the larger insertion body is claimed by this packet.

## Verification and boundaries

Project/program checked: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
base00400000. Both complete listings, live extents/prototypes, all three caller
argument/cleanup sequences and the prior read-only handoff were reconciled.
Strict MSVC Win32 `/MD /std:c++17 /EHsc /W4 /WX /permissive- /O2 /fp:strict /c /FAcs`
passes. These functions have no helper dependencies or local EH metadata.

`local/goal15g_pod8_inplace/compare_pod8_inplace_bytes.py [COFF-object] [live-byte-directory] [PE-path]`
accepts the primary registered object. All 36+62 owned COFF bytes equal both
fresh live captures and the original executable bytes, with ZERO masking and no
owned relocation. Final RET bytes are included. Default byte captures and object
are in that same ignored directory; PE defaults to config/target.json's binary.

One ignored Source fixture, `local/goal15g_pod8_inplace/pod8_inplace_probe.cpp`,
targets concrete alias risks: a word0 fill store modifies source word1 before its
read, and a two-element backward copy has the same word-level alias. Both expected
raw arrays and captured endpoints pass. A separate span in the same fixture uses
real-backed source/destination words with destination-end below source-last:
the DWORD offset underflows and effective-address addition wraps back to admitted
storage. Equal-range no-value-access and captured empty-copy endpoint also pass.
This is bounded Source alias/address evidence, not a Native differential or a
dynamic test of every sign-bit/misalignment/fault combination.

Run `local/goal15g_pod8_inplace/run_pod8_inplace_probe.cmd [registered-archive]`.
With no argument it links these two new kernels directly; with an archive it
links only the fixture and registered archive. The Win32 probe embeds a manifest.
Source fixture, exact-byte check and report/diff checks pass. Primary owns CMake
registration, full registered build/archive reproduction, saved annotation/export
and integration. No tracked test target, installed replacement, original C++
template/return type, Native runtime/SEH or game validation is claimed.
