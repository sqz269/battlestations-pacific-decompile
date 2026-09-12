# Actual MPKG archive construction and destruction

Addresses: `00BB9920`, `00BB9C10`, consumed memory data leaf `00BEF610`.

The AX implementation joins the actual provider, directory and entry-vector
owners. It constructs the original 34h archive storage through the existing
raw string pool, memory conversion and retained memory owners. The prior
`mpkg_archive.cpp` parser remains a separate semantic implementation.

| Routine | Original ABI | Source coverage |
|---|---|---|
| BB9920, 747 bytes | ECX archive, stacked original string header, EAX archive, RET4 | Complete source body; independent review and combined fixture pending |
| BB9C10, 145 bytes | ECX archive, no stacked arguments, RET | Complete source body including the restored post-free tail; fixture pending |
| BEF610, 7 bytes | ECX memory stream, EAX backing data, RET | Existing semantic implementation strengthened with the exact raw-owner leaf ABI |

Construction zeroes the embedded string before its self-header test, then
preserves the original resize/copy reloads. It initializes the vector at +28h
and end-record offset at +1Ch. The original name, rather than the embedded copy,
is sent to the captured current manager method with flags 2. It converts that
source to a memory stream, decrements the original source's intrusive reference
and invokes its current slot 0 only at zero.

The converted stream's low size DWORD controls a new 10h backing. Decoding uses
blocks of 2F1h bytes and the native XOR table at E144F0 with period 219h. In the
short final block, the source index uses the **global output index modulo the
short block length**. A conventional reversal of each independent block would
change that branch. Each iteration reloads the backing data through BEF610;
there is no new size, padding or initialized-prefix policy.

The converted stream is then deleted through its captured slot 4 with flags 1.
This is distinct from decrementing its reference. A new stream retains the
decoded backing; construction stores that stream at archive+0Ch before releasing
the backing's temporary reference. The end-record scan writes the stream size
at +10h and a hit offset at +1Ch. Field +0 remains untouched.

End-record reads reuse one native DWORD at ESP+30h. Its initial value is the
backing pointer, and every short read retains the unwritten bytes of its prior
value. The implementation preserves that scratch state, the current stream
reloads, the captured stream before publishing the entry count, and the wrapping
directory-base subtraction. The entry loader consumes actual reader and vector
storage; it adds no signature checks, duplicate filtering or bounds clamps.

FuncInfo DFE4DC and its three-state map DFE4C4 establish constructor cleanup.
The path and vector are owned by states 0 and 1; state 2 frees only the raw
backing allocation when its constructor throws. There is no additional rollback
for a converted or decoded stream. The source preserves those ownership limits.

Destruction captures the decoded stream and decrements its reference, then
lowers the cleanup state before resizing/freeing the vector and again before
returning the path. The original tail at BB9C68..BB9CA0 had been excluded by a
local erroneous free-call override and truncated function body. Both were
repaired under the write lock, with old state retained. Stale pointers and counts
are not cleared merely to make repeated destruction safe.

These are new C++ service interfaces over actual storage. Native FH3/SEH identity,
arbitrary aliases into native stack spills and simultaneous cleanup failures
remain outside their proof. Nested `NativeStringStorage::release` retains its
existing noexcept contract. No installed `.mpkg` archive was found in the current
installation, so the planned comparison uses explicitly generated archive bytes.
No original-game startup or gameplay claim follows from it.

Evidence is in `reports/native_mpkg_archive.json`,
`reports/native_ax_flow_repairs.json` and
`reports/native_ax_function_definitions.json`. Final integrated build and fixture
results will be recorded after the worker sources are merged.
