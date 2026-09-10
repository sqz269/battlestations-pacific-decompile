# Actual pooled alias-list clear

`clear_native_render_resource_aliases_004d05e0` exposes the complete recovered
`004D05E0..004D0632` operation on the actual list owner. In a renderer resource
record that owner is `record+8`: its first word is preserved, its sentinel is at
`+4`, and its count is at `+8`. The interface retains the real node and pooled
string allocations. It does not copy the list header into a host container.

The first node is captured before resetting the sentinel's next link. The owner
sentinel is reloaded for the previous-link reset and initial equality test; that
test precedes the count-zero write. Each iteration captures the current node's
data and next before returning a nonnull string through the actual `00419CC0`
pool, then frees the node through the existing ordinary allocator domain. The
captured next is compared against the current owner sentinel after those calls.
Volatile field accesses preserve these loads and writes in their native order.

Clear retains the sentinel allocation and the surrounding record fields. The
complete `00B2F990` record destructor now calls this shared operation, then frees
and nulls its current sentinel and releases its current name, preserving its
existing unwind timing. Its payload and resource remain untouched. The earlier
particle-oriented typed interface remains available and is preserved in the
ledger's prior-interface evidence; this is an upgrade of an already reconstructed
function, not an additional native function count.

The original list-clear `_free` call at `004D0620` had a false no-return analysis
that hid its captured-next comparison and loop tail. The continuation was
verified against the installed executable, repaired with existing comments and
names journaled, and saved. Both affected entries have refreshed evidence
annotations and exports.

The strict MSVC Win32 build passes the existing two CTest checks. One focused
extension of the existing resource-record native fixture calls clear directly,
checks the four-event alias release trace and retained sentinel/name, then calls
record destruction on the empty list and compares the complete six-event trace.
Original installed code and the reconstructed functions agree on captured-next,
current-sentinel replacement, post-free name reload and preserved callback
writes. It uses the actual pool and ordinary malloc/free boundary. Native
exception injection, binary replacement and gameplay were not validated.

Evidence and artifact hashes: `reports/native_render_alias_list_clear_audit.json`.
Record assignment and range insertion remain separate dependencies.
