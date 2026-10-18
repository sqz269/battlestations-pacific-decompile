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
