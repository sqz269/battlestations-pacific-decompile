# Raw unit-owner construction stages and owned lists

`src/native_unit_owner_entity.cpp` supplies concrete normal storage stages of
the real unit-owner constructor and destructor, plus two complete raw list
leaves. It does not construct a whole root, publish a unit, provide a callable
profile, or activate the native tick/application path.
Source names and field roles are descriptive hypotheses, not recovered C++
symbols; address and raw storage behavior are the evidence contract.

| Source entry | Original coverage | Contract |
| --- | --- | --- |
| `construct_native_unit_owner_first_list_0077ef27` | Qualified `0077EF27..0077EFA3` business effects | After actual base and recon-array success: bind three owner/index pairs, install numeric profiles, clear selected fields, allocate the real 0Ch sentinel and publish +29C/+2A0 |
| `construct_native_unit_owner_second_list_and_fields_0077efa9` | Qualified `0077EFA9..0077F053` business effects | After first-list success: allocate the real 18h sentinel, publish +2A8/+2AC, initialize observer/physical fields and capture actual CE4ADC bits once |
| `tidy_native_unit_owner_peer_list_0077b980` | Complete normal `0077B980..0077B9C7`, 72 bytes | Reset head links/count, free each node, free sentinel, zero header+4 |
| `clear_native_unit_owner_holder_list_0077d930` | Complete normal `0077D930..0077D97C`, 77 bytes | Reset head links/count; per node free a non-null +14 payload, zero that field, then free node; retain sentinel |
| `destroy_native_unit_owner_lists_0077e42c` | Qualified `0077E42C..0077E456` business effects | After actual observer destruction: clear +2A4 list, free its sentinel and zero +2A8, then tidy +298 list |

These Source interfaces target MSVC Win32 and use the genuine raw allocation.
They do not reinterpret `GameUnit`, synthesize a replacement header, zero an
entire receiver, or introduce an alternate allocation/process domain. Calls to
the accepted `buy_native_unit_owner_list_head_0077b960/0077b940` retain the
BF681B malloc/new-handler boundary. `singleton_lifetime_free` supplies the same
CRT free domain used by those allocations. Numeric profile words remain
evidence values; they are not host-callable virtual tables.

## Construction and failure ownership

The actual containing allocation and factory preimage must already exist.
Stage one requires successful `00928630(root, 1, unchanged full flag)` and the
actual three 34h recon records at +1E8/+21C/+250, constructed through
BF7CD1 with 805A70/804300. The current Seg60Host uint16 requested-ID interface
is narrower than the formal full DWORD; this remains an interface limitation,
not demonstrated gameplay divergence or authority to change that interface.

The three backrefs target root+1E4, not root: +214/+248/+27C receive that same
address and +218/+24C/+280 receive 0/1/2. Profile writes and scalar stores follow
the Original order. Stage one owns the 0Ch sentinel only after allocation
returns and the +29C store completes. If allocation throws, the enclosing
owner still owes its actual recon/base unwind.

Stage two begins at the second allocation, after the Original state-2
transition. If that allocation throws, the first list still belongs to the
enclosing owner. Neither stage substitutes guessed cleanup or erases evidence
of partial construction. Both successful allocations belong to the real root,
not to a temporary RAII wrapper. The ordinary list tidy leaf is available, but
its existence does not establish the enclosing Native exception frame.

Stage two preserves the BYTE stores at +2C0/+2F0 and WORD store at +2C8.
It reads CE4ADC once after +2C8 and before +2E8, then copies the captured bits
to +2E4 and +300. MOVSS is a bit transfer here, including non-finite patterns.
Untouched factory bytes and padding stay untouched. It stops before the real
E188A8 publication read, game+1FE4 comparison, E0AF20 byte and possible
77EB50 peer-holder construction. None of those dependencies is defaulted.

## Lifetime boundary and current Original evidence

The list destructor continuation starts only after the real session-message
arm, F87194 withdrawal and +2B0 observer destruction. It does not perform or
replace them. Its end precedes the recon-array destructor iterator and the
GameEntity/SceneNode destructor. Root scalar flags/free, embedded +310 tick
unlink and current-subnode deleting dispatch also remain outside this stage.

Fresh typed metadata verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The complete live/file-backed Original bodies are
77EED0..77F08D (446 bytes), 77E380..77E490 (273), and 77E810..77E82D (30), all
with `no_return=false`. The current complete destructor listing includes the
post-free continuation through E490. Its old E441 truncation comment is
historical; it did not justify a listing repair. No listing repair was made.
The actual recon destructor call at E46C targets **BF7C6E**; BF7D69 is not that
call's target. The scalar wrapper calls E380 at E813 and BF6989 at E820 when
flags bit zero is set. This packet does not replace that wrapper.

805A70/804300 and 925CE0/928630 remain owned by cc13-seg49 at this audit and
were queued, not changed. Remaining root gates include their concrete owning
composition, actual observer/weak/string/entity-ID/pending domains, World and
hierarchy publication, actual peer/session/global withdrawal, callable World
and effect teardown, complete scalar/exception ownership, genuine derived
factory, current tick methods and remaining-job dispatcher. Existing canonical
AA0/F878CC process identity is unchanged. Full receiver activation stays false.

## Validation and integration

Strict MSVC Win32 `/std:c++20 /EHsc /O2 /MD /W4 /WX /fp:strict` compilation
passed. All five emitted Source schedules were inspected for the ordered
mixed-width writes, initial-word capture, current-head reloads and real free
calls. A manifested isolated comparison links the new production object,
the actual head-producer TU and the existing concrete CRT allocation domain.
Seven complete live byte spans equal the file-backed PE spans.

The focused comparison executes copied Original normal fragments/leaves with
their real internal calls, redirects only the external allocator/free and
borrowed global address, and supplies private register/EH-state scratch. It
compares a nonzero 320h raw preimage byte-for-byte after both construction
stages and list teardown, normalizing only allocated sentinel identities.
It also checks retained self-links, empty/two-node lists, null/non-null holder
payloads, and initial-word capture before an aliased store: **20 checks, zero
failures**. These are synthetic storage fixtures, not constructed receivers.
They do not test allocation failure, Original exception frames, invalid lists,
fault behavior, original register ABI, startup or gameplay.

The worker's `scripts/build.ps1` Release baseline passed all three configured
tests after verified native-math seeds were enabled. The new TU is separately
strict-compiled and directly linked into the comparison; it is not in that
baseline's CMake target. The primary owns CMake registration and must add
`src/native_unit_owner_entity.cpp` to
`bsp_core`, then validate the resulting integrated build. See
`reports/native_unit_owner_entity.json` for exact validation status and pins.
