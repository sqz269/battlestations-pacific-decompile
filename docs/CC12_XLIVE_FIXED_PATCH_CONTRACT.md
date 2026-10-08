# Installed xlive fixed-RVA patch contract

The installed game-local `xlive.dll` removes the Original constructor's online IPC initialization call. Its fixed target also explains the captured Source startup access violation. This is installed-runtime interposition, not the unmodified Original PE contract, and it does not identify a genuine compatible peer.

This is a read-only audit: no Original/DLL/Source edits, SDK or game execution, account/pipe activity, Ghidra mutation, or new native reconstruction. The accompanying [JSON report](../reports/cc12_xlive_fixed_patch_contract.json) embeds the bounded DLL/Original bytes, decoded instructions, relocated dump bytes, debugger excerpts and SHA-256 pins.

## Exact bounded patch

All DLL addresses here are **RVAs** in the installed override, SHA-256 `71b50b3e91b5e17603f1d8fd44f1fc1da53c305f191dd126fb5059e5d3c841b2` (3,833,856 bytes; preferred image base `10000000`, image size `004FE000`, file version `1.0.0.0`). Original executable addresses are preferred **VAs**, base `00400000`; its SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The entire patch normal body is `0018C210..0018C2C2`, 179 bytes. The initializer calls thunk `0015965D` at `0017BF2B`; the thunk jumps to that body. It prepares five NOP bytes, obtains an argument-tail pointer, and performs these two copies in order:

| Copy call RVA | Destination | Bytes | Payload | Original meaning |
| --- | --- | ---: | --- | --- |
| `0018C271` | `[DLL+004D8450] + 00640F5E` | 5 | `90 90 90 90 90` | Replaces `00A40F5E: E8 ED B2 00 00`, exactly `CALL 00A4C250` |
| `0018C28C` | `[DLL+004D8450] + 004F841F` | 4 | Little-endian argument-tail pointer | Replaces only the immediate operand of `008F841E: PUSH 00CE8168`; Original literal is `cachedload` |

The second instruction is followed by `PUSH 0`, `LEA ECX,[ESP+28]`, and `008F8429: CALL 0073D410`. The first write leaves `00A40F58: LEA ECX,[ESI+3AC]` and `00A40F63: CALL 00A4D5D8` (`XOnlineStartup`) intact. It neither substitutes another IPC implementation nor calls a server.

The argument-tail helper `00185920..00185A1D` calls imported `GetCommandLineA`, skips a quoted executable token through its closing quote or an unquoted token through space/tab, skips subsequent spaces/tabs, and returns the remaining pointer. The AV dump captures its prepared return value `007299C3`; that memory begins with the Source command line's quoted `--frames`, `3`, `--log` arguments. The second write is not reached in that capture.

Before the patch, the initializer calls `0017C1F0..0017C29B` through thunk `0015B539`. This helper obtains `GetModuleHandleA(NULL)`, reads PE `SizeOfImage`, and requests `VirtualProtect(base, SizeOfImage, 40h, &oldProtect)`. It does not check that result or restore protection within its captured body; it subsequently calls `CloseHandle` on the module value. The patch body contains no version/hash/original-byte/size check on either fixed destination. These findings cover these bounded bodies, not every possible write or policy in the DLL. The complete static assignment chain for global `004D8450` was not recovered; the dump directly establishes its relevant value.

## Captured Source fault

The preserved dump loads Source at `10000000..104D8000` and the override at `698A0000..69D9E000`. The override's destination-base slot at actual address `69D78450` contains `10000000`, and the first local payload is exactly five `90` bytes. Therefore the first destination is `10640F5E`, beyond the loaded Source image.

Copy thunk `00154FD1` jumps to `00318B40`, an overlap-capable memory copy. For count five, its short-copy branch reaches `00319047: MOV EDX,[ESI]` and `00319049: MOV [EDI],EDX`. The exception is a write AV at actual instruction `69BB9049`, with `EDI=10640F5E`, `EDX=90909090`, `ESI=001ACF60`, `EAX=5`, `ECX=1`. The raw stack includes return address `69A2C276` followed by destination `10640F5E`, source `001ACF60`, and count `5`. This identifies the first copy independently of the debugger's approximate exported-symbol name and unreliable frame unwind.

