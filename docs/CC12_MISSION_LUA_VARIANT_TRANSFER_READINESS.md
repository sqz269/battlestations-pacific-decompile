# Raw mission Lua variant transfer wrapper: 00886DA0

The complete `00886DA0..00886DF3` body is 84 bytes / 24 instructions. It stamps
four destination DWORDs, leaves destination bytes `+8..+B` untouched locally,
then calls `00886C10` with that receiver and a source word loaded from its stack
argument. After the child returns, it restores its exception frame and returns
the destination in EAX with `RET 4`.

**Source closure remains held at the child.** The child is 369 bytes by metadata,
above this workflow's 300-byte gate, and has no current address-bound Source
implementation. The wrapper passes incoming EDX through to that child; whether
the child consumes it is unknown. This is not evidence of a proven hidden ABI,
but it prevents claiming transitive EDX independence. Both limits were routed
to the primary for an Astra/oversized-packet decision. No child body was read.

This packet changes only this document and its JSON report. No C++, CMake,
ledger, Ghidra, test, probe, build, Original execution or new credit is included.

## Scope and complete-body proof

- Published baseline: `4cb3a4c626523998c1f08d18b82dc2ad1f170af8`.
- Owner/lease: `agent/cc12_settings_live_fov_cell` /
  `cc12_lua_variant_transfer_readiness`.
- Owned body metadata: 84 bytes, 24 instructions, one block, no edges, one
  direct call. The 300-byte gate passed before reading the body.
- Each live CLI batch verifies the existing `C:/Users/sqz269/bsp.gpr`, project
  `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default` and
  image base `00400000`. Live/snapshot function count remains 64,729.
- All 84 live bytes equal the physical original PE and independently decode to
  the same 24 instructions. There are no omitted listing bytes or branches.
  The final `RET 4` starts at `00886DF1` and ends at `00886DF3`.
- Whole-span SHA-256:
  `6d1fb3bdf2163611f8c2ac61626732b1fdccdc242dfedec2e6bf978fb5b66e88`.
- No child, handler, vtable, string or caller body was opened. No function repair,
  no-return flag change, annotation, export or project mutation was performed.

## Exact receiver and source schedule

Let D be entry ECX and S be entry ESP. The public source word is at `[S+4]`.
The table follows the actual instruction order; the saved receiver is a local,
not an inferred C++ object or owning resource.

| Site | Operation |
| --- | --- |
| `00886DA0..DAE` | Install local FS record with initial state -1, handler `00C9742B`, and old FS head. |
| `00886DB5..DB7` | Push entry ECX as a local, save incoming ESI, retain D in ESI. |
| `00886DB9` | Zero EAX. |
| `00886DBB` | Store DWORD `00D0E6F4h` at `D+0`. |
| `00886DC1` | Store DWORD `FFFFFFFFh` at `D+4`. |
| `00886DC8` | Rewrite the receiver local with D. |
| `00886DCC` | Store zero DWORD at `D+Ch`. |
| `00886DCF` | Store zero DWORD at `D+10h`. |
| `00886DD2` | Set local unwind state to zero. |
| `00886DD6` | Load the **current** source argument word at `[S+4]` into EAX. |
| `00886DDA..DDB` | Push that word and call `00886C10`; ECX still equals D. |
| `00886DE0..DF3` | Load saved FS head, place D in EAX, restore ESI/FS/stack and execute `RET 4`. |

The four receiver-relative stores cover `D+0..D+7` and `D+Ch..D+13h`: 16 written
bytes inside a minimum 20-byte address extent. No receiver-relative instruction
reads or writes bytes `D+8..D+Bh`. This hole does not exclude effects from the
child or an alias with the separately accessed stack/FS record. No other
receiver-relative memory access is encoded here.
The prior words are overwritten without local cleanup. No complete class,
allocation size, field types, reference counts or ownership flags are recovered
from these four stores.

The wrapper never dereferences the source word. Its pointee offsets and full
read/write footprint therefore remain a child question. There is no null,
identity, overlap, alignment or bounds check. An ordinary unmapped null D faults
at the first store, with the local exception record installed and state still
-1. A zero source word is forwarded without a local check; its consequences
depend on the unopened child.

The source argument is fetched after all four destination stores. If D overlaps
the argument slot, those stores can change the word actually forwarded. If
source and destination denote the same storage, the child receives storage
already stamped by the wrapper. For any overlap, the exact ordered stores occur
before the child can read source contents. A cached incoming argument or a
preloaded source value must not silently replace this schedule. No self-copy,
overlap-safe transfer or malformed-stack behavior is established.

## Original register and stack contract

At the child boundary `00886DDB`:

