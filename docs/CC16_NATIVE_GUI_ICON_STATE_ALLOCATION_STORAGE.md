# Raw Icon state allocation Source proposal

Ignored four-file storage support for AB1520..AB1570,81B/25 instruction rows.
The existing high STL_allocator_allocate name/tag remains unchanged; Original,
vendor, helper, CG and duplicate-body credit remain zero. This is an ordinary
one-DWORD Source API returning raw backing, not vendor/Native ABI reconstruction.

The full unsigned count is captured once. Count0 still makes a real allocation0
request. Nonzero count uses literal UINT32_MAX/count<40h for the Native overflow
decision; the other arm computes count<<6 and directly calls the EXISTING
singleton_lifetime_allocate({Kind::object,bytes,bytes}). Native/host byte sizes
match on Win32. No new allocator/context/callback/Frame/owner, count cap or empty
shortcut is introduced. The same real transport is already used by raw Icon
vector copy. Exact H11..19/CPP51..63 and copy usage CPP194..198 quotes are frozen
in the report alongside all four whole-provider Git pins/current4ada equality.

The Native allocation-decision/return paths comprise15 rows including ordinary
ABI plumbing. Ten remaining Native rows construct/throw a bad_alloc-shaped
exception through BF6340/BF6885 with scratch/vtableD6923C/ThrowInfoE03CC0.
The Source overflow arm deliberately throws CURRENT SDK std::bad_alloc. This is
an explicit error projection, not those ten Native operations or their private
object/throw ABI. The existing concrete allocator uses CURRENT malloc/_callnewh
retry/bad_alloc policy. Its provider/new-handler/CRT/module validity and failure
semantics are inherited; no originalOOM/fault/FH3/SEH or positive Native-origin
certificate is asserted. Entered allocator/Source exception is not permission
for a caller to run Native cleanup. This qualification does not require a new
whole-CRT reconstruction merely to expose the isolated Source primitive.

No byte is zeroed and no state/subobject/T[n] lifetime, vector/header mutation,
registry reference, custody receipt or normal disposition is created. Real
private Source request/control backing is separate from callback-visible data.
Callers retain genuine provider/control/backing and any existing graph through
callbacks/escapes; their actual constructors supply any later individual birth.
No local catch, compensation, cleanup, noexcept, terminal or FP reset is added.

AB6070/AB59E0 growth/migration, exact BF0/private-HOME copy domain, real caller
production, Origin, original registers/private-stack/FLAGS/fault/exception ABI,
custody and activation remain separate. Old vector copy's size0/length_error
policy is unchanged and is not used as a substitute for this leaf. Old Source,
CMake/ledger/Ghidra are untouched; no compiler/test/probe/Game action occurred.
