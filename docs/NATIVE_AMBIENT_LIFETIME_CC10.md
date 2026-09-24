# Raw ambient lifetime

This packet closes the two destructor bodies for the actual98h owner made by
the existing raw B7C290 constructor. Separate host metadata can bind that
owner's existing +04 count into the canonical actual-owner registry. It does
not create a SceneResource, lighting view, another count, or a larger allocation.

| Native range | Bytes | Source | Coverage |
| --- | ---: | --- | --- |
| B7C450..B7C4B2 | 99 | `destroy_native_ambient_00b7c450` | Complete normal body |
| B7C7E0..B7C7FD | 30 | `delete_native_ambient_00b7c7e0` | Complete scalar wrapper |

B7C450 takes ECX=actual owner and returns with RET at B7C4B2. B7C7E0 takes
ECX=actual owner and stacked flags, returns the original numeric identity in
EAX and RET4 at B7C7FB (three bytes). These are new source interfaces; the
scalar function borrows the actual flags cell so its low byte is read late.

| Function | Call site | Native target | Binding |
| --- | --- | --- | --- |
| B7C450 | B7C47D | B7BC70 | Existing actual12B array, count0 path |
| B7C450 | B7C485 | BF6989 | Existing shared CRT free |
| B7C450 | B7C49D | BD30F0 | Genuine seven-byte base body |
| B7C7E0 | B7C7E3 | B7C450 | Complete new raw ambient body |
| B7C7E0 | B7C7F0 | BF65AC | Existing shared CRT free |
| CC1EB0 cleanup | CC1EB3 | AA6E10 | Existing genuine base cleanup, tail jump |

The supported backlink descriptor is the actual12B view at +08: pointer,+04
signed count,+08 signed capacity. Both count and capacity are nonnegative,
count<=capacity, and begin is null or a correctly owned CRT allocation. The
existing `resize_system_ambient_backlinks_00b7bc70(array,0)` therefore reaches
only its count-shrink/final-zero stores. It neither allocates nor touches an
element. Its SceneResource pointer type is only an opaque key width in this
path; no SceneResource object is fabricated, cast from the ambient, or accessed.
The shared helper remains outside this packet's ownership. Intermediate count
stores have no asynchronous observation claim beyond that provider's contract.

After the resize call, the body rereads current begin and frees it. It does
not clear begin or capacity, decrement the owner count, release a scene key,
or rewrite any other payload. After free returns it stamps D5C104, then invokes
BD30F0 to stamp CEB130. Callback changes to other surviving fields remain.
The scalar wrapper reads only the current low flags byte after this entire
body. `flags & 1` frees the owner; flags0 leaves the destroyed raw allocation with
its count and dangling pointer/capacity words intact. Explicit scalar deletion
works at nonzero count and does not decrement it.

Ghidra currently truncates B7C450 at B7C489 because BF6989 was treated as
nonreturning. Installed bytes and live bytes prove the missing fallthrough
B7C48A..B7C4B2, including the post-free stores and direct B7C49D call. The
worker made no Ghidra mutation; the report records that missing41B fragment
with inclusive end and final instruction length for primary repair. B7C7E0's
post-free ADD ESP,4 bytes at B7C7F5..B7C7F7 are also retained.

Native handler CC1EB8 loads descriptor DFAF90: magic19930522,one state,map
DFAF88. State0 maps to previous-1/actionCC1EB0, which loads this from[ebp-10]
and tail-jumps AA6E10. The normal path disarms the state before D5C104. The
source guard reuses this existing AA6E10 body on escaping C++ failure and
does not retry array cleanup. Valid count0 plus the existing nonthrowing CRT
provider does not ordinarily throw. Native FH3/SEH, CRT faults and exception
identity remain explicit boundaries, not proven equivalents.

`NativeAmbientReference` is separate postconstruction host metadata. Binding
requires completed actual98h storage with its already-live +04 atomic, current
D62F3C profile and current first two table cells BD30E0/B7C7E0. It validates
the canonical registry, adds no native write/retain, and refuses duplicates
without changing the native owner. No admission call is invented in B7C290.

At final zero, current virtual0 must be BD30E0. The genuine helper rereads
the owner profile; its current virtual4 must be B7C7E0 and receives flags1.
There is no unknown-profile fallback or second decrement. Explicit deletion
uses the same companion without testing/decrementing count. Both flags0 and
flags1 retire and unbind metadata. Entered destruction failure also retires
once, leaving partial native state/raw allocation caller-owned with no retry.
Registry unbind and companion destruction read host metadata only, so they do
not touch storage after free. Caller quiescence prevents reuse before retirement;
the companion, context, borrowed profile and common registry survive dispatch.

Strict MSVC Win32 build and both existing CTests passed. All129 body bytes,
the handler/map and concrete profile match the installed PE. The mechanical
call check verifies real targets and all available call-site listings; the
missing B7C49D listing uses the explicitly recorded raw-fragment exception.

One ignored probe compares two original/source scalar pairs over actual98h
allocations and real backlink arrays. It compiles this source directly with
only the free provider rebound to a probe seam that really frees memory and
then mutates surviving fields and, in one pair, flags1->0. The original copied
bodies use the same genuine source B7BC70/BD30F0 and the same free seam. Exact
98B snapshots match; checks prove the old profile/count0 at array-free entry,
post-free field preservation, final CEB130, late flags and array-before-owner
free order. Original native unwind is not exercised.

Separate source lifecycle checks use the real free provider without callback
instrumentation: unchanged98B binding/duplicate refusal, count7 with flags
borrowed from array.count3 (cleared to0 before the flags read), count9 flags1,
count1 final-zero dispatch, and metadata retirement/destruction after owner
free. The count-alias check concerns the new borrowed-cell source interface;
it does not claim arbitrary original-stack/owner alias ABI. Assertions are
active with a compile-time NDEBUG rejection. Exact `/MD /fp:strict` and
`/link /MANIFEST:EMBED` command/logs remain in the report's ignored artifacts.

Raw3Ch scene-resource construction/setters, its28h registry, render lighting
views, and application resource-graph admission remain separate work. The
older enlarged ConcreteSystemAmbientLight APIs are not used by these bodies
or this canonical companion. No gameplay or binary replacement claim is made.
