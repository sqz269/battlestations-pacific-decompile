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
`local/goal_oct15_crt_ascii_strnicmp_after_comments.json`. Root registered the Source; the actual normal MSVC Release Win32 gate passed at
`12523a9f6fdc6a7645f02234ae81b44e8bee173b`: all three existing CTests in
13.42 seconds. Immutable root receipt SHA256:
`c482ed444869ee019e1308fa09c5975c5073c5a0c407ae4b9ea31995fd322ab2`.
Actual build-log SHA256:
`f624ac2ae47e024262893892840ab393eaca945cd3d78d40304c5c221035ffd9`.
Source/header pins were asserted unchanged in both worktrees. One project
Source occurrence and one identical archive object are verified in the receipt.
No worker CMake edit,duplicate build,probe or new test was used.

The entire actual normal object and symbols were reviewed independently by
worker and primary. Its public function is172 bytes/78 instructions;helpers compare_bytes,
read_byte and read_word are23,10 and19 bytes,for224 total text bytes. The
1368-byte object SHA256 is
`3615f3481a3055797df6b4e00cc7857965015983998d51c77ec1c7121c4af073`.
All text bytes were decoded;there are no emitted calls,relocations or undefined
external symbols. Supporting full review with artifact/function hashes is
`local/goal_oct15_crt_ascii_strnicmp_emitted_review.json`.

Count DWORD load+18h precedes zero branch+25h and left/right pointer DWORD
loads+2Ah/+3Bh. Current left/right BYTEs at+43h/+45h are both read before
left-NUL testing;the repeated+8Ah/+8Ch reads preserve the same rule. Either
NUL reaches raw comparison before increments/folds/decrement. Unsigned A-Z
checks and CMOVA fold left before right. Only folded equality decrements the
retained private counter at+84h. SBB/AND FFFFFFFEh/INC gives exact-1/+1,while
the zero tail returns0. All three helper bodies were reviewed in full.

Compiler differences include a new172-byte public frame instead of97 Original
bytes,CMOVA folds,a normalized result idiom,and separate helper COMDATs. It
reuses new C++`[EBP+8]` as raw-word-address then captured-count backing,with
F8/FC/F4 temporaries. Those compiler-private cells are disjoint from admitted
actual cells/payload;they add no Original argument-home alias or frame proof.
The existing checks do not execute this leaf,and no Original entry/new body
runtime execution or differential probe was performed. Whole actual-storage
Source is ready;Original binary/private ABI,register/flags/fault/game and all
parent completion claims remain separate.

Final gate evidence was appended under the normal Ghidra write lock. Prior
values are logged in`local/ghidra-annotations-20261016T043149Z.json`;the
project was saved,export refreshed,and final readback in
`local/goal_oct15_crt_ascii_strnicmp_final_comments.json` verifies the
correct library name,every old comment,and final validation evidence.
