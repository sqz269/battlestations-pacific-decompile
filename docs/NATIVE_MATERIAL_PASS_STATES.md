# Actual material pass state owners

Addresses: 00B40AC0, 00B40B40, 00B40BE0, 00B420C0, 00B42140, 00B421C0,
00B422F0, 00B42310, 00B42330, 00B5EC40, 00B5ED60, 00B5EE00, 00B5F720.

`native_material_pass_states.hpp/.cpp` reconstruct twelve complete functions and
three initialization fragments using actual Win32 storage. The names below are
descriptive hypotheses. This is a dependency of the actual pass constructor,
destructor and effect loader; it does not implement those larger bodies.

## Storage and original ABI

Each owner is 14h bytes: numeric native vtable at00, atomic reference count at04,
data pointer at08, signed row count at0C, signed capacity at10. The last three
fields form the actual12h header passed in ECX to reserve functions.

| Pass slot | Vtable | Row bytes | Reserve | Destructor | Scalar deleting wrapper |
| --- | --- | --- | --- | --- | --- |
| +18 render | D61A2C | 8: state,value | B40AC0 | B420C0 | B422F0 |
| +20 sampler | D61A34 | 12: slot,state,value | B40BE0 | B42140 | B42310 |
| +1C third | D61A3C | 12: interpretation unresolved | B40B40 | B421C0 | B42330 |

Each two-word table contains BD30E0 followed by its scalar deleting wrapper.
The third profile's role is provisional; no texture-stage interpretation is
required by this implementation. The canonical companion borrows actual+04,
checks the bound current profile, destroys/frees the actual owner on final zero,
then retires separately. It has no child-owner registry or second counter.

Within B5F720, B5F793..B5F7AB, B5F7C0..B5F7D8 and B5F7ED..B5F805 are
24-byte initialization fragments (ends exclusive): EAX is allocated storage,
EBX=0 and EDI=1. Each publishes CEB130, count1, its concrete table and three
zero header fields. The parent allocates14h through BF681B and publishes
render+18, sampler+20, then third+1C. The fragment API initializes caller-owned
storage; it does not claim parent allocation, publication, defaults or unwind.

Reserve uses ECX header and one signed stack capacity, RET4. The request clamps
to1 and growth uses a signed comparison. Growth allocates request*stride via
BF55BE (a BF681B operator-new thunk), copies forward with current source/count,
frees the current old data through BF6989, then stores data and capacity in that
order. It does not change count or shrink a positive capacity.

B5EC40 (ECX pass, state/value, RET8) and B5ED60 (ECX pass, slot/state/value,
RET0Ch) scan the captured owner's rows for the first key match. Equal values
return; differing values overwrite that row. Absent keys append, doubling the
signed DWORD capacity when count equals capacity; a doubled result <=1 becomes1.
Neither reads the owner reference count or clones shared storage. The existing
semantic `set_material_sampler_state` remains separately available.

B5EE00 (ECX pass, state, RET4) removes only the first matching render key by
copying the last row into the gap. It reloads the owner from pass+18, computes
current count minus1, reserves if needed, then reduces/stores that count.
Order is unstable; a missing key leaves the owner and rows unchanged.

Destructors use ECX owner, RET. Negative capacity first calls the matching
reserve(0), which clamps to1 and performs its normal allocation/copy/free.
Positive count decrements to0; count is stored0, current data is freed and
BD30F0 publishes CEB130. The data pointer and capacity remain stale after
the final free. The state0 unwind cleanup also calls BD30F0: handlers
CBF0B8/D8/F8 select FuncInfo DF7A08/34/60; their one-entry unwind maps
DF7A00/2C/58 point at CBF0B0/D0/F0, which load saved ECX and jump BD30F0.
The rebuilt base cleanup follows that resource ordering, without native SEH
encoding. Scalar wrappers take flags on the stack, RET4, return the original
pointer and free the owner through BF65AC only when flags&1.

## Evidence and validation

`reports/native_material_pass_states.json` pins the tested source, original
spans, native profiles, frozen binaries, fixture output and annotation audit.
Nineteen live Ghidra spans matched the installed PE: twelve complete routines,
three initialization fragments, BD30F0 and three profile tables. Explicit
assembly was required because false no-return flow had hidden free continuations.

Nine flow-repair events decoded the missing bytes. Six internal gaps were
fully repaired. The three destructor tails were decoded, but Ghidra still
stores bodies ending B42118/B42198/B42218, excluding the final35 bytes through
B4213B/B421BB/B4223B. A locked, bounded body-extension attempt was rejected
because the server disables arbitrary Java scripts. No script ran. Full
destructor evidence and fixture bytes use the verified PE spans through RET;
the refreshed Ghidra pseudocode is still incomplete for these three entries.

The strict MSVC Win32 build and both existing CTests pass. One ignored native
fixture compares30 snapshots of all20 owner bytes (data identity normalized)
plus live rows. It executes all twelve recovered routines and the three
initialization fragments, including shared aliases with actual count2,
duplicate-key replacement, geometric growth, unstable removal, reserve no-op,
normal teardown and all three negative-capacity teardown paths at count1.
Pass bytes outside the state owners remain equal to their preimage. Three
rebuilt final-zero paths free the actual arrays/owners and retire their
canonical companions through `release_native_render_actual_owner`.

Fixture boundaries: original CRT allocation/free calls are rebound to the
shared host allocation domain; init fragments append a RET and receive their
observed EAX/EBX/EDI inputs. Original scalar wrappers run with flags2, so bit0
is clear; rebuilt companions exercise flags1. Native exception handlers are
not invoked. Original instructions run with ordinary SEH frames only. This
does not prove original CRT or exception-delivery compatibility.

The public C++ interfaces are not drop-in ABI replacements. Valid readable
nonnegative extents are required for setters/removal; reserve copy extents must
fit the requested buffer. Corrupt extents and DWORD allocation overflow are
host errors, rather than reproducing native memory corruption. Negative
capacity teardown is established only where its reserve(0) copy fits one row.
Real pass lifetime/default construction, shader/descriptor loading, D3D state
submission, rendering and gameplay validation remain open.
