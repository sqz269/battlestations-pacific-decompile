# Native session message 35 and word-array storage

Addresses: 005294F0, 005296A0, 00529980, 0052ABE0, 0052AD30, 0052AEF0, 0052AF40, 0052AF50, 0052B010, 00768530

## Scope and layout

R188 reconstructs nine complete normal bodies (1,085 native bytes): the type35 constructor, predicate, writer, reader, destructor and scalar delete, plus three custom word-array helpers. These helpers are not the STL three-pointer vector layout. Names are descriptive hypotheses, not recovered symbols. Source interfaces and borrowed profile metadata do not establish whole binary ABI.

The verified factory allocates54h at768B72 and calls52AEF0 at768B86. Its full7528-byte range matches the live program and PE. That constructor call lacks stored Ghidra membership and remains separate raw evidence.

| Offset | Storage |
| --- | --- |
| 00..17 | established base record |
| 18,1C | DWORD values |
| 20 | full DWORD subtype |
| 24,25 | raw Boolean flag bytes; padding26/27 |
| 28/2C,30/34 | two owned length/data headers |
| 38,3C | DWORD values |
| 40/44/48 | word pointer, signed size, signed capacity |
| 4C/50 | trailing owned length/data header |

The constructor calls actual75B430 with35, then sets mode1/profileCED098. It zeroes the two initial string headers, the array header, then the trailing string header. Flag25 is cleared before flag24, then subtype20 becomes3. Values18/1C/38/3C and padding11..13/26..27 are retained. The predicate compares the entire query DWORD with fixed35 regardless of the mutable wire typebyte.

## Custom word array

5296A0 receives ECX=12-byte header and a signed target/RET4. It clamps the request to at least1 and returns when signed capacity is already sufficient. Otherwise it allocates the32-bit-wrapping product target*2 through the existing BF55BE-to-BF681B allocation boundary. It copies words while a signed index is below the reloaded size, reloading source data per element and skipping only a computed null destination. It frees old storage through BF6989 **before** publishing the new pointer and capacity. Size is retained. There is no local allocation guard if copying/freeing escapes.

529980 takes the same header ABI and a signed size/RET4. It reserves only when the target exceeds signed capacity. Starting from the current signed size, it zeroes newly exposed words through reloaded data+index*2, skipping a null computed address. Shrinking repeatedly decrements the actual size field, then stores the target. It does not clamp negative sizes or shrink capacity. Source preserves native32-bit address arithmetic and requires backing for the original unchecked accesses.

52AD30 takes ECX=header and no stack arguments. It resizes to0 and frees data. The header retains its dangling pointer and capacity. Actual singleton allocation/free providers are reused; library allocator code is not reimplemented in this packet.

## Wire branches

The writer always sends type8, subtype20low4, value18low8 only when the **full reloaded** subtype equals1, value1Clow3, and Boolean-normalized flag24. It then captures the full subtype again. The reader decodes the unsigned4 subtype, conditionally reads value18, reads value1C and flag24, then captures the decoded subtype.

| Subtype and flag24 | Remaining wire payload |
| --- | --- |
| 0, false | full32 array count, word12 elements, owned string4C |
| 1, false | owned strings28 and30, Boolean flag25; if true, unsigned3 values38 and3C |
| other, or flag24 true | no further payload |

All owned strings use established429AC0/429F20 behavior: stored low-byte length on write, data reload after prefix, actual E17669 fallback, full byte-count consumption on read, first-NUL storage truncation, actual raw resize/copy and equal-length/null-data reuse.

The array writer emits the full32 bits of stored signed size, then loops while a signed index is below the **reloaded** size after each element. It reloads the word pointer per iteration. A negative stored size therefore emits its raw32-bit count but no elements before the trailing string.

The reader captures an **unsigned** wire count in a local, resizes the array to0, and iterates exactly that captured count. Each iteration reads a local unsigned12-bit word. When signed size equals capacity, it doubles capacity with32-bit wrap and selects at least1 by signed comparison, then calls reserve. It reloads data/size, stores the word unless the computed address is null, and increments the actual size. The trailing string follows the entire loop. No clamp or symmetric treatment of negative writer counts is invented.

