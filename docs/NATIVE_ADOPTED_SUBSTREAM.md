# Native adopted-source substreams

Addresses: 00BF1000, 00BF1040, 00BF1080, 00BF1090, 00BF10A0, 00BF10B0,
00BF10E0, 00BF1130, 00BF11C0, 00BF1240, 00BB8B80, 00BEF750.

The eleven complete owner/type routines cover 580 native bytes and now operate
on actual 28h owners. They supply the D68DB0 stream used by the large,
uncompressed MPKG entry route. This packet also binds that numeric profile to
the existing FileStore conversion and Lua/VFS dispatch. It does not construct
the native MPKG provider, populate its directory, or add an inflater owner.
Descriptive names are reconstruction hypotheses; C++ interfaces are separate
from original binary entry points.

## Owner and call behavior

BF1130 receives ECX raw owner and five stack DWORDs: adopted source, start low
and high, length low and high; it returns the owner in EAX with RET14h. It
writes reference-base CEB130, count1, stream profile D68DB0, source+8,
start+10/+14 and the wrapping end+18/+1C. Field+0C stays untouched. It acquires
no source reference. It calls the source's current slot1C with the absolute
start and origin0, then reloads the owner's start into current+20/+24. A
callback can therefore change the final current position by changing start.

BF1000 read and BF1040 write take buffer, requested count and optional actual
pointer on the stack (RET0C). They forward the requested count unchanged to
current source slot24/28, passing a temporary actual count initialized to the
request. They ignore the callee's return value, add the resulting actual count
to the owner's current 64-bit position, publish optional actual afterward, and
return actual in EAX. They perform no entry-end clamp or transformation.

BF1080 returns current-start; BF10A0 returns end-start. Both use wrapping
64-bit subtraction and EDX:EAX, RET. BF10B0 takes origin (RET4): zero selects
start, one current, every other value end. BF10E0 (three stack DWORDs, RET0C)
adds a wrapping distance to that base, publishes current before looking up the
current adopted source's seek slot, calls seek with absolute origin0, and
forwards its EAX. Failed or throwing seeks leave that published position.
BF1090 tail-dispatches adopted source slot18 and preserves its AL result.

BB8B80 ignores ECX and tests its stack token against the current two DWORD IDs
at 0109DB58/0109DB5C (AL, RET4). The next DWORD is the file descriptor's name
address, not another ID. Uninitialized zero IDs remain ordinary values.

BF11C0 installs D68DB0, captures source+8, and performs a real
InterlockedDecrement on source+4. It captures the source's current table and
slot0 only when the result is zero. The callable adapter passes that captured
table in EDX, as BF1201..BF1207 do. After a successful callback it clears the
current owner+8, even if the callback replaced that field. It then writes
stream-base D5C104 and reference-base CEB130; other fields stay stale. BF1240
destroys first, frees only on flags bit0 after successful destruction, and
returns the original owner address (RET4).

## Unwind and source domains

Constructor FuncInfo E02514/unwind map E0250C and destructor FuncInfo
E02540/map E02538 each have one action, CC78A0/CC78C0 -> BB86E0 -> BD30F0.
They restore D5C104 then CEB130. The constructor arms the action after its
source seek-slot lookup. A throwing seek does not release the adopted source,
clear source+8, or free the raw owner. A throwing zero-reference callback does
not clear owner+8, but the base profile cleanup still runs. The source uses
C++ unwind guards for these effects; original FH3/SEH identity is unvalidated.

NativeAdoptedSubstreamDispatch receives the method word captured at each call
site. Its callable adapter requires actual host functions with the documented
Win32 ABI; it never executes a numeric native table word. Numeric runtime
binding routes current memory, physical and nested substream methods to their
reconstructions and preserves original owner profiles. The memory seek bridge
also captures BEF540's EAX, although its existing source declaration is void.

BEF750 now supports D68DB0 through explicit adopted-stream dispatch and file
ID storage. Each type/seek/size/read call reloads its table and slot in original
order. Its memory-token read still precedes the first table read, and backing
data precedes read-slot lookup. Missing context or unknown numeric methods
raise source invalid_argument; this is source-domain enforcement, not native
failure handling. Zero-ID accidental type matches and their unsafe native
backing-sharing behavior are not normalized.

The runtime binding borrows and restores the FileStore dispatch slot, just as
it does the route slot. Memory/physical/nested substream read, seek and release
are bound. Numeric physical/memory write leaves remain outside this binding;
the complete substream write body is separately exercised through callable
original-ABI source methods. Native MPKG construction, startup ordering, seeded
manager/mount/FileStore trees and original CRT exception identity remain separate.

## Verification

All eleven bodies and nine additional table/base/unwind spans (774 bytes total)
match live Ghidra and the installed PE. Seven previously undefined leaf entries
were defined with the repository's locked tool and verified byte ranges.

The strict Win32 build and both existing CTests pass. The ignored original/source
probe passes 98 comparisons over owner bytes, callback traces, result registers,
actual-count output and buffer bytes. It covers wrapped arithmetic, all origin
branches, failed seek return, callbacks changing start/current/source fields,
counts 1/2/0/-1, short/zero/maximum actual counts, retained requested count,
optional-output aliasing, and subsequent dispatch through a callback-changed
source table. Nonfree deleting return identity and real freeing on both sides
are checked. Three additional C++ unwind cases are source-only.

The reference bodies execute unchanged at a common +30000000 translation.
External source methods are controlled host callbacks; the decrement uses a
host wrapper around the real atomic primitive and free uses the existing source
CRT boundary. Original FH3 handlers are not installed or executed. The first
96-case fixture missed zero-reference EDX; independent review found that gap,
and the corrected adapter plus callback register observation passed on rerun.

The concrete composition additionally passes actual memory-backed conversion,
unclamped nested physical reads, final real OS cursor checks, recursive owner
release and physical HANDLE closure, BEF540 EAX comparison, and nested binding
restoration. A FileStore entry retaining an actual D68DB0 owner adopts a physical
fundamentals stream, converts it to memory, and executes it through Lua. Platform
is cleared before loading and restored by the script; source and substream
references stay one. Canonical shutdown clears owners/counters. Earlier physical
and memory FileStore paths also pass in this same fixture.

These fixtures establish their stated paths. They do not establish arbitrary
stack aliasing, concurrent mutation, every failure/fault order, original EH,
complete startup, ABI replacement, or game execution equivalence. Artifacts,
ABI rows and call checks are pinned in reports/native_adopted_substream.json.

## Follow-up

Implement actual MPKG provider/entry construction and inflater ownership before
claiming native archive loading is composed end to end. Continue manager,
factory, provider-pool and container lifetime packets using their current maps;
the substream owner does not resolve those dependencies.
