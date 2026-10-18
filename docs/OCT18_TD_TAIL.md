# oct18-tD long-tail evidence, 2026-10-18

Scope: the 240 exact rows in the coordinator's `local/codex_tail_D.txt`.
Classification reads the original PE and exported assembly from project `bsp`,
program `/battlestationspacific.exe`; the live project/program was checked with
`bsp.py ghidra count`. No vendor bodies or game-host wiring are added here.

Compiler exclusions below use `library_instantiation_out_of_scope` as requested
by the packet. They are compiler support for game parents, not reconstructed
game-function credit. Existing descriptive names remain hypotheses. FH3 rows
are exactly `B8 <FuncInfo> E9 <rel32>` targeting `00BF6B43` (the existing CRT
`__CxxFrameHandler3` binding), with FuncInfo magic `19930522`. Cleanup rows
are actions referenced by real FuncInfo unwind-map entries; the entry address,
state transition, selector and decoded instruction sequence are recorded.
Their original ABI is MSVC x86 compiler EH: the dispatcher receives exception
runtime context with EAX selecting FuncInfo; cleanup funclets inherit the
parent's EBP frame. Neither is an ordinary callable C++ interface. This report
does not claim native unwind execution, ABI replacement or game validation.
No Ghidra names/comments were changed for these already-named support rows.

Original PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Compiler support classifications

