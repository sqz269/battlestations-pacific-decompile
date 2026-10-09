# Raw mission Lua variant loop child: 006EE5D0

The admitted saved body `006EE5D0..006EE84C` is 637 bytes / 215 instructions.
It ends at a call to `_free` and contains no return. Root separately authorized
the diagnostic window `006EE84D..006EE88F` after that boundary was reported.
Its first 54 bytes / 19 instructions form a **recovered continuation candidate**:
late count maintenance, two output DWORDs, saved-register/FS restoration and
`RET 0Ch` at `006EE880`. The remaining 13 bytes are `INT3` after that return.

The combined ordinary continuation candidate is 691 bytes / 234 instructions,
with 14 calls and one `RET 0Ch`; it is not a changed Ghidra AddressSet. All 704
authorized diagnostic bytes match the installed PE, but **704 is not a function
size or reconstruction-credit claim**. No Ghidra property or analysis was changed.

The normal zero-flag path rewires raw links and byte fields, calls unresolved
helpers, retires a payload and node, then publishes the late stack pair. The
entry name `STL_xlen_throw_006ee5d0` does not describe the whole behavior or justify
discarding its normal continuation. Source closure remains held at six small
unimplemented helpers, the native throw boundary, and raw lifetime/EH contracts.

## Scope, byte coverage and evidence

- Baseline: `96791d7c7a27eae29d58f1df60adbf954e1c3b4f`.
- Packet: `cc12_lua_variant_tag5_loop_child_ABI_readiness`; worker:
  `agent/cc12_lua_variant_copy_child_abi`.
- Root explicitly admitted the entire larger body after a fresh metadata gate:
  637 bytes, 215 listed instructions, 59 blocks, 87 edges and 14 calls. The usual
  300-byte gate was not silently bypassed.
- The first lease covered entry `006EE5D0` and exactly the two outputs. After
  Root's separate approval it was extended to `006EE84D..006EE88F`. No byte from
  the diagnostic window was read before that approval/lease extension.
- Before diagnostic bytes, the bounded index query found no entry in that
  window. Live metadata queries at all 67 byte addresses recognized no function;
  full boundary queries also reported no function. This does not prove that no
  possible external branch enters those bytes.
- Saved-body SHA-256:
  `ea548a4c719816157ac2a5c4274ca3fa42773a9a6b36a4d13364c679a81636e7`.
- Diagnostic-window SHA-256:
  `9e81a0529e1df787a4ca28a6d8080effc3b616ddc9d8873ffbfe9f337cbf1044`.
- Recovered 54-byte continuation SHA-256:
  `f37ac0f1d55d7507b318a5ae18ca039da83fa326d17022674a36b958ee7dfb49`.
- Combined 691-byte candidate SHA-256:
  `aca9db4fb0136284f01836c5cb6fccc4df1be4a9ab5ab19e438b5d770f55cdf6`.
