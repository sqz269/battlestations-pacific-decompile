# GuiGroup factory branch-input review

Review base: `33d4b470b7ec752da670c1d2807a41389cd76dab`. Sealed candidate: `476d3408a74ba5651d434dc49d50e926fb987883`. Decision: **accepted for this bounded Source interface correction, with no findings**. Primary common build/link remains separate.

This independent review owns only this document and `reports/cc16_native_gui_group_factory_inputs_review.json`. It performs static Source, original-PE and retained-object inspection. It compiles, links and executes nothing and changes no Source, CMake, shared metadata or Ghidra analysis.

## Retained evidence

Readiness commit `412722191f4d5b015380edf55e88f3ba527f6969` is frozen. All 35 audit/artifact receipts match their hashes and lengths; all 41 Source/config pins match both that worktree and this exact review-base Git tree. The retained analysis target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. No fresh live batch is needed.

The installed original PE matches SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Independently mapping its addresses to raw PE sections confirms all 15 retained ranges: complete `AA6560`, `AA12F0`, `AC6F50`, `AC6FA0`; jump table, profile and adjacent boundary, actual flags, one and name data; handler, FuncInfo, unwind map and both actions. The four bodies total 325 bytes/99 instructions. The full listings and Source contracts were inspected. This reuses retained live proof and grants zero new Native credit.

## Verified Source contract

Exactly four candidate files change: the factory header/Source and the author's document/report. All four sealed Git blobs, 13 artifact receipts, two edited-source pins and 39 unchanged provider pins match. The report's worktree/Git byte difference is CRLF only; the author worktree is clean. The complete changed Source and all repository call sites were inspected.

The closed input type couples default to genuine `D7A24C`, or copy to the actual raw source-start byte reference, genuine context and fresh frame. Its source binding and active union metadata are private and immutable. Binding the byte performs no value load, copy or prefetch. No independent source pointer can mismatch the alternative. The legacy optional-pointer API constructs its matching alternative and delegates one complete factory body, without a new error, fallback, fake input or input-domain restriction. Passing the closed value copies binding metadata only.

Source capture precedes allocation. A null allocation reads neither constructor input set. Frame validation stays inside `AA9520` after successful allocation. Default retains the real raw base's two one reads around sentinel allocation. Copy retains actual `AA9520`, current source `+60/+4C` and flags epochs, canonical Model26/3E,parent0 and branch-reached services. Both factory unwind states return only the captured slot after inner cleanup; diagnostic custody, absence of completed-clone rollback and the existing terminate-if-slot-return-fails policy remain intact.

## Complete strict object review

The author retained successful current/baseline MSVC x86 19.51.36244 `/c /O2 /EHsc /fp:strict /W4 /WX` logs. This reviewer invoked no compiler. An independent raw COFF parser checked all 22 current and 16 baseline sections, primary symbols, bytes, characteristics and relocations. Independent Capstone decoding matches every instruction: all eight current code sections, 481 bytes/174 instructions, including padding. Both full decoded listings were read.

The 29/18-byte input constructors and 63-byte legacy delegator access binding metadata only. The complete shared factory is 206 bytes. It captures source into EDI at `+2B`, calls allocation at `+2E`, records the slot at `+35` and tests null at `+38`. Only success enters state0 at `+4E`; copy reference slots are consumed at `+59/+5C`, raw `AA9520` at `+61`, and default one binding/raw `AC6F50` at `+83/+87`. There is no source, one, context or frame-content prefetch.

All 140 EH bytes and six relocations were independently decoded: two catch-all HandlerTypes, four unwind entries, full FuncInfo and both try records. Allocation is state -1. The outer catch at `+A6` reloads the recorded slot, enters state2 and calls `AC73D0` at `+B0`; successful return restores state1 before rethrow at `+C3`. The inner catch at `+A1` calls terminate. The entire security-cookie/FH3 handler and SafeSEH index62 resolve to the shared overload. No successful-payload cleanup appears.

Nine independent old/current comparisons pass: unchanged copy and scalar-delete bodies, compiler helper, EH handler, directives, both constant-data sections, zero-fill storage and complete EH data. Copy is 36 bytes/one relocation; scalar deletion is 58 bytes/four relocations, preserving late low-byte flag reading after `AA9730`. EH-owner signature normalization is limited to the old factory and shared overload; debug/checksum metadata and shifted indices carry no behavior claim. No production caller of either factory entry was found.

The real UI Group pool is explicit `CD7440 -> AC7410 -> atexit(CE0AE0)`, `F8BFA0`, payload `EC`, slot `F0`, hidden index `+EC`. The ModelGroup root's `188/18C`, `010902F4` and `D634F8` are separate. This correction establishes no current Host, Group lineage, Page-child adoption, lifetime admission, ABI/FH3/SEH/fault, runtime or activation proof. The primary integrator owns any common build/link validation.
