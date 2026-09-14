# Native online client leaves

This packet supplies complete normal source bodies for A43180 and A430E0 plus the required raw manager getters A3E510/A3EAC0. It borrows actual client, game and manager storage without constructing a client2218h owner or game projection. The downstream771630 body remains a required unresolved dependency. Descriptive source names are hypotheses; this is not a complete source implementation of voice/session policy or a binary replacement interface.

| Routine | Inclusive range | Original ABI | Coverage |
|---|---|---|---|
| Refresh client records | A43180..A43213,148 bytes | ECX captured client, RET | Complete normal caller body; required771630 boundary |
| Query client identifier | A430E0..A4312B,76 bytes | ECX client, identifier low/high DWORDs, RET8; AL result | Complete |
| Selected byte | A3E510..A3E516,7 bytes | ECX raw manager, RET; AL | Complete |
| Active user | A3EAC0..A3EAC6,7 bytes | ECX raw manager, RET; EAX | Complete |

Read-only BSP queries verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Saved bytes match the installed PE for all four spans. At the worker snapshot, A43180 and A430E0 had **no Ghidra function**. The full linear disk listings end at the RETs above and are followed by CC padding. The six parent direct call rows were recorded as real worker validation failures, pending locked root definitions. No Ghidra mutation, annotation or save is performed by this packet.

## Object and record producers

992A50 allocates2218h bytes, passes that exact allocation to A47EE0 and publishes its returned identity in F8A300 and F8A2FC at992A9A/992A9F. The coordinating root's prior publication audit identifies992A50 and992AC0 as F8A2FC's writers. This packet rechecks the allocation/publication and destruction snippets. A47EE0 calls base994080 and writes primary vtable D24878 atA47F1D; the retained constructor range reachesA4832C. The table's DWORDs D2489C and D248A0 are A430E0 and A43180, establishing slots+24 and+28 for that observed concrete client. Other client implementations are not inferred from these two entries.

The game has eight118h player records at+748. Existing `SessionParticipantPools` documents a partial projection of their flag fields, which this source does not reinterpret as native storage. The actual game allocation is zeroed at73E163, and4DDD6C constructs the eight records. Producer4BB440 claims the first record with +08 clear, writes +08=1 and copies its fifth/sixth payload DWORD arguments into +80/+84 in order. Reset4BB160 writes player +08/+09 zero. Thus the consumed flag offsets and the low/high identifier order are tied to producers; no additional record layout is invented.

The getters operate on the established `NativeOnlineManagerStorage`3F0h allocation. A3E510 returns raw byte+119 without normalization. A3EAC0 returns DWORD+11C. Their volatile machine-storage reads preserve discarded reads and current publication reloads. They do not create a second manager or reinterpret an existing sign-in/session projection.

## A430E0 query and its local preimage

The entry instruction is **PUSH ECX**. Its local output DWORD therefore initially contains the incoming client pointer. It is not an unknown stack value or an invented zero. The routine reloads current F8ABE8 and reads +11C once, discarding that result; it reloads and reads a second time for an unsigned comparison. Values greater than1 return AL=1 without an SDK call, including FFFFFFFF. On the accepted path it reloads F8ABE8 a third time and passes that current user's value to the SDK, along with the captured low/high words and a pointer to the local output DWORD.

A4D5E4 is a six-byte jump through CE2714. The PE import directory identifies `xlive.dll` ordinal5314; four pushed DWORDs and unchanged caller ESP after the thunk establish stdcall RET16. Other observed callers A4314D and A48FB1 use the same user/low/high/output order. Ghidra displays the descriptive name `XUserMuteListQuery`; the concrete source binding relies on the verified ordinal, not an assumed exported name.

The SDK return status is ignored. The result is whether the entire resulting DWORD is nonzero, returned in AL. Full, partial and absent output writes therefore differ: writing only a zero low byte leaves the high client-pointer bytes intact. A null incoming client is valid for this leaf's preimage calculation because this body never dereferences it. The output pointer is borrowed local storage valid during the synchronous call; it is not retained by this source.

