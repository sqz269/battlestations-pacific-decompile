# Raw scene name-hash DWORD lookup pair

Addresses: `00468660`, `00467EE0`. Names describe observed behavior and remain
hypotheses. Existing names/comments and the older `Seg6Host` projections are
preserved; those projections do not establish these actual storage schedules.

| Routine | Native span, inclusive | Bytes / instructions | Coverage |
| --- | --- | --- | --- |
| `00468660` class-name result | `00468660..004686B3` | 84 / 30 | Complete ordinary raw Source at accepted providers |
| `00467EE0` payload result | `00467EE0..00467F29` | 74 / 27 | Complete ordinary raw Source at accepted providers |

Both original bodies receive the database in ECX and one DWORD at entry ESP+4,
leave incoming EDX unused, preserve ESI, and end with `RET 4`. EBX/EDI/EBP are
untouched. The new ordinary interfaces explicitly bind the actual parameter4
and writable private iterator12 backing; they are not binary replacements.
The Original private iterator starts at entry ESP-C, with its node at ESP-8 and
index at ESP-4; saved ESI is at ESP-10. These physical/private frame relations
are evidence, not equivalences to the new C++ frame or argument homes.

Capture database+34h modulo32 and write only iterator.table0. Real Begin may
leave its index unchanged on the zero table-test path, so no private iterator
word is pre-zeroed. Reload iterator.node4 after Begin; an initial zero returns
zero without reading parameter4. Otherwise read that word once and retain its
arbitrary DWORD bits across every real Next. Each iteration captures node+8,
then compares the DWORD at that captured pointer with the saved key. Next's
EAX result is ignored; reload iterator.node4 and test it independently.

`00467EE0` returns the already captured payload pointer through `MOV EAX,ECX`
at `00467F21`; it does not return the node or reload node+8 on a match. Its old
evidence text described a node return; the assembly establishes these raw bits.
`00468660` instead reads matched node+4 once and returns that pointer, with a
null-name fallback to the immediate address `00E18560`. Both return zero on
exhaustion. No nominal payload class, ownership, lifetime or CString validity
is required or recovered from these raw results.

Stable Source metadata and explicit bindings remain separate from reached
storage. All reached actual input/private/table/node/payload accesses must be
valid. Payload links may alias the actual input/private cells. No implicit
native/helper/compiler-frame, saved-register, return/EH/spill or asynchronous
aliases are supplied. There is no extra key range, null-payload, count, owner,
rollback, callback or recovery guard. Native invalid-memory/fault/SEH transport
and private ABI remain open.

The only native children are the published complete raw iterator providers in
`native_scene_name_hash_iterator.hpp/.cpp`. Their actual table/index/head scans
and alias schedules are retained. The class-name entry takes a stable explicit
binding for the existing distinct E18560 empty-data role, borrowed from the
`GameNativeStringProcess` library-empty getter declared in
`game_native_string_process.hpp`; it creates no owner or provider and never
reads the fallback byte. The original instruction returns
an immediate address rather than reading a pointer cell. Current Source role
storage and original physical address are not claimed identical.

Live Ghidra bytes at E18560 are zero. This address lies in the PE `.data`
virtual tail: section RVA A08000, raw size10000h, offset10560h. It has no
file-backed byte there and receives zero initialization. The historical
`SCENE_ENTITY_FACTORY.md` claim that its initial bytes are `Ys` is superseded by
this read-only proof, not copied into the contract. Original writable DATA is
not claimed universally immutable; later runtime writes remain unproved.

Direct native transfers:

- `0046866E CALL 00466BB0`
- `0046868B CALL 00466C00`
- `00467EEE CALL 00466BB0`
- `00467F0B CALL 00466C00`

Representative actual incoming transfers:

- `0046BEAF CALL 00468660` in `0046BE90`: a DWORD input, null test and borrowed name construction afterward.
- `0046D41C CALL 00468660` in `0046CF40`: current database receiver, class DWORD, raw result forwarded to the entity factory.
- `0046DB1D CALL 00468660` in `0046D930`: raw result forwarded to the entity factory.
- `0046DE6B CALL 00468660` in `0046DC10`: current database receiver and raw result forwarded to the entity factory.

There are 13 live direct incoming sites for the class-name entry across these
four functions; only representative bounded contexts are reviewed, not their
whole bodies. The payload-result entry has no live incoming references; that
does not prove it is dead or infer an external ABI. Existing producer/layout
documentation supports the class-id/name interpretation, while this Source
requires only the exact DWORD comparisons and pointer fields above.

Whole guarded live listings and all 158 bytes match the installed PE. Evidence,
typed transfers, preserved locked annotation/save/export/readback and Source
hashes are recorded in the report. Unique CMake registration, normal Win32
build and focused compiler inspection are pending. No new tests are added;
packet-specific native execution, original ABI and game validation are unproved.
