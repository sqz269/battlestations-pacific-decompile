# Native interpolator source emission

Addresses: 00b35540, 00b36e30, 00b37000.

Addresses: `00B35540`, `00B36E30`, `00B37000`.

These normal-path reconstructions append directly to the actual B0h builder's
8h string at +4C using the accepted B34F20/B35030/B35110 pooled helpers. They
consume the actual field pointer lists and two-byte mappings produced by
B36800/B34AA0. No ShaderSourceBuilder, copied descriptor graph, semantic host
callback, alternative field owner or text-only projection is introduced.
Names are descriptive hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B35540 pack | ECX builder; RET | Complete normal valid mapping/field/string domain |
| B36E30 struct | ECX builder; three stack flag DWORDs, low bytes used; RET0C | Complete normal source/storage behavior, native signed arithmetic retained |
| B37000 unpack | ECX builder; stack actual0Ch field-list header; RET4 | Complete normal valid mapping/field/string domain |

The saved `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified by
the BSP wrappers. All three bodies match live/disk bytes and have zero live-flow
gaps: B35540..B357CF (656 bytes/211 instructions), B36E30..B36FF5 (454/147),
B37000..B372C9 (714/229). The report records all 53 direct CALL instructions,
the 32 calls in the three reused line helpers, and four outer-generator calls.
Those helpers are reused unchanged, not newly claimed or renamed by this packet.

## Storage and producer evidence

B354D0 initializes the actual B0h owner and its 0Ch array headers. B34E20
constructs actual 1Ch fields with an owning 8h name. B36800 starts output with
ScreenSpacePos and appends selected fields from current descriptor70 and
mode-descriptor74. B34AA0 starts at field1, appending a low-byte field index and
component byte0..3 for each current mask bit; semantic2 feeds builder54/58/5C,
semantic1 feeds builder60/64/68. FOG writes a low-byte field index and zero second
byte at6C. The source emitters do not reinterpret the array as projected structs.

Pack reads current builder.fields28 data. Unpack instead retains the supplied
0Ch header's identity and reloads its current pointer-array data every row. Both
reload current mapping data/count and field name data after each previous
formatting operation. A null field-name data pointer uses the actual empty cell
0108D6F2; invalid fields/mapping pointers and component bytes outside0..3 are
outside the native valid domain. There is no invented mask/index clamp.

## Pack and unpack ordering

Pack writes its function header and brace through actual temporary8h strings and
B35030, then the local declaration and unconditional
`INT.Position = OUT.ScreenSpacePos`. TEXCOORD assignments precede COLOR, each
using packed register row>>2 and channel row&3, destination field/channel from
the mapping bytes. FOG follows when byte6C!=FF, then return and closing brace.
Pack has no vPos assignment or zero-fog branch.

Unpack writes its header, brace and local declaration, then the inverse
TEXCOORD/COLOR assignments. FOG comes from INT.Fog when byte98 is zero and from
literal0 otherwise. Only after that formatted line returns, it reads current
descriptor70+30, short-circuiting to descriptor74+30, to optionally emit vPos.
It does not initialize other unmapped fields before returning PixelIn.

Caller B39110 invokes struct atB3934E with position1/fog1/vPos0, and pack at
B3961A. Caller B39880 invokes struct atB3997D with position0, fog low-byte
`builder98==0`, vPos1; B39EF4 passes its current stacked field-list argument to
unpack. Stack cleanups and register provenance are recorded in the report.

## Struct arithmetic and current state

The struct emitter appends its opening line, optional Position, TEXCOORD groups,
COLOR groups, optional Fog and optional VPOS, then `};` plus the native extra
newline. The first group count is `signed32(DWORD(mapping_count+3))/4`, with
truncation toward zero; last width is signed32(mapping_count)%4, replacing zero
with4. It publishes count then width at7C/80 for TEXCOORD and84/88 for COLOR.
These words previously remained opaque in the shared builder declaration, so
this packet accesses the same offsets without changing that shared layout.

Every emitted row reloads current group count to determine whether it is last,
and reloads last width only for the current final row. The loop bound is reread
after each formatted append. COLOR's mapping count is not sampled until the
TEXCOORD loop has finished. Negative/wrapped bit patterns retain native
arithmetic; no bounded semantic replacement is used. Pathologically large
unsigned iteration domains are not claimed to complete quickly.

## Actual temporaries and failure

The opening literals have fixed requested lengths49/24/49; pinned50/25/50-byte
copies contain exactly their terminating NUL. Copy uses the existing BF7680
overlap-capable contract (`memmove`). The outer data pointer is captured before
the B35030 child. Pack opening and unpack opening/opening-brace cleanup combine
that captured pointer with the current temporary length after the child returns.
Pack braces, struct opening and unpack final brace instead use the length
captured before append. Pool return leaves the stale header words unchanged.

Each operation retains its actual temporary, captures, output owner, current
mapping values, and exact B35030/B34F20 or B35110 child on failure. The caller
must retain all borrowed owners/context/shared scratch and exclude retirement;
this is not an automatically installed owner guard. Replay and destruction of
an unresolved operation are rejected. Cleanup requires explicit resolution of
the temporary and child before diagnostic acknowledgment. A failed helper's
header is a preimage, not evidence that its child buffer remains owned.

There is no reconstructed native FH3 ABI/unwind or rollback of appended bytes,
group words or previous resource work. All source interfaces are new C++ APIs,
not drop-in binary entry points. CRT formatting/string operations and the
actual pooled storage/lifetime layer remain explicit existing dependencies.

## Validation

The report pins the standard registered strict Win32 build, both existing
CTests after verify-seeds, and one focused original/source fixture. The fixture
copies six audited normal instruction bodies, relocating only direct CALL
operands to the other copied line helpers or existing actual string/pool/CRT
services. Literal pages come from the unchanged installed executable; format
scratch is the same mapped0108D6F8 in both legs. No fake formatter or generated
text is supplied to either emitter.

Actual B36800/B34AA0 producers supply fields/mappings. Comparisons cover exact
output bytes and pooled allocation/release traces, distinct unpack names,
mapping/list replacement, live group/width/descriptor changes, low-byte flags,
empty/signed-edge group words, and retained parent/child failure. See finalized
report evidence for outcomes. No permanent tests are added. Full B39110/B39880
generation, material compilation and gameplay execution remain integration work.
