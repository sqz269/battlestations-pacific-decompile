# Raw mission Lua queue and payload readiness

At published Main `3fe6fe7eb9ae9528341f3291897cbb26988c0182`, the complete
native queue clear and payload destructor are pinned, but their complete raw
Source dependency composition is still open. The smallest coherent next step is
the reached variant-retirement cycle, using real pool/malloc/free services and
a bounded whole-range erase-all library contract.

There is no independent missing leaf that can safely replace that cycle.
The real small initializer/string/link leaves already exist. The 17-byte tag6
helper calls variant clear before its tag store; a tag-only replacement would
omit required cleanup.

This audit changes exactly this document and
`reports/cc16_native_mission_lua_queue_payload_readiness.json`. No Source,
CMake, ledger, Ghidra, guard, caller, original-install or peer-tree edit was
made. No build, test, probe, executable, target body, game or DLL was run.

## Complete envelope and target proof

Live `bsp.py ghidra` queries verified the existing project `bsp`, program
`/battlestationspacific.exe`, x86 LE32 and base 00400000 before access.
The configured `C:/Users/sqz269/bsp.gpr` exists. Live/snapshot counts remain
77,631. No import, function definition, listing repair, annotation or save occurred.

Nineteen fresh live/PE code/data blocks total 1,236 bytes; all match the installed
PE. The report independently decodes every selected code byte. The PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Twenty-six relevant repository Source/config pins match the frozen base.

| Native body | Complete envelope | Bytes / instructions |
| --- | --- | --- |
| Payload destructor | `008876B0..00887743` |148 /49|
| Queue clear | `00887D90..00887DCD` |62 /26|
| Variant vector resize | `006B88E0..006B8983` |164 /58|
| Variant clear | `00886920..008869C7` |168 /59|
| Variant destructor | `00884AF0..00884B54` |101 /29|
| Subtree cleanup | `006EE890..006EE925` |150 /50|

The full selected 201-byte range-erasure span is captured only to pin the reached
whole-range path and its validation/domain. Its 1-byte NOP at6EE9DF is not reached
from ordinary entry:200 ordinary bytes/76 operations remain distinct from the
201-byte/77-operation decode. General vendor erase/copy bodies were not
reconstructed. The current6EE5D0 metadata envelope ends at6EE882; its earlier
truncated637-byte/691-byte continuation history is not new body credit here.

## Payload: exact strings, vector and EH epochs

P is the actual24h-byte payload at node+8. Its two8-byte NativeString headers
are atP+0 andP+8, followed by the12-byte vector atP+10h. The two trailing words
atP+1Ch/+20h are not read by this destructor. ECX=P, no public stack arguments,
ESI/EDI are saved, and the complete body ends with plain RET.

The148-byte body corrects the old58-byte-prefix/no-string-release comment:

1. Retain H=P+10h and arm full EH state1 before `006B88E0(H,0)`.
2. After complete resize return, reload the current H.data and pass that pointer
   to BF6989. Do not cache it before the child; no vector field is cleared.
3. Capture the current second-string buffer atP+C. Demote only the state low
   byte1->0 before its optional pool return. Read lengthP+8 only if that captured
   buffer is nonnull.
4. Capture the first-string buffer atP+4. Demote the full state to-1 before its
   optional pool return. Read lengthP+0 only when needed.
5. Each nonnull branch prepares captured block, wrapped DWORD length+1 and
   unused1 before no-argument00419CC0. Its returned pool becomes ECX for BD1510,
   which consumes the pending three words. They are not new getter parameters.
   Neither string header is cleared.

FuncInfoDC9AA4/mapDC9A94 encodes state1->0/actionC97538 for
`0041DD20(P+8)`, and state0->-1/actionC97530 for `0041DD20(P)`.
The actions use the saved payload through the EH-adjusted frame.
The selected bytes establish the actions and demotions; they do not prove the
FH3 interpreter's action-frame ABI or an equivalent Source unwind.

The full resize body reads current data/count/capacity at H+0/+4/+8 and uses
20-byte elements. Shrink decrements the current count before selecting an
element, reloads data/current vptr/slot0, invokes that current slot with flags0,
then rereads count. It finally overwrites count with the requested value.
Request0 is not proof that a complete resize implementation can omit reserve,
growth, copy, computed-pointer guards or element lifetime. Empty/fabricated
headers would conceal the actual missing work.

## Queue: detachment is not retirement

ECX=Q supplies the raw12-byte header: opaque+0, head+4, count+8.
The native has no null-head or null-node success guard.

It captures the first sentinel's next, reseats that sentinel.next, then reloads
Q+4 before reseating sentinel.prev. It compares the captured node with current
Q+4 before publishing count0.

For every nodeN it captures NEXT=[N] before `008876B0(N+8)`. Only after payload
normal return does it BF65AC the captured N. It then compares captured NEXT with
**current Q+4 after free**, and advances using that captured NEXT.

The public queue is already reseated/count0 when a callback fails. The current
node, detached remainder, vector allocation and partially retired nested values
can still be live. Independent current-node/NEXT/context metadata must survive;
an empty public header is not successful complete retirement. A cached sentinel,
C++ delete or an added null fallback would change this operation.

## Existing Source: precise classification

The gap is narrower than saying all existing code is semantic:

