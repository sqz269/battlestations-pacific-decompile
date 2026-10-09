# Mission Lua variant tag-6 child: 0074FB30

The complete 17-byte helper calls `00886920` before storing the full DWORD value
6 at `[ESI+4]`, then restores ESI and returns. It initially sets ESI to entry
ECX, so this is a receiver `+4` store if the child preserves that register.
There is no local source-payload access, profile stamp or other receiver store.

**Source closure remains held at `00886920`.** Incoming EAX, EDX and flags pass
through to that child without an owned read or overwrite. This establishes
delivery, not that the child ignores them or has a proven conventional ABI.
The primary's Astra worker owns its independent audit; this packet reads only
its current metadata and does not duplicate that body analysis.

## Scope and full-byte evidence

- Published baseline: `ef03278921416a877cb96569f64da20c7b136204`.
- Owner/lease: `agent/cc12_settings_live_fov_cell` /
  `cc12_lua_variant_tag6_child_readiness`.
- Owned body: `0074FB30..0074FB40`, 17 bytes / six instructions, one block,
  no branches, one direct call. Metadata passed the 300-byte gate first.
- Every live CLI batch verifies existing `C:/Users/sqz269/bsp.gpr`, project
  `bsp`, `/battlestationspacific.exe`, `x86:LE:32:default`, and image base
  `00400000`. Live/snapshot function counts both remain 64,729.
- The complete live byte span equals the physical PE and independently decodes
  through the plain `RET` at `0074FB40`. No listing gap or repair was found.
- Span SHA-256:
  `3d44dff971b203b5597087cc5fb87812946ffd65c26cba2e9b8b47d22e401a36`.

| Address | Bytes | Instruction |
| --- | --- | --- |
| `0074FB30` | `56` | `PUSH ESI` |
| `0074FB31` | `8b f1` | `MOV ESI,ECX` |
| `0074FB33` | `e8 e8 6d 13 00` | `CALL 00886920` |
| `0074FB38` | `c7 46 04 06 00 00 00` | `MOV DWORD PTR [ESI+4],6` |
| `0074FB3F` | `5e` | `POP ESI` |
| `0074FB40` | `c3` | `RET` |

## Registers, ordering and footprint

Let D be entry ECX and S be entry ESP. The helper saves incoming ESI at `S-4`
and sets ESI=D. At the child call, ECX still equals D, and ESI also equals D.
It pushes no child argument. EAX, EDX, EBX, EDI, EBP and flags have not been
changed by any owned instruction. There is no x87 or SSE operation and no
observed owned use of an incoming hidden data register.

The child must return with compatible zero-argument stack behavior, leaving
ESP at `S-4`; otherwise the subsequent `POP ESI`/`RET` would not balance.
The effective store address is **post-child ESI+4**. Preservation of ESI as D
is therefore a child contract requirement, not independently proven by this
six-instruction caller. With that contract and valid backing, the one local
receiver store writes bytes `D+4..D+7` after the child's full return. No prior
value is read. There is no cached field value or data-pointer reload.

The store establishes a minimum eight-byte receiver address extent, not an
eight-byte object or a complete variant layout. No receiver-relative access
to `+0`, `+8`, `+C` or `+10` occurs here. Child effects and aliases with the
separately accessed call stack remain outside that local footprint statement.
No source pointer is loaded or source pointee dereferenced by this helper.
The DWORD value 6 is observed; no payload representation or semantic value is
inferred from the tag number.

After the store, incoming ESI is restored from the saved slot, assuming valid
non-overlapping stack backing, and the plain RET consumes only the return
address. There are no public stack arguments. EAX/ECX/EDX and flags are residual
child results; the helper does not impose a receiver return or typed result.
Its own instructions never change flags. EBX/EDI/EBP preservation also depends
on the child. No generic prototype is used to erase these unresolved deliveries.

## Failure, alias and exception limits

The helper has no guard, local FS exception record, handler, allocation or free.
It calls the child before performing any receiver-relative access of its own.
For a null D, the child sees null first; only if it returns compatibly does the
helper attempt the DWORD store at address 4. If the child throws, faults or
does not return, the final store has not occurred. If the later store faults,
child effects have already occurred. No rollback or ownership guarantee follows.

The child may mutate the receiver before the final value 6 overwrites its `+4`
word. Moving that store before the call or removing the call changes observable
ordering. The receiver must remain live and writable through the call and store.
No overlap, bounds or alignment check exists; an alias with saved registers or
the return slot can affect restoration. Child/caller exception behavior remains
unproved. No cleanup, leak, callable profile or lifetime policy is inferred by
function name or metadata callee names.

## Accepted parent context and current Source

The published 369-byte parent audit records a nonidentity path that first calls
`00886920` with the destination. After that call, it reads the current source
tag and writes it to destination `+4`. If tag 6 is selected, `00886C52` calls
this helper with ECX equal to the destination and no pushed argument. The helper
therefore calls `00886920` again, then writes 6 again after its return. These are
two distinct child invocations on the normal selected path. Their redundancy
or equivalence is not established. The parent has no owned source-payload read
for this tag after reading the tag itself. This context comes from accepted
documentation/report pins; no caller body or switch data was reopened.

Current child metadata: `00886920..008869C7`, 168 bytes / 46 instructions,
nine blocks, 11 edges, five direct call sites and a generic `undefined(void)`
prototype. Its four distinct callee names include `_free` and sized-storage
pool operations. Only metadata was read; names do not establish its behavior,
field footprint, register use or release policy.

Bounded address searches, including forms without leading zeros, find no current
Source for `0074FB30` or `00886920`; the relevant reconstruction shards have no
records for either address at this baseline. No initializer, byte-copy, typed
owner, callback or no-op substitute is introduced. The existing initializer is
a separate operation and does not supply this child or a callable profile.

The next step is to consume the primary/Astra worker's owned `00886920` audit
and reassess child ABI, receiver effects and Source closure. No duplicate child
body, descendant, handler, profile, string or caller read is authorized here.

The companion report retains all six instructions, ABI qualifications, exact
store/call order and accepted-parent/current canonical pins. Physical CRLF and
canonical Git LF domains are recorded separately where applicable. Worker
checks are full-byte/PE agreement, pin replay, JSON, exact two-file scope and
`git diff --check`. No Source/CMake/ledger/Ghidra mutation, build, test, probe,
Original execution or new reconstruction/ABI/game credit is claimed.
