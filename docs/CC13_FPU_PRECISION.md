# x87 precision per thread during gameplay

Addresses: 00B2AFF9 00B298EE 00B2AD58 00B2ABD0 00B2AEB0 00B29670 00991EF0 00BF6A58 00C0682E
00C273F8 00C2D14E 00C2D21A 00C138A7 00C138C5 00C2CAE5 00BF69E7 00D693D8 00D693DC 00C03B44
00BF857A 00C0ED3C 0109DD7C 00C06893 00BF84E8 00C0843B 00E16460 00C28548 00BE2C70 00BE3150
00BE2FA0 00BE2EA0 00BE2BA0 00BE4800 00E0B6CE 008A1890 00875BB0 00825F20 00C37690 00C37680
00C33140 00C5BB30 00C57070 00C55EA0

Packet `cc13-fpu1`, worker `agent/cc13-fpu1`, 2026-10-11. Read-only: no Ghidra writes, no
ledger records. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; the
byte evidence is from the installed executable on disk. Descriptive names are hypotheses.
This builds on `docs/X87_CONTROL_WORD.md` (packet `cc_cruise_command`), which already settled
the CRT startup word and the `CreateDevice` flags. This packet re-verifies those, closes its
three open uncertainties, and corrects its conclusion about which thread runs ship motion.

## Answer

| Thread | Control word during gameplay | Precision | What runs there |
| --- | --- | --- | --- |
| Main (WinMain) thread, after the device is created | `007Fh` | **24-bit** | the frame, the fixed-step driver `00875BB0`, Lua scripts, and its share of the job-pool jobs |
| Main thread before device creation (app init phases 1-5) | `027Fh` | 53-bit | settings load, early config |
| Frame-job pool workers (`processors - 1`, `CreateThread`) | `027Fh` | **53-bit** | their share of the fixed-step element jobs, ship motion included |
| Dyn task workers (`NUMBER_OF_PROCESSORS`, `_beginthreadex`) | `027Fh` | **53-bit** | the Dyn solver fork-join and the collision pass |
| Loading, render-control, surface-save, telemetry/IPC threads | `027Fh`, unless the thread itself creates or resets the device | 53-bit | not gameplay math |

Rounding is round-to-nearest and every exception is masked on every thread. Only CRT
helpers that put the old word back change it, and they do so briefly: `_ftol`, `floor` and
the transcendental functions.

**So no single precision holds for gameplay.** The main thread runs at 24 bits. The Dyn
physics solver runs only on worker threads at 53 bits. Ship motion and the other unit
fixed-step advances are pool jobs that the main thread and the pool workers take from one
LIFO array at the same time. A given unit's step therefore runs at 24 or 53 bits depending
on which thread claimed it, unless the game runs with `use_workers` off (see below).

SSE is used too, and Direct3D's precision setting does not affect it. Arithmetic is still
almost all x87. See the SSE section.

## 1. Device creation and the flags

All `IDirect3D9` objects come from `Direct3DCreate9` (import `00CE23DC`, thunk `00A4C2A8`). It
has two callers:

| Caller | Use |
| --- | --- |
| `00B32410 BSP_D3D9Renderer_Construct` | stores the object at renderer `+1990h` (`00B3247E`) |
| `00991EF0` (options/adapter enumeration) | calls only vtable `+14h`, `+18h`, `+1Ch` and `+2Ch` (identifier, mode count, mode enumeration, multisample check). It never creates a device. |

`Direct3DCreate9Ex` is not imported. Every instruction that reads displacement `1990h` was
decoded (35 sites; `local/fp1_scan3.py`), and each object's vtable slot was followed.
Exactly two calls go through `IDirect3D9` slot `+40h` (`CreateDevice`):

