# Post-effect current argument cells (CC10)

This packet extends the existing B4E470 constructor and numeric 535320 material
factory with raw argument interfaces. It preserves the existing nested
logical model/camera/provider domain. It does not migrate those children to
the separate actual45Ch camera/actual24h scene graph or admit full distortion
initialization. Validation here is strict build and compiled ordering only;
there is no new runtime or copied-original/source fixture.

`NativePostEffect20ArgumentView` is immutable metadata pointing to three
initialized live volatile DWORDs. The shared B4E470 body borrows that backing
without starting an aggregate or `NativeString` lifetime over it. The original
typed C++ entry initializes three private input cells and calls the same body.
View metadata, context and diagnostics must be disjoint from mutable native
storage; the three cells remain stable through the call and synchronous source
unwind. Unexposed private/nested stacks, saved-register/return gaps, object
lifetime transitions and asynchronous mutation remain excluded alias domains.

B4E470 covers `[B4E470,B4E83E)` (974 bytes, 325 instructions). Native ECX is
the actual20h destination; the three stacked words are name, count and optional
size receiver; EAX returns the owner and B4E83B is three-byte `RET0C`. At entry
ESP E the inputs are E+4/E+8/E+C; normal post-prologue ESP S=E-48h makes them
S+4C/50/54. Only the second cell is written by this body.

| Native site | Shared engine operation |
| --- | --- |
| E4A5 | Capture current count after base profile/count, before derived profile and +C/+10/+14 clears; store captured count to owner+1C. |
| E5D4 | Capture current name-header identity for numeric 535320. |
| E5F0 | Capture current count after section creation; use it for all section count/topology stores. |
| E640, E6D4 | Publish model/camera allocation to the second argument cell; constructors retain their captured allocation identities. |
| E6E1 | Independently capture current name-header identity for raw 43C130 prefix. |
| E741 | Publish viewport allocation to the second cell. |
| E75B | Capture first optional receiver before consuming state 10. |
| E76E, E772 | After the height getter, reload optional receiver and capture its current profile. |
| E774, E778 | Store height into the second cell, then load width slot1C from the captured profile. |
| E77D | After the width getter, reread current second cell for height. |
| E7C3 | Publish draw-record allocation to the second cell. |

The optional dimension provider remains restricted to genuine D619A0 with
B3CD20/B3CD10. The existing numeric validation now accepts the captured
profile, so it does not reread the receiver profile after the height write.
The slot itself remains a late read. No general callback dispatcher, guessed
profile or fallback is added. Preserving these read points is source/compiled
evidence; arbitrary callback mutation is not newly exercised.

The numeric 535320 entry covers `[535320,53539C)` (124 bytes), ECX actual8h
name-header identity, EAX material, plain RET at 53539B. Its new `const void*`
overload shares the existing numeric cache engine. The `NativeString&` entry
forwards its address without reading the header. The genuine B318B0 loader
receives that captured actual header; current fields remain in its existing
raw domain. Pool construction, temporary effect release, canonical owners and
failure diagnostics are unchanged. The separate callable factory is untouched.

Descriptor DF86D4/map DF86F8 still has twelve states. Current incoming E+8 is
read by states4/7/10/11 for model return, camera return and the two CRT frees.
The source no longer clears that caller cell through a private `late_raw`
exchange. It consumes the state before cleanup, loads the current word and
leaves its stale value intact. The reached word must identify the corresponding
valid live allocation; normal constructors still use their captured native
register-equivalent result. Private frame/mesh cleanup metadata remains as
before. No extra completed-child rollback or reference credit is introduced.
The existing newest-secondary-C++-failure policy is explicit source behavior,
not native FH3/SEH equivalence.

The report includes all ten B4E470 funclets and CBFBAB handler, plus 535320's
C6C240 raw-slot return and C6C248 handler. Three returning-free listings remain
truncated in the frozen worker snapshot: CBFB38 needed CBFB41 POP ECX/CBFB42 RET;
CBFB95 needed CBFB9E POP ECX/CBFB9F RET; CBFBA0 needed CBFBA9 POP ECX/CBFBAA RET.
After release, the primary cleared the returning-free flow overrides and
recreated all three complete 11-byte bodies. Fresh listings include each final
POP/RET, and all 47 direct/tail checks pass. The free callee's annotation was
unchanged. Prior annotations and the frozen worker evidence are preserved in
`reports/cc10_post_argument_flow_repairs.json` and
`reports/cc10_post_argument_function_definitions.json`.

All 1,249 code bytes and 176 descriptor/map bytes match live Ghidra and installed
PE bytes. The 64 transfer inventory contains 47 verified direct/tail rows and 17
indirect rows pinned in the listings. Strict MSVC Win32 and all three existing
CTests pass. `local/cc10_post_effect_argument_view/compiled_checks.json` pins
36 selected instruction checks, the two actual compiled objects and listings.
For the shared constructor, current initial count is +D1 before derived store
+F5; later count is +7EA. The optional chain is second receiver +D86, profile
+D94, height write +DA9, late slot +DC7, width leaf +DE2, current height +DF0.
Cleanup reads are +FA/+172/+1E6, with no write back to that argument cell.
The typed factory forwards at +4 and raw factory pushes the captured header
at +168 before genuine loader +16C. This is bounded compiled-dataflow evidence,
not a full operand/ABI or runtime proof.

Prior CL admission independently records a genuine constructed base-effect
hot-cache fixture. That evidence is preserved; it is not rerun or extended to
raw headers here. Cold descriptor/program paths, source0 B3B280/B5F100 sampler
preimages, complete B4E470 composition, full B4F560 and B107F0 remain outside
this packet. In particular, caller+34/+38/+3C validity is not supplied by zeros
or inferred from a capability return. No application/game launch, fake COM,
unknown-profile success or new tracked test was added.

The packet archive retains both frozen initializer and argument-readiness
handoffs unchanged. Prior names, comments and reconstruction records remain;
no additional complete native-body credit is implied by the new overloads.
