# Actual render-command execution: next implementation boundary

Read-only discovery, 2026-09-10. `00B1D950` is **not ready for a full native-execution reconstruction claim**. The actual 44h command lifetime, actual 18h batch lifetime/reference adapter, context and group ownership are available on main. They do not close batch preparation, material dispatch, native queue, scheduler, renderer or diagnostic-service execution. The next bounded implementation should be the actual **8-byte preparation-job singleton lifetime**, described below.

This packet changes only this document and `reports/native_render_command_execution_next.json`. Its worktree starts at `588fc1c`; the source inventory was refreshed against main `c9f8b9d5d33d41c3c46a4a219418fcc94f55df10`, including the command-owner integration from `1e8b004`. Names are reconstruction hypotheses, not recovered symbols. The report records 36 live Ghidra/installed-PE comparisons covering 5,653 bytes, including complete command code, relevant leaf owners and virtual profiles. Installed executable SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Actual command and ordered execution

Original ABI: `ECX=actual44h command`, no stack parameters, plain `RET`. Full code is `00B1D950..00B1DAC0` inclusive. The three bytes skipped by Ghidra at `B1DA6D..B1DA6F` are unreachable alignment after a jump, not omitted execution. No native Boolean/error return or adapter-failure early return exists.

| Command offset | Actual storage/use |
|---|---|
| `+00` | Native table `D5E5E0`; its execution entry is `B1D950` |
| `+04` | Retained scene pointer, reloaded after camera execution |
| `+08` | Preserved word; this execution does not read it |
| `+0C/+10` | Two actual batch pointers; each batch is 18h with mode `+08`, pointer list `+0C`, signed count `+10`, capacity `+14` |
| `+14/+18` | Native length/data diagnostic string |
| `+1C..+27` | Three metadata DWORDs passed by address to renderer virtual `+114` |
| `+28` | Actual 18h context; camera `+08`, target group `+14` |
| `+2C/+38` | Group containers owned by lifetime code; execution does not enumerate them |

1. Reload global renderer `F8D394`, then call `B20240`: readiness requires renderer DWORD `+1D90 == 0` and lost byte `+1D8A == 0`. Failure returns before diagnostics cleanup is armed.
2. Read command context once. Capture renderer and its current virtual `+98`, capture camera from that context `+08`, and read target from the same context `+14`. Call target binding. Keep the captured camera for the rest of this command; native adds no temporary reference.
3. Call `B71360` on that camera. Reload command `+04` and call `B46A70` with scene in `ECX`, captured camera in `EDX`.
4. Reload optional diagnostic singleton `F8D39C`; when nonnull, call `B13030` with the command string view at `+14`.
5. Reload renderer and capture its virtual `+114`; prepare command metadata pointer. **Only now** arm unwind state 0 at `B1D9D6`, immediately before the metadata virtual call. The concrete `D5F0A8` profile uses `B20210`, whose entire body is `RET 4`; this particular native no-op is established, not a general permission to stub virtuals.
6. Reload renderer and call current virtual `+2C` (`B1FE20` in the concrete profile), testing actual active-frame DWORD `+1998`. If inactive, skip all batches but still reset diagnostics.
7. Select jobs only if byte `108D6DC == 0` and **both signed** actual batch counts, read through `B51B20`, exceed 50. The second count is not read when the first fails. Otherwise prepare serially.
8. Job path, for each index: read current batch and write mode `+08` **before** `4C1130`; reload that command batch cell **after** the pool getter; capture pool subobject `+04`; call preparation-job getter `B0FFB0`; then load the captured pool subobject's current table and enqueue virtual `+04` with `(actual job primary pointer, captured batch)`. Repeat independently. Obtain the pool again, then call its subobject virtual `+08` with mode 1. Do not retain one pool, one job, or a pre-captured batch array across these callbacks.
9. Serial path: load each current command batch and its current virtual `+0C`, then call with index. It does **not** perform the job path's mode write first.
10. Execute both batches in index order through each current batch's virtual `+08`, passing `(index, captured camera)`. Reload each batch cell/table when reached. The concrete `B55550` body ignores both stack arguments; alternative profile behavior must not be inferred from that.
11. Disarm state at `B1DAA2`, then call `B13510`, which reloads the current diagnostic singleton and assigns literal `X`. Handler `CBCA68`, EH info `DF4EF0`, and one-entry unwind map `DF4EE8` route armed exceptions through `CBCA60 -> B13510`. Exceptions in readiness, target, camera, system, or label-copy phases do not trigger this command guard. Reset exceptions occur after disarming, so there is no recursive second reset.

