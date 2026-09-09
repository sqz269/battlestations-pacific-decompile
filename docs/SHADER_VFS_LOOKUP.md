# Shader resource-name resolution boundary

Follow-up: `VFS_SEARCH_REGISTRATION.md`, `VFS_CANDIDATE_RESOLUTION.md` and
`VFS_MOUNT_LOOKUP.md` recover the registration/candidate dependencies identified
below and integrate the installed font probe. This document preserves the
earlier handoff audit; its unresolved search-list boundary is now narrowed to
native mount construction, archive providers and priority.

`00bdf4c0` is a resolving, mutating availability operation, not a pure file
exists predicate. The native basename-to-directory route belongs to its
resource manager dependencies; `00b46950` does not itself prepend
`shaderfx/gui/`. The installed loose file
`shaderfx/gui/guifontbilinear.shfx` was found, but the native registration
which makes that basename select that location remains unresolved.

## Resolve before loading

`00bdf4c0` takes manager ECX and one stack argument pointing to a mutable
native string pair (length, data pointer), returns a boolean in AL and ends
RET 4 at `00bdf586`. Its assembly preserves manager in EDI and argument in
ESI, avoiding the missing-this ambiguity of the decompiler prototype:

1. `00bdf4e3` calls `00bee690`, the existing resource-name normalizer, with
   the supplied string as ECX.
2. It copies the normalized string into a temporary.
3. `00bdf534` calls `00bddc80(manager, supplied-string)` and preserves AL.
4. On success, `00bdf547` calls `00bdeb40(manager, old-copy, current-string)`.
5. It destroys the copy and returns the resolver result.

The immediate `00bdeb40` body contains gated `<SRCH><` logging of its two
arguments, controlled by global `0109cee8` and manager byte `+79h`. It does
not perform a second resolution or copy the temporary over the result.
The supplied string can change even on failure because normalization precedes
the availability decision.

## Lower resolver and search dependency

`00bddc80` takes ECX manager and one mutable string argument, ending RET 4
at `00bde9b1`. Assembly at `00bddcac` calls the ASCII lowercase helper;
`00bddcc0..00bddcd2` replaces backslashes with slashes in place. It first
calls `00bdd6e0` with the same string as input and output at `00bddcd8`.
A true AL immediately takes the success path.

If the direct route fails, it uses slash string `00ce7898`, dot string
`00ce3a70`, and parsed name/extension components. It searches manager
structures rooted at `+54h` and `+60h`, including extension-keyed candidate
lookups and candidate lists, through `00bddaa0` and `00bdc680`. The complete
fallback precedence is not reconstructed here: the body contains several
alternative-extension and candidate-list passes, so flattening it into one
unordered basename map would discard observed ordering.

`00bddaa0` obtains a key range through `00bda260`/`00bda2c0`, copies each
candidate string from node `+14h/+18h`, and calls `00bdc680`. A successful
candidate replaces the output string and returns true. `00bdc680` takes
manager ECX and three stack arguments (RET Ch at `00bdc8a0`); it builds a
candidate from supplied strings, including a literal dot and extension,
then invokes manager virtual `+8h` at `00bdc7f3` with the candidate string.
That boolean decides whether the candidate is copied back. The decompiler
misrepresents its saved manager register as a local pointer; assembly
`00bdc6a6` saves ECX and `00bdc7e5` restores it before virtual dispatch.

The direct route `00bdd6e0` uses `00bdd0a0` with a temporary result object
whose vtable is `00d683f4`, then copies the reported resolved string on
success. `00bdd0a0` in turn performs provider dispatch at `00bdd281`
(virtual `+4h`, output object and name) and `00bdd28a` (virtual `+8h`,
no stack arguments). These are different receiver/ABI contexts from the
manager's virtual `+8h` candidate probe; equal slot numbers do not identify
equal methods.

This is the stopping boundary: identify the concrete providers traversed by
`00bdd0a0` and the initialization of the manager's `+54h/+60h` search
structures before claiming the precise `shfx` search directories, archive
precedence, collision behavior, or recursive lookup policy. These addresses
are concrete next dependencies, not a complete mount-system implementation.

## Shader loader handoff

`00b46950` takes effect ECX and one filename-string pointer, returns AL and
ends RET 4. With global `0108d6f0` clear, it passes filename/0/0 to
`00b45ee0`. With the global set, it performs filename/0/3, calls `00b41b10`
and `00b187a0`, then filename/1/3 and returns AL=1 regardless of the two
load results. This agrees with `SHADER_BUILD_ROUTE.md`.

`00b45ee0` passes the supplied main filename to descriptor reader
`00b43b00`. It resolves copied combiner filenames separately through
`00bdf4c0` before reading them. Thus basename combiners are not necessarily
relative to the main descriptor's directory. `00b43b00` forwards its source
path to script execution `00b69d40(path, 0)` at `00b43b4e`; no fixed GUI
directory prefix appears in that observed handoff.

A host fixture may explicitly map `guifontbilinear.shfx` to the verified
installed GUI file, but should label that mapping as its supplied resolver.
File discovery proves the artifact exists, not that native search selected
that artifact from the original mount/search state. No replacement policy
or material-cache behavior was altered by this audit.

## Byte evidence

Live batches verified `bsp`, `/battlestationspacific.exe`, image base
`00400000`. These ranges matched installed PE bytes exactly:

| Inclusive range | Bytes | SHA-256 |
|---|---:|---|
| `00bdf4c0..00bdf588`, complete wrapper | 201 | `1af8f347987129b4c22a38cb7d495bf0fda69e57fce9e682e59e64638554cb16` |
| `00b46950..00b4699d`, complete load selector | 78 | `d75cce99921eb31a204e48565800d1574647d8c3a372fb325509069549f15f57` |
| `00bdc7e5..00bdc7f4`, candidate dispatch | 16 | `9b7172a7bf50a9351a09be0f0bb8760239c64dc9bcca0cd0b47b1e90865d33b1` |

The larger resolver/provider observations are read-only assembly/pseudocode
audit evidence, not reconstructed or fixture-tested VFS behavior. No source,
Ghidra annotation, shared metadata, build, test, or commit was changed.
