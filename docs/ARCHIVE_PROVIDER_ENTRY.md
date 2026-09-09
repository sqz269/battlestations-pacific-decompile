# FileStore and MPKG provider entry points

The startup system path `filestore` selects a named in-memory provider. It
does not name a physical directory or archive file. MPKG uses a separate
factory and reader. This audit implements only the bounded memory-backed
FileStore path in `file_store.hpp/.cpp`; MPKG remains an evidence handoff.

## Startup factory distinction

Startup registers physical factory00bed990 inside00beda60, then registers
004fc150's result at0073d675 and00736a90's result at0073d688. Parent owns the
first-nonnull factory selector and mount registration.

004fc150 owns singleton0109db68, constructed by00be5320 with primary table
00d688b4. Its factory+4 is00be8120: ECX factory, stack system-path/virtual-name,
EAX provider, RET8. It rejects empty system paths;00425850 compares the path
to literal `filestore` (00cff1fc). A match calls00be80b0, which returns or
lazily creates factory+8's provider through00be7fa0. Assembly resolves the
decompiler's incorrect void return for00be80b0. Virtual-name is not consumed
by this factory body. Startup's `filestore` -> `.` registration consequently
refers to this provider, not a disk archive.

00736a90 owns singleton010904f4, primary table00cfea14. Factory+4=00bb9d90
accepts a system-path length greater than5 whose last five characters compare
to `.mpkg` through00425850; other inputs return null. It allocates0x18 bytes
and calls00bb9cb0. The latter initializes the base root/name and table00d64390,
allocates a0x34-byte archive state, and calls00bb9920, storing it at provider+14.
The factory ABI is also ECX factory, two stack strings, RET8; provider
constructor is ECX object, one stack path, RET4.

## Implemented FileStore provider

00be7fa0 constructs an empty base root and two empty trees at provider+14 and
+20. Primary table00d689e8 has open+8=00be5fa0, exists+10=00be5c00,
resolve+24=00bf0fb0. Other slots cover enumeration, secondary operations and
destruction; their full behavior is not represented by this fragment.

`add_file_00be7760(name,source)` corresponds to ECX store, two stack arguments,
RET8 at00be789e. It copies and normalizes the name using00bee780 ->00bee690,
then searches the **primary** tree (ESI becomes store+14 at00be7793). If a key
already exists, it logs a duplicate warning and returns without replacing the
stream or acquiring the proposed new reference. First insertion wins.

New entries retain the supplied stream wrapper: explicit increment00be77e7,
temporary record constructors00be6090/00be6250, insertion00be7340 at00be7829,
then balanced temporary destruction. The record has name at+0/+4 and stream
at+8; tree nodes place that record at+C, so open loads stream from node+14 at
00be601f. The typed map stores one shared owner per key. Its insertion result
distinguishes inserted/duplicate/invalid rather than emitting native logging.

Lookup00be5c00 is ECX store, one name argument, AL result, RET4. It searches
primary tree+14 through00be5a50.00be54d0 and00443d00 establish empty-name gates
then CRT `_stricmp` ordering, with **no length tie-break**. Lookup does not
normalize the request. The host preserves this: add may normalize
` A\\B ` to `a/b`, but lookup of the original spaced/backslashed spelling is
not independently normalized by FileStore. Case differences compare equal.

Open00be5fa0 is ECX store, name/flags arguments, EAX stream, RET8. It rejects
flags bit0 and returns null for missing primary-tree entries. On a hit it
calls00bef750 on the stored stream at00be6022. In the supported MemoryStream
domain, that existing helper creates a new cursor-zero wrapper sharing the
same backing. The implementation retains the original wrapper on insertion,
then uses `clone_reset_00bef6d0` on open. Stored cursor is unaffected, returned
cursors are independent, and dropping the store does not invalidate open
clones. Uninitialized-tail metadata is retained; no silent tail fill/rejection
is added to this provider's open. Decoders retain their own safety checks.

Resolve uses the already recovered00bf0fb0: existence followed by copying the
requested logical name on success, output unchanged on failure. It returns
neither physical root nor normalized insertion spelling.

The second tree is untouched by these recovered add/lookup/open entry points.
No recursive collection, batch registration, enumeration, removal or secondary
tree behavior is claimed. Native00be7ab0 is a separate population adapter:
it opens a name through VFS, obtains00bef750 memory view, calls00be7760 and
releases temporary memory/source references. The new class receives that
memory view explicitly instead of recursively invoking the incomplete VFS.

