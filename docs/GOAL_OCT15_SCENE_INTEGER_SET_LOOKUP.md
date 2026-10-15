# Actual scene integer-set lookup

Addresses: `00468DB0` only. Packet `goal_oct15_scene_integer_set_lookup`.

| Routine | Coverage | Native ABI | Ordinary Source |
| --- | --- | --- | --- |
| `STL_inst_00468db0` | Complete `00468DB0..00468E18`,105 bytes,39 instructions | ECX tree; EDX unused; stacked output8,key4; `RET 8`; EAX output | `find_native_scene_integer_set_00468db0` |

The existing library name is retained. Find and the scene integer-set family are
behavior-derived hypotheses rather than recovered C++ symbols or nominal types.
This is an independent actual-storage provider. It does not replace the existing
typed generation gate in `scene_entity_factory.cpp`.

## Readiness and layout

The live/indexed function had an STL tag and no reconstructed Source. Fresh live
xrefs identify four calls in two callers. There are no outgoing calls or native
diagnostics. The complete native listing was read because its exported void
pseudocode omits the output-pointer return consumed by callers; no unsupported
live calling-convention or return-type repair is applied.

The actual producer `004681B0` allocates14h bytes and writes links0/4/8, keyC,
color10 and nil11=0. Its accepted Source is `native_scene_integer_set_node.cpp`.
The accepted linker `0046A150` publishes the root through current tree+4 head,
then head+4. This independently confirms the nil11 family; the nil19 record-map
lookup is not used. No field declaration or nominal owner type is introduced.

## Complete read and write schedule

Read tree+4 head, head+4 root, and root+11 nil. If root is nil, do not read the key.
Otherwise load its signed DWORD once, compare each current node+C with that
captured search value, take right+8 when less, and otherwise retain that node and
take left+0. Test each newly selected child+11 before another key comparison.

After descent, reload tree+4. If candidate equals that current head, select the
head without another key read. Otherwise reread current key first, then read
candidate+C; select current head when key is signed-less. The native stages a
private owner/node pair; Source captures both words before ordered output+0
owner and output+4 node writes. This preserves valid aliases between supplied
tree/node/key/output storage. There is no output write before the final reads,
and no supplied-storage read after the first output write. Return output itself.

## All incoming transfers

| Site | Containing function | Target | Observed use |
| --- | --- | --- | --- |
| `0046B7B1` | `0046B730` | `00468DB0` | ECX owner+158h; push key pointer then separate output8; reads output cells directly |
| `0046B836` | `0046B730` | `00468DB0` | ECX owner+158h; push key pointer then output8; reads output cells directly |
| `0046BA8C` | `0046B730` | `00468DB0` | ECX owner+158h; push key pointer then output8; captures EAX and reads that output |
| `0046C741` | `0046C550` | `00468DB0` | ECX database+164h; push class-id cell pointer then output8; captures EAX and validates owner/node afterward |

All four use two stack words, reclaimed by the callee's `RET 8`. Diagnostics
following these calls belong to their callers. They are not part of this leaf.
Typed live rows establish CALL ownership and exact target; no native callee edge
is invented for the ordinary Source helpers.

## Evidence and qualification

The report records all105 original bytes and their installed-PE equality,
complete typed body metadata and all four incoming transfers. Guarded CLI
analysis/export batches verify project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86/image base00400000. Locked annotation records
prior names/comments, retains the correct library name and prior comments,
saves the project and refreshes the export; final receipt records readback.

Final validation: normal `./scripts/build.ps1` MSVC Win32 Release build
passed with this Source compiled and an x86 COFF object retained. Both enabled
existing checks (`reconstructed_math`, `tool_tests`) passed in8.59 seconds;
native math differential is not enabled in this worker cache. Primary
independent full-assembly/Source review passed with unchanged hashes. Unique
CMake registration used its queue turn and immediately released the short
registry lease. Four typed CALL rows passed with no failures; documentation
verification and diff checks passed. No new tests were added. No direct native
entry execution, original COFF equivalence, drop-in binary ABI, application
adoption or game validation is claimed. Native private argument/local aliases
and asynchronous mutation are outside this ordinary Source interface; supplied
storage aliases are retained throughout the reached valid-access domain.
