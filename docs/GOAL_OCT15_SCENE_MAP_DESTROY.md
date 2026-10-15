# Actual scene-record map destruction

The three52-byte native routine bodies below are reconstructed by one complete
ordinary Source body, with separate address-bearing entry points. Assembly and
installed PE comparison establish equality after normalizing only the two
same-target relative CALL operands. No existing canonical provider is changed.

| Routine | Native range | Coverage | Original ABI |
| --- | --- | --- | --- |
| `0046C500` | `[0046C500,0046C534)` | complete | ECX actual tree; RET; no semantic result, EAX cleared0 |
| `0046CED0` | `[0046CED0,0046CF04)` | complete | ECX actual tree; RET; no semantic result, EAX cleared0 |
| `0046EF70` | `[0046EF70,0046EFA4)` | complete | ECX actual tree; RET; no semantic result, EAX cleared0 |

Each body captures current tree.head4, then that head's minimum0. It prepares
the actual five-word outgoing range arguments in native push-store order:
last-node=captured head, last-owner=tree, first-node=captured minimum,
first-owner=tree, then private output pointer. No head or minimum reread is
inserted between these stores. The output8B belongs to this destructor's private
storage; it is passed distinctly from the range provider's own private output8B
used by a reached partial erase.

`NativeSceneRecordMapDestroyFrame` binds the actual20B argument cells through
the accepted `NativeSceneRecordMapEraseRangeFrame`, plus that separate actual8B
destructor output. Stable metadata is disjoint from cells/payload. Explicit
payload aliases are allowed when every reached read/write/free remains valid;
no original physical private-frame coincidence is implied. Current values and
returnable diagnostics can make the range provider take a partial or empty path.
Source therefore calls the complete actual range body, rather than narrowing
the request to unconditional full clear. Its child metadata/backing retains the
accepted provider's reached-only requirements.

After the range operation returns, each native body rereads CURRENT tree.head4
and frees that address through `00BF65AC`, then writes head4=0 and count8=0 in
that order. It does not free the initially captured head instead, clear tree+0,
delete mapped objects, add a validation guard or introduce rollback. If the
range operation fails, the later free/zero stores are not reached. There is no
local EH registration, placement call or local cleanup action in these52B bodies.

The concrete dependencies are the accepted full raw `0046B570` range provider
and existing `singleton_lifetime_free` current-CRT service. Native `00BF65AC`
is the `_free` thunk to `00BF9DC8`; its correct library name remains intact.
Current CRT/private frame/register/FH3/SEH identity is separately qualified,
not implemented by a callback or no-op host.

| Containing routine | Range call | Free call | Cleanup |
| --- | --- | --- | --- |
| `0046C500` | `0046C516` -> `0046B570` | `0046C51F` -> `00BF65AC` | Range RET14; free caller ADD ESP,4 at `0046C524` |
| `0046CED0` | `0046CEE6` -> `0046B570` | `0046CEEF` -> `00BF65AC` | Range RET14; free caller ADD ESP,4 at `0046CEF4` |
| `0046EF70` | `0046EF86` -> `0046B570` | `0046EF8F` -> `00BF65AC` | Range RET14; free caller ADD ESP,4 at `0046EF94` |

Live xrefs found no references to the first two entries. `0046EF70` is reached
by tail transfers `00C62001` and `00C62081`; both load a saved object then add18h
for its embedded tree. The report carries typed caller rows without mutating or
claiming ownership of those unwind functions. Their original compiler frames
are not supplied implicitly by this new C++ API.

Live batches verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All156 original bytes are compared with installed
PE. Prior names/empty plate comments are preserved under the Ghidra write lock,
then the project is saved and affected exports refreshed. The worker removed
only its own queue after a fresh unleased check and appended the unique CMake
line, without an exclusive registry lease. Strict MSVC Win32 build and existing
CTests are pending in the initial draft.
No new routine tests, candidate original-byte differential, binary ABI or
gameplay validation are claimed.
