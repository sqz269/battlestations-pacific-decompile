# Native allocator owned-message constructor: Source candidate

The registered MSVC Win32 build passes all three existing checks. Primary review verifies 1 complete Core definitions covering 78 Original bytes; ABI, production startup and gameplay remain open.

The following candidate receipt is historical; registration and compiled review are recorded below.

This candidate implements the complete `00BF6340..00BF638D` schedule in a raw
MSVC Win32 naked adapter: 33 Source assembly operations corresponding to the
78-byte Original body. It borrows the actual receiver and pointer-slot address,
preserving the slot's late reread, nullable allocation, null-path DWORD RMW,
full ownership write, receiver result and stack/flag schedule.

The candidate is **unregistered and unbuilt**. Static instruction mapping and
evidence checks do not establish actual emitted code, Core membership, linked
CRT resolution, caller/EH behavior or admission. Those reviews remain with
the primary integrator. No typed owner or existing consumer is changed.

Baseline: `8da47302782d1ec456aade3dbbf9a9b56d745905`. The four owned outputs
are the new header, source, this document and the JSON report. The saved
Original library name remains **`exception`**; no Ghidra or ledger edit is made.

## Explicit raw boundary

```cpp
void* __fastcall construct_native_allocator_owned_message_00bf6340(
    void* actual_receiver,
    std::uint32_t unused_EDX,
    const void* actual_pointer_slot_address);
```

The receiver enters in `ECX`, an explicit unused incoming word occupies `EDX`,
and the actual slot address is the first stack argument. The naked definition
returns the receiver in `EAX` and uses `RET 4`. There is no raw `noexcept`
promise. The Source declaration is an explicit interface, not proof of the
Original placement, caller ABI or C++ exception-class identity.

The actual receiver must provide leading 12-byte backing: four-byte writes at
`+0` and `+8`, a write at `+4`, and a prior read at `+4` on the initial-null
message path. The slot must supply a readable DWORD address value at each
reached read. Its `const void*` parameter borrows the slot address; it does not
capture an early character-pointer value or prove slot stability, deep const,
pointee lifetime or string validity. No 12-byte backing is cast to the existing
40-byte typed owner, and no new typed owner, slot or producer is manufactured.

## Complete instruction mapping

| Original address | Source operation |
| --- | --- |
| `00BF6340` | `push ebx` |
| `00BF6341` | `mov ebx, dword ptr [esp + 8]` |
| `00BF6345` | `push esi` |
| `00BF6346` | `push edi` |
| `00BF6347` | `mov edi, ecx` |
| `00BF6349` | `mov dword ptr [edi], 00d69370h` |
| `00BF634F` | `mov eax, dword ptr [ebx]` |
| `00BF6351` | `test eax, eax` |
| `00BF6353` | `jz null_message` |
| `00BF6355` | `push eax` |
| `00BF6356` | `call strlen` |
| `00BF635B` | `mov esi, eax` |
| `00BF635D` | `inc esi` |
| `00BF635E` | `push esi` |
| `00BF635F` | `call malloc` |
| `00BF6364` | `test eax, eax` |
| `00BF6366` | `pop ecx` |
| `00BF6367` | `pop ecx` |
| `00BF6368` | `mov dword ptr [edi + 4], eax` |
| `00BF636B` | `jz message_constructed` |
| `00BF636D` | `push dword ptr [ebx]` |
| `00BF636F` | `push esi` |
| `00BF6370` | `push eax` |
| `00BF6371` | `call strcpy_s` |
| `00BF6376` | `add esp, 0ch` |
| `00BF6379` | `jmp message_constructed` |
| `00BF637B` | `null_message: and dword ptr [edi + 4], 0` |
| `00BF637F` | `message_constructed: mov dword ptr [edi + 8], 1` |
| `00BF6386` | `mov eax, edi` |
| `00BF6388` | `pop edi` |
| `00BF6389` | `pop esi` |
| `00BF638A` | `pop ebx` |
| `00BF638B` | `ret 4` |

The complete Original body ends at `00BF638D`, with SHA-256
`37d4ff77838e865a70c14dfc5f56b19ed7f1096b0953472608e6c21bb801ef06`.
Every Source operation maps to an owned instruction, allowing only the three
explicit current CRT symbol substitutions and the two local branch labels.
No adjacent byte, Native child body or additional Native data was inspected.

## Preserved order and conditional effects

Let `D` be entry receiver, `P` the actual first stack argument, and `T` entry
`ESP`. The body captures `P` into `EBX` before publishing `00D69370` at `D+0`.
It then reads `M0=DWORD[P]`. An initial null performs the actual read-modify-write
`AND DWORD[D+4],0`, skips every CRT child, then writes full DWORD `1` at `D+8`.

