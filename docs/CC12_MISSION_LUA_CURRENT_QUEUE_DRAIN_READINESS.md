# Actual mission Lua queue drain: 00888230

The complete body at `00888230..008882A8` is 121 bytes and **47** contiguous
instructions. Its live bytes equal the installed executable. Signature metadata
reports 45 instructions; the report retains that discrepancy and uses the complete
listing and byte decode for the instruction schedule. The initial size gate was
121 bytes against the authorized 300-byte limit. No listing repair was needed.

This read-only audit establishes the drain's actual storage and call contract.
Source implementation remains on hold for two direct services: actual named-call
dispatch at `00887E50` and deferred-payload destruction at `008876B0`. The raw
mission Lua owner constructor already exists, as do concrete host CRT free and
invalid-parameter services. No replacement owner, callback, or empty-queue default
is proposed. Existing projected function credit remains unchanged.

The requested published baseline is
`287e0bb7af049a33ad6e75f0e9e67113066a6770`; worker baseline
`9e5df998239ef365e4a52d5e69274b393ecb462a` merges it while preserving the preceding
World expiry Source commit `279233b2f7660c61510a0862f7d127bd5f5d4942`, already
under primary review. This packet changes only this document and its report.

## Exact receiver, queue, and dispatch schedule

ECX supplies the actual mission Lua host. `PUSH EDI; MOV EDI,ECX` retains that same
host for the entire drain. The first full DWORD comparison at host `+10h` returns
when zero, without reading the queue head or touching ESI. On the nonzero path,
ESI is saved and the exact six-byte `LEA EBX,[EBX+00000000]` at `0088823A`
(`8D 9B 00 00 00 00`) executes once. It preserves EBX and flags. Subsequent loop
iterations branch to `00888240`, after this no-op.

At `00888240/43`, the body reads `host+0Ch` into EAX and that sentinel's first
pointer into ESI. If the captured front equals that captured sentinel, it calls
`00BF6713` at `00888249`. The ordinary CALL has an encoded continuation: if the
handler returns, the body continues using the same captured ESI without reloading
the queue or front before argument delivery. No new abort/throw policy is inferred.

The named call occurs **before any local unlink, destruction, free, or count
decrement**. From the selected node N, the body loads `[N+28h]`, then `[N+24h]`,
forms `N+8h`, and pushes the five arguments in this order:

| Push site | Pushed DWORD | Position at callee entry |
| --- | --- | --- |
| 00888257 | previously loaded `[N+28h]` | argument 5 |
| 00888258 | previously loaded `[N+24h]` | argument 4 |
| 0088825C | address `N+18h` | argument 3 |
| 00888260 | address `N+10h` | argument 2 |
| 00888261 | address `N+8h` | argument 1 |

`00888262` restores ECX to the captured host; `00888264` calls `00887E50`.
EAX still holds `N+8h` and EDX holds `N+10h` at this boundary. Existing Source
labels these arguments self key, name, argument-vector storage, first stack index,
and last stack index. This packet proves their address/value delivery, not the
callee's interpretation. There is no caller cleanup for the 20 argument bytes,
so a compatible named-call target must consume them. No typed result is used.

## Callback return, current front, unlink, and retirement

After the named call fully returns, `00888269/6C` reloads the same host's current
`+0Ch` sentinel and its current first pointer. The body replaces ESI with this
then-current node R. R need not equal dispatched N: callbacks can alter the queue,
and the drain itself does not read N again after dispatch returns.

If R equals the newly captured sentinel, the second ordinary `00BF6713` call runs
at `00888272`. Whether or not that call ran, `00888277` then compares the retained
R against **another current read of host+0Ch**. A returning handler can change this
final comparison, but the body does not reload R from a repaired front. Equality
skips local unlink/destruction/free/decrement and reaches the current count test.

For inequality, the unlink at `0088827C..00888288` is exactly:

1. Load R's `+4h` link, load R's `+0h` link, and store the latter through the former.
2. Reload R's `+4h` link, reload R's `+0h` link, and store the former at the latter's
   `+4h` address.

The second pair is freshly loaded after the first store; caching both links before
either store would lose the native alias behavior. Both writes precede payload
destruction. `0088828B` passes **R+8h** in ECX to `008876B0`, with no pushed
arguments. After that complete return, the body pushes **R**, calls `_free` at
`00BF65AC`, and performs its explicit four-byte caller cleanup. It reads no node
fields after the payload destructor returns and no released node fields after free.

Only after `_free` returns does `0088829C` decrement the host's **current** full
DWORD count using wrapping `ADD [EDI+10h],-1`. This is not a stored snapshot from
before the callback. `008882A0` rereads the current count; any nonzero bit pattern
repeats, including negative signed representations. The two saved registers are
restored by `POP ESI; POP EDI; RET` on the entered-loop exit path.

