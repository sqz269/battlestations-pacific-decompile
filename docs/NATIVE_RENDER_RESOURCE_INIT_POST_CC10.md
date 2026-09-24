# Render-resource initialization: B11599 post configuration

Addresses: `00B107F0`, fragment **[00B11599,00B118AF)** only.

This stage configures the actual `+70` HDRFinalPass owner published by the
previous continuation. It binds two input textures, registers seven borrowed
parameters, and assigns its color output. It stops before `B118AF PUSH 20h`,
the beginning of the next allocation. It neither completes initialization nor
admits destruction. No new owner registry, numeric host-vtable dispatch,
resource counter, field initialization or fallback provider is introduced.

| Routine | Coverage | Interface |
|---|---|---|
| `00B107F0` | Partial: only `[B11599,B118AF)`, 790 bytes. Earlier `[B107F0,B11599)` belongs to the entry/continuation packets; later `[B118AF,B13029)` is excluded. | Native containing function: ECX actual service, three stacked DWORDs, eventual `RET 0C` at `B13026`. New C++ function: retained predecessor, same context, retained post-stage state. No native return is executed here. |

The final included instruction is the five-byte `CALL B4CB70` at `B118AA`;
its inclusive final byte is `B118AE`. Live Ghidra instruction context confirms
the first instruction `B11599 MOV ECX,[ESI+64]` and excluded next instruction.
The containing body remains `B107F0..B13028`. Ghidra was read only through BSP
wrappers that verify `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`.
All 790 live bytes matched the installed PE; SHA-256 is
`f0de11cb12b48a06e546470f3cf76cf53d885245326e0fee7f174b8f7525a2c3`.
The report contains all 45 CALL sites; there are no missing function definitions
or new native names in this packet.

## Ordered bindings

1. Read current service `+64`, call `B4D170` for its holder and `B4CB10` for its
   texture. Read current `+70`, get its actual `+14` material through `B4CBA0`,
   then `B189F0` binds texture slot zero (`B115B5`).
2. After that call returns, read current service `+28`, call `B54CD0` for its
   output holder and `B4CB10` for the texture. Reload current `+70` and material,
   then bind slot one (`B115D6`).
3. Construct, register and return each temporary parameter name in the table
   below. Every source remains an actual address; this stage never snapshots
   the pointed-to float values. The context supplies the same original literal
   bytes used by the prior continuation. Live string bytes matched the PE.
4. After the final name return, read current service `+4C`, get its primary
   surface through `B4CB20`, reload current `+70`, then call `B4CB70` (`B118AA`).
   This existing wrapper reloads the actual post `+08` frame and calls full
   `B1FAB0` for color slot zero.

| Name / native literal | Borrowed source | Word count | Registration / EH state |
|---|---|---:|---|
| `cMiddleGray` / `D5E3F0` | service `+8C` | 1 | `B11610` / 32 |
| `cBloomScale` / `D5E3E4` | service `+94` | 1 | `B11676` / 33 |
| `cBloomSampleOffset` / `D5E3D0` | current service `+28` bloom, then `+428` | 2 | `B116DE` / 34 |
| `cMinLuminance` / `D5E3C0` | service `+98` | 1 | `B11744` / 35 |
| `cMaxLuminance` / `D5E3B0` | service `+9C` | 1 | `B117AA` / 36 |
| `cNewHDRParams` / `D5E374` | inherited EBP bits, service `+A0` | 2 | `B1180A` / 37 |
| `cSceneColorSampleOffset` / `D5E430` | service `+04` | 2 | `B1186D` / 38 |

Each name uses `41E870` (ECX header, stacked literal, `RET 4`). `B18B20` and
`B18B00` forward to existing `B17E10` with word count one/two and matrix zero,
then `RET 8`. The material shader and actual parameter provider retain their
existing profile and capacity requirements. Null shader behavior remains that
of the provider. No new interpretation of those service fields is asserted;
the names are the original shader parameter strings.

Current `+70` is read at each native receiver site, including the different
receiver-before-source ordering in rows two, four, five and seven. Row three
loads the current bloom after name construction. Row six uses the previously
retained EBP address rather than reconstructing it from a new service snapshot.
The two texture calls and final frame assignment preserve actual provider
publication, incoming retain, and captured outgoing release order. Their
existing counter/import and terminal-profile boundaries remain in force.

After each registration, the caller captures the name's **current** data
pointer, disarms EH state to `-1`, and only for nonnull data captures current
length plus one before `419CC0`. That current singleton consumes no stacked
arguments. `BD1510` receives `(data, length+1, 1)` and executes `RET 0C`; its
third word is unused. The seven distinct retained headers are left stale after
return, just as the native caller does. Their diagnostic pointers do not own
the returned storage. Native stack-slot aliasing remains excluded.

## Continuation and lifetime

The earlier actual producers and their contracts remain authoritative:
`+64` comes from `B542D0`, `+28` from `B54E70/B54F90`, `+4C` from `B4E020`, and
`+70` from `B4E470`. All are held by the retained previous construction state.
No reconstruction layout or substitute object is introduced for those owners.

The previous state now records its exact context object's identity at initial
consumption; the prior source change is limited to that assignment. Its header
adds this identity, a one-use post-stage identity and phase values. Entry
validation requires the published `B11599` site/state/mask, original entry
identity, original context, original borrowed argument cells and inherited EBP.
It claims the predecessor once before invoking any provider. Changing the
context object or replaying either state is rejected.

On failure, all three states become `failed`; the failing native site/state,
actual publications, prior construction blocks, parameter source/material
receipts and name headers remain retained. There is no automatic rollback,
retry, resource release, metadata unbind or child reset. The same context,
predecessor, entry, service, argument cells and all retained child blocks must
remain alive and immovable through later dependent stages. Existing child-block
quiescence requirements continue to apply. Host metadata does not add resource
credits or establish native FH3/SEH cleanup.

At `B118AF`, ESI is the same service, EBX remains `FFFFFFFF`, EBP remains
service `+A0`, EDI/ESP14 retain the raw `+70` allocation, ESP20 retains half
aligned width, ESP58 aligned width, ESP24 aligned height, ESP18/1C original
dimensions and ESP1D8 half aligned height. ESP10 mask is zero and EH state is
`-1`. Original argument cells are retained without reads in this stage.
Volatile registers and native flags are not represented by this source API.

The seven service-visible pass companion registrations remain external.
The published companion bridge is not wired here. Nested holders, surfaces and
textures keep their direct lifetime paths; blanket registration would not
establish correct retirement. Full initialization and teardown remain gated.

## Verification

The report-call verifier passed all 45 direct call rows with zero failures.
`./scripts/build.ps1` passed the fresh MSVC Win32 Release build and both existing
CTests (`reconstructed_math`, `tool_tests`), zero failures. The Release target
uses `MultiThreadedDLL` (`/MD`). No new tests, native probes or app wiring were added.
This composed stage has static/build evidence only. The prior continuation's
two leaf probes do not establish runtime behavior of either composed stage.
Native ABI, FH3/SEH, production device/shader execution and gameplay parity
remain unclaimed.
