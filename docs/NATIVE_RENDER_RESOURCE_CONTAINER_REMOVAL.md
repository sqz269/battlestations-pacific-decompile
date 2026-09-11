# Native texture-container removal

`remove_native_render_resource_by_alias_00b31dc0` reconstructs the complete
520-byte removal function at `00B31DC0..00B31FC7`. It operates on the actual
container, actual `2Ch` records and actual `10h` alias nodes. Its supported
resource domain consists of the three immutable profiles produced by the
texture manager's examined loader. This is not an arbitrary resource-vtable
implementation or a complete texture-owner lifecycle.

The original ABI is ECX = container, one stack pointer to an eight-byte
length/data name, `RET 4`, with no semantic return value. The container is
embedded at renderer `+1A74`: actual record pointer `+04`, count `+08`, capacity
`+0C`, accounting `+10`. The interface adds the existing actual string-pool
and invalid-parameter domains and borrowed views of the native table storage.
It does not construct a substitute vector, string owner, or resource object.

## Concrete resource dispatch

`00B3256E` installs manager profile `00D5F088` at renderer `+1A74`.
That table's `+08` entry is `00B2C2D0`. The full loader and the full insertion
owner `00B30B40` were byte-checked against the installed PE and live Ghidra.
`00B30F4A` inserts the record; `00B30F5A` invokes its resource's size entry.
The examined loader has these three nonnull creation routes:

| Loader route | Constructor and profile installation | Resource table `+0C` | Full leaf |
| --- | --- | --- | --- |
| 2D texture, resource type 3 | `00B2C60D -> 00B3F930`; `00B3F968` installs `00D61948` | `00B3CE30` | `8B 41 24 C3`: read current resource DWORD `+24`; return EAX |
| Cube texture, resource type 5 | `00B2C724 -> 00B3CED0`; `00B3CF0D` installs `00D61870` | `00A82250` | `33 C0 C3`: return zero, without reading ECX |
| Volume texture, resource type 4 | `00B2C7F7 -> 00B3CFA0`; `00B3CFDD` installs `00D618B0` | `00A82250` | Same real zero leaf |

The 2D loader stores its retained-source length at `+24` at `00B2C645`.
The cube and volume constructors also use `+24`, but their size entries do
not read it. Reading this offset for all textures would be incorrect.
The existing descriptive Ghidra name `BSP_MaterialEffect_ZeroAccountedSize`
identifies the shared zero leaf; this finding extends its known users.

At a match, removal reads the **current** record resource `+28`, then its
current numeric native profile. A small data mapping selects the borrowed
table storage. It then reads that table's current volatile DWORD at `+0C`
and dispatches the exact `00B3CE30` or `00A82250` implementation. Numeric native
addresses are never called as host function pointers. There is no behavior
callback, synthetic owner, early resource snapshot, or guessed zero fallback.

The selected view must be valid and immutable, contain at least four DWORDs,
and have the exact entry stated above. Other profiles/entries are outside
the new interface's preconditions (`__assume(0)`), not new native validation
branches. A view is required only when selected: a not-found call does not
read any resource/table and can receive three null views. The native function
also requires a readable nonnull resource on a match; no extra null check is
introduced. No claim is made for a differently derived manager, modified
vtable, arbitrary externally inserted resource, or construction reentry.

## Name and lookup order

1. Compare the original pointer with the actual local-header address, zero the
   local length and data, and on distinct identity call full `0041DD40` with
   the captured original length and preserve flag one. If the **current**
   original length is nonzero afterward, read current local length, current
   source data, then current local data for the copy. No original-name cleanup
   is armed during this initial construction.
2. Arm state zero, call full `00BEE780` to construct a separate normalized
   name, and immediately release that temporary's current buffer with its
   current length plus one. Its allocations, normalization, exceptions, and
   releases remain observable. The original local name remains the key.
3. Capture array count and begin, then form the end using low-DWORD
   `count * 2Ch` arithmetic. The scan does not reload this array boundary.
   For each record, capture its sentinel and first node. Compare against the
   captured sentinel for termination, and against the **current** sentinel
   before dereferencing and advancing a held node.
4. Native `00B31E73` compares EAX with itself, so its owner-mismatch error edge
   cannot run. The other two checks call the existing `00BF6713` boundary;
   if it returns, execution continues with the held node. No extra validation
   or cached repaired node is substituted.
5. Compare stored lengths before reading string data. Equal zero lengths
   match directly. Equal nonzero lengths call the actual host CRT `_stricmp`
   on current node/local pointers. It is NUL-terminated and locale-dependent,
   not a counted comparison or unconditional ASCII-only folding.