`resolve_native_online_client_sdk` borrows an already loaded module and resolves ordinal5314 with GetProcAddress. It does not load a DLL or execute an SDK request. The caller retains the module through all calls. A missing module/import throws at the explicit source binding boundary, outside the reconstructed leaf body.

## A43180 traversal and current publications

The entry also pushes the incoming client pointer into a retained DWORD. It first reads current E188A8 and gates on game+1FE4; only a nonzero gate reads current F8ABE8/+119. Any nonzero selected byte passes. Failure returns without reading client vtable or player records.

The loop walks exactly eight offsets0,118,...,7A8. Each iteration reloads current E188A8, forms game+748+offset with Win32 unsigned wrapping, retains the native null-result check and requires record+09 zero and record+08 nonzero. It reads identifier high at+84, then low at+80. It reloads the captured client's current vtable and calls slot+24 with that same client in ECX, followed by the low/high DWORDs on the stack. No current-client global replacement is substituted for the captured receiver.

After the virtual call, only the retained DWORD's low byte is overwritten by AL. Its upper24 bits still contain the original client-pointer preimage. AL is not normalized, so a replacement virtual returning80h passes80h. The full word is the third argument to771630, after low/high. Before that direct call, current E188A8 is reloaded again and ECX becomes that current game+1EF0. Mutating game publication or the captured client's vtable inside either callback affects subsequent reads exactly where the native listing reloads them. Captured identifier words survive those mutations. There is no cached game, vtable or slot target and no field write to the native client by this body.

## Explicit771630 dependency

The entire read-only771630..771796 body is359 bytes/121 instructions with RET12 and EH handling. It captures the current online manager, checks privileges FC/FB, may query friendship, and can force the low policy byte. Depending on session+F4 and the current user's identifier, it either constructs/sends a session message or calls local voice-pair mutation routines. Diagnostic calls and message ownership/unwind paths also occur. It is not a simple store of the incoming boolean.

`NativeOnlineClientApply771630` exposes its exact required raw call ABI: thiscall(current game+1EF0, identifier low, identifier high, full result DWORD), RET12. The caller must supply the genuine native body or its substantive source reconstruction. There is no default, no-op, synthetic success, projected session cast or call into original addresses installed by this packet. The body and its dependent policy/message/voice operations remain unresolved source work. They are not claimed as fixture-tested behavior: the fixture compares the original and source **arguments delivered to this boundary**.

## Verification scope

One focused fixture retains copied original code for all four routines, redirects the six direct calls and redirects only the two publication addresses. SDK and771630 boundaries receive deterministic fake providers; the actual raw getters and current borrowed client-vtable dispatch execute. Cases cover ignored SDK errors with absent/full/partial writes, client-pointer preimages including null, unsigned users2/FFFFFFFF, selected-byte and game gates, three successive eligible records across current-game/vtable replacement, captured identifiers and unnormalized AL. No2218h client or full game constructor is fabricated; buffers represent only readable test storage for these leaves.

The fixture builds with `/MD /W4 /WX /fp:strict /link /MANIFEST:EMBED`. Strict Win32 build, existing CTests, fixture results and the immutable source/dependency/toolchain/runtime/PE manifest are recorded in `reports/native_online_client_leaves.json`. No real SDK query, friendship/privilege request, session message, local voice mutation or game runtime was executed. These checks establish the bounded call/storage behavior, not completed online startup or gameplay parity.

Integrated validation at `a602d7cd3f6a728beda3645e8b807ba7f7d6cc71`: strict Win32 build, both CTests and all ten copied-original/source cases pass against the exact integrated library. Root defined both missing functions after checking live bytes against the installed PE, saved four reviewed annotations while retaining prior comments, and refreshed all four exports. All 22 direct call rows now pass. `local/checkpoints/a602d7cd/native-online-client-default/validation.json` freezes 3206 artifacts and references the separately verified 3,240-artifact worker archive. This closes the worker listing failures; the required771630 implementation, full client/game construction, application adoption and gameplay remain unresolved.
