# Conditional enum-table CString wrappers

Packet `cc12_enum_table_CString_wrappers` adds the complete ordinary normal
composition of two CString wrappers over the already admitted enum APIs.
These new C++ interfaces do not bind canonical enum-global startup, Original
class/stack/register/FS/EH transport or the whole `008F67B0` loader.

| Original body | New Source entry | Ordinary composition |
| --- | --- | --- |
| `[008F2B90,008F2C0B)`, 123 bytes, 36 instructions | `insert_native_cenum_table_name_008f2b90` | Construct a genuine key, call conditional `008F17E0` on owner+4, ignore its count, return key storage |
| `[008F2C10,008F2C95)`, 133 bytes, 41 instructions | `contains_native_enum_table_name_008f2c10` | Construct key, find through whole `0048D4E0` on owner+4, capture node presence before key return |

The insertion Original receives ECX owner and two stack slots, ending `RET8`;
it defines no retained ordinary return. Its lower count is not preserved through
cleanup. Membership receives ECX owner and one CString stack slot, ending `RET4`;
only AL is Boolean. A missing key returns false. Membership never reads mapped+8.

The lower APIs require `const NativeString&`, so these wrappers construct a real
local `NativeString`. Compile-time checks establish its eight-byte, four-aligned,
standard-layout storage, nothrow default construction and trivial destruction.
The complete raw `construct_native_string_header_0041e870` fills its actual
header; no cast fabricates an object over raw bytes. Its source default member
initialization writes isolated local zeros before the raw constructor's own
initialization. That additional local initialization is not a claim of literal
Original store-for-store or code identity. After the lower call, manual
`destroy_native_string_header_0041dd20` uses the same concrete process's raw
context, including resolve-before-return and the current pool return gate.
The trivial C++ destructor adds no release, header repair or RAII unwind claim.

`NativeEnumTableWrapperContext` borrows explicit actual initialized E175E8 table
and E17578 symbol pools on the same allocator list/current generation. It retains
the same concrete `GameNativeStringProcess` for raw strings and query-empty
E17654. The genuine stable enum-empty E186ED authority is supplied explicitly:
this packet adds no process accessor for it, default cell or static owner.
Never substitute property E17540/E175B0 pools, E18560 or114h property bags.

Insertion inherits the whole lower API's conditional domain: old and incoming
mapped values are null or disjoint genuine current-CRT19Ch CEnums produced by
`008F4DD0`, D16508 with scalar `008F59C0`. Replacement is not restricted to
null-only data. Retain coherent genuine10Ch owner/map+4, canonical owning keys,
fresh disjoint key copies and matching live heap/pool generations. Admit stable
closed ASCII/current C locale and successful providers, without alias, reentry,
trim, unrelated mutation, overflow, faults or unwind. Membership only reads the
map and does not access/start the context's table/symbol pools. These borrowed
bindings supply no concrete enum-global initialization/publication proof.

Read-only live `bsp.py ghidra proto/bytes` calls verified configured project
`bsp` / `/battlestationspacific.exe`, using `C:/Users/sqz269/bsp.gpr`.
Both complete live ranges match the installed PE and their stated instruction
counts/endpoints. Raw bodies, all instructions, provider/header pins and original
tool receipts are retained in `local/WholeEnumTableWrappersSource01`.

The first strict MSVC Win32 `/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`
compile passed. The3948-byte object has all eight code sections: two wrappers,
the genuine `NativeString` constructor and five accessors, totaling247 bytes and
100 instructions. All eight code relocations and six `debug$F` metadata records
were inspected. Membership tests at+4E, captures BL at+57 and cleans the key
at+5B. Insertion calls the lower writer at+4F, overwrites its count register with
the key address at+54, then cleans at+59. All emitted helper bytes are included.

Original worktree build `58dc81/session96687` completed naturally at
`28e5bd/exit0`; `reconstructed_math` and `tool_tests` passed2/2 in10.93 seconds.
The worktree has no `local/seed_reference.hpp`, so the native differential test
is absent from its configured graph. The packet report pins that actual graph,
log and original tool receipts; no further build/test invocation was made.
The new TU remains unregistered in CMake,
so strict compilation proves its compiler artifact while the repository build
checks the existing graph. No new tests, probes or runtime execution were added.
Root owns independent review, registration, integration and admission.
