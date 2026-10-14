# Root/mode sampler caller entry and shared residue

The two calls at `00B3C018` and `00B3C024` share the same physical borrowed
word within one invocation of `00B3B3C0`. They therefore share the word's
known/unknown state. The caller setup does **not** establish the initial word.
A safe Source adapter retains one word and one knowledge object per compiler
invocation, with distinct root/mode contexts and operations. It starts unknown
unless that invocation supplies an observed preimage.

This audit checks the current saved Ghidra program and executable against the
specific caller/callee bytes. It does not run the original game, mutate Ghidra,
or change the installed executable. The report is
[native_gui_sampler_entry_recovery.json](../reports/native_gui_sampler_entry_recovery.json).
The companion Source adapter is owned by the `cc12_native_gui_sampler_parent`
packet; this packet changes evidence only.

## Exact original stack relationship

Let `P` be the steady caller ESP at `00B3BFF9`. At both calls:

| Instruction or state | Physical address / value |
|---|---|
| Root `B3C000 PUSH ECX` / mode `B3C020 PUSH EDX` | `P-4`: descriptor from builder `+70` / `+74` |
| Root `B3C003 PUSH EBP` / mode `B3C021 PUSH EBP` | `P-8`: same actual pass |
| Root `B3C018 CALL` / mode `B3C024 CALL` | `P-0Ch`: `B3C01D` / `B3C029` |
| `B3B280 PUSH ECX` | `P-10h`: saved builder register, then loop index |
| `B3B281 PUSH EBP` | `P-14h`: saved caller EBP |
| `B3B29A/B/C PUSH EBX/ESI/EDI` on nonempty descriptor | `P-18h`, `P-1Ch`, `P-20h` |
| Nonempty sampler steady ESP `B` | `P-20h` |
| Borrowed word `W`, called `B-16` in Source | `B-10h = P-30h` |

The `B3B290` zero is written at `P-10h`, not at `W`. The caller's
`B3C006/B3C00C/B3C012` zeros target builder fields `+8C/+90/+94`, not this
stack word. They reset sampler counters before root; they do not seed residue.

`B3B3AA..B3B3AF` restores saved registers and executes `RET 8`, restoring the
caller ESP to `P`. The mode setup only reads builder `+74`, writes the two
argument words and the mode return address, and reloads ECX. It does not write
`W`. An empty root takes the shorter prologue/epilogue path and also returns
without writing `W`. This proves aliasing even when root has no samplers.

The recovered register/stack contract is ECX = actual builder; stack argument
one = actual pass; argument two = actual root or mode descriptor; callee `RET 8`.
The sampler body saves/restores the nonvolatile registers it uses. The three
validated ranges contain no x87/SSE instructions or explicit floating-point
argument consumption. There is no evidence for introducing an FPU seed in this
adapter. This bounded observation is not whole-compiler/native-EH equivalence.

## Why the first Source0 input is still unresolved

`B3BFF9..B3C018` has no write to `W`. Earlier, the reached path can execute:

1. `B3BFB6 -> B5F9B0`: the constructor's `B5F9C8 PUSH EBX` writes `W`.
2. `B3BFD7`: current COM Release, followed by pass pixel-owner replacement at
   `B3BFDE -> B5F080` (which itself can call current atomic/terminal providers).
3. `B3BFE7`: current `CE2220` decrement, and optional current terminal callback
   at `B3BFF7` when the decrement returns zero.

Those intervening implementations can reuse the same released stack area.
The saved EBX is an intermediate write, not proof of the final word at root
entry. Assigning zero, the builder, an earlier constructor register, or a
reconstructed C++ local's bytes would invent a preimage.

The historical reviewed `original_run03` capture recorded
`P=0162E86C`, `W=0162E83C`, and word `0162E964` at `B3BFF9` and before
`B3C018`; the word happened to equal that invocation's builder. The process
stopped before executing the root call. The prior
[capture](NATIVE_ORIGINAL_SAMPLER_CAPTURE_CC10.md) and
[origin audit](NATIVE_ORIGINAL_SAMPLER_WORD_ORIGIN_CC10.md) explicitly do not
establish a portable builder-derived rule. This packet reads those published
receipts; it does not revalidate the frozen archive or old process.

For Source0, `B3B326/B3B327` push stage/index and `B3B328` calls `B5F100`.
After its local allocation and saved ESI, `B5F10B` writes only the low byte
at `W`. After saved EDI, `B5F13C` reads the complete DWORD at the same address
and `B5F142` stores it to row `+4`. The pre-existing upper 24 bits remain a
real input; a complete observed DWORD is the current explicit Source interface.
The optional reserve is below the live local and does not redefine its bytes.

## Adapter and lifetime requirements

The paired invocation must retain one shared word and knowledge object. Root
and mode retain different descriptor contexts, operations, sampler indices,
release-capture arrays and capture cursors. The same shared state continues
from root into mode without a reset or copy from a separate default:

| Root's final effect on the word | Mode receives |
|---|---|
| Empty root, or no established write | The original unknown state |
| Source3 `B18FC9 PUSH ESI` | The actual sampler word, with subsequent native low-byte changes |
| State setter call at `B3B369` | Known return word `00B3B36E` |
| Source1 established loader/binding word followed by uncaptured successful release | Unknown state |
| Source1 successful release with matching same-invocation owner/sampler capture | Captured word and known state |

The state must be allocated afresh for each compiler invocation, including
reentrant calls. Preserve it for retained failed child operations until their
existing cleanup contract permits retirement. Sharing a renderer-wide word or
resetting knowledge between root and mode both violate the recovered mapping.
An inert initialized backing word does not itself establish native knowledge.
Source0 consumption remains guarded when knowledge is unknown.

The remaining entry capture is exactly `[ESP-30h]` at `B3BFF9`, or
`[ESP-28h]` immediately before the CALL at `B3C018`. Bind any observation to
the original thread/invocation, frame, builder, pass, root descriptor and native
site. A later Source1 release capture additionally belongs to its descriptor,
sampler index and released owner after the actual release path returns. An
unrelated original invocation or the Source C++ stack cannot supply those
bytes. No generic capture framework or runtime-admission bypass is implied.

## Current evidence and limits

The current Java process command line identifies
`C:/Users/sqz269/bsp.gpr`; the live CLI verifies project `bsp`, program
`/battlestationspacific.exe`, image base and x86 language on every query.
All 537 bytes in these three ranges match the executable on disk:

- `B3B280..B3B3B1`: complete sampler body, including `RET 8` (306 bytes).
- `B3BF93..B3C028`: preceding owner work and both caller CALLs (150 bytes).
- `B5F100..B5F150`: complete texture-reference append (81 bytes).

The JSON report preserves exact bytes, hashes, inspected Source/document hashes
and the baseline commit. Existing assembly exports also establish the constructor,
effect-name save and state-setter entry schedules above. No Source file or test
was added by this packet, so no build was rerun here; the companion Source
adapter requires its own strict Win32 build and relevant focused verification.
The initial preimage is still not generally resolved. This evidence does not
establish native sampler runtime parity, GUI Font/Text rendering, native EH/ABI,
startup completion or gameplay validation.
