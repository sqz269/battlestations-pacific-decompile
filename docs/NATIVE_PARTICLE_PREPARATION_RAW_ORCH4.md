# Raw particle preparation dispatch

## Scope and evidence

This raw-string companion reconstructs the complete dispatcher bodies:

| Address | Bytes | Original ABI |
|---|---:|---|
| AF40E0..AF4105 | 38 | ECX actual resource/variant, plain RET |
| AF9F50..AF9FA0 | 81 | ECX actual emitter definition, plain RET |

Both installed-PE spans match live Ghidra byte for byte through their final RET.
The live bodies contain 19 and 36 instructions with no analysis gaps. Their two
direct calls target AF9F50. AF9F50 also calls current virtual slots +14 and +0C.
Neither dispatcher has an exception frame or local owned temporary.

The existing `NativeParticlePreparationDispatch` interface remains available.
The added overloads take `NativeStringRawPoolContext&` and use the actual owner,
its embedded pointer rows, and the same actual string-pool publication and gate.
Both interfaces instantiate the same traversal body. They introduce no callback
adapter or replacement object for the five proven final particle profiles.

## Concrete method routing

All five original table prefixes were checked against live Ghidra and the PE.

| Final profile | Preparation +14 | Range +0C |
|---|---|---|
| D5DD18 Sprite | B0A000 | B00920 |
| D5DCC0 Axial | B07660 | B00920 |
| D5DCEC Floating | B087B0 | B00920 |
| D5DB00 Object | AF8A30 | B00920 |
| D5E048 Tracer | B0A0F0 | B0A100 |

These call the existing genuine providers in `native_particle_type_preparation`
and `native_particle_type_base`. Sprite, Axial and Floating assign their actual
Layer material names using the raw string pool; Object preparation and both
Tracer methods are genuine one-byte RET bodies. B00920 keeps the native signed
range calculation, including the empty-count -1 result.

Other table profiles require their actual callable native virtual methods.
That remains a foreign-object boundary; no missing method is replaced with a
no-op. The new overloads do not add a general registry of numeric game profiles.

## Traversal and mutation order

AF40E0 walks inline resource rows at +10 with current signed count +30.
AF9F50 first recursively walks emitter children at +3C with current count +4C,
then particle rows at +54 with current count +68. Each loop reloads its count
after the call. The next row follows the captured row address by four bytes.

The first particle call captures the current row member and table. After +14
returns, the implementation reloads the row member, reloads that member's table,
and resolves +0C from that second table. It does not reuse the first owner or
profile. Underlying provider exceptions propagate; the dispatcher owns nothing
to unwind. Counts, pointers and table slots keep their native validity contract.

## Validation and limits

- Strict Win32 `scripts/build.ps1` and all three existing CTests passed.
- An ignored probe copied both complete original bodies (119 bytes), relocating
  only their two direct calls and binding external method targets to the genuine
  reconstructed providers. Original and legacy runs used relocated table storage;
  the raw run used the five proven numeric profiles. All used actual string pools.
- Original/raw/legacy results agree for all five profiles: material strings,
  range fields, recursive children and members. A callable-method mutation case
  replaces the member and table after +14 and shrinks all three current counts;
  the original and both source interfaces agree. Signed zero/negative counts
  skip invalid unused rows.
- These are source composition and bounded original-code comparisons. Native
  exception transport, arbitrary invalid pointers, concurrent mutation, binary
  substitution and gameplay are unvalidated. Names remain descriptive hypotheses.

See `reports/native_particle_preparation_raw_orch4.json` for complete hashes,
original bytes, old annotations/ledger records, calls and probe receipts.
