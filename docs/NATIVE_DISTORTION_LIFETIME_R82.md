# Native distortion owner lifetime

Addresses: 00b4ed70, 00b4ee20, 00b4ee70, 00b4f0a0, 00b4f0c0, 00b4f150, 00b4f540

## Result and evidence boundary

R82 reconstructs seven complete bodies, totaling 764 native bytes: record reserve
(104), resize (80), resource clear (286), record destructor (23), owner constructor
(129), owner destructor (112), and scalar destructor (30). The strict MSVC Win32
build and three existing CTests pass. No repository tests were added.

A focused original/source probe passes two record-helper traces and four owner
constructor/lifetime cases. It uses real D3D holders and a frame target, plus
explicit sparse post20 fixtures. Camera and scene fields are deliberately null
in this fixture. **Their release paths, B4F560 initialization, B4E470 construction,
shaders, original exception handling, application resource startup and gameplay
were not executed.** The source exposes new interfaces with canonical provider
contexts; native entry ABI equivalence is not established.

## Actual owner extent and constructor

The B107F0 caller requests **26Ch**, at B11E40, allocates at B11E45, constructs at
B11E62 and initializes at B11E79. B4F0C0 takes ECX and returns it in EAX, RET.
It captures CF4848 bits 3FE66666 (approximately 1.8) before any owner write,
stamps CEB130/count1/null08/null0C, then D61F1C and the captured bits at +244.
Separate later reads of CE3958 (40000000, 2.0) and D1F3C4 (3FD9999A,
approximately 1.7) go to +248 and +24C. It zeros +254/+258/+25C,
+10/+14/+18/+1C/+20/+240, then bytes +250/+251, preserving other preimages.

In particular, **+34 camera, +38 frame and +3C scene are not initialized by this
constructor**. Later B4F560 publications establish them, but its capability-failure
path can return before those publications and the caller then releases the owner.
Safe failure behavior for that path remains unproved; R82 does not invent extra
zero stores. The probe compares all 26Ch constructor bytes, including these
preserved fields, before explicitly supplying null camera/scene fixture fields.

## Eight-byte record array

The actual 12-byte header at owner+254 contains a pointer, signed count and signed
capacity. Each element is two opaque DWORDs; no element retain/release is inferred.

* B4ED70 clamps capacity requests below one to one, compares signed capacity,
  allocates wrapping DWORD request*8 through BF55BE, copies both words of live
  records, frees the current old pointer through BF6989, then publishes pointer
  and capacity. It reloads count per iteration and the source base per record.
* B4EE20 reserves only when requested count exceeds capacity, zeroes each newly
  live pair, decrements stored count while shrinking, then publishes requested
  count. Shrinking preserves inactive record bytes.
* B4F0A0 resizes to zero and frees backing storage, retaining the now-stale pointer
  and capacity fields. The source preserves this behavior.

Original reserve/resize use ECX header, one stacked signed argument and RET4;
array destruction uses ECX/RET. Allocation failure, invalid extents and corrupt
headers are outside the demonstrated domain.

## Resource clear and canonical providers

B4EE70 takes ECX owner, RET. It releases +10 and +14 using a fresh CE2220
decrement-import read for each nonnull child. It captures +18 first, then captures
that import once for +18/+1C/+20/+38, camera unlink at +34, and +3C/+240.
Each nonnull field clears only after the terminal operation returns.

Supported zero-count terminals use current profile dispatch: D61EB8 holder to
B4E410, D61EC0 post20 to B4E430, D5E600 frame to B1FCF0, and D62D48 scene to
B72580, through the actual BD30E0 slot. Post20 and scene releases require the
existing canonical companion sharing the actual storage's count. The camera
uses the existing node registry's concrete NativeCameraReference and full B6DFA0
unlink/self-release; no replacement count or object map is introduced.

The scene provider currently uses the existing semantic SizedStoragePool name
domain. Raw scene construction/name-domain integration remains a dependency for
full B4F560/application wiring. The borrowed scene-companion publication may
become dangling after zero release and must not be reused by its external owner.
Camera and scene binding code compiled but was not exercised by this probe.

## Complete destructor and recovered listing tails

B4F150 stamps D61F1C, arms state1, clears resources, selects state0, resizes
the +254 record array to zero, frees backing, disarms and invokes full B0F5E0.
The original CBFC66 handler references DF87EC FuncInfo with two unwind entries
at DF87DC: state0 -> -1 runs CBFC50/base B0F5E0; state1 -> 0 runs
CBFC58/array B4F0A0. Source cleanup follows those actions; a secondary C++
exception during cleanup terminates. Original FH3/SEH behavior remains unproved.
B4F540 invokes the destructor, frees iff flags bit0, returns original ECX identity
in EAX and RET4.

Locked Ghidra repairs restored post-free tails without changing any CRT callee's
no-return flag: nine bytes in B4ED70, three in B4F540, 35 in B4F150 and five in
B4F0A0. The latter two needed explicit function-body recreation after flow repair
left the stored body truncated. Prior names/comments, repair events and saved
body bounds are recorded in the report and local evidence archive. All seven
descriptive names and evidence comments are saved and force-exported at closeout.

## Runtime verification

The record traces cover clamp, growth, copying, no-op reserve, shrinking,
zeroed regrowth, resize-zero and stale-header destruction, with header guards.
Four owner cases compare original/source behavior for flags0/1, all 26Ch
constructor bytes, three actual texture/surface holders, a real frame retaining
a holder surface, three sparse post20 fixtures with real frames/surfaces, and
record backing disposal. Flags0 retains an extra caller reference to one post
through parent destruction, then releases it separately. Tracking returns to
baseline, the canonical post registry empties, and final device/API counts are zero.

The original instruction bodies run from parent-reserved code bands. Allocator,
base and resource terminals use the documented full-source adapters. Native
exception-handler and camera branches trap if reached. The renderer is an
explicit zeroed 1D94h fixture, not application startup. Probe source, linked
objects, native bytes, logs, built artifacts and hashes are sealed before the
integration rebuild; the report distinguishes that run from integration checks.

## Follow-up packets

Recover B4F560's remaining providers, including B20160 format capability probing
and raw scene construction/name-domain integration. Then bind and execute full
distortion initialization and camera/scene teardown with actual shader/material
providers. B107F0 resource-service binding, failure paths and gameplay remain open.
