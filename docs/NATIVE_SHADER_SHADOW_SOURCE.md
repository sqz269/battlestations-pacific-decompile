# Native shadow helper source

Addresses: 00B38230, 00B382B0; existing dependency 00B34F20.

`B38230` and `B382B0` now append GetShadow/GetMapShadow text to the existing
actual B0h builder's `source_4c` using the unchanged native `B34F20` C-string
line appender. They reuse `src/shader_shadow_literals.inc` unchanged. No semantic
builder projection, format callback adapter, private output string or duplicate
line-appender implementation is introduced. Names are descriptive hypotheses.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `B38230` GetShadow source | ECX actual B0h builder, no stack arguments, plain RET | complete 5Ch normal body, RET at B38272/B3828B |
| `B382B0` GetMapShadow source | ECX actual B0h builder, no stack arguments, plain RET | complete 5Ch normal body, RET at B382F2/B3830B |
| `B34F20` line dependency | ECX actual8h output, one C-string stack argument, RET4 | existing complete native line appender reused unchanged |

`B39880` calls these helpers at `B39A77/B39A7E`, gated by current descriptor74
byte15. Both sites put the builder in ECX without a stacked argument. Every
helper line CALL pushes exactly one string and relies on B34F20's RET4. There
are six direct line-call sites in each helper and no disassembly gaps.

The first intro line returns BEFORE `B38243/B382C3` reads builder byte `+AA`.
That byte is captured once. A nonzero value selects four total line calls:
intro, projected position, projected return, closing brace. Zero selects three:
intro, filtered body, closing brace. Callbacks during the intro can change the
selection; callbacks during later lines cannot. Existing builder storage names
the byte `policy_aa`; its producer supplies the policy through native compiler
setup. No new interpretation of the device feature that controls it is claimed.
Specifically, B3C455 loads CL from the wrapper's policy argument and B3C465
writes `[ESP+BE]` after two pushes: the builder began at the earlier `[ESP+0C]`,
so this store is exactly builder+AA. Constructor B354D0 preserves that byte.

| Text | Literal address | GetShadow call | GetMapShadow call |
| --- | --- | --- | --- |
| GetShadow intro | D600B0 | B3823E | |
| GetMapShadow intro | D60C38 | | B382BE |
| Projected position | D60060 | B38253 | B382D3 |
| Shadow projected return | D60010 | B3825F | |
| Map projected return | D60C04 | | B382DF |
| Shadow filtered body | D5FB08 | B38278 | |
| Map filtered body | D606E8 | | B382F8 |
| Closing brace | CE4CD4 | B3826B/B38284 | B382EB/B38304 |

The eight literals, including their NUL terminators, were compared against
both live Ghidra and the installed PE. Their exact spacing, tabs, strict depth
comparisons, map size constants, branch text and newline schedule remain intact.
The reused line appender constructs a pooled copy, captures its data/length
across output resize, releases it, then appends/releases a separate pooled newline.

`NativeShaderShadowSourceOperation` is persistent host metadata outside native
storage. Each line uses an embedded optional `NativeShaderStructDeclarationOperation`;
a returned child can be replaced, while a failed child and its actual temporary
headers remain available. The parent records helper/call site, literal pointer,
completed lines and whether AA was captured. A failed intro leaves capture false.
Replay is rejected before side effects. Running/failed frames terminate on
destruction. Diagnostic acknowledgement frees nothing and requires the caller
to resolve all retained acquisitions first, including existing helper cleanup
boundaries: an entered header does not prove a buffer is still owned. The caller
keeps context/builder/strings alive and excludes retirement externally. No native
terminal interception, rollback or private FH3 unwind is installed.

One focused fixture runs the two original bodies plus copied original B34F20
against the reconstructed helpers with actual initialized builder storage and
the canonical actual pooled-string owner. Original internal branches retain
their offsets; audited direct dependency calls bind to established C-string,
resize, pooled-storage and overlap-copy bridges. All four helper/branch choices
compare exact text and pooled allocation/release traces, existing prefixes and
untouched builder bytes. AA changes during the intro newline release select the
new branch; another change during the body proves the choice remains captured.
A source-only allocation failure retains the first child temporary/output preimage
before AA capture; replay rejection and failed-frame retirement exit77 are checked.

The report pins the original 456 body bytes, 3,337 literal bytes, fixture inputs,
runner, executable, logs and build artifacts. Win32 build/CTest and fixture results
are recorded separately. This is source emission evidence: full native source
generation, shader compilation, shadow rendering, original binary ABI replacement,
native FH3 and rebuilt gameplay remain unproven.
