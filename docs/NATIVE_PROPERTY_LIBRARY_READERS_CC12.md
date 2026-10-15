# Property-library CString readers

Packet `cc12_property_library_CString_readers` reconstructs the complete ordinary
normal bodies of `008F2DB0` and `00469B60` through new MSVC Win32 interfaces.
It adds no loader, startup, map ownership, Original class ABI or game admission.
The accepted raw lookup and raw-string providers are reused without an ancestry
reproof. The canonical library node pool is not started merely to read a genuine
already-live map.

| Original body | New Source entry | Ordinary behavior |
| --- | --- | --- |
| `[008F2DB0,008F2E35)`, 133 bytes, 41 instructions | `contains_native_property_library_name_008f2db0` | Capture whether lookup returned a node before returning the temporary raw key; never read mapped+8 |
| `[00469B60,00469BE1)`, 129 bytes, 38 instructions | `read_native_property_library_word_00469b60` | Physically load node+8 before key return, retain the complete DWORD through cleanup, then return it |

Both Original bodies preserve ECX as the actual10Ch receiver, accept one nonnull
CString stack slot and end with `RET4`. Membership defines Boolean AL only.
The value reader supplies full EAX and has no missing-node branch. Its supported
ordinary domain therefore requires a found key; a found mapped DWORD may be zero.
The source value reader adds no miss guard or fabricated default.

Both allocate actual aligned eight-byte raw stack storage. They call complete
`construct_native_string_header_0041e870`, whose own first stores zero the header,
then find in the actual108h map at receiver+4 through complete `00468ED0` with a
separate writable bucket. No `NativeString` facade or assumed caller initialization
is used. After capturing the result, normal manual cleanup calls the accepted
raw-context `destroy_native_string_header_0041dd20` composition. For nonnull data
it captures length+1 and resolves `00419CC0` before `00BD1510`, including the live
return gate; null data skips the length read and pool return.

`NativePropertyLibraryReaderContext` retains the same concrete
`GameNativeStringProcess` supplying raw allocation/return and the distinct
`library_empty_00e18560()` and `query_empty_00e17654()` roles. It initializes no
owner and accepts no arbitrary fallback, callback or model host. Keep genuine
coherent raw owners, live14h nodes, canonical pooled keys, the same raw-string
generation and process bindings stable through lookup and cleanup. Admit closed
ASCII keys/current C locale, successful providers and nonaliasing storage, with no
mutation, reentry, overflow, invalid memory, provider throw/fault or unwind.
Manual normal cleanup does not claim Original exception/FS/FH3 transport.

Live `bsp.py ghidra proto/bytes` queries verified the configured `bsp` project at
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Both complete live byte
ranges match the installed PE bytes and decode to their stated boundaries/counts.
Raw bodies, all decoded instructions, tool receipts, source/dependency pins and
the selected compiler object are under ignored
`local/WholePropertyLibraryReadersSource01`.

Strict `/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict` compilation passed.
The 3040-byte object contains all four emitted code sections: two reader bodies
(122/133 bytes) and two complete accessor COMDATs (4/3 bytes), totaling262 bytes
and111 instructions. All14 code relocations and both `debug$F` metadata
relocations were inspected. Membership tests the returned node at+52, captures
BL at+5B and calls key cleanup at+5F. The value body physically loads `[EAX+8]`
at+5B, saves it at+5E, calls cleanup at+69 and restores the DWORD at+74. No value
miss guard is emitted. Current compiler stack-cookie transport is not Original
EH/ABI proof. The first strict attempt's reserved MASM local-name error was fixed;
both compiler receipts and the failed diagnostic are retained.

The original repository build `d61497/session5724` completed naturally at
`f3a43c/exit0`; both configured tests (`reconstructed_math`, `tool_tests`) passed
in7.28 seconds. This worktree has no `local/seed_reference.hpp`, so CMake's
conditional native differential target is absent. No additional check was run.
The result and exact configured graph are pinned in the packet report. This TU remains
unregistered in CMake, so that build validates the existing graph; the strict
object separately establishes compilation of these readers. No new tests, probes
or runtime execution were added. Root owns registration, independent review,
integration and admission. `008F67B0` remains a separate incomplete loader.
