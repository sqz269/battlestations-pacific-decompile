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
Source application is complete; official MSVC Win32 compilation and scoped actual
Core/EH review remain pending at this checkpoint.

Full E0 owning transport remains required. A future throwing family still cannot
pass through an unchanged `noexcept` queue/destructor boundary as if it were a
propagating route. Mesh generation/thread exclusion, retained failure records,
canonical/factory/Mission/Palette credits and actual runtime remain separate gates
in [the binding-gap record](CODEX_CC16_REMAINING_BINDING_GAPS.md).
No new tests, Native/vendor body credit, runtime result or raw-switch activation
is claimed by this prerequisite.