| Source | What already exists | Concrete closure still missing |
| --- | --- | --- |
| Argon queue/payload | std::string/std::vector payload, new/delete nodes and added null fallbacks | Actual raw payload/node/free/current-head composition |
| settings_seg29 | Real definitions over a12-byte vector header and20-byte pitch | Concrete LuaVariantHooks copy/initializer/current-element lifetime services |
| cc13_left55 | Real normal bodies and static-asserted raw Win32 layouts: string8, key16, variant20, map12, value1Ch, nil31h | Genuine Left55Host pool/free/range services and exact retained callback/failure composition |
| cc13_seg33b | Raw-address subtree normal body | Genuine variant destructor/pool/free services through its Seg33bHost seam |
| native_string | Complete raw0041DD20 overload taking NativeStringRawPoolContext | Actual shared pool/manager/gate identities retained through last use |
| native pool and leaves | Actual419CC0/BD1510/BD12A0,006EE020,006EDEA0,006EEAF0 and008849B0 | Their existence alone creates no payload/container owner or callable profile |

Bounded all-source name searches find Left55Host and LuaVariantHooks only in
their defining header/implementation files, and Seg33bHost only in its header
and two implementation files; no concrete subclass is present in those results.
These broad interfaces contain many unrelated pure methods. Filling the unused
ones with successful defaults, exceptions or fake providers would not close the
required services.

Fresh D0E6F4 data points at scalar00884FA0. The real raw20-byte initializer stores
that numerical identity, but it does not install/authenticate a Source table.
The complete30-byte scalar still depends on the actual variant destructor and
its matching free/receiver domain. Source current-slot transport remains separate.

## The smallest coherent retirement packet

The reached cycle is:

`00886920 ->006EE960(whole range) ->006EE890 ->00884AF0 ->00886920`.

It has three bounded normal bodies, totaling419 native-envelope bytes:

- `00886920..008869C7`: capture tag once. Tag1 captures/frees its real8-byte
  string descriptor after actual pool return, then clears retained variant+8.
  Tag5 captures mapM, calls whole-range erase, reloads **current M.head** for
  sentinel free, clears M+4/M+8 before freeing captured M, then clears retained
  variant+8. All normal paths write tag=-1; null selected payloads do not add a
  payload clear. It has no local FS frame.
- `00884AF0..00884B54`: stamp numericD0E6F4, arm state0 before variant clear,
  capture current name bufferV+10h afterward, then demote state to-1 before the
  optional pool return using late lengthV+Ch. No name/receiver free or clear.
  State0 actionC970D0 targets actual0041DD20(savedV+Ch).
- `006EE890..006EE925`: stop for any nonzero nil BYTE31h. Recurse right first;
  only after return capture current left. Retain keyK=N+Ch in the rewritten
  native node-argument slot; arm state0 before value destructor(N+1Ch).
  Afterward capture current key bufferK+8, demote state before optional return
  using late K+4, free captured N, and continue with captured left.
  State0 actionC830A0 targets actual006EE020(K).

These bodies already have normal Source implementations with broad host seams.
The next work should reuse/factor those bounded kernels and close real narrow
services/current/captured/failure metadata, rather than report duplicate recovery.

The required library operation is specifically genuine erase-all over the actual
map. At88693F..694F the caller supplies ownerM/first=head.left and ownerM/end=head.
With valid nonaliasing live Source backing and no ambient concurrent mutation
before branch selection, validations select the whole-range path without an
intervening external callback.

That path reloads current M.head/root, invokes the real subtree cleanup, then:

1. Reload H1; write H1.parent=H1.
2. Reload H2; write M.count=0, then H2.left=H2.
3. Reload H3; write H3.right=H3.
4. Reload H4/its left; write output ownerM and node, returning the output.
   Header+0 remains untouched.

Later clear reloads current M.head for its free. A real library contract must
preserve these effects and callback reloads; it cannot return a fabricated empty
map. This reached contract is **not** a claimed whole Source006EE960 function.
General iterator erase, copy and6EE5D0 remain named boundaries and earn zero
vendor-body reconstruction credit.

## Exact proposed implementation boundary

Root may choose `cc16-native-lua-variant-retirement-cycle`:

- New narrow interfaces/context in
  `include/bsp/native_lua_variant_retirement.hpp` and
  `src/native_lua_variant_retirement.cpp`.
- Reuse/factor only the named bodies in
  `src/cc13_left55_seg55.cpp` and `src/cc13_seg33b_misc.cpp` if needed to avoid
  duplicate kernels. Preserve existing broad-host API behavior; do not populate
  those interfaces with stubs or add an unused forwarding layer.
- Borrow the SAME real NativeStringRawPoolContext cells:
  pool01090AA8, manager01090AA0, return-gate01090AA4 and qualified raw deletion
  domain. Getter419CC0 must run for every nonnull return, including large blocks
  and disabled small returns.
- Use genuine variant20/map12/node52/string-descriptor8 backing and the matching
  singleton_lifetime_allocate/free Source domain for descriptor/map/node frees.
  Buffers use their actual pool return. The admitted node allocator and map
  initializer already establish their raw allocation/field contracts.
- Retain independent current variant/map/node/left/key/value/allocation/provider
  metadata across callback failure and through last normal/EH use. Do not infer
  automatic cleanup or completion from a public header reset. Source failure
  transport and Original FH3 equivalence remain separate.

This identifies the smallest coherent normal dependency composition; it does
not declare the complete queue/payload body ready or adopt a producer.
The 17-byte0074FB30 is not a replacement leaf: it calls variant clear before
storing tag6. Existing raw initializer/string/link leaves are already available.

Still held afterward: complete resize/reserve/copy/assign and actual element
scalar/current-slot services; then complete payload and queue composition;
CF/D0 authentication, full host/scalar/cell/producer adoption and runtime proof.

The historical helper receipts are pinned. Isolated helper files still match
relevant receipts, but older whole native_string and pool-storage file pins
differ from current hashes; no reason for those whole-file differences or current
aggregate build result is inferred. This packet adds zero Source, unique
Original, vendor, duplicate, ABI or activation credit.
