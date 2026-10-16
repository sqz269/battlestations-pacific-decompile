# Native ownership transfer of DWORD-vector headers

This packet completes the actual 16-byte header transfer core, its lexical
completed-prefix catch, its three-input dispatcher, and the two selected unwind
actions. It preserves the established STL names as library identities; those
names do not imply deep copy of the transferred input allocations. Only
construction from the private empty header uses the accepted copy constructor.

Source: `src/native_dword_vector_transfer_range.cpp`; interface:
`include/bsp/native_dword_vector_transfer_range.hpp`. Historical narrower actual
Source, Hook interfaces, fixtures and earlier evidence remain separate.

## Exact scope and original ABI

| Entry | Original identity | Current ranges (inclusive endpoints in separate cells) | Bytes / starts | Source coverage |
|---|---|---|---|---|
| 006980E0 | STL_UninitCopy_WordVector16 | 006980E0 through 0069816D; 00698198 through 006981BC | 179 / 66 | Complete normal core |
| 0069816E | Catch_All | 0069816E through 0069817C; 00698180 through 00698197 | 39 / 15 | Lexical prefix catch |
| 00698350 | STL_TagDispatch | 00698350 through 00698374 | 37 / 14 | Complete dispatcher |
| 00C7EC30 | Unwind | 00C7EC30 through 00C7EC37 | 8 / 2 | Actual private-header Tidy in outer exception scope |
| 00C7EC38 | Unwind | 00C7EC38 through 00C7EC48 | 17 / 7 | Qualified inert-action effect elision |

The core has ECX first, EDX last and four physical stack words: output,
unused owner bits, duplicate output bits, and tag. It returns the advanced
output in EAX and uses RET10h. Output is captured before the first equality
comparison, including the empty case. Last is captured once. There is no count
conversion, range alignment check, endpoint/null validation, or rollback.

The dispatcher has three stacked first/last/output inputs and RET0Ch: the new
Source API is explicitly stdcall. Native incoming ECX contributes owner bits
and a tag with its low byte cleared. It duplicates output into the other dead
core stack slot. The Source wrapper uses canonical zero owner/tag metadata.
This does not reproduce an ambient register, owner, or native private frame.
The core never reads its duplicate-output/tag slots; the catch reads owner
only if the prefix is nonempty, and actual D29D0 consumes neither owner nor EDX.
The dispatcher returns the actual core EAX without independent arithmetic.

## Normal order, aliases and ownership

The private 16-byte empty header has only +4/+8/+C initialized to zero. Opaque
+0 is unspecified and unowned. Before each element, native state 2 records the
current output in two private cells. Nonzero output invokes actual 557590 from
that same private empty header. Zero output skips only construction: all
following raw field accesses still occur.

For each field +4, then +8, then +C, read the input field, read the output field,
store the input value into output, and store the old output value into input.
Each pair completes before the next pair is read. For +C, advance the output
cursor by wrapped 10h between its output store and its input store. Only after
that final input store publish the completed output cursor; advance input by
wrapped 10h next. The loop compares raw addresses for equality. Source inline
assembly preserves these public alias effects without a typed array/header
snapshot. Direct +0 exclusion does not promise that arbitrary backing aliases
cannot reach some other object's +0 bytes.

The accepted constructor owns cleanup of its failing output. The parent catch
destroys only the already completed prefix, forward in 10h steps, using actual
D29D0 with no null skip. It captures both prefix endpoints, then conditionally
reads the unused owner argument. A bare CurrentCpp rethrow follows. There is no
cleanup of the failing element beyond its constructor, no undo of input swaps,
and no ownership rollback.

Normal exit reads private +4 once and conditionally frees that captured pointer.
It performs no normal Tidy or reset stores. The free remains lexically inside
the prefix try, with the outer temporary cleanup armed through its return.
Only then is temporary cleanup disabled. Exceptional propagation runs prefix
cleanup before actual CDCE0 tidies the CURRENT private header, including its
conditional captured-begin free and ordered +4/+8/+C clears.

## Raw native exception evidence

Current project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified through BSP before analysis batches.
The 112 bytes beginning DABA3C match disk and live memory:

