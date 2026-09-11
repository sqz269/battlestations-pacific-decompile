# Keyboard paired-axis selection

`006aa090..006aa1bf` selects alternate slots for an input only when its name
matches the **second** name of an `AxisPairs` entry and both named inputs have
the same ordered action-code description. `0069e860..0069e8f5` performs the
code-vector comparison. Both now have concrete host implementations in
`include/bsp/keyboard_axis_pairs.hpp` and `src/keyboard_axis_pairs.cpp`.
Names proposed here are hypotheses, not recovered symbols.

## Native association and behavior

The native device record has distinct containers:

| Offset | Association | Existing host field |
| --- | --- | --- |
| `+00h` | input name to ordered vector of int32 action codes | table-form `DeviceSettings::inputs` rows |
| `+0ch` | input name to vector of 14h binding descriptions | `runtime_bindings` |
| `+5ch` | vector of vectors of two pooled input-name strings | `axis_pairs` |
| `+6ch` | input name to independent reverse flags | `runtime_reverse` |

The loader calls `006a44b0` for the `+00h` map and appends input codes at
`006a8117`. Case-insensitively duplicate table-form rows therefore concatenate
their codes. Bare string rows belong to the input-order names and create no
code-description map entry. The reconstructed predicate assembles that same
description map view from the existing loader's row representation.

`006aa09a..006aa0a2` calls `0055c110` with `settings+8`, obtaining the device
by map subscript. `006aa0a2..006aa0de` walks the device's outer pair vector.
Each outer element is a 10h checked vector; each inner element is an 8-byte
pooled string. Native checks require at least two strings. The host `AxisPair`
already encodes exactly those two names.

`006aa0f7..006aa13a` compares the query with **inner element 1**, first requiring
equal stored string lengths and then calling `__stricmp` for nonempty strings.
Matching an entry's first name alone does not select alternate slots. Empty
names compare equal. Nonmatching entries perform no description lookups.

For a matching entry, `006aa171..006aa184` looks up the second name first,
then the first name through `006a44b0`, with the device pointer itself in ECX
(the description map begins at device offset zero). `006aa189..006aa191`
places the first vector pointer in ECX and the second in EDX before calling
`0069e860`. A true result returns immediately; a false result continues to
later entries, including another pair with the same second name.

`0069e860` computes both vector lengths using `(end-begin)>>2`, treating null
begins as zero-length. It rejects unequal lengths before calling the existing
library instantiation `STL_inst_0069dd80`. That helper walks by four bytes and
uses integer `MOV`/`CMP` at `0069ddc2..0069ddce`. Thus order, duplicates, every
32-bit code bit, and length all matter; two empty vectors compare equal.
The implementation uses length equality and `std::equal` on `int32_t` vectors.

Bindings, device types/indices, key numbers, slider flags, unknown binding+8,
reverse flags, and sensitivity floats do not participate. These routines
contain no floating-point instructions, x87 values, float conversions, NaN
handling, or rounding boundaries. An int32 code whose bits happen to encode
a float NaN is still compared as an integer. The existing runtime apply uses
the result to route binding installs to slots 2/3; that subsequent binding
and scale behavior remains in `keyboard_restore.cpp`.

## Mutation, aliasing, and host representation

Both lookups use native map subscript semantics. `0055c15d..0055c1be` inserts a
default device on a miss. `006a44fe..006a4533` constructs/inserts an empty code
vector on a missing input. Assembly confirms the function continues after
temporary frees at `006a4543` and `006a457f`; the saved decompiler incorrectly
shows early returns at those calls. The final mapped-value pointer is node+14h
at `006a459e`, followed by `RET 4`.

The host predicate therefore takes mutable `InputSettings&`; the corresponding
`KeyboardRuntimeHost` method now has the same mutable argument. A missing
device uses `settings.devices[device_name]`. A missing description appends one
empty table-form `InputEntry` to represent the newly created native map key.
It does not append an input-order label or create runtime bindings/reverse
flags. It preserves second-before-first insertion order. Equal pair names
reuse one description; a bare name does not suppress creation of its table
description. A later invocation observes the inserted empty row.

