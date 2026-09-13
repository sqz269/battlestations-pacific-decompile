# Canonical widget base copy (00AA9520)

`GuiWidgetOwnerRuntime::construct_base_copy_00aa9520` implements the normal base-copy caller using the existing layout, owner registry, and actual Model owners. It creates a base admission without running `00AA9390`, the default type factory, or any Text constructor. A derived copy can attach its one lifetime to that owner and later supply its completed implementation through `finish_base_copy_type_admission`.

The actual Model current10 operation is a mandatory service. The Text route needs flags **3Eh**, which the existing flags26h Model/mesh composition does not implement for positive index/vertex stream copies. This packet reconstructs base admission and the caller's stores; it does not close the Text copy factory or those renderer branches.

## Evidence and ABI

Read-only BSP wrappers verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The full stored assembly and decompilation were read. Live body: **00AA9520..00AA9722**, 134 instructions, zero flow gaps, final `RET 4` at `00AA9720` (three bytes). Original ECX is the destination; the one stack argument is the source; EAX returns the same destination. The saved SEH handler is `00CB73F1`. This C++ owner API is not a raw100h object or a binary replacement and does not reconstruct compiler SEH/CRT allocation failure behavior.

All normal call sites are:

| Site | Native operation | Implementation boundary |
| --- | --- | --- |
| 00AA95E3 | 00A9B720, ECX destination+64, RET | Fresh empty canonical child collections represent the new sentinel/count. No copied children and no CRT list implementation. |
| 00AA96FC | Captured source4C current10, ECX actual Model, stack flags,parent0, RET8 | Required `GuiWidgetModelCopyCalls::clone_current10`; current Model D62DE8+10 is B752B0. |

`00A9B720..00A9B739` was read before choosing that list representation: it allocates12 bytes with `00BF681B`, self-links forward/back pointers, and returns. The native allocator's failure behavior is outside the host collections.

`00B752B0..00B753F1` was read in full before naming the Model service. It allocates from01090054, obtains the **current source name** via B6D800, constructs B75030, forwards flags and parent to B6F150, and forwards flags with parent0 to current geometry10. It then associates geometry, consumes its creator, copies retained174 and the ten x87 pose words. The name does not come from the widget type table. B6F150 and the B742A0/B73F50 mesh bodies were also read; the latter's flags08 and10 paths invoke distinct stream-copy operations.

## Exact copy behavior

The base reference count becomes1. The caller copies0C..44 as individual x87 FLD/FSTP pairs. At AA95BA it loads48, clears4C at AA95BD, then spills48 at AA95C0. Colors50..5C are raw DWORD copies, as are A4..C0;94 and C4 use x87. The compiled helpers retain that distinction, including masked signaling-NaN conversion on x87 copies and raw payload preservation on DWORD copies. Alpha's existing transform projection receives the same copied5C word.

Type60 is copied, children start empty, and parent70 is copied without adding this owner to the parent's child list. Bytes74/75/77/78/79/84/D4 and pointersD8/DC, plus E0, are copied. Byte76 becomes1. Pointers88/8C/90 become zero. Layout key/source metadata and derived fields are not synthesized.

Represented destination fields **08, 7C, 80, 85, E4, E8 remain unwritten by AA9520**. Existing layout08/E4 remain in place; the new admission requires explicit allocation preimages for7C/80/85/E8. Float preimages use raw bits. Unknown native98..A0/C8..D0 fields remain outside the existing projection. Boolean projections represent valid logical byte values, not arbitrary malformed byte payloads.

The final source4C and type60 are read at the AA96E4/AA96E7 boundary. The current node profile is captured before reading the live type-indexed flags table; current10 is then checked against the established actual Model profile. The required service receives that actual source Model and parent0. A completed creator must belong to the same runtime's existing model map. The caller publishes it to the same node binding/4C projections, then clears bits0/1 at the actual Model138. Its creator reference transfers to the widget without another retain/release. Actual null returns follow the native null publication branch; there is no missing-provider default.

## D5C0B8 is a flags table

Live bytes for all19 DWORDs at D5C0B8 were verified: indices0..16 are **3E**, indices17/18 are **26**. Text3 reads **D5C0C4=3E**; Section17 and FrameBox18 read26. The D62DE8+10 word is **B752B0**. Earlier scene comments describing per-type names were incorrect; the header/source comments are corrected without changing the old diagnostic helper's type-id interface.

