# One-file property library loader and enum retention

Packet `cc11_scene_library_loader`, 2026-10-06. Source baseline:
`a4e43b5d6`. Ghidra was read only, with `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` verified before analysis/export batches.

**Result:** `008F67B0` loads one named file. The native reopens an existing enum
table and retains the first case-insensitive symbol value. The host previously
appended a second table that lookups could not reach, while its parser overwrote
a repeated exact-spelling symbol. The bounded correction retains the first
table/value and adds new symbols to that table. It does not reconstruct the
whole loader, VFS file constructor, typed property parser or group registry.

## Native body, callers and ABI

| Routine | Coverage | Contract |
| --- | --- | --- |
| `008F67B0..008F6B97` | Complete normal body read, 1000 bytes; no whole native source port | One-file dispatch, registries, normal tokenizer deletion and stack cleanup |
| `008F2E40` | Partial source projection: duplicate branch `008F2E83..008F2EE4`; native range/reverse-map insertion and allocation remain outside the binding | An existing symbol returns before value insertion |
| `008F6FC0` | Complete supporting normal-body/caller read | Enumerate a folder/extension and consume its entry list |
| `008F7100` | Complete supporting read, 14 instructions | Enumerate .enums first, .props second |
| `008F31A0` | Complete supporting read | Enum-body reader; invokes symbol insertion |
| `0048D480`, `0048D4E0`, `004895B0`, `00489610` | Complete supporting reads | Case-insensitive symbol/table lookup, uppercase hash folding |
| `008D9CF0` | Supporting file/VFS and allocation-path read; no new source binding | File tokenizer provider and buffer ownership |
| `008F5A00`, `008F54F0` | Supporting group-resolver/merge branches; other existing parser arms remain partial | Eager parent lookup and later-parent overwrite |

The recovered name `CPropTreeLibrary::Load` comes from the literal at
`00D1653C`, copied at `008F6812`. This literal is evidence of the name,
not proof of emitted logging. The sole caller is `008F6FC0`:
`008F708F` pushes an entry's C-string pointer, `008F7090` restores ECX to
the library, and `008F7092` calls the loader. A null entry pointer becomes
`00F89450`, the shared empty string. `008F7097` overwrites EAX without
consuming a loader status.

At entry, `008F67CD` loads the pathname stack slot into EBP and
`008F67D6` saves ECX as the library receiver. The terminal
`008F6B95 RET 4` establishes `__thiscall` with one stack slot. A semantic
return value and formal C++ parameter types are not established. Path content
is used as a C string; const qualification is not recovered.

`008F68A7` allocates 838h bytes. `008F68C4..008F68DA` supplies a by-value
native pathname string and delimiter literal `00D16534` (`;{}=:()`) to
`008D9CF0`. The outer loop peeks at `008F6906`, exits for tokenizer
`+80Ah` or unquoted empty token, and compares the two top-level keywords.
Padding `008F68FD..008F68FF` is `8D 49 00` (LEA ECX,[ECX]); no flow repair
was needed. Normal exit dispatches tokenizer slot 0 with flag 1 at
`008F6B7B`. Its vtable `00D15FE4` contains `008D9F00`, the existing
scalar-delete body, which destroys its owned state and frees the allocation.

Directory ownership is outside this function. `008F701B -> 00886280`
enumerates with the native caller's flag 1; `008F6FC0` pops each result and
cleans the string/list storage. `008F710E` uses `.enums`, then
`008F711B` uses `.props`. There is no sorting call in that wrapper.
The source host's enumeration fragment, flag 0, sorting and fallback paths
remain host policy rather than a reconstruction of these provider calls.

## Registries, duplicates and parent groups

The enum branch uses global `00E1867C`. `008F697C -> 008F2C10` tests
the table name. A hit resolves its existing object at
`008F699E -> 0048E960`; a miss constructs a 19Ch table through
`008F69C9 -> 008F4DD0` and stores its name at +11Ch. Both paths call
`008F69FF -> 008F31A0`. Only a new table is registered at
`008F6A18 -> 008F2B90`. Existing symbols/tables are not cleared.

The enum reader consumes a symbol, expects '=', reads its integer at
`008F31FF -> 008D9AD0`, handles an optional ';', then calls
`008F322C -> 008F2E40`. At `008F2E83`, that insertion function looks up
the symbol. A hit takes `008F2EB4..008F2EE4`, returning before insertion.
It calls `004254B0` with the duplicate marker format, but that actual callee
is an empty body: no native warning output is claimed. Lookup bodies compare
equal-length names through `00438E10`; their hash helpers use `toupper`.
The bounded source helper therefore uses the host's existing case-insensitive
comparison and preserves the first spelling/value.

