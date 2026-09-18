# Native game resource parser ownership and registration

Addresses: `00717E80`, `00716FE0`, `007170B0`, `00717180`, `00717250`,
`00717320`, `00716380`, `007163C0`, `00716400`, `00716440`, `00716480`,
`007161C0`, `00716220`, `00716280`, `007162E0`, `00716340`, `007258F0`,
`006FB000`, `007186E0`, `007187E0`, `00718760`, `004DDB90`.

## Result

`src/native_game_resource_parsers.cpp` implements the full registration wrapper
and five parser singleton lifetimes: 21 normal bodies, 1,628 native bytes.
The raw game constructor now binds `004DDFC9 -> 00717E80` to these concrete
services. Its only remaining pure service is grid construction `0070BD70`.

`GameNativeResourceApplication` owns the five additional publication cells,
extends its finite name dispatch, and installs their secondary deletion
bindings before any registration can occur. It borrows the application's
existing raw singleton manager and string pool. `GameVfsHost` exposes that same
context for raw game construction. No alternate registry, private manager or
extra string domain is introduced.

This implements the previously analyzed registration/lifetime frontier in
`GAME_RESOURCE_PARSER_REGISTRATION.md` and the parser-registration follow-up
in `NATIVE_GLOBAL_CONFIG_LOAD_R117.md`. Parser payload/virtual-slot-8 behavior
and admission of the raw game constructor into the running application are
separate work.

## Recovered contracts

Names below are descriptive hypotheses, not recovered symbols.

| Registration order | Parser | Getter | Publication | Primary / secondary profile |
|---|---|---|---|---|
| 1 | Aux | `00717320` | `00E19B88` | `00CFD840` / `00CFD83C` |
| 2 | ZoneDesc | `00717250` | `00E19B8C` | `00CFD830` / `00CFD82C` |
| 3 | Note | `00717180` | `00E19B84` | `00CFD820` / `00CFD81C` |
| 4 | GeomMesh | `00716FE0` | `00E19BD0` | `00CFD800` / `00CFD7FC` |
| 5 | ConvexObject | `007170B0` | `00E19A90` | `00CFD810` / `00CFD80C` |

The 103-byte `00717E80` wrapper has no native inputs and returns with plain RET.
Each triple gets the resource manager, captures it, gets the parser, and calls
`00B80A50` with that captured manager and parser. It repeats the manager getter
five times and ignores registration's AL result. The implementation reuses the
actual raw resource-manager constructor, tree registration and duplicate rules.

Each 206-byte getter has no native inputs, returns the actual 8h parser in EAX,
and uses a captured lifetime-manager critical section. After the locked second
publication check it allocates eight bytes, stamps the initial secondary then
final primary/secondary profiles, publishes, captures the secondary pointer,
gets the lifetime manager again and registers that captured pointer. Normal
and supported C++ failure paths release the captured guard. Registration
failure retains the published allocation. The allocator-null path is retained
in source but was not forced in the new execution fixture.

Each 58-byte scalar deleter receives ECX=primary and stacked flags, returns
the original primary in EAX, and uses RET4. It clears the current publication
without requiring identity, stamps secondary `00CE3818` and primary `00CFD7D0`,
and frees only when flags bit 0 is set. Each 8-byte secondary thunk subtracts
four from ECX and tail-jumps to its scalar deleter. No unregister or parser
payload destruction is invented.

Each 33-byte name getter ignores incoming ECX and constructs a fresh actual
8h output header from the mapped original literal through `0041E870`. It
returns that output and uses RET4. The finite source dispatcher honors the
captured slot target without rereading the parser table, and forwards other
targets to the existing extra/default parser name dispatch.

The new singleton deletion binding is appended at source offset 152; all prior
member offsets are unchanged and asserted. Its source struct size is now 156.
Native profile DWORDs remain identities, not callable C++ vtables.

## Evidence and validation

- Project/program verified as `C:/Users/sqz269/bsp.gpr` and
  `/battlestationspacific.exe`. The report contains original PE provenance,
  complete native bytes/listings, and live/PE comparisons for 2,102 bytes:
  21 new bodies, four existing family baselines, profile/name data and the
  parent call site.
- All 20 getter/deleter/thunk/name bodies match their existing reconstructed
  family instruction by instruction after the recorded operand substitutions.
  Distinct FH3 handler pointers are explicitly excluded from any EH identity
  claim. Five returning-free flow gaps were repaired, restoring the 3-byte
  ADD ESP,4 after each free. Five missing secondary thunks were defined from
  their verified eight-byte bodies. Locked tools preserve the mutation records.
- The new differential fixture copies all 21 bodies (1,628 bytes), relocating
  45 direct CALLs, five tail JMPs and 45 publication/import operands. It uses
  the real application raw singleton manager, resource manager/tree, string
  pool, Win32 locks, CRT allocation, name primitives and mapped original data.
  Only the specified library/service ABI bridges are supplied; the 21 tested
  bodies themselves execute from copied original instructions.
- Four isolated application children compare original and source, both with
  cold VFS services and after actual VFS phase 2 mounts. Each side checks
  80 primary/secondary deletion cases, including flags 0..3 and publication
  identity changes, plus 66 name/state snapshots. Both pairs match 25,556
  bytes. Registration builds eleven parser entries, repeats without growth,
  then existing phase 6 adds AnimationChannels/Bone for thirteen. Repetition
  still preserves thirteen entries. The actual shared drain clears all
  thirteen parser publications, the resource manager, lifetime manager and
  shared raw string pool. Context metadata survives the host's earlier
  destruction, as in application shutdown ordering.
- The existing parent fixture passes all 33 cases, 1,657 observations and
  121,405,968 comparison bytes with the new context routing. Its parser
  boundary stays controlled; the separate application fixture above executes
  the full registration dependency chain.
- The full Globals.lua comparison still passes 36 loads per side, 7,560
  observations and 13,668,480 bytes. The genuine Dyn-world comparison passes
  four worlds, 280 buffer pairs, 204 explicit handle closes and CRT atexit.
- Strict MSVC Win32 build and all three existing CTests pass. No permanent
  tests were added. Exact library/executable/source hashes and immutable
  tested/integrated archives are recorded in the JSON report.

## Remaining work

Original FH3/SEH, private stack aliases, fault identity, arbitrary native ABI,
allocator failure and concurrent publication races are not validated here.
The application children exercise actual owners and mounted services, not
rendered gameplay or the complete raw game constructor. No ordinary windowed
application rerun is claimed for this packet.

The next required game-constructor service is `0070BD70` and its `0070B330`
grid/geometry dependency, including actual renderer buffer creation. Claim
those functions and output files before implementation. Full startup and
gameplay validation remain required for the overall reconstruction goal.
