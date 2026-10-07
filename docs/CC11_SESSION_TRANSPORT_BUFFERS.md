# Complete embedded session transport buffer lifetime

Packet `cc11_health_transport_buffer_lifetime` reconstructs the complete ordinary
constructor `00783080..007830E2` (99 bytes) and destructor
`007830F0..00783118` (41 bytes). Both take the actual member in ECX and use plain
RET; the constructor returns that member in EAX. The C++ functions have new source
interfaces and do not reproduce the private native calling convention.

The member occupies `D3Ch` bytes in caller-owned storage. In the observed target
constructor `007839D0` it is embedded at target `+10h`:

| Member offset | Target offset | Storage |
| --- | --- | --- |
| `000` | `010` | Recovered numeric profile stamp `00D04264` |
| `004`, `468`, `8CC` | `014`, `478`, `8DC` | Three contiguous `464h` byte buffers |
| `D30`, `D34`, `D38` | `D40`, `D44`, `D48` | Three independent cursor pointer cells |

The profile value is a numeric stamp only. The implementation neither invents an
executable vtable nor dispatches through it. Storage has no default initialization,
automatic constructor, destructor, or enclosing-target ownership.

## Construction and release

Construction stamps the profile first. Each iteration then calls the existing
`singleton_lifetime_allocate` service with
`{SingletonAllocationKind::object, 0x10, sizeof(NativeBitCursor)}`. The static native
cursor size is `10h`, so native and host allocation sizes are equal. This is the
existing `00BF681B` source boundary in `src/singleton_lifetime.cpp`: `std::malloc`,
`_callnewh` on failure, retry when it succeeds, otherwise throw `std::bad_alloc`.
No lifetime manager, allocation domain, fallback heap or callback is constructed.

For a nonnull cursor, the constructor stores backing base, length `464h`, and bit
offset zero, clears only the backing's first byte, reloads the cursor's base into
its current pointer, and finally publishes the cursor cell. A null result skips
all cursor/backing writes and publishes null. Allocation normally returns a
pointer or throws through the existing service; the null arm remains in both
source and optimized assembly but is not artificially injected by the fixture.

No catch or partial cleanup is added. If a later allocation throws, the profile
stamp and earlier publications remain, while the current and later cells retain
their preimages. This function supplies no automatic unwind cleanup. Historical
CRT handler globals, exception identity and private exception machinery are not
reconstructed by this packet.

Destruction restores the numeric profile stamp, captures the first cell address,
then freshly loads and frees the three actual cursor values in ascending order.
`singleton_lifetime_free` is the existing matching `00BF65AC` host-CRT service and
calls `std::free`. Cells and embedded bytes are left unchanged. The caller must
finish cursor/backing consumers before this explicit operation; the enclosing
target storage remains caller-owned. The optimized Win32 listing retains the
fresh load/call/increment loop without clearing the cells.

## Native evidence and Ghidra limitation

The constructor's complete listing has 34 instructions. Its allocator call is
`007830A2 -> 00BF681B`. The destructor has 15 instructions, including returning
`00783106 -> 00BF65AC`, loop continuation at `0078310B`, and final RET at `00783118`.
Original disk bytes and live bytes were independently reviewed by the primary.

The primary cleared the incorrect local call-return override and decoded the
verified destructor tail. Stored Ghidra function-body metadata still ends at
`0078310A`; this is not a full Ghidra body/flow repair. The complete native listing,
not truncated pseudocode, establishes the reconstructed source boundary. See
`reports/transport_buffer_destructor_flow_recovery_cc11.json`. Bridge script
execution was not enabled and no body-extension mutation was made.

## Connected verification

The fresh MSVC Win32 fixture builds the new source plus 20 existing translation
units with `/O2 /MD /W4 /WX /fp:strict` and an embedded `asInvoker` manifest. Its
one adapted D2 case places the real member at existing target storage `+10h`,
checks all three distinct allocations and actual `D40/D44/D48` cells, and scans
all backing bytes to prove that only each first byte was cleared. It also checks
the enclosing target bytes remain unchanged.

The existing complete D2 constructor/profile, routing, target lock, serializer,
transport flush and owned enqueue operate on the newly allocated delivery-1
cursor. The fixture verifies the `380h`-byte payload, cursor reset, untouched
delivery-0/2 cursor state, and queued payload survival after overwriting all three
backings. It then explicitly destroys the cursors with consumers quiescent and
checks the profile stamp, retained cell/backing bytes, and surviving queued data.
Ascending release order is established by native and generated assembly inspection;
no replacement allocator/free hook is introduced to observe it.

The reused fixture passes 191 checks, 17 fixture cases including the one adapted
member case, and 72 retained original x87 kernel comparisons from transport
verification. Those kernel comparisons cover the existing timestamp wrapper, not
the buffer pair. All 63 source/header snapshot and workspace inputs and all three
copied/main support libraries match before and after linking. Artifact paths and
SHA-256 records are in `reports/cc11_session_transport_buffers.json`.

Null/throw injection, historical CRT identity, concurrent lifetime changes, full
target/history/derived construction, native executable profile dispatch, native
whole-body ABI, socket transmission and gameplay are not verified. Main CMake
registration, primary integration and the full main build remain the primary's
responsibility.

Primary integration 2efd789db1bb71e592baec015def28e79ff67fd8: full MSVC Win32 and all three existing CTests passed. The reviewed fixture passed fresh actual-main-TU compilation/run, with inputs stable pre/post link. Machine014C/resource24/id1/asInvoker verified and native direct CALL rows passed. All scope/provider/lifetime/CRT/FP/stored-Ghidra-body/ABI/game limits above remain explicit. Executable SHA256 8f80de24aae8f4bcfebc23efae4ae16fc487698379111084120ffa5f9879d664. Probe local/cc11_buffers_root.cmd; build log local/cc11_buffers_landing_integrated_build.log.
