# Ordered VFS candidate resolution

`vfs_candidates.cpp` reconstructs the lower candidate resolver `00bddc80`
with explicit, ordered supplied registrations and separate direct-resolution
and candidate-existence callbacks. It does not invent native registration,
mounts, recursive directory scans, or archive precedence.

## ABI and parsing

`00bddc80`: ECX manager, one mutable native string pair on the stack,
boolean AL, RET 4. `00bddaa0`: ECX manager, output-string pointer, stem
pointer, extension pointer, boolean AL, RET Ch. `00bdc680`: ECX manager,
mutable prefix/output pointer, stem pointer, extension pointer, boolean AL,
RET Ch. Typed functions are new C++ interfaces, not binary replacements.

The top-level routine lowercases through `004bcc00` at `00bddcac` and
converts backslashes to slashes at `00bddcc0..00bddcd2`. It does not trim
spaces; outer `00bdf4c0` separately calls the fuller normalizer. It first
calls direct resolver `00bdd6e0(input,input)` at `00bddcd8`. Success stops
immediately and can supply a rewritten resolved name.

On failure, `004bcb80` searches backwards for the final slash (call
`00bddcf1`). `_strcspn` finds the **first dot starting at that slash**, or
from index zero if no slash. This is not a last-extension algorithm:
`dir/name.part.tga` has full stem `dir/name`, basename stem `name`, and
extension `part.tga`. Missing dot or dot at index zero returns false.
Empty extension after a trailing dot is allowed by this parsing path.

The two stems are made at `00bddd66`/`00bddd9a`, then shortened to the
dot position; the extension is copied at `00bddd88`. Their case-insensitive
inequality is tested by `00449af0` at `00bddedd`, `00bde07d`, and
`00bde21c`. That helper returns `_stricmp(...) != 0`, not string presence.

## Registration views and exact candidate construction

Manager `+54h` is an extension-keyed ordered tree. `00bddaa0` uses its
lower/upper bounds `00bda260`/`00bda2c0` and visits equal-key entries in
native iterator order. The comparison is case-insensitive. Each candidate
prefix is the node's native string at `+14h/+18h`. Preserve the order among
equal keys; an unordered dictionary or sorted prefix list is not equivalent.

Manager `+60h` is an ordered group list. `00bdb1e0` scans groups and their
extension lists, choosing the **first** group containing the original
extension (equal length and case-insensitive contents). Group node `+10h`
holds the ordered extension list; `+1ch` holds ordered prefixes. Candidate
list strings reside at list-node `+8h/+ch`. Only the selected group is used;
a failed group does not cause fallback to the next matching group.

`00bdc680` forms exactly `prefix + stem + "." + extension`. It inserts no
slash and performs no independent normalization of supplied registration
strings. Assembly `00bdc715` obtains the second stack argument for
`stem + "."`; `00bdc72d` obtains the third for the extension; the result
is appended to the copied prefix at `00bdc750..00bdc770`.

It then calls manager virtual `+8h` at `00bdc7f3`, a boolean existence
probe. A true result copies the **constructed candidate spelling**, not a
provider-resolved output. Direct `00bdd6e0` has a separate output contract.
The reconstruction preserves this distinction with two callbacks.

## Pass order

Let F be the full stem, B the basename stem, E the original extension, G
the first matching group, and A the extensions in G that differ from E
case-insensitively. `roots(stem,ext)` probes every matching `+54h` prefix;
`paths(stem,ext)` probes G's `+1ch` prefixes. Each operation stops at its
first successful candidate. No candidate deduplication is performed.

| Order | Candidate pass | Assembly anchor |
|---|---|---|
| 0 | Direct original path resolution | `00bddcd8` |
| 1 | roots(F,E) | `00bdde1c` |
| 2 | If F differs from B and G exists: paths(F,E) | `00bddf77` |
| 3 | If F differs from B: roots(B,E) | `00bde098` |
| 4 | If G exists: paths(B,E); otherwise return false | `00bde180` |
| 5 | If F differs from B: for every A, roots(F,A) | `00bde2bd` |
| 6 | If F differs from B: for each A, empty-prefix F.A, then paths(F,A) | `00bde392`, `00bde460` |
| 7 | For every A, roots(B,A) | `00bde6ba` |
| 8 | For every A, paths(B,A) | `00bde88e` |

The distinction between passes 5 and 6 matters: **all** alternate-extension
tree searches precede the empty-prefix/group searches. Likewise all pass-7
tree searches precede pass 8. There is no general extra empty-prefix B.A
probe in pass 8, and no alternate-extension empty-prefix pass when F equals
B. Supplied empty prefixes can still generate such a path through registered
lists. Repeated extensions/prefixes retain their iteration effects.

## Installed texture and shader consequences

The independent registration audit in `VFS_SEARCH_REGISTRATION.md` recovers
startup `00738360`: textures register `dds` then `tga` in one group, with
prefixes including `fonts/` and `effects/`; shaderfx registers `shfx` with
prefixes including `shaderfx/gui/` and `shaderfx/lights/`.

Therefore an unresolved `Fonts/white.tga` can reach a candidate
`effects/white.dds` through basename/alternate-extension processing after
earlier candidates fail. The original requested spelling alone does not
prove that a same-directory .tga must exist. Likewise unresolved
`guifontbilinear.shfx` can reach `shaderfx/gui/guifontbilinear.shfx` through
the group's original-extension basename path pass. Actual selection still
depends on earlier candidates, supplied registrations, and mounted-provider
existence; these statements do not claim a live game lookup was observed.

## Typed implementation limits and verification

The supplied registration structure exposes native iteration order directly.
The callback layer is concrete dependency injection for the actual provider
lookup, not a success stub. Parent integration supplies its separately
recovered mount traversal. Registrations must remain stable during callbacks;
this function makes no native thread-safety or arbitrary callback-mutation
claim.

The typed domain explicitly rejects non-ASCII strings, embedded NULs, and
lengths over INT32_MAX before mutation, and rejects missing callbacks. This
avoids claiming CRT locale-dependent `_stricmp` parity or native invalid
string memory behavior. Within this domain, failure retains the lowercased,
slash-converted input. Candidate failures do not overwrite it. Constructed
names over INT32_MAX are rejected; allocation/provider exceptions propagate.
No full native allocator, SEH, or container ABI is reconstructed.

Live batches verified project `bsp`, `/battlestationspacific.exe`, x86 base
`00400000`. Complete function ranges matched installed PE bytes:

| Inclusive range | Bytes | SHA-256 |
|---|---:|---|
| `00bddc80..00bde9b3` | 3380 | `efb69974fc83888529ea8ca8125b0b1b51d309897bab04bb939c781b757ed9ba` |
| `00bddaa0..00bddc72` | 467 | `dc0b0a0864abc9528c86712da858192584e5677cdb79589ca4d21d16c51f20f7` |
| `00bdc680..00bdc8a2` | 547 | `77e209521b70bef2148332381c30d635fb2b7a8461277b5a955d9119970da722` |

Proposed names: `BSP_VFS_ResolveOrderedCandidates` (`00bddc80`),
`BSP_VFS_TryExtensionPrefixes` (`00bddaa0`), and
`BSP_VFS_TryConstructedCandidate` (`00bdc680`). Useful comments should
preserve the pass table and distinct direct/output versus existence contracts.
No Ghidra mutation, build, test, shared metadata edit, or commit was performed
by this subtask; parent owns integration and proportionate verification.
