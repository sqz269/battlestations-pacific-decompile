# Actual-pool wide-string resize

The new `resize_native_wide_string_header_004c53e0` overload accepts the
caller's SAME `NativeStringRawPoolContext` (AA8/AA4/AA0 reference identities).
It operates on the actual8h length0/data4 header without constructing or
casting a NativeString, wide string, owner or semantic container. Original
004C53E0 is thiscall(ECX header, stacked DWORD length/byte preserve), RET8;
this ordinary C++ overload does not reproduce that ABI, FH3 or fault behavior.

The old `NativeStringStorage&` overload and every other existing HPP/CPP body
remain literal unchanged, verified by whole-file inverse projection. In that
old interface release is noexcept; ActualNativeStringPoolStorage uses genuine
same cells but terminates on a getter escape. The added overload instead calls
the real raw419CC0/BD1120/BD1510 services directly and lets getter exceptions
propagate. It creates no adapter, callbacks, publication or allocation domain.

Native schedule and qualifiers:

- Capture initial length. Equality returns before any pointer/getter read.
- Zero request captures old data and initial2len+2 before419CC0, returns only
  when nonnull, then clears data before length after normal return.
- Nonzero request resolves419CC0 before allocating wrapped2requested+2 through
  actualBD1120. Preserve reads CURRENT length and data after allocation, takes
  the unsigned minimum before doubling, then uses the qualified overlap-aware
  memmove convention. Reached operands are read even at zero copy length; only
  that zero-byte call is omitted. No native callee-entry claim follows omission.
- Reread CURRENT old data after copy; when nonnull read CURRENT length/size
  before a NEW419CC0 lookup. BD1510 receives the actual AA4 reference after that
  getter and retains its own current large/disabled-return policy. Never cache
  a pool or skip this lookup for large blocks or disabled small returns.
- Only normal return publishes captured new data, requested length, then a
  WORD NUL at captured block+2requested. Size/address arithmetic wraps in Win32.

New helper reads/writes use fenced byte copies, not typed field overlays.
Compiler fences preserve Source header access epochs; they do not supply
cross-thread synchronization. Actual pool/manager/cell/import lifetime,
disjoint stable backing and external serialization remain caller contracts.
No null/overflow repair, guarded OOM, local catch, rollback, buffer free,
header restoration or retry is added on escape. Keep any partial native graph
and provider/invocation custody; this overload supplies no recovery/exit proof.

The matched existing raw-pool, current CRT allocation/copy and zero-copy limits
are inherited, not Original static-CRT/new-handler/library-body equivalence.
Valid reached buffers/header extents are required. Hardware faults, SEH/native
EH/double-failure/private-frame aliases and native ABI remain unproved.

Evidence: saved004C53E0 assembly53E0..548E; Source base6824d59e8; sealed
readiness reporte28ba78e. Report JSON pins actual providers and strict object
compile/inverse evidence. No tests, executable, game or Ghidra mutation.
This closes only the raw wide-resize exception boundary. RawBDEF90/0Ch-vector,
raw16 insertion, full parser/loading/startup/Font admission remain separate.
Source-ready, runtime, game, visual, Page/P4 and production claims remain false.
