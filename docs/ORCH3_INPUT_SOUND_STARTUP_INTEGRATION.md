# Input polling and sound startup integration

Addresses: 00a92c40, 00a92370, 00a922a0, 00a92090, 00a91d60, 00a7c350, 00a7c3f0, 00a7c460, 00a7bbe0, 00a7c2c0, 00a7ff80, 00a7de10, 00a7e0d0, 00a7c740, 00a7f9f0, 00a7ae00, 00a7ae80, 00a7b120, 00a81480

This batch combines three disjoint worker packets: full action-binding polling,
sound-class ownership and Lua-driven sound configuration. The primary integrator
uses `agent/orch3-20260910`, separate from the other orchestrators on main.

The input manager calls recovered rebind/poll functions directly, replacing the
two host callbacks. Each enabled action shifts state once inside the full poll,
then classifies its listener; dirty rebinding includes disabled records. Record
float fields +1C/+24 are previous/current input values, correcting the earlier
hold-duration interpretation. Device virtual methods and the CRT square-root
policy remain explicit boundaries. The original native differential compared
6000 poll cases and 400 paired-value cases, including service call order, with
NaN payloads and nondefault FPU/CRT exception modes excluded.

The audio configuration constructs retained class descriptors, groups/DSPs,
listeners and type routing through existing state types. Table growth adds null
slots; it does not create class objects. It preserves Lua iteration order,
strict numeric defaults, case-insensitive lookup and native error continuation.
An independent review checked class reference transfer, Lua cursor lifetimes,
advanced-setting offsets and DSP defaults. A local real-Lua fixture exercises
error continuation and multiple type entries. Installed Lua plus the actual
FMOD 4.18.04 DLL configured 12 groups and 11 DSPs without FMOD errors; see
`INSTALLED_SOUND_CONFIGURATION.md` for the concrete proof and its boundaries.

False no-return annotations on calls to `_free` had hidden cleanup and loop
instructions in 00a7ff80, 00a7f160, 00a7bbe0 and 00a7c460. The primary repaired
those flow gaps under the Ghidra write lock, saved the project and refreshed
exports. The restored tail in 00a7ff80 includes the SoundTypes loop back edge.
Four dedicated repair reports preserve the old gaps and installed-byte evidence.
Nineteen reviewed names/evidence comments were saved with previous comments
preserved and affected exports refreshed. The 00a81480 constructor name remains
provisional: only its established defaults are reconstructed in this batch.

All four new sources are registered in `cmake/startup.cmake`; no temporary worker
CMake include is required by the integration checkout. MSVC Win32 Release and
both existing CTests passed. The ownership and input worker probes are retained
under the integration checkout's `local/` alongside their logs. No broad test
suite was added. These are typed behavior projections with focused numerical
and library-runtime evidence, not a running or ABI-compatible game rebuild.
