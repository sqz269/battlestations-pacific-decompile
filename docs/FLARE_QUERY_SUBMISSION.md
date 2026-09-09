# Flare query submission: bounded unresolved dependency

The query Begin/End submission remains unresolved. This follow-up closes two
misleading routes: the two query-pointer getters are not entries in the
observed flare vtable, and the three flare geometry helpers do not directly
issue queries. No depth-state assignment to either query is established.

The owner constructor installs vtable `00d63420`. Its 23 DWORD entries occupy
`00d63420..00d6347b`; the next bytes are `flare\0\0\0`. Neither `00b8b6b0`
nor `00b8b6c0` occurs in this table. Live xrefs report no references to either
getter, and an installed-PE absolute-pointer search found neither address.
This does not prove the helpers are unreachable through every possible
computed address, but it rules out treating them as observed flare slots.

Their complete bodies are `8b 81 d0 01 00 00 c3` and
`8b 81 d4 01 00 00 c3`: ECX owner, no arguments, borrowed pointer returned
in EAX from owner `+1d0h` or `+1d4h`, ordinary RET. They do not Issue, poll,
or transfer a reference. A targeted search for direct owner-field loads
identified the already-audited getters, cached-result update, and destructor
in the flare range; this limited encoding search is not an exhaustive
interprocedural alias analysis.

## Geometry consumers

Assembly and pseudocode for `00b8c6b0`, `00b8c8a0`, and `00b8cc90` agree on
ECX owner and two stack arguments, ending in RET 8 at `00b8c89a`,
`00b8cc80`, and `00b8d114`, respectively. They populate four vertices with
40-byte stride. Their direct call sites contain no indirect query dispatch,
renderer draw dispatch, or depth-state call. This statement concerns their
own bodies, not all transitive callees.

`00b8c6b0` calls only `00b8c520` at `00b8c7ce`; it fills position, a packed
value derived from owner `+ach`, and UV fields. `00b8c8a0` additionally calls
transform refresh `00b6db70`, camera matrix getter `00b70490`, and transform
helper `00b62d10`; its value conversion is at `00b8cba7`. `00b8cc90` has
the same categories of calls and converts both owner `+ach` and `+184h`
at `00b8cfa5`/`00b8cfc4`. The exact packed-value meaning requires its own
audit; it is not necessary to invent it to distinguish vertex preparation
from an observed Issue/draw/Issue scope.

## Best next submission dependency

The concrete flare table slot `+20h` points to `00b748e0`. This method uses
ECX owner and four stack arguments (RET 10h at `00b74b58`). It tests the
owner scalar `+ach`, performs camera/bounds gates, calls owner slot `+58h`
at `00b74ac6`, and conditionally calls `00b72f80` at `00b74afd` when owner
`+180h` is nonnull. It subsequently traverses children and dispatches their
slot `+20h` at `00b74b48`.

Thus `00b72f80` is the concrete next dependency for tracing owner geometry
into render items. Its internals and downstream execution are deliberately
outside this bounded pass. It is a candidate route, not a proven query
submission routine. Neither this observed slot nor the three vertex writers
establishes which query measures a depth-tested pass, which measures a
reference pass, whether both execute in this build, or which states surround
them. Keep those questions open and preserve the existing query method and
cached-count reconstructions without adding an invented native scheduler.

## Evidence and limits

Every live batch verified project `bsp`, program
`/battlestationspacific.exe`, image base `00400000`. These live bytes matched
the installed PE:

| Range | Bytes | SHA-256 |
|---|---:|---|
| `00d63420..00d63483`, table plus following string | 100 | `190a4dc03dbb83c6a86e2c31288eb007f7b4162d199a62c2e26b1544e8de8972` |
| `00b8b6b0..00b8b6b6` | 7 | `2f98bb90d9799072a9b90f65a95f350aa7ee8424f38758907a23bd8b17ba0f4d` |
| `00b8b6c0..00b8b6c6` | 7 | `e6337f212583c172411eb2030631c8df1c0785ad1a5bb43d65645add600de74c` |

The geometry and owner traversal observations are assembly-reviewed, not
newly implemented or runtime validated. No Ghidra names/comments, source,
build configuration, or tests were changed. No new submission name is
proposed without the missing dispatch evidence.
