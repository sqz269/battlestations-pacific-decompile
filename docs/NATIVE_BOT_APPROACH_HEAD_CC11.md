# Complete shared bot-approach head constructor

`construct_native_bot_approach_head_009f9ce0` reconstructs the complete ordinary
152-byte, 38-instruction body `009F9CE0..009F9D78` (exclusive). It publishes the
actual caller-owned eleven-word prefix and preserves the native x87 speed-ratio
spill and comparison. `NativeBotApproachHeadHost` supplies the existing
`BotTaskHost::construct_speed_reference` call; all other host methods remain
required. This is a prerequisite for complete approach callers, including
`009AFE70`, rather than a whole task or landing-queue implementation.

Native interface: ECX head, stack unit pointer and binary32 reference speed,
EAX same head identity, `RET8` at `009F9D75`. The source API has an explicit
borrowed context and a new C++ ABI. Descriptive names remain hypotheses.

The constructor stamps the recovered numeric `D21C74` word, then publishes
unit, unit+538, unit+9D4, first unit+DF4 and a descriptor row. It independently
reloads unit+DF4, reads descriptor+34, multiplies modulo32 by `248h`, samples
the actual `F8A30C` cell, adds `0Ch`, and stores the resulting address. After
zeroing words18/1C/20 it freshly reloads unit+538. It does not reuse the earlier
published class pointer. There are no native CALLs, allocation or provider
phases in this body. The profile word is not an executable source table.

`FLD [fresh class+188] / FDIV [reference speed]` spills once to binary32.
`FLD1 / FLD spill / FCOMIP / FSTP` then keeps only an ordered quotient strictly
greater than one. `JBE` also selects the fallback for unordered comparisons.
Fallback and minus-one words are loaded from the actual `D7A24C/D7A260`
aliases at their original points; the comparison uses `FLD1`, not that fallback
cell. Original PE words were verified as `3F800000/BF800000`. No control-word
change, empty-stack assumption, C++ division or `max` replacement is used.

The context contains only references to the same live row-publication and
constant cells. Head/unit/class/descriptor storage must remain valid through
all actual reads and writes. No shadow registry, translated unit, lifetime,
default constants or allocator is supplied. The existing semantic
`bot_task_speed_ratio` and other task handlers remain unchanged.

The ignored primary probe compiles the actual new TU with MSVC Win32
`/W4 /WX /fp:strict`. Its original oracle is the complete 152-byte PE body,
with only three absolute data operands relocated to explicit fixture cells.
Every instruction, relative branch and `RET8` remains original; original PE
identity and all operand locations are asserted. No game process is involved.

The probe passed 293 checks, including 73 original/source whole-body component
comparisons. The x87 matrix has 72 cases: PC24/53/64, four rounding modes,
empty/three-value ambient stacks, and inexact-above-one, equal-one and
zero/zero unordered inputs. All eleven output DWORDs, returned identity,
control/status/tag and eight 80-bit register payloads match. Exceptions are
masked and at least two x87 entries are available. Instruction/data pointers,
EFLAGS, general registers, XMM registers and private fault/unwind paths are not
compared; this does not establish native binary ABI compatibility.

One valid aliasing witness places head+10 over unit+538. The cached class and
later numerator class then differ; the complete original and source agree.
It also exercises a freshly rebound row cell and modulo32 negative row index.
Explicit fixture constant changes verify current-cell loads and fixed-one
comparison, without claiming that the game's constant storage changes.
The abstract host bridge is compiled and reviewed, not a complete game host
or production task run. Actual global/profile/row ownership, larger caller
construction, task/holder lifetimes, whole landing flow and gameplay remain
unbound. Main build and integration receipts are recorded separately.

Artifacts: `local/cc11_head_primary.cmd`,
`local/cc11_head_primary/probe.cpp`, `original.hpp`, `head.asm`, `probe.exe`,
`inputs_before_link.json`; compile/probe output is
`local/cc11_head_primary_compile.log`.