| Address | Existing name | Exact evidence |
| --- | --- | --- |
| `00C61CB0` | `EH_SceneRecordMap_Insert_DestroyCompletedMessage_00C61CB0` | MSVC unwind action: bytes 8d4db0e918567aff; lea ecx, [ebp - 0x50]; jmp 0x4072d0. FuncInfo 00d893dc, selector 00c61cb8, unwind entry 00d893d4 state 0->-1 references this exact entry. Compiler cleanup funclet using parent EBP, not an ordinary game entry point. |
| `00C64D28` | `EH_MissionEntityLock_Construct` | 10-byte MSVC FH3 dispatcher: bytes b878d2d800e9111ef9ff; MOV EAX,00d8d278; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C64DA8` | `EH_FrameJobOwner_Construct` | 10-byte MSVC FH3 dispatcher: bytes b830d3d800e9911df9ff; MOV EAX,00d8d330; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C64DC8` | `EH_FrameJobOwner_Destroy` | 10-byte MSVC FH3 dispatcher: bytes b85cd3d800e9711df9ff; MOV EAX,00d8d35c; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C64EE8` | `EH_FrameJobPool_FreeFailedAllocation` | MSVC unwind action: bytes 8b45e850e8bb16f9ff59c3; mov eax, dword ptr [ebp - 0x18]; push eax; call 0xbf65ac; pop ecx; ret . FuncInfo 00d8d4f0, selector 00c64ef3, unwind entry 00d8d4e8 state 1->0 references this exact entry. Compiler cleanup funclet using parent EBP, not an ordinary game entry point. |
| `00C64EF3` | `EH_FrameJobPool_GetSingleton` | 10-byte MSVC FH3 dispatcher: bytes b8f0d4d800e9461cf9ff; MOV EAX,00d8d4f0; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C64F93` | `EH_MissionEntityLock_Get` | 10-byte MSVC FH3 dispatcher: bytes b8ecd5d800e9a61bf9ff; MOV EAX,00d8d5ec; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C65157` | `EH_LiveEffectReferences_Reserve` | 10-byte MSVC FH3 dispatcher: bytes b8a4d8d800e9e219f9ff; MOV EAX,00d8d8a4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C65B48` | `EH_UnitRegistryCountGrowth` | 10-byte MSVC FH3 dispatcher: bytes b80ce5d800e9f10ff9ff; MOV EAX,00d8e50c; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C65C68` | `EH_LiveEffectManager_Construct` | 10-byte MSVC FH3 dispatcher: bytes b898e6d800e9d10ef9ff; MOV EAX,00d8e698; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C65E73` | `EH_LiveEffectManager_GetSingleton` | 10-byte MSVC FH3 dispatcher: bytes b820ead800e9c60cf9ff; MOV EAX,00d8ea20; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C68808` | `EH_handler_00c68808` | 10-byte MSVC FH3 dispatcher: bytes b88019d900e931e3f8ff; MOV EAX,00d91980; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C68833` | `EH_handler_00c68833` | 10-byte MSVC FH3 dispatcher: bytes b8b419d900e906e3f8ff; MOV EAX,00d919b4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C688E8` | `EH_handler_00c688e8` | 10-byte MSVC FH3 dispatcher: bytes b8e41ad900e951e2f8ff; MOV EAX,00d91ae4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C68CF7` | `EH_handler_00c68cf7` | 10-byte MSVC FH3 dispatcher: bytes b8c01fd900e942def8ff; MOV EAX,00d91fc0; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C68FB4` | `EH_handler_00c68fb4` | 10-byte MSVC FH3 dispatcher: bytes b8d022d900e985dbf8ff; MOV EAX,00d922d0; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C6900C` | `EH_handler_00c6900c` | 10-byte MSVC FH3 dispatcher: bytes b82823d900e92ddbf8ff; MOV EAX,00d92328; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C69088` | `EH_handler_00c69088` | 10-byte MSVC FH3 dispatcher: bytes b84024d900e9b1daf8ff; MOV EAX,00d92440; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C6918B` | `EH_handler_00c6918b` | 10-byte MSVC FH3 dispatcher: bytes b8a025d900e9aed9f8ff; MOV EAX,00d925a0; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C691EB` | `EH_handler_00c691eb` | 10-byte MSVC FH3 dispatcher: bytes b85826d900e94ed9f8ff; MOV EAX,00d92658; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C6920B` | `EH_handler_00c6920b` | 10-byte MSVC FH3 dispatcher: bytes b88426d900e92ed9f8ff; MOV EAX,00d92684; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C84B64` | `EH_UnitPartConstruction` | 10-byte MSVC FH3 dispatcher: bytes b8f433db00e9d51ff7ff; MOV EAX,00db33f4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C878AC` | `EH_LiveEffectReferences_Append` | 10-byte MSVC FH3 dispatcher: bytes b8d871db00e98df2f6ff; MOV EAX,00db71d8; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C8F9A8` | `EH_handler_00c8f9a8` | 10-byte MSVC FH3 dispatcher: bytes b8ac05dc00e99171f6ff; MOV EAX,00dc05ac; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C9184D` | `EH_UnitInstance_OnWrecked` | 10-byte MSVC FH3 dispatcher: bytes b83c29dc00e9ec52f6ff; MOV EAX,00dc293c; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94CE8` | `EH_EffectDeletionLock_Construct` | 10-byte MSVC FH3 dispatcher: bytes b8e86adc00e9511ef6ff; MOV EAX,00dc6ae8; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94D33` | `EH_EffectDeletionLock_GetSingleton` | 10-byte MSVC FH3 dispatcher: bytes b8506bdc00e9061ef6ff; MOV EAX,00dc6b50; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94E90` | `EH_LiveEffectManager_Insert` | 10-byte MSVC FH3 dispatcher: bytes b8fc6cdc00e9a91cf6ff; MOV EAX,00dc6cfc; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94EA8` | `EH_EffectDeletionList_GrowCount` | 10-byte MSVC FH3 dispatcher: bytes b8286ddc00e9911cf6ff; MOV EAX,00dc6d28; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94F99` | `EH_LiveEffectManager_Destroy` | 10-byte MSVC FH3 dispatcher: bytes b8446edc00e9a01bf6ff; MOV EAX,00dc6e44; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C94FB8` | `EH_LiveEffectManager_EnqueueDeletion` | 10-byte MSVC FH3 dispatcher: bytes b8706edc00e9811bf6ff; MOV EAX,00dc6e70; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00C96770` | `EH_UnitPartPointerVector_InsertCopies` | 10-byte MSVC FH3 dispatcher: bytes b8a88adc00e9c903f6ff; MOV EAX,00dc8aa8; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA6D58` | `EH_PendingEntityList_GrowCount` | 10-byte MSVC FH3 dispatcher: bytes b808a0dd00e9e1fdf4ff; MOV EAX,00dda008; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA6DC8` | `EH_PendingEntity_Destroy` | 10-byte MSVC FH3 dispatcher: bytes b8e4a0dd00e971fdf4ff; MOV EAX,00dda0e4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA6DE8` | `EH_PendingEntity_Kill` | 10-byte MSVC FH3 dispatcher: bytes b810a1dd00e951fdf4ff; MOV EAX,00dda110; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA6E60` | `EH_PendingEntity_Drain` | 10-byte MSVC FH3 dispatcher: bytes b804a2dd00e9d9fcf4ff; MOV EAX,00dda204; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA6EF9` | `EH_MissionEntity_LuaSelfObject` | 10-byte MSVC FH3 dispatcher: bytes b8c4a2dd00e940fcf4ff; MOV EAX,00dda2c4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA70D0` | `EH_MissionEntity_OnKilledReleaseLuaSelf` | 10-byte MSVC FH3 dispatcher: bytes b8e4a4dd00e969faf4ff; MOV EAX,00dda4e4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CA71A0` | `EH_UnitClassLuaBinding` | 10-byte MSVC FH3 dispatcher: bytes b810a6dd00e999f9f4ff; MOV EAX,00dda610; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
| `00CB63A0` | `EH_ShadowTargetSingleton_Construct` | 10-byte MSVC FH3 dispatcher: bytes b8c4c5de00e99907f4ff; MOV EAX,00dec5c4; JMP 00bf6b43; FuncInfo magic 19930522. Compiler exception dispatch, no independent game logic. |
