# oct18-tA long-tail reconstruction

Scope: the 82 rows of `codex_tail_A.txt` supplied by cc16-progress on 2026-10-18.
Exact address leases only. No game-host wiring or game launches. New C++ interfaces
under `bsp::oct18_tail_a` are build-tested source reconstructions, not original
register ABI replacements. Object offsets assume MSVC Win32 and valid image-shaped
objects. Descriptive names remain hypotheses.

## Batch 1: game virtual leaves (17 functions)

Evidence is the saved per-address Ghidra decompiler exports; original register
accesses at 006D0D20, 006D0D30 and 006D1F40 were also checked in the assembly.
The existing reviewed BSP names and prior plate comments are retained.

| Address | Ordinary behavior | Original ABI |
|---|---|---|
| 006CA5B0 | return 4 | EAX, RET, no read inputs |
| 006CEBE0 | no-op | RET 4; ECX/stack word ignored |
| 006CEC10 | no-op | RET 0Ch; ECX/three words ignored |
| 006CF3F0 | return 1 | AL, RET; upper EAX unspecified |
| 006CFE30 | no-op | RET 4; ECX/word ignored |
| 006D0D20 | return interior pointer self+15Ch | ECX -> EAX, RET |
| 006D0D30 | load pointer at self+8C8h | ECX -> EAX, RET |
| 006D1640 | no-op | RET, inputs not read |
| 006D1D20 | return category 16 | EAX, RET |
| 006D1D40 | return 1 | AL, RET; upper EAX unspecified |
| 006D1D60 | ignore sync message | RET 4; ECX/word ignored |
| 006D1D70 | return 20 | EAX, RET |
| 006D1DE0 | return 6 | EAX, RET |
| 006D1EE0 | return 0 | EAX, RET |
| 006D1F40 | load byte 00E17BF2 | AL, RET; explicit borrowed global in C++ |
| 006D1F90 | no-op | RET 4; ECX/word ignored |
| 006F7F30 | return `undefined command name` | EAX -> literal 00CFB38C, RET |

No-op parameter meanings remain unknown beyond existing vtable evidence. The
command string has matching contents but a new source address. There is no
binary ABI, original allocation-address, runtime or gameplay claim.

Validation: `scripts/build.ps1` passed MSVC Win32 Release and both existing
CTest checks (reconstructed_math, tool_tests). Per-address naming evidence was
appended through the ledger CLI, applied with the serialized Ghidra annotator
(which verifies project/program, records old annotations and saves), and all 17
affected exports were refreshed. Ignored build/annotation receipts are under
`local/oct18_tA_batch1_build.log` and `local/tail_a_batch1_annotations.log`.

## Held rows

Skipped while cc15-rB owns 00735F30-00758EB0: 00737A50, 00737F30, 007420B0,
00742180, 007422A0, 00748A40, 0074CCA0, 0074E180, 00758140, 00758390.

## Batch 2: ordinary bodies and game thunks (19 functions)

Saved decompilers and assembly establish the three ECX camera pointer loads
0042ABD0 (+388h), 00435390 (+398h), 00519340 (+3ACh); key constructor 00438F80
(ECX key, stack text, EAX self, RET4; store text then byte +4=0); 006E2850
(FLD [ECX+3E0h], ST0, RET); 00644230 (EAX=[00E188D8], RET); and 004C9C90
(RET; ignored ECX message). Borrowed globals are explicit source parameters.

00436690 initializes a fresh 14h checkpoint record (0,-1,-1,empty native string),
ECX->EAX, RET. 00437490 copies three words then initializes/deep-copies its string,
ECX destination/stack source, EAX self, RET4. 0048E1F0 constructs two adjacent
native strings from two headers (RET8), and 0048E500 from another pair (RET4).
Both return self from incoming ECX. These four bodies compose the existing
complete raw-pool string resize/destruction source, retain fresh-header/self-alias
ordering and reread header fields after callbacks. Pair unwind destroys only the
completed first string. C++ scope cleanup models that ownership without claiming
the original FH3/FS exception ABI. String overlap uses memmove semantics, as the
original CRT memcpy supports overlap. No stand-in pool or heap was added.

Disk bytes prove five game tail jumps: 004A6B90 and 004AB030 -> 0048DBD0;
0054ED00 -> 004F71D0; 0060C610 -> 0042A910 -> 00923840; 006EB600 -> 0072D7E0.
All forward ECX and return through the target with no stack arguments. Source
wrappers reuse the existing target implementations. 0054ED00 inherits the target's
typed screen/table API and explicit slot-0 id argument; it does not add raw screen
dispatch. 004A6B90/004AB030 and 006EB600 inherit required target service contracts.

004F1870/004F1890 are ECX entity, RET: parent registration/removal first, then
the world list at CURRENT [entity+30h]+30h. Removal erases only the first payload
match. Their required services name the actual parent/list callees. 00450750 is
ECX dialog owner, one stack output vector, RET4; EDX is scratch. It retains the
two-word checked map iterator, appends node+0Ch keys, then increments, preserving
the order and rereading the sentinel. 00BF6713 is a returning invalid-parameter
binding (verified complete saved body); validation calls preserve the captured
register values and the subsequent reads, without a no-return assumption.
CRT/STL iteration/append remain bindings.
No game hosts were switched to any of these bodies.

Batch 2 validation: `scripts/build.ps1` passed MSVC Win32 Release and both
existing CTests, including after the returning-validation correction. Names and
classifications were appended using the ledger CLI; the locked annotator saved
prior comments and refreshed all 24 affected exports. Receipts are ignored files
`local/oct18_tA_batch2_checked_build.log` and `local/tail_a_batch2_annotations.log`.

## Library/compiler-generated rows (5)

| Address | Classification evidence |
|---|---|
| 00435640 | MSVC scalar deleting destructor: stamp 00CE3784, test flags bit0, free self, return self, RET4. Existing archive writer destructor projection remains unchanged. |
| 0044E4F0 | MSVC STL tree `_Erase` instantiation: nil byte +19h, recurse right, capture left, destroy NativeString key, free node, iterate left. |
| 004512E0 | Same STL tree `_Erase` schedule with nil byte +55h and mapped sequence-entry destructor before key/node release; compiler EH scaffolding. |
| 00550F70 | One JMP 00550EF0; existing target is the compiler-generated vector `_Tidy`: walk 1Ch elements/destructors, free allocation, zero begin/end/capacity. |
| 006DA6B0 | Same STL tree `_Erase` instantiation as 0044E4F0, nil +19h, key cleanup and node free, right recursion/left iteration. |

These get `library_instantiation_out_of_scope` function records, keeping prior
game-facing fragments and existing Ghidra names. They are bindings rather than
new game-body credit. Template identity is inferred from the complete standard
container destruction schedule, native layouts, and existing map/vector users.

