# Process lifetime for the pending entity owners

Addresses: `00F899A8`, `00F899B4`, `00CD3910`, `00CD3940`, `00CDF4A0`,
`00CDF4B0`. Ordering evidence: `00CCD6A0`, `00CD2D80`, `00CDEEC0`,
`00BFBC47`, `00BF6ED1`, `00BF6FB9`, `00BF6FF5`, `00BFBCD9`.

`bsp::game::game_pending_entity_owners()` exposes one process-static
`NativePendingEntityOwners` pair. Its destroy and kill fields represent the
actual adjacent 0Ch owners at F899A8/F899B4. The existing raw types and lifetime
operations come from `native_pending_entity_owners.hpp`; no alternative queue,
node type, STL container, payload ownership, or callback registry is added.

The original 18h storage lies in the loader-zero-filled part of `.data`:
none of these 24 bytes is disk-backed, and all 24 live Ghidra bytes are zero.
The new trivial namespace-static object has the same zero-filled pre-startup
state. Its accessor returns the same bytes and has no initialization effect.
A null head and zero count before startup do **not** constitute an initialized
empty ring and cannot be used by the pending drain.

## Explicit startup and actual CRT ownership

The startup integrator calls
`initialize_game_pending_destroy_owner_00cd3910()` and then
`initialize_game_pending_kill_owner_00cd3940()` exactly once. Each delegates
to the complete R wrapper with this same owner pair and a constant registration
binding. There is no C++ dynamic initializer, per-host construction, lazy
initialization, or repeated-startup guard. Calling either function again is
outside the once-only native CRT invocation contract; it is not made safe or
turned into a no-op.

The private registration bridge translates the two fixed native shutdown
identities into noncapturing process exit functions and calls real
`std::atexit`. Its context is null and unused. The raw owner object is trivial,
so it has no C++ destructor that could run before the registered callbacks.
Neither the owner bytes nor a borrowed host/log/registration object can expire
when the startup caller returns. Each callback delegates to its R destructor
against the same process-static bytes.

Both functions return the actual CRT registration result unchanged. Failed
registration retains the initialized owner, does not invent a callback, and
does not roll back allocation. Allocation failure remains the R wrapper's
exception path before publication/registration. This packet adds no extra
failure behavior on these valid paths. The private bridge rejects an unknown
shutdown identity with `std::terminate`; neither fixed R caller supplies one.
That unreachable binding guard is not a recovered native branch.

There is no public manual teardown operation. Process shutdown must be
quiescent, with initialized intact finite rings and no duplicate teardown.
The real CRT callbacks free list nodes and sentinels, clear head/count, and
preserve owner+0 and borrowed entity payloads. They do not dispatch pending
events. Module unloading while its CRT callbacks remain registered is outside
the executable-lifetime contract.

## Recovered CRT table order

The complete 1004-entry C++ initializer table occupies
`[00CE2734,00CE36E4)`. All 4016 disk bytes match the live program. Each entry
below occurs exactly once in that table; these are zero-based cell indices.

| Index | Pointer slot | Target | Direct CRT registration |
| --- | --- | --- | --- |
| 286 | `00CE2BAC` | `00CCD6A0` observer dispatch publication | none |
| 584 | `00CE3054` | `00CD2D80` settings initialization | `00CDEEC0` at `CD2D8F` |
| 611 | `00CE30C0` | `00CD3910` pending destroy initialization | `00CDF4A0` at `CD392E` |
| 612 | `00CE30C4` | `00CD3940` pending kill initialization | `00CDF4B0` at `CD395E` |

`__cinit` at `BFBC47` loads those bounds at `BFBC90`/`BFBC97`, skips null
cells, and calls each nonnull entry at `BFBCA7`. `BFBCA9` advances the pointer
by four before the next bound check. Therefore the two explicit pending
initializers belong after both observer publication and settings initialization,
with destroy immediately before kill in the native table. This component does
not invoke those earlier initializers or implement the intervening table cells.

`CCD6A0[14]` calls `696360`, adds four to the returned address and publishes
E198E4. It has no direct `_atexit` call. Its position does not establish a
direct observer-owner teardown slot in the CRT callback order. Existing
singleton ownership and application shutdown remain separate responsibilities.

