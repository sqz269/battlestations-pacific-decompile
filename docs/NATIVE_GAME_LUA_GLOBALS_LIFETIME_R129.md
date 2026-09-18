# Lua global vector cleanup

R129 supplies `00B6CF90` (258 bytes) and its forward record copy `00B6C360`
(77 bytes), then binds game-destructor call site `004DD0EB` to the actual
borrowed headers and string-pool context. All 335 bytes match the original PE
and live `bsp.gpr` program. The descriptive Lua label is provisional: the
neighboring `B6D3C0` calls this cleanup before string-vector assignment and
`B6D1B0`; that producer calls the existing VFS Lua-script override service.
Those producers are not reconstructed by this packet.

## Recovered behavior

`B6CF90` takes no native input and returns with plain `RET`. It operates on two
actual 10h headers `{opaque,begin,end,capacity_end}`:

- `0108FF30`: 8-byte pooled string headers.
- `0108FF40`: 12-byte records, each owning a CRT allocation in its first DWORD.
  The remaining two DWORDs are copied as opaque values.

The first vector's nonnull begin and nonzero signed-shift size gate **both**
cleanups. A null/empty first vector leaves the record vector untouched. The
source uses an unsigned shift only for the zero predicate; its result is zero
for the same differences `0..7` as the original arithmetic shift.

The string end is captured before validation. A returning first diagnostic
reloads current begin/end; a second diagnostic does not reload either captured
argument. `4954F0` receives the captured first/end with both iterator owners set
to the actual first header. The concrete default uses the existing checked
string erasure and the game's retained actual pool bridge. It keeps the backing
array, opaque header and capacity.

The record loop reloads current begin/end on every iteration. Its count is the
signed wrapped byte difference divided by 12 toward zero, compared against the
unsigned index. Each iteration frees the first DWORD at current begin plus the
running byte offset; the pointer slot remains stale. The loop advances the
index by one and byte offset by 12.

After the loop, capture its last observed end. The first returning record-bound
diagnostic reloads begin/end; the second reloads only end. If the resulting
begin equals the captured end, skip copying **and** the end store. Otherwise
copy from the captured end through the reloaded end to the retained begin,
then publish the copy result. Thus returning handlers can make the normally
empty suffix copy nonempty.

`B6C360` is native cdecl `(first,last,destination)`, plain `RET`, EAX result.
The caller also pushes three unused private scratch DWORDs. It computes the
wrapped return end from signed difference/12 and walks forward in 12-byte
steps. Each DWORD is loaded and immediately stored before the next load;
overlapping destinations ahead of the source can propagate an earlier value.
It is not a memmove contract. Valid accessible ranges terminating in 12-byte
steps are required.

## Source ownership and diagnostics

The parent passes `lua_globals`, its existing `profile`, and retained
`operation.lua_globals`. Missing contexts or foreign call services fail before
header access. Contexts and publication cells must remain valid throughout
partial cleanup diagnostics. No replacement global/string-pool domain is made.

Source failure retains the contexts, native site, index and partial ownership
graph and rejects replay. A freed payload remains in its stale record slot.
The caller must resolve remaining ownership before retiring diagnostics. Neither
child nor parent silently retries or rolls back; parent retirement requires its
child to be retired first. Native exception/unwind ABI is not provided.

## Evidence and validation

The `B6D02C` free call had a false no-return override, hiding 11 bytes containing
stack cleanup, index/offset increments and the loop back-edge. The locked flow
repair preserved evidence, cleared that override, decoded the gap and saved
Ghidra. Both final function listings have no gaps.

- Strict MSVC Win32 build and all three existing CTests pass.
- 46 copied-original pairs: 22 global-cleanup cases and 24 standalone record
  copies, 104 observed caller boundaries, 274,036 matching normalized bytes.
- Actual pool construction/return, raw lifetime manager and checked string
  erasure are shared services in valid-range cases; payload frees use the real
  CRT. The existing string erasure body is not independently re-proved here.
- Null/empty/sub-element first vectors; empty, populated and null-payload record
  vectors; current begin/end replacement, shrink/extension and nulling; returning
  validation repairs; retained captured end and skipped end store.
- Short strings and 148/149-character strings (13/149/150-byte allocations),
  covering the small/large allocation boundary and small-return gate 0/1.
- Forward copies of 0/1/3/6 records with destination offsets -12/-4/0/+4/+8/+12.
- Two source partial failures/replay rejections, four context guards, and a
  direct check of the actual returning CRT invalid-parameter provider.
- The controlled game-destructor comparison still passes 52 cases, 3,537
  snapshots and 141,402,076 matching bytes, plus four failure/replay cases.
  It checks both context arguments and the retained child's identity.

One global case leaves the captured string range malformed after the second
returning diagnostic. That case uses a controlled `4954F0` boundary solely to
compare captured arguments and subsequent caller behavior. It does not claim
safe real erasure of malformed string ranges. Other valid erasures compare
caller boundary and final pool/header state; internal string frees are not
individually instrumented. Pointer and pool-ring addresses are normalized;
freed bytes, private stack/scratch values and allocation slack are omitted.

No permanent test suite was added. Producer/session integration, malformed
memory, native FH3/SEH, concurrency, drop-in binary ABI, ordinary raw-game
admission and gameplay remain unproven. Remaining parent dependencies include
world/physics and resource-manager/cache cleanup bindings. See
`reports/native_game_lua_globals_lifetime_r129.json`.
