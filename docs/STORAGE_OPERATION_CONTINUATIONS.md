# Storage operation continuations and prompts

`006adb50` drives storage progress and prompts, and can invoke its argument
continuation before returning. Its native argument is a function pointer in
ECX. Describing this call as callback registration or asynchronous scheduling
loses its visible ordering. The reconstructed body is
`run_storage_operation_006adb50` in `src/storage_operation.cpp`.

## Native control flow

The driver captures the manager at `0109CECC` in EDI on entry. If callback slot
`00E198F8` is empty, it calls `0057bda0`, manager wrapper `00bd3580`, and
`0057bdc0`, in that order. The wrappers conditionally set and clear byte
`00E194B8`; they do not implement a nested counter or restore an earlier value.
`00bd3580` tail-jumps to manager virtual `+44h`, the actual progress boundary.
The byte's broader engine meaning is not established here.

The incoming continuation is stored in `00E198F8` at `006adb99` **once**, after
that optional first update. The backward branch targets `006adba0`, after the
store. A continuation installed by a host call during a subsequent iteration
must therefore survive the next iteration. The decompiler's apparent store in
its `do` loop obscures this distinction.

Each iteration tests manager byte `+0Ch != 0 && +10h == 0` (`006ad150`). When
true, it first sets `+10h=1`, then copies message `+14h` and flag bytes
`+0Dh/+0Eh/+0Fh` through `00bd41a0`. The string copy precedes the three sequential
byte reads and stores; the implementation retains self-copy and alias behavior.

The flag mapping uses zero/nonzero tests:

| +0Dh | +0Eh | +0Fh | Prompt kind |
| --- | --- | --- | --- |
| zero | zero | zero | `Busy` (3) |
| zero | zero | nonzero | `Accept` (2) |
| nonzero | nonzero | zero | `YesNo` (1) |
| other combination | | | Retain previous EBP kind |

`00bd4140` writes the message/flags, marks `+0Ch=1`, clears `+10h`, and resets
response `+1Ch=0`. Its inspected PC caller `00beb9b0` emits exactly the first
three combinations: saving, failure acknowledgement, and delete-slot question.
These two producer functions are evidence dependencies, not reconstructions in
this packet. The first prompt's retained EBP comes from an uninitialized stack
word at `006adb95`. The C++ projection rejects a first unsupported flag tuple
with `std::logic_error`; it does not claim a recovered fallback. Unsupported
tuples after a valid prompt retain that earlier kind as the native does.

The existing `raise_prompt_00531b00` body receives slot 0, the copied message,
the selected kind, callback address `006ad3c0`, unused flag 0, empty title,
timeout `0.0f`, timeout result 0, an empty owned countdown key, and
`dismissible=true`. Interactive kinds return immediately, leaving the
continuation in the shared slot and `+10h` set. A Busy prompt executes all of
`004c6c30` (TryBeginRenderFrame), `004ca440` (Game_Render), and `004ca1f0`
(FinishRenderFrame), irrespective of the first call's return value. It then
calls `00bd3550(manager,3)` before releasing the temporary message.

`00bd3550` is a response setter, **not a progress pump**: it stores `+1Ch`,
clears `+0Ch`, then clears `+10h`. It leaves `+08h` and the message unchanged.

After the optional prompt, a state other than 0, 1 or 2 causes another guarded
virtual `+44h` update. The state is tested again afterward. States 0, 1 and 2
all end this driver invocation; their broader success/failure meaning is not
inferred from this predicate. Other states continue the loop. A required host
implementation must perform real storage progress: no default no-op update,
invented completion, artificial iteration limit, delay, or fake queue is added.

At completion, the driver reads `00E198F8`, clears the global, invokes the saved
nonempty continuation, and only then dismisses prompt slot 0. If the slot was
empty, there is no dismissal. A continuation can synchronously reenter this
driver and install or complete another prompt; the outer invocation still
performs its post-callback dismissal. The implementation preserves that order.

