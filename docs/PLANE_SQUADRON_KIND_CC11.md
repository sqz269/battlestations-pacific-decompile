# Actual squadron class-word predicate

`native_plane_squadron_is_kind_007efb00` reconstructs the complete call-free
`007EFB00..007EFB2C` entry: 44 bytes, 15 instructions. The entire Win32 Source
COFF body is byte-identical to the Original PE and live analysis, with zero
relocations. This closes the raw predicate; the enclosing class, constructor,
initializer, virtual routing, world and gameplay are not bound.

| Native range | Coverage | Original interface | Source interface |
| --- | --- | --- | --- |
| `007EFB00..007EFB2C` | Complete, end exclusive | ECX actual squadron, stacked DWORD, EAX exactly 0/1, RET4 | Naked Win32 fastcall; conventional unused EDX parameter |

The entry first reads the stacked query. It accepts raw DWORDs 24, 2, 1 and 0
in that order without reading the receiver. Other queries perform exactly one
fresh comparison against the genuine DWORD at the same receiver's `+C4`, then
return full EAX 0 or 1. There is no eager field snapshot, class census, default,
callback, allocation, copied class cache or profile wrapper. The existing
`unit_kind_body_answers` value projection cannot supply this late field read.

The ordinary caller supplies a stable nonnull actual squadron, a genuinely live
aligned DWORD at `+C4`, and readable backing through `+C8`. The fixed-word
branches' lack of a field access establishes no constructor or ownership
contract. Null/invalid receivers, faults, concurrent mutation, structural
reentry and lifetime violations are outside admission; no runtime guard was
added. The raw entry reproduces its register/stack/result protocol, but does not
make an executable image profile callable or establish whole-class ABI binding.

The original body SHA256 is
`66fd7bfbfc5d413545f34c8605f0112e8a1d9d38ca7eef2347fbff09382843a0`.
Its full bytes are:

```text
8b44240483f818741b83f802741683f801741185c0740d3b81c4000000740533c0c20400b801000000c20400
```

`D087C0+5C`, at `D0881C`, contains the actual target `007EFB00`. The constructor
stores raw profile `D087C0` at `007F2CAD` and literal `18h` into root `+C4` at
`007F2DEC`. These are DATA/producer witnesses, not a Source default or recovered
constructor. All three spans and the complete predicate matched PE/live bytes.
The generic `00925F20` caller loads virtual `+5C` at `00926283` and actually calls
it at `00926287`, with EBP carrying 2. `009262AE` is a separate `0077F090` call.
Conditional target attribution requires the actual D087C0 receiver domain;
neither the generic world caller nor other class profiles are closed here.

The whole `007F4BA0..007F5450` initializer remains separate: 2224 bytes,
606 instructions, 61 direct sites/37 targets and 13 indirect sites. Its complete
base-init, Lua/property/registry/global, actual embedded plane `+170` controller,
observer, station/air-ops, allocation, pose/matrix and numeric services remain
required. The existing raw reindex and real observer providers close individual
dependencies only. No InitAll adopter, reduced initializer or opaque phase seam
was added. The previous read-only audit remains sealed and unchanged.

One new ignored connected family passed **61 checks, zero failures**. Four fresh
TUs compile the new predicate, existing sorted insertion, existing leader views,
and the probe. For each fresh Source/Original setup, the existing exact sorted
producer executes once on guarded Source-shaped actual squadron/plane backing.
It publishes real count/member/link/index cells. Both predicates then consume
that same root's live `+C4`: four fixed queries, dynamic match/miss, and a changed
high-bit class word. The fixture checks full EAX, callee DWORD consumption, all
backing/guards, and unchanged reader storage. It compares the whole linked
Source entry with the unchanged, unrelocated 44-byte Original RX copy. No
original class table or native constructor/initializer is executed.

The strict recipe is
`local/cc11_plane_squadron_kind/build.cmd`; it uses MSVC Hostx86/x86,
`/W4 /WX /fp:strict /O2 /MD`, four fresh objects and `/MANIFEST:EMBED`.
The safe executable name is `probe.exe`. The seal records 259 unchanged pre/post
inputs, 246 compiler-observed include paths (28 project headers), three actual
tools and six searched CRT/system libraries. No BSP core/Lua/zlib support
library is consumed. Original/profile/caller bytes, full COFF, manifest resource,
input hashes and logs are pinned in the ignored handoff receipts.

The recipe first exposed a LIB-environment parser error before object building
or execution; parsing the exact `LIB=` line corrected it. The final strict
compile and sole connected execution passed. Existing sealed families were not
replayed. No tracked test, CMake, ledger, Ghidra mutation or full build was made.
Root owns integration, full Win32 build and independent validation.

## Primary integration

Whole44-byte/15-instruction raw Source equals unchanged/unrelocated Original, zero calls/globals/relocations. Full EAX0/1/RET4; fixed24/2/1/0 before field access, otherwise one fresh actual+C4 comparison. One61-check connected family consumes actual sorted/count/member producer once per fresh setup and changed same live class cell/full guards. D087C0+5C and actual ctor stamps/C4 are DATA/structural only; no InitAll adapter, generic virtual/profile/class/constructor/lifetime/world/game binding.

Main Source `b92b4710f` passed the full MSVC Win32 build and all three existing CTests. The independent primary receipt uses 4 fresh TUs and is `local/cc11_squadron_kind_current_primary/inputs_after.json`. Its source inputs remain unchanged through integration. Saved annotations/exports/snapshot evidence follows in the report.
