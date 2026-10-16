# Actual hidden-record lookup pair

Addresses: `004691B0`, `00469480`. Existing descriptive names remain hypotheses,
with prior names/comments/evidence preserved. Older `Seg6Host` and vector-view
projections remain available; they do not establish the raw schedules below.

| Routine | Native span, inclusive | Bytes / instructions | Coverage |
| --- | --- | --- | --- |
| `004691B0` mapped-word lookup | `004691B0..0046923D` | 142 / 54 | Complete ordinary raw Source at accepted providers |
| `00469480` membership byte | `00469480..004694ED` | 110 / 43 | Complete ordinary raw Source at accepted providers |

Both original entries receive the database in ECX, leave EDX unused, preserve
EBX/ESI/EDI, and end with `RET 4`. Their first instruction reads the stacked
name-pointer DWORD at entry ESP+4, before private-frame writes. At entry ESP=P,
input4=P+4, key header8=P-10h and output iterator8=P-8 bind distinct regions.
The caller does not pre-zero the header or output. Stable/disjoint Source
metadata binds actual backing; payload aliases with those actual regions remain
admitted where reached accesses and accepted provider domains are valid.
Implicit native/compiler/helper/child-argument, return/EH/spill/saved-register
or asynchronous aliases are not supplied by these new ordinary interfaces.

The exact raw constructor initializes the actual header. Both find in the map
at database+18h modulo32 through the published raw Find. Read CURRENT key.data;
only if nonnull read CURRENT key.length, add one modulo32 and return the
captured data through the real current raw pool. Zero data skips the length read and
all pool work; wrapped zero byte counts still reach the pool. No header reset,
cached pool, retry, count, ownership or positive-only guard is added.

The mapped-word entry releases the key immediately after Find, before reading
the output owner or map head. Capture CURRENT output.owner, then CURRENT
map.head even for a null owner, before the first diagnostic. A null or wrong
owner uses the returning current-CRT report. Only after that continuation read
CURRENT output.node, retaining it thereafter. Equality with the captured head
returns zero. On the unequal path, captured null owner reports again; then read
CURRENT captured-owner+4, report if that head equals the captured node, and read
CURRENT captured-node+14h only after the final continuation. These are raw
DWORD results; no nominal record, pointer validity, ownership or lifetime is
inferred. A matched mapped-zero word and absence share the zero result.

The membership entry instead captures head from database+1Ch after construction
and BEFORE Find. Use the actual child EAX iterator pointer; capture its owner
once and retain it through the returning diagnostic. Compare CURRENT iterator.node at output+4
after the report with the earlier captured head, then read CURRENT key.data and
retain the comparison byte across normal release. It never tests mapped+14h.
A matched node with a zero mapped word can therefore return one. Original
`SETNZ BL` and final `MOV AL,BL` establish AL0/1; upper EAX bits can retain pool
return state and are not a canonical DWORD boolean contract. Source exposes
only the byte. The older vector-view `contains = find != nullptr` projection
does not cover this mapped-zero distinction.

Neither complete flat body installs an outer FH3 cleanup frame. The Source
uses raw backing and explicit normal release, with no outer RAII cleanup if a
child or report throws. Original private exception/fault/SEH transport remains
unproved. Returning-handler paths are complete, including the subsequent raw
owner/head/node accesses; no recovery or validity guard is invented.

Concrete provider closure: actual header constructor, complete raw map Find
and its accepted comparison/iteration boundaries, raw manager/pool getter and
return, and the real returning current-CRT report service. Source composition
does not expose the canonical Find's native private argument-slot aliases or
invent a callback/provider. Current CRT, pool lifetime, valid reached backing
and input C-string domains remain inherited. The result implies no CString
validity or game/binary ABI.

Actual direct native transfers:

- `004691C0 CALL 0041E870`
- `004691D4 CALL 00468CD0`
- `004691EC CALL 00419CC0`
- `004691F3 CALL 00BD1510`
- `00469208 CALL 00BF6713`
- `00469224 CALL 00BF6713`
- `0046922E CALL 00BF6713`
- `00469491 CALL 0041E870`
- `004694A8 CALL 00468CD0`
- `004694B9 CALL 00BF6713`
- `004694D7 CALL 00419CC0`
- `004694DE CALL 00BD1510`

Each entry has one current live direct caller:

- `0094477D CALL 004691B0` in `00944680`: the raw EAX result is stored in the spawn request at `0094478D`.
- `008A9F1B CALL 00469480` in `008A9E10`: the caller stores only AL at `008A9F24`.

Only those bounded setups/continuations are reviewed; no complete caller-body
or external ABI claim is made. All 252 live bytes match the installed PE and
all 97 instructions, including both complete returns of the mapped-word entry,
are covered. Evidence and Source/provider hashes are recorded in the report.
Both retained hypothesis names and full prior comments/evidence were verified
after locked annotation, project save and forced export refresh. All fourteen
native transfer rows/Markdown claims pass without failures or unchecked claims;
twelve fresh typed child sites have normal fallthrough and no overrides.
Independent primary review of both complete native bodies and whole
Source/header passed; the recorded Source/header and canonical provider hashes
were asserted unchanged before recording that review. Unique CMake registration
and the normal Win32 build remain pending; no CMake request is queued while the
primary finishes the preceding batch promotion. No new tests are
added; packet-specific native execution, original ABI and game validation are
unproved.