- ECX is D; ESI retains D.
- EAX equals the late-loaded source word, which is also the single pushed DWORD.
- EDX still equals entry EDX: no owned instruction reads or writes it.
- EBX, EDI and EBP are untouched by owned instructions.
- XOR EAX,EAX set CF/OF/SF to zero, ZF/PF to one and AF undefined. Subsequent
  MOV/PUSH instructions preserve those flags through child entry. No incoming
  flags are consumed by the wrapper.

Incoming EAX is overwritten by the old FS head at `00886DA7`, then cleared, and
finally replaced by the source word. The caller's incoming EAX value has no
owned use. In the accepted reserve caller, EAX held the byte offset and EDX held
the source pointer; the offset is discarded here, while EDX passes through.
That caller context comes from the accepted report, without reopening callers.
No assertion about the child's EDX or other hidden inputs follows from its
generic Ghidra prototype.

At entry S points to the return address. The wrapper's frame is:

| Address | Role |
| --- | --- |
| `S-4` | Unwind state: -1 during four destination stores, zero before child call. |
| `S-8` | Handler address `00C9742B`. |
| `S-12` | Old FS head; `FS:[0]` is set to this record address. |
| `S-16` | Receiver local, pushed at DB5 and rewritten at DC8. |
| `S-20` | Saved incoming ESI. |
| `S-24` | Pushed child source argument. |

There is no caller cleanup after the child. For the observed epilogue to balance,
the child must return compatibly with four-byte callee cleanup, leaving ESP at
`S-20`. This is a call-site requirement, not a full child-ABI proof. The wrapper
then loads old FS from `[ESP+8]`, copies ESI to EAX, pops ESI, restores FS, adds
16 to ESP and executes `RET 4`, consuming its own public source argument.

Normal return is full EAX=D regardless of the child's EAX result. ECX becomes
the old FS head. ESI is saved/restored locally and must remain D through the
child; EBX/EDI/EBP require the child's ordinary nonvolatile preservation. Final
EDX is whatever the child leaves, since the wrapper does not restore it. No
x87 or SSE operation and no owned body/listing repair issue was found.

## Profile, exception and lifetime limits

`00D0E6F4h` is an exact numerical profile identity. The wrapper does not read
the table or a target from it. Its stamp supplies neither a callable Source
profile nor proof of its entries. `FFFFFFFFh` remains a raw DWORD bit pattern;
no semantic tag or deletion policy is inferred.

The local state becomes zero only after the four stores complete. It stays zero
through normal child return and is discarded when the frame is removed; the
body does not explicitly reset it to -1. Handler `00C9742B` is unopened. A fault
during initialization can leave a written prefix with state -1; a child failure
occurs after the four stamps with state zero. Handler cleanup, nested unwinding,
retained resources and exception ABI compatibility remain unproved. Absence of
handler evidence is not a leak or no-cleanup conclusion.

The caller must provide live writable destination backing, a valid source
argument word and valid stack/exception-chain lifetime. The child controls the
additional pointee and ownership requirements. No local allocation/free or
source-content transfer occurs outside the child call. No RAII, byte-copy,
placement type, callback dispatcher or default owner is introduced.

## Current Source and next routing

Bounded exact-address searches across `include/src` and the reconstruction shard
find no current Source for either `00886DA0` or `00886C10`. The existing admitted
`initialize_native_mission_lua_variant_008849b0` is a different complete leaf:
it also writes zero at `+8`, which this wrapper's receiver stores do not write.
It cannot substitute for this four-store prefix. It provides no transfer child
and no callable Source profile. Its actual header/Source and published primary
admission remain pinned; no worker build or Core review was repeated.

Direct-child metadata only: `00886C10..00886D80`, 369 bytes / 114 instructions,
16 blocks, 18 edges, seven direct calls, generic `undefined(void)` prototype.
Metadata names six distinct callees, including the named native string copy
constructor and allocator; none of their bodies, strings or tables was read.
Names alone do not establish a transfer implementation or ownership semantics.

The next boundary is **primary/Astra review of the 369-byte child**, requiring
an explicit size/scope decision and fresh ownership before any body read.
Classify its use of incoming EDX and the ECX/EAX/stack source deliveries rather
than assuming a conventional prototype. A partial child prefix would not prove
the complete transfer. The local handler and actual profile targets remain
separate unowned dependencies. No deeper Native expansion is authorized here.

The JSON retains all 24 instructions, precise local stores and untouched hole,
late source capture, ABI/EH limits, accepted reserve context and canonical pin
replay. Current Source matches canonical bytes exactly. Historical worker JSON
files with physical CRLF versus canonical LF retain both hashes explicitly.
Worker checks cover full-span/PE agreement, pin replay, staged JSON, exact
two-file scope and `git diff --check`; no new runtime or reconstruction credit.
