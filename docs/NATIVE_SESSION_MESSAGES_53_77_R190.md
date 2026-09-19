# Native session messages 53 and 77

Addresses: 00759F40, 00759F70, 00759F90, 00759FD0, 0075A010, 00733620, 00733640, 00733670, 007336A0, 00733700, 00733720, 00C85C20, 00768530

## Scope and factory selection

R190 reconstructs eleven complete normal bodies (555 bytes) for factory cases 53 and 77. Names are descriptive hypotheses, not recovered symbols. Explicit C++ interfaces and source-only profile bindings do not establish whole binary ABI compatibility.

The factory reads an unsigned eight-bit type, rewinds eight bits, subtracts one, checks the unsigned result against E8h, then dispatches through the DWORD table at 76A298. Verified table entries resolve the two classes:

| Case | Table slot | Branch | Construction | Profile |
| --- | --- | --- | --- | --- |
| 53 | 76A368 | 768C52 | no-argument allocating creator 733720 | CFE96C |
| 77 | 76A3C8 | 768BB2 | allocate 20h, then call 759F40 at 768BC6 | D02CB8 |

The complete 7,528-byte factory body and both table entries match the live program and PE. The two construction call sites lack stored Ghidra function membership and remain separate raw evidence. The existing factory itself is not a missing function. Its common tail dispatches profile slot +8 to read the complete record, including its type byte.

This resolves the special constructor identified in R189: case 77 starts with type zero before deserialization. Physical allocation order must not be used to infer the switch case.

## Case 77: mutable type, DWORD and WORD

The 20h record contains the established 18h base, a DWORD at 18, a WORD at 1C, and two retained bytes at 1E/1F.

| Routine | Native bytes | ABI and behavior |
| --- | --- | --- |
| 759F40 constructor | 30 | ECX record, no stack arguments, RET/EAX identity. Clear 08/0C and owner 14, write type zero, delivery 1, then D02CB8. Retain padding 11..13 and the entire payload. No current-game access. |
| 759F70 predicate | 30 | ECX record, DWORD query, RET 4. Return true for query 77 without dereferencing the record; otherwise compare the full query with the zero-extended mutable type byte. |
| 759F90 writer | 54 | ECX record, raw 10h cursor, RET 4. Write type 8 bits, full DWORD 18, then the low 12 bits of WORD 1C. Reload each field after the previous call. |
| 759FD0 reader | 55 | ECX record, 18h wrapper/cursor +4, RET 4. Read type, unsigned DWORD 18 and unsigned WORD 1C. Clear the WORD's upper four bits and retain bytes 1E/1F. |
| 75A010 scalar delete | 31 | Stamp root CE4974, free through BF65AC iff flags bit zero is set, and return the captured identity; RET 4. |

The source constructor deliberately has no current-game context argument. Its fixture also leaves that process publication null, confirming this class does not select a session owner during construction.

## Case 53: allocating creator and owned string

The 20h record contains the base followed by an owned length/data header at 18/1C. Its five profile slots are 733700, 733640, 733670, 733620, and 4499C0.

The 123-byte creator at 733720 has no arguments and returns EAX. It allocates exactly 20h through BF681B, checks the result, calls established base construction 75B430 with type 53, then sets delivery 1, profile CFE96C, length zero and data null. A null result takes the native null-return branch. The source uses the existing actual allocation boundary and preserves that branch.

After allocation, native EH state zero protects the raw object while base construction runs. FuncInfo DB4EC8 points to map DB4EC0: state 0 to -1 invokes C85C20, which loads the saved allocation at EBP-10 and frees it through BF65AC. Source `__try/__finally` releases the allocation if construction escapes before completion. Allocation failure and original FH3 exception dispatch are not runtime-proven here.

The 30-byte predicate at 733620 returns true for fixed query 53 before dereferencing the record; otherwise it accepts the current type byte. The 38-byte writer at 733640 writes type 8 and the established owned-string codec 429AC0. The 40-byte reader at 733670 reads type 8 and calls owned-string codec 429F20. These preserve stored low-byte length on write, the actual E17669 fallback, full wire payload consumption, first-NUL storage truncation, actual raw string resizing, and equal-length/null-data reuse.

The 94-byte destructor at 7336A0 stamps CFE96C, returns a nonnull string with size `length + 1`, then stamps root CE4974. It retains the string header, including its dangling pointer after a successful return. FuncInfo DB4E9C/map DB4E94 has one unwind state, whose C85C00 action stamps the root through 4499D0 if returning the string escapes. Source `__finally` follows that cleanup order. The 30-byte scalar at 733700 calls the full destructor, conditionally frees the object, and returns the captured identity.

## Repairs and validation

- Five missing normal functions were defined with prior metadata preserved.
- The scalar's three-byte ADD ESP,4 after free was restored. The allocation unwind funclet regained its POP ECX/RET tail, and its stored body was recreated as C85C20..C85C2A (11 bytes). Both final listings have zero gaps.
- 9,577 live bytes match the PE, including complete new/support bodies, factory code, selector slots, profiles, fallback bytes, and EH metadata.
- All 45 owned direct CALL/tail-JMP rows pass verification. The two factory construction calls are recorded separately because they lack stored membership.
- Strict MSVC Win32 build and all three existing CTests pass.
- 1,813 original/source pairs match 4,116,464 bytes: four type-77 constructors, seven actual type-53 creators, 514 predicate cases, 752 string records, 480 DWORD/WORD records, 48 aliases, and eight freeing scalars.
- Predicate coverage includes every mutable byte value, full-width queries that differ above bit 7, and null-record calls on each fixed-query short circuit.
- String comparisons use the actual private pool and allocation/free providers, lengths 0..511 modulo 256, embedded NULs, fallback/null sources, equal/different/null reuse, both return gates, exact arena/ring state, pointer reuse assertions and distinct live buffers. Heap addresses are normalized as opaque; arena offsets remain exact.
- The actual type-53 creator's initialized fields are compared across valid and invalid signed owner indices. Its three uninitialized heap padding bytes are normalized. Serialization fixtures transfer the initialized record into owned guarded backing and preserve fixture padding; they do not treat allocator residue as evidence.
- The type-77 comparison covers all eight bit offsets, nonzero output backing, DWORD and 12-bit WORD boundaries, retained padding and bounded writer/reader overlap.
- One source-only access-violation case confirms that a failed string return still stamps the root and retains the header while leaving actual pool state unchanged. The scoped observer restores the real private pool publication, then continues exception search; it does not fabricate a provider result. The failed resource is subsequently cleaned up explicitly.

No freed bytes are inspected. Original FH3 exception compatibility, creator failure cleanup at runtime, allocator failure/null returns, arbitrary aliasing, concurrency, whole ABI, full factory composition, startup/network exchange and gameplay remain open. The game installation and peer runtime are untouched.

## Follow-up packet

The factory dispatches cases 55..62 through 8E1530 at 768C5E; that helper's ECX type argument must be preserved. The following 30h allocation at 768C6A constructs type 63 through 763720 at 768C7E with profile D03568. These are candidates only: claim and verify their full contracts before implementation. Full factory binding is still required by the packet recorder and network workers.
