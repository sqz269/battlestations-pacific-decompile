# Original CRT ASCII prefix comparison

Address `00C15700`; retain the correct library name `___ascii_strnicmp`.
The complete contiguous body is `[00C15700,00C15761)`: 97 bytes, 48 listed
instructions, 16 basic blocks, zero outgoing transfers, no globals, locale/TLS service,
allocator, exception action, or other unresolved provider. Every native byte
matches live Ghidra and the installed PE. No listing or flow repair is needed.

| Entry | Coverage | Original interface |
|---|---|---|
| 00C15700 | Whole Original byte algorithm | cdecl(left,right,unsigned count), plain RET; EAX exactly -1,0,+1 |

The new ordinary Source interface is
`compare_native_crt_ascii_prefix_00c15700(actual_parameters12)`. It binds the
three actual DWORD argument cells at offsets +0/+4/+8. Native entry P has the
left/right/count words at P+4/P+8/P+0Ch; after its prologue they are at
EBP+8/EBP+0Ch/EBP+10h. All three words have real semantics. Incoming ECX/EDX
are not inputs. EBX/ESI/EDI/EBP are preserved by the Original, while its caller
removes the argument words. The new explicit-cell C++ API is a separate ABI.

The complete access and result schedule is:

1. Read and retain the count DWORD first. Zero returns exactly zero without
   reading either pointer argument cell or any payload. There is no signed
   count restriction: every nonzero DWORD count enters the comparison.
2. Read the left pointer DWORD, then the right pointer DWORD, once each.
3. Each iteration reads the current left BYTE, then the current right BYTE.
   The right read at `00C15720` executes even if the left byte is NUL: MOV
   preserves the preceding OR AH,AH flags until the branch at `00C15722`.
4. If either captured byte is zero, compare those raw unsigned bytes and
   return exactly -1, 0, or +1. This path skips pointer increments, folding,
   and count decrement; it adds no early right-pointer shortcut.
5. Otherwise increment both retained pointers with DWORD wrap. Fold the left
   byte first, then the right: only unsigned bytes 41h through 5Ah gain 20h.
   Other bytes, including all high-bit values, are unchanged.
6. Compare the folded bytes as unsigned. A mismatch returns -1 or +1, rather
   than their arithmetic difference. Only equality decrements retained count;
   zero returns zero, otherwise repeat at the current retained pointers.

All payload accesses are BYTE reads. There are no alignment, null-pointer,
count-range, locale, SDK, whole-string, or extra validation guards. Count and
pointer cells are never reread, and each reached byte is captured once. The
leaf writes no payload or actual argument cell. The Source uses a tiny MSVC
x86 assembly DWORD load for raw argument words, admitting unaligned cells
without a typed DWORD alignment/alias requirement. Volatile unsigned-char
loads preserve each current payload read and its BYTE width.

Only reached storage must be valid. In particular, a zero count needs only its
count word, and a nonzero comparison needs both reached bytes even when the
left byte is zero. The binding address stays stable. New C++ binding metadata
and private-frame backing are disjoint from reached actual cells/payload;
payload may alias actual argument storage when those reached accesses are
valid. There is no owner, implicit string validity, lifetime, private native
argument-home alias, concurrent mutation, or invalid-memory timing claim.

The two true incoming transfers are distinct:

- `00BFAE15 CALL 00C15700`: push current count at EBP+10h, EDI word,
  current left at EBP+8; caller ADD ESP,0Ch.
- `00BFAEAD JMP 00C15700`: restore ESI/EBP, then tail-transfer with the
  incoming three argument words.

Typed metadata confirms `00BFAEAD` has an existing CALL_RETURN override:
default flow is UNCONDITIONAL_JUMP and effective flow is CALL_TERMINATOR.
Its physical instruction remains JMP. Neither that override nor either parent
was edited. The optional script-backed flow-properties query returned the
actual scripts-disabled error; the typed-flow endpoint independently supplied
all needed body and flow properties without any bypass. These are bounded
caller claims, not a claim that both complete parent implementations are ready.

This genuine Original ASCII leaf does not select the ASCII path for a parent,
implement `00BFAE6B` or `00BFAD7B`, or replace their global gate, validation,
locale, TLS, errno, invalid-handler, or cleanup behavior. The general Original
qualified-node parent `009251F0` remains unclosed.

Primary complete native behavior and entire CPP/header review passed. Its
one requested raw DWORD admission adjustment uses the accepted tiny x86 load;
the algorithm, BYTE payload reads, ABI interface and header are unchanged.
Normal locked annotation preserved the correct library identity and every
prior comment. Prior values are logged in
`local/ghidra-annotations-20261016T042613Z.json`;the project was saved,export
forced fresh,and name/old-library-comment/new-evidence readback verified in
`local/goal_oct15_crt_ascii_strnicmp_after_comments.json`. Root registration,
the actual normal Win32 gate,
and emitted-object inspection remain pending. No worker CMake queue, build,
probe, test framework, or new test is introduced. Static/build evidence does
not establish Original binary/private ABI, register/flags/fault or game proof.
