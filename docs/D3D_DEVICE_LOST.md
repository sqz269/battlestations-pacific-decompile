# Lost D3D device: the recreation crash and a hold switch (packet `cc9_d3d_device_lost`)

Addresses: 00B2ABD0 00B29670 00B24460

Worker cc9-gunnery14, 2026-09-29. Names are descriptive hypotheses, not recovered symbols.
Background: docs/DEVICE_RESET_SCHEDULER.md (the decision table of 00B2ABD0).

## 1. The crash

- **Where it came from.** Reference p's first batch lost the session's audio and display at
  10:23 local. Three in-flight runs died with c0000005 at `bsp_game.exe+280635`, reading 0.
- **The stack.** A heuristic scan of the minidump (`local\g14_dmp.py local\rb16_ijn01.dmp
  local\rb16\build\win32\bsp_game.map`) gives:

  ```
  set_native_renderer_render_state_00b24460 + 0xD5          (the read through [renderer+1A10h] = 0)
  recreate_native_renderer_device_00b29670 + 0x872
  process_current_request (00B2ABD0's body) + 0x2EC
  process_native_renderer_device_reset_00b2abd0 + 0x97
  begin_native_renderer_frame_00b2b200 + 0x65
  GameNativeRendererApplication::begin_frame, GameDeviceHost::clear_and_present, GameLoopCallbacks::frame
  ```

## 2. The host is faithful here; the image has the same crash

1. **00B2ABD0's DEVICELOST branch** (00B2AC7D..00B2ACC6) counts lost polls in `[0108D4C4]` and sets
   `renderer+1D8Ah`. Past 10, with the platform active and `GetFocus()` equal to its HWND, it zeroes
   the counter and calls 00B29670 (DEVICE_RESET_SCHEDULER.md).
2. **00B29670** releases every device resource, then does the following:
   - releases `+1A10h` (00B29880..00B29888) and stores 0 (00B29897);
   - calls `IDirect3D9::CreateDevice` through `[+1990h]` vtable `+40h` (00B298EE);
   - **does not test the HRESULT**, and immediately calls `00B24460(0A1h, ...)` at 00B29903, which
     reads the device's vtable.

   A CreateDevice that fails leaves `+1A10h` at 0. That happens while the session cannot own a
   device: a locked, disconnected or switched RDP session, with DEVICELOST. So the image
   dereferences null at the same instruction.
3. **The host matches.** `src/native_renderer_device_recreation_actual.cpp` reproduces that order,
   with the comment "Native does not ... branch on HRESULT". It is not a host divergence.

## 3. The switch (a deliberate divergence, ON since section 5)

`kRendererLostDeviceHoldBound` (`include/bsp/native_renderer_reset_process.hpp`):
- **ON:** the DEVICELOST branch still counts and marks the renderer lost, but never calls 00B29670.
  The device is kept, BeginScene stays skipped (`+1D8Ah`), and TestCooperativeLevel is polled every
  frame.
- **Recovery** comes from DEVICENOTRESET and the image's own Reset path (00B2ABD0 after the focus
  gate): release, `Reset`, restore, render states, gamma.
- **Still reachable with ON:** 00B29670 stays the fallback when Reset itself fails
  (a non-zero return from 00B2AD6B, through the 00B20C50 gate), so a Reset failure while the device cannot be created would still crash. That is not
  handled here.

## 4. Validation without touching the session

**The diagnostic.** `BSP_RENDERER_FAKE_LOST=<first>,<count>[,createfail]` (env-gated, never set in
reference runs):
- from the `<first>`th call of the request body, `<count>` polls read DEVICELOST and the next reads
  DEVICENOTRESET;
- the global lost byte is set on each;
- `createfail` makes 00B29670 skip CreateDevice, leaving `+1A10h` at the stored 0.

**The counters.** A new line, `summary native renderer lost device:`, counts lost polls, holds,
recreations, CreateDevice failures, resets and faked polls.

**The runs.** USN01, 600/400 frames, `local\dl*` logs. Builds: `local\dloff` (`6f841fce6`,
SHA-256 prefix `A6D2F13841A2`) and `local\dlon` (the same, flipped, prefix `DC4FE6C61152`).

| run | fake | exit | lost polls / holds / recreations / resets | frames presented |
| --- | --- | --- | --- | --- |
| `dl_off2_none` | none | 0 | 0 / 0 / 0 / 1 (the startup Reset) | 599 |
| `dl_off2_cf` | 200,20,createfail | **c0000005** at mission frame 171 | - | - |
| `dl_off2_rc` | 200,20 | **c0000374** (heap corruption) after mission frame 400 | - | - |
| `dl_on_none` | none | 0 | 0 / 0 / 0 / 1 | 599 |
| `dl_on_cf` | 200,20,createfail | 0 | 20 / 10 / 0 / 2 | 577 (23 skipped while lost) |
| `dl_on_rc` | 200,20 | 0 | 20 / 10 / 0 / 2 | 577 |

