# Text character acceptance lookup

Read-only audit on 2026-09-09. Every live batch verified project `bsp`, program
`/battlestationspacific.exe`. This resolves the immediate character acceptance
dependency identified in TEXT_INPUT_FALLBACK. No C++, shared metadata or Ghidra
annotations were changed.

## Call chain and character representation

Fallback00a96750 passes the original event to00ab6d30 with text-owner+24 in ECX.
00ab6d30 sign-extends the low byte to EAX and compares against positive constants
A2h,A3h,A5h,A7h,AAh,B2h. Those comparisons cannot succeed for a sign-extended byte.
The ordinary path tail-calls00ab6d00 while preserving the event stack argument.
Do not reinterpret that apparent blacklist using unsigned bytes.

00ab6d00 accepts byte20h (space) immediately, without dereferencing the context.
Otherwise it loads the pointer at context+108h into ECX, sign-extends the event
byte into AX, pushes EAX and calls00ad4500. Upper16 bits of EAX are not established
by this conversion, but the lookup reads only the low16 bits. Consequently:

| Input byte | Lookup key |
| --- | --- |
| 00h..7Fh | 0000h..007Fh, except space bypasses lookup |
| 80h..FFh | FF80h..FFFFh |

This is not UTF-8 decoding, a Windows code-page conversion, or zero-extension
to U+0080..U+00FF. The tree compares the resulting 16-bit keys as **unsigned**.
00ab6d00 returns EAX1 when membership succeeds and0 otherwise, RET4. Its context
is in ECX and the event occupies one stack argument. Space succeeds even when
no ordinary glyph node exists for it.

## Tree membership routine00ad4500

ABI: ECX is the object reached through context+108h; one stack key argument;
bool result in AL (EAX explicitly zeroed before SETNZ), RET4. It records
object+4 as the sentinel pointer, invokes00ad4370 with a temporary iterator
output and a pointer to the key stack slot, and checks the returned iterator
owner. Null owner or owner different from ECX calls00bf6713, the native
invalid-parameter path. Successful lookup returns true exactly when the returned
node differs from the recorded sentinel. There is no fallback-glyph acceptance
or mutable state in this routine.

## Exact-key lookup00ad4370

ABI: ECX tree object; stack output iterator pointer followed by key pointer;
EAX returns the output pointer, RET8. Ghidra's void signature loses that return.

| Location | Observed use |
| --- | --- |
| Tree+4 | Sentinel/end node |
| Sentinel+4 | Root node |
| Node+0 | Left child |
| Node+4 | Parent/root storage, unused by traversal except sentinel root |
| Node+8 | Right child |
| Node+C | Unsigned16 key |
| Node+15h | Nonzero marks nil/sentinel |
| Iterator+0 / +4 | Owner / selected node |

The search starts with candidate=sentinel and current=root. Until current.nil
is nonzero, it goes right if current.key<requested key; otherwise it records
current as candidate and goes left. This is unsigned lower-bound traversal.
After traversal, sentinel candidate or requested<candidate.key produces the
end iterator; otherwise the candidate is an exact match. It writes both iterator
words to the supplied output and returns that pointer.

The routine does not check null object, sentinel or child pointers, validate
ordering/balancing, detect cycles, or impose a step bound. Valid native tree
invariants are required. Its lookup does not consult payload, node color or
container count, so these need not be invented in a projected read-only interface.
The adjacent00ad4480 uses the same lookup for a value retrieval path, including
special/default records at object+4Ch/+6Ch/+8Ch. That is supporting context;
the meaning and lifetime of those records were not fully recovered here.

## Smallest viable port and remaining boundary

A read-only typed tree view with sentinel, root, left/right links, uint16 key and
nil flag can reproduce the traversal and membership check. The wrapper can then
preserve space acceptance and signed-byte-to-16-bit conversion exactly. A native
iterator ABI is unnecessary for a source-level bool interface, provided its
absence and empty/invalid-tree handling are explicit. Do not substitute a
platform-font query, isprint, or an ASCII whitelist: native acceptance tests
membership of this particular loaded tree.

This resolves the lookup algorithm, not its data source. Context construction,
the writer/ownership of context+108h, font/resource loading and tree population
remain unestablished in this bounded audit. A fixture-supplied tree can verify
the port but cannot prove that installed game text uses the correct glyph set.
Character remapping still follows acceptance, and string editing still requires
the dependencies documented in TEXT_INPUT_FALLBACK.

## Exact byte evidence

These inclusive complete-function ranges match both the installed executable
and the live saved Ghidra program. SHA256 covers exactly the listed bytes.

| Range | SHA256 |
| --- | --- |
| 00ad4500..00ad453d | 021f7dca2899d4615a6e07db7d38711830db21e49a596f238ae6b06c8cb00cff |
| 00ad4370..00ad43dc | 3ef5fa2954b729e45b6a2cfbf1576e6669455d3168f9090ccb931a22ac2ae9f9 |
| 00ab6d00..00ab6d28 | f75e438aec1c7bc3ce0ec000973f9d0069b7160144bd1ee2ba5df9d84405b4e0 |
| 00ab6d30..00ab6d6e | 0a93c9db9f5b51cc4d8bae428e274f1243f0c6a65da8b666f7558758cb8d7626 |

No build or runtime test was performed. These results establish static lookup
behavior, not native text rendering, font-loading parity or gameplay.
