# Native platform stop scheduling audit

No normal-shutdown call to platform slot `+8` (`00bebf70`) is established by
this audit. The proven call is `00becf27`, inside window configuration, gated
by an already non-null HWND. Native shutdown and the inspected platform
destructors do not themselves call that slot. This is bounded static evidence,
not proof that no transitive callback or unindexed caller can ever restore it.
Production power-policy initialization/restore should remain unbound until
that missing lifetime contract is resolved; this packet adds no host schedule.

All live batches verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Ghidra was read-only. The source implementation and
fixtures from PLATFORM_CONTROL_MESSAGES_CC10 are unchanged.

## Proven call and ordinary exit order

The first 11 inspected vtable cells at `00d68cc4` contain BECE30 at slot zero, BECEE0 at +4,
BEBF70 at +8, BEBFA0 at +c, BECE70 at +20, BEC1A0 at +24, and BED3B0 at +28.
The +8 entry is the sole live xref to BEBF70. At BECF11 the creator compares
`this+30` (HWND) with zero; BECF1D skips the call when equal. Otherwise BECF1F
loads the vtable, BECF22 loads slot +8, BECF25 restores this to ECX, and BECF27
calls EAX. Requested size and application fields were already written at
BECF14/17/1A; class/window/device/cache/power creation follows the call.
Nothing here proves complete old-HWND teardown: BEBF70 restores a captured
scheme, clears frames, and posts quit without destroying the window.

The normal WinMain sequence is explicit:

| Site | Native call |
| --- | --- |
| 008f8429 | 0073d410 application initialization |
| 008f8432 | 00bea800 platform loop dispatch |
| 008f843b | 00737f30 application shutdown |
| 008f8444 | 007379a0 application destructor |
| 008f8455 | 00bd0400 lifetime-manager destruction, when non-null |
| 008f845b | free captured lifetime-manager pointer; publication cleared at 008f8463 |

BEA800 jumps through platform slot +24. BEC1A0's exit block at BEC200..BEC214
checks +181, sets finished +43, and returns; it contains no +8 dispatch.
Application frame sites 737B56 and 737B6D only set +181 when global exit is
nonzero. The fully paged 737F30 body contains subsystem shutdown/destruction,
but no direct platform-global load or explicit platform +8 call. Its transitive
subsystem callbacks are not all re-proved by this packet.

## Destructor paths and analysis gaps

| Native range | Coverage / observation |
| --- | --- |
| 00bece30..00bece6e | Complete raw deleting-destructor body inspected; reinstalls D68CC4, enables screensaver at BECE41, destroys text queue, calls BE2B10, conditionally frees, returns this with RET4. No policy restore. Live function definition absent. |
| 00beca00..00beca29 | Complete raw nondeleting-destructor candidate inspected; reinstalls D68CC4, same screensaver/text cleanup, tail-jumps BE2B10. No policy restore. Live definition and incoming xrefs absent. |
| 00be2b10..00be2b76 | Complete saved body inspected; replaces vtable, returns title storage, calls BE2A00. No platform +8 dispatch. |
| 00be2a00..00be2a98 | Complete saved body inspected; unregisters platform under manager lock, clears 0109CF04, restores singleton-base profile. No policy restore. |
| 007379a0..007379fc | Complete raw normal continuation inspected. Saved body stops at free call 7379D5, ending 7379D9. Missing continuation adds ESP,4, calls BEA990 at 7379E7, restores SEH frame and returns at 7379FC. |
| 00bea990..00bea99a / 00bea8b0..00bea948 | Complete saved/live bodies inspected; application profile then tail-jump to unregister **E1AE90**, not platform CF04. |
| 00bd0430..00bd0490 | Partial lifetime-manager drain inspected: pops pointer before virtual slot zero with flags1 at BD0485, then reloads count. It does not dispatch slot +8 in this block. |

The new BECA00 candidate was found by a targeted D68CC4 literal scan. Three
text hits exist: BECA0D, BECDD4 (constructor), and BECE3D (deleting destructor).
Ghidra only had xrefs for the latter two routines' vtable stores. Neither raw
destructor calls SetActivePwrScheme. Its thunk C2DDD0 has just two live call
xrefs: BEBF82 restoration and BED264 modified-policy activation.

## Bounded global-reference audit

The current Ghidra reference set for `0109cf04` contains **66 sites**. All were
queried with ten instructions of local context and their first consumers were
inspected. Expanded windows resolved delayed calls. Observed direct platform
virtual dispatches from this set are +4 at 73DC25, +c at 8D60AF, +24 at BEA80B,
and +28 at BEC3CF. Other consumers read dimensions/aspect/focus/text/close
fields, call the window/focus/load-pump/text helpers, or register/unregister the
object. No inspected local consumer dispatches +8.

A targeted PE `.text` search for the exact four-byte global address found
**73 literal uses**. Every indexed use matched a literal. The seven additional
uses are real instructions, not byte-pattern collisions:

| Missing indexed use | Raw role |
| --- | --- |
| 004b4bc0, 00b210d0, 00b23680, 00b25f30 | Six-byte `MOV EAX,[0109cf04]; RET` getters; no live incoming xrefs |
| 0059cb31, 0059cb61 | Aspect-dependent float helpers reading byte +d |
| 00aa709a | Reads byte +d for layout; no stop dispatch in the inspected prefix |

The report preserves all 66 local instruction windows and the seven raw spans.
This set is exhaustive for the queried Ghidra references and exact literal
occurrences in the installed image's `.text`, **not** exhaustive pointer-alias
analysis or every transitive path through the lifetime manager.

## Remaining dependency and verification

Primary analysis follow-up: reports/platform_flow_repair_cc10.json now records
defined BECA00..BECA29 and BECE30..BECE6E bodies and repaired/extended
7379A0..7379FC metadata. The worker's absent-body observations below remain
capture-time evidence. Current call rows use actual containment; BECA25 is
explicitly a tail jump. The updated checker passes all 14 direct transfer rows.
This repairs analysis metadata without establishing a new power-restore path.

The next bounded step is to identify any raw relative CALL/JMP or data-pointer
users of the four unindexed getters, then follow only actual platform aliases
to slot +8. The lifetime manager's slot-zero dispatch establishes how a retained
platform reaches BECE30, but the complete registration/destruction ordering and
arbitrary subsystem callbacks are not a proof that another object never invokes
platform +8 first. No ordinary-exit restoration should be invented from a
method name or a desirable cleanup policy.

Exact native bytes for the call, both destructor candidates, application
continuation, vtable, and missing global uses are matched against live Ghidra
and the installed PE in the report. Call rows distinguish direct, resolved
virtual, imported, and absent-function cases. The report verifier checks the
available live bodies; raw call/JMP targets are independently decoded from
matched bytes. No source changed, no build or game run was needed, and no power
or screensaver API was executed.
