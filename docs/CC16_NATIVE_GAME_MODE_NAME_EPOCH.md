# Native initialization mode-name epoch: caller gate

Baseline: published Main `469375b4cdca7a60b91a13f0f052c1d2ce248a13`.
This packet revalidated the proposed mode-name preparation against the actual
current caller. **The complete epoch is blocked by its release hook. No source
or unused API is added.** The earlier readiness recommendation established the
real duplicate/free services, but did not establish a genuine current caller
crossing at native 73E466.

`GameStartupHost::run_initialize_phases(mode)` provides the actual nonnull mode
pointer. It is validated by `application_initialize` and currently passed
directly to `parse_command_line_0073ce20`. The native producer is closed:

| Saved native site | Actual effect |
| --- | --- |
| 73D933 | `MOV ECX,EBP`, the retained actual mode. |
| 73D93C | Call real 438E40, whose current Source returns an owned canonical allocation. |
| 73D945 | Store EAX to actual E1AE78. |
| 73D94A | Parser reads that current publication. |

Current `src`/`include` references to E1AE78 are only bootstrap comments/contracts
and the existing `GameNativeGameApplicationFrame` borrowed reference. There is
no real cell, application writer, runtime frame instantiation, legacy address
map or release caller to update. The standalone `BootstrapHost::command_line()`
API is an explicit semantic input contract, not another raw publication owner.

The only justified authority choice, if the whole epoch is later composed, is
one appended `char* volatile` authority in the existing retained
`GameNativeStringProcess`, alongside E1AE90 and the canonical AA0/AA8/AA4 domain.
Its one-shot phase and historical allocation/capture/disposition metadata must
also survive host destruction and interrupted initialization. The host and
future Game frame would borrow that same cell. No second host cell, shadow map,
new Application or mode substitute is justified. Those process files were read
but not leased or edited because their genuine caller gate is not closed.

## The missing release crossing

The saved complete parent places release after real Game and preload work:

1. Native 73E150..73E1D9 allocates/clears/constructs the actual 71A0h Game and
   publishes Application+14. Native 73E1DA reloads that Game, 73E1DD reads
   Application+1A, and 73E1E8 stores the byte into actual Game+719D.
2. Five raw FileStore preload blocks run for inputs, keyboard setup, controller
   input names, control presets and scoring. The last pool return is73E457.
3. Only then does the parent reach73E45C and capture CURRENT E1AE78.

The exact release sequence is `MOV EAX,[E1AE78]` at 73E45C; null comparison and
branch at 73E461/463; `PUSH EAX` at 73E465; BF6989 call at 73E466; stack adjustment
at 73E46B; and clear CURRENT E1AE78 at 73E46E. It clears after normal free even if
the callee changed the cell. Null at the initial capture skips both free and
clear. The next native operation reloads actual Application+14 at 73E474 and
calls whole 4E5540 at 73E477. The free uses the captured current pointer, not a
remembered original producer result, and performs no identity-checked clear.

Current `run_initialize_phases` does not execute that intervening parent
segment. After the real GUI/diagnostic checkpoint it runs optional probes and
mission-render preparation, then a decal loader originally at 73DE8C and
projected `GameMenuHost` / OnInitTitle work. It does not construct the raw Game,
call its application frame, or execute the five actual preload blocks here.
Its projected title point therefore does not establish a genuine73E466
crossing. Reaching the end of this partial initialization function, seeing
`native_application_live_`, or constructing the menu is not proof that native
Game/preload ownership has returned.

Inserting free before projected title would silently select a new Source
subsequence policy that skips those dependencies. Installing an active producer
with a dormant release method would instead retain a name across every ordinary
run; normal/failure exit and replacement-host custody would then require a new
explicit policy. Neither change is inferred from the requested closed native
epoch. A new log, null Game, success flag or empty provider would not supply the
missing transition.

## Smallest prerequisites for a later source packet

The first path is to supply the actual retained Game/frame and reached preload
caller contracts, then put release immediately after their genuine completion.
The full Game private-byte, GlobalConfig, grid/Lua and lifetime gates remain as
identified in the separate readiness audit; this packet does not widen them.

No Source subsequence policy is introduced. Skipping the unbound Game/preload
segment, enabling a producer without a genuine release caller, adding a dormant
release API or making ordinary startup terminal would change the requested
contract. The integrator explicitly selected this bounded blocker receipt.

The future owner must reject reentry/replay, record attempted
duplication before its external call, preserve any unpublished allocation or
indeterminate free disposition on interruption, and keep independent metadata
when the published cell changes. It must not free at catch time to satisfy an
RAII destructor. The real 438E40 allocation and canonical matching free already
exist; no vendor or native body reconstruction is necessary for them.

## Evidence and scope

The companion report pins the current caller/owner/contracts, saved73D410
export triple, and the configured original executable. The release instructions
were read through bounded `bsp.py show` assembly pages. Bounded `disasm-raw`
queries independently confirm both complete producer/release windows in the
installed original PE. Saved metadata identifies project `bsp`, program
`/battlestationspacific.exe`; configured project remains existing
`C:/Users/sqz269/bsp.gpr`. No live Ghidra batch or mutation occurred.

Only this document and its report are changed. No source, CMake, guard, ledger,
shared metadata, installation or frozen peer tree is edited. No compile, build,
test, probe, executable, game or DLL is run: there is no implementation to compile
or link. The integrator received the authority/hook findings before edits.
New Source, native, vendor, ABI and full-Game activation credit are all zero.
Names remain descriptive hypotheses. Existing runtime adoption gates remain
unpassed.

From the existing readiness audit, the smallest already closed scalar component
is `delete_native_game_grid_0070b280`, using the actual 84h allocation, SAME
construction `NativeGameGridContext`, current stream ownership and matching
canonical allocation/free domain. Its normal destructor and scalar body already
exist with historical build/device-context evidence. That does not make a new
application dispatch packet ready: actual allocation/publication provenance and
source-family selection remain uncomposed, and numeric CFD484 is not callable.
The raw Lua host scalar chain remains unready. No new grid/Lua scan or body work
was performed in this packet.
