# Native ship model pointer binding

Addresses: `0082D700`, dependency `0071B2C0`, caller `0082FE58`.

## Reconstructed scope

| Entry | Native bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| `0082D700` | 210 | ECX actual ship class, no stack arguments, RET | complete ordinary body `0082D700-0082D7D1` |

`bind_native_ship_model_pointers_0082d700` implements the complete body using
raw Win32 DWORD storage. The class's borrowed model pointer is at `+50h`. The
source actual type54 vector header is at model `+54h`, with begin/end/capacity
end at relative `+4h/+8h/+Ch`. The destination header is class `+6BCh`, with
the same three relative fields at class `+6C0h/+6C4h/+6C8h`.

The source layout is backed by the existing classification producer:
`append_and_classify_native_game_item_0071bb40` appends matching borrowed item
pointers through `0071B940` into resource `+54h`. That evidence establishes a
raw type54 pointer list. The pointee type and its ownership policy remain
unidentified; this body does not retain or release an item.

## Exact iteration and append behavior

The routine captures the initial model's source-header address and begin
iterator. It validates that captured begin does not exceed captured end. Each
iteration then reloads class `+50h`, the current source begin and current end,
checks the current header, and requires its address to equal the captured
source header. The current end captured before those returning validations is
used for the loop-end comparison. The source bound is reloaded from the
captured header before dereference and again after the destination append.

The destination keeps all prior entries. With nonnull begin and spare unsigned
capacity, the routine reloads the current destination end, stores one raw DWORD
pointer there, and publishes end plus four. Otherwise it captures the current
end as the position, diagnoses captured begin greater than that position, and
calls the established `insert_native_game_type54_one_0071b2c0` with:

| Native input | Supplied value |
| --- | --- |
| ECX list | class `+6BCh` actual header |
| stack output iterator | local two-DWORD storage |
| stack iterator owner | the same class `+6BCh` header |
| stack position | the current destination end |
| stack value | address of the captured raw source pointer |

The callee's live body was read before reuse. It has `RET 10h`, reconstructs an
iterator around `0071A760`, and can invoke returning `00BF6713` validations.
Its three live call sites were enumerated and their argument setup inspected:
`0071B99C`, `0082D7AF`, and `0093915A`. They all pass output, owner, position,
and value in that four-stack-argument order; no widened or item-owning contract
was inferred from this caller.

## Call evidence

| Site | Target | Contract |
| --- | --- | --- |
| `0082D71B` | `00BF6713` | captured source begin/end validation; may return |
| `0082D730` | `00BF6713` | current source begin/end validation; may return |
| `0082D73B` | `00BF6713` | captured/current source-header identity; may return |
| `0082D751` | `00BF6713` | pre-dereference captured source bound; may return |
| `0082D79C` | `00BF6713` | captured destination begin/current-position validation; may return |
| `0082D7AF` | `0071B2C0` | actual type54 insert-one, four stack arguments, `RET 10h` |
| `0082D7BD` | `00BF6713` | post-append captured source bound; may return |
| `0082FE58` | `0082D700` | only live caller; ECX restored to the ship class at `0082FE56` |

The complete target listing establishes all register provenance: EBP holds the
ship class, stack `+10h` the captured source header, EBX the source iterator,
and ESI the destination header at the insertion site. No stack arguments enter
the function, and the body restores its four saved registers and returns with
plain `RET`. `0082FE58` is inside `0082FE30-00831801`.

## Validation and limits

The source has a new C++ ABI and composes the existing source implementation of
`0071B2C0` plus the supplied actual returning invalid-parameter callback. It
does not reproduce native fault delivery, private stack layout, or original
exception machinery. The strict MSVC Win32 Release `/W4 /WX` build passed. Its
two existing CTests, `reconstructed_math` and `tool_tests`, also passed. The
report verifier checked all eight call rows with no failure. These checks
exercise compilation, repository integration and call-site consistency only;
this packet adds no duplicate fixture and makes no native differential, full
ship-binder, executable, or gameplay claim.

The full `0082FE30` binder, model construction, resource lifetime, source item
type, and all model activation behavior remain external contracts.
