# Input int/string tree iterator erasure

This packet reconstructs whole `006A07D0..006A0A9B` (716 bytes, 241 current
instruction starts) and the logical completed-message action `00C7F050..57`
(8 bytes, two starts). `00C7F058..61` and `00DAC05C..87` are read-only EH
support. Names describe a provisional STL instantiation, not recovered symbols.

The baseline is published Main `86da4661bd4f66e75379091d08ad6371a873088e`.
The five fixed nil19 leaves match accepted/published Source pin
`676bce181ecc723b2bb9965a963a5f9f747b443f`. All 32 accepted provider Source
fingerprints and the three frozen readiness/control artifacts were unchanged.
The handoff SHA256 is
`a36be77029d7c6fefe590dcdced6d352d890b814c0db53648388fae77bb3b5cb`;
readiness SHA256 is
`c2909630344ffd06fd27d912ff4c20312019c7b8c39e50ef9202dcc35971255d`.
Root's explicit packet brief superseded the handoff's pending authorization.

## Physical storage and interfaces

Native ECX holds the tree, EDX is unconsumed, and stacked arguments are output,
input owner and input node. EAX returns the output; the defined final instruction
at `006A0A99` is `RET 0Ch`. Ghidra's `undefined(void)` prototype is incomplete.
The new fastcall Source retains the 8-byte by-value iterator and adds the actual
borrowed `NativeStringRawPoolContext&`, so its compiled return is `RET 10h`.

Tree words are opaque0/head4/count8. Node links occupy 0/4/8, mapped pooled
string length/data 10h/14h, and byte color/nil 18h/19h. The Source uses direct
volatile DWORD/BYTE accesses and a real trivial 8-byte checked iterator. It
does not project nodes into a host container or route to nil15/99 providers.

## Ordered reconstruction

| Native span | Established behavior |
| --- | --- |
| `006A07E8..0841` | Capture input node and test nil19 before owner diagnostics. Nil builds a default-no-init real 1Ch SBO string, initializes only capacity15/length0/NUL, and assigns exactly 27 literal bytes through 408720. |
| `006A0846..0877` | Retain the original removed node separately, then invoke accepted 69AE50 on the actual private by-value iterator. With both children real, select the CURRENT advanced node and its right child, including the advanced-node-equals-original side case. |
| `006A0879..08DA` | Test replacement nil before reading removed.parent; perform root/left/right transplant, capture current heads, and update extrema with the accepted minimum/maximum entries. Reload the saved receiver before the maximum phase. |
| `006A08DC..0931` | Publish the retained first left child's parent, reread removed.left, test successor against CURRENT removed.right, and retain each parent/right reread and color capture before the two ordered color stores. |
| `006A0934..0A3E` | Execute both direction-specific black fixups. Keep short-circuit child/color order, nil sibling branch and all six accepted rotations. Discard every incidental rotation EAX. Move-up reads current head/root before parent.parent, including the final root iteration. |
| `006A0A41..0A5B` | Release the original node's mapped pooled string only after topology/colors are changed. |
| `006A0A5C..0A75` | Free the ORIGINAL removed node through the accepted current-CRT wrapper; reload receiver and CURRENT unsigned count, decrementing every nonzero DWORD, including high-bit counts. JBE after TEST skips only zero. |
| `006A0A78..0A9B` | Capture CURRENT private owner, output pointer, and CURRENT advanced private node before the first output store; store owner then node, return output. |

The normal inline payload is proved equivalent here to accepted
`destroy_native_string_header_0041dd20` on the header at removed+10h and RawCtx:
capture data14
once; if nonnull read current length10, wrap `+1`, resolve actual 419CC0 getter,
then return the captured block through BD1510. No header write or intervening
left-child capture occurs in this span. This is Source composition of the inline
effect. No Native 41DD20 transfer exists in the parent. Getter failure propagates after
transplant/rebalance and before original-node free, count and output. There is
no normal key cleanup guard, rollback or retry, including on getter failure.

The RawCtx borrows actual 01090AA8 pool, 01090AA4 small-return gate and 01090AA0
manager publication. It uses the existing actual pool/getter/CRT services.
Existing RawCtx, AV, typed and subtree providers remain unchanged.

The following are actual Native direct transfers. Read-only support rows retain
accepted helper or EH-handler evidence; Source scope remains the whole eraser
and the logical completed-message action.

