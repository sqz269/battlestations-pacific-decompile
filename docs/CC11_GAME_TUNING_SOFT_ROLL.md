# Native soft-roll offset producer

The reconstructed tuning loader omitted its derived word at `+580`. The active
GameUnits roll setup copies `GameTuningBlock::derived_580` into
`PilotBotRollInputs::soft_roll_offset`, and the servo uses it outside the soft
zone. The owning mission loader initializes the block to zero, so this missing
store left that offset zero even when the two authored inputs were loaded.

The Source loader now computes the word after loading and releasing the
`SoftRollMul` key. Its private helper uses the five original x87 operations:
load `+57C`, load one, reverse subtract, multiply by `+578`, then store `+580`.
The subtraction and product stay extended until the sole binary32 store.
Compile-time offset checks tie all three operands to the existing block.
The old unused scalar helper is unchanged; it does not implement this store.

The native evidence is the partial span `[007E6FB3,007E6FDA)`: 39 bytes and nine
instructions inside `007E2A20`. Five instructions perform the arithmetic; the
other four prepare the next Lua lookup's arguments and registers. No CALL is
inside this span. This is not recovery of the whole constructor, private Lua
state owner, singleton factory or original class ABI.

The ignored fixture in `local/cc11_tuning_soft_roll_root` executes all 39
unchanged original bytes and the actual private Source helper on separate
copies of the existing `6D0`-byte block. An appended four-byte `ADD ESP,8; RET`
repairs the two lookup-argument pushes and returns to the fixture driver.
No original instruction or operand is changed, and the pushed key pointer is
never dereferenced or used for a substitute lookup. The copied fragment is RX.

The strict Win32 fixture passed **391 checks across 96 original-fragment
cases**: PC24/53/64, four x87 rounding modes, eight raw input pairs including
signed zero, denormals, signaling NaN, invalid infinity multiplication and
overflow. Each starts with two occupied x87 values and real sticky status.
Full CW/SW/TOP/tag, all eight raw 80-bit registers, MXCSR and all eight XMM
registers match. Only `+580` changes in either block. FIP/FDP/opcode metadata,
general registers, unmasked faults and insufficient stack space are excluded.

Installed `PlaneGlobals.lua` authors `SoftRollCtrl=0.05` and `SoftRollMul=0.7`.
Their staged binary32 words produce `3C75C290` under PC53/nearest, approximately
`0.015`, rather than zero. These authored values were inspected and pinned;
the fixture does not execute the production Lua/VFS loader or whole controller.
The producer and consumer connection is Source and compiler evidence.

Two actual translation units are freshly compiled: the production CPP alone,
and the probe containing that same CPP so it can exercise the private helper.
The linked probe uses the second object; the first proves production compilation.
Nine Source/header/fixture/recipe/library/PE/installed inputs stayed unchanged
before and after. The PE32 asInvoker manifest was verified. Source COFF emits a
27-byte seven-instruction private helper without relocations, and inlines its
five x87 operations into the loader after the `57C` row's release loop.

Full main build and primary annotation closure are pending. Existing Lua key
order, defaults, missing-value policies and other derived fields are preserved.
No tracked tests were added. Native factory/lifetime, full Lua instruction or
FP parity, structural reentry, concurrency, register ABI and gameplay remain open.

Primary integration at `b7f1bb6c945b684dbe232f7b44f82b417af8bff5`: the complete Win32 build and all three existing CTests passed. An independent current-library probe freshly compiled 2 actual translation units and passed 391 checks. It pinned 3 Source/header/fixture inputs, three current support libraries and the original PE before and after; 2 actual compiler includes were verified. The report records native byte/literal checks, new Source COFF, manifest, logs and immutable receipts. Original ABI, whole ownership/world binding and gameplay remain unproved.
The current production compiler inlines all five original x87 operations into the actual loader row. The focused probe executes the private store, not production Lua/VFS loading or the full native tuning constructor.