Host guards reject embedded NUL, lengths aboveINT32_MAX and new insertions
without positive MemoryStream backing. Native insertion would dereference a
null source on a new key; duplicate detection precedes that dereference, and
the host preserves duplicate-before-invalid-source ordering. The class has no
singleton, intrusive ABI or allocator compatibility. Allocation exceptions
propagate. Comparator behavior assumes the CRT locale is stable while keys
are stored, as required by the native comparison-based tree too.

## MPKG format and read boundary

00bb9920 opens the path through VFS in mode2, converts to memory, allocates
another backing and transforms the bytes. Assembly/decompiler show blocks
of0x2F1 bytes, a reversed index within the current block (last partial block
uses the remainder), and XOR against table00e144f0 with period0x219 indexed
from the computed source index. Exact signed-size/error/zero-remainder edge
semantics have not been ported; this is not a claim that arbitrary packages
can be decoded by a generic ZIP reader directly.

After transformation the state creates a memory wrapper and calls00bb87a0.
That routine reads the last min(length,FFFFh) bytes and scans backward for
`50 4B 05 06` (ZIP end-of-central-directory signature), storing its offset at
state+1C.00bb9920 then reads fields following that signature and invokes
00bb9700 to populate entries. This establishes a transformed ZIP-like archive
structure, not a complete format implementation or malformed-input policy.
The scanner's raw tail after wrongly marked no-return `_free` was inspected.

Provider table00d64390 open+8=00bb8e70 and exists+10=00bb8e80 are tiny thunks:
replace ECX with provider+14 then jump00bb8d60/00bb8e00. Those routines scan
0x24-byte entries by equal stored lengths then `_stricmp`; open rejects bit0.
Matched open calls00bb8be0. That function obtains an entry data offset through
00bb8a90; nonzero method field+14 routes to stream wrapper00bbc1d0 followed
by memory conversion. Method zero and entry size+1C <=40000h use00bef840;
larger entries reopen the source path and construct00bf1130. Compression
details, decrypted versus reopened source offsets, slice ownership and full
directory parsing remain unresolved. These are the next concrete dependencies
before an archive implementation is coherent. No fabricated ZIP/decompressor
adapter was added.

The startup enumeration agent reports no loose `.mpkg/.zip/.pak` files in the
installed root. That does not prove absence inside other providers or another
installation, and supplies no archive fixture for runtime validation here.

## Byte evidence and validation

Every analysis batch verified `bsp`, `/battlestationspacific.exe`, base00400000.
The following complete routine spans, except explicitly marked windows,
matched installed PE bytes. Ignored exports are under
`exports/bsp/owner_textures/archive/`.

| Start | Bytes | SHA-256 |
|---|---:|---|
| 00be8120 | 43 | `7b04ce7d7b506df5d215d6751236701bcae5f35bd8a3087082052184a7c16baf` |
| 00be80b0 | 105 | `7ccbd70886a44c579ae9b92e65a164e4cf73985736d748bb81934c343678d2f4` |
| 00be7fa0 | 238 | `ca1ebfa3644d33ca4377a8b23aa70f064f10019c33efaef5ae0c2d92e6499ad2` |
| 00be7760 | 321 | `fb66b79def9ea3bc9074aea57a4247c7de99d13de973dbd498a5f83cd727f143` |
| 00be5c00 | 62 | `0c480f2dd3ab43d401e6309939641f64b1c8cf166a16938e54d14cb363699ae0` |
| 00be5fa0, padding window | 153 | `e57076ed3066ca0f14a2a6d6ceb93bcf115c51592a7bd033d93d6e0fcbb723c3` |
| 00be7ab0 | 106 | `7b91fe3afa47a5a50baa1944ac0ce230a16cc5baba7ed3d322efdf6a31384e9b` |
| 00bb9d90 | 211 | `f29b9e0da7b9117c1397ac017446b00b782a94cb9de0a799d32e2d4ad2c0121b` |
| 00bb9cb0 | 113 | `58d187cfc7cf73f4a35f6c6d17b1723816017216ef53e1e7e322ca156cb0d7bf` |
| 00bb9920 | 747 | `ae56080e3ddaa652e2eb8a0453e85bb08fa03043bae163f9957d554915d2502b` |
| 00bb8be0 | 370 | `eaf895523340f048952770a46a850d47573f4742b9f0896d9e84664e083aed7d` |
| 00bb87a0, padding window | 176 | `1742ea587eb40de5d2e65047cd2236a99498a521f34f2e10cc832a3b294d3c8f` |

00be5fa0 returns at00be602e;00bb87a0 returns at00bb8841. Source inspection and
diff checking were performed; parent owns compilation/probe integration. No
new tests, build invocation, Ghidra mutations, shared metadata or installed
asset modifications were made in this bounded task.