For subtype1, the two final3-bit fields are each decoded into a local DWORD before publication at38 and3C. Raw nonzero flags are normalized only on the wire. Whole-field/padding retention is preserved outside the selected branch.

## Destruction and unwind

52AF50 stampsCED098, destroys string4C, resizes/frees array40, destroys string30, destroys string28 and stamps rootCE4974. String headers remain unchanged; array size becomes0 while data/capacity remain retained. The scalar method always invokes this destructor, frees the record only for flags bit0, and returns captured identity after successful destruction.

FuncInfoD950F0 references mapD950D0:

| State transition | Funclet | Remaining cleanup |
| --- | --- | --- |
| 3 to2 | C6B89E | array40 via52AD30 |
| 2 to1 | C6B893 | string30 via41DD20 |
| 1 to0 | C6B888 | string28 via41DD20 |
| 0 to-1 | C6B880 | root stamp4499D0 |

Source nested __try/__finally follows these stages. Raw handlerC6B8A9, map and funclets are relocated for normal native comparisons with the actual CRT FH3 export. Original FH3 exception dispatch is not compared.

## Repairs and validation

Two missing functions were defined. Reserve's nine-byte post-free publication gap and the scalar's ADD ESP,4 gap were repaired. Array destruction was restored through52AD46, and the message destructor regained its94-byte tail through52B00F. Both truncated functions were recreated with full ranges and prior metadata preserved. All four repaired bodies have zero call gaps. Writer5294F0's skipped three-byte LEA ECX,[ECX] alignment at5295BD is not a returning-call gap.

- 10,139 live bytes matched the PE, including complete new/support bodies, profile, EH metadata, fallback and factory bytes.
- 77 direct CALL/tail-JMP rows passed ownership verification; the one factory call remains separate raw evidence.
- Strict MSVC Win32 build and all three existing CTests passed.
- 4,282 native/source pairs matched45,130,824 observation bytes:28 constructors,49 array sequences,3,712 records,320 embedded-NUL cases,112 null-source cases,56 asymmetric writers, one count alias and4 freeing scalars.
- Coverage includes all8 bit offsets and decoded subtypes0..15; full writer subtypes16/17/255/256/FFFFFFFF; negative writer counts; array counts through257 with growth to512; exact reserve, shrink/regrow zeroing, retained capacity, signed negative resize on owned prefix backing; string lengths0..511/mod256; raw Boolean values; reuse/null combinations; actual pool arena/ring state and record guards.

The bounded count-alias fixture uses an owned mapped word buffer. Its output overlaps the size field, making that size negative after three words. Both native and source writers stop after those three reads. The large logical count encoded by the mapping address is never allocated or traversed. This establishes the live signed-count reload without claiming arbitrary alias compatibility.

The first two probes differed only in heap-pointer identifiers after changed-length string resize. The captured mismatch contains two occurrences of the same opaque pointer field; payload, cursor and pool bytes match. The corrected fixture normalizes heap addresses as opaque nonnull heap pointers, preserves exact pool-arena offsets, and directly checks equal-length pointer identity and distinct live owned string buffers. Production source did not change for this correction. Failed artifacts and the analysis remain in the local evidence archive.

Four additional source-only fault cases exercise the three string cleanup positions and an array-resize escape. A scoped vectored exception observer restores the real private pool publication or owned array descriptor after the deliberately triggered access violation, then continues exception search. Remaining cleanup uses actual providers. Root/header states and resulting pool state match explicit reverse-order cleanup of the remaining resources; the failed resource is retained for manual fixture cleanup. No allocator/getter/return result is fabricated.

No freed heap contents are inspected. Allocation failure, lazy pool recreation, huge unsigned reader counts, overflowing allocation requests, concurrency, original CRT identity, original FH3 exception compatibility and whole ABI remain open. The game installation and peer gameplay are untouched.

## Follow-up packet

The next factory allocation is28h at768B92, constructing type36 through764670 at768BA6 with profileD03608. Claim its methods and any dependencies before implementation. Full factory composition, packet recorder, network workers, ordinary startup, network exchange and gameplay validation remain open. This packet does not establish a runnable game rebuild.
