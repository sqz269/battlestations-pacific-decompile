# Lua variant embedded cleanup and raw clear span

Root verified the complete saved101-byte /29-operation cleanup at884AF0 and a
168-byte /59-operation raw clear span at886920 against live GPR bytes and the
original PE. The latter has only46 saved instruction starts: thirteen starts
after free calls are absent. This report preserves that distinction and adds
zero Source, Original ABI, startup or gameplay credit.

## Embedded cleanup101

Entry ECX is captured into ESI inside a Native FS frame with handlerC970DB.
The local stored-this word is written, current receiver0 is stampedD0E6F4,
then full state0 is stored before CALL886920 with current ECX/no pushed args.
After compatible RET0, current receiver10 is loaded into EAX and tested.
A full state-1 store preserves those TEST flags into the null-buffer branch.

On the nonzero path, current receiver0C is loaded into ESI, replacing the
receiver register. Literal1 is pushed, ESI is incremented as a DWORD, then
that current incremented ESI and captured EAX buffer are pushed before419CC0.
Returned EAX becomes ECX beforeBD1510. Three words remain pending across this
sequence under getterRET0 and return-blockRET0C; no intervening snapshots or
validation are added. The minimum combined cleanup is12 bytes. Those child
returns and policy are requirements, not new subordinate-body proof.

The prior FS word is loaded before current saved ESI is popped. FS is restored,
ADD ESP,10h supplies final owned arithmetic flags, and plain RET returns through
the current return slot. There is no common semantic EAX result. The profile
DWORD at receiver0 is the only direct receiver-relative store. No buffer/length
field is cleared by this owner body. Handler, descriptor, Native child, pool,
publication and caller bodies/data remain unopened in this packet.

## Raw clear span168

The entry allocates eight local bytes, saves ESI/EDI, captures ECX in EDI, and
uses two consecutive SUBs on current DWORD receiver4 to distinguish raw tag1
from raw tag5. Other tags go directly to the final receiver4=-1 store. This is
a physical tag distinction, not recovered class or union type evidence.

For tag5, receiver8 is captured into ESI; null goes to the common final tag
store. Otherwise current ESI4 supplies a header pointer and its current word0
supplies another pointer. Five stack words are passed to6EE960 with ECX=ESI:
the caller-local output address at entryESP-8, the captured owner, captured
header-word0, captured owner again, and captured header. A normal RET14h is
needed to restore this entry's save frame. All five argument types/ownership,
the child body and interpretation of output remain unproved.

After that child, current ESI4 is reloaded and passed to Native free. On normal
return, current ESI itself is pushed, ESI4 and ESI8 are cleared as DWORDs before
the second free call, then ADD ESP,8 discards both free arguments. Current
receiver8 is cleared, receiver4 becomesFFFFFFFF, saved EDI/ESI are restored,
local space is discarded, and plain RET returns. Child register/stack validity,
free eligibility, saved-word and pointed-storage aliases remain required.

For tag1, receiver8 is captured into ESI; null skips to the final tag store.
Current ESI4 supplies a buffer. If nonnull, current ESI0 is loaded, literal1
is pushed, its DWORD value is incremented and pushed, then the captured buffer
is pushed across419CC0 andBD1510. After the required combined12-byte cleanup,
current ESI is passed to free, its argument is discarded with ADD ESP,4, and
receiver8 is cleared. All normal paths finally set receiver4=-1, restore
current saved EDI/ESI and end with ADD ESP,8/plain RET. The final owned flags
come from that ADD. Unknown tags and null owners do not unconditionally clear8.
No own local EH, fault guard, loop, profile store or meaningful EAX result is
inferred for this raw span.

## Saved-listing and runtime boundaries

The saved886920 extent displays6920..69C7 but contains only46 listing starts,
and metadata reports five calls. The complete raw span contains59 starts and
six physical calls. Eleven missing starts follow first tag5 free at6958; two
follow tag1 free at69AC. Exact context queries for695D/6965/696C/6982 report
no saved instruction. Raw continuations assume compatible normal free returns;
the saved AddressSet ownership and precise cause of the listing gaps are not
proved from displayed extent alone.

Native free metadata names the correct library function with one pointer
parameter and no displayed noreturn keyword. Its body is unopened. The missing
starts do not by themselves establish a function or instruction flow override.
Bridge scripting is disabled in prior exact-flow queries; no scripting was
enabled and no flow-property claim or repair is made here. Correct library
labels are retained. Ghidra names/comments/body/prototypes/flow/listing and the
saved project are unchanged; no new function or export is created.

Current Source80 inputs and four build artifacts were replayed against the
published compiled receipt; all three existing checks remain recorded as passed.
This read-only packet runs no build, test or executable. Source has admitted
pool interfaces, but no exact884AF0/886920 callable provider or full raw variant
lifetime composition is supplied. Native6EE960, child pool ABI, heap policy,
exception/runtime frame behavior and production/startup/gameplay remain open.

Complete code, gaps and pins are in `reports/cc12_mission_lua_variant_embedded_clear_readiness.json`.
