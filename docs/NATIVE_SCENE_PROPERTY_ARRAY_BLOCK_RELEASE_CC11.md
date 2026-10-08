# Raw scene property array-block release

`008F03F0..008F0417` is a complete 39-byte, 13-instruction leaf. Its native ECX
receiver is the actual eight-byte header at record+20h: owned data pointer at
+0 and **byte size** at +4. It preserves ESI and uses plain RET. The Source
interface retains that Win32 register ABI and the whole instruction schedule.

For non-null data it calls the canonical current `singleton_lifetime_free`,
restores the cdecl argument stack, and zeros the pointer. Both paths then zero
the byte size and pointer. There is no allocation, callback, class dispatcher,
element destructor, header guard, or extra field reset. The sole COFF relocation
is the actual direct free-call operand at byte 11 decimal (0Bh); the CALL
instruction starts at byte 10 decimal (0Ah). Every other byte matches Original.

The real byte-array producer's bounded `008EF306..008EF332` witness loads a byte
size, writes record+24h, allocates that same size through BF55BE→BF681B on its
owned-copy path, and stores returned EAX at record+20h before memcpy. BF6989
forwards to BF65AC. Current genuine Source allocation/free use the same current
CRT malloc/free heap. The whole producer, record constructor and historical
allocator remain unbound.

One fresh ignored family at `local/cc11_scene_property_array_block_release/run01`
compiled exactly three TUs: this leaf, current `singleton_lifetime.cpp`, and a
new probe. It linked zero BSP archives. The first compilation, link, admission
gate and execution passed. Actual consumed evidence records 8 project headers,
2 production CPPs, 166 host headers, 6 searched system libraries and 4 selected
compiler/backend files. Complete COFF and linked proof covers 354 code sections,
25,542 bytes and 983 relocations across the three objects, with 122 distinct
linked application spans; these membership counts do not claim execution.

The family used two separately current-heap allocated 38h raw storage blocks,
each with an actual header at +20h and a separate owned 21-byte allocation.
Source and qualified Original each executed owned and repeated-empty paths.
Checks inspected live headers and all adjacent bytes, preserved the other live
root, and verified ESP plus EBX/ESI/EDI/EBP. EAX/ECX/EDX and arithmetic flags are
unspecified and were not compared. DF was clear for the ABI calls. Freed data
was never inspected; raw storage was explicitly released only after all checks.

Before Original execution, complete linked allocator90/free6 bodies and their
actual malloc/free heap IAT slots passed COFF/link checks. The probe also checked
resolved IAT pointers against the named loaded heap DLL exports and passed the
Source ABI path. Only then it copied whole Original39, patched only its CALL
rel32 operand to that same canonical current free, and executed the RX copy.
Both original and patched hashes, the patch address/target, and unchanged RX
bytes after execution are sealed. This is qualified Original leaf execution
against the current heap, not execution of historical Original CRT free.

The manifest preserves 5,963 historical files, 341 earlier consumed pins and
all prior constructor/map/negative-audit receipts. Frozen Core071cf8b4/Lua/zlib
copies preserve the main build at Source8a56f5aee (all three existing CTests
passed); they were not linked. Mutable later main libraries are external context.

Whole type release158, record scalar/thunk, nested114h bag lifetime, whole record
construction, native class, historical CRT/EH, factory, world and game remain
unclosed. No old fixture was replayed, no tracked test was added, and the worker
made no Ghidra/shared metadata/CMake change or full repository build.
