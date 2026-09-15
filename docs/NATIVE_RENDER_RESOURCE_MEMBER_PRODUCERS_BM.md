# Native render-resource member producers (BM)

Addresses: `00B0F6E0`, `00B0FC00`, `00B107F0` and the bounded producer sites below.

This is an **analysis-only, partial producer-closure packet**. It adds no native body credit,
source, build, fixture, binary ABI or game claim. The report pins the baseline/source inputs,
original target, complete bulk release order, selected call sites, and remaining uncertainty.
Native function names here are descriptive hypotheses. Ghidra was read-only.

## The +70 preimage has a later producer, not an early zero

App `0073D410` pushes `6ACh` at `73DEDF`, calls `BF681B` at `73DEE4`, cleans four
stack bytes at `73DEE9`, then calls `B14A10` with that allocation in ECX at `73DF01`.
There is no intervening zero fill. `BF681B` is malloc/retry/throw, not calloc;
`B0F020` establishes base/service registration, not a zeroed derived block. Accepted BK
`B14A10` deliberately preserves `+70` because the native constructor does not write it.

The actual later writer is `B107F0`: ECX becomes service ESI at `B10816` and ESI stays
the receiver until the epilogue. The fresh path is selected by the byte `+1C4` test at
`B1081C/B10829`; it writes that byte to **one at B10904**, before the following sequence:

| Native site | Effect |
|---|---|
| B1150A / B1150C / B11513 | Allocate **20h (32 bytes)** with BF681B; caller removes4 bytes |
| B11529 / B11535 | Build string header from D5E360, `HDRFinalPass.mshd`, using41E870 |
| B1153F / B11541 / B11555 | Push0, push3, push header; ECX is captured allocation; call B4E470 |
| B1155C | Allocator-null branch supplies EAX=0 |
| B11563 | Publish returned EAX at service+70 |

There is no fresh-path branch edge from `B1084E..B11562` that skips past `B11563` in
the complete scanned listing. This establishes the uninterrupted successful path, including
the explicit null publication. An earlier exception can leave the original `+70` bytes while
`+1C4` is already one. The already-initialized fast path skips these allocations. Consequently
the flag is insufficient to admit constructor-only or failed-initialization bulk cleanup.

`B4E470` writes count1 and profile **D61EC0**, whose slots0/4 are **BD30E0/B4E430**.
It takes ECX plus three DWORD arguments and returns the receiver with `RET0Ch` at B4E83B.
It is distinct from the parent's **24h (36-byte), D61EC8** auxiliary owner/provider.

The 20h terminal `B4E1F0` releases material+14, unlinks nodes+0C then+10, frees raw+18,
then releases frame+08 and destroys the base. Material is captured before the CE2220 epoch
at B4E219; zero branches use current object tables and fields clear after each callback.
The raw+18 allocation is **28h (40 bytes)**: BF681B at B4E7BB, B51BD0 at B4E7EC,
publication B4E7F5. It is not the retained logical stream of the 24h owner. B4E470 transfers
its local stream into a mesh and then releases that local reference.

Existing material, frame-target, node attachment/unlink, and CRT providers are useful concrete
children, but the 20h canonical owner/reference and constructor/FH3 lifetime composition are
missing. No generic callback closure or zero-only destructor substitutes for them.

Saved listing gaps remain at B4E269..B4E26E (ADD ESP,4; clear+18) and B4E445..B4E447
(ADD ESP,4 in scalar deletion). Disk disassembly confirms these bytes. This worker did not
repair them. B4E430 destroys first, frees only for flags&1, returns captured allocation, RET4.

## Field-to-producer map

`B0F6E0..B0FBF8` visits **42 fields / 41 distinct offsets**, including +1D4 twice.
`B0FC00..B0FC04` is a five-byte tail jump to it. Both inherit ECX-service/no-stack-argument
ABI. The release body first calls B52270 on current+34, then captures CE2220 once for its
42 decrements, dispatches current slot0 only after zero, and clears each current field after
callback return. The final byte+1C4 clear is B0FBEF. Preserve the duplicate visit and order.

The map identifies sites for 37/41 fields; this does **not** mean37 concrete provider closures.
Rows refer to B107F0 unless another function is stated. Existing terminal source does not
automatically provide a missing factory or its registration/count lifetime. The report has
individual sites, profiles, initializer edges, and exact native cleanup order.

