# Structured resource reader

Addresses: 00715bf0, 00be42e0, 00be4620, 00be9a40, 00be9c40, 00be9df0, 00be9ed0, 00be9f10, 00be9fc0, 00bea150, 00bea250, 00bea380, 00bea680, 00bea700, 00bf0280, 00bf02a0, 00bf03e0, 00bf0430, 00bf0510, 00bf09a0, 00bf09b0.

The native reader consumes records as a DWORD tag-byte count, that many tag
bytes, a DWORD payload-byte count, and the payload. All nodes share one stream
cursor. Explicit skip advances over unread payload; node destruction does not.
This distinction is required by the resource dispatcher and its field parsers.

Twenty-one complete code spans (2,041 bytes) and three data spans (56 bytes)
freshly match the installed executable, whose SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live batch verified project `bsp`, project file
`C:/Users/sqz269/bsp.gpr`, and program `/battlestationspacific.exe` through the
configured CLI. Exact spans, hashes, original comments, ABIs, and annotation
proposals are in [structured_reader_audit.json](../reports/structured_reader_audit.json).
Descriptive names below are reconstruction hypotheses, not recovered symbols.

## Objects and ownership

`00bea150` initializes the 70h-byte reader. Its first 10h bytes are initialized
by `00bf09a0`; ten native strings occupy `+10h..+5Fh`, the active-path index
is `+60h`, a control DWORD starts at `FFFFFFFFh` at `+64h`, and a separate
string starts at `+68h/+6Ch`. The loader copies its resolved source name into
that last string. The active-path strings are copied on node construction;
closing a node decrements the index without clearing its old string slot.

`00bf0430` assigns reader `+0` to a stream. When old and new differ, it stores
the new pointer, atomically increments the new stream's `+4` reference count,
then decrements the old stream and calls old virtual `+0` on zero. Assigning
the same pointer does nothing. This retain makes the loader's subsequent
release of its acquired stream valid. `00be9f10` destroys both string areas
and calls base destruction `00bf09b0`, which releases the retained stream.
Optional base-buffer cleanup through `00bf0700` is not reconstructed here;
the ordinary constructor initializes that buffer triplet to zero.

Root and child nodes each occupy 24h bytes:

| Offset | Meaning established by constructors and cleanup |
| --- | --- |
| `+00h/+04h` | Vtable `00d68bb4`, intrusive reference count initially one |
| `+08h` | Borrowed reader pointer, cleared when detached |
| `+0Ch` | Borrowed parent node pointer; root has null |
| `+10h/+14h` | Copied native-string length/data for tag bytes |
| `+18h` | Depth; root zero, child parent depth plus one |
| `+1Ch` | Declared payload length |
| `+20h` | Remaining payload-byte accounting |

Node construction does not retain the reader, parent, or source stream. Their
lifetimes must enclose descendant use. The constructor appends its tag at
`reader+10h+8*reader[60h]` and increments that index without a capacity check.
The native array has ten slots; neither depth nor index is validated here.

## Record and scalar reads

`00bea700` allocates 24h bytes and invokes root constructor `00bea380`;
`00bea680` similarly invokes child constructor `00bea250` with the parent
object. Both return a supplied one-pointer output wrapper, writing null if
allocation returns null. They do not validate the source or parent wrapper.

For a complete read, a record is:

```
u32 tag_byte_length
u8  tag_bytes[tag_byte_length]
u32 payload_byte_length
u8  payload[payload_byte_length]
```

The stream methods use x86 little-endian DWORD loads. Root tag and length
reads debit a local value initialized to 1000; this is unused bookkeeping,
not a header-size bound. Child tag and length reads debit the parent's
remaining field by the actual transferred header-byte counts. Both store
the returned payload length in their own declared and remaining fields.
No constructor verifies a magic tag, version, payload fit, or minimum header.