For nonnull `M0`, the original argument remains pushed across `strlen` and
`malloc`. `INC ESI` computes `(strlen(M0)+1) mod 2^32` without an overflow or
zero-size guard. The nullable malloc result `Q` is tested, both retained
argument words are popped into `ECX`, and `Q` is stored at `D+4` before the
branch. The pops and publication preserve the test flags. Null allocation
still reaches the same ownership write of full DWORD `1`.

On nonnull allocation the body rereads **the same actual slot** after `D+4`
publication and passes `Q`, the original computed size and this late value to
`strcpy_s`. It neither snapshots the early value for the copy nor remeasures
the late one. It ignores the copy's integer result. Under ordinary
nonconcurrent accesses, `P=D+4` therefore supplies the newly stored `Q` as the
copy source too; that ordering is retained without asserting such overlap is
a valid CRT operation. No alias/null guard, prior free, retry, catch or cleanup
is added. Earlier writes can remain if a later slot read or child fails.

The original control DWORD is never read. `+8=1` is a complete DWORD write,
not a byte store or update of the previous flags. The profile is opaque numeric
data and creates no callable table, Native type, RTTI, throw binding, static
failure owner, slot production or lifecycle identity.

The three nonvolatile registers are restored from their saved stack words.
Under ordinary child returns and uncorrupted saved stack, `RET 4` finishes at
`ESP=T+8`; the successful-copy path reaches `T-28` at child entry and explicitly
cleans its three arguments. `EAX` returns `D`; `EBP` is untouched. On the
initial-null path `ECX=D` and `EDX` is unchanged. On allocation-null return,
`ECX` comes from the retained strlen-argument word, normally `M0`; after copy,
volatile-register values depend on that child. No added preservation promise
is made for current CRT effects.

Initial-null final arithmetic flags come from the RMW `AND`; allocation-null
flags come from `TEST Q,Q`: `CF=OF=SF=0`, `ZF=PF=1`, `AF` undefined. The copy
path leaves arithmetic flags from `ADD ESP,12`, based on the actual stack
value. Saved-register restores, profile/control stores and the final return
do not overwrite those flags. Arbitrary faults, stack aliases, concurrency
and wider exception/runtime behavior remain unproved.

## CRT, receipts and admission boundary

The Source follows the admitted copy adapter's callable-symbol pattern:
`<cstdlib>`, `<cstring>` and `#pragma function(strlen)`, with calls to `strlen`,
`malloc` and `strcpy_s`. Pointer, DWORD and `size_t` widths are asserted as four
bytes. There is no throwing allocator wrapper or fabricated CRT provider.
Modern Source CRT identity, invalid-parameter/failure policy and child machine
effects are not Original CRT/ABI proof.

The earlier copy receipt contains resolved UCRT imports and a strlen thunk;
those recorded addresses remain historical evidence. If this candidate emits
the same five-byte strlen call and six-byte malloc/strcpy import calls, its
body would occupy 80 Source bytes. That is conditional, not an emitted-size
claim. Actual call forms, branch displacements, relocation targets, complete
Core body and linked selection/resolution require primary review.

All 240 inherited current-baseline references are unchanged: 13 direct audit
inputs plus 227 receipt references. All 227 receipt references reproduce their
own historical commits; 221 also match this baseline, with the existing four
older CMake and two older owner references explicitly qualified. All 57 newest
build pins and its document pin remain current. No existing Source file is
modified by this packet.

The latest reviewed default-constructor receipt's eight Source EH sections,
typed `noexcept`/terminate behavior and earlier admissions remain inherited
Source context. They establish neither a consumer nor EH/runtime equivalence
for this new adapter. No prior 24-, 25-, 88-, 22- or 17-byte admission is
recounted. The candidate offers one 78-byte Original function for later review
while applying zero admission, Original ABI or gameplay credit now.

Explicit CMake registration and a normal `scripts/build.ps1` build must precede
the primary's emitted/linked review and admission. No worker build, tests,
probes, Ghidra/ledger edits or production consumer were added. Original
placement, callers, actual runtime/storage/lifecycle, startup and gameplay
remain unproved.

## Primary compiled review

Normal registered build: `2026-10-09T13:39:47Z..13:40:03Z`, exit 0; all three
existing checks pass. The primary retains 61 Source/build input pins, four
artifact pins, nine complete COFF objects and every physical relocation
graph, with ten unique positive Core roots. No tests or probes were added.

The naked constructor emits 80 bytes / 33 operations. strlen is REL32 at
operand 23; malloc and strcpy_s are DIR32 IAT operands 33 and 52. Every
noncall/nonbranch instruction matches the Original, and all branch target
indices match after the two longer Source calls. The selected current CRT
context uses UCRT heap/string imports; the new constructor is absent from
the application map, so these are not its final linked-operand bindings.

The prior typed owner has identical complete code/relocation contracts
for all 20 functions and identical payload/relocation contracts for all
eight EH sections. This Source context remains distinct from Original EH.
No production consumer, Native profile binding, default storage or lifecycle
was added. Core membership and build results do not prove execution.
