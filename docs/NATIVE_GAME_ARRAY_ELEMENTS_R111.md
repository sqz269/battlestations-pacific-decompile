# Raw game array elements and participant construction

Addresses: 004d6ba0, 004d3730, 004c8140, 004c3180, 004c1970, 004d2920, 004d0d50, 004c3700, 004ce990, 004ddb90

## Result

The four arrays in the raw game constructor now use real source element constructors. They comprise five10h vector headers at game+560, two arrays of eight118h participant records at748 and1008, and four0Ch list headers at7134. The allocator provider also has the existing concrete malloc/new-handler default. This closes the normal array-construction dependency; remaining game services, application lifetime and destruction are still required.

`004D6BA0` is a complete307-byte normal participant constructor. Earlier evidence covered only its first portion. This packet adds one unique game-function reconstruction; the eight supporting library-storage contracts and parent composition remain fragments. Defining a missing vector helper or describing normal CRT iteration does not add game-function counts.

## Actual participant storage

The constructor writes vptrCE7794 and the sparse native fields in order, including word10=FFFD, byte1A=0, DWORD2C=3,14=2,28=2, the callback-owner prefix at38, bytes88/89/90, raw CE4ADC bits at8C and DWORD94=3. It preserves unlisted bytes, including the allocator word at98 and trailing114..117.

At+98 it initializes countA0=0, allocates a10h sentinel through4C3180/BF681B and publishes it at9C. The zero-copy call4D2920 still reads the current head's next pointer. It then initializes A4=0,A8=FFFFFFC4,C8=0 and three8h string headers atEC/F4/FC through the actual415270 initializer. Final writes set110=FFFFFFFF, clear E0/E4/E8/104/108/10C and byte78. The opaque gaps are not zero-filled.

`004D3730` returns its receiver and clears only4/8/C, preserving the vector allocator word0. `004C8140` allocates a0Ch self-linked sentinel, writes header4 and clears8, preserving header0 and sentinel payload. Sentinel allocation retains the original independent wrapped-address guards; it does not add a synthetic null-success path.

## List and array contracts

`004D2920` captures current head->next before entering4D0D50, even for zero copies. The normal insertion loop captures that position, reads its previous link for each allocation, copies two pair DWORDs after allocation, grows the list count, then links the node. The guard is the unsigned comparison `1FFFFFFF - current_count < increment`. It preserves the native allocate-before-count-check order. The previous throw-only heuristic name at4CE990 was corrected because its normal path increments the count.

The source retains an unlinked allocation in diagnostic stage metadata if count growth throws. Its owning length-error transport reuses existing source code; original RTTI/FH3 behavior is not claimed. The original67-byte4D0D50 rollback block is preserved as evidence, not implemented as native source cleanup.

Normal CRT array iteration is an explicit library contract over the three callback/stride pairs used by4DDB90. Unknown pairs are rejected. Nested string iteration supports the real415270/41DD20 pair and invokes the existing raw empty-header initializer. Source operations retain the completed prefix, current element, inner call/allocation site and unwind state when construction fails; they reject replay. They do not silently run unported destructors or claim native CRT reverse rollback.

## Ghidra and validation

The previously missing14-byte4D3730 callback is now defined. An initial interpretation of4D0D50 as a truncated187-byte body was incorrect: its120-byte normal body excludes the separately defined67-byte `Catch_All@004D0DB5`. Recreation preserved that correct membership; no restoration was needed. The first call-site audit caught three exception calls incorrectly attributed to the parent, and those rows now name the actual handler. The copied187-byte envelope includes both functions, counted once. Definitions use the write lock and `disassemble_first=false`; no callee no-return flag changed. Prior names/comments were recorded and retained, except the documented correction of the heuristic4CE990 name. Evidence comments, readback, saved analysis and refreshed exports accompany the report.

`reports/native_game_array_elements_r111.json` retains16,253 disk/live-matching bytes:72 copied code spans (14,237), read-only score-layout producer1,930, SDK thunk6 and constants/literals80. One copied span contains the separately owned exception handler described above. All409 direct call rows pass the live Ghidra audit;373 copied-span calls and four external JMPs are relocated in the fixture.

Strict MSVC Win32 and all three existing CTests pass. Twenty-six original/source cases compare1,668 ordered observations and113,825,212 bytes. The existing game cases now execute real vector/participant/list callbacks, including all16 participant records. Added list cases cover nonempty insertion, zero-count insertion and payload mutation during allocation. The fixture uses controlled normal CRT iteration with copied original element callbacks; it does not execute the original CRT exception machinery.

Source failure checks stop the third participant allocation and the third final-list allocation. They retain two completed elements and the exact parent/element/storage sites:4DDD6C/state11 and4D6C42/4C3182/state0, or4DDEFC/state31 and4C8143/4C1972/state-1. Replay is rejected and diagnostic retirement is explicit. A separate real-CRT check constructs the participant, three pair nodes, four list heads and five vector headers, then manually releases eight actual allocations. That diagnostic release is not evidence of reconstructed game destructors.

The report records application artifact comparison and immutable tested/integrated archives. These raw game paths are not yet wired into application startup; no new executable runtime or gameplay result is claimed.

## Follow-up work

Bind the remaining collection, race/configuration/resource/Lua-host/Dyn providers and actual game owner into startup. Recover participant, vector and list destruction and the original array failure/teardown contracts before claiming complete lifetime behavior. Native ABI/EH, arbitrary aliases/fault timing, SDK runtime, visuals and gameplay remain open.