The body has five direct call sites to four distinct services and five internal
branches. It has no indirect virtual dispatch, lock operation, global-cell access,
x87 operation, local allocation, local EH frame, or rollback. The report contains
all 47 decoded instructions, branch targets, call encodings, and complete bytes.

## Current Source services and concrete storage

`construct_native_mission_lua_owner_008882d0` in
`src/native_game_storage_defaults.cpp:27` already initializes a raw 14h-byte owner:
vptr at `+0`, a 2Ch-byte self-linked sentinel published at `+0Ch`, count `+10h = 0`,
and Lua pointer `+4 = 0`, preserving the allocator word at `+8`. Its allocator route
uses `NativeGameArrayCalls::allocate_00bf681b`, whose concrete default uses
`singleton_lifetime_allocate`. The self-link helper writes sentinel `+0` and `+4`.
The existing raw Game constructor allocates 14h, calls this provider, and publishes
the result at Game `+1A08h`. These are current Source facts; this packet does not
reanalyze the constructors or establish full runtime ownership or table compatibility.

`singleton_lifetime_free` calls actual `std::free`. Existing validation adapters,
including `NativeInputDeviceRuntime::invalid_parameter_00bf6713` and the pending
entity drain's local adapter, call actual `_invalid_parameter_noinfo`; their Source
allows a configured handler to return. These are concrete library boundaries,
not invented no-op services. Matching allocation provenance, real handler policy,
native register/stack compatibility, and fault behavior remain qualification points.
Other lifetime wrappers named `free_00bf65ac` use `operator delete`, so a future
drain must select a service compatible with its actual node allocation domain.

The existing `run_named_call_threadsafe(MissionNamedCallHost&, const NamedCallRequest&)`
is a typed Host-mediated reconstruction, not an actual receiver plus five native
arguments. The counted `GameScriptOrdersHost::mission_lua_call_named_00887e50`
accepts one `std::string` and invokes a zero-argument Lua global through its chosen
interpreter; it also does not supply the required general raw service. No exact
`008876B0` Source definition or address reference was found in `include/` or `src/`.
Its live metadata is 58 bytes/18 instructions; the named dispatcher metadata is
984 bytes/296 instructions. Neither descendant body was inspected.

`MissionLuaDeferredCall` owns `std::string` and `std::vector` members, and
`MissionLuaCallQueue` owns a vector of those values. At
`src/mission_named_call_args.cpp:339..340`, `drain_named_call_queue` copies the front
and erases it **before** constructing a request and dispatching it. It therefore
does not preserve linked-node identity, callback-before-removal ordering, the
post-callback current-front selection, either invalid-iterator continuation, or
the native payload-destructor/free/count schedule.

`GameFixedStepHost` forwards to `GameStepSubsystemsHost`, whose separate
`lua_queue_` is that projected vector. Its `NamedCallBinding` supplies constant
state/stack values, logs unimplemented service boundaries, and discards queued
calls. The comments describe an empty main-thread path; they do not prove actual
queue behavior for the native owner. `GameMissionLuaHost` owns a separate
`lua_State*`; `GameNativeLuaServices` supplies fundamentals/globals/VFS bindings.
Their existence does not supply the missing raw named-dispatch/destruction pair.

## Lifetime, error, and evidence limits

The captured host must remain readable through every callback return, head reload,
free return, and count update. N and its passed subobject addresses must satisfy
the named callee's lifetime contract throughout dispatch. The retained R and its
links must remain valid through both unlink stores and payload destruction; that
destructor must leave R's base suitable for the subsequent free. Compatible
callees preserve ESI/EDI and the native callee-saved register contract.

Null host faults at the first count read; zero count returns even if head storage
is invalid or nodes remain. Nonzero count can dereference an invalid sentinel or
front. Invalid-handler returns do not cause automatic retry or front recapture at
the same site. A callback or destructor can affect subsequent queue/count reads;
there is no once-per-pass, fixed-work, FIFO-under-arbitrary-mutation, or concurrency
guarantee. Prior callback, unlink, destruction, and free effects remain when a later
operation fails. No catch, synthetic repair, cycle guard, or rollback is added.

Every live CLI batch verified project `bsp` and `/battlestationspacific.exe` with
x86/32-bit/image-base checks. A read-only exact flow-property request returned
`Script execution disabled`; no script enabling or mutation fallback was attempted.
Consequently, no exact no-return/flow-override metadata claim is made. The ordinary
CALL continuations and the path after free are established by bytes and the complete
listing, not by historical comments or unavailable flow flags. Native EH/unwind,
hardware-fault, actual Lua execution, ownership, and gameplay remain unproven.

The smallest next independent packet is a separately authorized, metadata-gated
audit of the 58-byte payload destructor. Actual named dispatch requires a separate
984-byte-scoped audit or appropriately justified subdivision; it must not be treated
as a leaf merely because a projected wrapper exists. This packet writes no Source,
CMake, ledger, or Ghidra changes and runs no builds, tests, probes, or native code.
It claims zero Original, Source, ABI, or gameplay credit.
