# Actual renderer shader binding

This packet supplies complete raw-storage source for the existing semantic
pixel and vertex shader binders. It preserves their current Ghidra names and
earlier typed implementation. The names describe observed behavior; they are
not recovered symbols. This is an upgrade of two previously recorded functions,
with zero new unique function credit.

| Native body | Logical cache | Post-call counter | Device virtual slot |
| --- | --- | --- | --- |
| `00B21C20..00B21D01` (226 bytes) | renderer `+176C` | `+1BBC` | `+1AC` SetPixelShader |
| `00B21D10..00B21DF1` (226 bytes) | renderer `+1770` | `+1BC0` | `+170` SetVertexShader |

Both native entries take the actual renderer in ECX, one logical shader pointer
on the stack, and return with RET 4. The new source interface adds the shared
actual synchronization globals in EDX. Its naked wrapper passes the address of
the original callee argument slot to the body. The body enters the real optional
renderer guard, then reads the old cache and that argument slot in native order.
It arms cleanup only after those two reads.

Two non-null logical wrappers with equal physical COM pointers at `+8` retain
the old cached wrapper and skip the API and counter. Otherwise the incoming
logical pointer is published first. For a non-null incoming wrapper, the current
device at renderer `+1A10` is captured, then incoming `+8` is read again, followed
by its current device vtable and shader slot. Unbinding captures the current
device, vtable, and slot before pushing zero and the device. A null/null call
still writes zero to the logical cache, but does not call Direct3D or increment
the counter. The DWORD counter wraps and increments after the COM call returns,
regardless of HRESULT. There is no shader AddRef or Release.

Cleanup uses the existing actual optional-guard providers (`00B33AD0`,
`00B33B00`, and `00B21110`). It reads the current mode after the body, consumes
normal cleanup before calling leave, and preserves the guarded renderer. The
saved entry result is ignored by the original leave provider. An entry-disabled,
exit-enabled transition exposes uninitialized native guard storage and remains
outside this source interface's valid domain.

The installed FH3 maps each contain one state: state 0 unwinds to -1 through a
guard-destroy funclet. Pixel uses handler `00CBCE18`, info `00DF53CC`, map
`00DF53C4`, and funclet `00CBCE10`; vertex uses `00CBCE38`, `00DF53F8`,
`00DF53F0`, and `00CBCE30`. Each funclet forms `[EBP-14h]` and jumps to
`00B21110`. The source implements the corresponding cleanup boundary with the
existing cleanup-exception policy. It does not reproduce native FH3 metadata,
private stack layout, hardware-exception behavior, or binary unwind identity.

Evidence is pinned in `reports/native_renderer_shader_binding_bx.json` to the
installed PE, current Ghidra bytes, source/provider hashes, exact committed
Win32 build, and generated-code review. No new test is added. Existing typed
shader probes do not count as execution of these raw entry points. Native
execution, ABI substitution, complete material-pass composition, rendering,
and gameplay validation remain unclaimed.
