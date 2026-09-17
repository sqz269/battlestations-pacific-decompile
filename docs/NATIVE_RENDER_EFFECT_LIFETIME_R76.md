# Native render-effect pass and bloom lifetime

Addresses: 00b0f5e0, 00b10120, 00b10140, 00b10160, 00b10180, 00b54e70, 00b54ea0, 00b54f70

## Result and scope

R76 reconstructs eight complete bodies (542 native code bytes): the common
render-effect destructor, four pass scalar wrappers, and bloom construction,
destruction and scalar deletion. Descriptive names remain hypotheses.
The new C++ context interfaces are not drop-in replacements for the original ABI.
Full effect initializers and application resource-service binding remain open.

| Entry | Native extent | Original interface | Recovered behavior |
| --- | --- | --- | --- |
| B0F5E0 | B0F5E0..B0F673, 148 bytes | ECX owner, RET | Common effect teardown |
| B10120 | B10120..B1013D, 30 bytes | ECX owner, stack flags, EAX owner, RET4 | Depth-downscale scalar wrapper |
| B10140 | B10140..B1015D, 30 bytes | Same | Particle-blend scalar wrapper |
| B10160 | B10160..B1017D, 30 bytes | Same | Downscale4x4 scalar wrapper |
| B10180 | B10180..B1019D, 30 bytes | Same | Downscale2x2 scalar wrapper |
| B54E70 | B54E70..B54E99, 42 bytes | ECX owner, EAX owner, RET | Bloom owner construction |
| B54EA0 | B54EA0..B54F69, 202 bytes | ECX owner, RET | Bloom teardown |
| B54F70 | B54F70..B54F8D, 30 bytes | ECX owner, stack flags, EAX owner, RET4 | Bloom scalar wrapper |

All five scalar wrappers destroy first, free only when flags bit 0 is set,
and return the original address even after free. Their three-byte ADD ESP,4
tails were absent after erroneous call-site CALL_RETURN overrides. One locked
repair batch restored all five tails; the CRT free function's flags were untouched.

## Common base and concrete child owners

B0F5E0 writes profile D5E140, captures member +08, then captures the actual
CE2220 decrement import once. It decrements each nonnull member's actual +04
count, invokes its current virtual slot 0 at zero, and clears the parent slot
after the call returns. It then freshly reads member +0C and uses the same
captured import. After disarming cleanup it calls the complete BD30F0 base leaf.

The admitted concrete child profiles are D61EC0 (post-effect20) and D61EB8
(texture/surface holder). Their actual readonly tables identify BD30E0 and
deleting slots B4E430 / B4E410. Post-effect20 dispatch resolves the existing
canonical NativePostEffect20Reference only at zero, verifies its storage/count
identity, and invokes its complete terminal lifetime. There is no duplicate
reference count or temporary owner. Holder dispatch uses the full R75 lifetime.

The four pass profiles are D5E164, D5E178, D5E18C and D5E1A0. Producer observations
at B540B0, B542D0, B544F0 and B546F0 identify respectively downscale_depth.mshd,
blendparticles.mshd, downscale4x4.mshd and downscale2x2.mshd. Their +08 children
come from B4E470 and +0C children from B4E020. These observations establish the
member domain; they do not constitute complete initializer reconstructions.

## Bloom layout and teardown

The B107F0 service caller pushes 43Ch at B10FB3, allocates at B10FB8 and calls
B54E70 at B10FD5 before publishing service +28. The constructor writes CEB130,
count 1, +0C zero, D62150, +18/+1C/+20/+24 zero, and +08 zero, in that order.
Dimensions +10/+14 and the parameter-array region +28..43B retain their preimages.

B54F90's producer places three holders at +18/+1C/+20 and post-effect20 owners
at +24 (blur5x5.mshd) and +08 (bloom.mshd). It also supplies dimensions and float
parameter arrays. That initializer is not implemented by this packet.

B54EA0 writes D62150 and releases +18, +1C, +20, +24, +08 in order. Each nonnull
operation reads the current CE2220 import anew (three static import operands;
the first executes in the three-holder loop). Every slot clears after its
captured child's call returns. It disarms derived cleanup and calls full B0F5E0,
which rereads +08 and +0C rather than assuming their values.

## Exception evidence

The base state-0 cleanup is CBBC20, handler CBBC28, FuncInfo DF3E8C, map DF3E84;
it calls BD30F0 only. Bloom uses CC0260, handler CC0268, FuncInfo DF8D98, map DF8D90;
its state-0 cleanup calls full B0F5E0. An escaping child exception may therefore
cause the bloom cleanup to revisit +08. The source preserves that behavior,
including terminate on a second C++ exception, rather than preclearing members.
This is a source C++ cleanup projection. Original FH3/SEH behavior is unproved.

## Validation

- Strict MSVC Win32 build, /MD /W4 /WX /fp:strict, and all three existing CTests pass.
- 850 selected live Ghidra bytes match the original PE: 542 packet code bytes,
  the existing 14-byte BD30E0 invoker, and 294 bytes of unwind/table/caller evidence.
- Twelve original/source real-D3D9 cases pass: eight four-pass wrapper cases,
  plus bloom with scalar flags 0/1 on both sides. All eight relocated packet
  bodies are exercised on normal paths; this is not branch-complete coverage.
- Original/source bloom constructors match across all 43Ch bytes from an identical
  CC preimage. Retained allocations verify cleared slots, base profile and untouched
  tails. The particle case keeps an extra caller post-effect reference, verifies
  count 1 and its registry entry after parent teardown, then releases it separately.
- Actual GPU holders, canonical post-effect companions and full frame-target
  lifetimes retire with registry/tracking restoration. The raw singleton drain
  releases two registrations; final device and API reference counts are zero.
- No new repository tests. Logs, probe code/dependencies, artifacts and hashes are
  sealed separately before integration; the report records the combined-build receipt.

The probe uses an explicit zeroed 1D94h renderer fixture, a real HAL device,
actual pools/raw strings/support/scalar domains and sparse 20h post-effect fixtures
with real frame groups retaining real surfaces. Node, material and draw-record
fields are null. It does not run B4E470 or any complete pass/bloom initializer.
Original numeric dispatch uses parent-reserved code bands and exact x86 adapters
to complete existing child lifetimes. Original FH3 handlers are unreached traps.
Fixture compilation required supplying the unused node type bootstrap's explicit
constructor inputs and removing a shadowed local name; production source was unchanged.

Unproved: native failure/exception execution, arbitrary profiles, import mutation,
nonempty node/material/draw-record destruction in this fixture, full initializers,
application resource-service wiring and gameplay. The next dependency work is the
remaining B107F0 subordinate initializer graph, retaining these concrete lifetimes.

## Correction from docs/NATIVE_RENDER_PASS_INITIALIZERS_R77.md

R77 supplies complete source bodies for B540B0 depth-downscale and B542D0
particle-blend initialization, plus the primary-surface/color-target helpers.
Strict build and arithmetic/helper comparisons pass; full initializer execution,
the remaining pass/bloom initializers and application binding remain open.
