# Actual scene integer-set single-iterator erase

Owned addresses: `0046A740`, `00C61C90`.

The whole scalar-key erase body uses actual tree, node, caller-argument and saved
node storage. Existing concrete integer extrema/increment/rotation services,
current CRT free and the actual owning legacy exception carrier are composed.
No host callback, stub, replacement STL owner, pool/string/key destruction,
rollback or success-only restriction is added. The misleading provisional name
`STL_xlen_throw_0046a740` is replaced by the descriptive behavior-derived hypothesis
`STL_SceneIntegerSet_EraseIterator_0046A740`; native symbol identity is unproved.
Correct compiler unwind naming and all old values/comments/evidence are retained.

| Entry | Inclusive body | Bytes / instructions | Original ABI / Source coverage |
| --- | --- | --- | --- |
| `0046A740` | `0046A740..0046A9E6` | 679 / 231 | whole; ECX tree; stack output,owner,node; RET0Ch/EAX current output |
| `00C61C90` | `00C61C90..00C61C97` | 8 / 2 | whole completed-message Source unwind; native LEA ECX,[frame-50h],tail `004072D0`; no independent Source funclet entry |

Total coverage is687 native bytes/233 instructions. Normal native body has61
blocks and13 physical call sites. Incoming EDX is unused. The Source interface
adds explicit frame backing and is not the original register/stack ABI.

## Actual frame and current cells

Tree head/count are DWORD `+4/+8`. The actual `14h` nodes have DWORD
left/parent/right `+0/+4/+8`, scalar key `+C`, color BYTE `+10`, nil BYTE `+11`.
The body never reads key or trailing padding `+12/+13`. Color1 is black and0 red;
other byte values retain the original comparisons, without normalization.

`NativeSceneIntegerSetEraseFrame` exposes actual twelve-byte stacked parameters
`{output,owner,node}` plus a separate saved-node DWORD. At native entry their
offsets are `+4/+8/+C` and `-54h`. Stable binding metadata is disjoint from all
described cells/payload. Actual backing may overlap when every reached access and
release is valid. Saved-node backing is required only after the nonnil path;
an initially nil argument does not access it. Source adds no implicit alias to
compiler/native frames, argument homes, return address, EH records or registers.

Capture the initial node and its nil flag first. Initial nil throws the complete
owning error before saved-node publication or increment. Otherwise publish the
captured original into saved-node, then increment the actual parameter owner/node
cells. The child retains all current CRT report/return paths; its return is ignored.
A returning handler can change the actual cells while the captured original and
captured tree remain stable.

After increment, capture original.left. Nil left selects current original.right;
nil right selects captured left; with two nonnil children reread CURRENT input
node as successor and its right link before comparing successor with original.
The successor==original branch remains. Simple removal checks replacement.nil
before original.parent, conditionally repairs replacement.parent, and relinks the
original root/parent with current tree head. For minimum, load current head.min
BEFORE reading CURRENT saved-node for comparison. For maximum, reload head, then
read CURRENT saved-node BEFORE head.max. Capture each head for its final store.

Transplant first writes captured original.left.parent, rereads original.left and
writes successor.left. Preserve the direct-right branch, otherwise read replacement
nil before successor.parent; unlink it, reread original.right separately for the
successor link and right-child parent repair. Root linking uses captured tree.
Reread original.parent, then load both colors before storing successor color and
original color in that order. No saved-node snapshot replaces any later read.

## Repair, current free and publication

After splice reread CURRENT saved-node and test THAT node's color. If1, compare
replacement with current root before its color check. Retain every red sibling,
nil sibling, both-black, near/far rotation and mirrored branch. Near-child pointers
are captured before far-child color reads and reused for the near recolor; they
are not reread from the sibling. Reload sibling after nonfinal rotations, using
the captured tree for each operation. Ordinary ascent reads current head/root,
captures the comparison inputs, and still reads parent.parent when root matched.
The black-path final replacement.color=1 store is retained, including sentinel.

Read CURRENT saved-node AGAIN only after repair and pass it directly to the real
current CRT free. This is distinct from the original splice pointer and the color
test's saved-cell value. Explicit aliases can select a different valid release
target; Source adds no guard forcing all three values to agree. No key/name
cleanup, pool getter or header reset is present in the native body or this Source.

