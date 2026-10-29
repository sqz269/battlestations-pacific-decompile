# Installed Bink sound callback machine contract

The installed `binkw32.dll` with SHA256
`7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724`
supports the declared two-level callback's **Win32 machine calling shape**.
This is a primary static inspection of that actual file, beyond the earlier
export-decoration and older-header evidence. Independent review and real
decoder/audio execution remain pending. No Bink DLL was loaded or called.

The PE is x86 PE32 with preferred base `18000000`. All addresses below are
preferred image addresses, not current loaded pointers or BSP-program addresses.
The analysis reads the file with PE/Capstone tooling and adds no vendor body,
function, source ABI, gameplay or activation credit. No Ghidra import/mutation
or source declaration change is made.

## Callback chain

| Installed code | Machine evidence |
| --- | --- |
| `_BinkSetSoundSystem@8`, RVA `CEA0` | Both reachable returns use `RET8`. At `1800CECE/CECF`, it pushes the second input DWORD and calls the supplied/selected function in EAX. There is no caller removal of that DWORD. |
| `_BinkOpenDirectSound@4`, RVA `11FC0` | Both reachable returns use `RET4`. The failure block returns EAX zero. The other return loads EAX with code address `18011CD0` at `180120AB`, then returns. |
| Returned function, RVA `11CD0` | Its three reachable returns use `RET18h`, consuming six DWORD arguments. No indirect/external jump leaves the inspected intraprocedural graph. Called helper bodies are not reconstructed or assumed equivalent. |
| `_BinkOpen@8` at `1800E251` | A statically reachable call invokes the saved return value through `[1802E59C]`, after six DWORD pushes at `E243/E244/E24B/E24C/E24D/E250`. It tests the scalar EAX result and performs no caller removal of those arguments. |

`BinkSetSoundSystem` stores a nonzero callback return at `1802E59C`. The later
indirect call through that exact word proves it is consumed as code, rather
than inferring a function-pointer return solely from a decorated export name.
The observed one-DWORD system callback, function-pointer EAX result and
six-DWORD stdcall sound callback match the source declaration's machine widths
and cleanup. The vendor sound state remains opaque. This does not independently
recover each parameter's semantic name, struct layout or a matching SDK header.

## Setup result is a limited witness

At `1800CED1..CEE5`, a null newly returned callback skips the store. The function
then reports whether the **existing** saved callback word is nonzero. Therefore
its signed integer result alone does not prove that this newly selected callback
opened successfully. A previous callback can remain. Earlier guards can also
reject a null input or a disallowed change before making the callback.

The source factory already ignores this ordinary result, matching its native
caller. No new success/failure policy is added. A runtime acceptance gate must
observe the required real callback/decoder/audio path and completed returns;
setup status is insufficient. Static graph reachability is not execution.

## Evidence and remaining gates

The bounded branch graphs contain 24, 66, 212 and 988 instruction starts for
the system setter, DirectSound opener, returned callback and BinkOpen window.
Each graph has no unresolved jump leaving its declared window; calls are
modeled as normal-return edges, not inlined vendor proof. The report retains
the installed-file hash, export RVAs, exact return bytes, source declaration
hash, scripts, listings and graph receipts. Initial export-to-next-export
listing windows are explicitly not treated as function-boundary proof.

Source factory/module ownership, actual loaded identity, genuine decoder
production, open/frame/audio/close, callback completion and module retention
still need their own evidence. No original BSP ABI/FH/SEH, hardware fault,
nonlocal transfer, concurrency, vendor-private object lifetime or unload
quiescence follows. Existing frozen game receipts keep their original scope.

[Machine evidence report](../reports/cc16_bink_installed_sound_callback_abi.json).
