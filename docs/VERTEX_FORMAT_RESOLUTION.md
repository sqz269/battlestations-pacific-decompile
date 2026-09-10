# Encoded vertex-format resolution

Addresses: 00b2dbd0, 00b317e0, 00b2c280, 0041dd40, 00b48330,
00b47900, 00b48af0, 00b305f0.

`decode_vertex_format_00b2dbd0` implements the native declaration loader's 54
aliases and usage/type token grammar. The resulting declaration uses the
existing reconstructed append and stride routines. No `.mvfm` file is opened.
`resolve_mesh_vertex_format_layout_00b2dbd0` supplies its actual stride and flat
element count to the structured mesh reader.

## Native route and grammar

Renderer virtual slot +38h points to `00b317e0`. It copies and lowercases the
incoming counted name, then requests registry +1A60h through `00b305f0` with
flags 0,1,1. Registry vtable `00d5f060` has resolver `00b2c280` at +4 and loader
`00b2dbd0` at +8. The recovered resolver copies a counted string without a
lookup or transformation. Output must be fresh; it is zeroed before the alias
check, so a source/output alias becomes empty. Native cache and reference
ownership are not implemented by the host value decoder.

The loader expands named aliases before parsing. All 54 literal pairings and
their branch evidence are recorded in `reports/vertex_format_resolution_audit.json`.
No replacement is itself an alias, so a single host substitution preserves
the native multi-stage alias checks. Native spelling includes `skined.mvfm`.

Usage tokens are `p,w,i,n,u,t,b,c`, with native values 0,1,2,3,5,6,7,10.
Type tokens, in native search order and enum order 0 through 16, are
`f41,f42,f43,f44,c,ub4,ss2,ss4,ubn4,ssn2,ssn4,usn2,usn4,ud3,sdn3,f22,f24`.
Each element consumes one usage followed by the first matching type. Append
uses offset `FFFFFFFF`, which advances the existing packed-offset cursor.
The remainder must compare case-insensitively equal to `.mvfm`, and the
declaration must contain at least one element.

Assembly at `00b2e84a..00b2e8f5` is essential: the decompiler incorrectly removed
the suffix-copy and comparison branches as unreachable. The assembly allocates
the five-character suffix, copies its literal, compares the remaining substring
through `stricmp`, and checks the flat declaration count. The parser therefore
rejects missing suffixes, trailing characters and an empty declaration.

Original loader ABI: incoming ECX unused; stack counted-name pointer and ignored
DWORD; declaration pointer or null in EAX; RET8. Renderer wrapper ABI: ECX
renderer, stack counted-name pointer, declaration in EAX, RET4. Resolver ABI:
incoming ECX unused, stack output string/input string/ignored DWORD, output
pointer in EAX, RET0Ch. Names describe established behavior but are hypotheses.

## Host domain and evidence

The host API uses owning C++ values and preserves output on failure. It rejects
embedded NUL and lengths that could overflow packed DWORD offsets; these are
explicit restrictions, not claims about the unchecked native behavior. Native
suffix `stricmp` can ignore bytes after NUL. Token and alias comparisons use
ASCII folding because all accepted grammar characters are ASCII.

Sixteen complete code/data spans match both live saved-image bytes and the
installed PE, including the complete 3435-byte loader, 193-byte renderer wrapper,
literal ranges and vtable slots. The supplemental identity-resolver audit adds
its complete 80-byte body, the 164-byte string resize helper and registry vtable.
Independent review checked alias pairings, branch order, token order and suffix
behavior. No new test target was added.

The installed format `pssn4nubn4ussn2.mvfm` resolves to usage/type pairs
(0,10), (3,8), (5,9), with sizes 8,4,4: stride 16 and three elements. The current
probe verifies 12 vertices, 192 vertex bytes and 96 metadata bytes from this
declaration. This is payload validation, not an installed mesh draw or gameplay
validation; see `docs/MESH_RESOURCE_INTEGRATION.md`.
