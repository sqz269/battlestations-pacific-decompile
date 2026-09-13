# Actual MPAK record copies and shared string-vector replacement

Addresses: 00bb65a0, 00bb6630, 00543e50, 00bb6180, 00cc44f0, 00cc4510

This packet reconstructs **three complete game-owned bodies, 332 bytes**, using
actual Win32 record storage and the existing native string/vector implementations.
It adds no owner domain and no STL implementation. The file record's `BB6180`
offset-vector copy remains an explicit fallible library operation supplied by
`NativeMpakOffsetVectorCopyLibrary`. Descriptive names remain hypotheses.

| Entry / inclusive end | Source interface | Original ABI | Coverage |
| --- | --- | --- | --- |
| BB65A0..BB662F, 144 bytes | `copy_construct_native_mpak_file_00bb65a0` | ECX destination24h, stack source24h, EAX destination, RET4 | Complete actual body and name-only cleanup schedule |
| BB6630..BB66AC, 125 bytes | `copy_construct_native_mpak_directory_00bb6630` | ECX destination14h, stack source14h, EAX destination, RET4 | Complete actual body and name-only cleanup schedule |
| 543E50..543E8E, 63 bytes | `copy_native_string_vector_00543e50` | ECX destination0Ch, stack source0Ch, EAX destination, RET4 | Complete actual shared helper |

The record-copy declarations are in `include/bsp/native_mpak_record_copy.hpp`.
The generic helper extends the existing `native_string_vector.hpp/.cpp` and
reuses `NativeStringVectorStorage`; it is not an archive-specific vector type.
The primary can implement a compatible external STL binding through the narrow
`copy_00bb6180(void* destination, const void* source)` virtual operation. It must
operate on original10h allocator/begin/end/capacity storage and preserve +0.
There is no successful fallback. The caller ignores its EAX return value.

## Recovered storage and ordering

The detailed producer, call-site and library classification audit is in
`NATIVE_MPAK_CONTAINER_BA_DISCOVERY.md`. `BB6870` and `BB7C50` establish the file
and directory layouts; the new copies preserve those layouts:

- File24h: pooled name at +0/+4, DWORDs +8/+C, flag byte +10, padding +11..+13,
  offset-vector allocator +14, and vector begin/end/capacity +18/+1C/+20.
- Directory14h: pooled name at +0/+4 and the custom data/count/capacity vector
  at +8/+C/+10. The outer directory array uses a different, STL10h header.

Both record copies zero the destination name header **before** their self-alias
guard. They then resize from current source length, reload the source length
for the nonempty branch, and copy current destination length bytes using the
current source and destination data pointers. The established BF7680 contract
uses `memmove`, preserving its original backward-overlap behavior.

The file routine next copies DWORD +8, DWORD +C, and byte +10 in that order,
without touching padding or the allocator word. It then arms name cleanup and
calls the supplied offset-vector copy on the two records+14h. The directory
routine arms name cleanup, zeros the three member-vector words, and calls the
actual shared replacement-copy helper. It does not cache a copy of either
record across allocation callbacks.

`543E50` first resizes destination to zero. It reads source count after that
call, reserves with the existing minimum-one-capacity rule, and appends while
the signed index is less than **current** source count, reloading source data
each iteration. A volatile view makes those reloads explicit. Self-copy thus
empties the vector while preserving adequate existing capacity; it is not a
successful no-op. All nine call sites across seven callers were checked during
discovery, including both preexisting and zeroed destination headers.

## Exception ownership

The original file metadata is `DFDE68 -> DFDE60 -> CC44F0 -> 41DD20`;
directory metadata is `DFDE94 -> DFDE8C -> CC4510 -> 41DD20`. Each has one
unwind state with successor -1. Both eight-byte actions use the saved
destination name. The actions and all three bodies matched the installed PE.

The source starts its `try` only after the initial name copy succeeds. A failure
while initially allocating/copying the name therefore has no armed owner. A
failure from the nested vector copy releases only the **current** destination
name and rethrows. It leaves header bytes and partially built nested storage
as they stand; there is no nested-vector rollback or whole-record destructor.
The shared helper itself adds no catch or cleanup. Existing
`NativeStringStorage::release` remains noexcept, so this is the documented C++
exception domain, not an original FH3/SEH replacement.

## Validation and limits

The ordinary strict MSVC Win32 build passes, along with both existing CTests
(`reconstructed_math` and `native_math_differential`). All eight native test
seed spans were reverified. No permanent test was added. A single ad hoc
fixture passed **six comparison pairs and 606 assertions**:

| Pair | Compared behavior |
| --- | --- |
| 0 | File deep copy; allocation callback changes source name pointer, DWORDs and flag; later loads observe them; padding/allocator remain A5 |
| 1 | Directory deep copy with two named members |
| 2 | Shared vector self-copy releases names backwards, becomes empty, retains capacity2 |
| 3 | File nested-copy failure preserves a partially published offset vector and releases only the name |
| 4 | Directory failure during the second member allocation retains the first member/backing and releases only the record name |
| 5 | Initial record-name allocation failure leaves the cleared name header and arms no cleanup |

The successful paths execute the retained three original bodies, relocated by
30000000h inside a private probe. The directory original calls the original
`543E50`. External calls share the existing actual string/pool/vector functions.
The file offset operation uses a clearly marked, fixed three-DWORD **test
contract** on both sides. It is neither a production binding nor a comparison
of original `BB6180` or the STL growth machinery.

For injected failures the fixture captures the executed original prefix,
restores the wrapper's register/stack/FS state, and directly invokes the original
eight-byte name action only when that state was armed. It compares this with
the source's normal C++ catch path. **Original FH3 exception dispatch is not
executed or validated.** The fixture has a narrowly scoped C4733 suppression
around restoration of the already captured FS chain; production code retains
the strict warning settings. Fixture teardown separately drains intentionally
retained partial storage and destroys its single actual pool publication.

The passing development attempt is
`local/mpak-record-copy-attempts/attempt03/manifest.json`: 15 linked objects
matched unique members of the current archives, and 283 compiler-observed
headers were captured. The runner pins the fixture, current source/header/
object/library inputs and exact HEAD across each replay. The development run
occurred before the implementation commit; its file hashes identify the tested
changes. Primary integration still requires validation of its final candidate.

The portable bundle contains the runner, probe source, byte header, native
manifest, support module, expected result and fixed original-outcome baseline.
Keep these seven files together, then run:

```
python local/replay_mpak_record_copy.py --repo <built-candidate> --output <new-attempt-directory>
```

The first fixture attempt failed at compilation on the FS-chain warning; the
second failed while reserving low virtual addresses before executing an
original body. Both are retained. Relocating the originals fixed the address
collision. The first source build also preserved a CMake-registration failure;
the only registration edit now uses the existing deferred form.

These results establish bounded reconstructed behavior with explicit shared
dependencies. They do not close the parser's outer-container contracts, prove
arbitrary stack aliases, allocation failure inside the original CRT/STL,
simultaneous cleanup exceptions, native ABI compatibility, installed archives,
startup reachability or gameplay.