| Call | Function | BehaviorFlags |
| --- | --- | --- |
| `00B2AFF9 call edx` (slot loaded at `00B2AFD6..`) | `00B2AEB0 BSP_D3D9Renderer_InitializeDeviceAndResources` | `ECX = (devcaps & 10000h && VS >= 1.1) ? 40h : 20h`, then `00B2AFE9 OR ECX,4`, `00B2AFEC PUSH ECX` |
| `00B298EE call edx` (`00B298D3 mov edx,[edx+40h]`) | `00B29670 BSP_D3D9Renderer_RecreateDevice` | the same: `00B298BF MOV ECX,40h` / `00B298C6 MOV ECX,20h`, `00B298D7 OR ECX,4`, `00B298E1 PUSH ECX` |

The flags are therefore `44h` (`MULTITHREADED | HARDWARE_VERTEXPROCESSING`) or `24h`
(`MULTITHREADED | SOFTWARE_VERTEXPROCESSING`). The value is built from two constants and a
caps test. No path ORs in a config field or a global, so **`D3DCREATE_FPU_PRESERVE` (`2h`)
can never be set**. This agrees with `docs/X87_CONTROL_WORD.md`.

`IDirect3DDevice9::Reset` is slot `+40h` of the device at renderer `+1A10h`. Decoding all 108
`1A10h` reads finds a single call: `00B2AD58` in `00B2ABD0 BSP_D3D9Renderer_ProcessDeviceReset`.
Its callers are `00B2AEB0` and `00B2B200 BSP_D3D9Renderer_BeginFrame`.

Device creation happens in app-init phase 6, inside the window routine `00BECEE0`, which
calls renderer virtual `+4h` = `00B2AEB0` (`docs/APP_INIT_RENDERER.md`). That runs on the
thread that creates the window and runs the frame, which is the WinMain thread.

### What d3d9.dll does with those flags (runtime probe, this machine)

`tools/d3d9_fpu_probe.cpp` is built with `cl /MT` for x86 and linked `/MANIFEST:EMBED`. It
creates a HAL device with flags `44h`, as the image does, and reads `FNSTCW` on each thread.
It ran on this machine's `d3d9.dll` under Windows 11 26200, console session:

```
main before D3D                     : 027F
existing thread at start            : 027F
main after Direct3DCreate9          : 027F
CreateDevice flags 44 hr 00000000
main after CreateDevice             : 007F
existing thread after CreateDevice  : 027F
new CreateThread after CreateDevice : 027F
new _beginthreadex after CreateDev. : 027F
main 027F then Clear/Begin/End/Present: 027F
main 027F then Reset hr 00000000        : 007F
main 027F then TestCooperativeLevel : 027F
main 037F then CreateDevice+FPU_PRESERVE hr 00000000: 037F
```

So `CreateDevice` without `FPU_PRESERVE` sets `007Fh` (PC = 00, 24-bit) **on the calling
thread only**. `Reset` sets it again. Ordinary device calls leave it alone. Threads that
already exist and threads created afterwards keep the OS default `027Fh`. The last line is
the control: with `FPU_PRESERVE` the word is untouched. This observes the system DLL and the
OS; it is not a run of the game.

## 2. Every writer of the x87 control word

The census covers every Ghidra function start, followed by a byte-level pass over all 513
`D9 /5` byte matches in `.text` (`local/fp1_scan.py`, `fp1_fldcw2.py`, `fp1_ctx.py`,
`fp1_rawfldcw2.py`). There are **330** `FLDCW` instructions. Re-syncing from 40 or more of 63
preceding offsets turned up no further match that decodes consistently as `FLDCW`. There is
one real `FNINIT`, and no `LDMXCSR` outside the CRT.

