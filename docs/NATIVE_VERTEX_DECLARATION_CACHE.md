# Native vertex declaration cache

Addresses: `00B317E0`, `00B305F0`, `00B2C280`, `00B31D20`,
`00B2FBB0`, `00B2F910`, `00B2FE20`, `00B300C0`.

The actual renderer's `D5F0A8+38` entry loads a declaration through its embedded
`D5F060` registry at `renderer+1A60`. This reconstruction uses that registry's
actual storage and the existing actual string pool, alias lists, decoder and VFS
date route. Names are descriptive hypotheses, not recovered symbols. The new C++
interfaces do not replace the original binary ABI or its FH3 exception machinery.

| Routine | Original ABI and inclusive range | Coverage |
|---|---|---|
| B317E0 | ECX renderer; stack name; EAX declaration; RET4 at B3189E; through B318A0 | Complete normal body |
| B305F0 | ECX registry; stack name, loader word, acquire-new, allow-load; RET10 at B30AD0/B30B34; through B30B36 | Complete schedule, qualified to D5F060 slots |
| B2C280 | ECX unused; stack output, name, ignored word; EAX output; RET0C at B2C2CD; through B2C2CF | Complete |
| B31D20 | ECX unused; stack declaration; EAX same; RET4 at B31D32; through B31D34 | Complete |
| B2FBB0 | ECX destination; stack source; EAX destination; RET4 at B2FC4F; through B2FC51 | Complete |
| B2F910 | ECX record; RET at B2F987 | Complete normal body and host cleanup schedule |
| B2FE20 | ECX array header; stack capacity; RET4 at B2FEFA; through B2FEFC | Complete normal body |
| B300C0 | ECX array header; stack record; RET4 at B3012C; through B3012E | Complete normal body |

## Storage and ownership evidence

The producer B2FBB0 copies name fields `+0/+4`, constructs the alias owner at
`+8`, then copies six DWORDs `+14..+28`. B305F0 constructs the same record at
`B30871..B309FA`: sentinel at `+C`, count `+10`, five date words `+14..+24`,
resource pointer `+28`. This matches the existing `NativeRenderResourceRecord`
layout; its former renderer+1A74 use does not make a second layout necessary.
The unknown `+8` word is preserved. No record copy/destruction calls AddRef or
a resource destructor. The registry array at `+4` has data/count/capacity words;
its `+10` accumulator receives a loaded declaration's actual `+CC` stride.

The first lookup visits every alias, requires equal counted lengths and then
uses native `_stricmp`. Empty counted strings compare equal without reading
their buffers. After identity resolution and normalization, a changed resolved
name causes a second scan that compares ONLY each record's first alias. A match
appends the requested normalized alias before testing the cached resource.
This asymmetry and the counted-string versus CRT-NUL distinction are retained.

A nonnull hit always invokes the current registry `+C` acquire entry, regardless
of either Boolean argument. B31D20 performs one `InterlockedIncrement` at the
actual declaration's `+4`. On a cold path only `allow_load` gates decoding; both
Boolean arguments use their low byte. A null decoder result still gets a new
record, aliases and an actual VFS date query, and is retried on later lookup.
Nonnull fresh results increase accumulated stride and acquire only when
`acquire_new` is nonzero. The cached creator reference is retained as native
storage ownership; no private host owner map or duplicate companion is added.

The renderer wrapper copies then lowercases its input before calling
`registry(name,0,1,1)`. Normal cleanup uses the data pointer captured after the
initial resize and the current length; its exception cleanup uses the current
header. These differ if an external service changes that temporary header.

## Dependencies and exception boundaries

All 61 direct call rows are recorded numerically in the report. Existing actual
bodies implement 41DD40/419CC0/BD1510, BEE690, 4D48A0, 4D05E0, 4C3020,
4CE6F0 and 4CE780. BF7680/BF7FBF remain current CRT memcpy/string comparisons.
BECCD0 drains the real Windows thread messages and invokes the supplied canonical
platform's required XLive and cursor services. The platform publication is a
typed application projection, never an overlay on the game's raw platform.
BDD340 consumes the current actual VFS publication and its existing date route.
The string pool supplied to the cache, decoder and VFS must be the same domain.

Current raw registry identity and slot tokens are read at each virtual call.
Only original `D5F060+4=B2C280`, `+8=B2DBD0`, `+C=B31D20` are implemented here.
Changed profiles/slots produce an explicit source-domain error; this is not a
recovered native failure. The loader word remains in the API although both
known resolver and decoder ignore it. VFS provider coverage and declaration
parser dependencies retain their own documented limits.

Returning-free gaps were verified against installed bytes: B2F944 calls BF65AC
and continues through B2F987; B2FEDB calls BF6989 and continues through B2FEE9
before the existing epilogue. Their local Ghidra call overrides were repaired;
the CRT callee no-return flags were not changed. Alignment gaps B3068D..8F and
B2FE7D..7F are not missing cleanup branches.

FH3 metadata `DF60D4 -> CBD820` and `DF61DC -> CBD8E0` shows current-name cleanup
for record destruction/copy. Reserve's `DF6260 -> CBD940` and append's CBD9D0
only call no-op placement delete 401130: no replacement-buffer or completed-row
rollback is invented. B305F0's five states at DF6420 unwind the normalized name,
resolved name, temporary resolver output, temporary-record name, or completed
record. The native decoder result is not released if later record work fails.
List-count failure likewise does not free its newly allocated unlinked node.
Source uses existing C++ exception transports; native FH3/SEH and allocator
failure ABI parity remain unverified.

## Verification

Eight complete spans match live Ghidra bytes and the installed executable
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Source and focused probe compile under MSVC Win32, `/W4 /WX /fp:strict`.
The initial probe passes31 original/source comparisons: record copy/destruction,
signed capacity growth and append, resolver identity, reference increments,
hot hits and lookup-only misses, renderer wrapper, and cold valid/null loads.
Cold paths invoke the actual decoder and an empty actual VFS tree; no mounted
physical-file date provider is exercised. The fixture binds one declaration
companion and explicitly drains outstanding references to verify actual+04
terminal deletion and pool return. It does not implement application registry
registration or cache destruction/release through B32210/B31D40.

The second resolved-name scan is assembly-reviewed but unexecuted: the admitted
identity resolver and stable normalization do not produce a changed name.
The initial native registry vtable is relocated to callable fixture addresses;
the source run uses the same original identity tokens. Exact integrated build
and current-library-only replay results are recorded in the report. Native
FH3/SEH, changed external profiles and gameplay remain unverified.

## Follow-up packets

Connect the same cached declaration creator reference to the application's
canonical `NativeVertexDeclarationReference` registry when its full renderer
owner is composed. Recover actual declaration-registry destruction B32210 and
release B31D40 before claiming complete registry lifetime. Extend cold-load
VFS coverage beyond the empty tree and exercise the changed-resolution branch
through an evidence-supported actual resolver. Actual B1DFF0 render-context collection and complete
renderer device recreation remain separate owners' reconstruction work.
