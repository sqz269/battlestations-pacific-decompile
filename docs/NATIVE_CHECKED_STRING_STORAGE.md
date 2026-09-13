# Checked pooled-string storage for input settings

Addresses: 00450540, 004954F0, 0048DA50, 00450180, 0044CE20, 0044F410,
006A7BE0, 006AA460, 006AB6B0

`native_checked_string_storage.cpp` supplies source storage contracts for the
actual checked vector `{opaque, begin, end, capacity_end}` and its eight-byte
pooled string elements. It uses the existing actual-header string assignment,
range destruction and CRT allocation services. It creates no shadow container
and does not reinterpret a projected `std::vector<std::string>` as native data.

The settings parser now calls the concrete append and erase functions directly.
The settings destructor and constructor unwind reuse the existing complete
00432050 range destructor for device order. These changes remove
`call_00450540`, `call_004954f0` and `call_00432050` from the required table and
destruction interfaces. Remaining tree and nonstring-vector operations still
require their providers.

## Storage behavior

Append at 00450540 constructs in unused capacity and publishes the captured end
plus eight only after construction returns. When full, the 00450180 wrapper
enters 0044F410 with count one and the captured end iterator. That path first
copies the source string into a temporary, preserving aliases into old backing.
It grows capacity to `max(size + 1, capacity + capacity / 2)`, subject to the
native element limit `0x1fffffff`. Old elements and the saved value are copied
into new backing. Old strings are destroyed forward before freeing the backing;
capacity, end and begin are then published in that order. The temporary string
is released last. Copy failure releases completed new elements and new backing,
while retaining the existing string-construction primitive's failure boundary.

Erase at 004954F0 checks that the iterator owners are nonnull and equal. It does
not require them to equal the receiver. For a nonempty range, 0048DA50 assigns
the captured suffix forward with the existing actual-header string copy. The
new end uses the old captured end, while destruction reads the current end
after assignment. Trailing strings are destroyed before publishing the new end.
The output iterator receives owner before position, including an empty erase.
Capacity and opaque header word are retained.

The append contract requires consistent storage throughout allocation callbacks;
it implements the native end-insertion use, not general 0044F410 insertion.
Callbacks may throw but must not structurally mutate the vector or source range.
This source API does not replace the original library, CRT, register or FH3 ABI.
Malformed storage, arbitrary private-stack aliases, native exception execution
and hardware-fault behavior remain outside the contract. Existing zero-byte
string-copy omissions are retained.

## Evidence and validation

Six complete byte envelopes match the live program and installed executable.
Register inputs, stack cleanup, growth publication and copy/destruction order
were checked in assembly. The 0044F410 returning-free gaps were decoded under
the Ghidra write lock without changing any callee no-return flag. Separately
owned catch blocks are attributed to their actual function, rather than the
enclosing byte envelope. All 37 owned direct CALL rows pass the live instruction,
callee and ownership audit. One decoded rethrow at 0044F5E5 remains outside the
saved catch body's ownership and is explicitly excluded from that count.

The retained settings lifetime fixture is adapted to the reduced provider
interfaces, so source settings construction, parsing, destructor and manager
drain use the new storage. One additional scenario checks append growth with
an aliased element, spare-capacity append, partial/empty/full erasure, a valid
iterator owner different from the receiver and complete pooled-string cleanup.
The strict Win32 build and both CTests pass. The settings comparison matches
122,545 words; the focused storage comparison matches 335 words, including
allocation/release counts, with no pooled strings left allocated. Source
constructor-failure cleanup and actual manager drain also pass. Exact source,
archive and fixture hashes are recorded in
`reports/native_checked_string_storage.json`.

This closes three storage dependencies. Production providers for the remaining
settings containers and complete application wiring are still required. No
running game is accessed, and no workers are dispatched for this packet.
