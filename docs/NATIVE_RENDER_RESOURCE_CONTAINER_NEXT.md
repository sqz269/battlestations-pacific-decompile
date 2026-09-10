# Actual resource-container removal after record assignment

The next useful packet is the **complete native substring `00469840` plus its
complete copy-constructor dependency `00426060`**. Implement both against actual
eight-byte headers and the existing supplied `NativeStringStorage` domain. The
primary agent completed actual-header destruction `0041DD20` and
ASCII lowercase `004BCC00` in `3ef5439`; their inspected interfaces are below. After
substring construction, the remaining concrete name chain is full `00BEE690`
and full `00BEE780`. The existing `std::string` normalization is a different host
interface and must retain its current behavior.

This read-only report starts from `c94a5bb` plus record-assignment commit
`a7f004d` (local cherry-pick `9e89ca9`). All 37 selected initialized code/data
spans match guarded live Ghidra reads and the installed PE. Two zero-filled
analysis-image observations are separately identified; they are not file-backed
bytes or running-process state. No C++, shared ledger, Ghidra annotation, build
registration, or test changed. No new execution or game validation is claimed.

## Smallest complete string packet

Use new `include/bsp/native_pooled_string_substring.hpp` and
`src/native_pooled_string_substring.cpp`, with evidence artifacts. The primary
agent owns `native_string.hpp/.cpp`. The next packet should own only these two
native entries; normalization and the container remain later full functions.

| Entry | Complete extent | Native ABI and role |
| --- | --- | --- |
| `00426060` | `[00426060,004260A2)` / 66 bytes | ECX actual destination, stack source, EAX destination, `RET 4`; copy construction |
| `00469840` | `[00469840,00469950)` / 272 bytes | ECX actual source; stack output, start, count; EAX output, `RET 0Ch`; substring construction |

`00426060` compares header identity, writes destination length/data to zero,
then acts on that comparison. Exact identity therefore abandons the old buffer.
For distinct headers it captures source length for `0041DD40(..., preserve=1)`,
rereads source length after allocation, then copies current destination length
from current source data to current destination data. It neither releases an
old destination nor adds cleanup if construction fails. It has **no null
destination guard**. Existing `0044BCB0` does have that guard and a placement
construction wrapper, so it is not an unconditional replacement.

`00469840` begins with an empty local header and output-constructed flag zero.
It interprets start as signed for the negative-start branch: add start to count
with DWORD wrap, then set start to zero. Otherwise clamp start to current source
length with an unsigned comparison. Capture source data; require nonnull data,
nonzero current length, and start below that length. Compare wrapped start+count
against the captured length, reducing count to length-start only when the
unsigned comparison says it exceeds the length. Do not invent range exceptions,
a saturating addition, an `INT32_MAX` guard, or a null-output check.

For a nonempty source range, capture source-data+start before allocating the
local header via full `0041DD40`. Capture the resulting local data pointer in
EBP. If requested count is nonzero, `strncpy` copies **that count** from the
captured source address; embedded NUL pads remaining bytes without reducing the
stored length. Construct the actual output from the local through `00426060`.
Only after that returns, set output-constructed flag one and lower EH state to
zero. Normal cleanup releases the **captured EBP pointer**, with the **current
local length+1**. It does not reread local data for this normal release.

The empty-range branch clears output length/data, compares output against the
local header, and calls resize-to-zero for distinct headers. It does not release
an abandoned output buffer. The newly cleared output keeps resize's equal-zero
early return. The function returns the actual output address in every normal
path.

FuncInfo `00D891E0` has two states. State 1 unwinds through `00C61B50`, destroying
the current local header, then state 0 runs `00C61B58`: test/clear bit 0 at
EBP-18h and, only if armed, destroy output from EBP+4. The flag is set after
output construction succeeds. A failure during that construction cleans the
local but does not synthesize cleanup of partially constructed output. Normal
temporary release occurs after the transition to state 0, so a native exception
from that release would clean the successful output. Existing host storage
release is `noexcept`; document that service boundary rather than claiming
native exception transport. Preserve explicit cleanup timing, especially the
captured normal pointer versus current-header unwind pointer.

Required existing/pending services:

- Integrated `resize_native_string_header_0041dd40(void*, NativeStringStorage&,
  uint32_t, bool)`; real `PooledStringStorage` supplies the actual sized pool.
- Primary-owned, committed `destroy_native_string_header_0041dd20(void*,
  NativeStringStorage&) noexcept` and
  `lowercase_native_string_header_004bcc00(void*) noexcept`.
  Their `3ef5439` source blobs are pinned separately from this earlier checkout.
- Actual `memcpy`/`strncpy` library operations, with the existing documented
  omission of null/zero-count host `memcpy` if required by the host contract.
  Do not replace native string operations with callbacks to missing bodies.

Use strict Win32 compilation and one focused original-body comparison covering
the concrete risk: source/output aliasing, allocation-time header changes, and
the local/output cleanup flag boundary. A successful ordinary string comparison
alone would not verify this ownership contract.

## Remaining normalization chain

`00BEE690` is the complete 240-byte in-place normalizer, ECX actual header,
plain `RET`. It calls raw `004BCC00`, then captures length for a signed-positive
slash loop while reloading data each iteration. Leading-space trimming uses an
unsigned length; trailing-space traversal uses a signed-positive length-1.
Only byte `20h` is trimmed. It invokes `00469840` for a separate result, arms
local cleanup only after the substring call returns, then resizes/copies back
into the actual input. The later source-length guard and destination copy count
are current fields. Normal and unwind cleanup both destroy the substring local,
with state -1 installed before normal release. FuncInfo `00E02168`, map
`00E02160`, cleanup `00CC75B0` establish that lifetime. The current semantic
`resource_path.cpp` rejects length above `INT32_MAX` and lacks these allocations
and callback-visible states; keep it separately qualified.