The host `execute_render_command_00b1d950_fragment` remains explicitly partial: `RenderCommand` is not actual44h storage, sort configuration is a host vector, job submissions use host callback arguments, diagnostics use `std::string`, and owner/adapter errors can stop execution. Captured arrays or empty callbacks cannot convert it into the native function.

## Dependency closure and remaining owners

| Boundary | Present implementation/evidence | Work still required for actual execution |
|---|---|---|
| Command lifetime/context/groups | `native_render_command_owner`, `native_render_group_lifetime`, actual-owner and reference adapters; command raw 44h and context raw 18h | Resolve these same raw identities during execution. Host associations are not execution implementations. |
| Native batches | `native_render_batch_lifetime` and `native_render_batch_reference` implement raw 18h storage/pool/refcount/recycle | Bind preparation and execution to live raw `+0C/+10/+14`. `InstanceRenderQueue` and `InstanceRenderEntry` remain semantic storage, not these raw overlays. |
| `B51DF0` batch preparation | Full 284-byte native body inspected; sort-key math and sort algorithms exist | Actual queue getter/config owner, raw entry/material/effect reads and raw list sorting remain open. Passing a captured vector of configurations does not close them. |
| `B55550` batch execution | Full 70-byte native body inspected; host batch fragment exists | It reloads renderer active gate, clears actual `108FBF4`, and reloads signed count/list after every entry callback. Resolve `entry+04 -> section+20 -> material+7C -> effect.virtual+14` on actual owners. |
| Effect `D61A00+14 -> B45360 -> B44750 -> B43410` | `MaterialEntryDispatcher` supplies typed orchestration; `B42350` material-constant builder is now complete at its typed interface | Actual effect/program/override arrays, camera-mode selectors, model/renderer update, geometry, pending-plane restoration/construction, effect texture lookup, native material callback ABI, and `B16F80` statistics ownership remain explicit requirements. |
| Material constant inputs | `build_material_constants_00b42350` is implemented | Concrete `MaterialConstantOwners`: live clone, mode, transform, bones, animator predicate/palette, decode fields, LOD, generated-model lifetime, shadow texture/light and dynamic-source virtual `+2C/+30`. A generic successful callback does not supply these owners. |
| Actual queue `4C11F0/F8D440` | `render_command_queue.cpp` reconstructs host fragments; earlier queue discovery identifies actual34h constructor `B1F280` | Native fixed configuration cells `+04/+0C`, worker control `+20`, 20-byte queue rows at `+24`, current context `+30`, pooled row strings, allocation/lifetime and live publication. Constructor control preimage 2 reaches renderer `+11C -> B28A90` worker-stop behavior. |
| `B1CB30` queue config leaf | Complete 31-byte body inspected | Reads byte `queue+04+index*8`, writes it first, then reads DWORD `queue+08+index*8` and writes it; `AL=1` retains upper EAX bytes from the DWORD; `RET 0Ch`; no bounds check. These are not the queue's 20-byte row records. |
| Camera `B71360` | Actual 45Ch camera owner and live frame projection exist; `D3D9CameraFrameAccess` provides typed camera operations | The helper captures a cache companion; native reloads `F8D394` separately for preparation `+A0`, viewport `+A4`, and Clear `+08`. Bind each stage to the current actual renderer's fields/counters/planes. Camera `+17C` gates execution; viewport `+180` is loaded after preparation; x87 depth `+18C` is read/stored even with flags zero before Clear. |
| System `B46A70` | `build_and_upload_system_constants_00b46a70` now composes the full typed body | Supply the same captured camera, actual scene outer `+1C`, clocks/foliage/service slots, initialized 77-register preimage, shader blocks and independently reloaded renderer per VS/PS call. `ConcreteSystemSceneResource` explicitly has a new C++ ABI, not native scene-resource `+3C`; renderer-service time projections are not its full owner. |
| Renderer `F8D394` | `D3D9StateCache` wraps concrete COM calls; actual parameter-region owner covers only `+1A14..+1A27` | Full renderer identity/domain, readiness/active fields, live table, device/default surfaces, target ownership/cache, viewport/planes/counters and synchronization are not a complete raw renderer owner. The parameter-region implementation does not close them. |
| Target `B24E70` | Host `shared_ptr<D3D9FrameTargets>` and concrete surface wrappers exist | Actual target group retains references/count at `+04`, four color cells `+08..+14`, depth `+18`, sRGB byte `+3C`, and renderer target slot ownership. Connect actual surface owners, application `F8D398` sRGB policy, default color, and live replacement ordering. |
| Diagnostics `F8D39C`, `B13030/B13510` | Host string fragments; actual pooled-string allocator/operations are available | Same actual service owner string at `+684`; this singleton also supplies matrix `+1D8` and region `+84` to system time. Preserve live destination/source reloads, literal `CE9A38 = X`, allocation failures and native unwind timing. No full diagnostic-service owner is supplied by the time projection. |
| Preparation-job singleton `B0FFB0/F8D444` | Complete getter, tables, deleting/body destructors and EH evidence below | Ready as a separate actual 8-byte lifetime packet; does not provide preparation or scheduler execution. |
| Job execution `B1BF70` | Complete 18-byte tailcall inspected | Reads stack batch, loads mode `batch+08` into the same stack argument, then tail-jumps batch's current virtual `+0C`. It cannot be closed by calling a host configuration-vector preparation fragment. |
| Frame scheduler `4C1130/109CF08` | Native getter/constructor and concrete tables inspected; `game_render_frame` offers only ordering helpers | Actual 138A8h owner, base at `+04`, construction/destruction, work entries, atomics, event owners, thread procedure/start/stop, registration and wait visibility remain. Enqueue/wait callbacks are not that implementation. |

