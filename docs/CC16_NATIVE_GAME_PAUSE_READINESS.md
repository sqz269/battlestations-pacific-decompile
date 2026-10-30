# Whole raw Game pause readiness

**The whole raw004D95F0 provider is not ready.** Its569-byte/140-instruction parent is fully recovered, but whole raw004D6480 parking and0054E440 hint advancement, their actual HUD/screen owners and current callable tables remain unclosed. Existing GamePauseState, std::vector and GameMenu reset-stack paths cannot supply them. Do not publish a zero-only wrapper or default-empty screen set as the whole pause binding required by raw frontend destruction.

This audit is frozen at published `f90f5d178da2ef3b8265145cd08ea8e9bc935467`. Later source publication is separate. Only this document/report and read-only4D95F0 lease are owned. No source, build, test, probe, game, DLL, Ghidra, shared metadata or frozen-peer change occurs.

## Whole native parent and callback epochs

Live configured-BSP bytes, the original PE and all140 saved instruction addresses agree. Body004D95F0..004D9828 SHA256 is `431f945fcd6f9814a34f67a9924289497c28d35f569ccad01964d4d6f758f6c3`. Original ABI: ECX=actual Game, one DWORD argument whose low byte is initially consumed, RET4; ESI is saved, with EBX saved only on the changed-state path. No semantic EAX result is recovered. The incoming Game is retained in ESI; it is not replaced by globalE188A8.

1. Read the incoming argument's low byte and compare it **byte-for-byte** with actual Game+61F. Equality returns before manager/input/vector work. Otherwise store that byte verbatim; values2/80/FF are not canonicalized to bool.
2. If the incoming byte is nonzero and current Game+620 is nonzero, load CURRENT E198C4, then its+A8 receiver, and call whole0054E440. There is no manager-null check in this arm. The child can mutate Game/publications.
3. Afterwards read current Game+1FE4 and current Game+61F. For nonzero session kind and now-zero pause, reload CURRENT E198C4; test nonnull, test its+E8 nonnull, reload+E8 and test that object's byte+4. These are post-hint reads, not entry snapshots.
4. If enabled, call actual004BEC00, unconditionally write DWORD1 at **that first returned owner+20**, call004BEC00 again, and invoke actualA93020 on the second result. No owner-null skip or single-getter substitution is native. Input refresh may produce further mutations.
5. Only then zero the native temporary's begin/end/capacity, acquire its cleanup and append the sixteen values below through sixteen separate442190 calls. The repeated allocator/diagnostic/copy epochs are material; a std::vector range constructor or one reserved allocation is not equivalent.
6. At004D97F4 reload the **current original argument DWORD home**, push it unchanged, and call004D6480 with the actual temporary. This is not a reload of Game+61F or a normalized bool. ECX is set back to the retained Game, although the004D6480 body does not consume it.
7. Capture CURRENT temporary begin after parking. If nonnull, call BF65AC. This normal free leaves header fields alone. State0 remains active through that conditional free and epilogue; no earlier state demotion is present.

Raw/frontend `call_004d95f0(Game,0)` must ultimately bind this whole behavior. An eventual C++ interface should preserve the live32-bit argument home or explicitly qualify its distinct argument/private-frame transport; callbacks and the final reload cannot silently become one immutable bool snapshot.

## Actual stack temporary and cleanup

With original entry ESP=E, the changed-state path has ESP=E-28h after saved ESI/EBX and locals. The repeated item DWORD is at **E-20h**. The vector16 begins at **E-1Ch**: opaque+0 is unwritten; begin+4 is E-18h, end+8 is E-14h, capacity+C is E-10h. Only the latter three words are zeroed. Every push receives the address of the same item DWORD; there is no native C++ library-container identity to infer from that layout.

Kept IDs, in exact order: `0D,0E,0F,28,3C,3D,53,55,57,58,59,5C,5D,19,5E,0C`.

At004D9699 state0 is armed before the first append, after the three pointer zeros. HandlerC665A8 selects FuncInfoD8F430 (magic19930522/maxState1/EHflags1), mapD8F428 `{-1,C665A0}`. ActionC665A0 forms `[EBP-1Ch]` and tail-calls0041CB90. That42-byte/13-op tidy captures CURRENT begin+4 once, conditionally BF65AC-frees it, then writes zero to+4,+8,+C in order; +0 is untouched. The acquired scope covers all appends, parking and the normal conditional free. A covered failure during that normal free reaches tidy again; no invented disarm, current-pointer repair or rollback is justified. Original FH3/SEH/hardware-fault/secondary-cleanup transport is not established by a new C++ guard.

## Genuine services available versus missing

