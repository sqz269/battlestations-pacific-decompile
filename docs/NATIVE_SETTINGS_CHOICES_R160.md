# Native settings choice copies and language selection (R160)

The raw settings loader `008D8190` copies renderer resolutions through
`008D4EA0` into the global header at `F8895C`, copies antialias samples through
`008D4DF0`, and selects language through `008D56C0`. These helpers previously
had projected implementations. This packet supplies their complete normal
bodies over actual native headers and catalog rows in
`native_settings_choices.hpp/.cpp`. Names remain descriptive hypotheses.

| Entry | Bytes | Native ABI | Source contract |
| --- | ---: | --- | --- |
| `008D4DF0` | 106 | ECX destination, stack source, EAX destination, RET4 | Clear/ reserve / copy stride4 data-count-capacity vector |
| `008D4EA0` | 141 | ECX destination, stack source, EAX destination, RET4 | Clear/ reserve / ordered stride8 pair copy |
| `008D56C0` | 215 | ECX settings, stack C string, native void, RET4 | Temporary pooled name, first matching catalog row, index at settings+04 |

## Vector schedules

The headers are 12 bytes: data pointer, signed count, signed capacity. They are
not STL begin/end/capacity pointers. The default call services delegate existing
complete `86A430` / `86A220` DWORD resize/reserve and `8D4750` pair reserve.
Those services use the actual shared CRT allocator and raw backing storage.

Self-assignment clears the destination before reading the source count, leaving
an empty vector. Pair assignment first calls reserve(0) if capacity is negative,
decrements any positive count to zero, stores zero explicitly, then reserves
the current source count. DWORD assignment delegates its initial clear to the
existing resize(0) body.

Each loop captures its source row before optional growth. Growth doubles the
capacity as DWORD arithmetic and clamps signed results below two to one. After
growth, the destination count/base are reloaded. Pair copies read/store the first
word before reading/storing the second, so overlapping rows can propagate the
first word into the second. The source header base and signed count are reread
at their original loop points. No alias guard, snapshot, rollback or STL
assignment is substituted.

## Language schedule

The context borrows the actual catalog pointer `F88974` and count `F88978`.
Each `20h` row contains four native string headers; selection uses its first
length/data pair. The input name is constructed through the actual raw pooled
string helper `41E870`, then its data and length are captured in native EBX/EBP
order. The caller's operation record retains the temporary if a service fails.

Only equal-length rows are compared. Equal zero lengths match without touching
either data pointer. Nonempty comparison calls `BF7FBF`; after every such call,
the catalog pointer is reloaded. The loop reloads the current signed count.
The first hit writes its index to settings+04; a miss writes zero. Final cleanup
returns the captured temporary data/length through the actual raw pool, even
though the temporary header itself is kept as dangling native state.

The comparison default calls the linked CRT `_stricmp`. ASCII language matching
is exercised through that shared boundary; this packet does not establish
original VS2005 locale or invalid-input behavior. The source interfaces add
explicit contexts and are not binary ABI replacements. Operation failures
retain the site and allocation, reject replay, and require explicit diagnostic
cleanup; original FH3/SEH behavior is not implemented by that record.

## Evidence and verification

The existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified
for live queries. Three bodies and the catalog header match **474 live/PE
bytes**, comprising 462 bytes of code and 12 bytes of data. All ten direct
CALL rows verify. The stored bodies have no listing gaps; no repair was needed.
Existing annotation history is preserved, then names/evidence are saved,
read back and exported.

The strict MSVC Win32 build and all three existing CTests pass. One local
comparison harness runs **15 original/source pairs with 2,300 matching observed
bytes**: five vector scenarios for each stride and five language scenarios.
It compares vector count/capacity, source counts, backing data and service
traces; language cases compare the entire `BCh` settings receiver, catalog
identity/count and comparison traces. Return receiver identities are asserted.

The vector cases cover ordinary growth, self-assignment, empty sources with
negative destination capacity, overlapping rows, and source pointer/count
replacement during inner growth. Controlled service instrumentation initializes
new unused allocation bytes for comparable preimages and forces the inner
growth/reload case. The DWORD resize's internally allocated unused empty buffer
has no such preimage hook and its unused bytes are not compared. Existing raw
reserve/resize bodies still execute on both sides.

Language cases cover first-match behavior, a miss, zero-length strings with an
unreadable unused data pointer, an empty catalog with an unreadable unused base,
and catalog replacement during comparison. A source-only comparison exception
retains the temporary and unchanged selection, rejects replay, and is explicitly
cleaned up. Actual raw string-pool publication drains successfully. Both lanes
share the host CRT comparison boundary; this is a caller-schedule comparison,
not a new comparison of the old CRT implementation.

The raw language-catalog producer, settings loading/application ownership and
native-game admission remain open. No ordinary application or gameplay test is
claimed. Malformed live pointers, dangling overlap across actual reallocation,
asynchronous mutation, native exception unwinding and original ABI are outside
the verified domain.

Evidence is recorded in `reports/native_settings_choices_r160.json`; the local
harness and sealed artifacts are under `local/settings_choices_r160` and
`local/evidence-r160`.
