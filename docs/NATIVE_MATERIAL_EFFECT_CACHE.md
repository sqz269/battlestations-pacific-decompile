# Actual material-effect registry

`native_material_effect_cache.hpp/.cpp` reconstruct the raw `D5F074` registry
embedded at renderer `+1A98`. Its vector at `+4/+8/+C` contains the existing
producer-verified `NativeRenderResourceRecord` (`2Ch` bytes); `+10` holds DWORD
size accounting. Names and alias nodes use the actual native string pool and
existing native list operations. Record copies never retain the resource at
`+28`. No semantic `EffectCache`, separate owner collection, or callable copy
of an original numeric vtable is involved.

The names are reconstruction hypotheses. These are new MSVC Win32 C++
interfaces, not binary replacements. The native function bodies and their
callees were inspected before composing them. Original SEH/FH3 encoding,
application platform dispatch, complete effect loading and gameplay are not
established by this packet.

## Entry points and ownership

| Address | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `B318B0` | ECX renderer; stack header; EAX effect; RET4 | Optional guard, copied/lowercased name, embedded registry with `0,1,1` |
| `B31090` | ECX registry; four stack DWORDs; EAX effect; RET10h | Complete lookup/resolution/load/record/accounting schedule for `D5F074` |
| `B2E940` | ECX unused; output/name/unused word; EAX output; RET0Ch | Actual mutable VFS name resolution with substring rewrite |
| `B31FF0` | ECX unused; stack effect; EAX same; RET4 | One InterlockedIncrement at actual effect `+04` |
| `B301A0` | ECX vector; stack record; RET4 | Append copied record, then increment current count |
| `B2FFE0` | ECX vector; stack capacity; RET4 | Signed minimum 64, copy/destroy/free, then publish new allocation |
| `B2FD10` | ECX destination; stack source; EAX destination; RET4 | Actual deep copy of strings/aliases; unretained resource pointer |
| `B2FA10` | ECX record; RET | Alias/sentinel/name destruction; leaves resource ownership untouched |

`B31090` first calls the current application's `BECCD0` load-event service,
copies and normalizes the request using `BEE690`, then searches every alias.
This search requires equal counted lengths before CRT `_stricmp`. Matching
null rows do not stop a later nonnull row from satisfying the request. Any
nonnull hit calls current registry `+C = B31FF0`, regardless of either flag.

On a miss, current registry `+4 = B2E940` copies the original name twice,
constructs actual `.shfx` and `.mshd` headers, and uses existing raw `4CAD40`
to replace every case-sensitive `.mshd` substring. It calls the required actual
`BDF4C0` service on the changed header first. On failure it resolves the original
header, then returns that mutable result even if the second call also fails.
Each resolver call reads the current VFS publication. Both candidate headers
are actual pooled strings; neither is a semantic string copy.

After normalizing the result, a distinct resolved name triggers a second scan
that compares only each row's first alias. A match appends the request alias
before reading resource `+28`. A null match still falls through to loading.
`allow_load` gates only that cold load, after resolution and possible alias
insertion. There is no negative-cache shortcut or effect-pointer deduplication.

Current registry `+8 = B2EBB0` returns an owned reference, which may be the
caller reference from recursive `error.shfx` fallback. After return, even a null
result constructs a record, queries the actual VFS date, adds aliases, and
appends the row. A nonnull result calls its current `+C` size method. The actual
`D5E534` and `D61A00` profiles both select the established `A82250` zero-return
body; accounting still performs the native call and DWORD addition. Only a
fresh nonnull load with `acquire_new != 0` adds another reference for the caller.

The caller-owned `NativeMaterialEffectCacheAcquired` records the nested loader
operation, returned owned identity, successful record publication and additional
caller acquisition separately. The loader's frame is present before the call;
`loaded` is published immediately after return, before sentinel allocation,
date query or append. Thus a throwing post-load operation leaves the acquired
reference visible. Once the row is published it owns that reference. The frame
does not add or release references, and cannot replay after it starts. Omitting
the optional frame supports hits and no-load requests; a cold load without a
frame is an explicit source error before invoking the loader. No temporary
frame silently discards a partially constructed effect.

## Captures, reloads and exceptional boundaries

`B318B0` captures its ECX receiver before optional guard entry. The guard uses
the separately sampled current `F8D394` renderer, so the guard receiver and
registry receiver may differ. It uses the actual `B33AD0/B33B00` synchronization
implementation. Normal cleanup tests current mode and releases the captured
name data with current length. Exceptional cleanup uses `B21110` and the
current string header. A disabled entry followed by enabled exit would consume
unwritten native guard bytes; the C++ interface explicitly rejects that domain
instead of manufacturing a successful guard result.

The `B31090` listing disproves its decompiler's `unaff_EBP` accounting receiver:
`B310AC/B310B4` save incoming ECX, `B3148A` reloads it into ESI, and `B314AC`
adds to that same registry's `+10`. EBX holds the loader result after `B3130F`.
`B314AF` reads the actual acquire-new byte at stack `+84`; the earlier
allow-load test is `B312EB`, stack `+88`. `B31137` is unreachable because
`B31133` compares EAX to itself. Other sentinel-validation calls remain live.

Vector reserve and append preserve signed comparisons, wrapping DWORD sizes,
current count/data reloads and placement-delete unwind without added rollback.
The reserve EH helper `CBD9A0` and append helper `CBDA30` call the actual no-op
`401130`; failed copies may leave replacement storage/completed copies
allocated, as in the native body. This does not consume or release an effect
reference. Record destruction nulls its current sentinel after returning free,
then releases the current name; unknown `+08` and resource `+28` remain intact.

Saved Ghidra analysis still has two returning-free omissions at packet review:
`B2FA44 -> BF65AC` truncates the body before `B2FA6B -> 419CC0` and
`B2FA72 -> BD1510`, although installed bytes continue through `B2FA87 RET`.
`B3009B -> BF6989` omits `B300A0..B300A9`, which publishes the replacement
pointer/capacity before restoring EBP. Full installed spans were decoded and
hashed; no worker Ghidra mutation was performed. The report retains those real
call rows so the verifier exposes the saved-body defect rather than hiding it.

## Required composition and validation

The cache calls the production `load_native_material_effect_00b2ebb0` directly,
with its persistent `NativeMaterialEffectLoadAcquired`. Loader/cache contexts
must point back to one another and use the same actual string bridge and VFS
publication as date queries. Numeric registry and effect profiles are checked
at reached virtual calls. Array allocation/free callbacks must use the actual
native allocation domain. Missing concrete resolver, shader-program, texture
fallback or application event providers remain explicit dependencies.

The tracked loader and shader-program bodies are separate concurrent packets.
This cache does not supply successful replacements for them. In particular,
these source bindings do not establish full Text material creation or rendering.
Build status, call-verifier output and any focused runtime evidence are recorded
in `reports/native_material_effect_cache.json`; exported, source-reconstructed,
compiled and runtime-proven coverage are distinguished there.

Correction from `docs/NATIVE_MATERIAL_EFFECT_INTEGRATION.md`: both returning-free tails are repaired and saved. B2FA10 now owns bytes through B2FA87; B2FFE0 owns through B300BC. The destructor had only default undefined locals and no parameters; its existing name and plate comment were preserved by the supported recreation helper. All 89 numeric call rows now pass; indirect sites remain separate evidence. The isolated extracted-leaf fixture experiment is not accepted runtime evidence; the combined-library record fixture is recorded in the batch report.