| Boundary | Existing genuine source and remaining custody |
| --- | --- |
| Input getter004BEC00 / refreshA93020 | Complete raw functions in `native_input_action_owner` / `native_input_action_contexts`. Preserve the two getter calls and first-result store. GameHost InputServices actually constructs GameInputRuntime over its application F8BBF8 cell and shared singleton lifetime; `action_context()` exposes that same owner. |
| Input configuration | GameInputActions constructs a real NativeInputActionConfigurationContext from the same runtime records/devices plus actual CRT/constants/timing and exposes `configuration_context()`. No production construction/use outside that class was located in the frozen sources. Borrow/compose the actual existing pipeline; do not invent a second owner, empty record service or dummy post-tick callback merely to instantiate a facade. |
| DWORD vector append | Existing complete raw Scene family has105B/44-op PushBack46C450 and137B/56-op One46B4A0. They match442190/441F30 exactly after the stated child-target relocation mapping. N441860 versus46AFB0 has the same instruction structure but11 call sites differ, spanning seven helper targets. Its transitive allocator/copy/fill/error mapping still requires an explicit reuse audit. Current raw family exposes live parameter homes/private outputs; parent-owned backing must follow those interfaces. No std::vector or vendor-body credit substitution is admitted. |
| Temporary heap/free | Match the accepted raw vector provider's actual allocation/current-CRT policy. Scene allocation uses its real allocation helper; old backing uses singleton_lifetime_free. Original BF65AC release does not license an arbitrary memory.release callback, scalar delete or foreign heap. Preserve normal conditional free versus unwind current-begin tidy/clears. |
| Actual managerE198C4 | Raw frontend destruction already names a borrowed actual cell; that interface does not produce a manager. Its semantic FrontEndManagerSet/GameHost HUD storage is not a raw manager or actual+A8/+E8 owner. A genuine supplier and receiver lifetime are still required. |
| ScreensE18B60 | Native95-slot table; original register4F71D0 and unregister4F71A0 govern it. Current source registration is a semantic FrontEndScreen registry. No qualified raw95-cell/callable-screen owner binding was located. A copied original vtable token is not a callable source method. |
| Whole parking004D6480 | Current `gui_park_screens_004d6480(GuiParkHost&,std::vector<int>&,bool)` is a projection. Raw body requires actual screen vslot24 collection into a native vector, current widget+121 accesses, exact resolved-position calls, ambient x87 arithmetic, live1000.0 constantCE47A0 and temporary cleanup. It remains a genuine prerequisite. |
| Position leafs | Raw AA7DC0 local setter, AA7220 transform and AA70E0 bounds already exist with real scratch/dispatch/constant contracts. AA6750 resolved getter and AA8240 resolved setter currently use logical transform/owner or Hooks interfaces, not whole raw providers. Their FLD/FSTP epochs, recursive parent reads, alias effects and ambient precision/rounding/exceptions must be retained. |
| Whole hint0054E440 | Current HudHintsState/std::string/host implementation is semantic. Raw body needs actual hints screen+A8, current pause-group vslot34, actual E188A8 for raw4D94F0, repeated input getter/reset calls, two native string headers and cleanup, real award-tracker4E1CA0/68E5B0/68E420/690FD0 services and actual pools/CRT. Raw string, sized-pool and input primitives exist; whole raw hint/hint-pause/tracker and owner bindings do not close from the semantic API. |

The existing GameMenu reset_interface_stack explicitly logs unimplemented at4D95F0. MissionFrame's hint path records a call and reports tutorial availability false. Neither is a required whole provider. Current semantic source remains unchanged.

## Smallest independent next source packet

Recover **only raw00AA6750** first:146 bytes/47 instructions, self-recursion as its only code dependency. Suggested owned files: `include/bsp/native_gui_widget_resolved_position.hpp`, `src/native_gui_widget_resolved_position.cpp`, `docs/CC16_NATIVE_GUI_WIDGET_RESOLVED_POSITION.md`, `reports/cc16_native_gui_widget_resolved_position.json`.

Use actual widget bytes, actual writable xyz output and an explicit borrowed **doubleD7A258** as the three required inputs; the verified original bytes are eight zeros. Existing GameNativeReadOnlyData maps authenticated original data bands and already supplies D7A258 to raw text-field code. Retain that genuine mapping/reference; do not add a default zero global or execute data-table entries as code. No screen table or logical widget cast is needed for this leaf.

Preserve all actual parent+70 reloads, size/pivot operand directions, every single-precision spill before/after recursion, output-store ordering and the live double subtraction. Even its no-parent arm uses sequential FLD/FSTP, not a three-DWORD snapshot. Use exact ambient x87 operations with no forced PC24/PC53 or ordinary double approximation. A safe new C++ interface may wrap a faithful internal helper; original private-frame, recursive stack, unmasked-FP/fault and binary ABI stay qualified until evidenced.

RawAA8240, whole raw parking, raw4D94F0/hint/tracker and canonical manager/screen root producers remain named prerequisites. This packet proposes **only rawAA6750 as the next source packet**. A future pause module must compose genuine completed providers; a required pure callback or unused wrapper alone is not completion or activation.

Evidence: six live/PE spans, fourteen export pins,38 frozen source pins and23 original direct call rows (22 parent plus tidy free), all passing. No source/build/test/runtime activity occurred; native/vendor-body, ABI, activation and gameplay credit are zero. Wait for a separately scoped source followup rather than starting a leaf in this read-only tree.

[Machine-readable readiness](../reports/cc16_native_game_pause_readiness.json).
