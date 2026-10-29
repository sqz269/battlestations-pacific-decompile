# Dyn radix slot removal and object retirement

`src/cc16_dyn_radix_retire.cpp` recovers complete `00C40CC0..00C40D48`
(137 bytes) and `00C40D50..00C40D6C` (29 bytes). The existing RTTI ledger
identifies the latter as slot1 of `00D7A118`. The helper's descriptive
name is a behavior hypothesis; correct existing RTTI/library names are kept.

The helper receives object in EAX and actual manager in ESI, with RET0.
It saves EBX/EDI, preserves ESI and conditionally saves EBP. The wrapper
receives manager in ECX and one stack object pointer, with RET4, saving
ESI/EDI. Neither body establishes a consumed semantic EAX result. The new
explicit C++ interfaces do not supply these native register/stack ABIs.

The helper reads the object's WORD index at +20h, manager data at +4 and
count at +8. It writes the last packed entry's key then object pointer into
the removed slot and publishes the moved object's WORD index. The original
base/count used for this swap are retained even if the writes alias other
storage; the next count is computed from a fresh manager count.

The remaining branch is deliberately unusual: unsigned `next_count =
count - 1` must exceed both a freshly read current count and capacity +Ch
before allocation. It is an underflow/growth arm, not an inferred ordinary
shrink. Capacity changes before `operator_new_00BF55BE(next_count * 8)`.
Copying uses unsigned live-count comparisons, per-output null checks and a
fresh source base per entry; each entry's two words use that captured base.
Nonnull old storage is freed via `00BF6989`, the new base is published and
the originally computed next count is written even if callbacks changed
the manager count. Unsigned wrap, invalid-address behavior and the absence
of an empty-vector guard are preserved.

The wrapper composes that recovered helper and then frees the same captured
object through original `00BF65AC`. This is an explicit required binding,
separate from `DynLiveCrt::free_00bf6989`; no CRT implementation, fallback,
allocator identity or interchangeable-free assumption is invented. No catch,
rollback, default table or process-global provider is introduced.

Full original assembly and live bytes were checked in the configured saved
BSP project/program. The report verifies the four direct native call sites.
The source is registered in `bsp_core`; common Win32 build receipts and
existing-suite results are recorded in `reports/cc16_dyn_radix_retire.json`.
No new fixture or game run is added for this source packet.

The bodies replace metadata-only placeholders as compiled/unwired source.
Actual manager/object/free custody, native vtable/root reachability and
runtime gates remain required for activation. The already discovered
`00C37210` constructor still needs a genuine vtable binding; neither this
wrapper nor the vtable's recorded pointer fabricates or completes it.

The combined Win32 Release build on accepted `452dea24d` passed, as did
both existing suites (12.09 seconds) and all four native call rows. Receipt:
`local/cc16_dyn_radix_retire_build.log`. Locked naming/comments were saved,
both exports refreshed and previous values retained in
`local/ghidra-annotations-20261029T044036Z.json`. No native-ABI or runtime
acceptance is added by these checks.