All ten retained DLL code spans match the current installed DLL after applying only the PE's recorded `HIGHLOW` relocations for its actual loaded base. The captured second target, had execution reached it, would be `104F841F`, also beyond Source's image end. The first store faults; neither completed Original patching nor successful Source modification is claimed.

The debugger log reported an `.ecxr` error and could not resolve `!address` because ntdll type symbols were unavailable. The register dump, raw memory, module records and matching bytes support the findings without treating those failed commands as successful evidence.

## IPC consequence and Source boundary

Current disk and bounded live Ghidra bytes agree at the patch sites and the direct client-chain calls:

- `00A40F5E -> 00A4C250`: initializes the slot at owner `+3AC`.
- `00A4C264 -> 00A4C030`: the slot wrapper passes a local output address; on a nonnegative result it publishes the returned handle, otherwise null.
- `00A4C0EA -> 00A5DE34`: the factory passes first argument `EBX=0`.
- `00A5DE50 -> 00A5E844`: the wrapper forwards that mode unchanged.

The existing [peer bootstrap audit](CC12_ONLINE_PEER_BOOTSTRAP_READINESS.md) establishes mode zero's parent-PID named-pipe client path and leaves the genuine external parent/server bootstrap unidentified. Its entire stored byte regions for these four callees still match the current Original file. Its deeper parent-discovery and protocol conclusions are reused as prior bounded evidence, not newly reconstructed here. Analyst `XLive`/`BSP` names and this pipe shape do not establish vendor ownership.

If the first patch succeeds against this Original, that constructor no longer calls the initializer and thus no longer enters the client chain at that site. This provides a concrete reason that a successfully running installation using this override need not reveal the genuine peer required by the unmodified path. It does **not** prove the bypass was executed in any particular older Original observation, identify a server, prove that no server ever existed, or establish the skipped slot's preimage and later lifetime safety.

Current `src/native_online_manager_lifetime.cpp:106` retains `initialize_native_online_ipc_slot_00a4c250`; `src/native_online_ipc.cpp:84` retains mode-zero open. Both files, `src/xlive_pipe_transport.cpp`, and `src/game_native_online_process.cpp` match the prior peer audit's SHA-256 pins. This audit does not authorize importing the override's bypass into Source.

## Identity and validation limits

The historical AV executable hash `cb346fc3918014f20dfd57d22525ed3d1dd6e9aa8a3406cf4e7183044f17989a` comes from the prior startup report. That build path has since changed, so this audit does not pretend to revalidate the historical whole file. The dump independently retains its module timestamp `6AC73BFC`, base and image size. The JSON records the separately sampled current build identity.

The preserved genuine runtime has file version `2.0.673.0`, SHA-256 `79da26ab6b2dc25936c3354087de0ba41da1a8b62924972e9100c29b95d34385`; its provenance records extraction from the pinned installed redistributable without executing the installer. It is distinct from the game-local override. The existing frozen successful Source executable still matches `eae233dd002584e779d77df107d8ba86f289ba7181e761aed787f5bc59c0c474`. Prior primary evidence shows a finite three-frame title smoke and exit zero using the genuine runtime. That does not validate faithful full constructor/IPC startup, menu, sign-in, shutdown or gameplay.

Every live query verified Ghidra project `bsp`, program `/battlestationspacific.exe`, language and image base through the repository client. No DLL was imported into that project. Offline verification used pefile 2024.8.26 and Capstone 5.0.7; declared instruction regions decode without gaps, direct call displacements agree, and retained code matches the dump with accounted relocations. The live report-call verifier checked all four chain rows with zero failures. A final seal check verified all 19 retained regions and 16 dump capture/code digests, unchanged Original/DLL file hashes, and the prior debugger log/dump pins. No C++ changed, so no build or new tests were warranted.

## Next evidence

A faithful Original runtime claim needs pinned Original/runtime identities, observed pre/post patch-site bytes, and actual client/parent activity. A modded Original run cannot substitute. Admission of the genuine client still needs the real parent bootstrap and compatible protocol service, or authoritative unmodified runtime evidence explaining its absence. A dummy peer, silent initializer omission, or a title-only Source smoke would not supply that evidence. A separately requested override compatibility mode would need its own explicit behavioral and lifetime contract.
