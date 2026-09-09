# Renderer state initialization

`src/d3d9_states.cpp` reconstructs five routines through a new C++ interface:

| Native address | Behavior |
| --- | --- |
| `00b24460` | Cached SetRenderState, optional renderer guard, call counter |
| `00b24610` | Cached SetSamplerState, vertex-sampler mapping, call counter |
| `00b26170` | Nineteen ordered render defaults and seven defaults for twenty samplers |
| `00b33ad0` | Enter optional tracked critical section and increment nesting |
| `00b33b00` | Leave optional guard and synchronize the observed mode byte |

The setters are thiscall, consuming two/three stack arguments respectively.
The default initializer uses ECX and consumes no stack arguments. Guard leave
consumes one stack argument but does not inspect it; the decompiler initially
omitted that argument. Assembly supplies the cache offsets and control flow.

Render validity bytes begin at object `+40h`, values at `+114h`. Sampler validity
starts at `+11cch`, with 72 bytes per bank; values start 16 bytes later. The new
cache separates validity and values into typed entries and does not reproduce
the complete renderer layout. Internal samplers 16..19 become API samplers
257..260; all twenty banks receive defaults.

The cache records requests before calling D3D9 and ignores HRESULTs, just as the
original does. Failed requests can therefore be cached. Counts measure API calls,
not successful changes. The new interface rejects out-of-range array indices.
It starts with invalid entries; `invalidate()` is a new interface utility for
future reset integration, not a port of the native reset implementation.

The shared synchronization projection stores native mode `0108d6dc`, observed mode
`0108d6dd` and nesting count `0108d6e0`. It uses the existing tracked critical-section
type. Enter increments nesting before entering the lock; leave decrements nesting
before unlocking. Both setters check the current mode at entry and exit, and leave
ignores enter's saved result. These semantics are preserved. Configure the mode
before starting workers: native concurrent mode changes and non-atomic counter
behavior are unverified. Native SEH is not reproduced by the C++ scope guard.

The real D3D9 probe runs defaults twice with locking enabled. It observes 19 render
calls and 140 sampler calls in total, balanced nesting/depth, and queries ZFUNC=LESS,
CULL=CCW, and linear minification on pixel sampler 0 and vertex sampler 257.
It does not query every default, exercise concurrency, compare original machine-code
execution, or establish rendering/gameplay equivalence. Existing CTest remains 2/2;
no additional test cases were added. Retained assembly and byte hashes accompany
the findings in `reports/d3d9_states_*`.

## Next dependencies

`00b238d0` obtains the render target and depth surface and constructs reference-counted
wrappers. Its pseudocode has broken stack tracking around COM calls, so wrapper
ownership must be recovered from assembly before porting. Constructors `00b4bbb0`
and `00b4bb60` install vertex/index buffer wrapper vtables `00d61e7c`/`00d61e58`.
Their attach methods and teardown remain to be traced.

`00b24a40` is the cached SetStreamSourceFreq helper; initialization sets streams
0..3 to frequency 1. It has a confirmed name but is not yet ported.

`00b2abd0` is the device reset processor. Initialization sets pending flag
`0108d4b8` then calls it. When the render thread owns execution and the platform
window is active and focused, it releases resources, resets the device with the
stored presentation structure, then restores resources/defaults. This explains
the initial windowed creation followed by a changed stored fullscreen flag.
The reset sequence and its lost-device recovery are not yet implemented.
