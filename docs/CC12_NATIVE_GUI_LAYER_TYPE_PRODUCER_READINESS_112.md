# Native GUI Layer type producer readiness (packet 112)

The next bounded Source work is a Layer role in the existing
`GameNativeGuiTextProcess`, over its same canonical VFS counter/root and existing
Element role. No missing callee prevents that capability. It is not a fixed-VA
Original provider, automatic startup change, upper Page adapter or production
graph admission. Packet 108 already supplies the required live three-word query
input; its algorithm is not re-audited here.

## Whole Original initializer

`CG_static_init_00cd73f0` owns `[00CD73F0,00CD743F)`, 79 bytes and 16 instructions.
It has no arguments, finishes with plain `RET`, ignores a receiver, and has no
local exception frame or cleanup. The two calls are its only external transfers.
Its CRT-array reference is `00CE349C`, whose DWORD is `00CD73F0`.

| Site | Exact effect |
| --- | --- |
| CD73F0 / CD73F7 | Read BYTE F8BF48; any nonzero value returns at CD743E. |
| CD73F9 | Prepare ECX = F8BC88, the actual Element descriptor target. |
| CD73FE | Write BYTE F8BF48 = 1. |
| CD7405 | Write DWORD F8BF94 = D5CB0C, the `cGuiLayer` name address. |
| CD740F | Call AA8200 on that Element target. |
| CD7414 / CD7419 | Read CURRENT DWORDs F8BC88 and F8BC8C, in that order. |
| CD741F / CD7424 | Store the captured words to F8BF8C and F8BF90, in that order. BOTH parent reads precede either store. |
| CD742A | Call the real shared counter getter 6FAC20. |
| CD742F / CD7432 | Read DWORD returned-owner+4 as n; form n+1 modulo 32 bits. |
| CD7435 | Store n+1 to that SAME returned owner's +4. |
| CD7438 / CD743E | Publish n to F8BF88; return. Counter increment precedes own-ID publication. |

The separate one-byte guard and 16-byte descriptor are loader-zero preimages;
the descriptor contains three ID words and one name word. The literal at
D5CB0C is exactly `cGuiLayer` followed by NUL. These are analysis/PE preimages,
not a running-game observation or proof that initialization has returned.

## Immediate provider contracts

AA8200 is already reconstructed. Its complete 62-byte / 17-instruction body
checks the separate F8BC80 byte, captures the ECX target, and on a clear guard
sets that guard, writes the target's +8 Element name, calls the existing root
initializer on 109DB84, copies its current own ID to target+4, obtains the SAME
6FAC20 counter, increments owner+4, then publishes the captured ID to target+0.
A nonzero Element guard skips the name/parent/counter work. No new Element or
root initializer is needed for the proposed Layer capability.

The accepted 6FAC20 body is 181 bytes / 48 instructions. Its fast path returns
the first current 109DB7C publication. The slow path captures the shared manager
section, enters it, rechecks publication, allocates/initializes the eight-byte
counter if needed, publishes it, obtains the registration manager again, then
registers the CURRENT publication. It releases the originally captured section
before the final publication reload. The reached owner must be genuine, live
and nonnull for the initializer's +4 access. There is no Layer null fallback.
Its shared-manager/Source CRT/exception contract remains the existing accepted
one; this packet does not reconstruct its lower providers or native EH helpers.

| Authority | Current Source provider | Bounded conclusion |
| --- | --- | --- |
| Counter / publication / deletion | VFS application's `NativeVfsOwnerServices::types()`; SAME `TypeIdCounterLifetime`, publication and shared host manager; deletion binding is installed before type getters | Reuse this exact retained service; no second counter or manager. |
| Root guard / descriptor | SAME `GameNativeTypeStorage::light_types()` view; `LightTypeBootstrap` borrows those references | Existing initialized canonical root is available; no new root/default descriptor. |
| Element guard / two IDs | Existing `GameNativeGuiTextProcess` members and AA8200 helper, completed during Text startup | Reuse those exact members and actual parent initializer. |
| Layer guard / three IDs / name | No role, startup method or lineage getter exists in the current process | This is the actual missing Source composition. |
| Layer query input | Packet 108's required borrowed three-word live view | Supply the SAME completed Layer member; no parallel array. |

