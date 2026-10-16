# Native scene string-list indexed insertion

Addresses: `0094F2F0`.

The complete ordinary successful body is reconstructed as a potentially throwing,
explicit-context C++ function. The new translation unit remains unregistered for
integrator review. Strict compilation and the existing repository build passed;
the insertion itself has not been executed.

| Entry | Original range | Coverage | New Source API |
|---|---|---|---|
| `0094F2F0` | `[0094F2F0,0094F39F)` | complete normal body, 175 bytes / 61 instructions | `insert_native_scene_string_list_indexed_0094f2f0` |

The saved Original bytes have SHA-256
`787b9a3800948a0295a991348590d05e32ed940e7bbb308659ce27da45216f78`.
The body includes an allocator-null branch and private exception registration;
their failure outcomes are outside the admitted successful domain.

## Entry and result

Original ECX contains the actual twelve-byte list. Entry stack offsets `+4`,
`+8`, and `+0C` contain the actual record pointer, signed index, and DWORD flags.
Only the low flag byte is tested. Normal exit uses `RET 0C`; EBX/ESI/EDI are
preserved, EBP is untouched, and private FS0 registration is removed.
Measured final EAX is the captured clamped insertion index.

The C++ API accepts those four values plus `NativeStringRawPoolContext&` and
returns `std::int32_t`. This is a **new Source exposure** of measured EAX.
Neither Original language-level return type nor caller meaning is recovered;
the ordinary C++ signature is not a physical native entry replacement.

## Normal access and ownership schedule

1. Read count at raw12 `+4`, then capacity at `+8`. Grow only on equality,
   before inspecting flags or reaching the copy providers.
2. Low byte zero allocates exactly `{object,8,8}` through the existing singleton
   allocator, then uses the actual raw-header copy overload with the supplied
   retained raw context. Every one of the 255 other low-byte values adopts the
   supplied pointer, including null. Upper 24 bits are ignored.
3. Only after the reached providers, interpret the index as signed. Negative
   becomes zero and skips the upper-bound count read. Otherwise read current
   signed count and clamp an above-count index to that value.
4. Independently reload current signed count into the descending shift cursor.
   Each iteration reloads slots at `+0`, captures `slots[cursor-1]` and the exact
   destination `slots[cursor]`, decrements the cursor, and stores the captured
   pointer at that destination. Continue using signed comparison with the
   captured index. No per-iteration count read, cached loop base, bulk move,
   record copy, or record destruction is introduced.
5. Reload slots again, publish the selected pointer at the captured index, then
   read/add/write current count. Return the captured index without a return-count
   reload. Existing owned header/data identities and order survive the shifts;
   transient duplicate slot values are not independently retired.

These are the three measured Original direct call instructions:

```text
0094F312 CALL 0094F010
0094F326 CALL 00BF681B
0094F345 CALL 00426060
```

The existing storage provider performs growth. Current singleton allocation and
free retain the matching heap binding. The raw-context copy provider retains its
already accepted resize/getter route, current `memmove` and nonzero-copy policy.
Later clear at `0094F3E0` and retirement at `0094F540` are lifecycle dependencies;
neither is an insertion callee. No CString adapter, append delegation, host,
callback protocol, global, TLS context, synthetic pool or new list wrapper is added.

## Mandatory admitted domain

The caller prepares the exact retained
`bsp::game::game_native_string_process().raw_context()` before entry. Its real
`01090AA8`, `01090AA4`, `01090AA0`, manager/deletion bindings and pool generation
remain valid through insertion and final dependent-record retirement. This body
performs no implicit process lookup and uses no `NativeStringStorage` adapter.

The raw12 is actual aligned writable live storage, genuinely constructed and
grown through the accepted providers: slots `+0`, count `+4`, capacity `+8`.
Require `0 <= count <= capacity <= 0x3fffffff`, capacity at least 64 and a multiple
of 64, and genuine nonnull unique slot allocation of exactly `4*capacity` bytes.
Full-list growth requires old capacity at most `0x3fffffbf`. Count increment,
index/cursor multiplication and every reached address must remain nonwrapping.
Slots `[0,count)` are initialized; spare destination `slots[count]` is writable
but need not be initialized before its first store.

Copy borrows a coherent live nonnull actual raw8 length/data header. It may borrow
an existing list record when the header/data remain disjoint and live across
growth and copying. Adoption transfers null or a genuine unique separately
allocated matching-current-heap raw8 with exclusive nonnull-data ownership.
Stack/inline/synthetic headers, duplicate ownership, and aliases into list,
slot or private-frame storage are excluded.

