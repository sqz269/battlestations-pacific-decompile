# Lua numeric element write

Addresses: `00B66580..00B665CA` (75 bytes). Proposed descriptive name:
`BSP_LuaObject_SetNumericElement`; this is not a recovered symbol.

The complete routine writes a signed integer key and a binary32 value into
the current table of the actual 20-byte (`14h`) `NativeLuaObjectStorage`.
Original ABI: thiscall, ECX object, two original public stack words, `RET 8`.
No kind gate, tracking update, local object, or native cleanup map is present.

The first call at `B6658D` is `A672F0` (`lua_checkstack`) on current owner/state
with two extra slots; its result is ignored. Only after that call does `FILD`
at `B66592` read the original signed key stack word. Owner and state are then
read at `B66596/98`, and `FSTP` writes the binary64 argument. `B665A1` calls
`A679D0` (`lua_pushnumber`).

The value sequence differs: `B665A6` captures current owner **before** `FLD`
at `B665A8` reads the original float stack word. `B665AC` reads that owner's
current state **after** `FLD`. `FSTP` and `B665B5 -> A679D0` push the value.
Finally `B665BA/BC/BF` read current owner, current index, and captured-owner
state, in that order, before `B665C2 -> A67E10` (`lua_settable`).

The Win32 source uses a naked fastcall interface with an unused EDX argument
(pass null), original ECX and original key/value stack words. Its inline
assembly preserves all x87 and object-read ordering. Four explicit stack
bridges call the linked Lua 5.1.1 C API and restore ESP before subsequent
native reads. It never changes the x87 control word, copies float arguments
early, converts through SSE, catches Lua errors, or owns caller storage.

`local/dl-lua-numeric-write-evidence.json` pins the installed executable,
live Ghidra bytes, complete body, original prototype/comments, and every
direct call. The tracked report records exact source validation and artifact
hashes after integration. `B1B890` is a dependent record-valued setter under
separate investigation; this body alone does not complete that caller.

The linked Lua VM is a source-compatible dependency, not the original game
VM or its private register ABI. Any local comparison that executes relocated
native bytes uses explicit C API bridges and must be labelled accordingly.
Full game execution, original Lua error/longjmp paths, unmasked x87 faults,
hardware exceptions, and native unwind compatibility remain unvalidated.

Validation at exact source `61fc799eb36dfb3f33762b3bc08cd34c7e2262ca` passed MSVC Win32 Release and both existing CTests with 2619 unchanged tracked build inputs. The complete generated body is89 bytes: all native noncall instruction bytes are identical, with only four explicit Lua C call bridges. One ignored local differential fixture compared source and relocated original body through those C API bridges. Both INT32_MIN/min-subnormal and -1/signalling-NaN cases matched exact binary64 bits and x87 status flags (0002 and0001), retained control word037F, balanced both stacks and preserved actual owner/object storage. No repository tests were added. The four original native call displacements were patched for this probe; the original game Lua implementation and unmasked faults remain outside that evidence.
