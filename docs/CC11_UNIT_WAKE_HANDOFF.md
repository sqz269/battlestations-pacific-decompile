# Whole raw unit wake handoff, 00815E20

`handoff_native_unit_wake_00815e20` reconstructs the complete
`[00815E20,00815ED6)` body: 182 bytes and 42 instructions. The descriptive name
is a behavior hypothesis, not a recovered source symbol. This is an MSVC Win32
raw entry with ECX = actual new unit, one stack argument = actual old unit,
and RET 4. The unused fastcall EDX parameter preserves that native argument
placement; no semantic return value is claimed.

The worker compiled the complete Source graph from six fresh translation
units and passed one new guarded Original/Source family: **435 checks,
6 cases, 12 calls, 0 failures**. No BSP library was consumed. This is conditional
raw-entry and borrowed-storage evidence. It does not establish original class
construction/lifetime, complete class ABI compatibility, startup or gameplay.
The primary integrator owns CMake registration, metadata and the full build.

## Native storage and order

Both units provide actual valid FAC-byte prefixes. The pose fields are parent
at 3C, local matrix at 74, valid byte at C8, world matrix at CC and derived-valid
byte at 10C. Every ancestor reached by the raw recursive pose body must also
have valid pose storage and a terminating chain. The handoff uses the actual
objects and ancestors directly, with no snapshot, substituted provider or
typed object overlay.

The inline wake ring starts at unit BD0 and occupies 3DC bytes. Unit FA0, FA4
and FA8 are precisely ring 3D0, 3D4 and 3D8 in that same storage. They are not
a separate copied residual record. The samples are at BD8 through F97, the
head at F98 and the separate flag at F9C. The original ring header and flag
are preserved by the copy leaf.

The complete body performs these ordered operations:

1. Capture the raw old FA0 and FA8 bits with MOVSS, before either pose refresh.
2. Refresh new through whole raw `00414DB0` if its current C8 is zero.
3. Read old C8 freshly and refresh old if it is still zero. New's recursion
   can already have refreshed old when old is an actual ancestor.
4. Subtract new world X from old world X and spill to float32. Do the same
   for world Z. Reload those differences, add the captured residuals and
   spill each sum to float32, all before copying the ring.
5. Invoke whole raw `00815680` on the two actual BD0 inline rings.
6. Publish the saved X sum to new FA0; XORPS zeros all XMM0 bits; write
   positive zero to FA4; publish the saved Z sum to FA8.

The intermediate x87 stores are observable. In the cached cancellation case,
`float32(2^24 - (-1)) + (-2^24)` produces zero with round-to-nearest; retaining
the difference in extended precision would produce one. The upward-rounding
case produces two. The reconstruction retains every native load, subtraction,
addition and float32 store in assembly.

The last x87 instruction is the FSTP at entry+A9, and the final data pointer
is actual new+FA8. Incidental native exits are EAX=new+F9C, EDX=old+FA4 and
ECX=0. EBX, ESI, EDI and EBP survive, RET 4 balances the stack, XMM0 ends
with all 128 bits zero, and MXCSR is unchanged. EFLAGS depend on the real
stack arithmetic; the comparison uses the same physical adapter callsite.

## Complete connected dependency graph

| Original entry | Complete bytes | Instructions | Production Source |
| --- | ---: | ---: | --- |
| 00815E20 | 182 | 42 | `native_unit_wake_handoff.cpp` |
| 00414DB0 | 89 | 28 | `native_entity_pose.cpp` |
| 00413920 | 874 | 290 | `native_camera_matrix_math.cpp` |
| 004134F0 | 103 | 35 | `native_camera_matrix_copy.cpp` |
| 00815680 | 456 | 135 | `native_unit_wake_copy.cpp` |

The raw matrix copy is **103 bytes**; the following CC padding is not part
of its body. Raw pose calls itself, the actual multiply and the actual copy.
The handoff calls actual raw pose twice and actual ring copy once. All six
Source call edges bind those canonical complete functions directly.

The Original oracle placed all five complete bodies, totalling 1704 bytes,
at their original relative offsets in one RX code island from 004134F0
through 00815ED5. Its extent is 0x4029E6, or 4,205,030 bytes. **Every Original
CALL operand stayed byte-for-byte unchanged**, including pose recursion.
There is no Original call patch, mock constructor, trampoline, dependency
bridge, source helper substituted for native code or BSP archive fallback.

The handoff Source COFF is exactly 182 bytes/42 instructions, SHA-256
`40ed41e0fd4e0e122c703a1d6798c05a9af22b4f129f3e1a88c080baae481f00`.
The only relocation operands are at 31, 41 and 8B, each a natural direct
CALL operand. All 170 other bytes equal Original, whose SHA-256 is
`e4929517cb6a00c4c7bbf62bb0127159641c5a26194246e75b0b4cfd89266e90`.
Complete COFF and linked-body comparison also passed for all four providers.
Pose has three natural relocations; the other three providers have none.
All five live Ghidra byte extents matched the pinned installed PE before the
run. All five Source and Original body images were saved before and after
the run and remained unchanged.

## Floating-state admission and cases

Cached and root-only paths require one free x87 slot. Any reached parent
matrix multiply requires **all eight slots free**, including when the parent
is already cached. An empty stack may have a nonzero TOP. Production does
not reset the floating environment. The fixture saves and restores ambient
x87/XMM0/MXCSR state around each call, and applies these admission conditions.

