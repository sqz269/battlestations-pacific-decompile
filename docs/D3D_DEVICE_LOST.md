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

## 3. The switch (a deliberate divergence, committed OFF)

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