`BF6FF5` (`_atexit`) calls the locked registration wrapper `BF6FB9`, which
calls `BF6ED1`. On a successful append, `BF6F6A` writes the transformed callback
pointer to the current end, `BF6F6C` advances that end by four, and `BF6F76`
publishes it. The normal exit path in `BFBCD9` decodes the current bounds,
subtracts four at `BFBD35` **before** the bound check and callback at `BFBD57`,
and repeats by jumping back at `BFBD59`. Thus, when all three registrations
succeed, the relative callbacks are `CDF4B0` (kill), `CDF4A0` (destroy), then
`CDEEC0` (settings). Other registered callbacks may intervene. No complete
process-wide destruction schedule or original VS2005 CRT execution is claimed.

## Coverage and interfaces

| Source entry | Native identity / ABI | This packet's coverage |
| --- | --- | --- |
| process destroy initializer | `CD3910`, no native inputs, EAX registration status, `RET` | complete process binding to existing R wrapper |
| process kill initializer | `CD3940`, no native inputs, EAX registration status, `RET` | complete process binding to existing R wrapper |
| private destroy exit function | `CDF4A0`, no native inputs, ECX=F899A8, tail JMP `924A70` | complete process binding to existing R wrapper |
| private kill exit function | `CDF4B0`, no native inputs, ECX=F899B4, tail JMP `924A70` | complete process binding to existing R wrapper |
| raw owner accessor | native data F899A8/F899B4; no recovered function counterpart | new C++ ownership interface |
| CRT walkers and settings/observer initializers | 12 complete byte spans retained for ordering evidence | read-only evidence; no new reconstruction here |

This is a new C++ interface, not a binary replacement or a reconstruction of
the whole pending subsystem. Native address names remain hypotheses. The R
packet's full-byte/canonical-helper proofs and stored Ghidra teardown-flow
limitations still apply. S performs no Ghidra mutation.

The future `PendingEntityQueueHost` must borrow these exact owner references
after startup. Producers, cancellation, copy/clear/drain operations, callbacks
into entities, and startup/frame wiring are not implemented by this module.
No existing game host, singleton host, event drain or producer source is edited.

## Focused proof

MSVC Win32 Release build and both existing CTests passed. The single focused
process-exit fixture passed all three exit observations with no provider
rebinding, and the live report gate passed all 13 direct call rows. Eight
existing seed spans matched the installed image before building. Ordering
evidence covers 14 spans / 4809 bytes: 12 code spans, the complete table, and
24 loader-zero-filled owner bytes (not disk-backed bytes).

The ignored source-composition fixture links the actual built `bsp_core.lib`
without rebinding or intercepting its owner, allocation, registration, or exit
providers. Its only extra CRT registrations are observation callbacks before,
between, and after the two module registrations. After the initialization
function's stack scope has returned, `main` returns normally. The real CRT
then proves three observable phases: both owners live; kill cleared while
destroy remains intact; both cleared at the same stable process addresses.
One raw fixture node per list proves that these are the lists actually freed,
with payloads and allocator preimages preserved. Direct ring seeding is fixture
setup, not a claimed producer implementation. Observation callbacks never
perform module teardown themselves and never replace the exit registry.

The exact source, compiler/link commands, actual provider library and objects,
link map, executable, native evidence and result logs are retained under
`local/pending-runtime-proof/` and hashed in its manifest. The R manifest is
retained as a dependency. No permanent test cases are added. The report states
the final Win32 build, existing two CTests, call-site gate and process-exit
fixture results, distinguishing them from absent game/native-EH validation.

The immutable R manifest retained `Hostx64/x86/cl.exe` and `link.exe` as tool
artifacts, but its fixture command called `vcvars32.bat`. That script explicitly
selects `x86`; S's captured `where cl`/`where link` resolves the fixture tools
to `Hostx86/x86`. R therefore lacked hashes for those fixture tool binaries.
S retains the actual resolved tool paths/hashes, the selected environment and
the `vcvars32.bat` evidence. The R manifest is unchanged; its independently
verified source, libraries, objects, executable and result logs remain valid.