Within `B51DF0`, configuration byte selects sorting using the original index. Index 0 builds raw keys from each entry's section/material chain: signed material `+34 > 0` gates texture `+10`, then material `+20`, effect `+B0` low six bits, effect byte `+C0`, and x87 depth from entry `+14` converted through `BF7456`; keys occupy entry `+20/+24`. Sort uses `B1DCE0` and comparators `B51B00/B51AB0`. Existing key arithmetic and introsort are reusable; actual owner resolution and mutable list storage still need implementation.

Concrete renderer profile `D5F0A8` was read from live Ghidra and disk: `+08 -> B21430` Clear; `+2C -> B1FE20` active frame; `+98 -> B24E70` targets; `+A0 -> B285A0` prepare camera; `+A4 -> B26770` viewport; `+114 -> B20210` metadata no-op; `+11C -> B28A90` worker stop. Actual pooled batch profile `D5E5AC` is `+00 -> B55680`, `+04 -> B1C630`, `+08 -> B55550`, `+0C -> B51DF0`. Its construction base `D62064` differs: `+00 -> BD30E0`, `+04 -> B51DD0`, `+08 -> BF698E` purecall, `+0C -> B51DF0`. These mappings describe the observed profiles, not arbitrary replacement tables.

The scheduler's `CE7550` primary table starts with `4BFB30`. The `+04` base-subobject table at `CE7554` has `+00 -> 4BFAC0`, `+04 -> BE3020`, `+08 -> BE3150`, `+0C -> BE2EA0`, followed by `4BFAB0/4BFAA0`. `4BFA40` constructs that base through `BE3040(-1)`. Native `BE3020` stores `(job,arg)` at base `+24+count*8` and increments base `+20`; there is no capacity check. `BE2FA0` uses interlocked decrement to drain from the end, dispatches primary job virtual `+00`, clears the pair, repairs the final count, then waits on per-worker event virtuals. `BE3150(1)` increments global `109DBE4` through `BF2CE0`, sets base `+18`, resets/signals events through `BE3110`, resumes workers, drains work on the caller, clears `+18`, and decrements the global through `BF2CF0`. Mode 0 takes another event signaling path. `BE2C70` creates native event objects (`BD1970`) and suspended Windows threads; `BE2EA0` and thread entry `BE2BA0` need their real event, thread-ID and random-thread registration behavior. Existing `RandomThreads` is reusable, not a completed scheduler.

`BE4800` also has a false-noreturn listing gap after `_free`: disk/live bytes `BE483C..BE4845` restore the saved `_atol(NUMBER_OF_PROCESSORS)` result and return. Failure of `__dupenv_s` returns 1. Do not infer unconditional return 1 or use Ghidra's `extraout_EAX` as the contract. This is a discovery observation only; no Ghidra repair was made here.

System inputs that must share actual publication domains include timer `1090AB0`, particle clock `F8D420`, foliage manager `F8C274`, foliage owner `F8C210`, service `F8D39C`, parameter words `108FC30`, register count `E13078`, and exponent `CFAD80`. Shader/material stages additionally require their existing live shader blocks and owner interfaces. This inventory stops at each named or explicit unresolved native interface rather than treating it as implemented; lower material/device factories remain follow-up work, not implicitly closed.

## Ready next packet: preparation-job singleton lifetime

