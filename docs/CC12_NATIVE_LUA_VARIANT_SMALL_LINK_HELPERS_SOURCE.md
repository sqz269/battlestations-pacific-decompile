# Source candidates for the two small Lua variant link helpers

Two naked Win32 fastcall candidates preserve the complete owned schedules at
`006ED9D0..006ED9EB` and `006ED9F0..006EDA0A`. Each takes one actual node pointer
in ECX and exposes the physical EAX pointer result. There is no dummy EDX,
stack argument, raw `noexcept`, child call, pointed-data store or new guard.
The first body's skipped three-byte LEA and the second body's reachable
six-byte LEA are retained with explicit `_emit` bytes.

This worker supplied Source text and static evidence. CMake registration,
compilation, complete emitted-object/Core review and Source admission belong
to the integrator. This packet claims none of those steps or Original ABI,
execution, startup or gameplay equivalence.

## Scope and accepted context

Packet `cc12_native_lua_variant_small_link_helpers_source`, based on published
`ac9205e2864b6d8604955dcd16a7032b8cc6d313`. The accepted read-only audit is
`bc001fbe7e75ff9f719c923575941f29aff49c54`, an ancestor of that baseline:

- `docs/CC12_MISSION_LUA_VARIANT_SMALL_PAIR_HELPERS_ABI_READINESS.md`
- `reports/cc12_mission_lua_variant_small_pair_helpers_ABI_readiness.json`

Ownership covers the two entries and exactly this document, its matching
report, `include/bsp/native_lua_variant_small_link_helpers.hpp` and
`src/native_lua_variant_small_link_helpers.cpp`. No parent691/99 migration,
production consumer, CMake, ledger, Ghidra mutation, build, test or probe is
part of this packet. No outside Native caller, neighbor, padding, profile,
data, assertion provider, handler or CRT body was opened.

## Native replay and Source mapping

| Entry and Source name | Full Native span | Raw instructions | Native SHA-256 |
| --- | --- | --- | --- |
| `006ED9D0`, `follow_native_lua_variant_links_006ed9d0` | 28 bytes | 11 | `ee04b97e773723b563e7d596fdf33d6845f5ea3c1ed6c9749abf22bcdc7612f7` |
| `006ED9F0`, `follow_native_lua_variant_links_006ed9f0` | 27 bytes | 10 | `dc6c4ac378bb555bc0e54cf1e9e9175b202005683c502c7f171c9086305a050e` |

Fresh `bsp.py ghidra` count, prototype, decompile, listing and byte queries
verified the configured existing `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe` program, x86 little-endian 32-bit language and
`00400000` image base before each CLI batch. The live/snapshot count remains
64,729. This does not assert a stronger server-side absolute GPR path check.
All 55 live bytes match the configured installed PE and accepted audit; the
PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Independent decoding again accounts for every raw instruction.

The report maps all 21 raw operations to Source lines and resolves all four
branch labels to their accepted addresses. Ordinary mnemonic/operand mapping
normalizes whitespace, numeric notation and `short` branch spelling. The two
LEAs use exact byte directives rather than an assembler-selected displacement
width. This establishes static schedule coverage; no object was emitted here.

`006ED9DD..006ED9DF` is `8D 49 00`, identity `LEA ECX,[ECX+disp8(0)]`.
The first body's owned unconditional jump bypasses it; ordinary entry flow
covers 25 bytes / ten instructions. The saved listing still omits these
three bytes. They remain inside the complete 28-byte span without a Ghidra
repair or claim that outside entry into them is impossible.

`006ED9FA..006ED9FF` is `8D 9B 00 00 00 00`, identity
`LEA EBX,[EBX+disp32(0)]`. It executes once when the initial child flag is
zero. The loop backedge bypasses it. This is a physical EBX read/write, but
neither a change to EBX's value nor a memory access through EBX. All 27 bytes
of this body are reachable in the accepted local entry-flow model.

## Entry, ordered reads and results

Both declarations are `void* __fastcall helper(void* actual_node)`. The
entry moves ECX to EAX and replaces incoming EDX with the selected DWORD
link: `+8` for `006ED9D0`, `+0` for `006ED9F0`. It then reads the child's
byte at `+31h`. Any nonzero value returns immediately. A zero value moves
that child to EAX, reads its next link into EDX, and tests the new child's
byte. Zero repeats; nonzero returns. The initial node's own flag is never
tested. Each link load and following byte load remain ordered.