| Fields | Producer / current profile | Concrete source and remaining work |
|---|---|---|
| +68,+6C,+70,+74,+80,+650,+654,+658,+65C,+660,+664 | B4E470, 20h, D61EC0 | New20h owner lifetime/constructor closure; material/frame/node/CRT children exist |
| +38,+3C,+44,+48,+4C; +40 alias of+4C | B4E020, 18h, D61EB8 | B4E020/B4E140/B4E410 family missing; retain alias explicitly |
| +50; +58 alias of+50 | B2A7C0, D619A0 | native_surface_owner terminal exists; full factory missing (only registry append fragment exists) |
| +54 | Renderer D5F0A8/+88 -> B2A070 -> B3F7B0, D61948 | native_texture_2d_owner lifetime exists; both factory bodies missing |
| +5C | Current+54/+30 -> B3FD80 | Accessor result/retain contract still open; surface terminal exists but domain not admitted by name |
| +60 / +64 | Inline20h /20h, D5E164/D5E178 | Initializers B540B0/B542D0 and terminals B10120/B10140 missing |
| +18 / +1C / +24 | Inline220h/90h/224h, D5E18C/D5E1A0/D5E1B4 | Initializers B544F0/B546F0/B54940 and derived terminal families missing |
| +20 | B50D40,250h,D61FE0 | Constructor/initializer B51090 and terminal B50FE0 family missing |
| +28,+2C | B54E70,**43Ch**,D62150 | B54F90 initializer and B54F70 terminal family missing |
| +30 | B4F0C0,**26Ch**,D61F1C | Derived initialization and B4F540 terminal family missing |
| +1C8,+1CC,+1D4 | B1FBB0,D5E600 | native_frame_target_owner constructor and lifetime contexts exist |
| +66C,+670,+674,+678 | B0FD70/B0FDC0/B0FE10/B0FE60: current renderer+64(name,0) | B319B0 cache provider exists; setters missing, returned profile must be established, not forced2D |
| +10,+78,+7C,+1D0 | BK zero; nonzero writer unresolved | No concrete domain admitted; bounded store search is not proof of no later producer |

In particular, +40 is retained from +4C at B10A95, not from the preceding +3C factory;
+58 is retained from +50 at B10BAF, not from +5C. Allocation size comes from the caller:
B54E70 receives43Ch at B10FB3/B11DE5 and B4F0C0 receives26Ch at B11E40.

The +1D0 store candidates at 4DF036 and5042C1 were rejected: their receivers are light
objects and their values are float-component data, not service ownership. Other candidate
stores and aliases remain open. There is no exhaustive global absence claim for the four
unresolved fields.

## The second default texture is also held by material slots

B107F0 loads service+67C at B12142 andB1253C. B4CBA0 at B12151/B1254B is exactly
`MOV EAX,[ECX+14]; RET`; it returns the +658/+65C owners' material and leaves the two
pending arguments intact. ECX becomes that material and B189F0 at B12158/B12552 consumes
slot3 and the texture with RET8. Its changed-resource path publishes, retains the incoming
texture, and releases the old texture; identity-equal assignment does not add a reference.

Those are two material-owned references when the assignments change. B0FEB0 clears
**slot2**, not those slot3 references, and separately releases remap+66C..+678. Material
terminal lifetime can release material-owned slots, but it does not prove disposition of the
original B14A10-returned reference in service+67C. B319B0's fresh cache publication itself
adds no retain; acquisition differs by path/flags. Complete cache/shutdown/alias closure is
still required. Omission from B0F6E0 and B14F60 alone proves neither a leak nor a balanced
lifetime; this packet makes neither claim.

## Next bounded implementation packets

1. Recover20h D61EC0 constructor EH/count lifetime and implement B4E1F0/B4E430 with the
   existing material/frame/node/CRT providers and canonical actual-owner binding; examine
   B51BD0's40-byte raw-block contract. Preserve failed-init+70 admission separately.
2. Close18h D61EB8 B4E020/B4E140/B4E410 before admitting its six fields/alias.
3. Split the five inline derived families and B50D40/B54E70/B4F0C0 families by actual child
   ownership dependencies. Names and profile bytes alone do not resolve their terminals.
4. Complete B2A070/B3F7B0/B2A7C0/B3FD80 and the four remap setters through existing
   concrete pools, cache, current profiles and owner lifetimes.

No source/build/tests were changed or run. The existing call verifier is recorded in the
report; indirect rows are explicitly labeled and its skip behavior is not indirect-target
proof. All original handler/EH, typed count lifetime, unknown profile, failure and runtime
limits remain explicit; no destructor source admission was made.
