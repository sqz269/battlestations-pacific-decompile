# Actual physical buffer ownership

`src/native_physical_buffer_owner.cpp` reconstructs the actual 2Ch physical
index/vertex buffer owners, their raw pointer arrays, their diagnostic record,
and the 30h-slot return operation. The four deleting slots preserve separate
heap and pool allocation domains. These are new C++ interfaces with borrowed
actual storage and existing service boundaries; descriptive names are
hypotheses. The implementation does not install callable native vtables.

## Storage and original entry interfaces

| Offset | Actual field |
| --- | --- |
| `00` | Native profile identity |
| `04` | Intrusive reference DWORD, initialized to one |
| `08/0C/10` | Raw pointer-array data, signed count and capacity |
| `14/18` | Raw flags and byte capacity |
| `1C/20/24` | Preserved buffer metadata after construction |
| `28` | Borrowed or owned COM buffer according to the operation |
| `2C` | Slab index in a pooled 30h slot, outside the 2Ch object |

| Address | Complete function | Original ABI |
| --- | --- | --- |
| `00B4BB60`, `00B4BBB0` | Pooled index/vertex construction | ECX owner, EAX same owner, RET |
| `00B4C250`, `00B4C370` | Index/vertex COM attachment and diagnostics | ECX owner; stack COM, flags, byte capacity; RET Ch; no stable result |
| `00B4B900`, `00B4BAB0` | Index/vertex destruction | ECX owner, RET |
| `00B4B490`, `00B4B530` | Index/vertex base destruction | ECX owner, RET |
| `00B4BB20`, `00B4BB40` | Heap scalar deleting slots | ECX owner; stack flags; EAX original address; RET 4 |
| `00B4C210`, `00B4C230` | Pooled deleting slots | ECX owner; stack flags; EAX original address; RET 4 |
| `00B49500` | Raw 30h-slot return | ECX actual pool; stack raw slot; RET 4 |
| `00B496A0`, `00B49700` | Raw pointer-array reserve/resize | ECX 0Ch header; stack requested DWORD; RET 4 |
| `00B3F4C0` | Borrowed-COM/string diagnostic-record cleanup | ECX actual 0Ch record, RET |

The 16 complete functions occupy 1,484 bytes. Five returning-free continuations
were verified and repaired in Ghidra while preserving their prior comments:
`B4B490` through exclusive `B4B4ED`, `B4B530` through `B4B58D`, `B4BB20`
through `B4BB3E`, `B4BB40` through `B4BB5E`, and `B496A0` through `B496FF`.

## Profile and lifetime distinction

Construction first installs base `00CEB130`, writes count one, zeros DWORDs
`08..20`, then `28`, then `24`, and finally installs the pooled index table
`00D61E58` or pooled vertex table `00D61E7C`. The trailing slab word is untouched.
Private physical objects are initialized within the logical-stream creation
paths, which remain separate work.

Destruction installs private index `00D61E10` or private vertex `00D61E34`, even
when the object occupied a pooled slot. It visits the actual resource-support
singleton before loading current COM `+28`. A nonnull current COM object is
released through its current table slot `+08`, then `+28` is zeroed after the
call returns. A callback change to the owner field does not change the captured
COM receiver or prevent the subsequent zero store.

The derived destructor's state-zero unwind calls its complete base destructor.
Normal execution disarms that state before calling the base. Base destruction
resizes the actual `+08` pointer array to zero, captures its current data pointer,
frees it, then installs `00CEB130`. Its own state-zero unwind installs that base
profile. Array data/capacity remain stale after free; no added nulling or COM
rollback occurs.

The unwind-only base call is nonthrowing: a second C++ exception from its
cleanup terminates, as in the original MSVC frame-unwind filter. The normal
base call remains throwable, preserving its own cleanup and propagation.

Deleting slots test only flags bit zero after complete destruction. Heap
slots `B4BB20/B4BB40` call the actual host scalar-free boundary. Pooled slots
`B4C210/B4C230` call the same full slot-return body with distinct actual globals
`0108FDA8` and `0108FDE0`. Every deleting slot returns the original address;
the storage can already be freed or reusable. Identical getters do not imply
an identical allocator.

## Attachment and diagnostic cleanup

