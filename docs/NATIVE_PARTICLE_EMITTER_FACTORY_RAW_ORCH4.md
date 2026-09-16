# Raw native emitter factory AF9FB0

`00AF9FB0..00AFA0F8` is the complete 329-byte factory body, with 103
instructions. It chooses one of three emitter allocation/constructor pairs,
then dispatches the current object's profile slot `+14h`. The descriptive
source names are hypotheses, not recovered symbols. Evidence and the preserved
prior reconstruction/name/comment records are in
`reports/native_particle_emitter_factory_raw_orch4.json`.

## Native contract

The original entry receives the actual eight-byte kind string header in ECX
and the actual eight-byte name string header in EDX. Its stack arguments are
`word10`, `word70`, and the actual text buffer. EAX returns the owner and the
final instruction at `00AFA0F6` is `RET 0Ch`.

| Matching kind, in order | Raw allocation | Constructor | Installed profile | Current parser slot +14h |
| --- | ---: | --- | --- | --- |
| ConeEmitter | 94h | B03940 | D5DEBC | B03EC0 |
| SphereEmitter | 8Ch | B02B90 | D5DE88 | B02FD0 |
| SmartAreaEmitter | 90h | B01CB0 | D5DE48 | B02210 |

Cone compares the current `kind+4` pointer against the literal using the CRT
case-insensitive comparison, skipping that comparison when the pointer is null.
The next two branches use actual-header helper `00425850`. Each constructor
receives the same borrowed name, the two caller words, and a zero flag.
Sparse constructor writes preserve all other allocation bytes.

For an unknown kind, `00AFA0D2` treats `word70` as an existing owner pointer.
It still runs that owner's current parser. A null allocation also proceeds
to the owner read; the native body does not turn that path into success.

At `00AFA0DA`, the factory reads the current owner profile; at `00AFA0DC`,
it captures that table's current `+14h` target. The call at `00AFA0E2` passes
ECX=owner and the actual text buffer as one stack argument. The parser return
value is ignored, and the factory returns the owner held in ESI. Profile
slot `+8h` creates records and is unrelated to this dispatch.

## Calls and exception ownership

| Call site | Target | Operation |
| --- | --- | --- |
| AF9FD9 | BF7FBF | Cone kind comparison |
| AF9FEF | BF681B | Allocate 94h |
| AFA016 | B03940 | Construct Cone |
| AFA031 | 425850 | Sphere kind comparison |
| AFA03F | BF681B | Allocate 8Ch |
| AFA066 | B02B90 | Construct Sphere |
| AFA08C | 425850 | SmartArea kind comparison |
| AFA09A | BF681B | Allocate 90h |
| AFA0C1 | B01CB0 | Construct SmartArea |
| AFA0E2 | Captured current +14h | Parse current owner |

The FH3 handler at CBAFD1 loads function info DF2E84. Its unwind map at
DF2E6C has three independent entries:

| State | Next state | Action | Captured pointer |
| ---: | ---: | --- | --- |
| 0, Cone construction | -1 | CBAFB0 -> BF65AC | `[EBP-10h]` allocation |
| 1, Sphere construction | -1 | CBAFBB -> BF65AC | `[EBP-10h]` allocation |
| 2, SmartArea construction | -1 | CBAFC6 -> BF65AC | `[EBP-10h]` allocation |

The allocator runs before the applicable state is armed. Constructor failures
first execute the constructor's own recovered cleanup, then free the captured
raw allocation through BF65AC. Every successful constructor and every null
allocation branch sets the factory state back to -1 before parser dispatch.
Parser failure therefore does **not** destroy or free the returned/borrowed
owner. Ghidra's action functions currently stop at the free call; the inspected
complete live and installed bytes also contain `POP ECX; RET` in each action.

## Composition and evidence status

The new source overload uses the actual raw constructors in
`native_particle_emitter_construction.*` and the fixed CRT allocation domain
from `singleton_lifetime.*`. Its context borrows current profile cells and
concrete parser contexts; those pointers can be assigned after creating the
mutually recursive parser/factory context graph. It adds no parser callbacks,
replacement object, or stand-in parser.

One acquired frame belongs to one invocation. Its retained owner, captured
allocation and parser diagnostics describe failure without replay or destructor
rollback. Failed concrete parser frames stay alive so their child obligations
remain inspectable. The caller supplies the parser's otherwise unwritten
incoming builder-kind residue explicitly. Unknown reached profiles or targets
remain explicit source boundaries; recognized current targets execute their
actual concrete bodies, including an unknown-kind reused owner.

All 329 body bytes and 223 bytes of related live data/code match the installed
PE. The report records each range hash, all call instructions, the final RET,
prior ledger records and the prior Ghidra comment. Ghidra mutations and ledger
integration are reserved for the primary integrator.

The body/EH audit is complete. Strict MSVC Win32 compilation with `/W4 /WX`
passes against the genuine parser declarations. Full-cycle linking and fixture
validation are pending. These are new source-level interfaces, not native FH3/SEH or
binary replacements; unrestricted faults, concurrent mutation and gameplay
behavior have not been validated.
