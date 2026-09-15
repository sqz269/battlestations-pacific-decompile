# Native damageable-section vectors

## Result and scope

The fourteen ordinary entry bodies below are reconstructed in
`src/native_damageable_section_vector.cpp`. They close the section-vector
append/insertion dependency of the still separate reader `0087CA80`. They use
actual borrowed 10h vector headers, 8-byte owner/position iterators, and 30h
records. There is no replacement STL container, invented vtable, unresolved
game callback wrapper, or fake record destructor.

The existing genuine `00878B40` copy constructor, `008798F0` uninitialized fill,
`00878EF0` temporary destruction, native legacy string/exception owners, and
canonical allocation/free and returning invalid-parameter boundaries are reused.
The actual current row/owner vslot0 remains a virtual call, as in the original.
The caller supplies the stable actual D0DF04-equivalent table identity.

## Entry coverage and original ABI

Names are descriptive hypotheses, not recovered symbols. All stack sizes below
exclude the return address. `report` means
`reports/native_damageable_section_vector_orch4.json`, which preserves previous
names, exact intervals, SHA-256 hashes, live instruction counts and every direct
and indirect CALL in those intervals.

| Entry | Bytes | Original ABI and behavior |
| --- | ---: | --- |
| 00744070 | 31 | ECX header; EAX signed byte-distance/30h bit pattern, zero when begin null; RET |
| 00876A10 | 84 | ECX unsigned count; EAX raw allocation; RET; count overflow throws bad_alloc; zero still calls new(0) |
| 00878790 | 130 | ECX destination; stack source; EAX destination; RET4; ordered assignment preserving vptr |
| 00878AC0 | 38 | two stack pointers first/end; RET8; ECX irrelevant; actual row vslot0(flags0), forward |
| 00878D60 | 76 | stack first/end/destination-end and three ignored words at its wrapper call; RET; EAX destination-begin |
| 00879390 | 38 | stack first/end/source; RET; ordered assignment fill; no semantic return |
| 008794F0 | 148 | ECX first, EDX end; stack destination and three ignored words; RET10h; EAX completed end |
| 008798C0 | 43 | stack first/end/destination-end; RET; forwards EAX backward-copy result |
| 0087A580 | 105 | no inputs; no return; real counted `vector<T> too long` length-error payload |
| 0087AD20 | 55 | stack first/count/source; ECX header only forwarded as ignored word; RETCh; EAX first+count*30h |
| 0087B610 | 37 | stack first/end/destination; ECX header only forwarded as ignored word; RETCh; EAX copied end |
| 0087B950 | 753 | ECX header; stack iterator owner/position/count/source; RET10h; owner word unused, no semantic return |
| 0087C2D0 | 173 | ECX header; stack result iterator/iterator owner/position/source; RET10h; EAX result iterator |
| 0087C870 | 161 | ECX header; stack source; RET4; append, no semantic return |

`0087B950` now has286 complete listed instructions. Older feature counts259 and
the intermediate281-instruction listing omitted returned-free instructions.
The integrator verified all753 live/disk bytes, repaired call flow after
`0087BAD9` and `0087BB13` under the shared Ghidra lock, saved, and refreshed the
export. The report names its separate repair receipt. `0087BC0A` is an internal
branch target; the call to `00744070` at `0087BA40` is real.

## State, aliasing and arithmetic

Every pointer/header access is an actual volatile32 load/store. Address
arithmetic wraps32; row distances perform wrapped subtraction followed by signed
division truncating toward zero. Comparisons used by capacity and validation are
unsigned. Captures and reloads follow the listing, including returning validation
callbacks, which can mutate current headers. The source does not add validation
retries, alignment checks, whole-header snapshots, or callback rollback.

Assignment `00878790` copies +4/+8, then six ordered x87 FLD/FSTP pairs at
+0Ch..+20h. It reads incoming+24h before outgoing+24h. If they differ, it first
publishes the incoming owner, then retains it, then releases the captured prior
owner, dispatching its current vslot0 if its refcount reaches zero. Only after
that callback returns does it transfer +28h/+2Ch with two more x87 pairs. The
destination vptr is untouched. Self-assignment still executes the transfers but
does no retain/release. Partial overlap is ordered, not memcpy/memmove semantics.

Insertion always copy-constructs a private30h source snapshot, even for count0.
It grows capacity by1.5 unless that exceeds05555555h, then raises it to the
required size. Reallocation constructs prefix, inserted values, then suffix;
only after these succeed does it destroy old rows, reload/free header.begin,
and publish capacity-end, end, then begin. An old-row destructor exception does
not reclaim the successful new allocation.

