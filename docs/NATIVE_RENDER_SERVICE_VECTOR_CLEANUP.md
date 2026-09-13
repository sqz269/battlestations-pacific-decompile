# Actual render-service vector cleanup

Addresses: 00b14500, 00b14590, 004324a0, 00432050

Four complete native function entries cover 195 original bytes, including the
five-byte B14590 forwarding thunk. The new C++ interfaces consume actual raw
storage and the application's existing `NativeStringRawPoolContext`.
Descriptive names are hypotheses. They are not original binary entry points.

The 6ACh service allocated at 73DEDF/73DEE4 and constructed by B14A10 embeds a
12Ch renderer-record vector at +68C and an eight-byte string-header vector at
+69C. Both layouts have an untouched prefix at +0 and begin/end/capacity at
+4/+8/+C. B14C35..B14C53 initialize only those six pointer fields. Constructor
EH states 2/3 consume B14590 and 4324A0 through CBC3A6/CBC3B4; B14F60 consumes
B14500 and the string range during normal destruction. This packet does not
implement either full service constructor or destructor.

| Native entry | Original interface and behavior |
| --- | --- |
| B14500..B14548 | ECX vector, no stack arguments, RET, no semantic result. Capture begin and end, destroy ascending 12Ch records through complete B10740, reread current begin for CRT free, then zero +4/+8/+C in order. Null begin skips end/record reads and still zeros the fields. |
| B14590..B14594 | JMP B14500, preserving its native interface. The new C++ wrapper forwards the borrowed context as well. |
| 4324A0..4324DC | ECX vector, no stack arguments, RET. Capture begin/end, destroy the raw string range, reread current begin before free, then zero the three fields. Prefix stays unchanged. |
| 432050..432087 | ECX begin, EDX end, two unused DWORD stack arguments, RET8. Ascending eight-byte headers until pointer equality. Capture data first; null skips length/getter. Capture current length+1 with DWORD wrap before current 419CC0 lookup and BD1510 return. |

The loops retain DWORD address wrapping and equality termination. No extra
owner checks, bounds policy, pointer resolver or allocator abstraction is
introduced. Exceptions from the actual string-pool getter escape. A failed
element/range destruction prevents subsequent vector free and clearing;
B10740 retains its existing first-header cleanup when second-header release
throws. A returning invalid-parameter handler may change storage observed by
later reads. Captured iteration endpoints and the freshly loaded free pointer
are deliberately different values in that case.

Existing complete providers are `destroy_native_renderer_record_00b10740`,
the raw overload of `destroy_native_string_header_0041dd20`, actual
`native_string_pool_get_or_create_00419cc0`, and
`return_native_string_pool_00bd1510`. Every nonnull string release performs
the getter, including large returns and disabled small returns. The context
borrows the one pool publication 01090AA8, gate 01090AA4 and manager
publication 01090AA0. `singleton_lifetime_free` calls the established CRT free
service. Original BF65AC/BF9DC8 `_free` names are retained.

The existing `destroy_global_config_name_range_00432050` fragment in
`global_config.cpp` uses semantic `NativeStringStorage&` and `noexcept`.
It remains unchanged, as does its
`STL_NativeStringRange_Destroy_00432050` name and prior evidence. This packet
adds the complete raw-domain composition separately.

Fresh complete listings include the previously omitted returning-free
instructions B1452D..B14530 (`ADD ESP,4; POP EDI`) and 4324C2..4324C4
(`ADD ESP,4`). The root integrator repaired and saved those analysis gaps in
`reports/native_service_vector_cleanup_ba_flow_repair.json`; this worker
performed read-only Ghidra queries. All four full live/disk spans match.

The generic caller inventory is broad: the recorded live query returns 188
4324A0 caller functions, all named `Unwind@`, and 57 432050 caller functions;
the snapshot's 432050 caller count is 64. The complete query outputs are
preserved with the fixture evidence. These counts have different query
provenance and do not establish every caller's ownership domain.

Worker validation and immutable replay inputs are recorded in
`reports/native_render_service_vector_cleanup.json`. The external fixture
composes complete original parent bytes with the established current-source
providers. Its private-process CRT import observation forwards to the actual
free implementation and is restored afterward. The real pool-registration
validation path supplies returning and throwing invalid-parameter handlers;
the fixture adds no success stub or replacement owner.

Original stack-entry/FH3 ABI, native private-stack aliases, unrestricted SEH,
concurrent mutation, all original descendant bodies, whole service lifetime
and gameplay remain unvalidated. The normal source is initially unregistered;
the integrator must register it, build the exact combined tree and replay
against that tree's three current libraries before integration acceptance.

## Worker validation

Strict MSVC Win32 source compilation passed. Eight native seed checks and the
baseline build passed both existing CTests; that baseline excludes this still
unregistered source. One focused fixture then linked the new source explicitly
with those three libraries: ten original/source pairs passed, comparing 36819
normalized state bytes and ten event DWORDs. All 195 original parent bytes
remain except seven declared rel32 operands. The fixture restores its private
CRT free import and pins code, fixture and library inputs before/after execution.
Final current-library-only replay follows root registration and integration.

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
