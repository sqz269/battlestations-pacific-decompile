# Landing state registry: conditional SOURCE closure (cc11)

The complete ordinary integer storage chain now directly supplies `009AF9A0`'s
eight registrations. It uses borrowed actual Win32 registry/pair storage and
the existing complete CRT allocation/free Source operations. There is no Add
callback, reserve callback, name recorder, default registry or task arena.
This is a new C++ Source interface, not an original register/stack replacement
or a gameplay binding. The complete enclosing `009B2E50` caller is still unbound.

The native consumers are `009B3010` in the land composite and `009B3204` in its
sibling. Both call the complete `009AF9A0` registrar. The latter captures actual
approach+B8 once (task+4B0) and registers members of that SAME approach; MoveTo
+CC, Follow+108 and Park+228 correspond to task+4C4/+500/+620 in the existing
outer constructor. No canonical task+404 cell or translated pointer cache is
introduced or changed. Actual constructor/observer/state/arena lifetimes remain
external requirements; the semantic GameUnitsHost registry is not substituted.

## Exact native coverage and original ABI

All bounds below are end-exclusive. Fourteen full disk/live bodies match:
1,705 bytes / 634 decoded instructions. Original private EH/diagnostic code is
present in the byte evidence and excluded from the ordinary Source contract.

| Entry and end | Native inputs/return | Ordinary operation |
| --- | --- | --- |
| 00410400..00410455 | ECX count; EAX allocation; RET | checked eight-byte pair allocation |
| 00410C00..00410C2C | ECX first, EDX last; stacked destination/metadata/unused words; RET10 | ordered two-word forward copy; EAX destination end |
| 00410FA0..00410FC4 | cdecl first/last/pair | ordered two-word assignment |
| 00410FE0..0041100C | ECX destination, EDX count; stacked pair/metadata/unused words; RET10 | ordered fill; incidental EAX not adopted |
| 004112E0..0041131E | cdecl first/last/destination end | backward two-word shift; EAX destination start |
| 004114D0..00411502 | ECX unread metadata; stacked destination/count/pair; RETC | real fill; EAX destination+8*count |
| 00411670..00411695 | ECX unread metadata; stacked first/last/destination; RETC | real copy; EAX destination end |
| 00411960..00411BA1 | ECX vector; stacked iterator owner/position/count/pair; RET10 | captured-pair insert, growth and both in-place branches |
| 00411BB0..00411C87 | ECX vector; stacked requested count; RET4 | complete reserve |
| 00411CA0..00411D29 | ECX vector; stacked output/iterator owner/position/pair; RET10 | insert one, return EAX output iterator |
| 00411D90..00411E13 | ECX vector; stacked pair; RET4 | spare append or real insertion |
| 00411E20..00411E6E | ECX registry; EAX same registry; RET | stamp, clear headers, real reserve16 |
| 00411E70..00411E9C | ECX registry; stacked name/state; EAX same registry; RET8 | pair construction and real append |
| 009AF9A0..009AFA45 | ECX approach; RET | eight ordered real Add operations |

Primary independently reviewed the misleading register/stack pseudocode for
`4114D0`, `411670` and the full insertion caller. Workers performed no new ABI,
x87 or listing recovery. Primary's supported flow repairs restore the returning
free continuations at `411A9F..411AA2`, private catch `411AD0..411ADC`, and reserve
`411C5F..411C62`; all call gaps are zero. See
[state_vector_flow_recovery_cc11.json](../reports/state_vector_flow_recovery_cc11.json).
The report records 30 ordinary direct call checks passing and 13 native calls
in excluded diagnostic/overflow/private EH branches. Excluding those calls
does not supply their fault/exception behavior.

## Storage, order and identity

Actual registry is 14h bytes: raw profile word+0, proxy+4, begin+8, end+C,
capacity end+10. The vector receiver passed by `411E70` is registry+4. Proxy is
untouched. Each element is exactly two Win32 pointer words, name then state;
neither field owns its pointee. All copy/fill routines read/write the low word
before reading/writing the high word. There is no raw struct copy.

`411960` reads the input state word, then name word, then initial begin, before
capturing both into its local pair. Capacity and size use native SUB/SAR3;
address advancement uses modulo32 LEA arithmetic. Growth first tries
capacity + floor(capacity/2), including the native overflow-to-zero choice, then raises
it to size+insert count if necessary. This is not 2*n+2 or history saturation.
It allocates, copies prefix from fresh begin to captured insertion position,
fills the captured pair, copies suffix to fresh end, reloads old begin/end,
frees old begin when nonnull, then publishes capacity end, end, begin in order.

