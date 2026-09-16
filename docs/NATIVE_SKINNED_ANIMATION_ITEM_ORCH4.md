# Actual SkinedMeshAnimation item parser and lifetime

The registered `SkinedMeshAnimation` parser slot at `D630D8+8` targets
`B92F20`. This packet reconstructs its actual item, owned vectors, reader and
terminal lifetime in `native_skinned_animation_item.hpp/.cpp`. It borrows the
application's existing `NativeResourceStreamReadContext` and
`NativeStringRawPoolContext`; it creates no replacement resource container,
string pool, stream, publication cell or callback implementation.

This is the missing item family identified by
[the Object model resource audit](NATIVE_PARTICLE_MODEL_RAW_ORCH4.md).
Parser and reference dispatch bindings remain a separate integration step.

## Evidence and coverage

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` was
verified by every standard live query. All thirteen complete disk bodies
(1,959 bytes) match current Ghidra bytes. The report records per-body SHA256,
original ABI, coverage and every direct call's exact site/callee/containing
function. Names are descriptive hypotheses, not recovered symbols.

| Native | Source operation | Coverage | Original ABI |
|---|---|---|---|
| B92300 | Read twelve float32 values | complete | ECX output30h, EDX handle; EAX output; RET |
| B92440 | Reserve inner values vector | complete | ECX vector, stacked signed capacity; RET4 |
| B926E0 | Assign inner values vector | complete | ECX destination, stacked source; EAX destination; RET4 |
| B928B0 | Destroy record | complete | ECX record; RET |
| B92970 | Construct record, consume name copy | complete | ECX record, stacked float/name8h; EAX record; RET12 |
| B92A50 | Copy-construct record | complete | ECX destination, stacked source; EAX destination; RET4 |
| B92AE0 | Reserve outer records vector | complete | ECX vector, stacked signed capacity; RET4 |
| B92BC0 | Resize outer records vector | complete | ECX vector, stacked signed count; RET4 |
| B92C40 | Append copied record | complete | ECX vector, stacked source; RET4 |
| B92CD0 | Read item payload | complete | ECX item, stacked handle; RET4 |
| B92EC0 | Destroy item | complete | ECX item; RET |
| B92F20 | Parse allocated item | complete | ECX parser unused, stacked handle; EAX item; RET4 |
| B92FA0 | Scalar-delete item | complete | ECX item, stacked flags; EAX original item; RET4 |

## Producer-established layout

The item is exactly `18h` bytes: `+0` profile `D63700`, `+4` reference count,
`+8` an uninterpreted serialized DWORD, `+C/+10/+14` an actual pointer/count/
capacity descriptor. The parser allocates `18h` at `B92F3A`, calls the existing
`B868B0` base constructor, writes the profile and zeroes the descriptor. It
does not initialize `+8` before the reader writes its first DWORD.

The outer element is `18h`: a serialized float32 at `+0`, a raw pooled
length/data string at `+4/+8`, and a second actual vector at `+C/+10/+14`.
Each inner element is `30h`: twelve float32 values read in order. Neither the
float's meaning nor a matrix interpretation of the twelve values is proved.
`B92970`, `B92A50`, `B92BC0` and `B92300` are the producers establishing these
layouts. Growing `B92BC0` initializes name and inner vector only; it deliberately
leaves the outer element's first float untouched.

`B92CD0` reads the item DWORD, then loops while the CURRENT handle's CURRENT
node remaining count at `+20h` is nonzero. Per record it reads float/name,
constructs a temporary record from an owned by-value name copy, appends a
deep copy, destroys that temporary, reads a signed values count, then reads
and appends twelve floats for each positive count. Zero and negative counts
skip values. The loop adds no EOF validation or rollback.

## Scheduling, ABI and ownership

All signed counts/capacities and wrapping DWORD address/size arithmetic follow
the listings. Growth uses signed `max(2*capacity,1)`, reserve clamps signed
requests below one, and successful reserve frees old storage before publishing
the new pointer/capacity. Inner copies are twelve forward DWORD stores, matching
`REP MOVSD` even for overlap. Assigning an empty inner vector still reserves at
least one element; assigning a vector to itself clears its count first.

`B92300` stores every scalar read immediately with x87 `FSTP32`. `B92A50`
copies the record float using `FLD32/FSTP32`; `B92970` copies its argument bits
with `MOVSS`. The source preserves those operations, including the reader's
extra float load/store before construction. The read entry is ECX output,
EDX handle, established by its whole listing and the `B92DB0..B92DB6` caller.
It is not the handle-in-ECX convention used by `BE99D0`.

Native allocations/free compose through the existing current CRT boundary
`singleton_lifetime_allocate/free`; pooled names use the application's actual
`01090AA8/01090AA4/01090AA0` publications. String copies reload fields after
resize except the constructor's explicitly captured argument length.

Native stack cleanup is explicit in the evidence: parser new `B92F3A` followed
by `ADD ESP,4` at `B92F41`; vector new `B92468`/`B92B26` likewise consumes one
size; name `memcpy` sites consume three arguments with `ADD ESP,0Ch`;
`B92970` consumes float plus the by-value name with `RET0Ch`; the remaining
stack-argument methods consume one DWORD with `RET4`.

## Repaired listings and exception states

Initial pseudocode omitted returning-free tails. The primary integrator repaired
and saved five functions while preserving prior names/comments. The complete
tails publish inner vector storage at `B924AF..B924B8`, release the record name
at `B92909..B92941`, publish outer vector storage at `B92BA3..B92BAC`, and call
the item base destructor at `B92F07` before returning at `B92F1C`. The scalar
delete body also regains `ADD ESP,4` at `B92FB5` after free. The worker
performed no Ghidra mutation. Before-edit documentation was archived in
`local/skinned_animation_documentation_before.json`; integrator repair receipts
are `reports/native_animation_item_*_q17.json`.

The FH3 metadata `DFC3A8..DFC503` and unwind code `CC2C70..CC2D84` establish:

- Record destruction cleans its name if inner cleanup fails, and normally
  frees inner storage before returning the name; headers remain untouched.
- Record construction consumes its name argument even if copying fails. Its
  incomplete destination name is not owned by that cleanup state.
- Copy construction cleans only its completed name on inner-assignment failure.
- Outer reserve/append unwind only through no-op placement-delete `401130`;
  there is no constructed-prefix or new-allocation rollback.
- Reader state 1 destroys the completed temporary record, then state 0 destroys
  the local read name. Already appended item records remain published.
- Item destruction cleans the reference base on failure. Scalar deletion tests
  flags bit zero only after destruction; flags 2 preserves the allocation.
- Parser allocation cleanup is disarmed before the item reader. Reader failure
  does not destroy or free the allocated item. Native state 0 covers the base
  constructor and plain field stores (`B92F4C..B92F68`); state -1 is installed
  at `B92F73` before reader call `B92F7B`. Source base construction is pure
  nonthrowing stores, with no native hardware-fault transport claim.

Ordinary C++ cleanup follows those ownership states; a second exception during
unwind terminates. Original FH3/SEH/CRT exception identity, stack-local aliases,
general-register/EFLAGS identity and hardware fault delivery remain outside
these new source interfaces.

## Validation and limits

`scripts/build.ps1` passed the full Release MSVC Win32 build and all three
existing CTests. A strict standalone `/W4 /WX` Win32 compile and the focused
ignored probe passed; report-call validation checked 57 rows with zero failures.
The ignored probe compares copied original `B92440/B926E0` bodies
against source reserve/copy/self-assignment and exercises the source parser
through the actual raw pool and structured reader context. It forces outer
relocation, reads 24 values and an embedded-NUL name, checks empty inner capacity,
full remaining-budget consumption and flags-2 deletion. Summed free-ring size
classes account for all arena bytes after cleanup (the pool's `live` counter
counts returned reusable blocks, not outstanding allocations). Original vector
comparisons use the same current CRT allocation/free
boundary, not original CRT execution. The parser fixture is source execution,
not original-parser differential or game runtime evidence.

An injected failure on the second values read preserves one appended outer
record with no appended values, cleans local temporaries and permits complete
item teardown. Free-ring size totals again account for all pooled arena bytes.

No `bsp_game` log, installed-game run, renderer output or gameplay validation is
claimed. This packet supplies complete item-family source; it does not by itself
bind the manager's parser dispatch or resource-container terminal dispatch.