Tag adapter `00bf0510` calls stream virtual `+48h`, supplying a nonnull
actual-count pointer, then subtracts that reported count from its budget.
Both inspected stream vtables map `+48h` to `00be4620`. That method reads the
tag length through virtual `+38h`, allocates the native string through
`0041dd40`, fills its logical bytes with spaces, and performs one raw read
for the declared tag count. The count it reports is the actual length-prefix
count plus actual tag-byte count. A short tag read leaves spaces in the tail;
it does not shorten the stored length. A zero tag length returns an empty
string. Allocation internals and arbitrary alternate stream vtables remain
external contracts.

Payload length adapter `00bf0280` uses virtual `+34h`, mapped in both tables
to `00be42e0`. Control/scalar adapter `00bf02a0` uses `+38h`, whose established
target is `00be4300`. Both concrete DWORD methods request four bytes through
raw-read virtual `+24h` and return the incoming argument slot as EAX. That
slot doubles as the read buffer. With a nonnull actual-count argument,
unwritten bytes on a short read preserve pointer-address bits. These callers
must not reuse the font adapter's null-count, initially-zero short-read path.

`00be9a40` reads one DWORD from the node's shared reader, debits the node's
remaining field through `00bf02a0`, and stores the result at reader `+64h`.
It is a control-word read, not handle initialization. Its meaning is not
proved; no value is checked. The root dispatcher `00b7f430` invokes it before
iterating. `00715bf0` returns one exactly when its valid wrapper contains a
nonnull node whose remaining DWORD is nonzero. It does not check the stream,
node attachment, sign, bounds, or whether enough bytes remain for a header.

Float reads through node wrapper `00be99d0` and budget adapter `00bf02c0`
retain the previously audited actual-count and x87 load/store behavior in
[STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md](STRUCTURED_RESOURCE_HIERARCHY_BOUNDS.md).
The adapter subtracts actual bytes; short reads have the same pointer-seed
distinction described in [STREAM_SCALAR_READERS.md](STREAM_SCALAR_READERS.md).

## Closing and advancing

Child header bytes are charged while constructing the child. Its declared
payload is charged to the parent once, when the child detaches. Reads within
the child separately debit the child's remaining field. These are different
accounting operations; charging each scalar to the parent as well would
double-count consumed child bytes.

Explicit skip `00be9c40` subtracts the child's declared payload from its
parent, if any. If its own remaining value is nonzero, it calls `00bf03e0`
to seek `(low=remaining, high=0, origin=1)` through stream virtual `+1Ch`,
then sets remaining to zero. It decrements reader `+60h` and clears node
`+8`. The extra pointer passed to `00bf03e0` is ignored; the adapter does not
debit it or inspect seek success. The established memory-stream implementation
interprets origin one as relative to its current cursor.

Handle release `00be9ed0` atomically decrements the node reference and invokes
virtual `+0` on zero, then clears the wrapper. Vtable `00d68bb4` maps `+0`
to established `00bd30e0`, which invokes deleting destructor `+4` with flag
one; `+4` maps to `00be9fc0`. That wrapper calls body `00be9df0` and frees
the node when flag bit zero is set.

`00be9df0` subtracts the full declared payload from the parent and pops the
path only if the node is still attached. It clears the reader pointer and
frees the tag. **It never seeks unread bytes.** Explicit skip detaches first,
so later destruction does not subtract again. Without explicit skip or full
payload consumption, destroying a node leaves the cursor inside its payload.
The native caller must traverse and close in depth-first order; these bodies
do not enforce that order, protect unsigned budget arithmetic, or make a
second explicit skip valid.

## Evidence limits and host projection

`00be42e0` was not defined as a Ghidra function at audit start; its complete
26-byte body was read directly. Base-reader destructor `00bf09b0` had a stale
`CALL_RETURN` override on its free call at `00bf0a0f`, truncating the inventory
span. Disk-matching bytes show its real epilogue through `00bf0a27`; the full
120-byte span is included here. The primary integrator owns both repairs and
all saved annotations.

The typed implementation belongs to the complete-transfer, initialized-memory
domain. Explicit host rejection of truncated headers, payload overruns,
excessive depth, or invalid lifetime/order is additional policy. Native
malformed-input behavior, arbitrary stream implementations, original ABI,
allocator failures, exception-unwind parity, and game validation remain
unproved. Source ownership, cursor advancement, build results, and installed
asset probes must be reported separately.
