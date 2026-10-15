# Actual scene-record map single-iterator erase

Owned addresses: `0046A470`, `00C61C70`.

The whole erase entry and its temporary-unwind scope operate on actual map,
node, caller-parameter and saved-map storage. Existing concrete providers supply
increment, extrema, rotations, raw string pool return, current CRT free, and the
owning legacy exception payload. No callback, stub, STL replacement, runtime-error
shortcut, rollback, or success-only admission is added. The retained Ghidra name
`STL_xlen_throw_0046a470` is misleading: this body performs complete erase and
throws only for an initially nil input node. Original library identity is unproved.

| Entry | Inclusive body | Bytes / instructions | Coverage / original ABI |
| --- | --- | --- | --- |
| `0046A470` | `0046A470..0046A73B` | 716 / 241 | whole function; ECX map; stack output, iterator owner, iterator node; EAX current output; RET0Ch |
| `00C61C70` | `00C61C70..00C61C77` | 8 / 2 | whole temporary-unwind Source scope; native LEA ECX,[frame-50h], JMP `004072D0`; no independent Source funclet entry |

This covers 724 native bytes and 243 instructions. Source adds an explicit frame
and raw-pool context; its new ordinary C++ signature is not the original ABI.
The one live parent is range erase `0046B570`, whose transfer at `0046B617`
pushes node, owner, and output address, with ECX map. Child `RET0Ch` consumes
exactly three DWORDs. The parent reloads its actual loop iterator after return.

## Live storage and splice

Map head/count are DWORDs at `+4/+8`. Nodes have DWORD left/parent/right at
`+0/+4/+8`, raw name length/data at `+C/+10`, unowned mapped DWORD `+14`, color
BYTE `+18`, and nil BYTE `+19`. Black is exactly 1, red exactly 0; no new color
normalization or tree validation is introduced.

`NativeSceneRecordMapEraseFrame` supplies actual twelve-byte stacked parameter
backing `{output,owner,node}` and one live saved-map DWORD. Their addresses and
the binding metadata remain stable; metadata is disjoint from all described
cells/payload. Actual parameter, saved-map and payload storage may overlap where
every reached access/release remains valid. Source provides no implicit alias to
its own compiler frame, argument homes, EH records, return address or registers.

The native entry captures input node at entry+Ch and reads nil BEFORE publishing
the incoming map into saved-map entry-54h. Source retains that order. Initial nil
throws before increment or tree repair, even when owner is zero. Otherwise the
captured original node remains the node ultimately freed. Increment receives the
actual parameter owner/node cells, including the complete returning current CRT
invalid-owner path. Its result is ignored; its writes and handler effects remain.

After increment, capture original.left. A nil left selects current original.right;
a nil right selects captured left. With two nonnil children, reread CURRENT
parameter.node, then its right link, before comparing it with the original.
Thus a returning handler's parameter changes affect successor selection without
replacing the captured original node. The branch where successor still equals
original is retained.

Simple removal checks replacement.nil BEFORE reading original.parent; repairs
replacement.parent if nonnil, then links the original parent/root. It reloads head
from the captured initial map for the minimum update. Only afterward it reloads
the CURRENT saved-map cell and that map's head for maximum update. Min/max return
the selected node, including their real starting-sentinel/child behavior.

Transplant writes captured original.left.parent first, rereads original.left,
then writes successor.left. Its direct-right case selects successor as fixup
parent. The other case checks replacement.nil before reading successor.parent,
unlinks it, and separately rereads original.right for successor.right and the
right child's parent repair. Root/parent linking still uses the captured initial
map. It rereads original.parent before storing successor.parent, reads both
colors before either color store, then swaps successor/original colors in order.

## Repair, release and output

Only original.color==1 enters repair. Capture CURRENT saved map, read its head/root,
then compare replacement to root before reading replacement.color. Each iteration
reads parent.left once to choose the mirrored case. Red-sibling conversion, both
black children, near rotation, far rotation and nil-sibling ascent all retain the
native order of volatile child/nil/color accesses and color/link stores.

After each nonfinal rotation, reread the sibling link FIRST, then reload CURRENT
saved map. Ordinary iterations keep that captured map for the next head/root read;
they do not introduce an extra saved-map reload. Ascent captures the root comparison
before reading parent.parent, and still reads parent.parent when the root matched.
The final replacement color store is unconditional within the black-original path,
including when replacement is sentinel. Every original rotation branch remains.

After repair, capture original.name.data; null skips the length read and all pool
work. Otherwise capture length+1 with DWORD wrap, resolve the real raw pool through
`00419CC0`, then return captured pointer/size through `00BD1510`. Getter exceptions
escape; native state remains -1, so no new node free, count update, output store,
or rollback follows them. Small disabled/large return behavior belongs to the
existing concrete pool service. No name reset or mapped-value disposal is added.

Free the captured original allocation through current CRT `singleton_lifetime_free`.
Only after free returns, reload CURRENT saved map, read current unsigned count,
and decrement iff nonzero. Finally read CURRENT owner, CURRENT output pointer,
then CURRENT node from parameter backing, in precisely that order before either
output store. Publish output.owner then output.node and return the captured output
pointer. Count/free/parameter aliases retain these current-cell distinctions.