With spare capacity and a tail shorter than count, insertion first constructs
the tail at position+count, then fills the gap starting at the reloaded old end.
It publishes the new end before assigning old tail cells. With a tail at least
count, it constructs the final count rows at old end, publishes that returned
end, copies the middle backward, then assigns the inserted range. Assignment
failures retain those published/partial writes.

The normal temporary cleanup is inline release only: the native state is already
-1 and neither temporary.vptr nor temporary+24h is rewritten. Exceptional cleanup
uses real878EF0, which does rewrite the table and clear its owner after release.
The two paths are kept distinct.

## Exception closure

`008794F0`: handlerC965D1 references FH3 dataDC87B4. Its DC879C unwind map is
state0->-1/no action, state1->0/C965C0, catch-state2->-1/no action. C965C0 calls
verified no-op placement cleanup401130. Catch-all87954B covers states0..1 and
destroys the completed prefix forward through current row vslot0(flags0), then
BF6885(0,0) rethrows. The incomplete row is not destroyed or freed.

`0087B950`: handlerC967B8 references DC8B50, five states and two catch regions.
DC8B9C maps state0->-1/C967B0; states1..4->0/no action. C967B0 calls878EF0 on the
private temporary atEBP-48h. DC8B74 selects state1's catch87BB02 and state3's
catch87BBAC; each catch transitions through its corresponding catch state2/4.

- Reallocation catch87BB02 destroys `[new_begin, completed)` and frees captured
  new_begin, then rethrows at87BB1F. Each called uninitialized helper already
  destroys its own partial prefix; outer `completed` advances only on return.
- Short-tail catch87BBAC destroys `[position+count*30h,
  current_header.end+count*30h)` and rethrows at87BBCD. The end is reloaded during
  cleanup. It does not roll back the header or old records.
- The outer temporary is armed only after copy construction completes. Its
  normal inline release is outside the try scope after disarming state0.
  Its exceptional destruction is a genuine unwind action: a second C++
  exception terminates. The explicit inner catch cleanup remains distinct;
  an exception there can replace the original before outer unwinding resumes.

`0087A580`: handlerC966D8/DC8984 has only state0->-1/C966D0, whose tail calls
4072D0 on the completed temporary SBO string. String assignment occurs before
state0 arms. Construction uses real411700, then D69260 length-error identity and
D83F98 throw metadata. The source reuses `NativeAliasListLengthError`, whose
actual28h payload/copy/destruction match those existing providers. Its host C++
catch type/RTTI is intentionally recorded as a boundary, not original throw ABI.
Allocation overflow similarly uses the canonical source std::bad_alloc boundary.

The C++ catch bodies preserve the observed ordinary cleanup actions and order.
Original FH3 personality, exception metadata/catch-type compatibility, SEH faults,
longjmp, native double-exception behavior and allocation-failure equivalence
are not validated by this source port.

## Validation and limits

The local manifested Win32 probe relocates exact original PE instruction bodies
for these operations and the existing copy/fill providers. It substitutes only
the canonical allocation/free boundary, returning invalid-parameter boundary,
Interlocked imports and the borrowed actual table identity. Unsupported native
throw calls abort the probe if reached; native EH is not exercised. Native FH3
handler addresses are retained and never invoked in this comparison.

Eight original/source scenarios cover empty append, spare-capacity append,
growing append, middle insertion with reallocation, both in-place insertion
branches, count-zero insertion, and checked insertion with a returning owner
validation callback. The source aliases existing vector storage in the nonempty
cases. Comparison covers all final row bytes (normalizing owner/table addresses),
size/capacity, iterator result, refcounts, destructor callbacks, header+0, and x87
status. Inputs include signaling NaNs, signed zero, subnormal and quiet NaN words.
A separate overlapping-assignment comparison also matches bytes and x87 status.
The source length-error payload is verified as D69260 with the exact18-byte text.

A focused source-domain subprocess sets a terminate handler, triggers length
failure while a private temporary is armed, and makes its owner destruction
throw. The subprocess reaches the terminate handler (exit73); the replacement
exception does not escape. A deliberately zero-starting fixture refcount forces
that callback after the temporary retain. This verifies the C++ unwind policy,
not original FH3 behavior or validity of such an owner in the game.

`scripts/build.ps1` passed in strict Release Win32 mode with no compiler warnings
or errors. Existing CTest checks `reconstructed_math` and `tool_tests` both passed.
The final probe was linked against this worktree's newly built `bsp_core.lib` and
passed with exit0. The report records its inputs, artifact hashes and receipt.
These are source and bounded fixture checks, not native binary ABI compatibility,
full reader reconstruction, game execution or gameplay validation.