| Scope | Function | Native transfer |
| --- | --- | --- |
| Read-only support | `0069AE50` | `0069AE58 CALL 00BF6713` |
| Read-only support | `0069AE50` | `0069AE67 JMP 00BF6713` |
| Owned whole eraser | `006A07D0` | `006A0818 CALL 00408720` |
| Owned whole eraser | `006A07D0` | `006A082A CALL 00411700` |
| Owned whole eraser | `006A07D0` | `006A0841 CALL 00BF6885` |
| Owned whole eraser | `006A07D0` | `006A084E CALL 0069AE50` |
| Owned whole eraser | `006A07D0` | `006A08B0 CALL 0069AAE0` |
| Owned whole eraser | `006A07D0` | `006A08D2 CALL 0069AAC0` |
| Owned whole eraser | `006A07D0` | `006A0972 CALL 0069C300` |
| Owned whole eraser | `006A07D0` | `006A09A5 CALL 0069A780` |
| Owned whole eraser | `006A07D0` | `006A09C1 CALL 0069C300` |
| Owned whole eraser | `006A07D0` | `006A09D6 CALL 0069A780` |
| Owned whole eraser | `006A07D0` | `006A0A1F CALL 0069C300` |
| Owned whole eraser | `006A07D0` | `006A0A39 CALL 0069A780` |
| Owned whole eraser | `006A07D0` | `006A0A50 CALL 00419CC0` |
| Owned whole eraser | `006A07D0` | `006A0A57 CALL 00BD1510` |
| Owned whole eraser | `006A07D0` | `006A0A5D CALL 00BF65AC` |
| Owned logical action | `00C7F050` | `00C7F053 JMP 004072D0` |
| Read-only support | `00C7F058` | `00C7F05D JMP 00BF6B43` |
| Read-only support | `0041DD20` | `0041DD30 CALL 00419CC0` |
| Read-only support | `0041DD20` | `0041DD37 CALL 00BD1510` |

## Completed-message cleanup and exception boundary

Native state0 is armed at `006A0826`, AFTER counted assignment succeeds and
BEFORE 411700 exception construction. The Source creates its completed-message
guard at that same semantic boundary and throws the accepted owning
`NativeHardwareLayoutInvalidIterator`: actual 411700 storage construction,
D6926C profile, 441760 copy ownership and 4412B0 destruction.

Native action C7F050 computes runtime EBP-50h and tails 4072D0. Its Source entry
instead takes the completed SBO storage directly and calls accepted 4072D0.
It has no EBP reader, raw-pool context or mapped-key cleanup. The original
unwind map DAC05C is `(-1,C7F050)`; FuncInfo DAC064 words are
`19930522,1,DAC05C,0,0,0,0,0,1`. The handler and original D863A8 throw type
remain evidence. The actual compiled Source nil helper has one completed-message
state and no try/catch map. The normal eraser has no local EH cleanup state.

## Verification and limits

Fresh read-only queries verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` for each batch. Full installed-PE/live/frozen
comparisons cover 1,074 code bytes, 375 decoded starts and 21 rel32 transfers;
384 exact current starts include the transfer targets. Another 104 bytes cover
the EH map/FuncInfo, original throw/catch-array data and literal. The eraser's
716-byte SHA256 is
`fe0f7051ceb2958cc33c758968f9912f5682f7616631d413b9d845e0d8b2aa12`.

Default `./scripts/build.ps1` passed MSVC Win32 Release with exit 0. All three
existing checks passed: reconstructed_math, native_math_differential and
tool_tests (13.97 seconds total CTest time). No new test or probe was added.
These existing checks do not execute this eraser against Native game state.

All 1,026 executable bytes/384 instructions and 35 relocations in the actual
CMake object were reviewed, including guard/throw/RTTI/EH support. Its eraser
is 762 bytes/280 instructions, uses compiler-frame EBP throughout, retains
removed in EBX, advances `[EBP+0Ch]`, and returns `RET 10h`. There is no inline
assembly. The object SHA256 and full decoded review are frozen in the report.
These are physical generated counts: the main includes a trailing INT3, the
111-byte/32-instruction throw helper includes an INT3, and the 42-byte/16-start
compiler EH section includes five INT3 and two NOP padding instructions. In the
generated move-up, root load `+25Dh` precedes parent.parent read `+260h`, then
register CMP `+263h`; Native CMP/root read `006A09FF` precedes parent.parent read
`006A0A02`. The public volatile load order is retained; exact instruction,
private flags and fault/runtime equivalence remain unproved.

Source ordering/build/static evidence is not Original incidental-register or
flags identity, private-frame alias equivalence, Original heap/CRT/FH3/SEH/RTTI
or fault delivery, drop-in binary ABI, packet differential execution, application
adoption or game validation. SBO/current-CRT noexcept behavior and the accepted
CurrentCpp owning exception transport retain their existing limits. No Ghidra
mutation occurred. The following 6A1B70 range, 6A2BB0 pair, C7F200 and 6AA460
owner entries remain outside this Source packet.
