# Normal zero callback interface prerequisite

The Source change allows the base `RenderCommandReference::release_zero_references`
and `release_render_command_reference` helper to propagate ordinary C++ exceptions.
The helper still performs the same single sequentially consistent decrement and
calls the terminal only on the same observed one-to-zero result. Every function
body and data-member declaration is unchanged.

The exact two-file patch is `832ece0b09eb6e2fdfddd7f3411ae24549fff5adce03b9ed4297eed21f3104e1`,
applied to published Main `ff7a7fc64730320d33ad59b3103a0c62f98fb4e6`.
Root and independent review read both complete files and replayed the complete
forward/inverse. Independent review `d200beeb2a6cc2e09645e778a9c7678af566c0df94ab7ac5b4cdca435302d8ba`
records 189 snapshot inputs and complete current type-use inventories. The other
124 terminal declarations/definitions remain explicit `noexcept`; retain, assign,
release-then-clear, concrete companions, queues and destructors keep their policies.
Current actual-owner normal/observed helpers already permit C++ escape and are
unchanged. No new throwing companion or raw binding is installed.

This intentionally changes C++17 function types and their nothrow guarantees.
Caller cleanup, termination code and EH metadata may change. The Source review
does not establish Original ABI, vtable/binary identity or unchanged emitted code.
Source application, official MSVC Win32 compilation and the scoped actual
Core/EH reviews below are complete.

Full E0 owning transport remains required. A future throwing family still cannot
pass through an unchanged `noexcept` queue/destructor boundary as if it were a
propagating route. Mesh generation/thread exclusion, retained failure records,
canonical/factory/Mission/Palette credits and actual runtime remain separate gates
in [the binding-gap record](CODEX_CC16_REMAINING_BINDING_GAPS.md).
No new tests, Native/vendor body credit, runtime result or raw-switch activation
is claimed by this prerequisite.

The official Release build at `f0a39bfda766257937a47f874ea1978f26b5a3d3` passed all targets and
both existing suites in 11.37 seconds. Literal comparison covers 17 selected
objects: 6,209 complete text sections are exact, ten change and seven compiler
handlers are added. The normal release helper remains exactly 28 bytes with
identical relocations. Native context differs only in its COFF timestamp.
Queue changes preserve field/count/terminal ordering and the existing hard
`noexcept` boundaries: all seven relevant FuncInfo records carry flags 5, with
genuine current FH3/terminate suppliers. The shared geometry kernel preserves
its count/import/receipt/failure schedule; its 149-byte scalar helper is exact.
Root also read both changed Renderer palette/bone retirement bodies and their
two handlers; pop-before-release, no post-disposal reference use and flags 5
remain explicit. These are scoped Source compiler observations.

Independent queue/context Core review is `a7a6901091c62656e1500b2cf17c4768193c18d2c10f1929edd918b7c029b02e`;
adjacent expert Core review is `a640881f22be6d888d65923c5d71a4ce3d81c504b203b6276bca52de2c82e3a6`.
Their 55 and 38 declared artifacts/inputs were verified and copied. Details are
in the JSON report and `local/zero_interface_primary_cc16_20261031/`.

Published raw-getter Main `ee32fc77f43a2bfff01c2e388a02588a07ce3b09` is joined. Root read both
complete new code files, author/peer/admission documents and the complete
reader; its immutable author check passes 124 assertions. Root compiled getter
code matches all eight reviewed sections (558 bytes/14 code relocations) plus
the complete 88-byte EH section/four relocations under one explicit anonymous
TU mapping. Whole-object/RTTI/PE identity is not inferred across worktrees.
The final combined official build at `10c1042d395e30b6874f2580ccb5981d2902ad04` passes all targets
and both suites in 14.07 seconds; all 17 reviewed interface objects remain
whole-byte-identical. The normal helper is linked in the canonical Game map;
the uncalled getter remains absent. No overlay or forcing remains.

All nine raw GUI switches remain false. Main retains the upstream V3 storage
selection at both sites. No new entry or Game was invoked, and no current
throwing family, whole E0 ownership, Original ABI or gameplay proof follows.