Native map insertions retain existing node references. The host row vector
can move on insertion, so the implementation copies the input query before
mutation and compares copied code lists. Query arguments may alias stored
names, and the primitive comparison permits identical vector references.

The existing `CaseInsensitiveLess` models ASCII folding. This packet uses
that shared convention for both map keys and equal-length pair-name checks;
non-ASCII locale behavior and embedded-NUL names are outside the established
loader/model contract. Native malformed checked-vector layouts abort via
`00bf6713`; the typed host pair cannot represent those corrupt layouts.
The predicate does not reproduce allocation failure details or native tree
storage. `apply_keyboard_setup_006aa640` takes a description-map snapshot before
calling its host predicate; traversal after inserting previously absent pair
names is not established for malformed/incomplete binding settings.

## ABI and evidence

| Address | Body | Original calling convention and result | Host symbol |
| --- | --- | --- | --- |
| `006aa090` | through `006aa1bf` | ECX=settings; stack device-name*, input-name*; result AL; `RET 8` | `use_alternate_axis_slots_006aa090` |
| `0069e860` | through `0069e8f5` | ECX=first vector*, EDX=second vector*; EAX=0/1; `RET` | `keyboard_input_codes_equal_0069e860` |

The decompiler omits register-carried arguments around `006aa191`; assembly
establishes their ordering. `0069e860`'s uninitialized upper temporary bytes
are ignored by the comparison instantiation and do not enter its dword loop.
The native error checks, storage layout, and calling conventions remain
documented evidence, not claims of a drop-in ABI replacement.

All live reads and exports used `tools/bsp.py ghidra`, whose `Client.verify()`
checks project `bsp`, `/battlestationspacific.exe`, x86 language, and image base
against `config/target.json`; project file is `C:/Users/sqz269/bsp.gpr`. Relevant
saved exports are `006aa090`, `0069e860`, `0069dd80`, `006a44b0`, and `0055c110`.
No missing function boundaries were found. The worker made no Ghidra writes;
the integrator owns locked annotations, preserving existing names/comments,
saving the project, and refreshing affected exports.

## Validation

- Eight existing seed byte ranges matched the installed executable.
- `./scripts/build.ps1` passed MSVC Win32 Release compilation and both existing
  CTests (`reconstructed_math`, `native_math_differential`).
- Three additional complete native body ranges were independently compared
  with the installed executable: `006aa090` (304 bytes), `0069e860` (150 bytes),
  and `0069dd80` (94 bytes). All matched saved Ghidra memory.
- One ignored fixture `local/keyboard_axis_pairs_fixture.cpp` passed 193 checks.
  It executes relocated original comparison bytes, including the original
  `0069dd80` loop, for 121 vector-pair combinations and 11 same-vector aliases.
  Cases include empty/unequal lengths, ordered mismatch, duplicates, signed
  extremes, and an integer with a NaN-like bit pattern.
- The same fixture executes the original `006aa090` body for eight predicate
  cases, using explicit typed map-lookup adapters for `0055c110`/`006a44b0`,
  host `_stricmp`, and a terminating checked-container error import. It checks
  direction, case-insensitive duplicates, fallthrough to later pairs, missing
  keys, equal empty descriptions, self pairs, empty names, and default device
  insertion. Host-only assertions cover query aliasing during row reallocation.

Logs are `local/keyboard_axis_pairs_build.log` and
`local/keyboard_axis_pairs_fixture.log`; exact range hashes are in
`reports/keyboard_axis_pairs.json`. No permanent tests were added. The fixture
does not execute original red-black-tree getters or the complete keyboard
runtime. No game process or installation was modified, and no game validation
or complete native ABI replacement is claimed.