| Case | Actual pose graph | CW | Entry TOP/tag | Result X/Y/Z bits |
| --- | --- | --- | --- | --- |
| 0 | Both cached; cancellation | 037F | 6/0FFF, two live canaries | 00000000 / 00000000 / 40800000 |
| 1 | New dirty root, old cached | 007F | 6/0FFF, two live canaries | 7FC00234 / 00000000 / 41D80000 |
| 2 | New cached, old dirty root; upward rounding | 0A7F | 6/0FFF, two live canaries | 40000000 / 00000000 / 40800000 |
| 3 | Two independent dirty roots | 027F | 6/0FFF, two live canaries | 41400000 / 00000000 / 40800000 |
| 4 | New -> actual old -> dirty A -> dirty root B | 037F | 0/FFFF, empty | 40000000 / 00000000 / C0400000 |
| 5 | Both dirty children -> same cached A; B unreachable | 0A7F | 3/FFFF, empty | 41400000 / 00000000 / 41880000 |

Case 4 exercises the fresh old-C8 check: refreshing new recursively makes
old valid before that check. Case 5 performs the reached parent multiplies
with an empty stack and nonzero TOP while leaving the cached ancestor intact.
Cases 1-5 include raw subnormal and signalling-NaN ring inputs. Copying retains
the subnormal bits, quiets the reached signalling NaN and preserves the native
sticky exception effects. All Y outputs are positive zero.

Each side restored the same actual 8960-byte guarded backing in place before
execution. Units begin at offsets 20 and 1020; ancestors begin at 2020 and
2160. Thus both sides used identical owner addresses, actual parent identities
and stack addresses. Every backing byte compared, with no modifications
outside the precise pose/ring/residual allowed-write mask; all guards and
the old inline ring remained intact. Dirty pose flags and actual world X
results were also checked against independent expectations.

Each call saved 168 bytes: ten DWORD register/stack fields, the complete
108-byte x87 state, all 128 XMM0 bits and MXCSR. Original/Source differed only
in the natural code-address DWORD at FPIP. All 80 saved x87 payload bytes
compared, including empty-stack cases. Both live 80-bit canaries survived
cases 0-3. CW, TOP and occupancy were preserved. SW was 7520 -> 7520 for
case 0; 7520 -> 7523 for cases 1-3; 4520 -> 4523 for case 4; and 5D20 ->
5D23 for case 5. EFLAGS was exactly 00000212 on both sides, ESP delta zero,
and MXCSR remained 00001FA0. No unmasked-exception or asynchronous observer
behavior is established by this fixture.

## Reproducible evidence and preservation

The sealed local artifact directory is
`local/cc11_unit_wake_handoff_20261007_a`. Its `recipe.json`, compile/link
logs, complete input pins, COFF/linked/original bodies, assembly, adapter
bodies, runtime storage/state files and manifest are indexed by
`reports/cc11_unit_wake_handoff.json`. The check count includes fixture
preconditions and artifact I/O as well as behavior/state comparisons.

Six fresh Source translation units comprise five production files and the
fixture. The build pins 12 actual project/fixture inputs, 175 actual host
headers, six searched system libraries and three selected toolchain files.
It uses `/MD /O2 /Gy /W4 /WX /fp:strict /showIncludes`; the I386 PE embeds
an `asInvoker` manifest with `/MANIFEST:EMBED`. The initial Source-only
compile and the connected build both passed; the family executed once.
The main core hash recorded in preparation is context only and was never
consumed by this family.

All 318 current prior-evidence paths remained unchanged throughout this
work. Of the earlier audit's 295 historical pins, 293 were still identical
on arrival; the other two were the wake-copy documentation/report that
changed during the authorized sync to published main. Both exact historical
versions are preserved in this new receipt, and both published versions
remained unchanged during this work. This is not a claim that all 295 old
path contents still match their historical hashes. No old fixture was
replayed, no prior receipt was overwritten, and Ghidra remained read-only.

The installed original image is 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
It and its frozen copy, all 12 actual build inputs, 175 headers, six system
libraries and three toolchain files passed post-run hash verification.
Reproduction must use a new artifact directory and preserve this receipt.

Actual unit construction, critical-section/handle lifetime, class destruction,
invalid pointers, cyclic ancestry, concurrent mutation, unmasked floating
faults, SetLeader/type/speed/detach/observer/world integration and live game
behavior remain outside the claim. Caller witnesses 0070D0D7 and 0070E54E
were checked statically; their enclosing bodies were not executed here.

## Primary integration

Whole182B42 raw handoff binds complete raw89 pose with fresh ancestor reload and whole456 ring copy. Original five-body exact-relative island preserves all1704 native code bytes and all6 CALL operands; Source five complete bodies and6 canonical edges independently verified. Preserve oldFA0/FA8 capture before pose, fresh oldC8 gate after new pose, x87 spill/difference/sum before inlineBD0 ring copy, then FA0/FA8 and positivezeroFA4/XMM0. One435-check/6-case/12-call family on actual8960 guarded backing verifies dirty roots/ancestors/shared parent and complete168-byte state except natural FPIP. Parent multiply requires EMPTYtagFFFF (nonzero emptyTOP permitted); other paths one free slot/two live canaries. EFLAGS matches within each common callsite; primary216 versus worker212 comes from distinct physical ESP. Constructor/lifetime/fullclass/faults/concurrency/group routing/world/game remain external.

Main Source `3ef9afe67` passed the full MSVC Win32 build and all three existing CTests. The independent primary fixture consumed 6 freshly compiled TUs; its receipt is `local/cc11_wake_handoff_current_primary/inputs_after.json`. Its actual consumed headers, toolchain, libraries and historical inputs remained stable. Saved annotations, exports and snapshot follow in the report.
