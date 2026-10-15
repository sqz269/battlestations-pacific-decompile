# Current Win32 sampler provider qualification

The real cached GuiFont/GuiFontBilinear acquisition and deferred fakemotion
cold/warm case now complete with the current installed Win32 DLLs. They retain
and retire their actual caller credits, pass the shared native drain and finish
with device/API COM counts zero. This establishes acquisition/lifetime behavior;
it does not establish POST drawing, Native Text drawing or gameplay.

The machine-readable receipt is
[`cc12_sampler_provider_support_29.json`](../reports/cc12_sampler_provider_support_29.json).
The earlier rejected profile diagnosis is preserved separately as packet 28.

The additional exact profile admits this complete DLL triplet:

| Module | Legacy timestamp / image size | Additional timestamp / image size |
| --- | --- | --- |
| D3D9 | `7678A845 / 17A000` | `3F41597F / 179000` |
| NTDLL | `427CC5BD / 1BF000` | `C96C22CB / 1C0000` |
| Kernel32 | `0F052F7B / F0000` | `01212B3C / F0000` |

Neither mixed revisions nor unknown versions are admitted. The original literal
tables and targets remain supported. Qualification still checks full selected
code bytes, recorded HIGHLOW relocations, CFG/import cells, virtual targets,
raw object associations, exact shader counts, retained device counts, actual
slot publication and native-owner release observations before granting a word.

The passive correspondence audit compares 11 spans, 672 bytes and 273
instructions against the existing bounded certificate. Stack/data instructions
and 24 internal branch targets have the same layout; three profiled callee edges
map positively. Runtime uses exact new bytes, including relative displacements.
The masks used to locate candidates are never runtime admission rules.

| Provider operation | Legacy RVA | Additional RVA |
| --- | --- | --- |
| Shader Release | `7EB90` | `7EAB0` |
| Release guard constructor | `489E4` | `489D4` |
| Shader counter helper | `48A34` | `48A24` |
| Device AddRef | `81990` | `818B0` |
| Nonfinal device Release prefix | `65BD0` | `65AF0` |
| CFG check stub | `817B0` | `816D0` |
| Shader AddRef wrapper | `122490` | `122170` |
| RtlLeaveCriticalSection | `53460` | `51660` |
| Backoff helper | `42410` | `40630` |
| InterlockedDecrement | `31AB0` | `31E50` |
| InterlockedIncrement | `31AF0` | `31E90` |

These are **module-relative** RVAs, not addresses in the game program. Ambiguous
short-body search hits were resolved through data identities: all four shader
Release table slots have adjacent AddRef target `122170`; the PE load-config
CFG cell `164790` identifies stub `816D0`. Import cells `1642B0/1642B8` name
Kernel32 Leave/EnterCriticalSection, whose forwarders resolve to the profiled
NTDLL entries. The current nonforwarded Kernel32 increment/decrement bodies are
byte-identical to the complete existing 20-byte bodies.

The same caller relationship remains: `W=P-30h`, with the release guard saving
the builder and RtlLeave's `[EBP-8]` local overlapping W. Its actual backoff
helper receives that local's address. Nonfinal device release and the later
slot increment/native-owner decrement do not replace that captured W. The
unchanged 96-byte bridge captures the real value immediately after COM Release.
It has no padding or relocations and matches the frozen probe, current object
and linked game byte for byte.

The compiler Frame header and actual compiler C++ file are unchanged. Provider
selection adds no frame member or default word. The standalone Source1 release
qualifier recognizes the current exact Kernel32 export with the same complete
instructions; its nonzero/count/identity/before-after requirements remain.

Source `22C2B894B` passes the canonical build and three existing checks in 7.30
seconds, then completes the real GUI-only acquisition case. Source
`29E1D0A6BE57C52D69818AFD3141A7D39E392C7A` adds compact read-only observations
through the existing sampler hook and passes the same checks in 7.50 seconds.
The hook finds the live compiler child by exact builder/pass identity and
**returns empty evidence**. It samples known-ready metadata inside `begin()`,
before the compiler publishes the captured value into its fresh root/mode
scope. It never supplies, replaces or universally reuses a caller word.

The observed GUI plus POST-acquisition run exits 0. Three compiler invocations
each report a matching profile, returned bridge call and qualified word. Each
shader count is 2 before release with return 1; corresponding device counts
remain `23/23`, `25/25` and `27/27`. The selected release RVA is `7EAB0`.

The real cache cursors advance `0 -> 704 -> 708 -> 1080` for the two fonts and
cold fakemotion. This records cursor values, not the number of shader bodies
compiled or exhaustive cache coverage. After normal shader-cache close,
fakemotion's warm call uses the exact cold name header and effect identity at
`B31436`, with a distinct acquisition frame and no new compiler observation.
Its count stays 2 across shader-cache close, becomes 3 on warm acquisition,
then retires warm `3 -> 2` and cold `2 -> 1`. Both font caller credits retire
`2 -> 1`; shared drain completes with final COM counts zero.

The selected fakemotion descriptor remains the actual SceneTexture Source0
case with empty mode samplers. This run does not prove the procedural Source1
nonzero/zero-terminal parent, POST submission/draw, full Native Text buffers
and sections, a visible native world, complete frontend or gameplay. Forty-three
host methods remain unimplemented in the smoke test. Historical Original-process
module/IAT identity, whole register/private-frame/FH3/SEH behavior, arbitrary
concurrent mutation and live legacy-host regression execution are not claimed.

The local evidence preserves the rejected attempt, full passive module copies,
paired disassembly/data mappings, exact compiled families, bridge attestations
and both successful runs. Original installation and saved analysis are preserved;
Ghidra names/prior comments remain, new bounded evidence is appended, the project
is saved and affected game-function exports are refreshed. No new test suite or
reference-count fixture was introduced.
