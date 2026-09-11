# Worker verification checklist

Addresses: none (process)

Every rule below exists because a second-pass review found the mistake in a merged packet.
A worker runs the checks before committing; an orchestrator runs the mechanical ones again at
integration (`python tools/verify_report_calls.py reports/<name>.json`). The rule names are the
words to use in a report when one of them could not be satisfied.

## 1. Callee before contract (`clear_help_line` -> `clear_command_bar`)

A host method's name and contract come from the **callee's body**, never from the call site's
arguments. Five empty strings pushed into `0054B530` looked like "clear the help line"; the body
rebuilds the command bar from five triples and the help texts have their own setter `0054A0C0`.

Check: for every host method, `python tools/bsp.py show <callee>` (and `--asm` when it is short)
before naming it. If the turn does not allow reading the callee, name the method by its address
(`call_0054b530`) and write `contract: unread` in the doc's host table; never invent a verb.

## 2. Boundary before attribution (`005997C1` and `005998D0` were not in `00599340`)

A call site belongs to the function whose Ghidra body contains it, not to the function you were
reading when you scrolled past it. `disasm-raw` is linear and shows no boundaries.

Check: `python tools/bsp.py ghidra proto <site> --brief` prints the containing function and its
body range; the host-table row must cite that function. `tools/verify_report_calls.py` checks
every `address`/`native` row of a report against the live function bodies and the call graph.

## 3. All call sites before a contract (`commit_screen_visibility(id)` needed `visible`)

A routine's contract is the union of its call sites. `004F83B0` was read from the enter pass (byte
just set to 1) and modelled as "show"; the exit pass calls it with the byte cleared.

Check: `python tools/bsp.py ghidra xrefs <callee>` (or `callers`) and read each site's argument
setup; a pure-virtual host method carries every value that differs between sites.

## 4. Producer before layout (`+0h` was `id`, not `title`)

A record field's meaning is settled by the code that **writes** it (the reader schema, the
constructor, the parser), not by the consumer's use of it. Two headers declared the same record
with different meanings for `+0h`; the schema in `bsp/mission_tree_data.hpp` had the answer.

Check: before declaring an offset constant, `rg -n "<record name>|<offset>" include/bsp docs` for
an existing declaration of the same record and reconcile with its producer. The duplicate-name
check at integration only catches identical constant names, not conflicting layouts.

## 5. Partial projection is labelled partial (`00588C70` clamp-only helper)

A reconstruction that covers one branch of a routine says so in the header comment, in the doc
and in the ledger status (`partial_projection`), and lists the unread branches by address range.
A later reader must not be able to mistake the covered branch for the routine.

Check: the doc's routine table has a `coverage` column: `complete`, or `partial: <ranges left>`.

## 6. Run-time evidence when the executable reaches the path (the drain-loop claim)

A claim about *why* something happens on a given frame ("the drain loop is why the menu appears
on the entry frame") needs a run log when `bsp_game.exe` can reach the path. The static reading
missed that `00684600` had already synced the request twice; a 120-frame run showed the service
pass finding nothing.

Check: if the routine is on a path the executable runs (docs/GAME_EXECUTABLE.md host tables),
run it (`--frames`, `--press-start-frame`, `--log`) and quote the log lines that support the claim.
A claim the run contradicts is wrong, whatever the listing seemed to say.

## 7. Argument counts from the stack cleanup (eleven ids, not eight)

For `__cdecl` and varargs calls, the argument count is the `ADD ESP, imm` after the call (or the
callee's `RET imm`), not the pushes you happened to list. Two level-1 lists were truncated at
eight ids; the cleanups at `0068AFF1` and `0068B0F1` said eleven.

Check: quote the cleanup instruction next to every list you transcribe from pushes.

## 8. Register provenance by filtering the listing (EBX at `006868AD`)

A register's value at a site is established by filtering the whole function listing for that
register (`python tools/bsp.py show <fn> --asm | rg -n "EBX"`), reading every write and the
callee-saved rules, never by searching for one zeroing idiom. `disasm-raw` resyncs from anywhere
and cannot establish provenance.

## 9. Lease only what you annotate

A lease covers addresses you name, document or reconstruct. Leasing addresses you merely call
blocks the packet that owns them (52 consumed addresses blocked one ledger record for a whole
wave). Claim the annotated set; extend it when you decide to name a callee.

## 10. The report carries the evidence the checks need

`reports/<name>.json` has, for every host step, `address` (call site), `native` (callee) and the
containing function when it is not the packet's main routine; for every routine, `coverage`; for
every correction, `was`, `is`, `evidence`. `tools/verify_report_calls.py` reads the first two.