Only after free returns, read the current unsigned count of the captured tree and
decrement iff nonzero. Then read CURRENT owner, CURRENT output pointer, CURRENT
node from actual parameters, before either output store. Publish owner then node
and return that captured output pointer. Current count and argument alias effects
are preserved. No extra rollback, node cleanup or exception conversion is supplied.

## Owning error and native EH evidence

The exact27-byte message `invalid map/set<T> iterator` is at `00CE44E0`.
Initialize the raw SBO temporary capacity15,length0,first inline byte0, then
assign through `00408720` while parent state is -1. Arm state0 only after success,
before `00411700`. Existing `NativeHardwareLayoutInvalidIterator` owns actual28h
legacy storage: raw logic-error construction then profile `00D6926C`, copy through
`00441760`, destruction through `004412B0` and the actual member/base services.

Handler `00C61C98` selects FuncInfo `00D893B0`: magic19930522,max state1, unwind
map `00D893A8`, no try/catch map, EH flags1. State0 transitions to -1 through
`00C61C90`'s completed-temporary destruction at native frame-50h. Failed message
assignment has no parent guard cleanup; failed logic-error construction retains
its established base cleanup. Neither entry claims a native private-frame ABI.

The verified shared ThrowInfo `00D863A8` has attributes0,destructor `004412B0`,
no compatibility callback and three catchables at `00D863E4`: out_of_range
`00D863F4` (type `00E0817C`,28h,copy `00441760`),logic_error `00D84000`
(type `00E08020`,28h,copy `004118D0`),exception `00E03C7C` (type `00E174F0`,
Ch,copy `00BF63A6`). Properties0 and PMD `{0,-1,0}` apply to each. The Source
throws the actual owning carrier with current C++ transport. Native RTTI/base
catch inheritance/FH3/SEH/CRT global identity and fault addresses are qualified,
without a runtime-error shortcut or fake native descriptor implementation.

## Caller and validation

The one live parent is range erase `0046B170`, with transfer `0046B217`.
It advances its live outer iterator first, then publishes captured old node,
captured old owner and actual private output address into child arguments in
that order. After the child's RET0Ch it reloads current outer node then owner.
This body and the Source's explicit private-cell contract were reviewed in full
by the primary integrator, including retained near-child values and current free.

Full assembly/proto/caller/real provider and EH contracts were
reviewed. All687 live bytes/233 instructions match the installed PE;15 typed native
transfers pass,with26 Markdown addresses and no failed/unchecked claims. Unique
CMake registration was appended only after an immediate unleased check,without
an exclusive file lease. Locked annotation retained prior values/comments/evidence,
corrected the misleading provisional label and saved the project; both exports
were forced and expected-name/comment readback passed. Its prior-value log is
`local/ghidra-annotations-20261015T213102Z.json`.

Actual extrema/increment Source drafts were merged at
`c46e1bedbff10428b00f5c1b947ccbadc7736f03`. Required `scripts/build.ps1` passed
strict Win32 and all 3 existing CTests in 8.36s; log
`local/goal_oct15_scene_integer_set_erase_build.log`. No new tests were added.
Source/header hashes remain identical to draft
`cd9d10a6d5a84545a9229368e81fbe7851381329`, independently reviewed with the complete
native listing and EH scope by the primary integrator, without a mismatch.

Focused MSVC object inspection confirms the reached nil guard precedes saved-node
publication and increment. In the actual 694-byte entry, minimum reads head.min at
`+8F` before the current saved-node value at `+91`; maximum reads the current
saved-node at `+B4` before head.max at `+B9`. The color target is reloaded at `+14B`;
near-child captures at `+1AA/+21B` survive far-child tests until recolors at
`+1C6/+24F`. After repair, another current saved-node read at`+283` supplies the
real free at `+286`, preceding current count at `+28B`. Current owner/output/node
loads at `+29F/+2A2/+2A4` precede output stores at `+2A7/+2A9`. The actual 11 REL32
entry relocations call the accepted concrete children/free and local owning-error
helper. The helper's real assignment at `+42`, guard arming at `+54`, owning carrier
construction at `+5B` and current C++ throw at `+69` retain a generated guard unwind
through the actual completed-message destructor. Object receipt and offsets are
recorded in the report and `local/goal_oct15_scene_integer_set_erase_object.txt`.
These are current compiler checks; native frame/register/COFF/EH identity is not
claimed. Packet-specific native differential/game validation remains unrun.
