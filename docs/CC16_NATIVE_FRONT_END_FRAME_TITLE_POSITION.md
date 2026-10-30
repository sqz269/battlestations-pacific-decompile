# CC16 actual Frame title, positions and collector

This packet adds four whole raw Source variants in `native_front_end_frame_title_position.hpp/.cpp`. It changes no existing semantic implementation, ledger, CMake, shared authority or application binding. The variants have not been built or run in this packet. Whole frontend readiness remains **false**; there is no new Original/vendor/duplicate-body/ABI/runtime/activation credit.

The read-only target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32 at base400000. The configured installed PE is 12,223,752 bytes, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. All 388 saved assembly operations and their physical 1,337 bytes were reviewed. Exact Git-LF Source/input pins, raw saved-export pins, original PE ranges, six literal preimages and three EH maps/actions are in the companion report.

| Whole entry/end | Operations / physical bytes | New Source symbol | Native inputs/return |
|---|---:|---|---|
| 5189B0..518B8C | 126 / 477 | `set_native_front_end_frame_title_005189b0` | ECX actual string8h; RET |
| 5186B0..518818 | 109 / 361 | `set_native_front_end_frame_title_position_005186b0` | ECX box XYZ, EDX title XYZ; RET |
| 518820..5189A1 | 119 / 386 | `get_native_front_end_frame_title_position_00518820` | ECX box output, EDX title output; RET |
| 518E00..518E70 | 34 / 113 | `collect_native_front_end_frame_children_00518e00` | ECX captured Frame, stack output-vector pointer; RET4 |

These are new C++ interfaces, not the original register/stack ABI. Existing typed `menu_screens_2` and `frontend_seg12_17_cc13` bodies receive no additional credit. The existing actual-pointer `gui_widget_get_local_position_00aa67f0` (`cc13_gui1.cpp:227`, LEA widget+0C/RET) is reused at both reached getter points; no new AA67F0 implementation or wrapper policy is added.

`NativeFrontEndFrameTitlePositionContext` requires eight borrowed references: the SAME process selectors E18D84/88/8C; the existing `GameNativeSettingsProcess::game_00e188a8()` Source publication cell; the SAME raw string-pool/manager context; a nondefault authenticated RO data view; the actual canonical BF7680 feature DWORD0109EEA4; and genuine operation bindings. There is no new selection/Game owner or copied four-field bank. The preentry view borrows two original name pointers (`title_Text` at CEC3D8, `szarnyas_FrameBox` at CEC3C4) and four original constant pointers (CEC380 double960, CEC3E8 double pad, CEC3E4 float minimum, CE3E18 float fixed width). It validates their original addresses and PE fingerprints. It also requires the SAME retained authenticated original-VA RO mapping for every CURRENT widget profile table DATA read.

Every getter pair keeps two distinct real return values: the first Frame supplies the Page array, the second supplies own160. They may differ from each other and from the current publication. The selector/own read order is preserved:

| Reached group | Read order after the second getter |
|---|---|
| Title initial Page guard and title_Text lookup | own160, then CURRENT title selector8C |
| Title FrameBox lookup and both position-name lookups | CURRENT title selector8C, then own160 |
| Collector's three groups | captured Frame own160, then each CURRENT selector84/88/8C |

Only the title initial Page slot is tested for null. No later child/receiver null guard, selection range restriction or successful fallback is introduced. Native DWORD wrap is retained in Page addressing.

Title constructs both actual name8h headers through existing raw41E870. Each position body explicitly zeros a header, calls raw41DD40(length,1), captures CURRENT data, then CURRENT length+1, and calls actual BF7680 when data is nonnull. It borrows the genuine feature cell, never a guessed constant. Child lookup receives the actual temporary header and literal one. Each normal return captures CURRENT header data and length after the lookup through raw41DD20, not the earlier copy destination; the header remains untouched afterward.

Title then calls genuine actual ABAED0(captured text, incoming actual header,1), with no empty-string shortcut. It captures the Game publication once, reads byte61F, and reads byte634 only if necessary from the SAME captured Game. Either nonzero selects the fixed width arm.

The ordinary width arm uses inline x87: FLD actual text+114, FDIV original double960, FSTP float; FLD original double pad, FLD the saved float, FCOMI, FSTP ST1, JBE. Equality and unordered select the minimum arm. The greater arm repeats the native float spill before the true AA6740 getter; after that getter, it reloads the saved float, reads the returned current height, FADDs the original double pad again, and FSTPs the output float. Minimum/fixed arms read height first, then MOVSS the respective actual constant bits. All own x87 pushes are popped before any required getter/dispatch call. No `std::max`, host double projection, CW/status reset or FP RAII is used.