- Installed PE SHA-256:
  `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
  The initial RVA/file offset is `002EE5D0h`; continuation offset is `002EE84Dh`.
- Each live CLI query verified existing project `bsp`, program
  `/battlestationspacific.exe`, language `x86:LE:32:default` and base `00400000`.
  Configured `C:/Users/sqz269/bsp.gpr` exists and was reused. Counts remained
  64,729 live/snapshot. This is not a stronger exact-path endpoint attestation.
- Capstone 5.0.7 decodes all selected bytes. Every direct jump target is inside
  the 691-byte candidate; there is no indirect branch or required jump table.
  The 13 trailing `CC` bytes are excluded from ordinary-entry reachability.
- All 215 saved-body instructions are present in the live listing. The missing
  continuation lies outside the saved extent; no no-return flag or cause was
  queried or repaired. The full saved pseudocode nevertheless discards 11 raw
  transplant-block entries, which remain explicit here.
- Only owned bytes plus the approved diagnostic window were read. No Native
  child/caller/handler/string/profile/table body or byte at `006EE890` was read.
  Immediate addresses and names printed in the owned pseudocode are references,
  not independently verified outside data.

The permissive local graph, allowing each call to return, reaches 234 instructions
/ 691 bytes. Forcing only the initial zero-flag branch reaches 216 instructions
/ 615 bytes; it excludes the 76-byte / 18-instruction exception-construction
path. These are static models, not execution, termination or child-ABI proof.

## Incoming state and exact frame

Let S be entry ESP; R be entry ECX; O, F and N be the three stack words at
`S+4`, `S+8` and `S+0Ch`. N0 is the initial value loaded from N. The accepted
`006EE960` parent delivers ECX=R, EDX=its uninitialized local-eight-byte output
address, and logical arguments `(O,F,N)`. This body obtains its output base from
the late O stack load; the duplicate incoming EDX is not selected as that base.

| State | Owned behavior |
| --- | --- |
| EAX | Incoming value is discarded by the first FS load. N0 is then loaded before local allocation and retained through the initial zero-flag branch. |
| ECX | Captured in EBP as R. R+4 supplies header pointers; R+8 is read/written only in the recovered post-free tail. |
| EDX | Entry value can reach the first child unchanged. Subsequent owned semantic uses follow local EDX/DL definitions; later children may see child-produced or preserved values. No transitive independence claim. |
| EBP | Incoming value saved, then replaced by R. Normal restoration is from the current saved slot. |
| ESI | Saved before the initial branch. On the zero-flag path it reaches `006EDA20` unchanged, then is assigned a link-context P before owned semantic use. On the exception path it is zeroed before string setup. |
| EBX / EDI | Saved only at `006EE642/645`, after the initial branch. EBX becomes N0; EDI reaches `006EDA20` as its incoming value, then receives a replacement pointer. |
| BL | At `006EE73A`, only the low byte of EBX becomes 1. The upper 24 bits retain the earlier pointer bits; later owned use is BL, but children receive the whole register. |
| Flags | No incoming condition is consumed. Local arithmetic/CMP/TEST instructions produce branch flags; intervening MOV/PUSH/POP instructions matter where they preserve them. |
| x87 | No owned x87 instruction or environment/status operation; children remain separate contracts. |

Under compatible calls and ordinary frame storage, the zero-flag path has:

| Address | Role |
| --- | --- |
| `S-4` | State, initially `FFFFFFFFh`. |
| `S-8` | Handler address `00C83088h`. |
| `S-0Ch` | Saved FS head; registration installed here. |
| `S-54h..S-0Dh` | 72 reserved local bytes; no blanket initialization. |
| `S-54h` | Saved-node slot Z, explicitly assigned N0 at `006EE64A`. Later reads are current memory. |
| `S-50h..S-35h` | 28-byte temporary string area used only on the nonzero initial-flag path. |
| `S-34h..S-0Dh` | 40-byte exception-storage area used on that path. |
| `S-58h`, `S-5Ch` | Saved EBP and ESI. |
| `S-60h`, `S-64h` | Saved EBX and EDI; steady normal ESP is `S-64h`. |
| `S+4`, `S+8`, `S+0Ch` | O, F and N arguments; the adjacent F/N words are exposed by address. |

The initial byte `[N0+31h]` is read after EH installation and local allocation,
but before EBP/ESI saves. There is no N0-null guard. Any nonzero byte selects the
exception-construction path; zero selects normal link work. The pushes and R
capture between that CMP and JE preserve its flags.

## Nonzero initial flag and EH boundary

The body initializes only temporary-string capacity 15 at S-38h, length zero at
S-3Ch, and first data byte zero at S-4Ch. Its leading word/unused bytes are not
initialized. `00408720` receives ECX=`S-50h`, source address `00CE44E0h` and count
`1Bh`; the admitted Source/ledger contract is `RET 8` and EAX destination. No
outside string bytes were read to validate that source range or its contents.

After assignment returns, the body passes the temporary address to `00411700`,
with ECX=`S-34h`, and sets its own state to the current ESI, initially zero under
compatible preservation. The admitted constructor contract is `RET 4` and EAX
destination. It then pushes address `00D863A8h` and the local exception pointer,
overwrites the exception's first DWORD with `00D6926Ch`, and calls `00BF6885`.
The table/throw-data addresses and local handler were not opened.

No local terminal instruction follows that call: physical fallthrough reaches
`006EE642`. A hypothetical return would need the two pushed words removed and
compatible registers/stack. EAX would then be the child's result, so the later
`MOV EBX,EAX` would not independently recover N0. State also remains zero; no
normal temporary destruction or state reset is inserted. The native throw
contract is unclosed. Neither the entry name nor a metadata library name proves
whole-function no-return or this hypothetical continuation's validity.

On the initial zero-flag path, no further explicit state store follows the
initial -1; absent frame/alias corruption, it remains -1 during rewiring, helper
calls, payload cleanup, free, count update and output publication. Handler
`00C83088` remains unopened; normal restoration does not prove exceptional
cleanup, search/unwind behavior, rollback or second-failure handling.

## Exposed pair and link replacement

Use raw notation `a(X)=[X+0]`, `p(X)=[X+4]`, `b(X)=[X+8]` for DWORD links,
`c(X)=byte[X+30h]`, and `s(X)=byte[X+31h]`. Color/sentinel and transplant/repair
terminology describes observed operations; it does not recover a C++ tree type.
The +31h tests use zero versus any nonzero byte. The +30h tests distinguish exact
0 and exact 1; arbitrary other values must not be normalized to booleans.

At `006EE642..64E`, the body saves EBX/EDI, sets EBX=N0, writes N0 to Z and calls
`006EDA20` with ECX=`S+8`, the address of the adjacent F/N argument words, and
no pushed argument. It ignores the returned EAX by immediately loading `a(N0)`.
The child is unopened; its mutation footprint is not confined to one DWORD.

With X=N0 still retained in EBX:

1. Capture A=`a(X)`. If s(A) is nonzero, capture Y=`b(X)` and use the simple path.
2. Otherwise capture the current `b(X)` in EDX. If its s byte is nonzero, use
   Y=A and the simple path.
3. Otherwise load I from the **current N stack word** at `006EE66D`, compare
   it with retained X, and load Y=`b(I)` before branching. If I equals X, use the
   simple path; if it differs, enter `006EE6DF`.

All eleven block addresses that the pseudocode calls unreachable are present
in the saved listing and the raw zero-flag graph. The late N read can differ
after the child receives the pair address or through aliases. Removing that
branch before proving `006EDA20` would discard encoded behavior. No cause of
the decompiler's simplification is asserted.

On the simple path `006EE678..6DD`:

- Test s(Y), then load P=`p(X)` even when Y's s byte is nonzero. Store `p(Y)=P`
  only for s(Y)==0.
- Load H=`[R+4]`. If `[H+4]==X`, write `[H+4]=Y`; otherwise compare `a(P)`
  with X and write `a(P)=Y` or `b(P)=Y` accordingly.
- Reload H from R+4 before testing `[H]` against the **current Z slot**. When
  equal, s(Y)!=0 selects P; otherwise call `006ED9F0` with ECX=Y and use its EAX.
  Store that value through the retained H pointer, without reloading H after
  the call.
- Reload H again before testing `[H+8]` against current Z. When equal, s(Y)!=0
  selects P; otherwise call `006ED9D0` with ECX=Y. Store through that retained
  H+8, again without a post-child header reload. The helper names/results are
  not independently classified as minimum/maximum traversal here.

On the I!=X path `006EE6DF..733`, preserve the exact store/reload order:

1. Write `p(A)=I` using the earlier A, then reload `a(X)` and store it to `a(I)`.
   Compare I with the current `b(X)` after that write.
2. If equal, set P=I. Otherwise test s(Y), load P=`p(I)`, optionally write
   `p(Y)=P` for s(Y)==0, then write `a(P)=Y`.
3. In that unequal subpath, reload `b(X)` and write `b(I)`; reload `b(X)` **again
   after the write**, then store that new pointer's +4 field to I.
4. Load H=`[R+4]`. Replace `[H+4]` with I if it equals X; otherwise load `p(X)`
   and select its +0 or +8 link by comparison with X, then store I there.
5. Reload `p(X)` after those stores and assign `p(I)`. Capture c(X), then c(I),
   and store the captured c(X) to c(I) followed by captured c(I) to c(X).

Raw aliases can make the repeated loads observe preceding stores. Caching an
earlier X link or header, or replacing the byte exchange with reordered reads,
would change the observed schedule. The I!=X path skips the simple path's header
endpoint updates and joins directly at `006EE736`.

## Byte-field repair loop

At `006EE736`, load current Z, set BL=1, and compare `c(Z)` with 1. If unequal,
jump directly to payload cleanup, bypassing even the final c(Y)=1 store. If equal,
compare Y with the root word of a freshly loaded H=`[R+4]`; equality goes to that
final store. Otherwise the loop begins at `006EE751` and continues only while
c(Y)==1. There is no loop-depth, cycle or structural-validity proof.

When Y equals `a(P)`, the encoded half uses sibling W=`b(P)`:

- If c(W)==0, store c(W)=1 and c(P)=0, call `006EDC80` with ECX=R and pushed P,
  then reload W=`b(P)`.
- If s(W)!=0, ascend. Otherwise read W's +0 child and compare its c byte with
  1; only when needed read W's +8 child. Both exact-1 bytes select c(W)=0 and
  ascent. The subsequent far-child decision reloads W+8 instead of retaining
  the earlier pointer.
- If that far-child c byte is 1, store 1 through the captured near-child pointer,
  store c(W)=0, call `006EDCD0` with ECX=R and pushed W, then reload W=`b(P)`.
- Copy current c(P) to c(W), set c(P)=1, load the current W+8 child, set its
  c byte to 1, call `006EDC80` with pushed P, and leave the loop.

When Y differs from `a(P)`, W is the already loaded `a(P)` and the other half
mirrors the offsets explicitly: an initial c(W)==0 calls `006EDCD0` on P and
reloads W=`a(P)`; the first child tested is W+8, then W+0; a far-child c byte of
1 calls `006EDC80` on W and reloads `a(P)`; the final stores use W+0 and call
`006EDCD0` on P. The report/raw decode preserve each individual instruction and
call site. This description does not prove what the two helpers do internally.

Ascent at `006EE7F3..804` reloads H=`[R+4]`, copies old P into Y, compares Y
with `[H+4]`, then loads `P=p(old P)` **before** the JNE backedge. The parent-link
read therefore still happens when the comparison will exit. MOV preserves the
comparison flags. The single backedge is `006EE7FE -> 006EE751`.

Every repair exit reaching `006EE834` stores c(Y)=1. Six helper call sites pass
one pushed pointer and ECX=R, with no caller argument cleanup. Their callers
require four-byte callee removal and preservation of live R/P/Y/BL context;
the unopened 78-/82-byte child bodies do not yet prove those contracts.

## Retirement, count and exact output schedule

At `006EE837`, load the **current Z slot**, add `0Ch`, and call `006EE040` with
that ECX and no pushed argument. After return, reload Z again at `006EE843`,
push that current pointer and call `_free`. The payload receiver and eventual
free argument can differ if a child or alias changes the saved slot. No cached
original-node substitution is justified by these instructions.

The separately admitted continuation begins immediately after the call:

| Site | Exact effect |
| --- | --- |
| `006EE84D` | Read current DWORD `[R+8]` **after free returns and before cleaning its argument**. |
| `006EE850` | `ADD ESP,4`; then TEST the loaded count, POP EDI and POP EBX. |
| `006EE857` | JBE consumes TEST's flags: CF is zero, so only a zero DWORD skips decrement. |
| `006EE859..85C` | Nonzero count becomes count-1 and is stored to R+8, before output-argument loads. |
| `006EE85F` | Load current F from S+8 into ECX. |
| `006EE863` | Load current O from S+4 into EAX. |
| `006EE867` | Load current N from S+0Ch into EDX. |
| `006EE86B` | Store captured F to `[O]`. |
| `006EE86D..871` | Load current saved FS head at S-0Ch, then POP ESI. |
| `006EE872` | Store captured N to `[O+4]`. |
| `006EE875..880` | POP EBP, restore captured FS head, discard 54h bytes, `RET 0Ch`. |

Both output values and O are captured before the first output store. The FS
word is loaded **after** that store; ESI is restored before the second store and
EBP afterward. Aliases can therefore affect frame restoration differently.
The count write precedes all three argument loads and can affect them through
aliases. EDI/EBX have already been restored before the conditional count write.

Under compatible children and current saved slots, final ESP is S+10h: return
address plus 12 argument bytes consumed. EAX remains the late-loaded O. Exactly
two output DWORDs are written; no prior output read, null-output guard or
disjointness check is encoded. This closes the parent's local eight-byte output
and callee-cleanup requirements as recovered-continuation evidence, not a drop-in
Native ABI result. The parent separately reloads its own F/N after this call.

The selected direct receiver extent reaches R+0Bh and the raw node/link extent
reaches byte +31h. These are minimum 12-/50-byte address spans, not full class or
allocation sizes. The payload child receives Z+0Ch with an unknown footprint.
No node membership, link symmetry, acyclicity, allocator matching or general
null/extent/overlap guarantee follows from the pointer comparisons.

Failures can occur after graph mutations, byte changes, payload retirement or
free, including the late count/output/frame reads. No transaction, rollback,
leak freedom, repeat safety or safe arbitrary alias graph is established.

## Current Source, receipts and next bounded children

Exact searches find no current Source or reconstruction admission for this
entry or its six non-library children. All Native children remain unopened.

| Child | Metadata bytes / instructions / calls | Current status |
| --- | --- | --- |
| `006ED9D0` | 28 / 10 / 0 | ECX=Y, no pushed arguments; EAX stored through retained header+8. |
| `006ED9F0` | 27 / 10 / 0 | ECX=Y, no pushed arguments; EAX stored through retained header+0. |
| `006EDA20` | 99 / 38 / 2 | First priority: ECX points to adjacent F/N argument words; late N controls the transplant path. |
| `006EDC80` | 78 / 30 / 0 | Three call sites, ECX=R plus one pushed pointer; exact helper footprint/cleanup unproved. |
| `006EDCD0` | 82 / 30 / 0 | Three mirrored call sites with the same caller-required boundary. |
| `006EE040` | 98 / 29 / 3 | ECX=current saved node+0Ch, no pushed arguments; metadata references the admitted readiness cycle and pool services. |
| `00408720` | 193 / 88 / 4 | Current raw SBO Source: alias checks precede maximum length, conditional growth, secure copy and terminator; explicit host CRT/EH boundary. |
| `00411700` | 95 / 28 / 2 | Current actual-storage Source initializes base/member fields and uses an armed base-cleanup guard around member assignment. |
| `00BF65AC` | 5 / 1 / 0 | Admitted typed host `std::free` service; no new Original binding proof. |
| `00BF6885` | 74 / 29 / 0 | Native throw boundary stays unimplemented here; metadata naming/decoration does not prove a returning-path cleanup or native exception interoperability. |

The six small unresolved bodies each remain below 300 bytes, but need separate
leases and full-body packets. The 99-byte pair helper is the immediate ABI and
progress dependency. No source wrapper/stub or rotation/iterator semantics were
invented to close it.

All 64 inherited input pins replay exactly and are unchanged at this worker
baseline. Earlier BF00 shard drift remains recorded historically; it is not
reintroduced as current drift. Current Source/header/report files for the new
string/exception children are separately pinned and actually inspected.

Twelve Source artifact hashes from the two child receipts replay in explicit
canonical or CRLF domains; six recorded entries differ from current full files.
The original exception-owner artifact uses try/catch, while current construction
uses `BaseUnwindCleanup`. Its current 703-byte constructor text matches the
constructor text in the historical Source file reproduced by the later
`cleanup_search_correction` pin; that whole Source file has since changed.
It differs from the original 738-byte constructor slice. Counted assignment and
base-construction slices are unchanged.
The later SBO integration records the raw-storage header correction. These
distinct historical receipts are preserved; no old fixture, aggregate hash or
Source match becomes a fresh Original ABI execution claim.

Root's separately evolving checkout was not read. All current file hashes name
this worker's fixed baseline; Root must qualify any later integration drift.
There are no Source/CMake/ledger/Ghidra changes, builds, tests, probes, original
execution, gameplay validation or reconstruction credit in this packet.
