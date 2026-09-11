# Native camera cache getters

This packet reconstructs all branches of B6FCB0 (58 bytes), B6FCF0 (112),
B70490 (116), B70510 (61) and B70710 (69): 416 original bytes. Names describe
observed behavior and remain hypotheses. Existing semantic camera APIs remain
separate.

All originals take ECX camera, return the cache address in EAX and use plain
RET. The first four source fastcall entries keep those locations. Frustum adds
EDX pointing to the existing fixed `NativeCameraFrustumContext`; one saved
context word lies outside the original local frame. The hit path does not
dereference this context. The full existing CRT runtime must already bind its
actual math-error word and real errno accessor as documented by that provider.

These functions consume raw native bytes. Parent+30 is an actual 32-bit node
address. Every ancestor reached by full B6DB70 must support flags+5C,
local+B0 and world+F0; the provider captures each parent once and recursively
refreshes it when its world-valid bit is clear. The canonical hierarchy
integration now supplies actual node-address words through `NativeNodeBinding`,
including the prefix used by `NativeCameraOwner`. Pass actual initialized raw
storage, never the C++ companion address. Full camera owner constructor and
destructor replay remain separate; see `NATIVE_NODE_RAW_HIERARCHY.md`.

| Getter | Valid bit | Returned cache | Dirty work |
| --- | --- | --- | --- |
| B6FCB0 view | flags+5C bit8 | +60 | Refresh world if bit2 clear, scaled affine inverse to local temporary, ordered matrix copy |
| B6FCF0 projection | flags+2F0 bit8 | +2A0 | Native scalar argument preparation, full projection/tangent, copy to+1E0 then+2A0 |
| B70490 VP | flags+2F0 bit10 | +220 | Inlined full view path, full projection getter, full multiply and copy |
| B70510 inverse VP | flags+2F0 bit20 | +260 | Full VP getter, full general inverse and copy |
| B70710 frustum | flags+2F0 bit4 | +2F4 | Publish bit4 first, full VP getter/extraction, assign six planes with flags7 |

Projection preserves the original ordered FLD/FSTP loads from far+1D8,
near+1D4, aspect+1C8 and fov+1C4 into local scalar argument copies. The completed
projection provider may overwrite its fov/far slots; those slots are never
camera fields. EDX points to these actual temporary slots and the getter
removes them after the new builder's plain RET, replacing native RET10h
cleanup. The original 64-byte result temporary remains separate, followed by
both complete sixteen-pair x87 copies.

View, projection, VP and inverse VP OR their validity bit into the current
DWORD only after the cache copy finishes. Frustum instead loads the current
flags DWORD, sets bit4 and stores it before calling VP. It never rolls that
store back on a downstream failure. Frustum assignment writes six 20-byte
records through camera+36B, while records6..15 and count+434 remain untouched.
No owner lifetime, hierarchy mutation, cache invalidation policy or floating
control changes are introduced.

The strict MSVC Win32 worker build, both existing CTests and all eight native
seed checks passed. Nineteen fresh guarded Ghidra/installed-PE spans pin 5,308
bytes: the five getters, their complete original world/math/frustum body
closure and three readonly constants. The focused fixture runs those original
bodies against the full frozen worker library. Its sole external original
bridge supplies the existing fixed context to the complete library vector
length/CRT provider; it substitutes no arithmetic or exception result.

Four paired scenarios cover the cold recursive parent chain followed by
frustum/inverse VP and hits, every hit with invalid unused parent/context,
mixed world/projection-valid flags, and an actual invalid-parent access fault.
Sixteen paired calls compare 133,072 literal observable bytes. The fault case
retains flags+2F0 bit4 and reports the same read at address 0x5D in both paths.
Full raw hierarchy/camera arenas preserve the owner scalar fields, unrelated
flag bits, unassigned planes and count. Successful returns additionally compare
result pointers, nonvolatile registers and selected floating state.

The proof checks all mapped COFF sections after stripping every map `f`/`i`
flag: 145 sections total, including 77 library sections, all relocations and
eleven exact archive members. Every getter's complete instruction schedule
matches after only declared provider-call and ABI mappings. Nine postimages
pin all original bodies/constants and loaded source text/rdata. Fifty-six
loaded import targets match their runtime module files and loader relocations.

The ignored seal is
`J:/PROG/battlestations-pacific-decompile-native-camera-cache-getters/local/camera_cache_getters_fixture/sealed.json`
(SHA256 `4e34e203d4a50ae9fb796c48873f86d531fbd37f7a98382bdd01aaa95f334708`).
It pins 100 artifacts, including the full library with SHA256
`b19cdf5e8ed76e3e079c6dce68210b8e5028e33334e6f96da4fe34cc63b04098`,
the exact objects and 36 provider source/header files.

Floating exceptions are masked. Fault-case floating/register context parity,
unmasked hardware FP and asynchronous mutation are not claimed. Private stack
scratch, return addresses, instruction/data pointers, inactive x87 state and
XMM4..7 are outside the runtime comparison; raw frames remain available.
No permanent test suite, original caller integration or gameplay validation
is claimed. The primary subsequently completed main registration, integration build,
replay and saved-analysis metadata, as recorded below.


## Primary main-library integration

All five getters are registered in main. Strict MSVC Win32 compilation and
both existing CTests passed; all eight fresh native seeds matched. The primary
verified 100 worker artifact pins and 36 current provider files: 33 literal
matches and three with CRLF/LF differences only. Nineteen fresh guarded spans
matched all 5,308 bytes.

The unchanged probe linked actual main library
`90f6e12be9a855a4abcd0f5f7c04ba8518868fee030f3660bc25a25c10a9a265`.
Four paired scenarios and sixteen paired calls again matched 133,072 literal
observable bytes. All eleven exact archive objects, 145 complete COFF sections
(77 library), fifty-six actual import providers and nine immutable code and
constant postimages passed. All five complete getter instruction schedules
matched their originals after only the declared provider and ABI mappings.

The read-only primary bundle is `local/camera_cache_getters_primary/`, seal
`05b29c9a71ba20c254249e318e70a3d830f3313eef3339706cf0b0aed5a2150c`.
Existing Ghidra names/comments were preserved, reviewed evidence appended and
saved, affected exports refreshed and complete source records registered.
The raw-parent contract and all runtime/ABI limitations above still apply.