Both attachments first store flags and capacity. They capture old COM `+28`,
compare it with the supplied pointer, and on inequality publish the supplied
pointer before AddRef-new and Release-captured-old. Each COM call reads that
captured receiver's current table. Equality skips those reference calls but
still performs the diagnostic work. No exception rollback of flags or COM is
invented.

Both use the actual `00D61EA0` string `Vertex or Index Buffer`, including its
terminator. The first native eight-byte temporary is initialized and resized
to length 22; its data and length are captured and used for the first copy.
This occurs before state zero. A first construction/copy exception therefore
has no local cleanup.

The diagnostic record captures current owner COM `+28` without retaining it,
and initializes its separate eight-byte name. State zero protects its resize
and copy; if either throws, only the first current temporary is destroyed.
State one is armed only after that second copy completes. A support-call
exception then destroys the current record name using `B3F4C0`, followed by
the first current temporary using `41DD20`.

Normal record cleanup disarms state one and releases captured record data with
the current record length plus one. Normal first-temporary cleanup disarms
state zero and uses the captured first data and length. Neither cleanup clears
the headers. `B3F4C0` separately reads current record data `+08` before length
`+04`; it never releases or changes the borrowed COM word.

## Actual array and pool operations

Reserve interprets request and capacity as signed DWORDs, clamps the request
to at least one, and allocates `request*4` with native DWORD wrapping. It copies
each pointer through the current source data/count header, skips a wrapped null
destination, then frees current old data before publishing captured replacement
and requested capacity. Count is not restored. Resize calls reserve only on
the native signed comparison, captures count afterward, reloads data for every
default-zero element, decrements current count one step at a time when shrinking,
and finally writes the requested count. There are no element destructors.

Raw pool return borrows an initialized pool: real critical section at `+0C`,
tracked depth at `+24`, slab-pointer array at `+28`, and earliest available slab
index at `+34`. It enters before incrementing depth, then captures the slot's
slab index at `+2C` and reads the current slab array. Each slab contains 32
30h slots, WORD free indices at `+600`, and a WORD free count at `+640`.
The signed wrapped slot-minus-slab difference is divided by 30h with truncation
toward zero, matching the native signed-multiply sequence. Its low word is
stored at the old free count, the current count is incremented with WORD wrap,
and the pool's earliest index is lowered if needed. Depth decrements before
the real Leave call. No new bounds guard, pool owner, allocation or EH guard is
introduced by this return operation.

## Evidence and validation limits

The current evidence map contains 43 spans and 2,005 bytes matched against the
installed executable and the existing `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe`. It includes all 16 bodies, six original EH maps
and FuncInfo records, their native unwind actions, the diagnostic text, and
declared external call/Win32 boundaries. All 32 fixture relocations have
verified operand or data preimages. The strict Win32 build and both existing
CTests pass. One focused original-caller sequence matches 14,520 trace DWORDs
across 30 comparisons against all 16 definitions from the primary library.
The primary agent reverified all native spans, source and library hashes,
private artifacts and link-map definitions, then reran that sequence.

One isolated double-fault regression reproduced the cleanup mismatch before
the fix: native execution terminated, while the first reconstruction propagated
the replacement exception. Both native and corrected source now invoke the
controlled termination handler after the same two failing allocations.

Six original FuncInfo/maps and their native unwind actions are retained with
host-image registered bridges to the actual MSVC frame handler. Seven distinct
unwind actions execute. Index attachment state-one action `CBF9E8` is verified
but unexercised; the vertex analogue and complete `B3F4C0` execute. Real Windows
critical sections are used. COM receivers are explicit IUnknown-compatible
fixture objects, and injected exceptions are fixture observations.

NativeStringStorage remains the existing explicit allocation/sized-release
boundary. Its release is nonthrowing, allocation cannot return null, and zero
byte memcpy is omitted as in the existing actual-header helpers. These limits
do not establish the original CRT's exceptional pool-getter behavior. The
actual resource-support global and lifetime domain are borrowed rather than
replaced. Pools must already be initialized, objects must belong to the chosen
allocation domain, and raw fields must be valid for the native operation.
No driver, full pool initialization, whole logical-stream lifetime, installed
binary ABI, renderer execution or gameplay validation is claimed.
