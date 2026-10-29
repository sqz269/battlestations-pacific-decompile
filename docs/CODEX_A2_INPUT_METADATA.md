# Input metadata before startup shader loading

The startup host now prepares its sole `InputServices` immediately after the
successful shadow-target startup and before the A1 scene-domain call at the
73DD12 position. Both `input_runtime_` publication and `core.startup()` stay at
73DD8E. An early resource sampler can therefore borrow the same `input_->core`
without constructing a native input backend earlier.

This is a Source ownership change, not a reconstructed native body or ABI.
Original 73D410 calls 73BF80 at73DD12 and constructs input at73DD8E. No native
shader preload is added by this patch.

The prior input-only audit follows `InputServices` in game_hosts.cpp,
GameInputRuntime::Impl, NativeInputDeviceRuntime, the default-empty SDK and
DirectInput trackers, and GameSingletonHost::bind_input_backend/actions. The
metadata constructor captures existing constants, borrowed contexts and the
selected XInput function pointer, queries processor SSE2 capability, then binds
the two deletion contexts by nonthrowing pointer stores. It neither publishes
`input_runtime_` nor creates/registers a native backend or action owner.

The real frame clock, XInput DLL, platform, SoundServices/XLive/cursor cells and
singleton host are already established here. The existing shutdown retains
them through raw drain, then releases SDK state and input metadata. Preparation
does not move before the factory/platform boundary, copy a context, create a
second backend, or retarget a later sampler. Before actual backend startup the
cursor helper observes the canonical null backend and returns before cursor
lookup/reset; the full loading message pump remains effectful and requires its
own runtime acceptance.

MSVC Win32 Release and both existing CTests passed. Log:
`local/codex_a2_input_metadata_build.log`. No new tests were added. This patch
has source/build evidence only; startup timing, queued callbacks, input/gameplay
parity and the eventual real shader-preload path remain unproven. A2 still
requires shared early procedural factories and retained sampler/effect/cache/
Application-row ownership through shutdown.
