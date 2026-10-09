# Pending tick storage and initializer bindings

The admitted `00875890` Source constructor has an exact data binding available
for its `00D7A260` initializer. The pending and group lists still require a
common mutable owner and verified group initialization before a game caller
can use the new constructor and splice consumer.

This read-only primary audit selects only 28 Native data bytes. The saved
Ghidra program is `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`,
with unchanged 64729 functions. All selected bytes match the exact installed
PE image; no Native code body, profile table, original process or handler was
opened or executed. Initial file contents are distinct from running state.

| Address | Selected bytes | Initial saved words | Storage |
| --- | --- | --- | --- |
| `E0B6D0` | 12 | `0, 0, E0B704` | `.data`, pending head prefix |
| `E0B704` | 12 | `0, E0B6D0, 0` | `.data`, pending tail prefix |
| `D7A260` | 4 | `BF800000` | read-only `.rdata`, exact `-1.0f` bits |

Both selected sentinel profile words are initially zero. The constructor's
`D0DEC8` publication on an actual registration node does not establish that
the sentinels contain that profile. The prefixes prove head.next/tail.prev
identity for the initial linear empty list, without asserting the contents
of their unselected remaining fields or a complete Source class layout.

The Source constructor reads its bound initializer after publishing the
node's pending links. It transfers one DWORD into node+30 with no float
arithmetic. The selected `BF800000` word is a default literal, not evidence
of an advancing clock or a new timer producer. Current Ghidra references to
the word are reads; their absence of writes is not an exhaustive dynamic
immutability proof.

`GameNativeReadOnlyData::data_at(D7A260,4)` can supply the original word when
that service owns the required mapped band for the caller's entire lifetime.
Its verified domain is `CE2000..E07B23`; current particle-runtime Source
already borrows a float at this address. A raw DWORD consumer must retain the
bits and late load, rather than convert a clock or cache a float at entry.
This is an available Source service contract, not a demonstrated binding of
the newly admitted constructor in the application or a new mapping test.

The same service excludes mutable owners and cannot supply the pending
sentinels at `E0B6D0/E0B704`, which lie outside its domain. Copying the Native
numeric next/previous values into separately allocated Source objects does
not establish the required pointer identity. The constructor's pending-last
cell remains the actual tail+4, including its two separate current reads.
The existing fixed-step host only records its assumed-empty splice path;
it does not mutate those actual cells. The abstract unit construction hook
has no demonstrated concrete binding to this new Source constructor.

The earlier 111-byte splice audit establishes the real group address
expression `F876C0 + raw_index*68h`, with no range check. Its saved group0
prefixes are virtual zero-fill and its bounded `CD27D6` store is insufficient
to establish all group initialization. Current metadata has no function at
`CD27D6`; Ghidra's ordinary group xrefs do not report that saved instruction.
This metadata discrepancy does not authorize a guessed initializer boundary
or a flow repair. No initializer or adjacent instruction was opened here.

The next useful boundary is a separately bounded primary/Astra audit that
establishes the group initializer's real entry, full body and startup route,
then a shared mutable owner with preserved pending/group identity and phase.
The production caller must also provide valid receiver+34h storage, real
payload/group provenance, registry/manager lifetime and constructor-failure
behavior. A standalone empty list or substitute deletion callback would not
resolve those contracts.

The report pins 14 current inputs and nine capture files against the published
baseline, including the admitted constructor primary report. No Source,
CMake, ledger, Ghidra annotation or flow change, build, fixture, probe, ABI
credit, application startup or gameplay result is added by this audit.
