# Mount preparation and priority insertion

The semantic fragment in `vfs_mount_registration.cpp` reconstructs prefix
preparation and ordered insertion from `00be1740`. It accepts an actual
supplied provider's callbacks; provider factory choice and provider destruction
remain separate. It does not reinterpret mount order as archive discovery order.

## ABI and value retention

Assembly establishes ECX manager and four stack arguments: provider pointer,
virtual-prefix native string pointer, signed priority DWORD, ownership byte.
The method returns with RET 10h at `00be1886`. Its decompiler stack variables
are badly confused by the by-value temporary, so the assembly is authoritative.

`00be1764` constructs the canonical prefix using `00bee390` with ECX output
string and EDX source string. `00be177a` applies `00584110` with slash string
`00ce7898`. The 10h-byte temporary built at `00be177f..00be17d1` contains
prefix length/data, raw provider pointer at +8, and the original byte at +Ch.
`00be17d4` obtains priority in EDX for `00bdeec0`, which combines that key
with the payload. `00bdce40` copies the resulting record, and `00be1802`
inserts it into manager `+3Ch` (head at manager `+40h`).

`00bdcfc0` and `00bdce40` copy strings but merely copy the provider pointer
and byte. There is no observed provider AddRef or reference-count increment
in this insertion path. The native record retains the pointer value and flag;
this alone does not specify its eventual deletion policy. The typed wrapper
preserves `native_ownership_flag` as a byte and does not attach an invented
native deleter. Host callbacks may capture a shared_ptr to keep their actual
provider alive; that is explicit host ownership, not native reference parity.

## Prefix canonicalization

`00bee390` is stronger than resource normalizer `00bee690`: it lowercases
via CRT tolower, accepts slash/backslash separators, collapses repeated
separators, removes dot segments, and processes parent segments. Its original
register ABI is ECX destination string, EDX source string, returned destination
in EAX, ordinary RET. The typed implementation directly follows its read/write
cursor and boundary-marker algorithm rather than using filesystem canonical.

An initial separator produces one leading slash. At a segment boundary,
`./` is consumed; a final `.` retreats the output cursor by one byte when
possible (`00bee4b1..00bee4bb`). `..` followed by end/separator either backs
up over the preceding segment or, at the saved boundary, appends `../` and
advances that boundary (`00bee46f..00bee4af`). Unresolved leading parents
are retained, including after a leading slash. This is not a path sandbox
and must not be replaced by a containment check as native behavior.

`00584110` then removes trailing slashes but stops at the first character.
A sole `/` remains `/`; it does not become the empty virtual mount. `.`
does become empty through canonicalization. Spaces are not trimmed. Examples
derived from these branches: `A\\B///` becomes `a/b`, `a/../b` becomes `b`,
`../a` remains `../a`, and `a/.` becomes `a`. These are assembly-derived
examples, not a new test suite or native differential run.

The host projection rejects non-ASCII, embedded NUL, and lengths above
INT32_MAX, preserving output on rejection. ASCII restriction avoids claiming
locale-dependent tolower behavior for high bytes. C++ string storage replaces
the native scratch allocation and its memory-unsafe edge cases; allocator and
SEH compatibility are not claimed. Input/output aliasing is supported by
building a temporary before assignment.

## Priority and ties

The exact comparator is `CMP EDI,[EAX+Ch]; SETG DL` at `00be1350..00be1355`:
**signed new priority greater than existing priority** chooses the left child
at `00be1360`. Otherwise, including equality, it chooses the right child at
`00be1364`. The insertion helper `00be0dd0` attaches the new node left or
right according to that flag (`00be0e7a..00be0e99`). It does not reject an
equal-priority record or overwrite one with an equal virtual prefix.

The native iterator starts at the leftmost node, so traversal is descending
signed priority, with existing equal-priority entries before newly inserted
ones. Tree rotations preserve that in-order sequence. Prefix specificity is
not the comparator; a longer prefix does not implicitly outrank a shorter one.

`register_ordered_mount_00be1740_fragment` therefore inserts immediately
before the first record with a lower signed priority. It rejects an already
unsorted supplied vector rather than silently reordering it, and requires
both actual provider callbacks. These are typed preconditions. The snapshot
helper copies callback values and normalized prefixes into `VfsMountContext`
in that exact order. Native tree node layout/allocator balancing and manager
teardown are not reconstructed by this vector projection.

## Integration and evidence

The new files are `include/bsp/vfs_mount_registration.hpp` and
`src/vfs_mount_registration.cpp`; existing mount traversal files were not
edited. Provider factories, root construction, and ownership-sensitive teardown
are parent/dependent tasks. No test, build, Ghidra mutation, shared metadata
change or commit was performed by this subtask.

Live batches verified project `bsp`, `/battlestationspacific.exe`, image base
`00400000`. Complete body bytes below matched the installed PE:

| Inclusive range | Bytes | SHA-256 |
|---|---:|---|
| `00be1740..00be1888` | 329 | `9b1eef1ceb91e145b356b0a22e0f14091ef6ccf8c95cf1fbc6b7a78613d574e5` |
| `00be1330..00be139a` | 107 | `1bf48a301d14e17f3f10320b5f39638b1d80d29b24608078cf84f1e91124767e` |
| `00bee390..00bee512` | 387 | `f3db474665b49451a2e7ec5b0662cbbac9c9531025ba92c19256c2f6eafc134a` |
| `00584110..00584165` | 86 | `931e1956c83024441bf3bfa9bd8b21411311deab674692b5b004c14242c9602b` |
| `00bdce40..00bdce9b` | 92 | `a5ef499d39c97ffb41adf8b97c96715a54f480569e907537c16dcf53088d5e24` |

Suggested names: `BSP_VFS_RegisterPrioritizedMount` (`00be1740`),
`BSP_VFS_InsertMountDescendingPriority` (`00be1330`), and
`BSP_Path_CanonicalizeMountName` (`00bee390`, descriptive of this use but
not necessarily its only callers). Preserve the priority sign/tie behavior
and lack of AddRef in comments; do not label the payload byte a COM flag.
