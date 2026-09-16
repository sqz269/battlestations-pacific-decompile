# Actual renderer 40-byte record append

`append_native_renderer_records40_00b257b0` borrows the existing 12-byte raw
header. It preserves capacity capture, equality-only growth, signed comparison
of the wrapped doubled capacity, and the existing B22A70 reserve provider.
After reserve it reloads used before data, computes the destination with DWORD
wrapping, and skips input reads when that computed address is zero. It still
increments the current used word after the copy or skip.

The copy interleaves DWORDs at +00/+14 with eight explicit x87 FLD/FSTP pairs.
Source and output remain live throughout; overlapping records or a destination
that aliases the header cannot be replaced with a snapshot or memcpy. The
current floating-point environment controls conversions and exceptions.

`append_native_renderer_debug_record40_00b29330` takes an actual renderer,
the shared borrowed guard context, and the original ten argument values.
Its eight numeric arguments are supplied as DWORD bits. It captures the guard
owner's actual +04 section, enters and increments that section, initializes the
local record, and retains a nonnull opaque owner through its real +04 interlocked
counter. MOVSS captures preserve the native argument order, including the early
color read and late color store. Append receives the actual renderer +1D18 header.

The opaque owner receives no invented type or terminal. The native unwind map
has only state0 guard cleanup through CBD2D0 -> 411EE0. The source therefore
reuses that real guard cleanup for C++ exceptions and does not compensate the
earlier retained-reference increment. Normal exit decrements and leaves the
captured section directly. The guard publication and manager are the same cells
borrowed by the separate R53 provider; this module creates neither owner.
State0 remains armed through the normal leave operation. A secondary source
C++ exception during guard cleanup terminates instead of replacing the first.

The interfaces do not reproduce the native private stack, FH3/SEH, hardware
fault delivery or arbitrary concurrent mutation. Valid reachable storage and
the native floating-point environment remain caller preconditions. See the
R54 report for exact byte/call receipts, the focused append fixture, strict build
results and frozen artifacts. Public renderer composition and active application
ownership are separate validation boundaries.