- **`dl_off2_cf` reproduces the incident.** The minidump of the matching first run (`dl_off_cf`)
  scans to the same `00b24460 + 0xD5` under `recreate_native_renderer_device_00b29670`.
- **`dl_on_*` recover.** After the tenth lost poll the hold returns; the faked DEVICENOTRESET then
  takes the image's Reset path (resets 1 -> 2) and rendering resumes.
- **The switch changes nothing without a loss.** `pair_diff dl_off2_none dl_on_none` exits 1
  (gameplay identical). `dl_on_none` against `dl_on_rc` also exits 1, so the lost frames do not
  touch the simulation.

**A second finding, open.** When 00B29670 **succeeds** (`dl_off2_rc`), the mission completes, but
the process then dies at teardown with STATUS_HEAP_CORRUPTION.
- The host keeps its own references to the first device, which the image's recreation does not know
  about:
  - `GameNativeRendererApplication::Impl::retained_device`, which holds an extra AddRef
    (`src/game_native_renderer_application.cpp`);
  - the font and frontend hosts built on `*device_->device()` (`src/game_hosts.cpp`).
- So recreation in this process leaves those holders on a stale device.
- Not fixed here. With the hold ON, recreation is not reached on a plain loss.

**Not validated:**
- a real session lock or RDP switch (never done, by instruction);
- a real DEVICENOTRESET after a real loss;
- the image's behaviour at runtime (the crash is inferred from the listing, not observed in the
  original executable).

## 5. The flip

`kRendererLostDeviceHoldBound` is **ON**, approved by the lead as a labelled robustness divergence:
- Without a loss it is gameplay-identical (section 4).
- On a loss, the image's own behaviour when CreateDevice fails is a null dereference. The hold keeps
  the device and recovers through the image's Reset path instead.
- The image's recreation 00B29670 stays reachable only as the Reset-failure fallback (section 6).

## 6. The teardown heap corruption after a recreation (packet `cc9_d3d_recreate_holders`)

**The cause: a host-only reference.**
- `GameNativeRendererApplication` AddRefs the first device as `retained_device` and releases it in
  its destructor. The image holds no such reference.
- When 00B29670 replaces `+1A10h`, that reference stays on the old device. The process then dies at
  teardown in `destroy_singleton_lifetime_manager` with c0000374.
- This is reachable with the hold ON as well: 00B29670 is the fallback of a failed Reset.

**Bisection.** Each run was a single USN01 run at 600/400 frames with
`BSP_RENDERER_FAKE_LOST=200,20,resetfail`, the new option that sends the faked DEVICENOTRESET
down the Reset-failure fallback.

| variant (local experiments, reverted) | exit |
| --- | --- |
| as committed before this packet (`rh_base_rf`) | c0000374 |
| 00B29670's layout, logical-buffer, shader, texture-record and physical-buffer walks skipped (one at a time and all together) | c0000374 (or an access violation where the skip leaves the renderer inconsistent) |
| `retained_device` follows `+1A10h` | **0** |
| the sprite bridge drawn with `GameDeviceHost::device_` pointed at the new device | c0000005 in d3d9.dll at mission frame 182 (the frontend's textures belong to the old device) |
| the sprite bridge skipped, `retained_device` stale | c0000005 in d3d9.dll at teardown |

**The fix** (`324ca9b81`, host-side, no native counterpart):
- `GameNativeRendererApplication::begin_frame` follows `+1A10h` after 00B2B200. It AddRefs the new
  device, releases the old one, and logs `native renderer device replaced by 00B29670:
  generation=N`.
- `GameDeviceHost::clear_and_present` follows the renderer's device.
- **Limitation, recorded:** the sprite bridge (the frontend and font hosts' textures, a host
  scaffold rather than the native GUI path) is not rebuilt on the new device. It is retired, with
  the note `sprite bridge retired: ...`. After a recreation that milestone overlay is no longer
  drawn.

**Validation:**

| run | build | fake | exit | recreations / resets | final COM release |
| --- | --- | --- | --- | --- | --- |
| `rh_fix_rf` | tree `324ca9b81` (hold ON) | 200,20,resetfail | 0 | 1 / 1 (+1 failure) | device=0 api=0 |
| `rh_fix_rc` | the same | 200,20 | 0 | 0 / 2 | - |
| `rh_off_rc` | `local\rhoff` (hold OFF, SHA-256 prefix `DBDFA132F170`) | 200,20 | **0** (was c0000374 in `dl_off2_rc`) | 1 / 2 | device=0 api=0 |
| `rh_fix_none` | tree | none | 0 | 0 / 1 | - |

`pair_diff dl_on_none rh_fix_none` exits 0 (identical).

**Two notes on running this diagnostic:**
- Concurrent runs compete for focus. 00B2ABD0's focus gate (`GetFocus()` against the platform
  HWND) then keeps a lost device unrecovered, as the image would. Three simultaneous faked-loss runs
  skipped 402 presents each.
- Run faked-loss checks one at a time.
