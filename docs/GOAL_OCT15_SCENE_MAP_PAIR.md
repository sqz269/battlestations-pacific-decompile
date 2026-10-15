# Scene-record map pair construction

Addresses: `00469980`, `00469A20`, `00469FC0`, `00C61BE0`, `00C61BE8`.

This packet reconstructs the actual 12-byte pair bodies and the constructor's
two ordinary C++ unwind scopes. The eight-byte key header is at `+0`/`+4`;
the following DWORD is at `+8`. The observed scene-reader caller supplies a
hidden-entity record pointer there, including null after allocation failure.
The Source API keeps opaque DWORD bits and introduces no alternative owner,
provider callback, stub, or copied NativeString object.

| Routine | Inclusive body | Bytes / instructions | Coverage | Native ABI |
| --- | --- | --- | --- | --- |
| `00469980` | `00469980..004699C7` | 72 / 28 | complete | ECX destination12B, stack source12B, EAX destination, `RET4` at `004699C5` |
| `00469A20` | `00469A20..00469A67` | 72 / 28 | complete | same, `RET4` at `00469A65` |
| `00469FC0` | `00469FC0..0046A060` | 161 / 56 | complete at accepted raw-storage boundaries | fastcall ECX destination12B, EDX mapped DWORD, stacked by-value8B key, EAX destination, `RET8` at `0046A05E` |
| `00C61BE0` | `00C61BE0..00C61BE7` | 8 / 2 | complete Source unwind scope | native EH virtual EBP+4 is the by-value key; no standalone Source frame ABI |
| `00C61BE8` | `00C61BE8..00C61C00` | 25 / 7 | complete Source unwind scope | native EBP-14h completion bit, EBP-10h output pointer; no standalone Source frame ABI |

The three normal bodies cover 305 bytes/112 instructions. The two private
unwind scopes add 33 bytes/9 instructions of native behavior evidence. The
native FH3 handler and metadata are read-only evidence, not binary replacements.
Existing descriptive Ghidra names remain hypotheses and are retained.

## Copy construction

Both copies have identical non-call bytes and behavior. Compare destination
and source, then clear destination length and data before testing identity.
For distinct headers capture source length, resize with preserve1, then reload
the current source nonempty guard, destination length, source data and
destination data. The concrete raw `00426060` helper matches this entire
prefix. Its accepted raw-pool composition and overlap-capable `BF7680` copy
are reused. Finally, read CURRENT source `+8` and store those same DWORD bits
to destination `+8`, including empty and identity paths.

The late loads are `004699BB: 8B 4F 08` and `00469A5B: 8B 4F 08`; the stores
are `004699BF: 89 4E 08` and `00469A5F: 89 4E 08`. There is no scalar numeric
conversion or x87 operation. A prior destination buffer is abandoned even on
identity. Initial construction failure adds no output cleanup. Aliases must
remain within the accepted live-memory/ownership domain of the header helper;
the late trailing-word load observes completed header/allocation/copy work.

## By-value constructor and cleanup

At entry ESP=`S`, the native by-value parameter occupies `[S+4,S+12)` and the
return address occupies `[S,S+4)`. After the 36-byte prologue, `ESP+28h` is
key.length, `ESP+2Ch` is key.data, `ESP+20h` is EH state, `ESP+10h` is the
output completion flag, and `ESP+14h` is the retained output pointer.

1. Capture EDX into EBX. Capture key.data into EDI at `00469FEA`, THEN
   key.length into EBP at `00469FEE`, before any destination write.
2. Clear destination `+0`, then `+4`. Compare the actual destination address
   with the address of the actual by-value parameter cell. Arm EH state1 at
   `00469FFD`, before entering the distinct branch.
3. For distinct storage, resize from captured length with preserve1. The
   nonempty guard uses captured EBP, not a reload of the parameter header.
   Copy CURRENT destination length to CURRENT destination data from captured
   EDI. The source pointer is never reloaded after allocation.
4. Store captured mapped bits at destination `+8` (`0046A023`). Set the output
   completion flag at `0046A028`, then switch to state0 at `0046A030` before
   attempting normal parameter-buffer return.
