# Native language catalog lifetime (R164)

Addresses: 008D5AA0, 00CD2DA0, 00CDEEA0. Actual catalog header: 00F88974.

Three complete normal bodies now operate on the actual 0Ch pointer/count/capacity
header and the R162 pooled 20h rows. This closes the remaining container lifetime
dependency of the raw language-catalog producer. Descriptive names are hypotheses,
not recovered symbols. The ordinary application's projected catalog remains separate.

## Contracts and evidence

| Entry | Native ABI and size | Reconstructed behavior |
| --- | --- | --- |
| 008D5AA0 | ECX header, signed stack count, RET4; 113 bytes | Compare capacity as signed; reserve if required. Capture post-reserve count, use wrapped 32-bit slot arithmetic and zero eight words of each nonzero new slot. Shrink by decrementing the current published count first, destroying that current last row, then rereading count. Publish requested count last. |
| 00CD2DA0 | No inputs, EAX registration result, RET; 12 bytes | Register CDEEA0 through BF6FF5. No construction or header writes. Source composition supplies a stable shutdown thunk; default registration uses host std::atexit. |
| 00CDEEA0 | No inputs, RET; 25 bytes | Resize actual F88974 to zero, reload its backing pointer, then BF6989. Leave the pointer and capacity dead; no post-free reset. |

The resize helper does not add a negative-count guard, clear unused capacity, or
free storage when shrinking to zero. A zero computed growth slot skips its eight
stores but still advances the loop. Shrink order is descending rows, with each
row's four current pooled strings returned in descending member order by R162.
Callbacks use existing raw pool publication and lifetime services.

Ghidra originally had no function at CD2DA0 and stopped CDEEA0 at the free call.
The twelve-byte registration function was defined from matching live/disk bytes.
For shutdown, explicit tail repair cleared the erroneous call flow override;
recreation after that repair restored CDEEB7 POP ECX and CDEEB8 RET to the stored
body. The earlier recreation alone did not extend it; both attempts are retained
in evidence. No global no-return annotation was changed. Saved names, appended
comments, prior annotations, refreshed exports and forced snapshot are recorded.

Source operations retain the current row, completed row counts, last call site,
nested storage operation and captured shutdown backing on failure. A failed or
unfinished operation requires explicit caller cleanup before acknowledgement;
replay is rejected. This is a source failure contract, not recovered native FH3.

## Validation

- All 150 code bytes plus the 12-byte catalog data span match live Ghidra and
  the original PE. All five direct CALL rows are mechanically checked.
- Strict MSVC Win32 build and the three existing CTests pass.
- One local differential diagnostic executes copied original caller bodies and
  source bodies: 11 paired groups, 768 identical observed bytes.
- Resize groups cover growth with and without reserve, shrink and actual pooled
  return order, unchanged size, zero computed slot, and a controlled valid backing
  preimage for a negative target. The unused growth tail is explicitly checked.
- Shutdown covers populated/empty catalogs and a free callback that replaces
  all header fields; source and original both retain those callback stores.
- Registration checks the exact callback identity, unchanged header, zero and
  negative return values, and explicit invocation of the registered shutdown.
- A source-only free exception retains the already-shrunk catalog and backing;
  replay rejection, manual backing cleanup and actual pool/manager drain pass.

Both differential lanes share the already reconstructed R162 reserve/row destructor
and raw pool/manager helpers. Only these three new caller bodies are newly copied
and compared. Allocation addresses are normalized to presence and reclaimed member
identity; live string bytes, count/capacity, unused-row preimages, and callback
trace are inspected. The standalone fixture controls globals and allocation
preimages. Default host atexit internals and actual process-exit ordering are not
compared. Registration is inspected/invoked explicitly by the fixture.

Original ABI replacement, FH3/SEH, private-stack aliasing, hardware faults,
asynchronous mutation, ordinary startup binding and gameplay remain unproven.
No game process was launched or stopped for this packet.

## Follow-up packets

Compose the full raw 008D7BC0 producer using R161 scanner, R162 row/list storage,
R163 hints owner, and this lifecycle. Then finish the actual settings loader and
canonical application ownership. Do not copy the projected settings or hints
objects into raw storage or treat the fixture as game validation.

Evidence: reports/native_language_catalog_lifetime_r164.json. Local comparison,
saved analysis receipts and sealed artifacts are under local/language_catalog_lifetime_r164
and local/evidence-r164; their hashes are in the report.