`00BEE780` is a separate complete 126-byte constructor: ECX output, EDX source,
EAX output, plain `RET`. It zeroes output fields before acting on identity,
resizes/copies, **then arms its construction flag**, and calls `00BEE690`.
FuncInfo `00E02194`, map `00E0218C`, cleanup `00CC75D0` destroy output only when
that flag was armed. Failure during initial copy does not get the cleanup that
a failure during normalization gets. Exact identity empties the output before
normalizing it. No native null-destination guard exists.

## Full removal contract and resolved dependencies

`00B31DC0`, full `[00B31DC0,00B31FC8)`, receives ECX = the entry renderer's
actual +1A74h container, stack original string, `RET 4`, no semantic return.
It makes a raw local copy, then constructs and immediately destroys a **separate
normalized copy** through `00BEE780`. Alias comparisons still use the original
local copy. Eliminating the apparently discarded normalization would remove
real allocation, failure, and cleanup behavior.

The scan captures array begin and count-derived end once, walks actual 2Ch
records, captures each sentinel for its end comparison, and reloads that record's
current sentinel for dereference/increment validation. The apparent
owner-inequality validation at `00B31E73` is `CMP EAX,EAX`; its failure call is
unreachable. The two current-sentinel validation sites can invoke `00BF6713`
and continue if it returns. Compare stored lengths first. Equal zero lengths
match without touching data. Nonempty equal lengths call actual CRT `_stricmp`;
this is a NUL-terminated comparison, not a counted byte comparison.

`00BF7FBF` dispatches to ASCII comparison only while CRT locale gate `0109DE1C`
is zero; otherwise it calls `00BF7EEC` and locale-aware lowering. Invalid nonempty
data sets errno 22, invokes the CRT invalid-parameter path, and returns
`7FFFFFFFh` if that path returns. The zero seen in Ghidra's uninitialized analysis
image does not prove the running gate is zero. Preserve a concrete CRT service
boundary; do not claim arbitrary ASCII-only folding is complete native behavior.

On a match, retain the record address, reload resource+28h, its vtable, and
current slot+0Ch; invoke it with ECX resource and no stack argument. Subtract EAX
from current container accounting+10h. Read diagnostic name arguments after
that call. **Diagnostic `004254B0` is exactly `C3` (`RET`)** in this executable;
it has no format processing or concrete owner dependency. Both removing and
not-found calls target this same leaf. No fabricated logger callback is needed.

After the diagnostic, reload count and data to find the last record. Assign it
to the retained match only when their addresses differ, using the now-complete
`00B30510`. After assignment, reload count and data again, destroy the resulting
last record with full `00B2F990`, then decrement the current count. Preserve
capacity and array allocation. No resource retain, release, or virtual destructor
belongs in this name-removal path. FuncInfo `00DF65DC`, map `00DF65D4`, cleanup
`00CBDC60` only destroy the original local name; accounting/list/name changes are
not rolled back. State -1 precedes normal local-name release.

Actual 2D texture profile `00D61948` slot+0Ch at `00D61954` points to the full
four-byte getter `[00B3CE30,00B3CE34)`: `MOV EAX,[ECX+24h]; RET`. The loaded-2D
constructor installs that profile and initially zeros +24h at `00B3F968`.
This proves a stored accounting-size getter for that profile, not a replacement
for every resource's current slot. `00B3CE30` is still absent as a separate saved
Ghidra function. Full texture ownership remains outside this read-only packet.

## Actual header and array lifetime

| Offset from container | Current evidence |
| --- | --- |
| +00h | Actual profile `00D5F088`; base destruction publishes `00D5F038` |
| +04h, +08h, +0Ch | Actual 2Ch-record array pointer, count, capacity |
| +10h | DWORD accounting total |
| +14h, +18h | Fallback name length/data |
| +1Ch | Initialization flag byte; padding is not initialized by this fragment |
| +20h | Fallback resource pointer |

Renderer-constructor fragment `[00B32556,00B32590)` zeros these fields and
publishes the actual profile. The vector header begins at **container+4**, not
at the container itself. Full `00B30340` resizes that header and full `00B2FF00`
reserves storage with a signed minimum capacity of 64, concrete record copy
construction `00B2FC60`, record destruction, and ordinary array free. Its hidden
post-free tail publishes the captured new array and requested capacity at
`00B2FFC4/00B2FFC6`; the saved decompiler falsely stops at `_free`.

The full reserve/resize path still needs concrete record copy construction,
list construction `004D48A0`, sentinel allocation `004C3020`, and their exception
contracts. `00B316C0` clear invokes each current resource size virtual and the
container's current release slot before rereading count/array and destroying a
record. That is different from name removal. Base destructor `00B32090` calls
clear, resizes the vector to zero, ordinary-frees the actual array, and has a
verified post-free epilogue through `00B320EE`. Derived `00B32370` separately
releases fallback resource/name before the base. These are precise future
lifetime dependencies; a new host vector or field-only header cannot establish
their completion. The smaller substring packet avoids opening that larger
allocation/copy/lifetime chain prematurely.

The JSON report pins the current concrete resize, record assignment/destructor,
alias insertion/checked operations, node/count helpers, pool and runtime-domain
sources. Earlier removal advice that record assignment or a diagnostic owner
were still missing is superseded by this current audit. Previously reported
native fixture results remain those artifacts' evidence; this packet only
revalidated current source and selected live/disk bytes.
