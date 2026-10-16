# Original CRT byte comparison leaf

| Native routine | Span | Coverage |
| --- | --- | --- |
| `00BF7EB7 ___ascii_stricmp` | `[00BF7EB7,00BF7EEC)` 53B/24ops | complete intrinsic raw-byte leaf |

Ordinary Source is `compare_native_crt_ascii_strings_00bf7eb7` in the new
`native_crt_ascii_stricmp` module. It has no direct/indirect calls, globals,
allocation, ownership, locale, TLS or errno dependency. All unsigned byte
values are supported by the actual leaf; the word ASCII describes its folding
rule, not an input restriction. Existing local semantic helpers remain separate.

Native ABI is cdecl with two stacked pointers, EAX signed int and plain RET.
It reads the right pointer at entry ESP+8 before saving ESI, then the left
pointer at current ESP+8 (entry ESP+4). ESI and EDI are preserved. Ordinary
Source takes the two already-bound pointer values; it supplies no implicit
native argument-home/private-frame aliases or binary-entry ABI identity.

Each iteration reads the current left unsigned BYTE, forms its modulo-DWORD
value minus41h, advances the left address modulo32, then adds20h only when
that unsigned range is at most19h. It then performs the same complete sequence
on the right. Thus right is read even if left is NUL. Only A..Z are changed;
all other byte values retain their numeric values.

After both reads/folds/advances, folded left zero exits before the equality
test. Nonzero equality loops; inequality exits. The returned signed difference
is in [-255,255], avoiding unsigned-to-signed overflow or narrowing assumptions.
Source uses ordered volatile unsigned-byte reads and explicit Win32 integer
cursor arithmetic. It calls no SDK ctype or comparison service and introduces
no null guard, normalization, loop cap or source-owned fallback cell.

Only reached valid raw byte storage is required. Mismatch or left NUL permits
normal return; a complete strlen-sized object is not demanded past an early
mismatch. Aliased/identical input spans require no special case. Ordinary C++
storage/lifetime and stable pointer bindings remain required; data races,
fault/SEH identity, arbitrary private-stack aliases and game ABI are unproved.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `00BF7EEC` | `00BF7F72 CALL 00BF7EB7` | left current[EBP+8],right EDI; two POP ECX clean8; result retained |
| `00BF7FBF` | `00BF7FF8 JMP 00BF7EB7` | tail jump after POP ESI/EBP; inherited two cdecl pointers/plain return |
| `00C0115A` | `00C01384 CALL 00BF7EB7` | left current EDI,right D6967C; TEST result then two POP ECX clean8 |
| `00C0115A` | `00C0139A CALL 00BF7EB7` | left current EDI,right D69678; TEST result then two POP ECX clean8 |

All four incoming transfers are typed from actual instructions and containing
bodies. The xref API labels the BF7FF8 transfer as a call, but its actual
instruction is JMP. No unowned caller annotations or Ghidra flow repair are
performed. The third caller's larger semantics remain unclaimed; its two
inspected argument/cleanup sequences establish only these incoming bindings.

This complete callee closes a genuine dependency of Original stricmp dispatch
and locale-aware comparison's C-locale branch. It is not a fastpath-only parent
implementation. Original toupper/locale/TLS/errno/table/Win32 mapping remains
open, including the known Original-versus-current-SDK EOF scheduling gap.
No property-provider or scene-parent general-readiness upgrade follows.

The full53 live bytes match the installed PE. The report pins complete bytes,
Source/header hashes, original ABI, prior library name/comment and typed rows.
Correct library name `___ascii_stricmp` is preserved. Locked annotation saved
the original project and refreshed the export, preserving prior values in
`local/ghidra-annotations-20261016T000422Z.json`. Primary independent full24-op
and complete Source/header review passed at `5ed8c0888`, with hashes asserted
unchanged. A unique CMake registration was appended when the file was unheld
and this packet's queue was first; its queue was removed immediately without
an exclusive whole-file lease. Normal registered Win32 build is pending. No new
tests, native differential execution, parent locale equivalence or game claim
is made.
