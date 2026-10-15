# Actual scene integer-set head allocation and construction

Complete ordinary Source now covers the two actual14h-node head routines below.
The existing semantic `Seg6Host::int_set_buy_head_node_004682b0` projection is
preserved. New code calls the genuine existing current-CRT allocation service,
with no host callback or stub replacing either routine.

| Routine | Native range | Coverage | Original ABI | C++ symbol |
| --- | --- | --- | --- | --- |
| `004682B0` | `[004682B0,004682E7)` | complete | No arguments; EAX head; RET | `allocate_native_scene_integer_set_head_004682b0` |
| `00468EA0` | `[00468EA0,00468ECD)` | complete | ECX actual tree; two unread stack words; RET8; EAX original tree; ESI preserved | `construct_native_scene_integer_set_head_00468ea0` |

The allocator requests exactly14h bytes through `00BF681B`. Its full body has
the same conditional native-address link stores as accepted `00468260`: test
the returned address before zeroing link0, test wrapping address+4 before its
zero store, and test wrapping address+8 before its zero store. It then writes
color10=1 and nil11=0 unconditionally. PayloadC..F and padding12/13 are untouched.
These are inline placement-style guards, with no placement call, EH frame or
local cleanup action. Source uses DWORD address arithmetic and volatile storage
without whole-node initialization or an added null-result recovery policy.

The real existing `singleton_lifetime_allocate` implementation follows the
observed malloc, `_callnewh(size)` retry and `std::bad_alloc` schedule. Its return
is nonnull-or-throw. Native14h and host14h sizes match in this strict Win32 API.
Original CRT handler identity and private exception representation remain host
boundaries, not original ABI compatibility claims.

The constructor preserves the caller's tree address across allocation,
publishes captured head at tree+4, then sets captured head.nil11=1. It reads
current tree+4 independently before each self-link store, ordered parent+4,
left+0, right+8, and writes count+8=0 last. Tree+0 is untouched. The two native
stack words consumed by `RET8` are never read, so the Source API takes only
the actual tree pointer. No prior-head destruction, added validation or rollback
is introduced. Its full body matches accepted `00468E70` except the callee and
the nil-byte offset required by the smaller node layout.

All five live allocator-helper caller setups were inspected. `00468943` and
`0046CF13` are tiny constructor variants. `00468EA3` is the constructor above.
`0046F212` and `0046F23F` initialize the scene database's embedded trees at
+158h and +164h. The same containing constructor supplies DWORD integer keys
44h/5Eh/1Dh/47h/3Dh and 47h/3Dh respectively to the actual unique-insertion
routine `0046AC50`, which reads node+C. This supports the integer-set consumer
description without identifying a nominal enum or class-map meaning. Live xrefs
found no references to `00468EA0` itself, nor callers of the other tiny variants;
the packet does not assert this standalone constructor is game-reachable.

| Containing function | Site | Native callee | Contract |
| --- | --- | --- | --- |
| `004682B0` | `004682B2` | `00BF681B` | Stack size14h; EAX raw allocation; caller ADD ESP,4 at `004682B7`; nonnull-or-throw. |
| `00468EA0` | `00468EA3` | `004682B0` | No native arguments; EAX actual14h head; RET. |

Live queries verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Complete native spans are checked against the
installed PE. Existing Ghidra names and empty comments are preserved before
evidence is added under the write lock, followed by project save and exports.
Strict MSVC Win32 build and existing CTests are pending in the initial draft.
No new routine tests, candidate-specific native differential, original private
ABI/FH3/SEH compatibility, startup execution or gameplay validation are claimed.
