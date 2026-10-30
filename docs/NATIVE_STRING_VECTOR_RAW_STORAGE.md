# Actual0Ch string-vector services

Four new ordinary MSVC Win32 overloads operate on actual0Ch
`{data0,count4,capacity8}` and raw8h string headers through the caller's real
AA8/AA4/AA0 pool cells. This is NOT checked10h begin4/end8/capC storage.
No NativeString/NativeMeshWeightNameStorage/STL object cast, vector constructor,
manager/publication, callbacks, generic framework or startup caller is added.
The old typed vector HPP/CPP remain literal unchanged.

Native ECX is the actual vector. Reserve426520/resize427110 take a signed
DWORD; append4CDC20 takes a borrowed raw8h header; each uses RET4. Full23B
destructor4D0FA0 uses plain RET. New Source signatures add the raw context;
Original register/stack/private-frame/EH/SEH/fault ABI is not reproduced.

- Reserve clamps requested capacity to minimum1, compares signed capacity,
  captures new requested*8 allocation, and deep-copies while signed index is
  below CURRENT count. Each iteration reloads CURRENT old data, and tests its
  computed destination before the tiny private fenced inline-copy body.
  Self-CMP precedes key0/key4 zeroing; real raw41DD40(preserve1) is followed by
  CURRENT source-length guard, destination length, source data, destination
  data, qualified overlap memmove. Zero-byte call alone is omitted after its
  operand reads. No unfenced426060 facade is called or edited.
- Old names return forwards using CURRENT count/data each iteration. A nonnull
  captured data/len+1 pair precedes actual419CC0; realBD1510 receives CURRENT
  AA4 through its reference after that getter. Null skips length/getter/return.
  Free CURRENT old array even when null, then publish captured fresh data and
  requested capacity. Count is not restored/published by reserve.
- Resize reserves only when signed request exceeds CURRENT capacity. A local
  index starts from CURRENT count; grow zeros length then data at CURRENT data
  each iteration. Shrink actual count DWORD-RMW-- occurs BEFORE CURRENT
  count/data/header capture and return; each continuation compares CURRENT
  count. Only normal completion stores requested count.
- Append captures capacity before CURRENT count. Only equality grows to signed
  max(wrapped2capacity,1). After reserve reload count then data, construct at
  the resulting address when nonnull, and actual count RMW++ only after copy
  returns or computed-null destination skips it. Do not preserve/clone an input
  header that becomes invalid across reserve; caller supplies readable backing.
- Destroy calls resize0, then frees CURRENT data. Pointer/capacity stay stale;
  no second destruction/retry/dedup permission is manufactured.

Native reserve/append selected cleanup is bare RET (401130), so these functions
add NO catch, element cleanup, allocated-copy rollback, buffer free, count/header
repair or retry on escape. Partial new/old headers, already returned payloads,
unpublished allocations and counts may survive. No frame/receipt is invented
to authorize recovery; normal return alone is not full owner/loaded-table proof.

BF55BE is the audited BF681B thunk. Arrays use genuine matched
singleton_lifetime_allocate({object,bytes,bytes})/singleton_lifetime_free.
Payloads use real raw419CC0/BD1510 on the SAME referenced cells; context objects
may differ only when those cell addresses are identical. No noexcept storage
adapter or cached pool narrows getter exceptions. Current heap/new-handler,
copy/null-allocation and lower raw41DD40 qualifications remain explicit; these
are not Original vendor/static-CRT or transitive fencing/ABI equivalence.

Local byte reads/writes are compiler-fenced and RMWs are actual DWORD ADDs,
with wrapping address/unsigned arithmetic and signed field comparisons.
Fences are not cross-thread synchronization. Caller retains genuine initialized
header/payload/backing provenance, disjoint stable extents, real providers,
external serialization and every partial graph through callbacks/failure.
No source-success empty substitution, parser/raw16/VFS/loading/Font admission,
Native body credit, runtime, game, Page/P4 or production claim follows.

Evidence: saved426520..42663F,427110..427185,4CDC20..4CDCD8,full4D0FA0..4D0FB6;
Gui contracta27c4ec6 and Root qualified allocator amendment. Only the recorded
strict Win32 object compile validates this packet; no tests/executable/game.