For `008F2E40`, `008F2E60` saves the ECX enum receiver; the caller at
`008F3224..008F322C` pushes the integer-read result and symbol C-string.
The duplicate branch's `008F2EE4 RET 8` establishes two stack slots and
`__thiscall`. The caller ignores its return. The helper's C++ table/string
interface does not replace that ABI or establish the native formal types.

The property branch uses global `00E18678`. `008F6A69 -> 008F2DB0`
tests presence; `008F6A7F -> 00469B60` resolves an existing bag, otherwise
a 114h bag is allocated. At `008F6AD7`, ECX is that bag and stack arguments
are tokenizer, flag 0, and the saved library receiver as group resolver.
Only a new bag is registered at `008F6B42 -> 008F29A0`, with ECX=library+4h.

Parents are resolved during loading. `008F5AA9` gets each named parent and
`008F5AB3` merges it with flag 0: later parent fields overwrite earlier ones,
then authored fields overwrite the result. Installed parent lists can contain
multiple names. This differs from the host's lazy base traversal and first-base
retention; that broader projection is unchanged. Ordinary scene entities use
flag 1 at `0046D2F4..0046D30C`, so listed groups fill missing fields before
authored writes. The earlier JM06 empty-bag/default verdict remains valid.

## Error, reset and VFS boundaries

There is no whole-library reset or transaction in the loader. Existing bags
are mutated directly; new groups/tables are published after their parser calls.
The loader does not test a parser success result or roll back partial work.
Unexpected top-level tokens branch at `008F6A35` back to the cached peek
without consumption. The host consumes and skips them instead; that malformed
input recovery remains explicitly unreconstructed. No invalid native input was run.

The file tokenizer calls provider `[0109CEEC].vt[4]` at `008D9DCF` with
mode 32h. It owns the returned stream, read buffer, pathname label and delimiter
storage until deletion. It reads size via slot 30h and bytes via slot 24h.
The null-open branch does not initialize the buffer/size/cursor fields set only
by a successful stream. The loader nevertheless peeks unconditionally.
Graceful missing-file behavior, allocation failure, exception unwind and rollback
are therefore not claimed. The host's mode-2 memory opener, initialization
guards, parse-error collection and continue-on-failure policy are separate
source projections. A faithful one-file port needs actual provider/file-tokenizer,
registry and typed-parser bindings; no failure stubs were added.

## Installed inputs, change and verification

The read-only installed declaration scan covers fifteen files under
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/universe/library`:
one .enums and fourteen .props, 33 enum tables, 22 property groups and 1522
unique case-insensitive enum symbols. The report pins every file's byte count
and SHA-256. `global.enums` has hash
`A9F99C0E8FC650E2EF86E404990D56C7F2E4B518FF65BE685ACC3C6999ACE027`.
Its VehicleClasses Otenjo rows 48/324 and ShipClasses rows 381/582 all assign
349. There are no repeated table or group names in these inputs; those repeated
symbols keep the same effective value before and after this correction.
`global.enums` also defines Common/MultiEntity property groups, so extension
order supplies parent groups as well as enum tables.

Only `src/game_hosts_scene_contents.cpp` changes behavior. Its private
`insert_library_enum_symbol` is used by both the parser and reopened-table
registration; `PropertyLibrary::add_enum` adds new symbols to the first matching
table. The sole production registration caller is `parse_library_file`.
No public API, tracked test, mode, Hidden, class8 or other switch changes.

The ignored focused probe includes the actual production translation unit.
Strict MSVC Win32 /W4 /WX compilation and execution both exited 0:
`tables=1 symbols=2 original=7 added=8 unqualified=7 retained=1`.
The case inserts Keep=7, attempts KEEP=99, reopens the differently cased table
with keep=101 and Added=8, then checks scoped/unscoped lookups and one-table
count. It links 74 existing primary game objects (excluding the entry point
and scene TU), plus existing core/Lua/zlib libraries. No providers were stubbed.
PE machine is 14Ch, with an embedded asInvoker manifest. Logs and executable
hashes are in the report. Native direct-call verification and diff checks pass.

This is a source fixture and compile check for enum retention, not an original
game run, native binary ABI replacement or whole loader closure. No full build
or gameplay run was performed; primary owns integration verification.
