# Damageable effect-by-ID wrapper identity readiness

The complete **42-byte** wrapper at `[00870CD0,00870CFA)` captures incoming
output/ID, obtains the current manager, then reads the caller's stack flag and
passes all three inputs to `008700E0`. It returns the captured output address
and uses `RET4`. Its prologue addresses a reserved local stack word for the
zero store; it does not dereference the output pointer. No Source implementation
is added.

Root first received the exact one-range metadata proposal, then explicitly
authorized this complete body. The lease was refreshed before opening only
`00870CD0..00870CF9`. No adjacent bytes, callee body, data cell, table, or handler
were opened. All live body captures finished at epoch 10 before Root's planned
separate parent annotation; Root was notified when this bracket was complete.

Baseline is `f12ca22f0b6fba86fcff4299ae471771fc99cb15`. Only this document and
`reports/cc12_damageable_effect_id_wrapper_identity_readiness.json` are new
tracked files. Complete metadata receipts and Source snapshots are under
`local/cc12_damageable_effect_id_wrapper_identity_readiness/`.

## Current identity and prototype evidence

The four single-address typed captures around prototype/body collection and
the final 18-instruction typed batch all pass at modification number **10**.
Repeated payloads are identical. They verify actual
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`, and default space `ram`.

Both exact-function and containing-function references equal `00870CD0`.
The sole record is currently named `BSP_EffectHandle_Acquire`, with
`no_return=false`, `is_thunk=false`, and no direct thunk target. The complete
AddressSet contains one range and exactly 42 addresses; this is not a min/max
estimate over a potentially discontinuous body. The descriptive name remains
a hypothesis, not a recovered symbol.

At the entry, exact and containing listing units are a one-byte
`InstructionDB`. Default and effective flow are both `FALL_THROUGH` to
`00870CD1`, override `NONE`, and no fallthrough override. Entry target arrays
and entry direct-call records are empty. These facts describe the queried
instruction only; they do not establish every internal call or the epilogue.

The existing `bsp.py ghidra proto` command returns:

```text
Signature: undefined BSP_EffectHandle_Acquire(void)
Body: 00870cd0 - 00870cf9
```

This zero-parameter undefined-return prototype is database metadata, not proof
of a no-argument ABI. It does not establish ECX/EDX inputs, stack arguments,
callee-saved registers, EAX result, stack cleanup, instruction order, or EH.
No prototype was repaired or accepted as an ABI contract. The separate body
audit below, rather than this prototype, supplies the wrapper's register proof.

The command also calls the existing `get_function_signature` endpoint, whose
response is a signature fingerprint rather than a C prototype. Its complete
raw response is retained, including aggregate instruction/block/call counts,
callee names, immediate-value summary, and basic-block hashes. The operand
summary and block hashes are not used as body or ABI evidence. Physical bytes
and listing were obtained only after the separate body authorization. In total,
22 typed responses, six prototype/identity responses, and eight body/identity
responses have **36 retained raw HTTP hashes**; typed before/after identities
are also preserved. Loaded Java CodeSource remains unattested.

## Exact physical and assembly audit

One body window was read: exactly 42 original-image bytes at file
offset 4,656,336, using the retained affine mapping. They equal the 42 bytes from
current Ghidra, SHA-256
`c7496df13da7da597978d5e88061aa915aedfc9d6d061b1aff570ff9019aaa60`.
The original image's size/mtime match the retained whole-image identity and stay
stable across this selected read. The older whole-image hash is retained as
historical; it was not recomputed by opening the rest of the executable here.
No PE headers or other executable bytes were read.

Capstone x86-32 consumes all 42 bytes as exactly 18 instructions. Every start
and length matches the current typed listing, and every start agrees with the
independently retained Ghidra assembly listing. All 18 remain inside the same
complete owner. Each has override `NONE`, no fallthrough override, and identical
default/effective flow, targets, and fallthrough. The two direct-call targets'
metadata has `no_return=false` and `is_thunk=false`; their bodies were not
opened. The 18 instructions are:

| Address | Instruction | Local effect |
| --- | --- | --- |
| `00870CD0` | `PUSH ECX` | Reserve/copy one local word. |
| `00870CD1` | `PUSH ESI` | Save incoming ESI. |
| `00870CD2` | `PUSH EDI` | Save incoming EDI. |
| `00870CD3` | `MOV EDI,EDX` | Capture incoming ID bits. |
| `00870CD5` | `MOV ESI,ECX` | Capture actual output address. |
| `00870CD7` | `MOV [ESP+8],0` | Zero the local word at entry ESP-4. |
| `00870CDF` | `CALL 004C1650` | Invoke manager getter unconditionally. |
| `00870CE4` | `MOV ECX,[ESP+10h]` | Read flag from entry ESP+4 after getter. |
| `00870CE8` | `PUSH ECX` | Pass current flag word. |
| `00870CE9` | `PUSH EDI` | Pass captured ID bits. |
| `00870CEA` | `PUSH ESI` | Pass captured output address. |
| `00870CEB` | `MOV ECX,EAX` | Use returned manager as lower-call receiver. |
| `00870CED` | `CALL 008700E0` | Invoke the genuine lower ID acquisition. |
| `00870CF2` | `POP EDI` | Restore incoming EDI under retained callee cleanup. |
| `00870CF3` | `MOV EAX,ESI` | Return captured output, independent of lower EAX. |
| `00870CF5` | `POP ESI` | Restore incoming ESI. |
| `00870CF6` | `POP ECX` | Consume the local word zeroed before both calls. |
| `00870CF7` | `RET 4` | Return and consume the caller's single flag word. |

With entry stack pointer S, the three initial pushes leave ESP=S-12. The zero
store is at S-4; there is no wrapper write through the output pointer. Under
the retained getter return contract, CE4 reads `[S+4]`. The subsequent lower
call sees receiver=manager and stack arguments output, ID, flag in that order.
The existing lower contract's `RETCh` leaves ESP=S-12 for the epilogue. These
callee cleanup contracts are retained dependencies, not fresh callee-body proof.

The wrapper's normal register interface is ECX=output address, EDX=ID bits,
one stack flag word, EAX=captured output address, and `RET4`. ESI and EDI are
saved/restored; ECX reloads the earlier-zeroed local. Under ordinary private-
frame preservation and no output/provider alias into save/local slots, these
restore incoming ESI/EDI and yield ECX=0. Arbitrary private-frame aliases are
not silently covered by that normal-return summary. Signed ID
meaning comes from the reviewed caller/lower Source, not arithmetic inside
this wrapper. No full-width flag normalization or ID test occurs in the body.
No internal conditional branch, local rollback, or EH-frame setup appears in
these 42 bytes. Child failure behavior and Native exception ABI remain held.

The flag read's timing matters: output/ID are captured before getter entry,
but the caller's flag word is loaded after getter return. A future Source API
must preserve that observable load order, or state a valid explicit alias
restriction, with private Source frame/context/binding aliases excluded.
A premature flag copy or zero-ID fast path is not admitted.

## Retained parent evidence stays separate

The accepted cross-field readiness report records `CF96 PUSH1` as the pending
wrapper flag and `CF98 PUSH0` as `00B66380`'s fallback. Its `RET4` consumes only
the fallback, leaving the flag pending. At `CFAF`, EDX carries the integer ID
and ECX points to actual output `S+28h`. That establishes the reviewed caller
edge. The separately authorized wrapper audit now proves its own register and
stack instructions, without opening the parent again. The Source integer-
conversion mode remains a distinct provider input.

The parent receipt also preserves assignment and cleanup ordering: dereference
the acquisition's returned output address, capture and compare the old row
owner, then enter the temporary's cleanup state. For a changed pointer, publish
the row before retaining the new owner and releasing the captured old owner.
Read the current temporary again for its later cleanup. No new parent-body,
`0087CA80`, `0087D1A0`, IAT, vtable, name, or handler query was made.

## Genuine current Source and the binding frontier

The bounded address-matched search of current `src/` and `include/bsp/` finds
comments, virtual service declarations, and field metadata for `00870CD0`, but
no implementation named for that address. This is a bounded search result, not
a claim that no arbitrarily named equivalent could exist. The wrapper cannot
be filled by treating those comments as a function body.

The complete current `gameplay_effect_acquisition.cpp` has these real bodies:

| Existing Source | Contract and remaining boundary |
| --- | --- |
| `acquire_gameplay_effect_by_id_008700e0` | Takes `GameplayEffectManager&`, `void*& out`, signed ID, full-width flag, and `GameplayEffectAcquisitionContext&`; returns `&out`. Uses the canonical manager map, actual definition references, and the current Lua/string/component services. |
| `acquire_gameplay_effect_by_name_00871b50` | Empty name writes null; otherwise resolves the name through the process index and invokes the real lower ID body. |
| `acquire_gameplay_effect_by_name_00871ba0` | Gets the current manager using `context.manager`, invokes the lower name body, and returns `&out`. This is the name wrapper, not an ID-wrapper implementation. |

The lower ID body stores null for `id <= 0` without releasing an incoming
output. A cache hit increments the actual definition's reference word. On a
miss it borrows current game Lua, loads `Effects[ID]`, uses only the flag's low
byte for non-table rejection, allocates/loads/identifies a definition, and
performs unique map insertion. Its final output load follows temporary string
and Lua cleanup. These are reviewed current Source behaviors. The separate
wrapper audit establishes its unconditional manager call even when ID is zero.
No zero-ID wrapper shortcut is admitted.

`GameplayEffectAcquisitionContext` borrows a manager context, `NativeStringStorage`,
an acquisition host, and a scalar component dispatcher. The manager context
shares its application's genuine `SingletonLifetimeDomain`, volatile publication
slot, and allocation words. Its `GameplayEffectManager` contains a canonical
`optional<map<signed ID, void*>>` projection. It is not the native raw 10h manager.

The separate `get_native_gameplay_effect_manager_004c1650` has a real Source
body over actual raw manager/effect publication cells. The raw tree helpers in
`native_effect_handle_acquisition.cpp` implement native find/insert mechanics.
They do not supply the full lower ID Lua miss path or component loading, and
their raw manager cannot be cast into the canonical map projection. Choosing
a genuine compatible representation and binding is an explicit future gate.

The definition allocation/constructor and identity helpers are concrete Source.
The component loader calls current component services; the dispatcher handles
its known tables and forwards remaining tables to the supplied service. Texture,
sound, remaining component readers, and zero-reference operations still require
their real contexts. `GuiLua51Host` borrows the actual embedded game Lua state;
its registry references are host handles, not raw 14h Native LuaObject storage.

The name wrapper additionally requires a live `NativeString`, a real `void*`
output object, and `bind_gameplay_effect_name_index_00f87670` with dependencies
that survive actual CRT cleanup. Bounded current Source search finds the
binding declaration/definition, but no caller of that setup function. This is
not proof that every possible embedding lacks a binding.

`release_native_ref_counted_handle_0041de40` is a real existing Source helper:
capture the current slot, skip a null owner, decrement owner+4, call the current
zero-argument virtual slot0 on zero, then clear the original slot after normal
return. The caller must provide live storage and a callable compatible lifetime
binding. Original profile vtable DWORDs alone are not executable rebuilt tables.
No new handle type, release policy, callback, or row assignment helper is invented.

Current `global_subsystems.cpp` invokes the genuine name API through an injected
acquisition context. The current scene host's acquisition paths still explicitly
log an unimplemented operation and return null. Those existing placeholders do
not close the DamageableClass binding; this packet adds no replacement stub.

## Source admission and remaining work

The authorized 42-byte body audit is complete. No further Native scope is
proposed. A future ordinary Source wrapper could use the genuine lower Source
only with its genuine manager/context and live output object, preserving the
capture/getter/flag-read/lower-call/return ordering above. An actual raw-storage
path would also need a compatible lower acquisition and provider composition;
the raw manager getter/tree helpers alone do not provide it.

No Source wrapper or new interface is admitted in this packet. The existing
lower/name interfaces are dependency evidence, not permission to silently adapt
raw parent slots into live C++ objects. Full DamageableClass composition,
drop-in Native ABI/EH, application binding, startup, and game behavior remain
held. The Compact Source fragment and the preceding destination's role remain
unchanged and unresolved.

No GPR write, POST, script, restart, configuration change, C++/CMake/ledger/
provider edit, build, test, probe, link, or runtime execution occurred. This
packet adds bounded physical/listing evidence and offline raw-response, Source
baseline, artifact-pin, and archive replay; it makes no runtime claim.