Nonnull data has exact positive nonwrapping allocation size `length+1`.
Sizes 1 through 149 require the valid canonical pool generation and size class;
sizes at least 150 require matching current CRT provenance. Genuine positive
resources and normal returns are required from all reached providers.
Operations are serialized without faults, throws, asynchronous SEH, overflow,
OOM, callback/new-handler mutation, reentry, concurrency, trim/drain/teardown or
pool/lifetime replacement. Flat Win32/ES and DF-clear storage-provider conditions
remain mandatory. These are preconditions, not newly invented runtime repairs.

## Separate current C++ exception policy

Only the copy branch guards its captured fresh eight-byte header. A current C++
copy exception frees only that header through the canonical current free provider.
It never destroys string data or frees adopted/existing records, slots or raw12.
The source disarms after final slot/count publication and preservation of the
captured result. No rollback of earlier growth or partial shifts is invented.
The public API remains potentially throwing; this policy adds no Original EH credit.

Saved private evidence has handler `00CA8CEB` selecting descriptor `00DDCA60` and
the private dispatcher target `00BF6B43`. The 36-byte descriptor records magic
`19930522`, maxState 1 and map `00DDCA58`; the map is `(-1,00CA8CE0)`.
The 11-byte cleanup reads private `[EBP+0C]` and has header-free target `00BF65AC`;
no data destructor appears. Header-only intent is supported, while actual saved
frame identity, dispatch/establisher mapping and all throw/fault outcomes remain
unproved. The Source guard does not replicate that private frame or handler.

## Verification and emitted-object limits

One strict MSVC Win32 compile used `/std:c++17 /O2 /Ob2 /Oy- /MD /W4 /WX /EHsc
/Gy /GL- /fp:strict` with actual `/sourceDependencies` output. Compiler version
was 19.51.36244.0. All 105 declared Source/header inputs matched fresh precompile
snapshots; the 132 candidates and compiler/tool/provider/build pins were observed
before this worker's first compile. Current Main providers were checked separately
from historical Root validation reports. Imported v180 Win32
`Platform.Common.props:22` supplies the default disabling frame-pointer omission;
no source-specific CMake option was added.

The actual object is 4,791 bytes, SHA-256
`6661578da198786ca5af25a1eb1beef1617193064c3b26edc1ec32a73df16093`.
All 19 sections, 43 primary symbols plus 19 auxiliary slots, 1,342 string-table
bytes, directives, runtime data, CodeView, frame debug, SafeSEH, current GS/EH,
and relocation fields are accounted, with zero unaccounted file bytes.
Eleven executable sections contain 498 bytes and 204 decoded rows, including
five CC alignment rows and four NOP instructions. Thus 199 function instructions
remain after excluding CC alignment, or 195 rows after excluding all NOP/INT3.
The entry section is 265 bytes / 98 decoded rows, including two NOP rows.
Eight emitted helper COMDATs have no runtime incoming relocation; disarm still
has its associative frame-debug reference. They are accounted, not presumed
linker-discarded. There is no all-code-reachable claim.

There are 12 code and four metadata relocations: five `DIR32 (0006)`, two
`DIR32NB (0007)` and nine `REL32 (0014)`. `0007` is not `SECREL (000B)`.
The two 16-byte frame-debug records refer to destructor/disarm procedure sizes
14/7. SafeSEH resolves symbol index 51 to section 15 offset 13. The current FH3
map resolves its only cleanup to the captured fresh-header guard. All nineteen
opaque `.chks64` words and section auxiliary checksum fields are retained;
their internal checksum algorithms were not independently validated.

The compiler inlines two shift paths and reschedules pure destination arithmetic,
cursor decrement and result-register assignment. Each actual volatile load/store
and publication remains in the required normal-domain order. It eliminates the
explicit normal disarm/destructor after the nonthrowing publication suffix and
removes its current FS0 registration. These observations do not prove native
register/flags identity, instruction order, fault equivalence or Original EH.

One normal `./scripts/build.ps1` passed its two already configured checks,
`reconstructed_math` and `tool_tests`. The new translation unit remained
unregistered and unexecuted. No new tests, probes, replay compile/build or game
run occurred. Required call-site verification covers the three explicit pairs
in both this Markdown and the structured report; exact results are recorded in
the report and sealed local receipts.

The worker evidence family is
`local/WholeSceneStringListIndexedInsertSource01`, including fresh input closure,
the actual object and complete audit, genuine worker tool transport receipts,
a tool-only trace with original byte/line bounds and authenticated source-prefix
hash, four Git blobs and prior-family preservation.
Earlier sealed readiness/planning/ownership families remain external evidence.
Independent review, registration and the integrator's Main/archive validation
remain pending. Physical ABI/private CRT/EH/SEH, application ownership at
`008F3270`, caller/startup/runtime and game validation remain unproved. Saved
direct-caller evidence found none; that does not exclude indirect callers or
establish an application create/pass/retire schedule.
