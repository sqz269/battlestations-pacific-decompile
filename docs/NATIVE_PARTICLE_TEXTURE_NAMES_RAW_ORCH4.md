# Raw particle texture names and atlas lookup

Addresses: 0043BBF0, 004CAD40, 00AEFB20, 00AF37D0, 00AF38B0, 00AF3960, 00AF3A20, 00AF3B50

## Scope and native coverage

This packet adds raw-pool overloads for eight complete native bodies. Existing
`NativeStringStorage` overloads and their callers remain unchanged.

| Address | Bytes | Original ABI | Coverage |
|---|---:|---|---|
| 0043BBF0..0043BC23 | 52 | ECX actual8h header; stack length/fill; RET8 | complete |
| 004CAD40..004CAF0E | 463 | ECX actual8h header; stack search/replacement/count; RET0C | complete |
| 00AEFB20..00AEFF33 | 1044 | ECX actual manager; stack filename; RET4/EAX borrowed item | complete |
| 00AF37D0..00AF38AD | 222 | ECX output8h, EDX filename; RET/EAX output | complete |
| 00AF38B0..00AF395B | 172 | ECX output8h, EDX filename; RET/EAX output | complete |
| 00AF3960..00AF3A1A | 187 | ESI output8h, EAX stem; RET/EAX output | complete |
| 00AF3A20..00AF3B4F | 304 | ECX filename; RET/EAX signed count | complete |
| 00AF3B50..00AF3D73 | 548 | ECX output8h, EDX filename, stack index; RET4/EAX output | complete |

All installed-PE bytes match the live Ghidra program. The raw overloads use
`NativeStringRawPoolContext`, which resolves the actual 01090AA8 pool publication,
01090AA0 manager publication and 01090AA4 return gate through the existing genuine
00419CC0/BD1120/BD1510 implementations. No storage callback, replacement atlas or
converted item object is introduced.

The `NativeParticleTextureNamesRawContext` field named
`actual_atlas_manager_00f8c26c` is a borrowed volatile reference to the real
publication cell. AF3A20 reloads that cell at 00AF3AAC for every numbered probe.
AEFB20 itself receives one manager in ECX; within that call it reloads manager +4
(the item-pointer array), manager +8 (the signed count), and the selected row.
An item is a borrowed actual 30h record whose name is the eight-byte header at
+0C/+10. The empty-stem and null-pattern fields are borrowed direct byte addresses,
not manager snapshots.

## String and atlas behavior

43BBF0 captures the old length, calls the genuine raw resize, reloads current
length and data, and fills only a newly grown tail. 4CAD40 performs case-sensitive
`strstr`, creates suffix/prefix/replacement joins, assigns the current joined
header, resumes after the current replacement length, then decrements the unsigned
limit. Search and replacement headers may alias the mutable value; no field is
cached across a pool operation where the native reloads it. BF7680 copies use
`memmove`, matching the existing recovered overlap contract.

AF37D0 removes the last dot and extension through a resize; AF38B0 returns the
last-dot suffix, or the whole filename when no positive-index dot exists. AF3960
walks backward and cuts at the third zero encountered, matching the native loop.
The fixed 256-byte and 32-byte CRT buffers and unbounded `strcpy`/`sprintf` behavior
remain native validity boundaries.

AEFB20 rejects null and empty filenames before constructing a query. It removes
the extension, changes backslashes to slashes, strips every leading slash,
lowercases ASCII, and scans in row order. A match is either the existing whole
name comparison or the existing slash-delimited suffix comparison. After a miss,
it retries from the basename following the last slash. It returns the reloaded
borrowed row pointer and never owns an item. AF3A20 recognizes the `000` suffix,
then probes contiguous names from zero against a freshly reloaded manager. AF3B50
constructs prefix + zero-padded signed index + original extension.

## Ownership, aliases and exceptions

Temporary headers use the native construction states and reverse cleanup order.
A state is armed only after its constructor returns; normal cleanup clears the
state before asking the current pool to return the block. Stem output cleanup is
armed before 41E870, because AF37F5 establishes that state first. Frame-name output
cleanup is armed only after the final copy returns at AF3C82/AF3C88. Thus a later
temporary-return failure consumes the output, while a failed output construction
does not claim ownership that the native had not established.

Cleanup calls may resolve the pool again and may throw. A second failure during
unwind terminates under the established raw-string C++ policy. This is source-level
composition, not original FH3 transport identity. The native output/source identity
branches are retained: 4CAD40 assignment skips identity, while AF3B50 clears its
output before its copy-constructor identity test.

## Listing and validation boundaries

AEFB20 has two three-byte Ghidra listing gaps at 00AEFCDD and 00AEFDED. Live bytes
at both sites are `8D 49 00` (`lea ecx,[ecx]`) after unconditional jumps. They are
alignment instructions, not missing control flow; this read-only packet does not
mutate Ghidra.

The strict Win32 build passed and both existing CTests passed. The ignored focused
probe links the resulting `bsp_core.lib`, constructs the real approximately 9 MB
string pool, and uses actual manager/item/header layouts. It passed normalized
whole-name hit, suffix hit, miss, three contiguous frames, frame-name construction,
extension/prefix, resize-fill, and both search/value and replacement/value aliases.
The probe covers normal ownership and cleanup flow; allocation failure, malformed
pointers, concurrent mutation, original FH3 transport, binary substitution and
gameplay remain unvalidated. Descriptive names remain hypotheses.

Full hashes, prior ledger records, old Ghidra documentation, call-site evidence and
validation receipts are in `reports/native_particle_texture_names_raw_orch4.json`.
