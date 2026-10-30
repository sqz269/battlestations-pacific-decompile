# Raw GUI widget resolved position, 00AA6750

Frozen source base: `60c8453a1b661dfa62625945986e38d71bee0216`. This packet implements the complete normal raw body at `00AA6750..00AA67E1`, 146 bytes / 47 instructions. The earlier pause-readiness commit `6326a9e11f65a800d9129bfcf1c3ed0e6ceb122e` identifies this leaf as a prerequisite; its private tree was read only and is not a production dependency. Existing semantic widget and pause interfaces are unchanged. No caller adopts this source here.

## Original and public interfaces

The original receiver is incoming ECX. Its single stacked argument is a pointer to three writable single-precision output fields. Both returns pop that argument with `RET 4`; EAX returns that same pointer. The body saves/restores ESI and does not use or write EDX. Its only call is self-recursion at `00AA6778 -> 00AA6750`. There is no native external callee, allocator, virtual dispatch, null check or cycle guard.

`get_native_gui_widget_resolved_position_00aa6750(void* actual_widget, void* actual_output_xyz, const NativeGuiWidgetResolvedPositionData&)` is a new C++ interface. It forwards the actual storage addresses without a semantic widget conversion. The private helper retains the native ECX/output/EAX/RET-4 convention and additionally borrows EDX for the actual double-cell address. EDX is not modified by any helper instruction, including recursive invocations. The helper is not a binary replacement for the original entry point.

`NativeGuiWidgetResolvedPositionData` is binding metadata, not a native widget or ownership class. Its required constructor calls the existing `GameNativeReadOnlyData::data_at(0x00D7A258, 8)` and retains the returned read-only address. There is no default constructor or literal-cell constructor. The actual authenticated data owner and mapping must remain alive throughout all uses of this binding and its copies; the binding acquires no ownership. The existing data service checks the authenticated mapped range/reservation and may throw while forming the binding. That happens before a getter invocation touches widget/output storage. The getter and its recursion do not call the data service.

The supported original image contains eight zero bytes at `00D7A258`, but the implementation reads the genuine mapped double at its original instruction epoch. It neither substitutes `0.0` nor snapshots that double in the binding. This is the same existing `GameNativeReadOnlyData` service and cell domain used by native GUI text storage. No new global, callback, alternate image mapping or numeric native-code call is introduced.

## Memory, alias and register epochs

Actual widget/parent storage must provide the reached native fields; output must provide twelve writable bytes. The parent chain must terminate for normal return. No null, cycle or container policy is added. Binding metadata, its read-only mapping and private source frames must remain valid, stable and disjoint from writable payload/private scratch. Within those preconditions, receiver/output payload aliases follow the original instruction order rather than a C++ member snapshot.

The parent branch performs these epochs:

1. Compare current `[widget+70h]` with zero, then separately reload `[widget+70h]` into ECX. The comparison result is not reused as the pointer value.
2. Load the selected parent's `+20h`, multiply by its `+18h`, and spill single. Then load `+24h`, multiply by `+1Ch`, and spill single. Both products precede recursion and retain their own rounding epochs.
3. Recursively resolve that selected parent into the original twelve-byte local output area. The recursive call uses the same genuine data binding.
4. Load each returned component and add the corresponding own `+0Ch/+10h/+14h` field. All three sums spill single before the first caller-output store. The Z sum remains in x87 across `POP ESI` before its single spill.
5. Load the saved X sum, subtract the saved X product, and store output X. Then perform the analogous Y subtraction/store. Only after output Y does the body load its saved Z sum, subtract the actual double at D7A258 and store output Z.

The no-parent branch instead loads own X and immediately stores output X, then loads/stores own Y, then loads own Z, restores ESI and stores output Z. It deliberately does not snapshot all three fields or use a three-word copy. An overlapping output can change a later field read; the x87 loads/stores also retain their NaN, denormal and exception behavior.

With native entry ESP denoted E, the helper reserves `20h` bytes and saves ESI at E-24h. Saved products occupy E-20h/E-1Ch; recursive output occupies E-0Ch/E-08h/E-04h; the three own-plus-parent sums occupy E-18h/E-14h/E-10h. The helper's stack operands and all save/restore/output ordering match the listing. Its recursive call receives one output word, with no extra context argument or cleanup word.

## Floating-point contract and limits

The implementation uses the actual x87 instruction sequence, operand widths/directions and single spills. It performs no C++ floating-point arithmetic, precision selection, rounding reset, status clear or MXCSR change. Ambient x87 precision/rounding/masks, prior stack contents and sticky status remain inputs to the same arithmetic and storage operations. There is no new empty-stack precondition. On successful execution, either branch has net x87 depth zero and a maximum extra depth of one; the recursive call occurs at relative depth zero. Parent and no-parent paths have eight and three FSTP epochs respectively. The instruction stream retains single-load/store NaN handling, signed-zero arithmetic, denormal processing, infinities, single overflow/underflow and the late double subtraction rather than algebraically simplifying them.

This is instruction/storage proof at a new source boundary. Actual x87 environment instruction/data addresses, source stack addresses, caller flags/register live-outs, physical private-stack aliases, stack overflow thresholds and original ABI integration are not proven equal. Carrying the cell in EDX changes private register context. In particular, unmasked x87 delivery, SEH/hardware-fault transport, a handler that resumes after modifying registers, and original exception/fault identities have not been validated. No C++ catch, original exception registration, retry or cleanup policy is invented.

## Static evidence

The configured project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; the live target was checked before the original byte/cell queries and call verification. Shared saved assembly and decompilation, original image bytes, fresh live byte output and the genuine cell are pinned in the report. Original body SHA-256 is `4bec621d37131c0143aa7a8df0e4eac74286cc4955adc5ec853b2422055d3c48`.

Strict MSVC Win32 compilation passed with `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX`, no warning. Only a COFF object and assembler listing were produced. The three relevant emitted functions are the binding constructor (27 bytes), public wrapper (36 bytes), and private helper (142 bytes / 47 instructions). The first two contain no x87 or SSE floating-point instructions. The constructor's sole call relocation is the real `GameNativeReadOnlyData::data_at`; the wrapper's sole call is the new helper.

The full private helper comparison passes for all 47 instruction positions, register operands, widths, operand direction, stack offsets, branch destinations by instruction index, spill epochs and output stores. The single data adaptation is native `FSUB qword ptr [00D7A258]` becoming `FSUB qword ptr [EDX]`, reducing the body by four bytes. The self-call relocates to the helper and the short branch displacement follows the shortened body. The helper has exactly one relocation, that self-call; it never writes EDX. Both native/source returns use `RET 4`. The original call verifier checked one row with zero failures. No additional target is inferred from that check.

Retained evidence is in `local/native_gui_widget_resolved_position/` of this worktree: strict compile command/log, object/listing, independent PE/COFF instruction audit and JSON, live body/cell captures, and original-call receipt. Seven existing source/header dependencies are unchanged from the frozen base and fingerprinted. The generated object also contains compiler/library COMDATs from included standard headers; no vendor implementation or additional body credit is claimed.

## Admission boundary

This packet is source-only and unwired. It does not provide AA8240, whole parked-screen handling, the pause hint, raw Game pause, screen/current-owner providers or Game configuration/lifetime adoption. A real caller must supply the actual widget/output storage and a binding from its retained authenticated data owner. Shared registration, integration build/link and independent review belong to the primary. No full build, test, fixture, executable, native probe, game, DLL, Ghidra write, shared ledger or guard change was performed. New native-body, duplicate, vendor and activation credit is zero; runtime and original ABI admission remain open.