Both26 and3E set20, skipping recursive node-child copy in B6F150. The additional08/10 bits in3E select index/vertex stream cloning in B73F50. The existing `clone_native_gui_text_model_00b752b0` documents its26-only branch and must not be silently substituted. New services must implement the actual3E operation with canonical model/mesh/material owners and preserve temporary creator publications on failure.

The stream helpers were independently read in full, including assembly: B729A0..B72A65 uses the actual F8D394 factory current60 and maps/copies/unmaps both index streams; B72A70..B72B19 uses factory current5C and maps/copies/unmaps both vertex streams. Both take their source in ECX and end in RET. Their factories, mappings, and raw payload producers are not replaced by projected vectors here.

## Admission and ownership limits

Source and destination are distinct owners in one runtime. Source copy and destination admission remain guarded against retirement during the Model callback. Failure after admission preserves the registered incomplete owner, copied fields, and caller-owned acquired references. The operation is not resumable; restarting would duplicate native effects.

`retire_base_copy_admission` is explicit host cleanup for an untyped owner with no attached derived lifetime, children, or timed state. It releases only an already transferred primary through the existing node lifetime domain. Independent acquired creators remain the caller's responsibility. It is not a claim about native copy-constructor unwind. The standard layout cleanup routes untyped admissions here instead of invoking a nonexistent derived implementation.

After complete base copy, the separate Text packet's `GuiTextAfterBaseCopy00aa9520` admission can attach the sole Text lifetime, and its copy continuation performs ABB2C0's remaining operations. Concrete copied `GuiTextRuntimeImplementation` ownership/factory admission remains a required integration. No default Text constructor is run by this packet. The shared owner preflight incorporates the Text packet's `has_incomplete_copy()` guard.

The finish seam takes the caller's `unique_ptr` by reference and transfers it only after preflight succeeds. A rejected admission leaves that sole implementation/lifetime owned by the caller.

## Validation

The ignored fixture `local/widget_copy_probe.cpp` uses production Group and actual Model constructors and links built production libraries. It checks x87 versus raw signaling-NaN handling, destination allocation preimages, no default copy factory, both callback-time retirement guards, and an acquired actual Model creator surviving an explicit flags3E boundary. It does not simulate a successful Model copy. Final4C/138 publication, complete Model3E, derived Text copy, and the game path remain unexercised by that fixture.

Build, report verification, and object-inspection results are recorded in `reports/gui_widget_copy_runtime.json`. No Ghidra changes or permanent tests are included.


## Retained constructor ownership

`GuiWidgetCopySourceBorrow` protects the admitted source across pool callbacks
and pending derived construction. Only its matching token may pass the AA9520
source preflight. This is host lifetime metadata, with no native AddRef or copied
widget state. Releasing the token restores ordinary retirement eligibility.

`begin_base_copy_type_admission` publishes a borrowed implementation owned by
the one constructor operation. `implementation()` resolves that same object for
native constructor callbacks. External owner operations remain blocked while
the constructor pointer or an incomplete Text lifetime exists. The concrete
implementation must distinguish permitted constructor reads from external
mutation. Final admission requires the same implementation and completed Text
continuation before transferring its unique ownership into the widget.

The combined Win32 build and both CTests passed. One existing actual-owner
prefix fixture was extended locally to verify token authorization and retained
source protection after an interrupted Model provider; it passed. Borrowed
constructor dispatch is build-checked here and requires the separate copied
Text runtime integration for execution. No permanent test was added.

## Correction from docs/GUI_WIDGET_MODEL_COPY.md

The concrete adapter now supplies the actual Model flags3E/26 composition previously left as a required provider. AA9520 keeps the same final Model ownership transfer, and validates any outstanding stream creator/companion/map phase with the existing acquisitions. `GuiTextRuntimeCopyOperation` uses the retained source and constructor admission hooks above; full positive copied Text remains dependent on its actual retained native identity and cursor/executable providers. Exact integrated build and positive base-copy evidence are recorded in `reports/orch5_stream_text_copy_batch.json`.
