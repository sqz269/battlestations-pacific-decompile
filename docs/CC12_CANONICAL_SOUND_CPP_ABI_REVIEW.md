# Canonical Sound and GlobalConfig C++ ABI review

Independent review of the eight adopted Source files passed for the bounded
storage/control-flow change. **This changes the C++ binary ABI.** It supplies
no new Raw Original ABI, Original-function credit, or game validation.

The exact Source adoption is Root receipt SHA-256
`0d15771c308aa924cdaefe55873d4b2933502a3f748f231306c4706bed08e32e`.
Root performed one normal Win32 build; all three existing CTests passed. This
worker did not compile, link, run tests/targets, edit Source, or mutate Ghidra.
Root's sealed post-build pins are
`2fca2aa72c98d06db3aa45955fcf6240d39071f8754ee569bace5d61cbd76cfa`.

## Ownership and control flow

`GlobalConfigContext::lifetime` is now a borrowed `SoundLifetimeAccess` value.
The getter captures the first manager's section once, enters/increments it,
constructs and publishes the owner, resolves the second manager before reading
the registration argument, then decrements/leaves the captured first section.
Its final publication reload follows guard destruction. Both Source and emitted
COFF preserve that order. The unchanged lifetime-adapter object contains the
semantic branch and actual manager `+10h` / section `+18h` branch; no separate
manager/domain was introduced.

`SoundServices` takes `app.vfs_->borrow_raw_services().strings`. The returned
view contains a reference to the VFS application's retained
`NativeVfsOwnerServices` wrapper. Its temporary view does not make that wrapper
temporary. The compiled constructor reads view `+18h`, then copies the reference
into `GameSoundRuntimeServices +1Ch`. Runtime startup explicitly forwards it to
resource construction, Lua initialization, the public Lua fragment, category
factories, and both `NativeName` temporaries. Each name stores and releases
through the same supplied wrapper. Listener configuration already uses its
configured storage; on this application path it is that same wrapper.

VFS, Lua, and scripts precede Sound construction. Raw singleton drain runs while
those services remain alive; Sound is reset before singleton-host deletion and
VFS is deleted after Lua teardown. The existing failed-constructor recovery
retains the same shutdown/string services. These are static Source/COFF facts;
this packet does not add a live lifetime or gameplay test.

## C++ interfaces and compatibility

| Reconstructed function | Old/new x86 stack words | Emitted interface |
| --- | --- | --- |
| `get_global_config_00432650` | 1 / 1 | `__cdecl`, context reference on stack, owner pointer in EAX, plain RET |
| `destroy_global_config_004325b0` | 2 / 2 | `__cdecl`, owner/context references, void, plain RET |
| `scalar_delete_global_config_00432710` | 3 / 3 | `__cdecl`, owner/flags/context, original owner in EAX, plain RET |
| `construct_sound_system_00a88770` | 12 / 13 | `__cdecl`, owner reference in EAX, plain RET; runtime caller cleans `34h` |
| `initialize_sound_configuration_00a7ff80` | 5 / 6 | `__cdecl`, void, plain RET; caller cleans `18h` |
| `apply_sound_configuration_lua_00a7ff80_fragment` | 5 / 6 | `__cdecl`, void, plain RET; caller cleans `18h` |

All three Sound declarations now end in `NativeStringStorage&`, so their exact
decorated symbols change (full old/current names are in the report's indexed
interface artifact). The trailing defaults preserve inspected old-arity Source
calls **after recompilation**. They preserve neither old function-pointer types
nor old decorated exports, and create no compatibility overload or thunk.

GlobalConfig's free-function decorated names are unchanged, but its context is
now 20 bytes instead of 16: the two-pointer lifetime value occupies `0/4`, the
publication reference moves `4 -> 8`, strings `8 -> Ch`, and effects `Ch -> 10h`.
The emitted getter and destructor use those new offsets. An old context cannot
be passed safely merely because its function name still links. The actual
`GlobalConfigOwner` storage remains `2E8h`.

`GameSoundRuntime::startup` itself remains `thiscall(ECX=this, stack bool)` with
`RET4`; its internal call pushes the new thirteenth argument. `SoundServices`
construction remains `thiscall(ECX=this, stack app-reference)`, returns this in
EAX, and ends `RET4`. The local `NativeName` grows from 8 to 12 bytes; constructor
is now `thiscall` with text/storage stack arguments and `RET8`, and destructor
uses its saved storage rather than fetching CRT storage again.

The current Source census contains exactly one runtime-to-startup call, one
startup-to-initializer call, and one initializer-to-fragment call. All forward
storage explicitly; no additional function-address client surfaced. Actual
pre/post `CL.read.1.tlog` records identify the same **12** GlobalConfig header
consumer TUs. Every one appears as compiled in the sole normal-build log and has
a final object pin; `rebuild_coverage.json` records exact names, log lines, and
dependency-record lines. This includes indirect header consumers beyond the
obvious getter callers.

## Original interfaces remain separate

| Original entry | Recovered inputs | Return/cleanup evidence |
| --- | --- | --- |
| `00432650` | No consumed inputs | EAX current singleton; RET at `0043270F` |
| `004325B0` | ECX owner; no EDX/stack input | Void contract; RET at `0043264A` |
| `00432710` | ECX owner, stack flags | EAX original owner; RET4 at `0043272B` |
| `00A88770` | ECX owner, disabled byte plus two unused stack words | EAX owner; RET0Ch at `00A88AA6` |
| `00A7FF80` | ECX manager; no stack arguments | Void contract; RET at `00A8145B` |

The listings, rather than misleading decompiler fastcall labels, establish the
ECX/no-EDX interfaces. Lua numeric results are consumed from x87 ST0 (for example
`00A80164 FSTP [EBP+4]`); this change does not requalify the old numeric projection.
Eighteen exact native callsites passed the read-only report checker with zero
failures. No native function was retagged or registered by this audit.

## Object and data evidence

Six old and six current whole COFFs were retained and decoded: the five changed
translation units plus the unchanged lifetime adapter. Every selected relocation
uses its physical symbol-table index and record/operand offsets. Complete
in-object sections include cold continuations and unwind/data sections; a primary
symbol span is not silently treated as the complete reachable function. External
and virtual targets remain boundaries.

The current selected roots have respectively 21, 10, 42, 453, 189, and 633 indexed
in-object edges for GlobalConfig, lifetime, startup, configuration, runtime, and
GameHosts. Initialized data and uninitialized storage are distinguished: the
configuration path is a 21-byte `.rdata` object bound to symbol 1723; runtime
`callback_owner` is symbol 8 in the four-byte `.bss` section with raw offset zero.
GameHosts retains its 16-byte `.bss` section and its eight-byte initialized fade
constant. Whole sections and relocation bindings accompany those positive facts.

The final game link map positively names the new Sound startup, initializer, and
fragment providers. The GlobalConfig constructor/getter/destructor/scalar-delete
have **no final `bsp_game.map` entries**; this review gives them emitted COFF
evidence only and does not claim game linkage or execution. Root separately owns
the complete archive/member and link-input reconciliation.

The machine-readable report is
[`reports/cc12_canonical_sound_cpp_abi_review.json`](../reports/cc12_canonical_sound_cpp_abi_review.json).
Whole retained evidence is under this worktree's
`local/cc12_canonical_sound_cpp_abi_review/`; its manifest SHA-256 is
`a5447fdef31f0d6f5138b1b8099e21bb244e01e617fa2b89cb83d3eea5ec7c33`. All eight adopted Source hashes were rechecked before closure.
