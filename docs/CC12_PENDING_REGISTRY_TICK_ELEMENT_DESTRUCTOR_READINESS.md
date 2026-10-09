# CC12 tick-element destructor wrapper readiness

Baseline: `0bfa628055f152f329bdc62c6b2ae1fdfe678b24`.
Packet: `cc12_registry_tick_element_destructor_readiness`; owned address
`00875490`. This is a read-only readiness audit with no Source or Native credit.

The complete owned wrapper is **32 bytes, `00875490..008754AF`**, containing
nine instructions, zero physical CALLs and one tail JMP to `00874F00`.
The initially requested 30-byte span ends halfway through the final JMP's
four-byte displacement. Live function bounds independently end at `008754AF`.
The integrator approved completing these last two owned operand bytes;
no adjacent function or dependency body was read.

There is **no direct `00875280` getter call** in this wrapper. The indexed edge
to that getter is contradicted by the complete physical bytes and current live
callee query. Its cause was not investigated and no metadata was changed.

## Complete physical evidence

Configured/live-query target: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit, image base `00400000`.
The project-aware `bsp.py ghidra` commands verify the configured target before
queries. Live and snapshot function counts were both 64,729.

The complete live GPR bytes equal the file-backed `.text` bytes in the original
installed executable. Full image SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The wrapper's SHA-256 is
`467c74d1ee7fcb9f8c2c2bb983d52e5245e0d0246a6b3dc1a143e6cc96002ff5`.
Independent Capstone decoding covers every owned byte with no gap or overlap:

| Address | Bytes | Operation |
| --- | --- | --- |
| `00875490` | `8b4104` | `MOV EAX,[ECX+04]` |
| `00875493` | `8b5108` | `MOV EDX,[ECX+08]` |
| `00875496` | `c701c8ded000` | `MOV DWORD [ECX],00D0DEC8` |
| `0087549C` | `895008` | `MOV [EAX+08],EDX` |
| `0087549F` | `8b4108` | `MOV EAX,[ECX+08]` |
| `008754A2` | `8b5104` | `MOV EDX,[ECX+04]` |
| `008754A5` | `895004` | `MOV [EAX+04],EDX` |
| `008754A8` | `83c11c` | `ADD ECX,1C` |
| `008754AB` | `e950faffff` | `JMP 00874F00` |

The tail's next EIP is `008754B0`; signed displacement -1456 gives `00874F00`.
All 32 bytes are file-backed at PE file offset/RVA `00475490`.
No profile/table bytes, Native handler, Native caller body, or dependency body
were inspected. A full-file hash is provenance, not analysis of other bodies.

## Receiver, fields, access schedule and tail boundary

Let R be entry ECX. The wrapper first captures DWORDs `[R+04]` and `[R+08]`
into EAX and EDX, then stamps `[R+00]` with `D0DEC8`. It writes the captured
second value at the first value's `+08`. Only after this store does it reload
the current `[R+08]`, then the current `[R+04]`, and write the latter at the
former value's `+04`. Descriptive previous/next link names fit this reciprocal
unlink pattern but remain hypotheses rather than recovered type information.

The second pair must remain fresh reads: an indirect neighbor store can alias
the receiver or its fields. Replacing these reads with the first pair's cached
values would add an unproved alias assumption. There is no explicit direct
clearing of the receiver's link fields; indirect writes can still alias them.
Both initial link reads precede the profile store. There are no null, identity,
membership, range, alignment, overflow or already-unlinked checks.

At the physical tail transfer, ECX is `(R+1Ch) mod 2^32`, EAX holds the second
pair's current `+08` value, and EDX holds its current `+04` value. No incoming
EAX or EDX value is consumed by this wrapper. EBX, ESI, EDI, EBP and ESP are
untouched; no stack argument or return address is read or written locally.
MOV preserves arithmetic flags; the final ADD sets flags normally. There is
no x87 instruction, conditional branch, explicit allocation/free, section
operation, registry access, local EH frame or hardware-fault recovery here.

The tail adds no return address and retains the original caller's stack.
`00874F00` therefore determines the eventual return, stack cleanup, outgoing
register effects and semantic result. The wrapper alone proves its ECX receiver
and outgoing raw register state, not a complete callable Source prototype.
The callee's use of outgoing EAX/EDX or caller stack cannot be excluded from
its zero-parameter metadata. No `void`, thiscall/fastcall declaration, flags
parameter, hidden callee register contract, or destructor ownership policy is
forced from the current annotations/decompiler signature.

## Metadata discrepancies and dependency gate

The existing descriptive name is `BSP_UnitTickElement_Destruct`. Its saved
comment still states `00875490..008754AD`, a single unit-destructor caller,
and receiver `unit+310h`. Those are inherited hypotheses; Native caller bodies
were not read to validate them. The rebuilt index lists 21 callers and two
callees (`00874F00`, `00875280`). Current live callee metadata lists only
`00874F00`, matching the physical tail. Its reported call count of one must
not be presented as a physical CALL instruction.

Only metadata was queried for `00874F00`: live bounds `00874F00..00874F63`
(100 bytes), zero formal parameters, 42 listed instructions, 14 basic blocks,
complexity 8 and no listed calls. The current lookup shows a descriptive name
but no Source reconstruction. No conclusion about its full algorithm, ABI,
return behavior, helper dependencies or possible transitive registry usage is
admitted here. A bounded audit of this actual tail target is the next required
step before a faithful wrapper Source interface can be chosen.

The getter remains Source context, not an owned direct dependency. Its current
primary report admits the ordinary two-reference interface for actual `F878CC`
and actual `01090AA0`, with complete compiled 250-byte/85-instruction body,
140-byte Source EH table, actual providers and Core resolution. Its recorded
normal build and three existing checks passed; those checks did not execute
the new root. All 30 Source pins selected from that primary report and its
current document pin match the present checkout. The compiled body and EH
payload hashes were replayed from the accepted report, without a new build
or fresh reads of the prior ignored build artifacts.

The getter preserves captured initial/final-current publication behavior,
first-manager section capture, Enter/increment before outer cleanup arming,
inner allocation cleanup ending before publication, second lookup before the
fresh registration argument, and outer cleanup through normal Leave/final
reload. Its explicit references, generated Source catches, current CRT and
provider semantics still do not prove original no-input ABI, FH3/SEH, mutable
Native spill aliases, hardware faults, throw identity, nested failure or game
behavior. Primary admission supersedes the candidate's old unbuilt status;
historical sub-records are not new build evidence for this packet.

## Production readiness and bounded result

Exact searches across current `include/bsp` and `src` C++ files find only the
getter declaration and definition. The only wrapper/tail address references
are the unit-destructor plan's step and pure virtual
`destroy_tick_element_310()` Host slot. The plan invokes that abstract slot;
it supplies no concrete `00875490`/`00874F00` implementation or getter call.
This agrees with the primary report's absence of the getter from its game map.
No Source production caller or demonstrated owner lifecycle is established.

The separate `F899E8` pending-entity lock has its own `009248D0` getter,
`00924180` constructor and `D190C4` profile. Its header is pinned only to retain
this distinction. It cannot substitute for the admitted `F878CC` pending
registry or provide the missing `00874F00` contract.

This audit adds only this document and its JSON report. Complete owned-byte
comparison/decoding, tail arithmetic, access/register review, current Source
searches, accepted getter pins, JSON and whitespace checks passed. No Source,
CMake, ledger or Ghidra mutation; no build, probe, test, new annotation,
production activation or reconstruction credit was added.
