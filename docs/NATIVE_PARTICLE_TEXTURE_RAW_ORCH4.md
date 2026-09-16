# Raw particle texture loading

Addresses: 00b01350

## Complete source composition

`load_native_particle_type_texture_00b01350` in
`native_particle_type_texture_raw.hpp` reconstructs the complete B01350..B015BF
body, 624 bytes. Original ABI: ECX actual particle definition, one stacked
filename, RET4/AL. The existing host interface remains available.

The raw context borrows the application's actual string pool, atlas publication,
D3DX9 float-to-half import and numeric cell. On an initial atlas miss it also
requires the existing `NativeTextureCacheContext` and canonical
`NativeRenderActualOwners`. These optional pointers are untouched on atlas-only
paths; they cannot supply a successful fallback when the cache is required.
All string contexts must share the same actual pool publications. Cache textures
and the release service must share the same canonical owner domain.

Null and empty filenames return false without touching the definition. Otherwise
the actual +68h record descriptor is cleared. An atlas hit feeds item +14h/+1Ch
through the existing B00880 x87 producer and appends its 1Ch record. A miss:

1. Constructs the actual pooled filename header, then captures the current
   renderer/profile/+64 slot from the cache's live F8D394 publication.
2. Dispatches supported D5F0A8/+64=B319B0 to the existing complete cache wrapper
   with the same renderer, actual header, zero word and retained cache child.
3. Returns the captured filename buffer before producing default UV values.
4. Reads D7A24C once, stores `(0,0,one,one)` without x87 conversion, calls the
   actual D3DX import, and appends the record.
5. Uses `release_native_render_actual_owner` once for the entire actual +4
   decrement/current-zero-terminal sequence. No extra cache retain is invented.

Frame counting and names use the new raw texture-name helpers. The frame count
is captured once. Each later atlas search reloads the actual manager publication.
Later records retain four MOVSS bit copies, unlike the first atlas record's
FLD32/FSTP32 sequence. Missing later frames return their name and stop the loop.
The signed clamps then publish +54h followed by +50h. Each record's unwritten
+18h stack DWORD is an explicit input; it is neither guessed nor initialized to
an assumed default.

## Exceptions and retained operations

`NativeParticleTypeTextureRawAcquired` retains the original local header/record
region and one existing `NativeTextureCacheAcquired`. Its phase metadata adds
no native ownership. It cannot be replayed. Unresolved failed cache children
must remain alive under their existing contracts; their destructors reject an
unresolved running/failed operation. There is no automatic rollback or retry.

FH3 handler CBB490 selects FuncInfo DF34F0 and map DF34E0: state 0 cleans the
initial filename through CBB480, state 1 cleans the later frame name through
CBB488; both transition directly to -1. The source arms only after the matching
constructor returns (state 0 additionally follows renderer target capture).
Normal returns disarm before the pool getter. Unwind cleanup is nonthrowing;
a secondary failure terminates. Textures and published records have no invented
exception cleanup. The native texture decrement is consumed before canonical
lookup, including when that lookup fails.

## Validation and limits

The complete 624-byte body, two eight-byte cleanup funclets and 52 bytes of FH3
metadata match the installed executable and live Ghidra program. Prior full
annotations, all call sites and artifact hashes are retained in
`reports/native_particle_texture_raw_orch4.json`. The three-byte jump-over
alignment instruction at B0148D is included in full-byte evidence.

The strict Win32 build and three existing CTests passed. An ignored manifested
probe linked to the final library compares the complete original body and raw
source in six cases using actual pooled headers, atlas items and SysWOW64
D3DX9_40: null/empty input, single/multiple frames, x87-first versus SSE-later
signaling-NaN bits, explicit stack residues and signed range clamps. It compares
the complete definition, initialized records, floating status and pool counters.
Direct callees are bound to their existing genuine source providers; this is a
comparison of B01350's composition, not a new differential claim for every callee.

The cache-miss branch was reviewed against the concrete provider audit and built,
but was not executed by this atlas probe. Native FH3/SEH identity, unsupported
renderer profiles, arbitrary stack aliases, malformed storage, OOM, concurrent
mutation, binary replacement and gameplay remain unvalidated.

Follow-up: compose the common B015C0 property loader and nested particle parsers.
Object resource loading additionally needs the animation-item dependency and
resource-container dispatch described in `NATIVE_PARTICLE_MODEL_RAW_ORCH4.md`.
