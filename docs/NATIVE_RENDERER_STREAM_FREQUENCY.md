# Native renderer stream frequency

The full `B24A40..B24AF7` body (183 bytes) uses the actual renderer and the
existing actual synchronization providers. The original interface is ECX
renderer with stack stream/value and RET8. The new C++ entry adds explicit
synchronization context and does not replace the original caller ABI.

Entry conditionally records the renderer and calls full `B33AD0`. It reads the
cache DWORD at `(stream+178h)*16+renderer` before arming cleanup (`B24A8E` before
`B24A90`). Equal values skip the COM call. Changed values publish the cache
before loading the current device at renderer+1A10 and its current table+198
`SetStreamSourceFreq` method. The raw stream/value are forwarded, including
wrapped index arithmetic. HRESULTs are ignored and no call counter exists.

Both returning branches read current mode before disarming cleanup, then use
the saved renderer and full `B33B00`. A thrown COM observer preserves the cache
and other earlier writes while invoking full `B21110` cleanup. The skipped
entry record remains uninitialized; no default renderer or taken flag repairs
it. Native `CBD038` selects `DF56C0`, with one unwind state and funclet `CBD030`
passing the guard at EBP-14 to `B21110`. The established actual-provider policy
terminates a second C++ exception during unwind and leaves other SEH handling
to the caller.

Complete source passes the strict Win32 build and both existing CTests.
Original-code fixture checks remain pending.
No drawing, original-caller ABI, full reset or gameplay validation is claimed.
