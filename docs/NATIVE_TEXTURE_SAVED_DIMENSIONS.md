# Actual texture saved dimensions

Addresses: 00b3ce70, 00b3ce80

The actual D61948 texture profile selects B3CE70 at slot48 and B3CE80 at
slot4C. These complete four-byte leaves return the raw DWORD at actual
owner+34 and owner+38. Their native interface is ECX owner, no stack arguments,
EAX result and plain RET. The new fastcall source entry uses the same input
and result registers without supplying an object layout or dispatch resolver.
Names are descriptive hypotheses, not recovered symbols.

The actual owner constructor B3F930 receives saved width and height in its
third and fourth native stack arguments. Its stores B3F9EA and B3F9F9 write
those values to +34/+38, after the earlier COM GetLevelDesc outputs were stored
at +28/+2C. The existing `native_texture_2d_owner` implementation preserves
those distinct fields. B3CE50/B3CE60 expose the descriptor fields and cannot
substitute for these saved-dimension getters.

B52550 consumes current texture slots48/4C for its three unsigned integer
division ratios and slot48 for a right shift. For each ratio it holds the first
result and reuses the captured texture receiver while rereading its vtable.
The parent still requires full loading, ownership and exception cleanup; these
two leaves do not establish that parent, other texture profiles or gameplay.

The integrator checked all eight original bytes against live Ghidra and the
installed PE, the current vtable cells, full producer argument setup and its
two stores. The functions were defined from those bytes in the existing BSP
project. Their source uses the same MOV/RET instructions. Compilation and
emitted-body comparison are recorded separately in the report; no new runtime
test is added for these two read-only leaves. General binary replacement,
invalid-pointer fault identity and concurrency remain outside this evidence.

## BA integration checkpoint

The integrator reviewed the complete native body and actual producer evidence,
saved its original signature and complete stored range in the existing BSP
project, and registered the source. Current combined validation follows
separately from the source or worker checks above. No complete owner lifetime,
original binary replacement or gameplay claim follows from this checkpoint.

## BA exact merged validation

The exact combined source commit `b852ae06a7fdd93c799cdacc015a1a5a96adf9f1` passed the strict Win32
build and both existing tests. Four current-library-only original-byte fixtures
cover the ambient, registry and two vector modules; the saved-dimension leaves
have exact complete emitted-byte checks in the built library, with no runtime
fixture added. See `reports/native_lighting_service_ba_validation.json` for
hashes, immutable captures, coverage and limits. Earlier pending statements
describe worker stages. Native ABI, full rendering and gameplay remain open.
