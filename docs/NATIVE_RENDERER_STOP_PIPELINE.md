# Actual renderer stop and pipeline

Addresses: 00b28a90, 00b26920, 00b33bf0

This packet reconstructs the complete normal bodies at `00B28A90`,
`00B26920` and `00B33BF0` over borrowed actual renderer, worker, owner,
event and synchronization storage. Names are descriptive hypotheses.
The new C++ interfaces carry explicit contexts and are not original ABI
replacements. Full worker construction/start/thread/destruction is separate.

| Routine | Inclusive end | Coverage | Original ABI |
| --- | --- | --- | --- |
| B28A90 stop worker mode | B28ABB | complete | ECX renderer; no stack arguments; RET |
| B26920 clear stop pipeline | B26A00 | complete | ECX renderer; no stack arguments; RET |
| B33BF0 request worker stop | B33BFD | complete | ECX worker; no stack arguments; tail JMP; forwards wait EAX |

The stop body is exactly 44 bytes. It captures renderer ECX in ESI, reads
current worker `+1970`, and calls B33BF0 only when that pointer is nonnull.
It then calls full B33AA0 with a zero stack byte, full B26920 on the captured
renderer, and real `Sleep(100)`. Its RET does not establish a semantic result.
There is no worker pointer clear, wake signal, thread join, wait-result branch
or timeout. An earlier exception prevents all later work, including Sleep.

B33BF0 writes worker byte `+04=0` before loading current acknowledgment owner
`+10`, its current table, and current slot `+08`. In the evidenced D6821C
profile this selects existing full BD17C0, which loads actual HANDLE `+04`
and returns `WaitForSingleObject(handle, INFINITE)`. Failed waits retain their
actual return value; B28A90 ignores it. Worker extent `14h` is a touched extent,
not a new whole-worker layout. The existing eight-byte NativeEventOwnerStorage
is reused. No semantic Win32Event or alternate wait provider is introduced.

The producer chain was freshly checked: B33DA0 at B33DD4 zeroes CL, calls
BD1970 at B33DD9 and stores returned EAX into worker `+10` at B33DEB.
BD1970 allocates eight bytes, publishes D6821C at BD198E, calls CreateEventA
with the zero-extended manual-reset byte and false initial state, and stores
the returned HANDLE at `+04`. D6821C `+08` contains BD17C0. The constructor's
event is auto-reset and initially unsignaled. A never-started worker can
therefore leave stop waiting forever. The bounded source preserves that fact.
The renderer producer also agrees: B32410 captures ESI=ECX at B3242D,
initially clears worker+1970 at B32473, allocates 24h at B328A0, calls
B33DA0 at B328BC, then stores its returned pointer at renderer+1970 at
B328CD. This producer observation does not reconstruct the whole constructor.

## Pipeline order and current providers

B26920 is 225 bytes, with an FH3 registration and one cleanup state.
The complete existing providers, including reached intrusive terminal
callbacks and their allocation/pool domains, execute at each step.

| Native site | Current target | Arguments and order |
| --- | --- | --- |
| B2694D | renderer current +130 -> B24710 | Twenty null textures, indices 0 through 19 |
| B2696D | renderer current +134 -> B24840 | Four null streams, indices 0 through 3 |
| B26985 | renderer current +138 -> B24B00 | Null logical index, base zero |
| B2698A | B241C0 | Actual cache identity renderer+34; full sparse clear/release |
| B2699A | B23D80 | Reload current default wrapper renderer+197C, color slot zero |
| B269AE | B33AD0 | Optional entry under current actual mode byte |
| B269D0 | current device +9C | Real SetDepthStencilSurface(nullptr) |
| B269ED | B33B00 | Normal leave only under current mode; state already disarmed |

Each loop reloads the current renderer table and its slot. Its evidenced
selector is D5F0A8. The source uses the actual borrowed profile's original
numeric words to select concrete providers; they are not host function
addresses. Unlike B262C0 and B24BF0, this body advances stream indices 0..3.
It does not call stream zero four times. Texture and stream callbacks can
change the current device subsequently used by later provider calls.

B241C0 is the current full implementation, not its older construction-only
fragment. Its 26 owner cells, release-before-clear order, sixteen texture
banks, twenty sampler-valid banks and sparse fields are retained. A throwing
terminal leaves the failing cell and later work as observed by that provider.
The source does not synthesize owners, shadow reference counters or duplicate
cache storage. Default-color binding increments its attempted counter only
after the actual call returns. The pipeline's direct depth call has no counter.

All binding and cache contexts must refer to the application's same canonical
globals, actual owner identity/counts, pools, allocators and current profiles.
Reached cache terminal domains are material D61A2C/D61A34/D61A3C, layout
D62AF4, index D61DE0, vertex D61D6C, textures D61948/D61870/D618B0 and frame
D5E600. Existing logical lifetime still requires its actual nested ownership
domain; the runtime producer of nonnull logical base `+4C` remains unproved.
No new success fallback extends any provider's input domain. Reached data
must remain valid at native access points. The nested renderer touched extent
is `1BC8h`; the current default surface must be valid.

