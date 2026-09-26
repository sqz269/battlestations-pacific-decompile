# Stock XLive ordinal 5030 guards

The pinned stock export supports conditional normal FALSE returns before SDK message processing. It does not establish a general pre-initialization API contract or a successful handled-message call. The preceding genuine declaration/cache case recorded two pumps, two focus calls and **zero ordinal 5030 calls**: no successful `PeekMessageA` result entered its retrieved-message body. Sent or internal message processing during Peek remains possible without a returned MSG.

This is read-only disk-code evidence for `C:/Windows/SysWOW64/xlive.dll`: I386, 14,303,392 bytes, SHA-256 `79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385`, preferred image base `00400000`. Ordinal 5030 is a non-forwarded export at RVA `B4E6C`. Its complete 822-byte body ends exclusively at `B51A2`, contains 200 instructions and has one `RET 4` at `B519F`. All static branch targets stay on instruction boundaries in that body; no indirect or external jump remains. The body SHA-256 is `f78365978f5fc7a1d068feaf365148c5681d97275d34d34aa52c9005ff74662d`.

The first guards precede any MSG field or imported call:

| RVA | Native schedule |
| --- | --- |
| B5041 | Capture the MSG pointer argument without reading a field. |
| B5099 | A null pointer returns FALSE through B5199. |
| B50A1 | Current DWORD at RVA 633570 equal to zero returns FALSE through B5199. |
| B50AD / B50B0 | Read MSG.message at +4 and compare it with current registered-message DWORD at RVA 632E7C. |
| B5100 | For a different message, wParam at +8 must equal 0xE5; otherwise return FALSE. |
| B510D / B5114 | That 0xE5 route also requires message 0x100 or 0x104; otherwise return FALSE. |

Consequently a genuine nonnull Peek-produced MSG can return FALSE with guard zero before a field read or import. With a nonzero guard, a message different from the current registered ID and wParam different from 0xE5 also returns FALSE without reading HWND or calling an import. These are conditional binary facts; the old zero-call case observed neither these branches nor the live guard.

When the message matches the current registered ID, wParam 0/1 calls `ImmAssociateContextEx(HWND, 0, 0/0x10)`. wParam 2 calls `ImmGetContext`, then for a nonnull context `ImmNotifyIME(context, 0x15, 1, 0)` and `ImmReleaseContext`. Other wParam values take TRUE without an imported call. The wParam-2 route rereads **current HWND at B50F2 after ImmNotifyIME**, before the release at B50F4. TRUE is formed at B50F9; reached Windows imports must obey their genuine ABI and return normally.

For a different message 0x100/0x104 with wParam 0xE5, `ImmGetVirtualKey(HWND)` is called. Key 0x17 takes TRUE; key 0x19 takes TRUE when current language WORD at RVA 632DA0, masked by 0x3FF, equals 0x12. Otherwise, **B5143 captures the current USER32 GetKeyState IAT target once into EBX** from IAT RVA 1494. B514B, B5157 and B5163 are calls through that captured EBX target, for control 0x11, shift 0x10 and alt 0x12. They are not three fresh IAT reads. Their high bits are compared with 23 recovered constant modifier/key rows; a match takes TRUE and no match takes FALSE.

The body has eight actual call sites resolving to six imports: the five IMM32 APIs above and USER32 `GetKeyState`. It reads only MSG HWND+0, message+4 and wParam+8; it does not read lParam, time or point. The subsequent real Translate/Dispatch route still requires the complete live MSG and valid window procedure/payload lifetimes.

The three state slots are zero in the pinned **disk image**; live values are unobserved. A bounded producer window calls `RegisterWindowMessageA("PanEnableIme")` and stores its return at B65E8 into RVA 632E7C. An explicit guard=1 store occurs at B6A70; other register-valued stores occur at B86BE, B87B4 and B8928. A language-word store occurs at B3C84..B3C8A. Six writer windows and 23 relocated references to these three slots are retained, including unresolved address-taking references. Caller reachability, DllMain/SDK paths, register values and indirect mutation closure are incomplete. The guard is not established as an “SDK initialized” flag, and loading is not proved to leave it zero. This export itself calls no SDK initializer.

The frozen evidence is `local/output/cc10_stock_5030_first_message_readiness/readiness_evidence.zip`, SHA-256 `7b9242c1dee88b397853460dc83bd45c31dfa15d83a0775fe711bd6fc8c4cace`: 57 hashed rows / 58 ZIP members. It retains exact disk bytes, complete CFG/listing, import identities, key table, state references/writer windows and the earlier MSG/pump evidence. Primary verification covered the entire body and archive. The frozen readiness remains unchanged; this publication clarifies its shorthand about GetKeyState calls by recording the single captured IAT target.

A defensible future call still needs a genuine naturally retrieved MSG, its actual fields and lifetime, qualified live state, normally returning genuine imports, and the downstream procedure/payload and quiescence obligations. Synchronous window creation does not guarantee a later returned queue MSG. State snapshots alone do not prove a branch if writers can intervene. No natural-window fixture is designed here, and no forced, posted, default or fabricated message is admitted.

This packet changes only this document and its report. It adds zero BSP-native functions, ledger records or byte credit. No C++, build, test, DLL load, probe/controller/game execution, SDK initialization, Ghidra import or mutation was performed for publication. Static CFG reachability is not runtime reachability, official pre-init support, live IAT/state evidence, physical loaded-image proof or game validation.
