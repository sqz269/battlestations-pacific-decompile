# Default surfaces and depth binding

The renderer probe now uses a typed capture projection of00b238d0 instead of
directly constructing only a color binding. D3D9DefaultSurfaces uniquely owns
both color and depth COM references and releases them on destruction. Native
intrusive wrapper construction, allocation and registration remain unported;
capture is recorded as a fragment, excluded from whole-routine coverage.

Capture gets render target0, performs the native extra GetDesc, and initializes
the replacement through the existing00b3cc80 helper, which retains the surface
and queries its description. The replacement is installed before the old owner
is released, then the getter reference is dropped. Depth is acquired/replaced
next, followed by depth binding. Reacquiring the same COM surfaces is supported.
Both wrapper flags and recreation-kind are zero, matching both native
00b3f630(surface,0,0) calls, including depth. Default surfaces are reacquired
during reset; do not infer recreation-kind from depth format or pass these
captured defaults to the generic offscreen-surface recreation path.

The depth setter reconstructs00b21690: ECX renderer, wrapper pointer on stack,
RET4. It enters the existing optional guard and always calls SetDepthStencilSurface
with the wrapper's pointer, or null for a null wrapper. Non-null wrappers increment
the unsigned32-bit counter even when their COM pointer is null or the call fails;
null wrappers do not increment. It has no identity cache or retained wrapper.
The new interface exposes HRESULT, whereas native ignores it.

Capture has no whole-operation guard; its final depth setter guards itself.
Callers must coordinate owner access and retain device/synchronization/lock
lifetimes. Failures preserve preceding successful changes: depth-get failure can
leave color replaced. Temporary references are cleaned up on initialization
failure. Native unchecked failures, wrapper registry side effects and redundant
balanced accessor AddRef/Release pairs are outside this projection. The new
owner cannot be shallow-copied; its cleanup is not the native wrapper destructor.

## Evidence and validation

RENDERER_INIT_HANDOFF records complete disk/live byte hashes for capture
(562 bytes) and depth binding (162 bytes). Each analysis/export batch verified
`bsp`, `/battlestationspacific.exe`. Both evidence comments were updated with
previous annotations preserved, the program saved, and exports refreshed.

Build and both existing CTests pass. The existing D3D9 resource check captures
both defaults twice, verifies retained depth identity/descriptions, binds a null
wrapper and a non-null wrapper holding null, then restores depth. It observed
four counted non-null calls, balanced guard/critical-section state and zero
recreation-kind/flags for both defaults. Dynamic buffers, separate surface
recreation, shader/material draw readbacks, camera comparisons and installed
DDS/atlas checks also passed in that run. Results are retained in
`reports/default_surface_probe.txt`; no test target was added.

This is a real-device ownership/binding check, not a native execution differential
or proof of complete startup/reset behavior. Registry participation, thread
ownership, focus-gated reset and resource restoration remain dependencies.