## Cleanup boundary

At B2699F the pipeline tests the current mode. When enabled it writes the
captured renderer into the native guard and stores only the returned AL.
When skipped those fields stay uninitialized. It next loads the current device,
its current table and depth slot. Only then, at B269C8, is cleanup state zero
armed. Earlier bindings, cache clear and color binding are outside this outer
guard; their own full providers retain their own cleanup boundaries.

On normal return, B269D2 reads current mode, B269DA disarms state, then
normal leave may execute. Exceptional state-zero cleanup uses B21110.
There is no binding/cache rollback. The C++ cleanup wrapper terminates a
second C++ cleanup exception; arbitrary hardware SEH is not established.
Changing mode to enabled after skipped entry can expose uninitialized native
storage and remains outside the valid input domain, rather than being repaired.

The current saved unwind thunk CBD160..CBD167 forms ECX=EBP-14h and tailcalls
B21110. Handler CBD168..CBD171 is a complete ten-byte MOV EAX,DF5874/JMP
BF6B43 span with `no_ghidra_function`; no worker Ghidra mutation was made.
The original unwind map at DF586C and FuncInfo at DF5874 were captured.

## Verification and limits

Twenty-seven spans, 3,172 bytes, freshly match saved Ghidra and the installed
PE. Every query uses bsp.py's project/program verification for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, port 8089.
The installed game and analysis were not modified. The report carries hashes,
numeric native call-site rows, current source pins and the unknown-body note.

The owned source passes MSVC Win32 `/W4 /WX /O2 /MD /fp:strict`. After seed
verification, scripts/build.ps1 passes both existing tests. Shared CMake is
integrator-owned; this worker separately compiles its new source for the
fixture. No permanent test was added.

The ignored fixture is preserved in `C:/Users/sqz269/bsp-aw-renderer-stop`.
Its default `run.ps1 -Root <integrated checkout>` compiles only `probe.cpp`
and links the supplied current bsp_core, bsp_lua511 and bsp_zlib121 libraries.
`-WorkerSource` explicitly adds this packet's new source for initial checks;
`-LibraryRoot` can separately select those three libraries. The executable
uses `/MD` and `/MANIFEST:EMBED`. Original-address reservation collisions
return 77 and permit a bounded retry; no existing mapping is overwritten.

Seven original/source comparisons match 105,448 postimage bytes and 318 event
words. Two private real RTX 5090 HAL devices and their real backbuffer validate
twenty texture attempts, four distinct stream releases, index unbind, full
current cache, current device/default-color reloads, wrapping attempted
counters and preserved depth counter. Five current COM slots on each device
are observed and forwarded, then restored before destruction. Controlled
exceptions cover final depth, earlier texture and a raw cube-terminal COM
release during cache clear. The latter uses a fixture COM observation object
and genuine cube slab/pool/provider; it throws and has no successful stand-in.
The native final-depth case executes original FH3 cleanup through a host-image
jump-only trampoline. The two normal stop cases call actual Sleep(100).

Six actual-event comparisons cover original/source controlled blocking with
two current acknowledgment identities and null-HANDLE WAIT_FAILED. The test
creates and joins only its own short harness threads, observes run-byte zero
while the call is blocked, then signals the selected real event. Production
wait remains INFINITE. No original worker is created or started.

Mapped B28A90/B33BF0/BD17C0 and profile/handler/metadata bytes are unchanged.
Mapped B26920 has exactly two global-operand relocations into the same source
globals and one EH-registration relocation to the host trampoline. Its full
control flow, loop arguments and stack cleanup remain original. Dependency
bridges invoke current complete binding/cache/color/synchronization providers,
so this is a parent-composition differential check, not independent native
proof of every nested provider. Full worker lifecycle, native caller ABI,
general FH3/SEH behavior, arbitrary concurrency and gameplay remain unproved.

## AW integration analysis refresh

The integrator saved all ten AW original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. The seven-byte
B3D7B0 body and two ten-byte EH handlers CBD2FB/CBD168 were defined under
owned leases and the Ghidra write lock. Missing-function observations above
describe the earlier worker capture. EH definitions are analysis metadata,
not additional reconstructed normal-body claims. Combined final-commit
validation remains separate from the worker fixture evidence.

## AW exact merged validation

The exact combined source commit `842045e886c30b2dce4cb63f333cffb9a851697b` passed the Win32 build,
both existing tests and four current-library-only original-byte fixtures.
Full counts, original-byte relocation, exception branches and parent coverage
are recorded in `reports/native_renderer_device_recreation_aw_validation.json`.
The earlier pending statements describe initial capture stages. This does not
establish whole-game rendering, native ABI identity or general concurrency.