Suggested packet: `native_render_preparation_job_owner`. Claim these five entries and dedicated `native_render_preparation_job_owner.hpp/.cpp`, documentation/report files; coordinate metadata shards through the integrator:

| Entry | Complete original ABI and behavior |
|---|---|
| `B0FFB0..B1007D` | No inputs, EAX current singleton, plain RET; lazy actual 8-byte getter |
| `B0F1D0..B0F1E9` | ECX primary owner, plain RET; clear `F8D444`, write `CE3818` at owner `+04`; null path writes address zero rather than silently succeeding |
| `B0F210..B0F243` | ECX primary owner, stack flags, `RET4`, EAX original primary pointer; clear singleton, write base secondary table, free original primary address iff `flags&1` |
| `B0F1C0..B0F1C7` | Secondary deleting thunk: subtract 4 from ECX, tail-jump `B0F210`; flags/RET inherited |
| `B0D930..B0D958` | Construction-base scalar destructor: ECX base pointer, flags/RET4; clear singleton, write `CE3818` at that pointer, optional free of that same pointer, return it |

Raw owner layout is exactly two DWORDs. `B0FFB0` stores secondary table `D5E154` at `+04`, then primary table `D5E160` at `+00`, then final secondary table `D5E15C` at `+04`. `D5E154+00 -> B0D930`; `D5E15C+00 -> B0F1C0`; `D5E160+00 -> B1BF70`; post-destruction `CE3818+00 -> 412440`. The adjacent `D5E158 -> B0F1F0` belongs to a different service destructor, whose unregister behavior must not be imported into this job.

Getter algorithm and callback boundaries:

1. Read `F8D444`; a nonnull fast path returns that captured value.
2. Call lifetime getter `415350` and capture its current section `+10`. Enter that captured section if present, then increment its recursion `+18`; only after entry/increment arm the guard.
3. Reload `F8D444`. If still null, request exactly eight native bytes from `BF681B`. Write the three table transitions above, or publish null if allocation returned null. There is no invented successful object or extra rollback.
4. Reload the published singleton and compute its secondary pointer `+04` (or null) **before** the second lifetime-manager getter. Register this captured secondary pointer through `BD0C30` on that second current manager. Registration is attempted even for null; the real manager decides how null/validation behave.
5. Decrement captured section recursion before calling LeaveCriticalSection. Reload `F8D444` **after unlock** and return it. Callback replacement of the global or lifetime manager must preserve these precise captured/live distinctions.
6. Getter handler `CBBC48`, info `DF3EB8`, map `DF3EB0`, action `CBBC40` call guard destructor `411EE0`: reset guard table `CE37FC`, decrement captured section `+18`, and leave. The getter does not unpublish/free a job when registration throws. If initial enter throws, state is not yet armed and no leave is invented.

All three destructor bodies clear `F8D444` unconditionally: no identity check, no unregister call, no primary-table reset. Final registered destruction goes through the actual secondary pointer and its adjustment thunk; the allocator must receive the original primary allocation. `B0D930` describes the distinct construction-base profile and must not be substituted for final secondary deletion. A valid-owner precondition is needed for the native null-write paths; a new host guard must not be presented as recovered behavior.

The existing `SingletonLifetimeDomain`, real pointer-container manager, OS critical-section operations and allocation boundary are sufficient dependencies for this owner. Use that same manager domain. The manager's registered-destructor dispatch must recognize the final secondary profile and run the concrete adjustment/deletion route. Preserve an eight-byte raw object plus external host association if needed; do not put a host vtable, refcount or scheduler state in those bytes.

Keep `B1BF70`, raw `B51DF0`, queue creation, `BE3020/BE3150`, and full `B1D950` explicitly outside this packet's completion claim. Storing the observed primary table describes its identity; it does not implement its execute target. Reuse existing allocation/manager validation and build checks. Only add a focused behavioral check if needed for the concrete risk of secondary-pointer deletion or publish/register/unlock replacement ordering; do not expand a scheduler test suite while implementing this owner.

## Verification boundary

Every live analysis batch used `bsp.py ghidra`, whose client verifies project `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default` and image base before querying. The configured project file is `C:/Users/sqz269/bsp.gpr`; no import, rename, comment, function creation, flow repair, save or other Ghidra mutation occurred. Hashes and exact inclusive/exclusive spans are in the companion JSON. Byte agreement establishes installed-code evidence, not game execution.

No C++ implementation, reconstruction/name/tag metadata or packet ledger changed. No build or new test was needed for these two evidence files. The recommendation is an implementation-ready owner contract; neither command execution, ABI-compatible replacement, nor game/render validation is claimed.
