# Type5 isolated fixture: static gate peer

The bounded static peer passes for Root's `local/type5p3` fixture at the final
bookend on 2026-10-09. **Source credit remains 0.** This packet is independent
static inspection, not Root's `manual_static_review` receipt or a target launch.
The only direct edge leaving the 53 gated bodies is the explicitly identified,
ungated and runtime-unadmitted GS failure frontier described below.

The peer used a separate PE32/COFF parser and x86 decoder over frozen bytes.
It did not import or execute the selected recipe, helpers, readers, compiler,
Native queries, provider code or probe. Root's prepare, build and static-gate
closed records were read as data: all report exit 0, no timeout and passing
pre/post bookends. No tracked C++ or shared metadata changed.

| Evidence | Independent result |
| --- | --- |
| Root fixture | Exactly 575 frozen files; original and frozen hashes unchanged at the final bookend |
| Selected dependency record | SHA-256 `3b8c0e2d492e973770145a7fd47ff7f3c6d4c2f094be742b4ca4e0a602677637` |
| Built executable | SHA-256 `8e5f8389b43ee81a3c8d29c2219fd337b019733943718a87459b58849cf42ffa` |
| Packed gate | 53 disjoint spans, 9,283 body bytes; SHA-256 `18c3a99a3731aeaa5e3d0b791d7f6930fb428719e2584465ff40b7a0ba9a1050` |
| Retained TU functions | 35 defined bodies plus 2 weak aliases, 37 rows total; Source/duplicate/canonical/probe definition counts 1/2/9/23 |
| Relocations | 279 symbolic relocation checks, 273 distinct physical operands; raw COFF, linked image and gate bytes agree |
| Gated instructions/edges | 2,809 instruction tiles; 489 direct edges, 57 explicit IAT edges, 8 other indirect sites |
| Entire executable virtual section | 130 mapped intervals, 13,216 bytes; 4,540 instructions covering 13,201 bytes plus 15 final zero alignment bytes |
| Providers | Four packed 104-byte records match two frozen I386 provider files, named nonforwarded exports, hashes and 32-byte export prefixes |

The 63-byte Source body at linked `45001000` contains 19 instructions and
`RET 12`. All 59 bytes outside `[35,39)` equal the frozen Native body from
`008EF2B0`; the only changed operand reaches the qualified duplicate at
`45001080`. The 57-byte duplicate retains 49 literal bytes with only `[34,38)`
and `[44,48)` substituted, reaching allocation adapter `45001040` and memcpy
thunk `4500421C`. Each masked body has exactly one match in the executable
section. This review does not rebind historical Source associations.

The canonical weak `??_Eexception` and `??_Ebad_alloc` symbols use COFF
`IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY` aliases to the corresponding `??_G`
definitions. Both alias bodies, sizes and complete relocation identities agree;
they are not extra synthetic definitions. All four actual build objects and the
executable match their build records, which consume no BSP archive or old object.

All eight non-IAT indirect sites have a bounded reaching definition. The
allocator loads EBX from malloc IAT `4500509C` and EDI from `_callnewh` IAT
`450050A0`. `read_gate` loads EDI from parsed `KERNEL32!ReadFile` IAT `45005028`
at `45001CA7` for call `45001CC0`, then reloads it at `45001DBE` for calls
`45001DD1` and `45001E0B`. The ordinary Original caller and raw caller use their
target entry argument at `[ESP+10h]` and `[EBP+8]`, respectively.

The allocation adapter's cookie frame and complete 14-byte cookie helper are
present. The full 43-byte `__alloca_probe`/`__chkstk` body includes mapped internal
labels `cs10` and `cs20`, the page-touch loop and final stack exchange. Main's
`AND ESP,FFFFFFF0` is a separate alignment operation before the stack probe.
The full cold delete chain is gated: 16-byte sized delete at `450034F0`, the
five-byte unsized tail at `45003850`, and six-byte free thunk at `4500424C`.
All 14 required named observer, hashing, memory and exception-support imports
have covered edges. Cold allocation failure and EH execution remain unclaimed.

The sole outgoing direct gate edge is cookie tail `450034E9` (`E952030000`) to
`___report_gsfailure` at `45003840`. The peer additionally read that complete
16-byte mapped interval: `mov ecx,2; int 29h; ret`, followed by eight `CC` bytes.
It is an explicit fail-fast frontier, is **not part of the 53-span gate**, and
is not runtime-admitted. Whole-section byte decoding also includes CRT startup
and other MSVCRT scaffolding; it does not expand the smaller runtime gate or
establish those routines' runtime behavior. Root retains its own final
disposition of this frontier.

The raw caller remains 181 bytes with one `CALL [EBP+8]` and a bare `RET`.
`Capture` is 132 bytes: 16-byte leading sentinels, 100-byte register record and
16-byte trailing sentinels. Its emitted instructions write all 25 U32 record
slots, including the three dead argument slot/word pairs before `PUSHFD` can
overwrite them. ESP-before precedes the three argument pushes; Source `RET 12`
is the matching cleanup. The raw and ordinary callers introduce no floating
point save/restore adaptation. EAX/ECX/EDX, callee-saved registers, EFLAGS, ES and
stack guards are recorded; no actual passing register observation or general
ABI preservation is claimed here.

Two earlier attempts of the peer's own decoder stopped before completion. The
first rejected repeated nonexecutable `.idata$6` map labels; a fresh decoder
retained every row and prohibited ambiguous relocation lookup. The second
stopped at 15 final zero alignment bytes after the last mapped function's RET;
a fresh decoder retained and hashed that range separately and verified no
direct branch targets it. Those closed failures and corrections are sealed.
They did not change selected fixture artifacts.

The evidence family is
`J:/PROG/battlestations-pacific-decompile-cc12_type5_complete_guard_materialization_TEXT/local/t5staticPeer01`.
Its manifest lists 2,736 artifacts; together with `artifact_manifest.json` and
`seal.json`, exact family membership is 2,738 files. Manifest SHA-256 is
`d77f45a277b0d83ad838223d0d4d172a821cf63744d2e49c4aaa93b911a91bcb`;
seal SHA-256 is `8342e8a95c13a7f6ea28cded5c6fdf611ece72811851557d5e71b9796ae8412a`.
Before/after checks cover 2,683 original/copy pairs (575 fixture, 6 Source,
2,102 peer-analysis dependencies), 5,366 hashes, 16 analysis scopes and 3,588
scope memberships, with equal peer environment and PowerShell host identity.
The final Root membership check found no additions or changed originals.

The machine-readable companion is
`reports/cc12_type5_isolated_static_gate_TEXT_peer.json`. Root's independent
selected-reader/protocol checks, final manual receipt and any actual target
launch remain separate work. No differential runtime, drop-in original ABI,
startup, factory or game equivalence is established by this packet.
