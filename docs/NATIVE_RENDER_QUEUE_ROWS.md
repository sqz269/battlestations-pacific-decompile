# Actual render queue row storage

The complete `00B1DB30` reserve, `00B1E7F0` resize and `00B1F150` destructor
bodies now operate on the caller's actual 12-byte row header. The queue embeds
that header at `+24h`: data pointer, signed count and signed capacity. Each
row is 20 bytes: native string length/data followed by three float words.
The functions accept `void* actual_header` and use byte access; they do not
overlay a new C++ header object on existing queue fields or maintain another
owner/vector/count. For example, pass the address of `queue.rows_data_24` from
`NativeRenderCommandQueueStorage` directly.

The public `NativeRenderQueueRowStorage` declaration documents the raw row
layout and has no initialization or destructor. The implementation does not
cast existing storage to that type. It uses the existing actual-header string
resize and supplied `NativeStringStorage`, plus the shared raw
`singleton_lifetime_allocate/free` boundary. It implements no queue getter,
queue owner, renderer, worker, scheduling or command execution.

## Native behavior and original ABI

| Entry | Original ABI | Complete byte span, exclusive end |
| --- | --- | --- |
| `00B1DB30` | ECX actual header, signed capacity on stack, `RET 4` | `00B1DB30..00B1DC66`, 310 bytes |
| `00B1E7F0` | ECX actual header, signed count on stack, `RET 4` | `00B1E7F0..00B1E86D`, 125 bytes |
| `00B1F150` | ECX actual header, `RET` | `00B1F150..00B1F167`, 23 bytes |

The new C++ interfaces return `void` and add an explicit string-storage
argument. Incidental native EAX results are not an advertised return value.
Stored Ghidra prototypes are still `undefined ...(void)`; the ABI above is
derived from the assembly. Proposed descriptive names are
`BSP_RenderQueueRows_Reserve`, `BSP_RenderQueueRows_Resize` and
`BSP_RenderQueueRows_Destroy`, not recovered symbols. The audit preserves
the exact current names, prototypes and comments for primary integration.

Reserve clamps requested capacity to at least one and returns if current
capacity is sufficient. It allocates raw `requested * 20` bytes using native
DWORD multiplication, with no additional overflow check. It copies rows
while comparing against the current signed header count. For each computed
nonnull destination it captures the current source row, clears only destination
string length/data, and skips string copying if the two row addresses are
equal. Otherwise it calls actual `0041DD40(destination, source_length, true)`.
After that call it tests the captured source's current length, then reads the
current destination length, source data and destination data for the copy.
A storage callback can replace the source fields or the outer header; these
later reads and the next row iteration retain their native meaning.

Each row then executes three explicit ordered x87 `FLD m32`/`FSTP m32` pairs,
at offsets `08h`, `0Ch` and `10h`. Integer or SSE copies would fail to quiet
signaling NaNs or retain the x87 status effects. The implementation uses Win32
inline assembly and does not change the caller's floating-point control word.

Old strings are released forward, using the current header data and count
for each iteration. Each release captures the row's nonnull data pointer and
reads current length plus one. It leaves both row words untouched. Reserve
then reloads current old data for array-free. Only after free returns does it
publish replacement data and requested capacity; it does not assign count.
Free callbacks can therefore change count, and those changes survive the
two publication stores.

Reserve has one native unwind action at `00CBCA80`. It computes the end of
completed construction and calls `00401130` with that and the current row
pointer. The whole verified helper is byte `C3`: **RET**. Consequently a
failure during string construction leaves replacement array storage and
previously constructed strings unreleased. This implementation adds no RAII
rollback, header restoration or extra frees. All completed callback effects
remain visible. This is the original body, including its cleanup quirk.

Resize reserves first when requested count exceeds capacity, then reloads
old count. Growth clears only the eight string-header bytes in each computed
nonnull new row. It rereads header data for each slot, preserves the three
float preimages and does not increment the header count during growth.
Shrink decrements the actual count before reading that count/data and capturing
the current tail string. After release it rereads count for the next decision.
It leaves freed row words unchanged and finally assigns the requested count.

The destructor calls resize zero, reloads the header's current data and frees
that pointer. It retains the dangling data value and capacity. No C++ header
lifetime is implicitly ended and no extra reset occurs.

All address/row-offset arithmetic uses unsigned native 32-bit operations,
including wrap and the native computed-null-row branches. The caller must
provide suitable readable/writable spans for actual accesses; live header
count and capacity are normally nonnegative. The functions do not repair
corrupt headers, replace failed allocations with empty rows, synchronize
access or add a size policy beyond the existing host boundaries.

