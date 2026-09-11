# Native hardware layout teardown

Six complete functions cover the derived hardware layout destructor, its base,
four retained declaration records and the actual pool return. Names describe
reconstructed behavior; they are not recovered C++ symbols.

| Address range (exclusive end) | Original ABI | Behavior |
| --- | --- | --- |
| `00B60700..00B60765` | ECX owner, RET | Release COM declaration, access support, destroy base |
| `00B60770..00B60790` | ECX owner, stack flags, EAX original owner, RET4 | Destroy, then optionally return slot |
| `00B48960..00B489CF` | ECX owner, RET | Renderer removal, four records, reference-counted base |
| `00B483F0..00B48419` | ECX record, RET | Atomic retained CPU declaration release |
| `00B48950..00B48960` | ECX records begin, RET | Reverse four records through native array semantics |
| `00B60110..00B60175` | ECX pool, stack slot, RET4 | Return actual 48h slot under its pool lock |

The six functions contain 402 native bytes. The owner occupies 44h bytes;
its raw pool slot adds a slab-index DWORD at `+44h`. The four 0Ch records begin
at owner `+08h`. Each contains a retained CPU declaration pointer followed by
eight untouched bytes. COM declaration storage is at `+40h`. The destructors
do not change intrusive count `+04h`, bytes `+38h..+3Fh`, or slot metadata.

## Captures, dispatch and cleanup

`B60700` writes derived profile `D62AF4`, captures the current COM pointer and
arms state 0 after its test. A nonnull pointer calls its current COM Release
slot. Only a successful return clears the current owner `+40h`, including
overwriting a value changed by the call. The routine then calls the complete
`B3E730` support accessor using the shared published slot and lifetime domain.
It disarms state 0 before normal base destruction. It therefore does not retry
that base if the normal base call throws.

`B48960` writes profile `D61D10`, reads the current global renderer, its current
profile and notification slot `+44h`, then arms state 1 immediately before the
notification. The established renderer table `D5F0A8` contains `B2F4C0` at that
slot. The C++ interface reads a borrowed immutable view of that exact table
and invokes the complete actual-tree removal operation. The native renderer
ECX argument is unused inside that removal. The tree header at `0108D530` is
the canonical checked-iterator owner identity.

After notification returns, state 0 covers reverse destruction of the four
records. After the array returns, the routine disarms cleanup and writes base
profile `CEB130`. The array wrapper's native ECX argument is the first record,
not the enclosing owner. It decrements remaining count before each destructor;
a failure destroys only the lower remaining prefix and never retries the
failed element.

Each record captures its declaration, performs real `InterlockedDecrement`
at declaration `+04h`, and only for a zero result reads the current profile's
zero-reference slot. The known CPU declaration table `D61D1C` names `BD30E0`,
which rereads the current profile before calling deleting slot `B48CA0` with
flag 1. The C++ path composes the existing complete declaration destructor and
actual `0108FD38` pool return. A successful release clears the current record's
first word. A null first word is left alone. A throwing terminal leaves it
nonnull; the count has already been decremented.

The context explicitly requires the evidenced renderer and CPU declaration
profiles and their immutable original-token tables. Other runtime profiles
are outside its input domain. No arbitrary virtual callback supplies missing
ownership behavior, and native numeric profile addresses are not host vtables.

## Native exception evidence

| Handler and metadata | State | Unwind action |
| --- | --- | --- |
| `CC12F8`, FuncInfo `DFA084`, map `DFA07C` | 0 to -1 | `CC12F0` invokes complete `B48960` on saved owner |
| `CBF6E3`, FuncInfo `DF80CC`, map `DF80BC` | 1 to 0 | `CBF6D8` adds 8 to saved owner and invokes `B48950` |
| Same base handler | 0 to -1 | `CBF6D0` invokes `BD30F0` on saved owner |

Normal array/base calls may throw. Calls made only during active cleanup use
`noexcept` helpers to preserve termination on a second C++ exception. This
also preserves the native `BF7C10` array-unwind boundary. These state choices
come from assembly and original FH3 metadata; the pseudocode's aliased local
state variables do not accurately show them.

## Actual pool return

`B60110` borrows an initialized pool: real Win32 critical section at `+0Ch`,
tracked depth at `+24h`, slab-pointer array at `+28h`, and earliest available
slab at `+34h`. Each slab contains 32 raw 48h slots followed by WORD free indices
at `+900h` and a WORD count at `+940h`.

Under the same lock, it increments depth, reads the slot's slab index, loads
the current slab pointer and computes signed, DWORD-wrapped displacement.
The native signed magic multiplication implements division by 48h toward
zero. The low WORD is written at the old free count. The count is reread and
incremented after that write, including when the two addresses alias. An
unsigned minimum updates the earliest slab; depth decreases before leaving
the lock. There is no added exception guard, lock initialization or substitute
pool. `B60770` calls this only after successful destruction and `flags & 1`.

## Verification limits

The primary strict MSVC Win32 build and both existing CTest checks passed.
Current live Ghidra bytes are compared with the installed executable and
pinned with the source and existing dependencies in
[the audit](../reports/native_hardware_layout_owner_audit.json).

An independent original-code comparison is in progress. Build checks alone
do not establish the exception paths, D3D driver behavior, runtime renderer
profile, binary ABI compatibility, full pool lifetime or gameplay behavior.
Construction and arbitrary declaration/renderer profiles remain outside this
packet. No new tracked tests are introduced.
