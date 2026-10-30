# Raw widget resolved-position independent review

Qualified source pass for `632df2e40a58e33c288b0fe1ae16bc8c516bf593`, frozen parent `60c8453a1b661dfa62625945986e38d71bee0216`. No source blocker was found. The result covers instruction/storage composition at the new C++ interface with genuine retained data, not original binary ABI, unmasked fault transport or caller/runtime adoption.

Only this review document and its report are changed. The four candidate files were read: `native_gui_widget_resolved_position.hpp/.cpp` and the matching source document/report. Existing semantic widget interfaces and callers remain unchanged; the new raw getter has no caller in this candidate. Zero duplicate, new-native-body, vendor or activation credit is assigned.

## Complete original and private helper

The original body is `00AA6750..00AA67E1`, 146 bytes/47 instructions. ECX is the actual widget, its one stacked word is the XYZ output pointer, EAX returns that same pointer, and both exits use RET4. ESI is saved/restored; EDX is neither read nor written. Its sole call is concrete self-recursion at `00AA6778 -> 00AA6750`; the existing checker independently passes that one row.

The live target was checked through BSP wrappers against `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Complete saved pseudocode and assembly, fresh live body/cell bytes and installed PE bytes agree. The original body SHA256 is `4bec621d37131c0143aa7a8df0e4eac74286cc4955adc5ec853b2422055d3c48`; installed executable SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. No function/signature annotation or Ghidra mutation was made; the machine listing establishes this ABI rather than the incomplete analysis prototype.

The 142-byte/47-instruction naked helper preserves the same 20h reservation, saved ESI, recursive output word, operand widths/directions, stack displacements, output ordering and RET4 exits. Source explicitly extends its private ABI with EDX pointing to the actual mapped double. Every recursive helper invocation leaves EDX untouched and uses that same cell address, without another context word or spill.

The independent exact-byte reconstruction changes only:

- Short branch displacement `74 6A` to `74 66`, preserving the no-parent instruction target after four-byte shortening.
- Self-call REL32 operand at offset41 to its COFF relocation; the sole relocation is type20 to the concrete helper, not a provider/stub.
- At original offset103, `DC 25 58 A2 D7 00` (FSUB qword[D7A258]) to `DC 22` (FSUB qword[EDX]).

Every other helper byte equals the original. All 47 instruction positions also match in mnemonic, operand size/direction and effective operand structure after that explicit cell adaptation; branches map to the same instruction index. There is no extra FP operation, hidden value snapshot, cycle check or external callee.

## Required actual cell and owner

`NativeGuiWidgetResolvedPositionData` is borrowed metadata, not widget storage or a mapping owner. It has a required `const GameNativeReadOnlyData&` constructor and no default/literal-cell route. Its actual constructor calls real `data_at(0x00D7A258,8)` and stores the returned pointer. It does not read, convert or cache the double value.

The existing read-only service's ordinary initialization authenticates the complete supported executable, reserves only selected 64-KB bands, copies the verified section bytes and protects committed data pages read-only/non-executable. `data_at` checks nonempty section bounds and ownership of each reached band, then returns the actual recorded address. D7A258 lies in the D7 band (index9 relative to CE0000); that band must have been requested/mapped. Missing coverage throws before a binding exists, with no fallback. Eight zero bytes in the installed image are evidence of the cell contents, not permission to replace its FSUB with a literal or omit it.

The private deferred read-only constructor is only the staged canonical-owner route: `GameNativeCanonicalDataOwner` acquires both subsets, calls the real read-only initializer/authentication, initializes/verifies the mutable side and only then completes ACK/permanent publication. This review does not construct that owner or perform those operations. The existing text-field storage already uses the same real D7A258 service/cell domain.

The actual data owner is noncopyable/nonmovable; its normal destructor releases its owned reservations. The binding and its copies do not retain that lifetime. A caller must keep the same initialized data owner/mapping and stable metadata alive, preserve read-only data immutability and keep metadata/mapping disjoint from writable payload and private scratch. Availability of a permanent canonical route does not make an arbitrary borrowed owner permanent. Forged/dangling metadata is not reauthenticated at getter entry.

## Alias, spill and access epochs

Parent+70 is compared and then separately reloaded into ECX. The parent +20 * +18 product spills single before the +24 * +1C product; both spill before recursion. The recursive call runs at relative x87 depth0 and writes its result into the original local XYZ area.

After return, each returned component is added to the corresponding own +0C/+10/+14 component and spilled single. All three sums are stored before the first caller-output store. The Z sum remains on x87 across POP ESI, then spills. Saved X minus saved X product is stored to output X; saved Y minus saved Y product is stored to output Y; only then does saved Z subtract the genuine double cell and store output Z.

The no-parent branch is deliberately progressive: FLD own X/FSTP outX, then FLD own Y/FSTP outY, then FLD own Z/POP ESI/FSTP outZ. A payload output alias can alter a later input field. There is no three-DWORD snapshot or semantic widget conversion.

With entry ESP=E, saved ESI is E-24h, products E-20h/E-1Ch, recursive output E-0Ch/E-08h/E-04h and sum spills E-18h/E-14h/E-10h. The original pushed-output-word and pop ordering explain the changing ESP-relative offsets. The helper uses those same offsets and restores the same scratch size. Physical source/native frame addresses and arbitrary private-frame aliases are not equated.

## Fresh object and ambient FP checks

Fresh strict MSVC Win32 compilation passes without warnings using `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`. Only a COFF object and assembler listing were produced. The retained static audit was rerun on the fresh object/live byte receipts with only its HEAD assertion adapted from the pre-commit base to the frozen candidate.

| Relevant record | Bytes | Physical check |
| --- | --- | --- |
| Binding constructor | 27 | Integer-only; preserves ESI; pushes8/D7A258 and calls real data_at; stores returned pointer; returns metadata with RET4 |
| Public getter | 36 | Integer-only; saves/restores EBP, reads metadata pointer, loads ECX/EDX, pushes exactly one output argument, calls helper and returns its EAX |
| Private helper | 142 | All47 original instructions mapped; one self-call relocation; no EDX writes; saved ESI and RET4 paths preserved |

All three fresh records equal the author's object and report in bytes and normalized relocation records. The claim of no FP is restricted to the constructor/wrapper records, not unrelated standard-library/compiler COMDATs. Seven existing source/header dependency pins equal the frozen base, all25 report file pins match, and the candidate differs from the base only in its four new files.

The parent path has eight FSTPs; the no-parent path three. Each successful path has net x87 depth0 and maximum additional depth1; recursion occurs at relative depth0. Existing stack contents and capacity remain inputs. No empty-stack/default-PC assumption, control-word/MXCSR change, rounding/mask reset, status clear, C++ floating arithmetic or algebraic replacement is introduced. Single spills and the late qword subtraction retain their native numerical/exception operations under the ambient environment.

## Remaining admission limits

This is static instruction/storage evidence. Original EDX/flags/register live-outs, private frame/address and stack-overflow identity, FPU instruction/data-pointer identity, unmasked x87 delivery, context-resuming handlers, SEH/hardware faults and original binary ABI remain unproved. EDX is an explicit new private input. No fault translation, EH wrapper or default successful callback was added.

Reached actual widget/parent fields must be readable, output twelve writable bytes, and recursion finite for normal return. Stable valid binding/data custody and disjointness are required even on the no-parent path: the wrapper reads metadata first, although that helper branch does not load the double cell. The getter owns neither raw widgets nor their parent graph.

Actual AA8240/pause/screen/Game callers, configuration, owners and shutdown/borrower lifetime adoption remain separate. Parent common registration/build/link is pending and is not inherited from another source batch. This review changed no source, shared CMake, metadata, guard or Ghidra state and touched no frozen peer. No test, executable, allocation execution, body execution, probe, DLL call, full build or game was performed.
