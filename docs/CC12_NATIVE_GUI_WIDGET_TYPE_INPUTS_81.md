# Explicit raw widget type inputs (packet 81)

The existing `AA2490` raw-key helper now has an additional, nondefault
`NativeGuiWidgetTypeKeyContext` overload. Callers must supply the actual empty
and underscore byte pointers, the same array of seventeen literal pointers,
and an explicit `int (__cdecl*)(const char*, const char*)` comparer. It supplies
no literal owner, global/pool, locale/error owner, implicit CRT provider or
factory. No production caller or normal native Text activation is added.

The legacy three-argument overload's body is byte-for-byte identical to baseline
`9bc71b4b10fdaa858c1352481c0fb1bee86ad1b7`, merged with exit 0 before work.
Its SHA-256 is
`16189bea32542c7cdb994246e15fef454a1f121a6dc9d1381eae21ba357f8da1`.
The remaining child-link helper bodies are also byte-identical. Their existing
bounded contracts and the separate logical loader classifier are unchanged.

## Input and call schedule

The context borrows a `const NativeGuiWidgetTypeLiteralViews&`, where that type
is `std::array<const char*, 17>`. It neither copies strings/header data nor
creates `std::string_view` snapshots. It holds the explicitly supplied comparer
pointer. Constructor preflight rejects null metadata with ordinary Source
`std::invalid_argument`; it does not scan strings or consult CRT/locale state.
Complete valid profile admission occurs before the raw operation. There is no
partial-profile or Original invalid-parameter/error-transport claim.

At entry, the new overload captures the actual key's `data()` once and uses the
required empty pointer if null. It then calls the existing whole raw Source
`reverse_find_native_string_bytes_004bcb80` on the **same key/header**, actual
underscore pointer and signed start `7FFFFFFF`. Unsigned `cut+1` preserves the
not-found wrap to zero. One captured suffix pointer is passed unchanged as the
left operand of each comparison. The right operand is the corresponding
borrowed literal pointer; IDs and comparison order are fixed by the body.
The first zero result returns immediately, otherwise all seventeen misses
return zero. It does not reload the key or replace any pointer with a copy.

The caller keeps the context, same array/pointer identities, readable terminated
literal bytes and valid key backing stable/live for the whole operation, with
externally serialized callbacks and locale/error-domain changes. A real
Original-compatible comparer remains an explicit caller/provider boundary.
Selecting the owning modern UCRT `_stricmp` is an honest Source substitution;
it does not prove Original BF7FBF locale/PTD/error ownership or non-C version
equivalence. Ordinary Source provider failures propagate. No new cleanup,
Original register/private-stack, FH3/SEH or binary replacement ABI is supplied.

## Original evidence and literal order

Verified BSP wrapper batches used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Whole `AA2490` is `[AA2490,AA265A)`, 458 bytes and
163 instructions, SHA-256
`6791bd5ad0db17abc38d5b4ec550396352321e20cef12e5ed5e73977b705cb23`.
All live bytes match the unchanged installed PE. Assembly establishes ECX as
the actual eight-byte key header, EDX unused, no stack arguments, EAX result,
RET. It captures `[ECX+4]` before real `004BCB80`, then retains ESI's suffix
through the seventeen cdecl `BF7FBF` calls. Library `__stricmp` remains named.
The live report verifier checks all eighteen Original direct CALL sites with
zero failures; this is separate from execution of the new Source overload.

The following operands and installed terminated-byte preimages were checked
against that whole live body. Empty is `F8BC60`, underscore is `CE7890`.
These addresses are evidence, not hardcoded/default Source pointers or proof
that this packet supplies their live production owner.

| Index | Literal | Original address | ID |
| ---: | --- | --- | ---: |
| 0 | Icon | `00CE8EC8` | 6 |
| 1 | Text | `00CEC778` | 3 |
| 2 | Group | `00D05680` | 2 |
| 3 | Progbar | `00D5BE04` | 5 |
| 4 | Line | `00D5BDFC` | 4 |
| 5 | Scrollbar | `00D5BDF0` | 7 |
| 6 | Grid | `00D5BDE8` | 8 |
| 7 | Model | `00D5BDE0` | 9 |
| 8 | Movie | `00CEBB64` | 10 |
| 9 | Listbox | `00D5BDD8` | 11 |
| 10 | AnimIcon | `00D5BDCC` | 12 |
| 11 | Curve | `00D5BDC4` | 13 |
| 12 | Sound | `00CEA3FC` | 14 |
| 13 | Button | `00D5BDBC` | 15 |
| 14 | ClipBox | `00D5BDB4` | 16 |
| 15 | Section | `00D5BDAC` | 17 |
| 16 | FrameBox | `00D5BDA0` | 18 |

## Focused proof and limits

MSVC Win32 `/MD /O2 /W4 /WX` compiles the fresh production object. The canonical
`scripts/build.ps1` exits 0 and all three existing CTests pass. The existing
unrelated `spawn_request_id_matches` duplicate-symbol LNK4006 warning remains.
No CMake edits or persistent tests were added.

One private manifested Win32 probe links that production object with the real
canonical libraries. A genuine `NativeString` uses existing
`assign_0041e870`/`crt_string_storage`, then real raw Source `004BCB80`. Its
explicit Source literal arrays stay live; they do not claim Original fixed-VA
identity. A transparent observer checks each left/right pointer and call index,
then returns the real owning UCRT `::_stricmp` result. This is the exact narrow
function pointer returned by packet 61's explicit Source adapter, without
linking a game-host graph merely to obtain it. No comparison result is faked.

For `root_section_gRoUp`, it observes exactly Icon, Text, Group, the same captured
suffix and same supplied right pointers, returns ID 2, and leaves the key header
unchanged. The fixed remaining IDs/order and null/not-found gates are inspected;
no additional dynamic cases or new Native differential are claimed. No locale
is changed, cached or mirrored by production or probe. Existing legacy Native
comparison evidence is not transferred to this new injected-comparer overload.

Only established `AA2490` receives appended evidence through the annotation
write lock, prior-value history, save and refreshed export. Reconstruction
metadata preserves the legacy record/status and adds a scoped packet-81 field.
All unrelated rows remain unchanged. This closes an explicit Source input seam;
actual Original literal/comparer owners, all-type factories, layout/lifetime
admission and game validation remain separate work.
