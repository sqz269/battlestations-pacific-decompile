# Native session messages 26 through 32

Addresses: 0075DB70, 0075DBF0, 0075DC00, 0075DC20, 0075DCA0, 0075DCB0, 0075DCD0, 0075DD50, 0075DD60, 0075DED0, 0075DF50, 0075DF60, 0075DFA0, 0075DFE0, 0075E140, 0075E1C0, 0075E1D0, 0075E210, 0075E250, 0075E270, 0075E2F0, 0075E300, 0075E340, 0075E380, 0075E3A0, 0075E420, 0075E430, 0075E470, 0075E4B0, 00768530

## Scope and profiles

R186 reconstructs29 complete normal bodies (1,457 native bytes): seven constructors, predicates and scalar methods, plus four writer/reader pairs. Types26–28 reuse the existing type25 codec, as their actual profile slots establish. No duplicate reconstruction of those two codec bodies is counted. The full factory remains a dependency fragment.

| Type | Profile | Writer/reader | Payload after the type byte |
| --- | --- | --- | --- |
| 26 | D03084 | 75DAD0 / 75DB10, existing type25 | signed6 DWORD18, signed6 DWORD1C |
| 27 | D03098 | same existing pair | same |
| 28 | D030AC | same existing pair | same |
| 29 | D030D4 | 75DF60 / 75DFA0 | unsigned6 DWORD18, Boolean bytes1C and1D |
| 30 | D030FC | 75E1D0 / 75E210 | unsigned6 DWORD18, Boolean byte1C |
| 31 | D03110 | 75E300 / 75E340 | unsigned6 DWORD18, unsigned3 value1C |
| 32 | D03124 | 75E430 / 75E470 | unsigned6 DWORD18, Boolean byte1C |

Each profile has five slots: scalar delete, writer, reader, fixed-tag predicate and the existing always-true4499C0. Source profiles have no additional context metadata in this packet. Source interfaces remain explicit C++ interfaces rather than a whole-binary ABI claim. Names describe behavior and are not recovered symbols.

## Allocation and retained bytes

The verified factory allocates20h for each of these seven classes. Constructor sites are768A66,768A86,768AA6,768AC6,768AE6,768B06 and768B26. They call75DB70,75DC20,75DCD0,75DED0,75E140,75E270 and75E3A0 respectively. This establishes each allocation size before interpreting payload layout.

All constructors perform the existing base initialization with their fixed tag: mode3, zero08/0C, base profileD02C68, low typebyte and one E188A8 game capture. Signed selection18EC in0..7 chooses owner18CC; other values produce null. They then write mode1 and the final class profile. Every payload byte18..1F and base padding11..13 is retained. Complete instruction shapes match after normalizing only the fixed tag, final profile and relative branch addresses.

Every predicate compares the full DWORD query with its fixed class tag, independently of a subsequently modified typebyte. Scalar methods stamp rootCE4974, call actual BF65AC free only when flags bit0 is set and return the captured identity. No payload allocation or destruction is added. Their complete instruction shapes match, and all seven listings have zero call gaps.

## Wire details

Writers receive a raw10h cursor; readers receive an18h stream wrapper with its cursor at+4. Stream profile, ownership byte and padding remain unchanged. All paths first transfer the8-bit type.

Types26–28 reuse the actual existing signed6 writer and reader. They retain full DWORD storage at18/1C, with the reader sign-extending each six-bit wire value. Source uses the established type25 storage and codec under distinct class profiles.

Types29/30/32 read and write an unsigned6 value at18. The typed reader clears the entire destination DWORD. Their Boolean writers examine each raw byte for nonzero, so values such as02,80 andFF all emit one. The Boolean reader stores a canonical0/1 byte after decoding. Source stores flags as raw bytes so writing noncanonical values remains valid. Type29 retains padding1E/1F; type30 and type32 retain padding1D..1F. The complete30/32 codec instruction shapes match after relocation normalization.

Type31's writer performs MOVZX from only byte1C and emits its low three bits. It never loads the upper three bytes of the stored DWORD. Its reader decodes three bits into a local byte in the stack argument area, then MOVZX and a full DWORD store to1C. This clears the upper24 bits. Capturing the local byte before that store also matters when input overlaps the destination: decoding directly into byte1C would clear wire data prematurely through the typed reader's initialization.

## Ghidra and validation

Eighteen missing functions were defined under the write lock with prior state recorded. New bodies have exact verified ranges; all seven scalar methods have zero listing gaps. Confirmed names/comments preserve prior values and evidence, and affected exports are refreshed.

- 10,003 live bytes matched the PE: new/support bodies, all profiles and the complete factory range.
- 52 direct CALL/tail-JMP rows passed ownership verification. Seven factory sites lack stored Ghidra function membership and remain separate raw evidence; no factory ownership claim is fabricated.
- Strict MSVC Win32 build and all three existing CTests passed.
- 22,866 original/source cases matched4,835,776 observation bytes:196 constructors,22,176 records,224 raw readers,256 local-byte aliases and14 freeing scalars.
- Coverage includes all8 cursor offsets; zero and nonzero output patterns; signed6 boundary values; unsigned6 truncation; noncanonical Boolean bytes; values with only high DWORD bits set; full owner-selection bounds; retained fields/padding; wrapper state; guard bytes around records; and retained/freeing scalar flag combinations.

The focused type31 alias case places wire input over the record. The unsigned6 read clears18..1B, then the three-bit field reads1C itself before the final DWORD store there. All256 low-byte inputs match the original and leave the expected zero-extended value. This is a bounded alias proof, not a claim covering arbitrary object/cursor aliasing.

Native scalar free is bridged to actual singleton free, with root profile observed before release. Freed heap contents are never inspected. The fixture does not touch peer gameplay or the game installation. No original CRT identity, allocation-failure behavior, concurrency, whole ABI or FH3/SEH compatibility is claimed. The13 compiled-object comparisons include existing dependency translation units pulled by the shared type25 source module; those are not counted as new string or pool reconstructions.

## Follow-up packets

Continue from the factory allocation at768B32, after claiming its constructor/profile dependencies. Full factory composition, the packet recorder, network workers, ordinary startup, network exchange and gameplay validation remain open. This packet does not establish a runnable game rebuild.
