# CC12 subject/parent unlink readiness

**Unready: Source 0, ready Source packets 0.** The complete ordinary leaf is
known, but its actual subject/parent storage phase and owning embedded-root
provenance are not independently admitted. This audit adds no Source,
headers, fixtures, builds, tests, shared metadata or Ghidra mutations.

Baseline: `c006e9fc052d9e7fca5ff40b7c4e842509f06d1b`, in the new named
`cc12_subject_parent_unlink_readiness` worktree. Root has separately accepted
erase79 in the current canonical allocation/free domain. Completed removal46
and all earlier successful families remain unchanged and unexecuted here.

## Whole physical body

`[00928570,00928594)` is **36 bytes / 16 instructions / one CALL**, SHA256
`5e5953808bcedd0d474f116b3a761958693034335fa2d06906821f25d06a4ab2`:

```text
8bd18b4a308b412883c12485c074149039500874088b400485c075f4c350e83db2b5ffc3
```

Live Ghidra, the pinned installed PE and the immutable earlier removal audit
match before/after. Each live batch verified `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe` and installed PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The live name remains `FUN_00928570`; this audit makes no symbol mutation.

Entry ECX is the actual subject. MOV EDX,ECX retains its identity; incoming
EDX is overwritten. The body reads subject+30 as parent, then parent+28 as
head, **before** ADD ECX,24 establishes the actual embedded root. It tests
head, compares node+8 payload with subject identity, and follows node+4 next
until the first match or null. There is no subject or parent null guard.
Null subject/parent is outside the ordinary contract; null head is the
defined empty path. Duplicate payloads stop at the first match.

PUSH found EAX at 0092858D precedes the sole CALL at 0092858E to 004837D0.
Its operand is `[31,35)`; the other 32 bytes are invariant. The call supplies
ECX=parent+24 and one actual node DWORD on the stack. EDX holds the subject
at call entry and is ignored by accepted erase's padding formal. Accepted
erase's physical RET4 balances the pushed node. This wrapper has **zero
stack arguments and plain RET**, at 0092858C and 00928593. It does not call
removal46 and does not overwrite or clear subject+30.

Empty/absent EAX is zero, ECX is parent+24, EDX is subject and nonvolatiles
are untouched. Final TEST zero defines CF0/PF1/ZF1/SF0/OF0 (`mask 0x8C5 =
0x44`); AF is undefined and excluded. On match, EAX inherits erase's
successor result, rather than forcing subject identity; ECX/EDX are volatile
across real current free. Nonvolatiles survive the ordinary accepted erase/
free ABI. Matched arithmetic flags inherit erase's final ADD ESP,4.
With S equal to this wrapper's entry ESP: PUSH node S-4; erase entry S-8;
erase saved ESI S-12; free argument/return S-16; final ADD result S-12;
erase RET4 restores S; wrapper plain RET yields S+4. The ADD mask `0x8D5`
includes defined AF. This is a static ABI derivation, not new execution.
Real current CRT requires ordinary DF0; no blanket FP preservation is claimed.

## One genuine caller and the ownership blocker

The sole selected caller is 00487230, named provisionally
`BSP_Path_UnregisterFromWorldLists`. Its exact entry `[00487230,00487238)` is
8 bytes / 3 instructions, `568bf1e838134a00`, SHA256
`dd15b5ab576561ac4bccd452cc6a9bf302575d49876569186449ef449807a25a`:
PUSH ESI; MOV ESI,ECX; CALL 00928570 at 00487233, without a payload push.
The next live instruction, 00487238, reloads ECX from [ESI+30]. The caller
preserves its actual subject in ESI and expects that relationship to remain
readable. Only its entry/CALL boundary is independently byte-qualified;
bounded pseudocode/listing identifies a remaining parent+36C scan, without
admitting the whole caller, its receiver construction or its class lifetime.
The other 14 caller references are unexpanded metadata hints.

Historical `UNIT_INSTANCE_LAYOUT.md` distinguishes world-node +30 from
hierarchy-parent +3C, and names a +30 store at 00925906 inside 009258F0.
It does not establish that initializer for this actual Path receiver.
`CONSTRUCT_WORLD.md` describes 97 registration triples beginning at +18,
so this parent+24 triple is category 1, physically embedded in its owner.
The native registrar 00928560 reaches append; placement 00928860, subject
writer 009258F0, base initializer 00925CE0, parent constructor 004CB030 and
post-construction 009037F0 remain named, unexpanded ownership/phase frontiers.

Current `construct_world_object_004cb030` returns a new typed
`WorldObjectLayout`; `run_world_construct` delegates actual creation and
post-construction to a host. The semantic layout and its opaque sentinel
values do not initialize a genuine native parent block or establish physical
embedded-root ownership. Unit constants and host/state interfaces likewise
do not admit construction/publication/teardown of this actual subject.
Accepted append/erase establish their own coherent current-canonical raw
list domain; they do not create or prove subject+30, the parent's physical
storage phase, stable lifetime, or placement/observer sequencing.

The blocker is concrete: **no admitted actual subject construction/phase
establishes and preserves its +30 relationship to a live owning parent with
the physical embedded root +24**. A fabricated parent buffer and manually
assigned subject+30 would supply the missing class/root phase without evidence.
No Source packet is proposed.

Next prerequisites are a genuine bounded subject/parent initialization or
publication contract with actual receiver type/extent; physical parent+24
root initialization and owner lifetime; accepted append-produced owned
nodes carrying that same live subject identity in the admitted embedded
root; and the relevant observer/detach/teardown sequencing. Original private
allocator/CRT, whole parent/world closure, EH/failure paths and gameplay stay
separate. A fresh connected family may follow only after independent
ownership readiness and Root registration.

## Bounded evidence and immutable seal

The actual graph is 11 nodes + 10 edges = **21/24**, including the named
incomplete constructors, placement/registration, ownership and native free
frontiers. Only the target and one selected caller were live-read. Leased
erase, removal, float storage and scalar-registration bodies were not queried.
All Source/build/native/old successful phase execution counts remain zero.

There are 9,022 unchanged prior actual artifact pins, 31 consumed input
pins and 17 frozen historical files. Metadata copies with manifest/receipt
basenames are included by exact paths. Old original-path Source and external
metadata associations remain historical and unconsumed, without repinning.
The prepared verification regex was diagnosed before live use; a separately
pinned literal-path helper preserves the original prepare. A later bookend
parser incorrectly expected JSON from the hex-text byte CLI; the successful
saved reads were recovered post-only, without query or phase replay.

The full ignored `local/up36` inventory contains **126 artifacts + two seals**:

| Artifact | SHA256 |
|---|---|
| `proposal.json` | `9d3f5607db6c14ce2cdd9f7a446cd2eff8b7030e40488a6964626bf7f51a371c` |
| `readiness_receipt.json` | `6064dcb28616c2a5afb2de7b42476a57583595ed3d8cae65759c4829b061073a` |
| `artifact_manifest.json` | `cf706c34e26144ecafcad924b2add23e673ad5b254a7343d8e0b14564c8e1379` |

The manifest excludes only its exact root path and the exact receipt path.
`audit.py`, `live_audit.py`, `post_only.py`, `make_proposal.py` and `seal.py`
are preserved in the inventory. The Source count and readiness result are
zero throughout; native static evidence is not class or game admission.