Both spare-capacity insertion branches are complete. If suffix length is
smaller than insert count, copy the suffix out, fill remaining new storage,
publish/reload end, then assign the old interval. Otherwise copy the tail out,
publish end BEFORE backward shift, then fill the insertion interval. An input
pair aliasing an existing element is protected by the initial snapshot.
Spare append separately publishes its captured end+8 after the real fill.
Reserve reloads begin/end after allocation/copy and retains cap/end/begin order;
sufficient capacity produces no header writes. Native diagnostic-only volatile
field observations are retained while their unsupported failure paths remain
outside the admitted domain.

`411E20` writes numeric `00CE37DC`, zeros begin/end/capacity and calls real
reserve16. The raw word is explicitly UNCALLABLE in this Source facade. Disk
and live table word0 is `00411810`, whose complete scalar/class profile is not
reconstructed here. No callable Source class profile, default token or class
destructor is fabricated. Reconstructing this storage does not admit virtual
calls through that word or create an original executable object.

The registrar preserves actual immutable char-pointer identities and order:
moveto(D1FE1C)->CC, follow(D1FE0C)->108, line(D1FE00)->1A0,
standby(D1FDF0)->1C0, begin(D1FDE4)->1E0, final(D1FDD8)->200,
park(D1FDCC)->228, abort(D1FDC0)->254. Names are neither copied nor looked up.
Generic pair word values may be null tokens; live array storage must satisfy
its independent nonnull/range/ownership domain.

## Validation and limits

Fresh registry and probe TUs compile with MSVC Win32 `/O2 /MD /W4 /WX`.
One embedded manifested ignored executable passes 39 Source checks. It uses
actual-shaped task/approach/registry storage, real Source constructor reserve16,
nine retained entries and the eight actual registrations, triggering 16->24
growth at element17. It checks borrowed prefix/name/state identity, untouched
proxy and SAME canonical404 cell. Continuing the SAME vector checks nonempty
reserve, sufficient-capacity reserve, both insertion branches and captured
existing-element alias through backward shift. It calls real CRT Source
allocation/free; no callback/lock/profile/provider substitute is used.

Source COFF receipt: insertion reads state+1C/name+1F before begin+27, free+150
then cap+162/end+169/begin+16D. Reserve free+9C then cap+A7/end+AE/begin+B2.
Constructor stamp+13 and header zero stores omit proxy. Registrar has eight
real Source Add calls. These are compiler ordering checks, not original register,
EFLAGS, exception, binary ABI or native runtime differential tests.

The three main support libraries were copied only after primary authorization
from its stable ae90949a7 build. Original hashes before/after copying matched
their unique pinned copies; copies remain immutable across compile/link/run.
The fixture's state/name identities are Source instrumentation, not original
image/provider/profile proof. Probe paths and full receipts are in
[landing_state_registry_cc11.json](../reports/landing_state_registry_cc11.json).

Admission requires coherent stable caller-owned Win32 storage, terminating
ranges, successful actual CRT allocation, valid disjoint copy allocations and
the intentional in-place overlaps implemented above. Counts/capacities and
size+insert count must be <=1FFFFFFF, with native-valid signed distances.
No structural reentry/concurrent mutation, invalid iterator/placement, backing
allocation failure/overflow, private EH/diagnostic behavior or pointee lifetime
is silently supplied. Existing raw buffers must belong to the same CRT free
domain. Complete class destruction, lower/composite/queue state construction,
native executable profile binding, owner scheduler/death lifetime, original
ABI and in-game behavior remain separate unfinished dependencies. Root owns
CMake registration, full main build and independent integration validation.

## Primary integration

Main `d87f925047dfcf3c79d345e3ba5c0b62a3e1fa3a` passed the full Win32 build and all three existing CTests. Root independently compiled the real registry and complete connected fixture with ten current Source/header inputs and three current main libraries, verified all inputs and the original PE before/after linking, and checked the embedded asInvoker PE32 manifest. The fixture passed39Source checks. All14 complete bodies (1705bytes) independently matched original disk and live Ghidra bytes;30ordinary native CALL checks passed. The13excluded diagnostic/overflow/privateEH calls remain unsupported. Native class/profile/lifetimes/lower/composite/ABI/runtime/game qualifications above remain. No tracked tests were added.
