# CC12 tick-element sublist cleanup ABI readiness

Baseline: `2307e3374e67761712886394517ecd4e18ee8c3b`.
Packet: `cc12_tick_element_sublist_cleanup_ABI_readiness`; address `00874F00`.
This is a complete owned-body, read-only audit. It adds no Source, build,
Ghidra mutation, reconstruction, Native ABI or gameplay credit.

The whole `00874F00..00874F63` span is 100 bytes. It contains 97 bytes/42
instructions reachable from normal entry and a skipped three-byte
`LEA ECX,[ECX]` at `00874F0D`. Linear decoding therefore produces 43
instructions. All 14 reachable basic blocks and 20 internal edges are covered.

There is **one physical indirect CALL**, `CALL EAX` at `00874F5A`, despite
metadata reporting zero calls/resolved callees. It uses the captured current
head as ECX and pushes DWORD `1`; the target is loaded from that head's current
profile pointer and its current slot zero after the unlink phase. No actual
target address, profile table or callee implementation is established here.

## Target, bytes and complete control flow

Project-aware live queries verified the configured `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 32-bit image base `00400000`.
Live and snapshot counts were both 64,729. The complete live 100 bytes equal
the file-backed installed PE `.text` span at RVA/file offset `00474F00`.
Image SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
owned-body SHA-256 is
`e3f7894ef6fe0b62f548fa5841e8dcfa2144db535e47de0545c6877e2b0ef611`.

The JSON report retains every byte/instruction, all branch boundaries and the
complete graph below. The skipped `8d4900` encoding is inert and has no normal
owned incoming edge. There is no inline branch table, x87/SSE instruction,
direct child call, tail call, or local exception frame in the owned body.

| Block start | Last byte | Action and normal successors |
| --- | --- | --- |
| `00874F00` | `00874F0A` | Save ESI/EDI, capture list, zero EDI; count zero to `61`, otherwise `0B` |
| `00874F0B` | `00874F0C` | Jump over the three skipped bytes to `10` |
| `00874F10` | `00874F18` | Capture current head and its `+08`; nonzero to `28`, zero to `19` |
| `00874F19` | `00874F1D` | Current head `+0C` nonzero to `24`, zero to `1E` |
| `00874F1E` | `00874F23` | Signed current count greater than 1 to `54`, otherwise `24` |
| `00874F24` | `00874F27` | Recompare captured `+08`; zero to `30`, nonzero edge to `28` |
| `00874F28` | `00874F2F` | Read current head `+0C`, store at captured previous `+0C`, jump `35` |
| `00874F30` | `00874F34` | Read current head `+0C`, store list head, continue `35` |
| `00874F35` | `00874F3B` | Reread current head `+0C`; zero to `44`, nonzero to `3C` |
| `00874F3C` | `00874F43` | Read current head `+08`, store current next `+08`, jump `4A` |
| `00874F44` | `00874F49` | Read current head `+08`, store list tail, continue `4A` |
| `00874F4A` | `00874F53` | Clear head `+0C`, then `+08`, decrement current count; continue `54` |
| `00874F54` | `00874F60` | Load current slot-zero target, push 1/call; reread count: nonzero to `10`, zero to `61` |
| `00874F61` | `00874F63` | Restore EDI/ESI and plain RET |

The graph includes the syntactic nonzero edge from `24`. Every ordinary
arrival at that block already has the unchanged captured previous value zero,
so that edge is redundant under the established local data flow. This does
not remove the separately reachable previous-neighbor block `28`.

## Raw list fields and exact mutation schedule

Let L be entry ECX, captured in ESI; EDI is set to zero. L has a current head
DWORD at `+00`, tail DWORD at `+04` and count DWORD at `+08`, establishing a
minimum direct list extent of `0Ch`. Zero is the only entry/loop empty test;
negative or inconsistent counts receive no general validation.

Each iteration captures N from current `[L+00]` without a null guard, then
captures P from `[N+08]`. Previous/next names describe the reciprocal link
operations; they do not establish a recovered C++ class. If P is nonzero,
the routine reads current `[N+0C]` and stores it at `[P+0C]`. If P is zero,
the routine first tests current `[N+0C]`. When that is also zero and the
current count interpreted as signed 32-bit is greater than 1, it skips **all**
unlink stores, both node-link clears and count decrement, proceeding directly
to the slot-zero call. The count test is signed JG, not an unsigned threshold.

On the remaining P-zero path, a fresh `[N+0C]` read is stored in `[L+00]`.
After either first splice/head store, another fresh `[N+0C]` read controls
the second splice. If nonzero, current `[N+08]` is stored at that next node's
`+08`; otherwise current `[N+08]` is stored in `[L+04]`. Then `[N+0C]` is
cleared before `[N+08]`, followed by an actual current-memory DWORD decrement
of `[L+08]` modulo 2^32. No earlier count snapshot substitutes for this RMW.

All receiver/node/neighbor reads and stores remain in their physical order.
An indirect store can alias the list, captured node or other fields, so neither
the second link reads nor the subsequent profile reads may be cached across
those writes. N itself remains the captured receiver even when `[L+00]` changes.
The minimum direct node extent is `10h`: this body reads profile `+00` and
reads/writes links `+08/+0C`; it does not directly access node `+04`.

## Current virtual target and cleanup limits

At `00874F54`, the routine reads current `[N+00]` into EDX, then current
`[EDX+00]` into EAX. Both reads follow any unlink/count changes. It pushes
DWORD 1 and calls EAX with ECX still N. The dispatch uses the captured node's
current profile, not a cached top-of-loop profile or the list's new head.

This is a scalar-deletion-shaped slot-zero request. The actual virtual target
may destroy/free the node, update list state, recurse, throw or fail, but none
of those callee effects is established without its identity and body. There
is no direct owned `free`, allocator, registry getter, lock, or fallback call.
The `P=0, next=0, signed count>1` path makes the same request while leaving its
owned list changes undone. The caller adds no progress repair or cycle bound;
termination depends on current state and normally returning callee effects.

After a normal callee return, the routine rereads current `[L+08]`. If nonzero,
it reloads the current head at the next iteration. It neither saves nor uses
the prior node as the next iteration's node. There is no local rollback or EH
cleanup if a store faults or the dispatched callee does not return normally.
Completed unlink/count changes are not restored on those paths.

## Register ABI, stack and residual state

Only incoming ECX is consumed as an owned ordinary data input. Incoming
EAX/EDX are not used on a nonempty path before definition; incoming ESI/EDI
are saved and restored. EBX/EBP are not locally modified. No caller argument
is read by this body. No hidden incoming arithmetic register or floating input
is required by the complete owned schedule.

For entry ESP S, saved ESI is at S-4 and saved EDI at S-8. Each dispatch pushes
flag 1 to S-12, and CALL pushes return address `00874F5C` to S-16. The wrapper
does not reclaim the flag. A normally returning target must therefore consume
that one DWORD, as a RET4-equivalent callee would, and preserve ESI=L/EDI=0
for the subsequent count test. The target's additional incoming-register or
stack requirements are unknown; an arbitrary cdecl callback is not justified.
The local epilogue restores both saved registers and uses plain RET, consuming
no caller-supplied argument. This conclusion is conditional on balanced calls
and the required target nonvolatile-register contract.

If the initial count is zero, EAX/EDX remain their incoming values and ECX
remains L. After one or more dispatches, EAX/ECX/EDX are the last callee's
residual values; the body does not overwrite them after the final call.
There is no uniform captured-receiver or other semantic return value produced
by the owned instructions. No Source return type is forced from decompiler
`void` or metadata `undefined(void)`.

On ordinary return with EDI still zero, the final count comparison is CMP 0,0:
ZF/PF are 1 and CF/OF/SF/AF are 0. POP/RET preserve those arithmetic flags.
This does not establish every EFLAGS bit or callee fault/exception behavior.

## Parent composition and current Source limits

The primary-corrected parent `00875490` is 32 bytes through `008754AF`, has
zero physical CALLs and only tail-jumps here after adding `1Ch` to its original
receiver. Its outgoing EAX/EDX are the second current link pair, not implicit
inputs consumed by this nonempty body. If this list is initially empty, those
parent values pass through unchanged. No direct `00875280` getter edge is
introduced by either body. Possible behavior of an unknown virtual target is
not proof of a transitive getter dependency.

The current live parent comment retains the original historical wording and
the primary correction append. Its old 30-byte/single-caller hypothesis must
not override the appended physical proof. The original parent report's own
document pin consequently differs after that approved append; its newer
`primary_doc_pin` matches. The remaining eight historical pins still match.

The admitted tick-registration Source constructor initializes outer receiver
`+1C/+20/+24`, which correspond to this L-relative head/tail/count through the
parent's `+1C` adjustment. Its whole 152-byte/52-instruction compiled body,
unchanged actual getter and Source EH, and existing three checks are pinned
through its current primary report. All 44 selected Source pins and current
constructor document pin match. This provides structural Source context,
not a concrete child-node producer, callable profile or executed lifecycle.

Current Source exact-address searches find only the unit-destructor plan and
pure virtual `destroy_tick_element_310()` Host slot for `00874F00`. The
fixed-step Source has subnode link-offset constants and action classification;
tick-element override tables are numeric metadata. Their parent `D0DEC8`
profile is not evidence of any captured child node's current profile. The
existing finite singleton deletion dispatcher covers its own recovered owner
domain, including `D0DEA0` registry retirement, and provides no established
adapter for this dynamic subnode slot. None may be substituted as this callee.

A Source implementation needs a recovered actual subnode/profile producer and
concrete deletion contract before choosing dispatch bindings. This audit does
not invent a callback, callable Native table, free shim, getter call or Source
prototype. No child/caller/handler/profile/table/string bytes were read. Whole
byte/CFG/ABI checks, current evidence pins, JSON and staged two-file whitespace
checks passed. No build, probe, test, CMake/ledger edit or Ghidra repair occurred.