## Prompt response and binding

`006ad3c0` takes the prompt result in ECX. Result 1 maps to storage response 1;
result 0 maps to 2; every other result maps to 3. It reloads the current manager,
calls `00bd3550`, then reads the current `00E198F8` and tail-jumps to `006adb50`.
It does not clear that slot itself. The native body is the 42-byte range
`006ad3c0..006ad3e9`; the next byte starts INT3 padding.

`StorageOperationState` projects the manager pointer and both shared globals.
All nested calls must share this object. `StorageManagerOperation` projects
only fields consumed here, including its existing owned NativeString; the host
owns the manager and releases its string. `StorageContinuation` is a
`std::function<void()>` host binding, not a native function-pointer ABI.

`StorageOperationHost` supplies the required manager virtual `+44h`, the live
front-end prompt screen and host, and the three rendering calls. The driver
uses the already reconstructed prompt raising/dismissal functions directly.
`FrontEndPromptHost::invoke_callback` must route address `0x006ad3c0` to
`respond_storage_prompt_006ad3c0` with the same storage state and host. Profile
bindings can capture their concrete profile completion functions in a
`StorageContinuation`; callers must allow synchronous delivery.

## Address and original ABI ledger

All descriptive names below are hypotheses, not recovered symbols. The
worker queried only existing `bsp` project `/battlestationspacific.exe` through
the verifying CLI. The integrator separately created the missing `006ad3c0`
function under the Ghidra write lock and saved the project; this worker then
refreshed its export. Previous names were `FUN_<address>` (the new response
function had previously been an unowned label).

| Address and inclusive end | Reconstructed name | Original ABI |
| --- | --- | --- |
| `006adb50..006addbe` | `BSP_StorageOperation_RunContinuation` | ECX=void(*)() continuation; RET |
| `006ad120..006ad134` | `BSP_StorageOperation_IsStateZeroOrOne` | ECX=manager; EAX=0/1; RET |
| `006ad150..006ad164` | `BSP_StorageOperation_HasUnobservedPrompt` | ECX=manager; EAX=0/1; RET |
| `006ad3c0..006ad3e9` | `BSP_StorageOperation_RespondPrompt` | ECX=int result; tail JMP006adb50 |
| `0057bda0..0057bdb0` | `BSP_StorageOperation_BeginUpdateGuard` | No arguments; RET |
| `0057bdc0..0057bdd0` | `BSP_StorageOperation_EndUpdateGuard` | No arguments; RET |
| `00bd3580..00bd3586` | `BSP_StorageOperation_Update` | ECX=manager; tail JMP virtual+44h |
| `00bd3550..00bd3561` | `BSP_StorageOperation_SetResponse` | ECX=manager; stack int response; RET4 |
| `00bd41a0..00bd41f4` | `BSP_StorageOperation_CopyPrompt` | ECX=manager; stack message*,flagD*,flagE*,flagF*; RET10h |

Assembly resolves the ECX inputs, one-time callback store, flag-pointer order,
response mapping, ignored render return, and clear/call/dismiss sequence. The
only x87 operation in this driver stores zero for the timeout. Flow reports
found no fallthrough gaps in these nine bodies. Export and disk hashes are in
`reports/storage_operation_continuations.json`.

## Validation boundary

MSVC Win32 Release compilation and the existing two CTest checks passed after
seed verification matched all eight native seed byte ranges. One ignored
fixture calls the real reconstructed prompt lifecycle and verifies interactive
return, response 1, replacement of the shared callback during repeated manager
updates, clear-before-callback, synchronous nested continuation, Busy rendering
order, response 3, and final prompt cleanup. Its source and logs remain under
`local/storage_operation_fixture*`; the build log is
`local/storage_operation_build.log`.

This is exported, reconstructed, build-tested and fixture-tested host behavior.
It is not ABI-compatible, a drop-in replacement, native differential proof for
this driver, storage-device validation, or game/runtime visual validation.
