# Native session messages 78 through 81 (R197)

Addresses: `0075BD50`, `0075BDC0`, `0075BE00`, `0075BE60`, `0075BDB0`, `0075BDE0`; `0075A030`, `0075A070`, `0075BEC0`, `0075BF10`, `0075A060`, `0075A090`; `0075A0B0`, `0075A0E0`, `0075BF60`, `0075BFB0`, `0075A100`, `0075C000`; `005F9660`, `005F9690`, `005F96B0`, `005F96D0`, `005F96F0`, `005F9A00`. Partial parent: `00768530`. Reused dependencies: `0075B430`, `0075B480`, `0075B4C0`.

## Scope

Twenty-four complete normal game methods (1,038 native bytes) and four five-slot profiles are reconstructed. Thirteen previously absent functions are defined at verified instruction boundaries. Four scalar-deletion flows have no remaining gaps. Names describe observed behavior and are hypotheses, not recovered symbols.

| Type | Size | Constructor | Predicate | Writer | Reader | Destructor | Scalar delete | Profile |
|---|---|---|---|---|---|---|---|---|
| 78 | 28h | 75BD50 | 75BDC0 | 75BE00 | 75BE60 | 75BDB0 | 75BDE0 | D02F38 |
| 79 | 24h | 75A030 | 75A070 | 75BEC0 | 75BF10 | 75A060 | 75A090 | D02CCC |
| 80 | 24h | 75A0B0 | 75A0E0 | 75BF60 | 75BFB0 | 75A100 | 75C000 | D02CE0 |
| 81 | 20h | 5F9660 | 5F9690 | 5F96B0 | 5F96D0 | 5F96F0 | 5F9A00 | CF3BA4 |

All receivers use ECX. Writers take one raw 10h cursor pointer, readers one 18h stream pointer whose cursor begins at +4, predicates one full DWORD query and scalar deletion one flags DWORD; these methods end in RET4. Constructors and ordinary destructors have no stack arguments. Profiles hold scalar deletion, write, read, predicate and the existing always-true `4499C0` entry. The source profile's trailing session-context pointer is an explicit borrowed binding and is not part of the native five-slot layout.

## Constructors and lifetime

Message78 constructor is 89 bytes. It inlines base construction: delivery3, zero fields8/C, D02C68, type78, one capture of E188A8 and signed owner index at Game+18EC. Indices0..7 select Game+18CC[index]; other values produce null. It then zeroes WORD18 and bytes1A/1C, sets delivery1 and D02F38. Payload20/24 and padding remain untouched.

Message81 constructor is 39 bytes. It calls existing `75B430` with type81, then zeroes WORD18 and bytes1A/1C, sets delivery1 and CF3BA4. This preserves the real base-constructor call boundary.

Messages79/80 have 40-byte constructors: fields8/C and owner14 are zero; mutable type10 is **zero**, WORD18 and bytes1A/1C are zero, delivery is1 and the own profile is installed. They do not consult Game and leave payload20 and padding unchanged.

All four 32-byte predicates ignore the receiver and mutable type. They accept their own full DWORD type, 73 or 70. High bits are not truncated. All seven-byte ordinary destructors only stamp CE4974. The 31-byte scalar wrappers inline that root stamp, call BF65AC only when flags bit0 is set, and return the captured receiver address. No payload ownership or new EH handler is invented.

## Wire behavior

The existing `NativeMessage75ExtendedHeader` represents bytes00..1F. Common fields are mutable type10 width8, WORD18 width12, Boolean1A and Boolean1C. Boolean reads canonicalize to0/1 while padding remains unchanged.

| Type | Fields after the common header | Total bits |
|---|---|---|
| 78 | Boolean20, unsigned DWORD24 width4 | 27 |
| 79 | unsigned DWORD20 width4 | 26 |
| 80 | **signed** DWORD20 width4 | 26 |
| 81 | no additional fields | 22 |

Message78 writer/reader are 90/88 bytes, message79 78/77 bytes and message80 78/77 bytes. Their common-header operations are inlined in native code and remain explicit in the source. The signed message80 reader calls `428D30`, preserving sign extension of wire values8..15; message79 uses `428D10` and retains unsigned values8..15.

Message81 writer/reader are each31 bytes. They call existing `75B480`/`75B4C0` for type8, WORD18/12 and Boolean1A, then separately write/read Boolean1C. The source uses those concrete providers. Their original 53-byte bodies are added to the fixture as previously reconstructed support, not counted as new game methods or library ports.

## Validation and integration boundary

Strict MSVC Win32 compilation and three existing CTests pass. The fixture compares **13,593 original/source pairs and 32,289,717 observation bytes**. The 2,184 new cases comprise 112 constructor/destructor cases, 1,024 predicates, 24 scalar deletions and 1,024 wire cases. They exercise all eight alignments, full-DWORD predicate rejection, null receivers for fixed predicates, arbitrary retained-byte patterns, signed owner bounds, noncanonical Boolean inputs, four-bit truncation/sign extension, cursor positions, read-wrapper ownership bytes and raw five-slot dispatch. The 11,409 earlier cases and four earlier source-only fault checks remain in the corpus; no new fault claim is made for these classes.

The collector verifies 30,245 live Ghidra/PE bytes, 41 owned CALL edges and 921 fixture relocations. Raw factory calls `768DC6`, `768DE6`, `768E06`, `768E26` remain outside that total because their instructions lack stored Ghidra function ownership. The separately verified table at76A3CC maps types78..81 to labels768DB2,768DD2,768DF2,768E12, with allocations28h/24h/24h/20h. Ghidra renames/comments preserve prior values, use the shared write lock and are read back after saving.

This evidence does not establish whole original ABI/FH3, allocator failure handling, malformed input safety, arbitrary message/cursor aliases, concurrent mutation, full factory composition, startup/network execution or gameplay. It uses the existing concrete bit/storage/allocation services, not a claim about the entire original CRT. Borrowed session publications and profiles must remain alive. The full `00768530` factory is still unbound.

Next dependencies are the remaining factory arms beginning at type82 and their concrete class methods. Once dependencies are ready, bind the full stream factory, packet recorder and networking in the startup path. See `reports/native_session_messages78_to81_r197.json` for byte/call evidence and immutable tested/integrated artifacts.
