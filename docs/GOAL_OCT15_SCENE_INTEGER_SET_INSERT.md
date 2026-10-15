# Actual scene integer-set insertion

Owned addresses: `0046A150`, `0046AC50`, `00C61C10`.

These are whole ordinary C++ bodies over actual tree, node, caller-parameter
and private-cell storage. Existing concrete owning legacy exception services and
the actual integer-node/rotation/decrement providers are composed directly. No
host callback, stub, STL replacement, runtime-error shortcut, positive-key-only
domain, rollback or success-only branch is introduced. Existing descriptive names
are retained; `STL_xlen_throw_0046a150` actually performs full linking/rebalancing,
with a capacity-throw branch.

| Entry | Inclusive body | Bytes / instructions | Native ABI / coverage |
| --- | --- | --- | --- |
| `0046A150` | `0046A150..0046A33B` | 492 / 167 | whole; ECX tree; stack output,left DWORD,parent,key pointer; RET10h/EAX current output |
| `0046AC50` | `0046AC50..0046AD08` | 185 / 73 | whole; ECX tree; stack output,key pointer; RET8/EAX current output |
| `00C61C10` | `00C61C10..00C61C17` | 8 / 2 | whole Source temporary-unwind scope; native frame-50h LEA and tail `004072D0`; no independent Source funclet entry |

This is 685 native bytes and 242 instructions. Source signatures explicitly expose
live backing and are not the original register/stack ABI. Native EDX input is
unused. Tree head/count are `+4/+8`; actual `14h` nodes have left/parent/right
at `+0/+4/+8`, integer key DWORD `+C`, color BYTE `+10`, nil BYTE `+11`, and
untouched trailing bytes `+12/+13`. Signed comparison admits every 32-bit key.

## Live linker and allocator cells

Linker's actual sixteen-byte parameters are `{output,leftDWORD,parent,keyPtr}`.
First read current unsigned count; values at least `3FFFFFFEh` throw before any
parameter or allocator-cell access. This literal is native evidence, not a bound
derived from the node size. On the ordinary path capture key pointer, current head,
then parent, in that order. Publish the distinct twenty-byte allocator parameters
in push order: color0, key pointer, captured head as right, captured parent,
captured head as left. Call the actual allocator only after all five publications.

Allocator `004681B0` allocates `14h` before every input read. Its nonnull sequence
reads left,parent; stores node.left; reads right; stores node.parent; reads key
pointer; stores node.right; reads key DWORD, then current low color byte; stores
key, color and nil0. It preserves trailing padding. The conforming concrete
current CRT allocator retries through the current new handler or throws; the
native normal-null branch is still retained in its provider. The linker adds no
null fallback or success-only guard. Handler changes to the explicit live cells
are visible; parent/key/head values already captured by the linker stay captured.

After allocator return capture CURRENT head BEFORE current count read/increment.
Count addition wraps as a DWORD without another capacity check. Captured parent
equal to that captured head links root, then separately reloads head for minimum
and maximum stores. Otherwise read only CURRENT low byte of the left DWORD;
write the selected parent link, reload current head and conditionally repair the
matching extremum. Capacity/allocator failures produce no new rollback or cleanup.

Existing `TreeInsertAccess<10h,11h>` repair retains every parent/uncle/color reload,
both recoloring branches, all three explicit rotations and the original final
inlined left rotation. Local static adapters bind the concrete separate `11h`
integer-node rotations; no host callback is exposed. Blacken CURRENT head.root,
then read CURRENT output pointer and publish allocated NODE before tree OWNER.

## Unique search and publication

The unique driver's actual eight-byte parameters are `{output,keyPtr}`. Capture
key POINTER at entry, then head/root. Read initial root.nil BEFORE writing only
the low flag byte to1. If that captured nil flag is zero, capture the signed key
DWORD ONCE before the loop; the backedge at `0046AC8C` targets `0046AC71`, skipping
the key load at `0046AC6E`. Each iteration reads the current candidate key, captures
the parent, computes signed less-than, stores the low flag byte and follows the
selected current link, then reads its nil flag. Upper flag bytes stay untouched.