| Item | Address | Contents |
|---|---|---|
| Catch descriptor | DABA3C | 40h, type 0, displacement 0, handler 0069816E |
| Try record | DABA4C | low 1, high 3, catch-high 4, count 1, descriptor DABA3C |
| FuncInfo | DABA60 | magic 19930522, maxstate 5, unwind map DABA84, one try DABA4C, IP map/type list 0, flags 1 |
| Unwind states | DABA84 | 0: (-1,C7EC30); 1: (0,0); 2: (1,C7EC38); 3: (1,0); 4: (0,0) |

Initial state 0 owns the private header. The equality comparison installs
state 1 even for an initially empty input. State 2 precedes the constructor
null branch. No explicit state-3 normal store appears. Catch state 4 leads to
the private-header action. The normal free still has state 1, inside the
declared try; there is no native early disable. The saved graph incorrectly
attributes normal cleanup to a catch and producer sites to old catch labels;
complete current typed ranges and bytes govern this packet. No repair occurred.

C7EC30 selects actual CDCE0 with LEA ECX of the private header. C7EC38 reads
two saved private cursor cells, pushes both, invokes the literal one-byte RET,
balances eight stack bytes and returns. Its elision is limited to valid
private-frame/current C++ behavior. It supplies no public EBP helper, fake
destructor, or stub. Native handler C7EC49 and the literal RET are support only.
The skipped three bytes at 0069817D are 8D4900, with no current instruction start
or owned function body; they are not counted as catch instructions.

## Exact transfer proof

| Exact instruction | Scope |
|---|---|
| `00698138 CALL 00557590` | Owned core |
| `006981A0 CALL 00BF65AC` | Owned normal captured free |
| `00698183 CALL 004D29D0` | Owned lexical catch |
| `00698193 CALL 00BF6885` | Owned lexical rethrow |
| `0069836C CALL 006980E0` | Owned dispatcher |
| `00C7EC33 JMP 004CDCE0` | Owned private-header unwind |
| `00C7EC40 CALL 00401130` | Owned inert action |
| `00C7EC4E JMP 00BF6B43` | Support handler only |
| `006984CD CALL 00696890` | Incidental unowned producer context |
| `006984F0 CALL 006980E0` | Unowned direct producer |
| `00698500 CALL 00698310` | Incidental accepted fill context |
| `0069851C CALL 006980E0` | Unowned direct producer |
| `006985B3 CALL 00698350` | Unowned dispatched producer |
| `0069862B CALL 00698350` | Unowned dispatched producer |

Current typed containment places all four transfer producers in 00698400,
independent of stale snapshot catch names. Direct sites prove six physical
inputs; the first advanced result becomes the subsequent fill destination.
The second direct result is discarded. The first dispatched result is
discarded by a current-end reload; the second is published to receiver +8.

## Verification and boundaries

Owned evidence is 280 bytes / 104 exact current starts. Handler/literal support
is a separate 11 / 3. Three unowned producer windows total 180 / 74 and prove
only bounded argument/return use. Total code evidence is 471 / 181; raw EH is
112 additional bytes, and the skipped gap is three separate noncode bytes.
All inspected code/EH bytes match PE and live memory; producer overlap also
matches the preserved ignored readiness receipt. The required MSVC Win32 build passed; all three existing suites passed in
8.41 seconds. All 14 report transfers (12 CALL and two JMP) and 14 Markdown
claims pass with no failures, unchecked claims or warnings. Fourteen focused
actual CMake COFF checks pass. No new tests were added.

The accepted constructor and both distinct Tidy entries are actual Source,
not Hook providers. Their accepted implementations bind current allocation,
diagnostic and CRT services. In particular, `singleton_lifetime_free` is an
existing noexcept current-CRT boundary. Its termination/fault behavior does not
establish the native free-origin throw path, despite the preserved lexical
scope. Source C++ catches/rethrows do not establish original FH3 exception
identity, nested cleanup-failure behavior, RTTI, SEH/access-fault cleanup,
asynchronous faults/mutation, native private-stack aliases, ambient register
identity, or binary patch compatibility. Valid raw storage and finite equality
traversal are preconditions inherited from native callers. Build/static proof
is separate from original runtime and gameplay validation. No parent range,
insertion or append is promoted by this packet.

The separately authorized optimized object compile also passed 13 checks.
The actual CMake core is identical across all 327 bytes and nine emitted call
relocation targets; the 39-byte compiler handler is also identical. The actual
27-byte dispatcher and both 8-byte bit helpers were fully inspected separately.
The compiler schedules a private input-register increment before the completed
frame-slot store, after every public swap and before the next call; the Source
private-frame qualification applies. Core size includes six trailing INT3 bytes.