The native `00BF7FBF..00BF800E` entry checks its runtime locale gate
`0109DE1C`, routes to the ASCII or locale helper, and has a returning invalid
parameter path. The reconstruction binds the real host CRT service; it does
not claim that a current UCRT reproduces all legacy CRT locale tables/state.
The existing raw-string convention skips a zero-byte host `memcpy` that would
otherwise pass null pointers; counted nonzero copies remain exact.

## Match, miss, and failure

After the exact size leaf returns, subtract its EAX from **current** accounting
`+10` with DWORD wrap. Read current matched-record and local name pointers for
the diagnostic. A missing pointer substitutes native `0108D5A4`, which is
never dereferenced here: complete diagnostic `004254B0` is one byte, `C3`.
The no-match diagnostic instead reads the original caller name's current data.
No logger callback or output-producing implementation is invented.

After the diagnostic, reload current count and current array to compute the
last record. Assign it into the held match only when the two addresses differ,
using full `00B30510`. Then reload count and array **again**, destroy the newly
selected last record through full `00B2F990`, and decrement **current** count
after that destructor. Capacity and array allocation are preserved. This owner
does not release, decrement, destroy, or retain the resource at `+28`.

Complete EH metadata is pinned: FuncInfo `00DF65DC` (36 bytes), unwind map
`00DF65D4` (8), funclet `00CBDC60` (8), handler `00CBDC68` (10). Magic is
`19930522`, maximum state one, state zero transitions to -1 via
`00CBDC60: LEA ECX,[EBP-14h]; JMP 0041DD20`; there is no try/catch map,
no ESTypeList, and EHFlags is one. Only the current original local-name
buffer is cleaned up by this owner. There is no accounting, assignment,
record-count, or array rollback. Normal original-name release occurs after
state -1 is installed. Existing host storage releases are `noexcept` and
already bound to the actual pool, so native singleton-getter exceptions at
an inline normal release are outside that host service boundary.

## Verification and integration

One private Win32 fixture executes the entire original removal body, both
original getter bodies, the original diagnostic RET, and original unwind
funclet. All 29 complete owner/support/profile-evidence/dependency spans are
checked against installed bytes before loading. An 11-MB sparse private image
is relocated; four original EH pointers and three table entries are checked
before relocation. One owner EH-handler immediate is bridged to the real
host `__CxxFrameHandler3`. Relative owner calls and control flow remain native.
The independent C++ run uses immutable, aligned, exact PE table preimages and
numeric native profile identities; its actual records are independent inputs,
not copies used to shadow updates during either run.

Actual resize, normalized-path construction, record assignment/destruction,
and raw name destruction are bound to their existing **full** C++ bodies.
Unchanged helper source is privately compiled with symbol renames only, so
wrappers can expose the actual local header and observe allocation phases.
The ordinary allocator/free and sized pool perform real allocation and free;
no list/vector/string algorithm is replaced. CRT `_stricmp` and `memcpy` use
the real host runtime. The loader/constructors are pinned static evidence;
they are not executed or claimed fixture-tested by this packet.

Twelve variants match **12,384 normalized DWORD observations** (172 events of
72 words), with three original assignment calls, seven record destructors,
eleven path constructions, ten original memcpy calls, nine original CRT
comparisons, and three original owner unwind funclets. They cover discarded
normalization, case-insensitive/raw-length matching, all three profiles,
empty and embedded-NUL keys, unarmed initial allocation failure, current-name
cleanup after path/assignment failure, and current array/count reloads. A
real CRT thread-local invalid-parameter handler changes the current sentinel;
the ensuing returning/throwing owner validation is observed. The returning
case also changes the actual matched resource and accounting before the size
dispatch, proving the current volume profile's zero result is used.

Fixture corrections were confined to the fixture: the preferred image range
was unavailable, so relocation is used; an empty-key variant initially gave
an owned alias string an invalid pointer, which the correct record destructor
attempted to free, so that owned field was corrected to null. Production source
did not change after its first successful compilation. Pool release callbacks
do not expose the size argument; current length-plus-one is additionally
established by the pinned native/helper bodies rather than claimed as a
direct callback observation. Deliberately abandoned buffers are freed only
after each observation window.

Strict private MSVC Win32 build uses `/std:c++17 /O2 /Oy- /EHsc /fp:strict
/MD /Gy /Gw /W4 /WX`. `scripts/build.ps1` passes; all eight native seed spans
match; the final existing checks pass 2/2 (`reconstructed_math` and
`native_math_differential`). This is build and focused fixture validation,
not native exception-ABI compatibility or a live-game test.

Integration adds `src/native_render_resource_container_removal.cpp` to
`bsp_core`. Root owns Ghidra function creation/annotation/export refresh,
shared CMake and ledger changes. This worker edits exactly the new header,
source, document and audit report. Audit and private reproduction paths:
`reports/native_render_resource_container_removal_audit.json`,
`local/build_native_render_resource_container_removal_check.ps1`, and
`local/build_native_render_resource_container_removal.ps1`.