5. If captured EDI is nonnull, return it through the concrete raw getter and
   sized pool using captured EBP+1 with DWORD wrap. Getter evaluation happens
   even for large blocks or disabled small returns. No parameter-header clear
   or reload occurs on this normal return path.

This constructor cannot delegate to `00426060`: its source captures occur
before destination clearing, its guard and copy source use those captures,
and it consumes the incoming by-value buffer. The new Source interface passes
the actual callee-owned eight-byte parameter cell by address, preserving that
cell's identity and allowing unwind to read its current fields. It does not
copy that cell into a temporary owner. On success the caller discards the cell
without another destructor, just as native `RET8` discards the stacked value.
Its bytes are normally unchanged and may describe storage already returned.
Identity still clears the output header, stores the mapped DWORD, and returns
the captured original buffer. Partial overlaps are admitted only when every
access and ownership return remains valid under the concrete pool authorities.

Handler `00C61C01` selects FuncInfo `00D892A0`: magic `19930522`, max state2,
no try blocks, EH flags1, unwind map `00D89290`. The map is:

| Active state | Next state | Action | Complete Source binding |
| --- | --- | --- | --- |
| 1 | 0 | `00C61BE0` | destroy CURRENT actual parameter header through raw `0041DD20` |
| 0 | -1 | `00C61BE8` | if completed, clear its flag before destroying CURRENT actual output header through canonical raw `0041DD20`; native `00468B30` is instruction-identical |

A failure during initial key construction runs current-parameter cleanup;
the incomplete output's flag remains clear, so its partial storage is not
destroyed. A failure during normal parameter return runs output cleanup and
does not retry the parameter return. A noexcept RAII destructor performs these
actions during true C++ unwinding; a second cleanup exception terminates before
later work. The normal raw getter exception remains propagating. This models
the ordered Source cleanup domain, separately from native FH3/private frame,
SEH exception classification, instruction addresses, or fault resumption.

## Caller and dependency evidence

The only live native caller of `00469A20` and `00469FC0` is
`BSP_SceneFile_ReadEntityBlock` (`0046CF40..0046D927`). `00469980` has no live
xrefs. The live caller excerpt establishes the actual argument lifetime:
`0046D6FF SUB ESP,8`; `0046D709 MOV ECX,ESP`; `0046D71B` constructs the key in
that stack cell. `0046D720 MOV EDX,ESI` forwards the newly constructed record
pointer (or zero from `0046D6FD`) and `0046D729` enters the consuming pair
constructor. Its `RET8` discards exactly that key. The returned pair is pushed
at `0046D72E`; `0046D73E` makes the pair copy used by the map insertion.
The caller later releases both pair keys separately. No ownership of the
mapped record is added by either pair body.

| Site | Function | Native target | Verified contract |
| --- | --- | --- | --- |
| `0046999E`, `00469A3E`, `0046A00A` | respective pair body | `0041DD40` | raw actual-header resize, preserve1 |
| `004699B3`, `00469A53`, `0046A01B` | respective pair body | `00BF7680` | overlap-capable copy; captured source for consuming constructor, live source for copies |
| `0046A03E` | `00469FC0` | `00419CC0` | real getter before normal captured-block return; getter consumes no stacked return arguments |
| `0046A045` | `00469FC0` | `00BD1510` | captured block and captured length+1, third stacked word1 unused, `RET0C` |
| `00C61BE3` | `00C61BE0` | `0041DD20` | tail jump, current parameter header |
| `00C61BFB` | `00C61BE8` | `00468B30` | tail jump, current completed output header after flag clear |

Existing header layout in `scene_record_map.hpp` and the producers above were
checked. The prior scene-map source is an injected semantic host and contains
no unindexed implementation of these actual pair bodies. Relevant references:
`SCENE_RECORD_MAP.md`, `SCENE_FILE_READER.md`. All live batches verify the
configured bsp project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`.
Prior names/comments/documentation were saved before locked annotation; names
and comments are preserved, the project saved, and affected exports refreshed.

The report records draft/build status and verification receipts. These are new
ordinary C++ explicit-context interfaces with actual storage. No new tests,
standalone native funclet entries, original FH3/SEH ABI, binary replacement,
packet-native differential execution, caller adoption, or game validation is
claimed. The accepted raw helpers omit a zero-count host copy after reading
the same fields. No source/overflow/null validation or rollback is added.
