# Native enum symbol membership (cc11)

`contains_native_enum_symbol_0048e8d0` binds the complete ordinary normal paths
of `0048E8D0` to the existing string constructor, dictionary finder and real
raw-pool cleanup. It returns whether a node was found, independently of that
node's mapped word. Dictionary ownership, declaration identity and production
traffic binding remain separate contracts.

## Original body and result

The complete body is `0048E8D0..0048E954` inclusive, exclusive end `0048E955`:
133 bytes / 41 instructions. Live Ghidra bytes equal the installed PE image;
SHA256 is `ad39abf90584ec004a7a14ac94c2d3aff197417d2595ad3ef662cf0b6963468b`.
Each live query verifies project `bsp`, `/battlestationspacific.exe`, language
and image base against `config/target.json`; the configured saved project is
`C:/Users/sqz269/bsp.gpr`.

The entry pushes state `-1` and handler `00C62BB8`, installs an `FS:[0]` frame,
reserves an eight-byte temporary and saves EBX/ESI. ESI captures incoming ECX.
`0048E8E8` loads the one incoming stack CString; `0048E8F5` calls `0041E870`
with ECX pointing at the temporary. At the finder call, the temporary length
and data are at current ESP+8/+0C. `0048E8FA` takes the address of the incoming
CString stack slot for the bucket output, after copying its text; it does not
write caller-owned string bytes. `0048E904` sets the dictionary receiver to
owner+4 and `0048E90F` calls the complete symbol finder `0048D480`.

`0048E914` tests the returned node pointer. The following MOV reloads temporary
data without changing flags; `0048E91A SETNZ BL` captures membership before
cleanup. There is no node+8 read. Thus a present zero or `FFFFFFFF` mapped word
still yields true, and a missing node yields false.

`0048E91D` tests temporary data, `0048E91F` clears the cleanup state to `-1`,
and `0048E927` skips pool calls when data is null. Otherwise the body captures
length+1 and data, pushes the native return arguments, calls pool getter
`00419CC0` at `0048E934`, and block return `00BD1510` at `0048E93B`.
`0048E945 MOV AL,BL` returns the saved Boolean after cleanup, restores EBX/ESI
and the prior FS frame, and terminates with `RET 4` at `0048E952`.
Only AL is established as the Boolean result; full EAX canonicalization is
not established. These observations do not recover the original EH/SEH ABI.

The report has four direct-call rows, distinguishing unconditional
constructor/finder calls from the nonnull-temporary cleanup calls. Verification
passes all four rows. Native traffic context `0049D25A..0049D284` first gets
the `LandVehicleClasses` table through `0048E960`, passes that pointer in ECX
to membership at `0049D261`, tests AL, and reads a symbol word through
`0048E840` only on the vehicle branch. `008331F9` is another membership call;
its caller tests AL at `008331FE`. Neither containing caller is ported here.

## Source and provider boundary

The new public interface borrows an actual Win32 owner and valid closed ASCII
CString, including empty. It constructs one genuine owning `NativeString`
header with `construct_native_string_header_0041e870`, invokes
`find_native_enum_symbol_node_0048d480(owner+4, ...)`, captures `node != nullptr`,
and calls raw-context `destroy_native_string_header_0041dd20` before returning.
The supplied pool is the existing actual-layout raw-pool composition, not a
semantic allocation callback. Both found and missing keys have normal paths.

Admission requires the existing finder's stable valid string/node/bucket
storage, finite consistent chains, actual distinct empty-cell bindings
`00E186ED` and `00E17654`, a returning pool, ASCII/C-locale providers and no
aliasing/reentry/concurrency. Null/invalid CString or owner, allocation failure,
other locales/non-ASCII, faults and exceptional cleanup remain outside the
binding. The Source interface is new; no original register/stack or full-EAX
compatibility is claimed. No semantic `PropertyLibrary` conversion, global
guard, insert/default, enum declaration identity or traffic policy changes.

## Connected fixture

Ignored artifacts are under `local/cc11_scene_enum_dictionary_membership/`.
Run `python local/cc11_scene_enum_dictionary_membership/run_probe.py --out
<fresh-directory>` from the worker worktree. Successful `run02/` pins all inputs
in `inputs/manifest.json` and `inputs/sealed_pre_execution_manifest.json`.
`membership_probe.cpp` uses the real Source `SceneLexer` to extract the installed
`global.enums` LandVehicleClasses (22 symbols) and SoldierTypes (6 symbols).
The input's pre/copy/post SHA256 is
`a9f99c0e8fc650e2ef86e404990d56c7f2e4b518ff65be685acc3c6999ace027`.

Genuine raw-pool string constructors populate externally backed, producer-proved
14h slots and the borrowed owner field extent. This is not original node
allocation, insertion, CEnum construction or lifecycle execution. Actual table
getter results are passed directly to membership and then, only for hits, to
the symbol word getter. Seven cases pass: LandVehicle Us_jeep/466 and
None/FFFFFFFF; absent Soldier USMatroz in LandVehicle; Soldier USMatroz/0;
absent Us_jeep in Soldier; uppercase US_JEEP; and empty missing key. Six known-found
word calls comprise two table pointer reads and four symbol reads.

Before every getter/membership call, a genuine identical-text allocation/return
primes the exact-size pool cache. The complete actual pool prefix through the
byte before `critical_section_8ad484` is compared after the call, covering both
Boolean outcomes. Borrowed owner bytes, all 14h slots and owning key buffers
remain unchanged. Empty input allocates no temporary buffer; real pool shutdown
publishes null and sets the disabled flag. Checked-free actual-address empty
cells are exact image-zero bytes and read-only; they are fixture bindings,
not captured live-game globals.

Eight fresh TUs compile with `/W4 /WX /fp:strict`; 39 actual project headers plus
seven production CPPs make 46 unique production inputs, or 48 with fixture CPP
and recipe. Frozen current b7 core/lua511/zlib121 support has matching original
pre/copy/post and pinned pre/post link/run hashes. Link/run exit zero. All COFF
objects and the executable are x86; the embedded manifest is `asInvoker`.
The changed-TU COFF calls the real constructor/finder/destructor providers and
captures SETNE before destruction. Historical lookup run05 and word-getter
run02 receipt hashes remain unchanged.

The initial fixture attempt compiled/linked but stopped at an installed enum
separator before membership calls. Its extraction loop was corrected to match
the previous selected-table lexer probe; production parser/source behavior did
not change. The successful run is preserved separately.

Original `0048E8D0` was **not executed**: its complete reached string/pool/prolog
and EH graph is not replay-bound. Evidence is full original assembly plus a
Source compiler/lifecycle fixture, not original-wrapper differential, original
ABI, map/declaration ownership, production traffic execution or gameplay proof.
Main CMake registration, integrated full build and independent verification are
owned by the primary agent.

Primary integration at `b728a6343161f29f251a29d40c17ceffc1590584` passed the complete MSVC Win32 build and all three existing CTests. The independent current-library fixture freshly compiled 8 actual TUs, with 47 Source/header/fixture pins and 39 actual compiler project includes. Source, generated recipe inputs, original PE and three current libraries remained unchanged; COFF matched the worker output and native direct calls/tail transfers passed. The report records the complete receipts. Seven membership witnesses and six known-found word calls passed. SETNE50 precedes real destroyCALL53; the original native wrapper remains unexecuted. Original ABI, full remaining constructors/world/consumer binding and gameplay remain qualified.
