# Text cleanup return progress

Source preparation over pinned OFF baseline `677c3024f`, preserving plan3cb.
This packet adds independent completion diagnostics to the existing AB8250 /
AB8EE0 retirement record. No new Native body credit, header ownership domain,
cached-shader credit, raw producer, activation, build or runtime claim is added.

`CODEX_GUI_RAW_TEXT_CONTENT_RETIREMENT_PLAN.md` remains a planning record. Its
proposed owned-members enum/scalar admission is **not** implemented: no truthful
raw owned-header acquisition/publication receipt exists yet. Generic raw string
helpers do not provide one; AB8C30's retained field acquisition accepts the
legacy reference and exports a Font borrow. Cached1EC additionally needs a real
raw producer/credit transfer. All current DEFAULT, initial93/122, private widget
and production-owner admission stays unchanged.

## Record and exact write boundary

`NativeGuiTextRetirementAcquired::unwind_actions_returned` is a zero-initialized
independent DWORD mask, using the SAME state/action bits as the existing unwind
consumption record. The existing unwind loop decrements Native EH state and
sets consumed/disarmed BEFORE calling its selected helper. It now sets the
returned bit AFTER that helper returns normally, including a null-data helper.
The new write touches only the retained diagnostics; it reads no native payload.

| Bit/state | Existing cleanup call site | Selected genuine helper/member |
| --- | --- | --- |
| 6 | CB7EE7 | 41DD20, narrow header at1C8 |
| 5 | CB7ED9 | 41DD20, narrow header at1C0 |
| 4 | CB7ECB | 436430, wide header at1A4 |
| 3 | CB7EBD | 5D6CF0, pointer-vector header at194 |
| 2 | CB7EAF | 41DD20, narrow header atF4 |
| 1 | CB7EA1 | 436430, wide header atEC |
| 0 | CB7E93 | AA9730, base widget destruction |

For the normal AB8250 path, existing `normal_actions_returned` remains the
completion authority for each action, including null-data skips. For a nonnull
normal string, its existing per-member `returned` flag is set only after the
actual BD1510 call returns; captured data alone, getter entry or an entered
return is not success. Normal return-member DATA-before-LENGTH capture, EH
disarm, CURRENT actual pool/disabled-cell use and native size arithmetic are
unchanged. Headers are not zeroed after return and must not be read as fresh
ownership afterward.

An unwind consumed/disarmed bit proves only that the action was consumed before
entry. Its returned bit proves the selected helper returned, not that a nonnull
buffer existed or that a physical pool return occurred. If a selected helper
escapes, the new bit is absent; previously returned bits remain. Existing raw
host-metadata rejection and escaped-cleanup termination behavior is unchanged.
No mask alone proves whole Text/Model retirement, flags1 pool return, allocation
provenance or permission to replay/reuse a record. Keep the independent record
and all providers through the existing sticky failure disposition.

## Source changes and lifetime checks

- `include/bsp/native_gui_text_retirement.hpp`: the new zero-initialized mask
  beside existing normal/consumed progress, with explicit null-data semantics.
- `src/native_gui_text_retirement.cpp`: fresh admission rejects nonzero new
  progress; the only completion write is after the existing unwind switch.
  Every Native action, call, EH-state/disarm write and exception edge remains
  in its existing order.
- `src/native_gui_text_reference.cpp`: its separate scalar/destruction fresh
  predicate also rejects the new nonzero mask. Initial93/raw122 ownership and
  payload admission are unchanged. The existing known-borrowed region checks
  use `sizeof` context/frame/diagnostics, so their extents include the new field
  automatically; no region or provider is omitted or weakened.

Original-address evidence and admitted uncertainty remain in
`native_gui_text_retirement.hpp`'s AB8250/AB8EE0 contract and
`CODEX_GUI_TEXT_RAW_MODEL_TRANSPORT.md`. This is new Source diagnostics over
the already reconstructed cleanup bodies, not Native FH3/SEH/private-stack ABI.
The new field is C++ metadata outside Text pool storage and is never a native
field/publication, second owner, count or resource registry.

Validation here is bounded source/diff inspection and whitespace checks only.
No C++ build, existing CTest, fixture, game or Ghidra mutation was run for this
packet. Root owns later compilation and existing checks. Nonempty raw members,
main4C geometry/routing, Font borrowing and recursive glyph/Page activation
remain separate prerequisites; DEFAULT guards remain strict.