| Class | Count | Effect |
| --- | --- | --- |
| Inline `_ftol` set: `FNSTCW [s]; MOVZX EAX,[s]; OR EAX,0C00h; MOV [t],EAX; FLDCW [t]; FISTP` | 140 (data flow) | sets RC to chop for one `FISTP`; PC unchanged |
| Restores of a word saved by `FNSTCW`: the `_ftol` restore `FLDCW [s]`, plus CRT-internal restores in `00C0E05E` / `00C0EB36` / `00C0ED3C` | 139 (data flow) | puts the saved word back |
| `_ftol` pairs with pushes between the store and the load, checked by hand in context (`004FA90F 004FA920 004FA981 004FA98D 00510D71 00515868 00745E8E 00803001 00A82684 00A826AB 00AB8F63 00ABA372 00ABA39D 00B2A9FC 00B2AA25 00B52B24 00B52F34 00B841BD 00BB086E 00BC87BF 00BD491E 00BD4930`) | 22 | the same pair |
| CRT transcendental prologs `00BF84F7 00BFA087 00BFA97B 00BFE647 00BFE7A7 00BFEE0B` | 6 | `CMP word [ESP],27Fh / JZ / FLDCW [00D6A678]` (= `027Fh`): if the caller's word is not `027F`, the function body runs at 53 bits; `__math_exit 00C0843B` restores `[ESP]` (`00C08460`) |
| Other CRT-internal: `__trandisp1/2` (`133Fh` built), `00C07FAA`, `__startOneArgErrorHandling`, `00C083A5` (`AND 300h / OR 7Fh`, keeps the caller's PC), `00C0842E`, `00C08479`, `00C0E05E` / `00C0EB36` / `00C0ED3C` (`OR 33Fh` or `OR 300h`), `00C1A21E` (`OR 300h`) | 19 | temporary; each path restores the caller's word |
| `__ctrlfp 00C138A7` (`00C138C5`) | 1 | the set/restore primitive behind `floor`/`ceil`/`modf`/`frexp`/`ldexp` and the libm error paths; each caller restores |
| `_control87 00C2D14E` (`00C2D21A`, `00C2D526`), `___control87_2 00C2CE44` (`00C2CF17`) | 3 | the only **persistent** setters |

Who calls the persistent setters (rel32 and absolute-dword scan, `local/fp1_callers.py`):

- `_control87` is called only from `_controlfp_s 00C273F8`.
- `_controlfp_s` is called from `__setdefaultprecision 00C0682E` (`00C0683C`, `PUSH 30000h /
  PUSH 10000h`: `_PC_53` under `_MCW_PC`) and from `__set_controlfp 00C2C6FF`, which has no callers.
- `__setdefaultprecision` is called from `__fpmath 00BF6A58` (`00BF6A6E`, reached through
  `_FPinit` `00D693D8` from `_cinit`, main thread at startup) and from `_fpreset 00C2CAE5`
  (`FNINIT` at `00C2CAED`). `_fpreset` has **no** rel32 or absolute reference.
- `___control87_2` has no callers.

Thread start: `_threadstartex 00C03B44` calls `_FPmtinit` through `00D693DC` (`00C03BB8`), and
`00D693DC` points to `00BF69E7`, a lone `RET`. A CRT thread therefore starts with the OS
default and nothing in the image changes it.

**Positive control.** `floor`'s x87 fallback at `00C0F394` sets `__ctrlfp(173Fh, FFFFh)`
(word at `00E16460` = `173Fh`: RC down, PC = 11). It calls `__frnd 00C28548` (a bare
`FRNDINT`, no `FLDCW`) and then restores with `__ctrlfp(saved, FFFFh)` at `00C0F433`. Its only
`FLDCW` is `00C138C5` inside `__ctrlfp`, and the census found it. Ghidra has no function
start at `00C0F394`; it sits inside `___libm_error_support`'s range.

**The `00C0ED3C` leak path is unreachable on current CPUs.** `docs/X87_CONTROL_WORD.md` left
this open. `fmod 00BF857A` calls `00C0ED3C` at `00BF8593` only when `[0109DD7C] == 1`
(`00BF8586`). `0109DD7C` is `_adjust_fdiv`, written once at `00BF6A67` from
`__ms_p5_mp_test_fdiv 00C06893`. That function returns
`IsProcessorFeaturePresent(PF_FLOATING_POINT_PRECISION_ERRATA)`, true only on a Pentium with
the FDIV bug. Gameplay calls `fmod` heavily (for example `0060507C` in the angle wrap), but it
always takes the `FPREM` path.

**Conclusion for the image:** no game code writes the PC field. After CRT startup the main
thread holds `027Fh` until `CreateDevice`, and from then on `007Fh`. `BeginFrame` resets and
any recreate set the same value again. Other threads hold `027Fh`.

## 3. Which threads run gameplay math

Thread creation sites in the image (import calls plus the CRT wrapper):
- `004278A0`: the `auto test` harness.
- `00501620 BSP_LoadingQueue_StartWork`.
- `00A4C030` XLive IPC, `00A52470` telemetry and `00A60624` pipe IPC.
- `00B33DA0 BSP_RendererControlWorker_Construct`.
- `00B5E230` and `00B5E380`, surface save.
- `00BE2C70 BSP_FrameJobPool_CreateWorkers`.
- `00C37690 DYN_TaskManager_SetWorkerCount` (through `_beginthreadex 00C03BC4`).

### Fixed-step element jobs: split between precisions

- `00875BB0 BSP_Game_RunFixedSimulationSteps` queues each group's elements into the frame-job
  pool and dispatches with the byte at `00E0B6CE` (`00875D09/D79/DE9/FB0`;
  `docs/FIXED_STEP_JOB_WAVES.md`). Slot `+8h` of a level-5/6 unit is
  `00825F20 BSP_UnitInstance_UpdateShipMotion` (`docs/TICK_ELEMENT_OVERRIDES.md`).
- `00BE3150 BSP_FrameJobPool_Dispatch`: with a non-zero byte it signals the workers' start
  events and calls `ResumeThread` on them (`00BE3167..00BE3194`). On both paths it then
  drains on the calling thread through `00BE2FA0` (`00BE31B9`). Workers (`00BE2BA0` →
  virtual `+0Ch` = `00BE2EA0`) claim, execute and clear jobs from the same array
  (`docs/NATIVE_FRAME_JOB_EXECUTION.md`).
- Worker count is `NUMBER_OF_PROCESSORS - 1` (`00BE4800` reads the environment variable;
  `docs/NATIVE_FRAME_JOB_LIFETIME.md`). The workers are plain `CreateThread` threads, so they
  run at `027Fh`.
- `00E0B6CE` is initialised to **1** in `.data`: the dword at `00E0B6CC` is `00010001`. The
  only literal-address writers belong to the Lua binding `SetSingleThread` (`008A1890`,
  `00E0B6CE = !arg`; `004E27E0`'s body contains the same store at `004E354F`). A byte search
  of all 35,331 non-media files in this installation finds `SetSingleThread` only in the
  executable, so no loose script calls it.

So by default the fixed-step unit advances (ship motion among them) run on the main thread
at 24 bits and on `processors - 1` worker threads at 53 bits, assigned by a LIFO race. A
machine with one processor, or a run that calls `SetSingleThread(true)`, keeps everything on
the main thread at 24 bits. `BSP_Game_Render` dispatches with 0 (caller only), so the camera
and world-view jobs stay on the main thread.

### Dyn physics: 53-bit

`Dyn_World_Substep 00C5BB30` (`00C5BCCA`, `00C5C0E8`, "the solver fork-join" in
`docs/DYN_PHYSICS_SUBSTEP.md`) and `Dyn_Scene_RunCollisionPass 00C57070` (`00C577EE`) submit
tasks through `00C33140`. That function queues them, releases the semaphore and **waits**.
The tasks run only on the Dyn workers (`00C37680` entry, `docs/DYN_TASK_MANAGER.md`).
`00C55EA0` takes the worker count from the descriptor's first dword. `004DE11C` fills that
dword from `00BE4800`, which is `NUMBER_OF_PROCESSORS`. These are `_beginthreadex` threads,
whose `_FPmtinit` is a `RET`, so the solver and the collision pass run at `027Fh` (53-bit).

## 4. SSE

The game code below `00BF0000` was compiled with SSE for `float` moves and compares, but its
arithmetic is x87 (`local/fp1_sse.py`, function-start sweep):

| Game code | Count |
| --- | --- |
| `movss` | 59,571 |
| `xorps` (zeroing) | 4,429 |
| `comiss` / `ucomiss` | 2,904 / 471 |
| `cvtsi2ss` / `cvttss2si` | 616 / 168 |
| `subss` | 916 (for example `0040194B`: `movss xmm0,[const]; subss xmm0,[ecx]`) |
| `addss` / `mulss` / `divss` | 2 / 3 / 1 |
| SSE2 double arithmetic | 0 |
| x87 `fmul`/`fmulp`, `fadd`/`faddp`, `fsub*`, `fdiv*` (whole image) | 18,094, 12,783, 10,288, 3,027 |

The SSE operations always round to IEEE single and use MXCSR. No game code writes MXCSR:
the six `LDMXCSR` are in `___set_fpsr_sse2`, `___statusfp_sse2` and `__clearfp`, reached only
from CRT control-word primitives. MXCSR therefore stays at the default `1F80h`. The SSE2
double code (`mulsd`, `addsd` and others, about 430) is all in the CRT, for example
`BSP_CRT_PowSSE2Core 00C19279`. `cvttss2si` truncates without consulting RC, and the inline
`_ftol` sets RC to chop itself, so float-to-int truncation does not depend on any thread's
word.

The x87 PC field matters only for intermediates that stay on the x87 stack. A single x87 op
on `float` operands that is stored immediately gives the same result at 24 or 53 bits. A
chain such as `a*b+c`, or anything with a `double` constant, rounds each step to 24 bits on
the main thread and keeps 53 bits on a worker.

## Corrections

- `docs/X87_CONTROL_WORD.md`, Answer: "The ship motion at `00825F20` therefore runs at a 24-bit
  mantissa". That holds only for steps the main thread drains. With the default
  `00E0B6CE = 1` and more than one processor, some steps run on frame-job workers at `027Fh`.
  Its uncertainty 3 (other threads) is answered above, and uncertainty 2 (`00C0ED3C`) is
  answered in section 2. Its uncertainty 1 is now confirmed by the probe, for this machine's
  `d3d9.dll`.
- `docs/DYN_CONTACT_SOLVER.md:145` ("x87 at the game's 24-bit") and `docs/DYN_LCP_IMPULSE_MATH.md:222`
  ("control word at 24-bit precision"): the solver tasks run on Dyn workers at 53 bits. These
  documents belong to other packets and were not edited.
- `docs/CC13_LUA_SCRIPT_LOADER.md` ("after D3D init, script arithmetic runs at 24-bit x87
  precision"): correct for scripts run on the main thread, which is where the frame runs them.
  `lua_Number` arithmetic in `double` is rounded to a 24-bit mantissa per x87 operation there.
- `docs/GAME_EXECUTABLE.md` §4 measured `0x00020000` (24-bit) at the reconstruction's
  fixed step. That is the host's single-threaded step on its own main thread, not the image's
  pool.

## What this means for the reconstruction (for cc9)

- The original has no single reference precision for unit fixed-step math. Each step is 24
  or 53 bits depending on thread timing, so even the original is not bit-reproducible across
  runs on multi-core machines. A host that runs the waves on one thread at 24 bits matches
  the original's single-thread mode (`SetSingleThread(true)`, or a one-processor machine)
  exactly in precision, and matches its default mode only statistically.
- To match the original's Dyn solver, evaluate it at 53 bits, not 24. SSE2 `double` gets
  close. SSE2 `float` under `/fp:strict` rounds each step to single, which is the 24-bit
  behaviour, not the 53-bit one.
- Main-thread presentation, camera and Lua: 24 bits.

### Per-function thread checks (cc13-fix13, 2026-10-17)

| Function | Reached from | Thread / precision | Reconstruction |
| --- | --- | --- | --- |
| `00C37F60` body add force at point | `0092B350`, fixed-step slot `+8h` (wave 3) | main 24-bit or pool worker 53-bit | float per op (24-bit), `src/cc13_dyn1.cpp` |
| `0085E4D0` rotate by angular velocity | HUD `00526A40`, `00640A20`; fixed-step `007C18B0`, `007D9F60`, `00901C20` | HUD: main 24-bit; fixed-step: main 24 or worker 53 | float per op (24-bit), `src/cc13_front4.cpp` |
| `00611770` powerup screen slide | update slot `+24h` (`0061BEA0`) via `004D8620` GUI frame update | main 24-bit | float per op, `src/cc13_sweepB3.cpp` |
| `007DC830` free-flight step | `007CC2F0` / `007CBFA0` from `007CE040`, plane element slot `+8h` (wave 3) | main 24-bit or pool worker 53-bit | unchanged (frozen for cc9); still models 53 bits |

## Uncertainties

1. The probe observed this machine's `d3d9.dll` and OS, not the game process. It shows that
   `CreateDevice` and `Reset` set `007F`, that ordinary calls do not, and that threads start
   at `027F`. Older d3d9 builds are documented to behave the same, but that was not observed.
2. Which thread calls `BeginFrame` (and so `Reset`) was not traced in full. If the render
   control worker calls it during loading, that worker drops to 24 bits, which does not
   affect gameplay. The main thread is at `007F` either way, from the first `CreateDevice`.
3. The writers of `00E0B6CE` were found by literal-address scan. A block-copy or a
   base-plus-offset write through `00E0B6C8` would not show up. No such writer was seen
   among the `00E0B6C8` references, but they were not all decoded. Scripts packed in archives,
   if the installation has any compressed ones, were not searched.
4. The function-start sweep decoded about 36% of `.text` bytes, because it stops at in-line
   data. The `FLDCW` count is backed by the byte-level pass. The SSE and x87 counts are lower
   bounds; their proportions are what this document relies on.
5. No game run read the control word on a worker thread. A `FNSTCW` logged from a job body
   (for example the entry of `00825F20`) together with `GetCurrentThreadId` would confirm the
   split directly.

## Reproduction

Scripts are in `local/` of this worktree (not committed): `fp1_scan.py` (`FLDCW`, `FNINIT`,
`LDMXCSR`, thread imports, rel32 callers), `fp1_scan3.py` (`+1990h` object slots),
`fp1_fldcw2.py`/`fp1_ctx.py` (writer classification), `fp1_rawfldcw2.py` (byte-level
completeness), `fp1_callers.py` (rel32 and absolute references), `fp1_sse.py` (SSE census) and
`fp1_grepinstall.py`. The probe source is committed as `tools/d3d9_fpu_probe.cpp`.


## Live original-entry observation, 2026-10-30

The installed-original USN04 sampler check recorded main/entry TID85524 and
15 genuine00825F20 motion entries on six other threads, all CW027Fh. Counts
were3/3/3/3/1/2 on TIDs10228/117204/128976/100276/62112/114196. Six distinct unit
addresses were observed. No main-thread hit was captured; this partial sample
neither establishes exclusive-worker execution nor measures the main thread's
CW. It confirms that a PC24-only assumption does not describe these entries.
No PC24 classification or reconstruction was changed. cc13/cc9 notifications
are1068/1069. Details and thread histogram: `ORIGINAL_MOTION_ABI.md`.

Collection requested20 hits but a guard receipt IO failure stopped it after15.
Raw recovered receipt: ignored
`local/original-baseline-clock-20261030/launch-orphan-recovered.json`.
Original files/hashes were unchanged. These are execution-context observations
for this installed build, not numerical parity proof.

The histogram was collected after the clock CSV, and none of its six unit
addresses matches the fixed CSV unit. It does not assign a precision word
to those trajectory rows.