The normal host currently starts Text types with this VFS counter and root view.
Its documented Source bootstrap is late relative to Original CRT execution and
does not establish Original absolute numeric IDs. This packet adds no host call.

## Partial completion and lifetime

The Native guard is set before either call and is never rolled back here.
AA8200 failure leaves the Layer guard/name sticky, with no Layer parent or own-ID
publication yet; the reached Element/root/counter state may also be partial.
6FAC20 failure occurs after both Layer parent stores; earlier counter allocation,
publication or registration effects may already exist. On normal return the
counter store precedes the final own-ID publication. There is no cleanup,
decrement or `atexit` registration in CD73F0. A subsequent Native entry with a
nonzero guard simply returns, including after an interrupted earlier attempt.
That return must not be confused with successful Source initialization.

The existing process owns Source backing objects for the Element/Text roles;
they are expressly not fixed-address Original storage. A matching Layer member
can be the sole Source Layer authority, initialized from its loader-zero
preimage. Its lineage must remain unavailable until complete return. Zero words
before completion are not a usable descriptor or fallback; zero after genuine
initialization remains an ordinary ID. The process cannot retain the borrowed
VFS counter/root/services by storing pointers: the caller must keep the same
graph alive through initialization, all reached callbacks, consumers and shared
drain, and forbid a late initializer after that domain retires. External
serialization must cover startup, reentry, exposure and domain shutdown.

## One concrete next Source packet

Modify only `include/bsp/game_native_gui_text_process.hpp` and its already
registered `.cpp` (plus a new receipt). Add one private Layer guard, descriptor
and completion state to this existing owner, with explicit
`initialize_layer_type_once_00cd73f0(TypeIdCounterLifetime&,
LightTypeBootstrapStorage)` and a required-reference
`layer_lineage_00f8bf88()` getter.

Use a narrow Source admission: pool and Text startup must already have completed.
Before effects, verify exact identity with the process's stored counter, root
descriptor and root guard. A completed repeat may return only in that same
domain; an entered failure/reentry cannot replay or expose partial words. Mark
the attempt entered before Native effects, include it in
`native_operation_interrupted()`, and mark it returned only after final own-ID
publication. This is explicit Source failure policy, not a Native guard change.

Within the admitted body, keep the tabled guard/name/AA8200/capture/store/counter
schedule. Use the existing actual Element helper even though the admitted prior
Text startup normally made its guard nonzero. Capture the two current Element
words before either Layer parent store. The existing generic initializer's
per-word copy loop is not by itself an exact proof of that capture order; do
not silently claim such a proof. Preserve current native cells through explicit
volatile access where order is material, without changing the shared generic API.
The Layer name remains the opaque D5CB0C evidence word. The public getter returns
the SAME three-word member to packet 108, without copying IDs or constructing a
query-only descriptor. No new counter, pool, owner, CRT registration, numeric ID,
normal-host call or original ABI adapter is needed or authorized by this plan.

## Evidence and scope

Published Main `4b2507b6889c6da63fb48fede8505539131894cb` merged cleanly (exit 0)
into this worktree as `5b9c49d30aee469fdce81e8f911d20b18bdd5071`, preserving 108.
Fresh BSP wrapper batches verified the configured `C:/Users/sqz269/bsp.gpr` /
`/battlestationspacific.exe`. Ten relevant ranges, 382 bytes total, equal the
unchanged installed PE/loader image, including virtual zero-fill where relevant.
Whole CD73F0 SHA-256 is
`834216a540048a72140a02574ae495b1bb4fd04b913064b25bb89f850862c343`.
The Original executable SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The JSON receipt records thirteen unchanged Source/registration hashes, exact
provider references, the two primary call sites and verification result. Private
artifacts are sealed in
`local/cc12_native_gui_layer_type_producer_readiness_112_artifact_manifest.json`.
No Source, CMake, ledger or Ghidra mutation, build, test, probe or Native execution
was performed. Callee-byte checks and provider inspection do not establish the
Original EH/ABI, whole CRT timing, fixed-VA ownership or production readiness.