`NativeFrontEndFrameTitleScratch` preserves the actual contiguous 32-byte home relation: first name header0, second header8, plus/fixed float2 at16, minimum float2 at24. The first header's length word becomes the float quotient home after that header's return, matching native ESP+0C. The true size getter runs between its store and reload; the code does not hide it in a separate copied variable. The final current widget profile and profile+58 DATA word are captured only after the selected float2 is complete, then transported to genuine captured-target Source dispatch. No numeric codeword is called or cast to a callable pointer.

Set-position finds text then FrameBox, returns each name, calls FrameBox AA7DC0 first with the captured box pointer, and calls text AA7DC0 second with the captured title pointer. Neither input XYZ is copied before its reached call. The existing raw AA7DC0 implementation already captures all three source words before its own stores, then recomposes transform and refreshes bounds; the actual retained transform/bounds binding is still required.

Get-position finds text then FrameBox and reuses the real AA67F0 pointer getter for FrameBox. Its three FLD/FSTP component pairs execute in order, each store preceding the next component load. Only after all three stores does it call the text AA67F0 getter and execute the second group. This retains overlap and ambient x87 conversion behavior; it is not a memcpy/vector snapshot.

Collector never reacquires Frame. It tests the first selected slot, forms its actual address, then reads the output-vector pointer home ONCE, including when that first slot is null. It appends that ADDRESS through 4D6790 if nonnull, before reading the next group's own160/selector. Panel and title similarly reload separately after prior callbacks. The vector provider receives a pointer to the real slot and must read it at its actual append schedule, including allocation callbacks; a copied Page value or current-publication substitution is invalid.

Native EH static evidence is finite: title handler C6A980/FuncInfo D93DFC/map D93DEC; set-position C6A940/D93D94/D93D84; get-position C6A960/D93DC8/D93DB8. Each has two states, each transitioning directly to -1 and calling41DD20 on its respective temporary. State is armed only after construction/copy returns and cleared before explicit normal pool return. Source catches surround only the armed getter/lookup interval. They directly perform one cleanup; if it returns, the original provider exception rethrows; if cleanup throws, that failure escapes. No extra termination, retry, rollback, catch around normal cleanup, acknowledgement or inferred Native secondary-exception policy is added. Native FH3/SEH secondary identity remains unproved.

Seven pure mandatory endpoints in `NativeFrontEndFrameTitlePositionBindings` remain actual consumer composition requirements:

| Endpoint | Required actual binding; remaining boundary |
|---|---|
| `frame_004c1ac0()` | Existing genuine Frame getter with SAME manager, publication and preprepared captured-allocation custody; retain old/current epochs separately. |
| `find_child_00aa7e00(parent,header8,1)` | Same actual Page/widget graph and name/node/list identities; typed/actual-owner supported overloads do not establish every captured raw receiver. |
| `text_source_00abaed0(text,header8,1)` | Genuine changed raw source/localization/inner updater/current+50 path; the existing unchanged-storage fragment alone cannot satisfy it. |
| `widget_size_00aa6740(widget)` | The true pointer-returning widget+20 getter at the native call point; no precomputed height or width-only semantic provider. |
| `set_local_position_00aa7dc0(widget,xyz)` | Reuse existing raw body with actual transform dispatch, bounds scratch and canonical half constant; no typed widget replacement. |
| `call_widget_virtual58(widget,profile,target,float2)` | Genuine finite Source transport for captured CURRENT target/receiver; no ignored token, re-resolved different epoch, numeric invocation or fallback. |
| `push_page_slot_004d6790(vector,slot_address)` | Genuine raw output-vector append/insert/allocator lifetime with late slot-address read; no `std::vector<Page*>` projection. |

The future actual Startup-owned `GameNativeFrontEndApplication` consumer must retain these contexts, authenticated mappings, per-operation scratch, captured Page/widget/Frame clients and genuine target providers across Game/Menu deletion and both singleton-drain retirement paths. Existing Source library bodies and required endpoints do not establish that consumer graph. No shared owner/view/application or destruction/scalar Source is changed here.

Static Source/listing/PE/EH inspection is the only verification performed. MSVC Win32 integration/build, genuine binding, Original register ABI/FH3/SEH/private-frame and hidden alias identity, exact faults/unmasked FP/stack-fault behavior, runtime/game visuals and activation remain unproved. Prior passed build/runtime receipts are immutable and do not validate these new files.