Publish private predecessor NODE before OWNER. The captured `left` decision is
retained even when those actual stores alias other described storage. On its
begin/minimum branch, prepare linker parameters in key,parent,literal DWORD1,
predecessor-address order. Otherwise invoke actual `004679F0` on private
predecessor8 when required and reload CURRENT predecessor.node afterward.

For the final comparison read predecessor.key FIRST, then CURRENT key DWORD from
the originally captured key pointer. On insertion read the FULL current flag
DWORD, including retained upper bytes, and publish key,parent,that flag,predecessor
address into linker parameters before the child. The child's returned iterator
address can differ from the originally supplied predecessor when a current handler
changes its output parameter; use that returned address.

Inserted publication reads returned iterator.owner, then CURRENT outer output
pointer, then returned iterator.node, BEFORE output stores OWNER,NODE,inserted1.
Duplicate publication reads CURRENT outer output pointer first, CURRENT predecessor
owner second, then stores OWNER,captured predecessor NODE,inserted0. Both preserve
the result's final three padding bytes. Duplicate adds no allocation or ownership.

Frame bindings are stable/disjoint metadata. Only reached backing is required:
flag4, predecessor8, linker16 and allocator20 bytes. Actual described storage may
alias when every reached access/release remains valid. The flag upper bytes must
be supplied when its full DWORD read is reached. Source supplies no implicit alias
to private compiler argument homes, frames, EH records or return addresses, and
does not admit unsynchronized concurrent mutation.

## Capacity exception and native cleanup evidence

The exact 19-byte message `map/set<T> too long` is at `00CE47BC`. Initialize actual
SBO temporary capacity15, length0, first inline byte0, then assign via `00408720`.
Parent state remains -1 during assignment; arm state0 only after it succeeds,
before `00411700` logic-error construction. Publish verified length-error profile
`00D69260`, then native descriptor `00D83F98` accompanies the throw transfer.

Handler `00C61C18` selects FuncInfo `00D892CC`: magic `19930522`, max state1,
unwind map `00D892C4`, no try/catch map, EH flags1. Its state0 entry transitions
to -1 through `00C61C10`, which destroys the completed temporary at frame-50h
through actual `004072D0`. Failed assignment has no parent guard cleanup; failed
logic-error construction retains its provider's own base cleanup.

ThrowInfo `00D83F98` has attributes0, destructor `00411780`, no compatibility
callback, and catchable array `00D83FD4`. Its three records are length_error
`00D83FE4` (descriptor `00E08000`,28h,copy `00411940`), logic_error `00D84000`
(descriptor `00E08020`,28h,copy `004118D0`), exception `00E03C7C` (descriptor
`00E174F0`,Ch,copy `00BF63A6`); all have properties0 and PMD `{0,-1,0}`.
Existing `NativeHardwareLayoutTreeLengthError` is the concrete `28h` owning
carrier: actual logic-error construct plus D69260, exact length-error copy,
actual logic-error destructor. Source message guard and owning C++ transport
compose those services; native RTTI/base catches/FH3/SEH identity is unproved.

## Callers and validation

The linker site at `0046ACAE` lies in `0046AC50`; both insertion branches share
that transfer. Scene database constructor `0046F160` supplies the unique driver's
seven live references: `0046F27C`, `0046F295`, `0046F2AE`, `0046F2C7`,
`0046F2E1`, `0046F2FA`, `0046F30F`. They supply a real key DWORD address and actual
twelve-byte result backing; their observed positive constants do not narrow this
Source API. Provider frames retain separate native push/store timing qualification.

Initial Source draft: complete assembly, live boundaries/bytes, caller setups,
native FuncInfo/throw metadata and actual provider contracts reviewed. All 685
live bytes/242 instructions match the installed PE, and17 typed native transfer
rows pass verification; Markdown reports40 addresses with0failures/unchecked/warnings.
The unique CMake line was appended after an immediate unleased check and removal
of this worker's queue entry,without a whole-file lease. Locked annotation/save/
export,provider Source integration,required strict Win32 build and existing CTests/
object review are pending. No new tests are added; packet-specific differential/
game validation and original ABI/RTTI identity are not claimed.
