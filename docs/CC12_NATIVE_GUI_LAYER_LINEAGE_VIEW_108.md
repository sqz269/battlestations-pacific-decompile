# Native GUI Layer live lineage input (packet 108)

This packet adds an explicit Source input capability for the already recovered
`00AA38B0` Layer class query. It does not recover a new algorithm or construct the
Layer lineage. The actual Layer producer and retained authority remain open.

## Source change and admission

`include/bsp/gui_widgets_cc13.hpp` retains the exact existing
`gui_type_is_kind_of(const GuiTypeDescriptor<Depth>&, uint32_t) noexcept`
signature. Both it and the new required `const volatile uint32_t (&)[Depth]`
overload use one indexed `detail::gui_type_ids_contain` implementation. The typed
descriptor's behavior is unchanged. Neither route reads the descriptor name.

The named, noninline `native_gui_layer_is_kind_of_00aa38b0` wrapper is defined in
the already registered `src/gui_widgets_cc13.cpp` and requires exactly three
borrowed live ID words. There is no default, copied descriptor, additional
counter, initializer, state, owner, callback or address-to-pointer conversion.
Zero is an ordinary matching ID. Each word is read only after the preceding
word missed; a hit returns before any later word is read.

The caller must supply the same genuinely initialized, live Layer lineage from
the intended type domain, preserve its storage lifetime throughout the call,
and externally serialize access with its actual producer. Volatile qualification
preserves these individual Source reads; it supplies neither synchronization nor
an ownership protocol. This packet does not provide the actual `00F8BF88`
authority or the `00CD73F0` producer, and does not permit a private parallel array
to stand in for that authority. See packet 105 for the still-open producer.

## Original and Source boundaries

Fresh verified-wrapper queries identify `BSP_GuiLayer_IsKindOf` at
`[00AA38B0,00AA38D8)`: 40 bytes, 12 instructions, no calls or exception frame.
The function ignores the ECX receiver, loads the ID from `[ESP+4]`, scans current
DWORDs in `[00F8BF88,00F8BF94)` in order, returns a boolean in AL and executes
`RET 4`. It does not read a name word or run initialization. Only AL is established
as the return value; the listing does not establish a full-EAX boolean ABI.

The new wrapper has an ordinary two-argument Source C++ interface and `bool`
return. It is not a binary replacement for the Original receiver/stack/AL ABI.
There is no new fixed-address binding, upper page-dispatch adapter, concrete
Layer producer, production graph admission, or normal Text activation.

The fresh live bytes equal the unchanged installed Original executable bytes.
The body SHA-256 is
`0a40e1d1e3b89248d0082369363f10f5c299108fabd79de001948e49f6771d05`;
the executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All live batches used the BSP wrappers verifying `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`.

## Compilation and evidence

Baseline merge `d15ba1ba6d9faec2b71e73fa0d4c5dd30c56bda2` completed cleanly
(exit 0), producing own-worktree baseline `98c8cc6cbeac42c301eef8f0fef244488bc5c071`.
No Main, CMake or unrelated Source edits are included. Existing registration is
`cmake/startup.cmake:764`; the generated Win32 project contains this exact TU.

The strict standalone MSVC Win32 compile passed with `/MD /O2 /W4 /WX /EHsc`.
In its optimized wrapper, offset `10` performs the sole current ID read,
`cmp [edx+eax*4],ecx`; offset `13` branches directly to the success return on a
hit. Only a miss increments the index, and the bound is three. Thus no later
volatile read occurs after an earlier hit, and zero needs no special path.
Root reviewed the complete Source diff and optimized scan and found no blocker.

`scripts/build.ps1` passed (exit 0). All three existing CTest checks passed in
9.50 seconds. The real canonical `build/win32/Release/bsp_core.lib` contains the
named wrapper symbol. No new tests, probe, Native differential or wrapper
execution was added. Compilation and static instruction review do not prove
initialization, real lineage identity, Original ABI compatibility or gameplay.

## Annotation and receipt

Only owned `00AA38B0` was annotated through the locked annotation tool, with
previous values recorded, prior comments preserved, project saved and affected
exports refreshed. The established `BSP_GuiLayer_IsKindOf` name and original
typed reconstruction record remain unchanged. The name evidence appends the
verified whole extent to supersede its historical no-function/end-address text;
a distinct raw-input record documents this new Source capability. All unowned
rows in both touched ledger shards remain unchanged.

The machine receipt is
`reports/cc12_native_gui_layer_lineage_view_108.json`. Ignored packet artifacts,
including the optimized scan, build log, original bytes, annotation history and
refreshed exports, are sealed by
`local/cc12_native_gui_layer_lineage_view_108_artifact_manifest.json` before any
future build reuse. The leaf has no direct calls; no zero-row call-map verifier
is represented as behavioral evidence.
