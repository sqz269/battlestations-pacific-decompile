# Input int/string tree range erasure

Whole `006A1B70` has 200 defined bytes and 76 instructions in two ranges:
`006A1B70..006A1BEE` (127 bytes) and `006A1BF0..006A1C38` (73 bytes).
The 201-byte envelope includes the outside NOP `006A1BEF`, which remains
undefined and unpromoted. The whole full and partial paths are reconstructed.
Names describe a provisional STL instantiation, not recovered symbols.

Root published the accepted eraser at Main
`aaa3e4b03dbc0084d9ebad5d4a42ada2517bc491` before Source authorization.
The worker synchronized to published descendant
`ce76166caeee3f71479775408198aad9febbb33a`. The accepted eraser Source and
metadata pins are `d56a08ef10ee2eab7e75e7b6b82c40e13068be19` and
`7d1316db57065ab71c596c9bd1b502d2ba0dcbb3`; the fixed nil19 leaves retain
pin `676bce181ecc723b2bb9965a963a5f9f747b443f`. All 36 readiness provider
Source/header fingerprints and the frozen parent Native/COFF/build receipts
remain unchanged. Readiness SHA256 is
`660f14f7705bdc0991e4dbce96f175b5351421d066b9b44f9755b809e3517f2a`.

## Storage and ABI

Native ECX retains the receiver, EDX is unused, and five stacked words are
output, first.owner/node and last.owner/node. EAX returns output; both exits
use `RET 14h`. Source uses the existing real trivial 8-byte
`NativeInputIntStringTreeIterator` for both by-value arguments and adds the
borrowed actual `NativeStringRawPoolContext&`, giving `RET 18h`.
Tree words remain opaque0/head4/count8. The same fixed nil19 actual subtree,
iterator and eraser providers are used directly. There is no host node,
container, callback, generic range/Kind/direction/context bridge or prefix
shortcut. Existing populated AV/typed/RawCtx domains remain unchanged.

## Ordered behavior

| Phase | Established schedule |
| --- | --- |
| Entry | Capture first.owner before current head and head.left. Read both head/begin values before the returning first-owner diagnostic. Capture current first.node only afterward. |
| Full candidate | If cached first.node equals retained begin: capture current last.owner, then current tree.head, before the last-owner diagnostic. Read current last.node afterward and compare it with the retained head. Returning diagnostics do not refresh captures. |
| Full destruction | Reload current head/root and invoke the published actual subtree with the same receiver and RawCtx. There is no count/nil shortcut or head free. |
| Full reset | Current headA.root=headA; capture current headB before count=0, then headB.left=headB; capture current headC, then headC.right=headC. Preserve all captures and the count-before-left store order. |
| Full output | Capture current head.left result before current output pointer; store receiver owner, then captured result node, return output. |
| Partial diagnostic | Test cached first.owner for zero before any last.owner read; otherwise compare it with current last.owner. After a returning diagnostic compare cached first.node with current last.node. Equality is considered only after reached diagnostics/full-candidate checks. |
| Partial iteration | Form OLD from cached owner/node; advance the actual private FIRST argument through the accepted iterator; invoke the accepted eraser with OLD by value, an ignored real private 8-byte output, and the same RawCtx. The eraser advances its distinct private OLD copy again. |
| Partial reload | After successful erasure reload current first.node before current first.owner, then repeat. Diagnostic return does not repair the cached values. |
| Partial output | Capture current output pointer; cached owner and node already precede both stores. Store owner then node and return captured output. |

All reached public loads/stores are raw volatile accesses. Source preserves
the separate origins of the range's private FIRST argument, OLD child input,
ignored child output and recursive subtree node. It does not add an empty
range shortcut, iterator repair, owner/output guard, local EH or rollback.

A full-path RawCtx getter exception preserves completed subtree destruction
and occurs before range header resets/output. A partial-path iterator or
eraser exception preserves prior iterations and the completed FIRST advance.
The eraser's pool failure also preserves current topology/color/pool effects,
before current-node free/count/output. No range guard retries cleanup.
Existing current-SDK returning diagnostics and CurrentCpp/CRT boundaries apply.

These six transfers are actual Native instructions belonging to the owned
whole range. The child entries are accepted support and gain no new Source
scope in this packet.

| Native transfer | Purpose |
| --- | --- |
| `006A1B8A CALL 00BF6713` | Initial owner diagnostic |
| `006A1BA6 CALL 00BF6713` | Full-candidate last-owner diagnostic |
| `006A1BBA CALL 006A0AA0` | Actual subtree destruction |
| `006A1BFA CALL 00BF6713` | Partial-loop owner diagnostic |
| `006A1C09 CALL 0069AE50` | Actual private FIRST advance |
| `006A1C17 CALL 006A07D0` | Actual eraser on OLD by-value input |

## Verification and limits

Fresh Source-phase queries verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Installed PE/live/readiness bytes agree for the
entire 201-byte envelope. All 76 defined starts, six decoded rel32 targets and
four callee entries were checked: 80 exact starts plus the expected undefined
NOP query. Typed body metadata retains two complete ranges, 200 defined bytes,
and no no-return/thunk flag. Canonical JSON rows and this table are checked by
the existing report verifier.

Default `./scripts/build.ps1` passed MSVC Win32 Release, exit 0. All three
existing checks passed (reconstructed_math, native_math_differential,
tool_tests), 15.79 seconds total CTest time. No new tests or probes were added.
These checks do not execute this range against Native game state.

The entire actual CMake object was reviewed: 267 executable bytes, 115
instructions and six relocations. Its public range is 220 bytes/90 physical
instructions, including one generated 2-byte NOP; both exits use `RET 18h`.
EBP remains the compiler frame. EDI retains receiver, ESI/EBX retain old
owner/node across the private FIRST advance, the ignored child output is
`[EBP-0Ch]`, and first.node reload precedes first.owner reload. All six
relocations resolve the accepted SDK/subtree/iterator/eraser entries. No local
EH/RTTI/unwind data or inline assembly is introduced.

Source/build/static proof does not establish Original incidental registers or
flags, private-frame aliases, heap/CRT/FH3/SEH/RTTI/fault delivery, Original
binary ABI, packet differential execution, application adoption or game
validation. The added-context Source ABI and accepted CurrentCpp/noexcept
boundaries remain qualified. No Ghidra mutation or publication occurred in
this worker. Pair/owner/destructor entries remain outside this Source packet.
