# Native texture-source payload lifetime CC10

`C30470` now begins the lifetime of a separate `NativeTextureSourcePayload` at
actual source+08 before initializing its pointer header. The old constructor
wrote raw DWORDs to source+10/+14/+18; that did not establish the typed payload
and header needed by future source consumers. The new default placement-new
has no parentheses/braces or member defaults, so it adds no initialization
stores. The original +10=null store now uses the live volatile pointer member.

The payload is18h bytes: selected DWORD at actual+08, float rate at+0C,
the existing `NativeRenderPointerArrayStorage` at+10, initialized byte at+1C,
and three padding bytes at+1D. Size/alignment/offset, standard layout and trivial
default construction/destruction assertions passed in the actual Win32 build.
It has no owner/destructor/counter wrapper, callable vtable or complete34h-owner
claim. The actual+04 atomic remains outside this object and is not restarted.

The source constructor first executes the existing B19980 unchanged. It then
places this payload at+08, captures the returned new-object pointer, stamps
the original D79B54 profile and writes through that pointer's volatile
children_08.data_00 member. The six post-base stores retain their exact order:
profile00 -> pointer10 -> count14 -> capacity18 -> selected08 -> byte1C.
The count/capacity scalar DWORD accesses remain their matching signed/unsigned
integer accesses. The constructor still returns the original complete receiver.
The public comment distinguishes that receiver return from the placement-new
result used for the typed store.

The payload excludes the existing atomic placement at B19980's original
count1 point. No default atomic, second header, repeated construction, retain,
allocator fallback or cleanup was added. The constructor writes no rate bytes
+0C..+0F, padding+1D..+1F or derived payload+20..+33. That untouched-byte result
is **static** source/compiled-store proof only; no indeterminate bytes were read
or initialized with fixture sentinels.

Fresh, exclusive, four-byte-aligned backed storage is required. Current real
BBC6F0/BBC810 creators allocate34h through the existing source CRT boundary;
they call C30470 only on nonnull captured storage, stamp final integer profiles
D64478/D644B4 and return that same allocation identity. Their shared helper now
inlines both constructors. The native BBC670/BBC7C0 wrappers also call C30470,
stamp those profiles and return the same captured receiver; they are native-only
wrappers, not new reconstructed functions. No old payload/header reference,
active count user or canonical companion may survive a fresh construction.

Using placement-new's returned pointer needs no launder for this constructor
store. No generic getter/cast was added that would adopt arbitrary native bytes.
Later typed users must establish the actual constructor/lifetime provenance;
launder alone cannot create an object. Raw base/profile/derived lvalue and native
32-bit address-arithmetic assumptions remain separate source boundaries.

One strict Release Win32 build passed with /MD, /EHsc, /O2, /W4, /WX and /fp:strict;
all three existing CTests passed. No new test or runtime case was added.
Old source and the full old core library/member were captured before a safe
fast-forward to disjoint main a40922b3a. The constructor/header inputs did not
change through that synchronization. The new build was sealed at that baseline.

All20 old/new leaf code sections,674B in each object, are byte/relocation
identical. All26 old/new raw code/data/metadata sections are also identical.
The two complete5523B COFF objects differ only at timestamp bytes04/05.
Exact old and new native_resource_cache_leaves.obj members match their archived
full core libraries. Complete selected bodies are B19980 source27B, C30470
source65B, the shared creator133B and two creator wrappers14B each. The shared
creator inlines the unchanged base/source store schedules; each wrapper forwards
its same final profile. The selected253B is source COFF evidence, not a new
native-byte total. Complete C30470 and the shared creator each retain exactly
one actual+04=count1 store, the six post-base stores and the same receiver return.
There are no new external calls, extra owner writes or FP instructions.

The six original PE windows total262B: C30470's44B capture includes its complete
35B body and9B padding; B19980[22], BBC670[18], BBC7C0[18], BBC6F0[80], BBC810[80]
are complete bodies. All match the installed original PE. Existing native ABI
is ECX receiver/EAX same receiver/RET for both constructors and wrappers; the
creators allocate and return the captured owner. The unchanged C++ source
interfaces are not drop-in original ABI/FH3/SEH/hardware-fault replacements.

Current 735FF0/737390/C304A0 raw pointer lvalues/slot writes and backing-slot
lifetimes remain unresolved. The naked C302A0/C302F0 consumers were not changed
or executed. The real C30570 producer, Lua raw-error transport, actual animation
assets/key/nonnull-child behavior, platform/MSG/XLive and broader record/string/
provider pointer domains remain separate. No source0/W, nonempty clock/sampler,
startup/draw/application activation, full ISO-safe graph or game proof is claimed.

Evidence is frozen at `local/cc10_texture_source_payload_evidence.zip`, with
`local/cc10_texture_source_payload_implementation/manifest.json`. The archive
contains full old/new libraries and leaf objects; compiled_sections.json has
code bytes/relocations, complete_coff_sections.json has all raw sections, and
compiled_schedule.json has explicit store/return/caller proof. Named source/
config/tool pins and actual build tool-return objects are retained. Nested
prior/cc10_positive_texture_source_readiness.zip is a separate read-only native
audit, not a reconstructed C30570 provider or runtime result. No Ghidra mutation
was made by this worker; the ledger appends one zero-new-byte fragment.