On direct return EAX is the initial node. After a loop it is the last node
whose flag was tested zero. EDX contains the stopping nonzero-flag child;
EAX is its predecessor. No minimum, maximum, valid tree, membership, ordering
or Native node type is established by the descriptive Source names.

ECX retains the original node. EBX's value, ESI, EDI and EBP are preserved.
There are no pushes, pops, stack adjustments or public stack arguments. With
S=entry ESP, plain `RET` consumes the current return address and leaves ESP
at S+4. The final nonzero byte CMP supplies the returning arithmetic flags:
ZF=CF=OF=0, with SF/PF following the byte. No incoming arithmetic flag is used.

The initial node must expose the selected link DWORD: a highest-address span
of 12 bytes for `+8` or four for `+0`. Every tested child exposes byte `+31h`,
a 50-byte highest-address span. A zero-flag child also exposes the next link
DWORD. These are access footprints, not object sizes, owners or blanket
initialization requirements. Pointer/read aliases, including stack aliases,
are permitted by the schedule; the helper makes no data writes. A normal
return still requires a usable current return address.

There is no null, cycle, type, alignment, range, membership or lifetime guard.
Zero-flag cycles can hang and invalid reads can fault. No concurrent-mutation
consistency, exception recovery, rollback, x87 or EH provider is introduced.

## Accepted caller and current Source versions

The pinned accepted parent receipt supplies the caller facts; no Native
parent bytes were reopened. At `006EE6B3`, ECX delivers replacement Y to
`006ED9F0` and the caller stores EAX to its retained header H+0. At
`006EE6D5`, the corresponding `006ED9D0` result is stored to retained H+8.
There are no pushed arguments. EBX=H, ESI=P, EDI=Y and EBP=R survive locally.
The accepted endpoint comparison and replacement-zero-flag tests govern
reaching each call; the helper does not repeat the initial-node flag test.
No parent source or consumer is changed by these candidates.

All 150 repository pins from the small-helper audit replay at this baseline
without drift. Both latest primary Source reports retain identical 61 input
pins, all replayed here with their canonical and physical forms distinguished.
The four recorded Core/executable/map/test-log artifacts also match their
hashes and remained stable during reads. This is artifact hashing, not a new
build, execution or reparse of compiled graphs.

Directly inspected current link-repair Source remains the admitted naked
fastcall receiver/unused-EDX/stack-pivot pair, with three RET4 arms each and
no calls. Its primary receipt reports exact 78+82 emitted bytes / 60 operations,
zero relocations/local EH and two unique Core definitions. The owned-message
Source preserves pointer-slot capture, publication-before-read ordering,
wrapping length increment, nullable allocation, publication-before-branch,
the late same-slot read, ignored `strcpy_s` result, null-path DWORD RMW, final
ownership DWORD and receiver result/RET4. Its receipt reports 80 Source bytes
/ 33 operations for 78 Original bytes with qualified current Source CRT calls.
These actual Source implementations were read, and their four header/body
pins still match their historical candidates.

The latest registered build is the inherited `2026-10-09T13:39:47Z..13:40:03Z`
batch with three existing checks passed, nine whole objects and ten unique
Core roots. The three admitted functions are absent from the application
map, and have no production consumers. Existing Source strlen/UCRT bindings
do not prove a selected new caller operand or Native CRT/allocator/failure/EH
identity. The reported 20 typed-owner code/relocation and eight EH
payload/relocation contracts remain inherited unchanged Source evidence.

Earlier receipt versions remain separate: both candidate documents changed
during registration, though the four Source header/body pins did not. The
older 57-input default-primary receipt replays historically; its CMake pin
is stale. The audit's older 140-input receipt retains its historical
qualification, including CMake and names/reconstruction shard drift. None is
promoted to a whole-current build receipt. Both latest primary documents
replay against their current pins.

## Integrator review and remaining gaps

Before Source admission, compile through the normal MSVC Win32 build and
inspect the complete emitted functions, all 55 expected bytes / 21 raw
operations, short branch displacements, both explicit LEAs, plain RETs,
relocations and local EH. Review actual Core definitions and application
selection in the registered build. Worker static mapping and existing batch
artifact hashes do not provide these checks for the new candidates.

The accepted 691-byte parent, the `006EDA20` assertion child, parent99
integration, Native EH, production storage and usable caller backing remain
open. The candidate Source names are provisional. There is no new Source
admission/function credit, Original ABI credit, runtime or gameplay claim.
