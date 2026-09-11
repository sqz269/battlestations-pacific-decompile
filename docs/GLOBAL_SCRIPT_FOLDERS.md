# Global script folders

Addresses:00886370,00886900. Descriptive names are hypotheses, not recovered symbols.

The complete normal flow of00886900 runs `Scripts/global/` with enumeration
flags1, then `Scripts/datatables/autoload/` with flags0. Both calls retain the
same mission Lua host captured in ECX. Each00886370 call enumerates `.luab`
before `.lua`, reloading the current0109CEEC resource service for each pass.
Provider order and duplicate entries are retained.

00886370 appends a deep copy of each compiled filename to its remembered vector
before invoking00885110. A failed open, malformed chunk or runtime error still
shadows the source file. For each source filename the routine copies the string,
uses00469840 with start0 and unsigned length-minus-three, then appends `luab`.
It compares counted lengths first and uses CRT case-insensitive comparison only
when those lengths match. A match suppresses execution; otherwise00885110 receives
the original source path. This is enumeration-based precedence, with no separate
compiled-file existence probe.

The implementation reuses the existing00885110 mission file/chunk sequence and
the00886280 enumeration interface implemented by `VfsLocaleRuntime`. It preserves
pooled string copies, copy-before-pop, remembered-name publication before script
execution, capture of candidate data/length before suffix release, and normal
cleanup order: source list, compiled list, reverse remembered names. Both native
list objects persist until the final cleanup even after their entries are popped.

## Evidence and ABI

00886900: ECX=mission host; no stack arguments; final RET0088691E.
00886370: ECX=mission host; stack directory pointer and flags; final RET8 at008867EB
(three bytes, end-exclusive008867EE). The original pseudocode stopped in cleanup
after a false no-return annotation on CRT free. Raw instructions and live byte
comparison establish the rest of the normal path. Body SHA256:
`9a5a37adb0daa992aa704b44a1ba9c91830e3fd2bc59efba0371f90c43e137e9`.

`reports/global_script_folders_flow.json` records the locked repair of the false
CALL_RETURN at0088678F and90 decoded tail bytes through008867ED. The project was
saved and the export refreshed. Ghidra's stored function-body metadata still ends
at00886793; the bridge repair did not extend that metadata. Complete C++ normal
flow is supported by the verified listing, not by a claim that this Ghidra body
limitation was fixed.

## Validation and boundaries

Build and focused fixture results are recorded in `reports/global_script_folders.json`.
The fixture binds the existing chunk sequence to real Lua5.1.1, including generated
bytecode, and checks failed-compiled shadowing, duplicate order, source execution,
live enumeration-provider replacement, folder flags, stack restoration and pooled
allocation balance. It supplies enumeration results explicitly; it does not test
mounted game archives or gameplay.

Standard deque/vector nodes represent native STL allocation and iterator layouts.
Enumeration returns standard strings before their pooled payloads are materialized;
provider allocation/exception timing is outside this projection. Native valid
32-bit counted-string inputs are required. C++ exception cleanup does not reproduce
native SEH. Existing00885110 stream/chunk adapter boundaries remain, including its
negative-size guard. A real mission-Lua service binding is still required; none is
replaced with a production stub. These are new C++ interfaces, not native ABI or
game-validation claims.

## Follow-up packets

- Bind the recovered mission Lua host services to the actual application state and
  existing mounted VFS, preserving the distinct mission owner at game+1A08 and
  embedded configuration Lua owner at game+1A0C.
- Extend the stored00886370 Ghidra body only through a supported, locked mutation;
  preserve the recorded evidence that the current flow repair alone did not do so.
