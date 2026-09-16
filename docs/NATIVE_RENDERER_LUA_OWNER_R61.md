# Renderer Lua owner integration (R61)

The full renderer constructor `B32410` allocates a 0x4CC-byte Lua owner and
discards its returned pointer. Its registered lifetime and publication at
`F8D434` therefore need real construction and shutdown paths.

This packet brings the six reviewed owner/base/scalar implementations from
`agent/orch5-native-renderer-lua-owner` into current main. Source provenance is
recorded against commits `952d8a2bc` and `9447b175b`; unrelated branch work is
not imported. New singleton-deletion bindings dispatch profiles `D5E5A4` and
`D5E5A8` to their recovered scalar destructors.

## Native behavior

| Entry | Bytes | Contract |
| --- | ---: | --- |
| B1BA30 | 145 | Base profile; capture first manager lock; publish owner; obtain manager again before rereading current publication and registering; leave captured lock. |
| B1BAD0 | 153 | Unregister current publication, clear it, leave captured lock, stamp CE3818. Unregister clears the first matching slot and preserves vector length. |
| B1BB70 | 30 | Base destruction; free only when flags bit zero is set; return original address. |
| B1BB90 | 92 | Base construction; derived profile; actual 0x4C8-byte Lua owner at +4; genuine Lua open with mask 1 and canonical bootstrap services. |
| B1BBF0 | 82 | Close Lua; disarm base unwind cleanup before explicit base destruction. |
| B1BC50 | 30 | Derived destruction; optional ordinary free; return original address. |

Base unwind stages perform recovered profile and captured-guard cleanup.
Derived construction failure closes constructed Lua before base cleanup;
derived destruction failure still performs base cleanup. Secondary cleanup
failure terminates. Original FH3 maps and prior Ghidra values are retained in
[the report](../reports/native_renderer_lua_owner_r61.json).

The raw manager borrows a stable `NativeRendererLuaOwnerContext` at deletion
binding offset 124 (new binding size 128). It uses the same manager, publication,
string and Lua services through shutdown. No source callback is invoked by
treating an original numeric profile as a callable host table.

## Current validation

- Strict Win32 build and three existing CTests pass.
- Nine focused probe checks pass with all six complete copied native bodies.
  Base cases compare the entire 0x4CC-byte preimage and registration effects;
  derived cases compare the entire owner after closing real Lua. Freeing cases
  inspect only returned address bits and publication state.
- The probe uses the existing application raw VFS, actual string pool,
  fundamentals cache, Lua service binding and singleton manager. Its captured
  657-byte fundamentals buffer exactly matches the current installed loose
  script, SHA-256 `20e128ebe9a1cb7678e3324b0baae93adab4f34f4d96bd081fb7b86505f63526`.
- Shared shutdown dispatches both Lua profiles and drains the fundamentals,
  VFS, pool and manager. All observed publications clear.
- Twenty-eight fresh PE/live-Ghidra spans (1,076 bytes), 22 direct call/tail
  rows, linked provider edges and 24 same-process physical I386 modules are
  recorded. Evidence artifacts are sealed before the combined integration build.

Ghidra had omitted `ADD ESP,4` at B1BB85 and B1BC65 following `free` calls.
The supported repair tool cleared those two `CALL_RETURN` overrides, restored
the three-byte instructions, saved the project and refreshed exports. Fresh
full-body listings have no missing instruction starts.

## Limits

The native caller probe uses explicit ABI bridges to genuine source manager,
Lua and free helpers. Original helper bodies and FH3/SEH failure paths are not
executed. `DoFile` is installed as the real source callback; the probe does not
invoke it directly. The string adapter retains its established nonthrowing
release/returning-getter domain.

This closes a renderer-constructor dependency. Full `B32410` construction,
application renderer ownership, active rendering and gameplay remain open.
The installed script match does not establish retail asset provenance.