## Existing host boundaries

The base revision is `e36b33d`. It already contains
`resize_native_string_header_0041dd40` against actual header storage. This
packet uses that checked-in interface without copying later work from the
primary checkout. Its existing limitations remain explicit:

- Supplied `NativeStringStorage` replaces the repeated native `00419CC0`
  pool-singleton lookup and its first-use side effects. The original allocate
  and release flag is one; the host interface does not carry that flag.
- Storage release is `noexcept`. Existing `PooledStringStorage` pool bounds
  and allocation policies are unchanged.
- The actual string helper omits a native zero-byte `memcpy`, which can have
  a null source. The focused fixture accounts for those calls separately.
- `singleton_lifetime_allocate/free` remains the explicit raw allocator
  boundary. The request uses matching native/host byte counts and the existing
  `pointer_slots` allocation category; no new allocator ownership is added.

## Verification

Thirteen spans, 730 bytes, were read through guarded live Ghidra queries and
matched byte-for-byte to the installed executable. They include the three
complete row functions, complete actual string resize, reserve EH action/
handler/map/info, the exact `RET` helper and six service-hook preimages.
The report pins the installed binary hash, source revision and nine dependency/
build-file hashes. Every live query verified `bsp`,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 language and image
base `00400000`. No Ghidra state was changed.

One private Win32 fixture executes all three full native row entries and
actual `0041DD40` from a sparse copy of the installed executable, checking
all 730 bytes again before execution. The only service replacements are
array allocate/free, pool getter, string allocate/release and `memcpy`.
It relocates the reserve's absolute EH pointers and routes its checked native
handler through the host `__CxxFrameHandler3` personality. An image-resident
registration thunk tailjumps to that relocated original handler; direct
registration of the sparse-memory handler was not dispatched on this host.
This is explicit fixture EH plumbing, not a game-runtime or mitigation claim.
The final fixture observes exactly one native exception search and unwind
through state zero and the original unwind map/action/`RET` helper.

The compared header is the existing `NativeRenderCommandQueueStorage+24h`,
with surrounding queue words checked unchanged. Nine focused scenarios cover
callback changes during allocation, captured-source copying, current forward
destruction, publication after array-free, x87 signaling-NaN quieting and
masked invalid/denormal status, failure on the second string allocation with
no rollback, backward shrink/count mutation, destructor data reload, preserved
growth float words, self-address copying, computed-null slots, minimum-capacity
clamp/no-grow and wrapping allocation multiplication. The fixture compares
pointer roles, sizes, call order, header/row words and string bytes. It keeps
the deliberately unreturned fixture buffers available for inspection.

```text
PASS: full B1DB30/B1E7F0/B1F150 versus actual queue+24: 11890 normalized words; 9 scenarios; 11 pool boundaries; 3 zero-copy omissions; original EH partial failure and x87 status agree.
```

The new source, current actual string source and fixture compile with MSVC
Win32 `/O2 /EHsc /fp:strict /W4 /WX`. All eight existing `verify-seeds`
spans matched, and `scripts/build.ps1` passed both existing CTest checks.
CMake/shared registration files are untouched: the new module was compiled
and linked by the private fixture driver; the normal repository build checked
the existing registered sources. The primary integrator must add
`src/native_render_queue_rows.cpp` to the normal source registry.
No tracked test or broad test suite was added.

Private preparation, fixture and driver files use the
`native_render_queue_rows` prefix under `local/`; their hashes, build logs and
fixture executable/object hashes are retained in the audit. This establishes
full-body reconstruction against actual storage with a native-byte differential
check across explicit service/EH boundaries. It does not establish original
allocator/pool runtime behavior, unmasked floating-point exception handling,
game execution, concurrent queue safety or drop-in binary ABI compatibility.

## Primary integration follow-up

The native `00BF6989` free call is incorrectly marked no-return in current
affected listings. `00B1DB30` is missing the ten bytes at
`00B1DC49..00B1DC53` containing stack adjustment, replacement data/capacity
publication and saved-register pops. The full function ends at `00B1DC65`.
`00B1F150` currently ends at `00B1F161`, omitting `ADD ESP,4; POP ESI; RET`
at `00B1F162..00B1F167`; its actual final byte is `00B1F166`. These bytes
were included in both the audit and native execution. The primary owns any
flow repair, name/comment updates, refreshed exports, sharded ledger entries
and CMake registration, using the repository's normal Ghidra write lock and
preserving the captured prior annotations.
