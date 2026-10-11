# Application retirement descriptor suffix

Two exact24-byte reads of `00DB57C4..00DB57DB` agree. Independent manual
and standard-library little-endian decoding gives six unsigned DWORDs
**0, 0, 0, 0, 0, 1**; the signed values are identical.

```text
00DB57C4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00DB57D4  00 00 00 00 01 00 00 00
```

Suffix SHA-256: `640e494dd31edb66b0748239aa90547d77e8ab37843b5bc8352a5f39e5296f06`.

## Numeric presentation and provenance

The first three DWORDs below come from the explicit immediate positive
Git report `reports/cc12_application_vector_retirement_descriptor_prefix.json`,
selector `prefix`, whose exact12-byte value is
`2205931901000000b057db00` and SHA-256 is
`f739968ba1ecaa1310f01fdbc561487a62ea03c389bc22582ca82714940100ec`.
Its accepted PRIMARY report is also a finite positive Git input.
No prefix bytes were freshly queried. The last six DWORDs are the new paired
suffix capture; this table is not a fresh36-byte read.

| Descriptor offset | Address | Hexadecimal DWORD | Unsigned value | Evidence |
| --- | --- | --- | --- | --- |
| `00` | `00DB57B8` | `19930522` | 429065506 | prior positive prefix |
| `04` | `00DB57BC` | `00000001` | 1 | prior positive prefix |
| `08` | `00DB57C0` | `00DB57B0` | 14374832 | prior positive prefix |
| `0C` | `00DB57C4` | `00000000` | 0 | fresh paired suffix |
| `10` | `00DB57C8` | `00000000` | 0 | fresh paired suffix |
| `14` | `00DB57CC` | `00000000` | 0 | fresh paired suffix |
| `18` | `00DB57D0` | `00000000` | 0 | fresh paired suffix |
| `1C` | `00DB57D4` | `00000000` | 0 | fresh paired suffix |
| `20` | `00DB57D8` | `00000001` | 1 | fresh paired suffix |

The values remain numeric. ClassicFH3 layout, field assignments and handler
semantics remain conditional and unauthenticated. No value was dereferenced
or used to follow a target.

## Actual saved unit and admission

Root independently reviewed paired typed metadata at all36 exact addresses
`00DB57B8..00DB57DB` before admitting any suffix byte read. The saved unit is
a defined `ghidra.program.database.code.DataDB`, start `00DB57B8`, length36.
At the entry, both code-unit aliases refer to that unit. All35 interior
`code_unit_at` aliases are null and every containing alias refers to the same
unit. Both instruction aliases, both function aliases and instruction flow
are null throughout; all function lists are empty. Defined DataDB metadata
does not authenticate a language-level EH descriptor structure.

After Root admission, the packet read exactly24 suffix bytes twice, with
counts before the first read, between reads and before the closing36-address
metadata sweep, followed by a final count. The closing metadata agrees with
both opening sweeps. All108 complete direct typed responses preserve the
same saved `bsp.gpr` / `/battlestationspacific.exe`, x86/32-bit identity and
epoch98; all six count checkpoints are64730.

No adjacent data, prefix bytes, instruction text, prototype, function body,
callee or destination was queried. Automatically returned companion/target
objects remain opaque. Root gate JSON and its physical producer are retained;
the worker does not execute that producer.

## Offline closure and limits

The proof retains eight explicit immediate Git input blobs, actual
frozen/staged/final commit/tree objects, exactly this document and its report,
all Native command/raw/stream records, physical helper bodies and executed
collector/sealer preimages. No Native or helper attempt failed.
Pure original/distinct-copy ZIP replays verify manifest/Git closure, every
admitted metadata alias, paired suffix bytes, numeric decoding and the
positive prefix selector. Each guarded replay allows one archive read,
one declared receipt write and one retained code execution after bootstrap.
Source subtree IDs are revision identity only; Source content and previous
proof archives were not read.

This packet establishes numeric suffix evidence. Original/FH3/ABI, field
roles, action invocation/frame/receiver, state0 behavior, ownership,
activation and runtime remain unproved. No Source, build, new test, probe,
game, OS audio or Ghidra mutation occurred. Root review controls any later
parent Source authorization.
