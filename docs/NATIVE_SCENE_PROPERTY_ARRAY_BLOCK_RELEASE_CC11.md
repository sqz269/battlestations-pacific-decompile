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

## Primary integration

One distinct complete39B13 raw array-header release at8F03F0: actual ECX eight-byte header at record+20, DWORD ownedpointer/header+0 and DWORD size measured in BYTES/header+4, preserveESI/plainRET, direct canonical currentcdeclfree only whennonzero. Retains original free/ADDESP4/pointerzero/sizezero/pointerzero order, including empty/repeated path. SourceCOFF/unique linked body matches all Original instructions except sole CALL rel32 operand11..14; exact target resolves genuine current singleton_lifetime_free. Independent ONE fresh3TU main family used genuine current-CRT allocations for two raw38h storages/two21byte owned buffers, Source-owned/empty plus qualifiedOriginal-owned/empty, real ECX/ESP/EBX-ESI-EDI-EBP and full liveheader/adjacentguards. Original39 changes ONLY four CALL operand bytes after exact whole Source/allocator90B34/free6B1 linkedCOFF and actualmalloc/free heap-IAT/runtimeGetProcAddress gates. HistoricalOriginalCRT unbound; volatileEAX/ECX/EDX/arithmeticflags unspecified and never compared; no freeddata/root access. Zero BSP archives/oldreplays/newtrackedtests; explicit rawstorage release after allchecks. Source admission does not establish actual record constructor/scalar/whole158B type/nested114h/lifetime/class/EH/factory/world/game. Parent preparation relative-owned-report path failure preceded all directory/build/execution; read-only seal expected scalar compileExit instead of actual three-row list, corrected without rebuild or replay; failed scripts retained.

Exact main Source `d02cf8082` passed MSVC Win32/all three existing CTests. Independent manifest `local\cc11_array_release_current_primary\primary_after.json` records three fresh TUs, 8 production headers, 166 compiler-observed host headers, six searched libraries, four backends and 248 stable worker paths. Rebuilt Core is build context only; zero BSP archives link.
