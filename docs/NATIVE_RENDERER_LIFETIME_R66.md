# Canonical hardware tree and full renderer lifetime (R66)

Addresses: `00CD7960`, `00CE0C50`, `00B32410`, `00B2AEB0`, `00B2ABD0`,
`00B29670`, `00B32920`, `00B339F0`, `00B32900`.

## Production change

`GameNativeHardwareLayoutTreeProcess` now owns the actual 12-byte `0108D530`
header and a process-lifetime invalid-parameter binding to the current source
CRT. Its complete native initializer `CD7960` (64 bytes) and shutdown callback
`CE0C50` (64 bytes) were already reconstructed. Startup now calls that initializer
after the particle pools and before the graphics pools, matching their relative
CRT order; the original tree entry is at `CE3504`.

The original header is wholly in the PE's zero-filled region; live Ghidra also
shows twelve zero bytes. Initialization builds the real 28h sentinel and its
self-links, retains the actual atexit result and does not roll back a failed
registration. Completed startup is cached; an exception cannot be retried.
Bookkeeping construction finishes before native callback registration, so the
header and invalid-parameter binding survive actual CRT tree cleanup. Its C++
destructor does not repeat native destruction. A returning current CRT invalid
handler retains the native continuation; invalid inputs were not exercised.

## Full reconstructed renderer execution

Current D3D9 HAL creation works. A focused source fixture now runs two complete
`B32410` constructor / `B2AEB0` device-startup / `B32920` normal-destruction
lifetimes. The second retirement uses the canonical singleton manager and the
`B32900` secondary / `B339F0` scalar dispatch with flag one.

The fixture borrows the real application VFS, raw string pool, singleton
manager, Lua services and child publications. It uses the production surface,
texture and ten R65 graphics pools plus the new canonical tree. Device startup
creates a real HAL device and default surface owners, initializes four stream
frequencies, and allocates distinct real **16 MiB vertex / 1 MiB index** buffers
through the canonical physical pools. Their COM descriptors are checked.

Both full destructors join the actual control thread and clear renderer/child
publications. Shared allocator trimming verifies both physical slots were
returned. At actual process exit, the tree and all twelve pools finish their
real CRT callbacks before a post-cleanup observer checks the still-live
bookkeeping. Repeated tree startup preserves its sentinel.

This executes the reconstructed C++ bodies; it does **not** execute copied
original renderer machine bodies or prove their original register/FH3/SEH ABI.
The fixture holds explicit external COM references so receivers remain valid
through the destructor's later diagnostic AddRef/Release pairs. Production
composition must establish the same valid lifetime domain.

The hidden window is actually unfocused. `B2ABD0` therefore retains pending=1;
no focused Reset or `B29670` recreation occurs. The latter is bound to its genuine
complete providers and the same tree/pools, but is unexecuted here. Gamma is
enabled with cached/requested zero, so the equal-cache arm runs without power or
ramp generation. Active renderer callbacks, models, retained textures, shader
drawing, online callbacks and failure/EH paths remain open.

## Application run and R65 crash clarification

The exact installed `xlive.dll+319049` failure was already explained in
[XLIVE_STUB.md](XLIVE_STUB.md): the AlterBSP DLL patches fixed original-game
offsets while loading. R65's observation matches that known problem.

The four Microsoft files in [XLIVE_PRIVATE_RUNTIME.md](XLIVE_PRIVATE_RUNTIME.md)
still match the recorded SHA-256 values and pass Authenticode verification.
Using the existing `--xlive-dll` option and explicitly preloading its matching
`msidcrl40.dll` through `--xlive-dependency` lets the current application create
a window/device, present **two frames**, save a 640x480 screenshot and exit zero.
The new tree initializer logs atexit zero. No stub DLL or installed-file change
was used. Settings use an isolated local root; the established save-directory
code still references Documents/Battlestations-Pacific/save.

The screenshot visibly contains front-end artwork and a sign-in/continue
prompt from the current installed assets. This is the existing milestone
frontend bridge. Its device/render loop still contains documented unimplemented
services; two frames are not native-frame, visual-parity or gameplay proof.

## Evidence and follow-up

Strict MSVC Win32 build and all three existing CTests pass. No permanent test
cases were added. Fresh original bodies, context/FH3 data, call-site checks,
fixture source and logs, linked source intervals, physically resolved I386
modules and runtime identities are retained in the
[report](../reports/native_renderer_lifetime_r66.json). Seventeen missing
instruction starts in `B29670` are unreferenced alignment LEAs skipped by
unconditional jumps; no executable listing repair was needed this batch.

Exact source/probe/compiler/build evidence is sealed before integration, and
the combined integration build is recorded separately. R64's hardware limit
is historical; this packet supplies successful current source lifetime evidence.

Next: move the verified context composition into the application using the same
canonical publications/pools/tree and valid COM ownership; then replace its
milestone device/frame path. Bind actual platform, renderer control callbacks,
scene owners and resource consumers before claiming complete application or
gameplay reconstruction. The lifetime fixture is a prerequisite, not that wiring.