## Invalid iterator ownership and native EH evidence

The original counted message is exactly 27 bytes, `invalid map/set<T> iterator`,
at `00CE44E0`. Initialize temporary capacity=15, length=0, first inline byte=0;
assign through complete actual `00408720` while parent state is -1. Only after
successful assignment arm state0, then construct the owning `28h` logic-error
payload through `00411700`, and publish verified out_of_range profile `00D6926C`.

Handler `00C61C78` selects FuncInfo `00D89384`: magic `19930522`, max state1,
unwind map `00D8937C`, no try/catch map, EH flags1. Its single state0 entry
transitions to -1 through `00C61C70`, which destroys the completed temporary
at native frame-50h through actual `004072D0`. Assignment failure has no parent
cleanup. Logic-error failed construction keeps its own established base cleanup.

Throw descriptor `00D863A8` has attributes0, destructor `004412B0`, no forward
compatibility callback, and catchable array `00D863E4` with three entries:

| Catchable | Native type descriptor | Bytes | Native copy |
| --- | --- | --- | --- |
| out_of_range, record `00D863F4` | `00E0817C` | 28h | `00441760` |
| logic_error, record `00D84000` | `00E08020` | 28h | `004118D0` |
| exception, record `00E03C7C` | `00E174F0` | Ch | `00BF63A6` |

All have properties0 and PMD `{0,-1,0}`. Existing concrete
`NativeHardwareLayoutInvalidIterator` is exactly `28h`, owns actual legacy storage,
constructs logic_error then writes D6926C, copies through `00441760`, and destroys
through `004412B0`. Source throws that actual owning carrier and arms only the
completed message guard. Current C++ allocation/copy/destruction/throw transport
is qualified separately from original RTTI, catch inheritance, FH3/SEH, private
frames, fault addresses and binary calling convention. It is not `std::runtime_error`
and does not promise native std::out_of_range catch identity.

## Validation

Initial Source draft: whole listing, live boundaries/bytes, caller setup, native
FuncInfo/throw records and actual provider Source reviewed. All 724 bytes and 243
instructions match the installed PE; seventeen typed transfer rows pass live
verification. Prior names/comments and full documentation were archived, then
both annotations retained names/comments under the tool write lock, saved the
project, and refreshed both exports. The prior-value log is
`local/ghidra-annotations-20261015T204857Z.json`; comments were read back.
The unique CMake line was appended only after an immediate unleased check and
removal of this worker's queue request, with no exclusive whole-file lease.

Accepted extrema Source `af4feef5e` and registration `a6bf90804` were merged
without provider edits at `f6b977c8090055c9f66d36f698a9700653182f4f`. The only
merge conflict was resolved mechanically by `merge_resolve.py`'s CMake union.
Erase Source/header remain unchanged from draft
`8e1be2328be24a9f6a18817ed88986f60aed4ba7`.

The worker's required `scripts/build.ps1` Release MSVC Win32 build passed with
`/W4 /WX /fp:strict`; process-local `/MP4` only enables parallel compilation.
All three existing CTests passed in 8.76 seconds: reconstructed math 0.23s,
native math differential 0.04s, and tool tests 8.44s. An unrelated existing
`LNK4006` duplicate `spawn_request_id_matches` warning remains. No new tests
were added; existing math differential does not cover this entry, and packet
differential/game validation did not run.

The actual compiled erase entry has a `30Bh` code section (779 bytes, including
terminal INT3). It retains the initial node/nil capture before saved-map store
at `+11/+14/+1A`, actual increment at `+28`, and current successor reload at
`+49`. All six rotation calls remain, at `+18C/+1E7/+22F/+253/+274/+29C`.
After nonfinal rotations, actual sibling reads precede saved-map value reloads;
the current repair map is retained for ordinary ascent. The compiled ascent
captures the root word before reading parent.parent, then performs the pure
comparison afterward; its inputs and resulting branch remain the captured
pre-load values. Native flag timing is not claimed.

Captured name data/length loads are at `+2A5/+2B2`, before getter `+2BA`;
the compiler computes captured length+1 after getter without rereading length.
Pool return is `+2CA`, node free `+2D3`, current saved-map/count reload
`+2E2/+2E4`. Final owner/output/node loads at `+2F4/+2F7/+2F9` precede ordered
stores at `+2FC/+2FE`. These are new Source offsets, not native addresses or
private caller-argument aliases.

The throw helper assigns the counted message at `+42`, arms state0 at `+54`,
constructs the existing owning carrier at `+5B`, and invokes the real current
C++ throw runtime at `+69`. Its real unwind funclet transfers to the completed
message guard, whose destructor calls actual `004072D0`. Current compiler
throw metadata references the carrier's real copy/destructor and one Source
catch type; it does not claim the native descriptor's three-type hierarchy.

The object is 6995 bytes, SHA-256
`b50a4247d92cf57303889b5f7a988de576a9b606a911ba0568e1af7cf876cea4`.
Receipts are `local/goal_oct15_scene_map_erase_build.log`,
`local/goal_oct15_scene_map_erase_object.txt`, and
`local/goal_oct15_scene_map_erase_bytes.json`. Diff whitespace checks pass.
