# Native property-record Type4 raw storage, CC12

Source admission is **1 for the bounded current-provider raw storage domain**.
The whole constructor `[008EF230,008EF268)` is 56 bytes/16 instructions, all
literal bytes, with no CALL or relocation. The existing CPP/HPP are unchanged. The provisional raw-storage name and
evidence are saved in the configured Ghidra project; the affected export is refreshed.
This is a partial initializer with a new Source interface, not an owning class
or a binary replacement/gameplay qualification.

ECX supplies a fresh writable unowned 56-byte root; EDX is unused padding.
The two genuine stack DWORDs at `T+4/T+8` remain opaque and are stored at
`+0x28/+0x0C`. `RET 8` returns the actual root in EAX, zero ECX and value EDX.
It writes 41 bytes and preserves 15. Nonvolatiles, ES and DF are preserved;
defined XOR bits satisfy `flags & 8C5 = 44`, excluding undefined AF.
Byte `+0x2C=1` establishes no resource ownership or liveness.

## Independent complete-code validation

Root generated three fresh TUs in `local/t4p3`, using 184 actual headers and
seven libraries. No BSP archive, old object, accepted stage or process was
replayed. The exact all-files seal includes stopped fresh-reader evidence,
all pycache, actual Root helpers/logs and independent recorded decoding:
**457 files**, SHA-256
`bd2cefbe278f318b0b6dbe948a021bd7018093bab3153f35358bd00f2e9c3602`.
The sealed Source0 summary is immutable history before this external Source1
decision. All 24,186 prior immutable files and consumed inputs remained unchanged.

Independent raw COFF/MAP/PE/packed-byte readers derived 31 logical symbols,
29 physical TU bodies, 219 logical relocation checks, 213 physical operands
and six alias rechecks. The two mapped weak aliases actually use
SEARCH_NOLIBRARY1; ALIAS3 support is not evidence that ALIAS3 occurred.
All 47 whole spans cover every retained TU and 18 support bodies, including
full std imports, cookie14, stack43, sized-delete16 to unsized5 to free6,
ordinary-original69 and its EH29. The normal RET is followed by five provably
unreachable CC alignment bytes, all retained inside its full69-byte gate.
The original stopped reader and the separate corrected reader are preserved.
Cold code coverage adds no cold-EH or GS-failure execution; the cookie's named
GS-failure destination remains unexpanded.

Root reviewed every instruction of Main2934/791, including target selection,
full comparisons, code/provider gates, failure paths, snapshots and cleanup.
Its raw CFG proves 17 ordering/dominance pairs and 47 guard failure edges that
cannot reach later accepted targets, frees or success output.

The sole fresh process entered Source with DF1 and unchanged RX Original with
DF0. Two actual current-canonical56 roots received complementary poison6D/92,
identitiesE79B42D1/3AC5682F and opaque values7FC35A91/80000000. Two frees
disposed their actual allocation bases. Both full Capture124 records
(Registers92, post guard at108) and both full56 arrays were independently
decoded. Stack/GPR/ES/DF, external Capture guards, other live root/Capture
preservation and both dead argument words passed. `R=EBP-20` is captured;
`T=R-12` is derived. There are no invented inside-allocation root guards or
separately serialized target-entry ESP. The raw wrapper restores DF0 before CRT.
Ordinary wrappers are retained code only, with zero ordinary dynamic entries.

Three UCRT bindings are recorded for malloc/free/_callnewh. Before target entry,
the probe verifies MEM_IMAGE/AllocationBase, mapped-vs-handle NT paths,
FileID/size, full SHA, I386 PE, export/IAT equality and adjusted32-byte prefixes.
After both roots/RX are freed it rechecks held-handle FileID/size, unchanged
IAT, full SHA and live prefix. It does not repeat VirtualQuery, NT path or
GetProcAddress then. The independent saved decoder reads frozen DLL bytes and
recorded generation identities only; it creates no new live-provider attestation.

## Build and connection boundary

The Main Win32 build at `cdee01a3a6e6d2974a90ae7a36ffe64df7d4a1f2` passed all
three existing checks with 4,037 actual Source/header/CMake input hashes
unchanged. This refreshed build includes the later signed-count lifecycle fix; its inputs remained unchanged during the build.
No CPP/HPP changes or new tests were needed for this requalification.

The [recursive lifetime audit](CC12_TYPE4_RECURSIVE_LIFETIME_DOMAIN_AUDIT.md)
establishes that tag4 follows the release default, clearing scalar fields
without dereferencing/freeing either stored word. Scalar flags govern only
the actual root's disposition within the existing root/context contract.
Bag publication, a complete raw clone binding, declaration/enum semantics,
phase/vtable/class ownership, private Original CRT/EH, drop-in ABI, game
startup and gameplay remain unadmitted.
