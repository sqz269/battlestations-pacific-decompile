# OCT18B A1: retained mission render resources

This packet composes existing producers for the opt-in ordinary-command ship
milestone. It adds no unique function credit. The primary renderer application
owns switch handling, facade entry, frame sequencing and integration. The graph
must only be instantiated on `BSP_NATIVE_MISSION_SHIP=1`; its constructor creates
host metadata, and `construct_and_initialize()` performs native allocations.

`MissionRenderResourcesGraph` is included by
`src/game_native_renderer_resources.inc`. It borrows the original application
renderer, VFS, pooled strings, manager, synchronization, texture/surface pools,
geometry/actual-owner registry, declaration/layout providers and camera domain.
Its material cache, material lifetime and model environment come from the same
mission model/effect application. No parallel cache, simulation world or native
owner count is introduced.

## Binary dependencies verified before wiring

The null service is supported by `00B2B200`'s diagnostic-record clear and by
`00B13510`'s label reset. It is **not** supported by the complete ordinary path:

- `00B1D9AA` calls `00B46A70` before the optional diagnostic-label branch.
- `00B46A7B` reads `00F8D39C`; `00B46A88` calls `00B0D100` and copies the service
  matrix at `+1D8`. `00B46D1A/00B46D36` obtain the service region at `+84` and
  consume its time fields without a null check.
- `00B14A10` constructs the real `6ACh` service and publishes it through
  `00B0F020`. Its constructor leaves `+70` and the matrix at `+1D8` unwritten.
- `00B0F6E0`, reached by the genuine `00B14F60` destructor, releases `+70`.
  `00B107F0` supplies it. A constructor-only service cannot be admitted to
  ordinary native drain by inventing a zero preimage.
- `00B0D120` is not a setter entry; it lies within the `00B0D100` candidate.

The full existing `initialize_native_render_resources_00b107f0()` composes its
normal `[00B107F0,00B13010)` stages. A1 retains that operation and every nested
post20/post24/pass/luminance/bloom/distortion/dust acquisition record. It binds
the service's optional direct-terminal domain to the same completed holder,
runtime texture and surface producers before entry.

## Matrix producer

`prepare_screen_matrix()` extracts two already reconstructed spans from
`native_main_scene_execution_prefix.cpp`, without admitting `D5E13C`:

| Span | Native work | SHA-256 of installed/live bytes |
| --- | --- | --- |
| `00B1586D..00B158D7`, 107 bytes | Capture height texture first, width texture second; original unsigned x87 half-texel conversion and float32 spills/reloads produce service `+4/+8` | `5801be66ee96733a5e63bc7c1c5fc502cb379b103f0758777f80f51249c56786` |
| `00B15D1D..00B15DDB`, 191 bytes | Original non-lens one/half/zero setup and complete ordered matrix stores `+1D8..+214` | `1f72db3fb0658c51c8a1b69763efe4b170d0cfb9bea749ce24dfb335c3e93021` |

It rereads the actual service `+4C` holder, current texture profile slots and
read-only constant cells for each invocation. It preserves positive zero,
unsigned conversion, x87 spill/reload timing and the native half-texel result.
It changes no main-scene camera clear flags or render targets. Its inputs are
genuine completed native resource owners, not host fallback textures or an
invented matrix default.

`bsp.py ghidra` calls run `Client.verify()` before their query; the configured
project was `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
language `x86:LE:32:default`, image base `00400000`. The live count matched the
76774-function snapshot. The two complete spans were read through the verified
CLI and compared byte-for-byte against the configured installed PE. The local
receipts are `local/oct18b-a1-byte-proof.json` and the two `*-live.txt` captures.
This is byte provenance, not an Original/source differential execution claim.

## Integration and lifetime contract

The facade must admit entry only in its ready phase, over the borrowed **full**
platform/configuration owner. Downscale initializers read actual `0109CF04`
`+24/+28`; the legacy focus-only `+30/+41` projection is insufficient.

Construct the mission model, sampler, effect-cache and ordinary-command metadata
first. Pass their stable cache/material/model/render-access references into the
resource graph. A shared native shader-cache bracket may encompass A1 and the
model acquisition; A1 borrows an already-live bracket. If none is published, it
retains its own new `00B3A600` operation and normally releases only that exact
created owner through `00B3B140`. Failures retain the whole graph and bracket.
Lua calls run inside the existing renderer binding's activation scope.

The existing distortion constructor preserves `+34/+38/+3C`. The original
capability-failure terminal could read those allocation preimages. A1 therefore
admits only the same serialized renderer's successful native format-capability
domain, checked before allocating the service. It adds no zero stores or false
initializer result. A generation change/reentrant destruction during resource
construction remains outside that admission; failed attempts require process
retention. The current native creation/recreation providers and publication
cells are shared with `DeviceGraph`, rather than copied device pointers.

`requires_process_retention()` rejects incomplete operations and an unclosed
owned shader bracket, while allowing normal live completed native owners.
Application retention must still reject an active layout acquisition, but must
allow the mission graph's managed live layout companions after its initialization
completes; otherwise the native owners cannot reach their actual drain.

Before each ordinary system-prefix gather, invoke `prepare_screen_matrix()` in
the application's serialized frame path. Keep the resource, model, material,
cache and all borrowed provider objects alive through genuine singleton drain.
After drain, `after_native_drain()` first checks that the service publication is
null, then closes metadata through the existing child blocks' terminal checks.
`NativeRenderResourceInitialization::close_children_after_host_quiescence()`
performs no native release/free/retry and leaves the operation one-shot.

The ready-only `GameNativeGuiEffectApplication::native_effect_cache()` accessor
validates the same canonical domains before lending its cache. Callers own their
retained acquisition frames, Lua activation and serialization obligations.

## Validation boundary

Source and lifetime contracts were reviewed; `git diff --check` passed. Primary
integration runs the required Win32 build and mission checks. No game process
was launched by A1. This packet does not establish drop-in ABI/FH3/SEH identity,
all failure/reentrancy paths, device-loss gameplay, complete main-scene rendering
or a visible installed ship. Original `00B14A10` is ECX receiver/no stacked
arguments/EAX same receiver/RET; `00B107F0` is ECX service/three stack DWORDs/
RET0C. The retained C++ interfaces and extracted fragments use new source ABIs.
