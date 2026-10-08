# Whole raw group member-vector entry

`copy_native_unit_group_member_vector_0070d980` reconstructs the complete
`[0070D980,0070D9F4)` entry: **116 bytes, 36 instructions, no calls or
relocations**. Original PE, saved-program bytes, fresh Win32 COFF and the
unique linked Source body are identical, with SHA256
`5b1bb5bd45da881947218c7b24f695d163b08f9915483c699f147c4c06bb9e59`.
The name describes the observed operation; the three record floats' wider
role remains a hypothesis.

The raw interface uses ECX for the actual group, ignores incoming EDX, and
takes two stack words: the actual output pointer and integer member identity.
Both paths return that same output pointer in EAX with `RET 8`, preserving
ESI and EDI. The saved decompiler's `void` result is incorrect: the actual
caller at `007672E0` passes a stack buffer and then reads three floats through
returned EAX. That caller obtains ECX from `[EBX+0x284]` and passes EBX as the
member identity. Its enclosing class and virtual-call admission remain unbound.

The entry captures signed `group+0x4F8` once. It searches actual 0x34-byte
records beginning at `group+0x18`, returning the **first** integer-identity
match; zero identity is not skipped. A hit copies record offsets
`+0x04`, `+0x08`, `+0x0C` through the original interleaved x87 `FLD/FSTP`
sequence. The output may overlap the reached record, so loading all three
values first, `memcpy`, or a copied record facade would change behavior.
There is no independent group store, but output aliasing can modify group
storage. The ordinary bounded storage domain is an aligned 0x508-byte group,
valid reached records and positive count 1–24. No guard or clamp was added.

A missing match or nonpositive count reads actual native DATA at
`00F87574`, `00F87578` and `00F8757C`, with sequential `MOVSS` loads/stores.
The header requires those **actual readable addresses**; an aliased output
also requires writable destination storage. Production creates no mapping,
default, replacement global or provider. Only the three numeric absolute
loads use `_emit` to retain their original encodings without MSVC's redundant
DS prefix.

Those DATA cells occupy writable `.data` beyond its raw file size. Their
zero PE-loader image is not twelve literal on-disk bytes, and neither it nor
saved-program zero cells establishes runtime immutability or provider
ownership. The accepted audit inspected capped samples of 40 references per
cell and current borrowed volatile declarations; these do not exclude other
or indirect writers. Copied zero vectors and semantic default-position hosts
cannot become native runtime providers.

One fresh standalone family compiled four translation units: the new entry,
current `native_unit_group_storage.cpp`, current
`native_unit_group_lookup.cpp`, and the ignored probe. The strict Win32 build
used `/W4 /WX /fp:strict`, linked no BSP archives, and passed on its first
attempt. The current 132-byte constructor and 57/56-byte lookup bodies matched
their accepted Source fingerprints and their unique linked bytes. The build
pinned eight frozen inputs, 176 actually included host headers, six system
libraries, three selected tools and four compiler/backend support files.
The PE32 fixture has an embedded `asInvoker` manifest.

Before opening its PE file view, the fixture reserved its own exact 64 KiB
ranges at `00CF0000` and `00F80000`. It then mapped qualified PE-loader
snapshot pages at `00CF4000` and `00F87000`, made them read-only, and checked
all page bytes and protection bookends. The first page provides the actual
`CF4888` snapshot cell required by the genuine Source constructor; the second
is the native loader-zero DATA snapshot. This admits only the frozen fixture.
The installed executable and live game were untouched.
The exercised output alias is inside actual group storage; aliasing the native
DATA cells was not exercised because these fixture pages are read-only.

The sole accepted execution used four phases:

| Phase | Caller input and output | Observed result |
| --- | --- | --- |
| Constructor-empty | Count zero, separate output | Native DATA fallback; positive-zero triplet |
| Positive absent | Four records, absent actual identity | Native DATA fallback; positive-zero triplet |
| First duplicate | First record `[-0, SNaN, 1.5]`, separate output | First match; `80000000 / 7FC12345 / 3FC00000`; masked invalid set |
| Zero identity plus alias | Zero member at record 2; output is that record's vector address plus four | Negative zero propagates through all three output words; the overwritten SNaN is never loaded |

Each trial rebuilt the same actual group through the current Source
constructor, then applied explicit caller-authored count, identities and
record float words. Both current Source lookup functions observed that same
storage. Comparison snapshots were output evidence, never target inputs.
Original and Source used the same actual group, member backing, output and
entry-stack addresses.

The fresh raw adapter captured 372 physical bytes: full GPRs, EFLAGS and DF,
ESP and both real stack arguments, native ESI/EDI save slots, 64 stack guard
bytes, all eight XMM registers, MXCSR, and the complete x87 save image before
any C++ float spill. Seven exact 80-bit canaries left precisely one free x87
slot. Both tested control words survived, including `0B7F`; the alias phase
also preserved DF=1. Whole 1,424-byte guarded group and 144-byte guarded output
storage were checked against the declared writes.

All capture bytes matched except the explicitly asserted and mapped final
`FSTP` instruction pointer for hits: Original entry `+0x6E` versus Source
entry `+0x6E`. Fallback preserved the seeded x87 instruction pointer exactly.
The duplicate phase set x87 SW `4D21`; the alias phase retained `4D20`, proving
that its overwritten signaling NaN was not converted. This entry returns
no value in ST0.

The family passed **14,572 checks**, with exactly four Original and four
Source calls, eight genuine Source constructors and sixteen Source lookup
calls. It executed once; no failed build, native retry or previous family
replay occurred. The report links the sealed recipe and manifest. All 821
prior preservation pins passed, including the 47-, 25-, 187- and 238-artifact
families, the separate 12- and 44-artifact audits, and the newly published
nested payload review. Historical receipts and build associations were
preserved without repinning them.

This establishes a conditional raw-memory Source component and its bounded
fixture. Root integration, full project build and independent verification
are separate. Actual mutable runtime DATA ownership, original class and
lifetime, concurrent mutation, unmasked faults, enclosing caller/world
admission and gameplay remain unbound. The production entry does not reset
the caller's x87 control word.
