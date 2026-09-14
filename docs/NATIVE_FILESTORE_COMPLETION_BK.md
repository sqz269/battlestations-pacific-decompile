# Native FileStore completion

`native_filestore_completion.cpp` reconstructs the complete resident insertion
and completion bodies BE7760, BE78B0, BE7B20, BE5CF0, BE6090, BE6250 and BE7340.
These source interfaces use actual Win32 FileStore, iterator, pooled string and
intrusive stream storage. Names remain descriptive hypotheses.

BE7760 normalizes a copied name and searches the actual resident tree at
store+14h. A duplicate returns without retaining the supplied stream. The fresh
path increments stream+4, constructs two retained 0Ch pairs, inserts uniquely
through BE7340/BE6CF0, then destroys both pairs and releases the temporary stream
reference. The node retains its own reference. The nonnull stream precondition
and returning/noexcept string-pool release domain are retained. The diagnostic
target4254B0 is the verified one-byte RET; no replacement log callback exists.

BE7340 preserves the native case-insensitive search, predecessor check and
duplicate result. Actual output is owner/node/inserted byte; bytes9..11 remain
untouched. The independently recovered resident helper packet supplies BE4D20,
BE6CF0, BE6590 and BE6170; existing resident rotations are reused.

BE78B0 copies and normalizes the first completion name, locates its pending node
in store+20h, captures callback+14h, and erases that node before adding the stream
to the resident tree. It then passes the two original borrowed name headers to
the captured callback. No null-callback guard, stream argument or cancellation
policy is introduced. The native callback removes eight stack bytes.

BE7B20 receives stream/first/second, calls the current4FC150 factory getter, reads
that factory's current cache+8, and invokes BE78B0 on that provider. It does not
remember which provider submitted the request. Final callback identities require
an explicit source dispatcher; they are never executed as original code addresses.

Independent listing review corrected two returning-CRT capture orders. BE7760
captures owner/node/head before BE77B6 and compares the captured node/head after
the call. BE78B0 captures owner before BE78FF, node after that first CRT call,
then reloads erase-owner after BE790D while retaining the captured node for the
callback lookup and erasure. Both corrections were reviewed again.

Original ABI: BE7760 is ECX store with name/stream and RET8; BE78B0 is ECX store
with first/second/stream and RET0C; BE7B20 has three stack arguments and RET0C.
BE6090 is ECX destination with name/stream-cell and RET8; BE6250 takes one pair
and RET4. BE5CF0 takes ECX pair and RET; BE7340 is ECX tree with output/pair and
RET8. The new C++ interfaces are not drop-in original ABI/FH3 implementations.

The seven complete normal bodies total1223 bytes. Strict MSVC Win32 compilation
passed. Original/EH bytes, the independent review and the combined actual-storage
fixture are recorded separately in `reports/native_filestore_completion_review_bk.json`.
The integration report records the final combined source/build/run revision;
source fixtures do not establish native exception ABI or game fidelity.

Hardware-fault cleanup, mutable native private-stack aliases, original CRT
exception identity and a throwing raw pool getter remain outside this source
bridge's domain. The complete queue/loading-manager caller is a separate packet.
