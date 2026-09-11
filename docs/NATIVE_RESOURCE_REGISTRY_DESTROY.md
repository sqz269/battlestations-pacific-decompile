# Native resource registry destruction

This packet reconstructs complete `B1B5F0[111]` and `B19760[17]` using the
existing actual range erase and owning string-pool providers. It borrows the
actual F8D41C publication word. It does not create another registry singleton,
pool, registry population route or factory ownership policy.

| Entry | Original ABI | New source |
| --- | --- | --- |
| B1B5F0..B1B65F exclusive | ECX registry; plain RET; no semantic result | `destroy_native_resource_registry_00b1b5f0` |
| B19760..B19771 exclusive | ECX captured registry; plain RET; no semantic result | `reset_native_resource_registry_00b19760` |

These are new MSVC Win32 C++ interfaces. The explicit `void* volatile&`
publication parameter must borrow actual F8D41C storage rather than a copy
of its value or a private replacement. The destructor also accepts the
existing actual pool and returning CRT service. No native ABI or installed
game execution is claimed.

## Storage and native schedule

Direct accesses require a 10h-byte registry prefix: profile +0, preserved
DWORD +4, tree head +8 and tree count +0C. This does not establish the full
size of a most-derived allocation. The tree is registry+4, so its own head
and count are +4/+8. The caller retains the registry allocation throughout;
neither entry frees that allocation or follows the publication's old value.

`B1B5F0` captures the registry, current head and that head's minimum. It
constructs by-value first `{tree,minimum}` and last `{tree,head}` iterators
and an uninitialized output slot. Native state 0 becomes active only after
these arguments are captured, immediately before complete `B1A2F0` range
erase. The current reconstructed provider retains its full-range and partial
branches, independent increments, raw node key ownership and returning CRT
semantics; this parent does not replace it with a subtree-only substitute.

After range erase returns, the destructor reads the current tree head and
frees it through the existing actual CRT free boundary. It does not free
the originally captured head merely because that was the range endpoint.
The full returning-free continuation `B1B638..B1B65F` then clears current
head, clears count, clears actual F8D41C publication, and writes CE3818 on
the captured registry, in that order. There is no initial profile write.
The preserved DWORD and opaque factory values are not owned or altered.

`B19760` is a distinct complete entry. It clears publication first and
writes CE3818 on its captured registry second. It never reads the previous
publication, registry profile, head or count, and it performs no cleanup of
partial tree state. A publication that currently points elsewhere does not
redirect the reset target.

## Unwind and provider boundaries

Fresh FH3 evidence is handler CBC778 -> FuncInfo DF4AD8, unwind map DF4AD0,
state 0 -> CBC770. The action loads the captured registry from EBP-18 and
jumps to full B19760. The native state stays active through range erase,
head free and the normal reset stores; ordinary return removes the frame.
On propagation the action only resets publication/profile. Partial tree
links, head and count remain as the failing operation left them.

The source guard starts after argument capture. It calls the full reset
entry on host C++ propagation and is disarmed after the final normal profile
store. No generic rollback, extra head/count repair, retry, owner free or
factory-value disposal is introduced. Native FH3/SEH and host exception ABI
remain separate. Existing pool release and free are `noexcept`; failures
crossing those interfaces are not transformed into new recoverable throws.

The actual pool/lifetime implementation and complete range/erase/tree leaves
remain the selected source providers. Named host CRT, allocation, locking,
native exception runtime and current source exception-carrier boundaries
retain their existing qualifications. This packet does not close B1B730 /
B1B810 publication/getter, registry construction/population, scalar deleting
destructors B1B660/B1B710, or the B1A4F0 cache consumer.

## Evidence and focused verification

All fresh live queries verify `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Twelve finite spans, 1,214 bytes total, pin the
two full entries, direct range/erase/subtree/free dependencies, FH3 actions
and data, base profile data and neighbor padding. All match the unchanged
installed PE. Both saved entry extents are complete; the primary's prior
B1B5F0 returning-free repair is retained without worker metadata changes.

F8D41C has separate guarded data evidence. It lies in the PE section's
virtual zero-fill tail and has no on-disk DWORD. Saved zero bytes therefore
do not establish the current runtime publication. The caller-bound storage
contract is explicit in the API and audit.

Current provider files are pinned independently. The prior erase worker seal
used CRLF for its CPP; this checkout normalized it to LF. An exact comparison
after only newline normalization confirms identical source text, and both
raw hashes are retained. Other selected key provider files match their
previous source pins exactly. The isolated hook adds this source and the
already completed erase source because the base predates committed erase
CMake registration; tree leaves are already registered. Shared CMake and
metadata remain unchanged. Strict Win32 `/fp:strict /W4 /WX`, both existing
CTests and all eight native seeds passed. The two-operation comparison
passed with 23 trace words. Ten exact archive members contain 204 nonempty
COFF code sections, 14,514 bytes and 469 relocations; the fixture links 85
sections, 8,154 bytes. Full member/code bytes and relocation records are
pinned, with linked bytes checked outside relocation operands.

One ignored focused fixture compares the normal destructor and the full
direct reset. Original B1B5F0 and B19760 remain byte-for-byte intact in the
mapped code arena; only B1A2F0 and BF65AC provider entries are bridged to
their complete current source implementations. The original full range and
pool/CRT helpers are not executed, and original unwind is static evidence.

The fixture reserves writable `[00F80000,00F90000)` for the literal F8D41C
slot after checking that it does not overlap its own PE. Both native stores
and the source's borrowed reference use this exact slot. Reservation failures
are setup failures. This isolated mapping does not touch the installed game.

During actual node free, a fixture observer changes current head to a second
real allocated head. Complete range erase publishes empty links on that
current head, and the destructor frees it. The original captured head remains
allocated and unchanged. During the actual current-head free, the observer
changes head/count/publication; the normal continuation clears them and
resets the captured registry rather than the new publication target. The
actual small pooled key is returned and the factory word remains untouched.
Leftover fixture storage and canonical pool lifetime are cleaned separately.

The direct reset comparison uses a non-dereferenceable head value to verify
that tree fields are untouched, with publication pointing at a different
registry prefix. No artificial exception callback or partial-iterator
injection is used. This complete ordinary full-range route has no safe
throwing callback for an honest unwind fixture, while inherited pool/free
interfaces terminate on crossing exceptions. Static original FH3 and current
linked guard code establish the reset schedule; no unwind execution claim
is made. Whole-object evidence remains broader than executed paths, and
linked comparisons exclude relocation operands rather than proving every
resolved target. The audit and immutable bundle retain exact build, test,
seed, trace, archive, COFF and linked-code evidence for primary relinking.


## Primary integration

Primary registered all 2 complete source entries and passed the strict Win32 build, both existing CTests and eight fresh seeds. All three packets use the same frozen main library `b1fa83e3959f4db6a0cdeb207054a57057bea9749cd48f8b5d3037b7f09e14c5`. Unchanged focused fixture matches23 words for original/source destroy and directreset; actual nodefree changescurrenthead and headfree changescount/head/publicationdecoy. Original128 ownedbytes execute with complete current range/free providers and literal writableF8D41C mapping, no fixturePE overlap. Ten exact archive members/204 wholeCOFF code sections14514bytes469relocations;85 linked sections8154bytes checked outside relocation operands, not every resolved target. Original nativeEH and sourceguard unwind static only; inherited pool/free exception boundaries remain. No factory/getter/scalar-delete/nativeSEH/originalcallerABI or game claim. Reviewed names and evidence are saved with prior comments retained; all affected exports were forcibly refreshed. Immutable primary evidence: `local/registry_destroy_primary/`.
