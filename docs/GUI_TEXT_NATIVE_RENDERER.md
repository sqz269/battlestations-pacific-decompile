# Text actual renderer factories

This packet replaces the callable raw renderer-slot requirements in AB8400 and
AB8910 with concrete source dispatch to B317E0, B287C0 and B288B0. The receiver
remains the same actual renderer. Its identity is D5F0A8, and borrowed current
table views contain original numeric code addresses, never a substitute callable
C++ vtable. Source names remain descriptive hypotheses.

| Body | Original ABI and inclusive range | Coverage |
|---|---|---|
| AB8400 | ECX unused; stack capacity, mesh; RET8 at AB852B; through AB852D | Complete normal buffer sequence using actual factories |
| AB8910 | ECX Text; no stack arguments; RET at AB8C24 | Existing normal caller; concrete declaration/vertex subpath, downstream requirements remain |
| B317E0 | ECX renderer; stack counted name; RET4 at B3189E; through B318A0 | Reuses existing complete raw cache wrapper, no duplicate implementation |

The existing project C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe were
verified by the read-only BSP wrappers. Both Text listings and B317E0 were read;
register provenance and stack cleanup were checked against assembly. Native
table bytes D5F0E0/D5F104/D5F108 resolve current38/5C/60 to
B317E0/B287C0/B288B0. This worker made no Ghidra mutation.

## Identity and references

GuiTextNativeRendererServices borrows NativeVertexDeclarationCacheContext and
NativeStreamCloneServices. The same actual string pool feeds the name, cache
and decoder. Streams share renderer publication, synchronization, physical
contexts, declaration type-size table and canonical geometry registration.
The existing NativeVertexDeclarationReference exposes a read-only terminal
context comparison so a previously bound declaration can be reused safely.

B2DBD0 constructs declaration+04=1. B305F0 retains that creator in the actual
registry record. B317E0 calls the registry with acquire-new=1 and allow-load=1,
so a fresh nonnull return has both cache and caller references. A hot nonnull
hit always adds one caller reference through B31D20. B287C0/B288B0 construct
one stream creator and append unretained raw pointers to renderer arrays;
their appends do not supply a second ownership count. Vertex construction
retains the same actual declaration. Existing mesh setters retain streams.

GuiNativeGeometryOwners stores declaration companions beside its existing
mesh/section/material/stream companions. The registration domain supplies an
explicit nonmutating find hook; lookup failure is never interpreted as absence.
An existing NativeVertexDeclarationReference is reused only when raw identity,
pool, type sizes and terminal profile match. Neither creating nor reusing a
companion changes actual+04. Terminal destruction returns the actual pool slot,
then unbinds and retires that one companion. No second retained-object registry
or semantic stream storage is added.

## Native order and interruptions

AB8400 captures current renderer38 after format creation, releases the format,
then reloads renderer5C for capacity*4,flags1,declaration. Mesh publication
precedes vertex release, then declaration release. Only afterward does it
reload renderer60 for capacity*6,flags1,format65. Mesh index publication precedes
index creator release. DWORD products wrap. AB8910 similarly reloads after
name release, requests four vertices, and releases declaration BEFORE vertex;
it has no index factory. Model/material/layout order remains unchanged.

GuiTextGlyphBuffersAcquired and the cursor's factory records preserve returned
declaration references and stream creators across source exceptions. The actual
stream factories publish creators before renderer-array append. Companion bind
failure also keeps the raw creator and any unregistered companion available;
there is no rollback or retry. Every potentially terminal creator release clears
its diagnostic pointer first. Declaration-cache internal failures before B317E0
returns retain that module's existing native effects and documented boundary:
its decoded creator is not exposed by the existing cache API if later record
append/date work throws. This packet does not claim repaired cache exception ABI.

The new AB8400 overload takes services and caller-owned acquired state. A lower
overload accepts the same actual renderer publication/string/native services
without requiring a Text widget. The old five-argument signature throws an
explicit unsupported-context error until its content caller is migrated.
GuiTextBufferServices.native_renderer defaults to null only for aggregate
compatibility; reaching creation requires a real binding.

## Remaining requirements and validation

AB8910 material creation at AB8B85 invokes 535320, whose current renderer+48
target is B318B0. The existing material factory still expects that actual callable
effect-loading boundary. Its material constructor, pools and terminal services
are already reconstructed. Section B865A0 still requires current stream+24 and
renderer+40 B2F710 services. Actual Text parameter-owner retention B18A40 must
bind the real Text+04 owner. None of those dependencies is replaced with success
callbacks. This packet does not establish complete Text/cursor execution.

The report records the exact current build and ignored fixture results. The
focused fixture composes actual declaration decoding/cache, the original raw
declaration pool, real D3D9 HAL vertex/index creation, canonical cache-hit reuse,
metadata-registration interruption and real stream/declaration terminals. It
uses initialized fixture renderer/pool storage, an empty actual VFS tree and
observed fixture platform callbacks. Full renderer/device recreation, native
FH3/SEH, material/layout/Text identity composition and gameplay remain unverified.
