# Explicit current Source Text comparison, packet 52

`GuiTextContentComparison` now selects one of two explicit borrowed comparison
contracts at the existing content equality point. The Source-current mode has
no Original changed cell and requires its own current comparator. This adds a
Source capability; it does not recover or instantiate an Original CRT runtime.

## Exact seam and preserved native route

`prepare_gui_text_content_00aba8d0_fragment` still ensures the actual draw
sections, copies the input and invokes all required uppercase callbacks before
reaching equality. At the existing ABA921..ABA94F equality point it calls
`e.comparison.equal_content(text.text, copied, e.calls)`. No application-entry
refresh, previous operation or cached locale name decides the comparison.

Both modes preserve the original wrapper-length gates: an empty stored wrapper
is equal only to an empty copied wrapper; a nonempty stored wrapper differs
from an empty copied wrapper. Neither path invokes a comparator. Two nonempty
wrappers reach the selected comparison even when their lengths differ. The
helper forwards the original stored/copied `c_str()` pointers without another
copy, transformation or length-equality shortcut.

Borrowed-cell construction retains the existing native-style dispatch. The
actual live cell is read at equality: zero uses the unchanged unsigned UTF16
ASCII A..Z folding loop; nonzero invokes the required
`GuiTextContentCalls::compare_current_locale_00c0392a`. Source-current mode does
not supply a fallback for that route. The host still preflights its native
comparator even if the borrowed cell happens to be zero initially.

## Explicit Source-current mode

`GuiTextContentComparison::source_current(compare_now, context)` rejects a null
comparator before any Text/application operation. Its private state stores the
borrowed function/context and no cell. Every reached nonempty equality calls
that function then, consuming zero as equal. It never reads an Original gate
or invokes the unused native comparator. The callback must provide its current
Source semantics on each call and preserve the live input pointers throughout
the call. There is no implicit callback, frozen locale snapshot or new process
owner. Locale mutation and synchronization remain the supplied provider's
responsibility; this change neither sets nor restricts a locale.

The selected value propagates through
`GameNativeTextApplicationBindings::content_comparison` and
`GuiTextContentEnvironment::comparison`. Source-current host bindings may pass
null for the unused `compare_current_locale_00c0392a` function. An accidental
direct native-service call with that unbound function fails explicitly; there
is no fallback comparison.

For example, with caller-owned live providers:

```cpp
auto current = GuiTextContentComparison::source_current(compare_now, &provider);
GuiTextContentEnvironment environment{locale, locale_host, current, buffers, calls};
```

The implicit borrowed-cell lvalue constructor preserves the previous aggregate
position for native callers:

```cpp
GuiTextContentEnvironment environment{locale, locale_host, actual_changed_cell,
    buffers, calls};
```

There is no default mode. Temporary integer-cell construction is rejected.
The public member names and reference-to-value representation changed, so
consumers using named members or a precompiled Source ABI must update/rebuild.
Callback, context, native cell and all existing owners remain borrowed; no
ownership, thread-safety or exception cleanup contract was added.

## Evidence and limits

One focused manifested MSVC Win32 `/MD /O2 /W4 /WX` probe links the rebuilt
production `bsp_core.lib` and invokes this same equality helper. It first sees
a controlled current state that differs, then calls the actual
`locale_uppercase_00a9eba0` path, whose real host callback selects equal state.
The next equality sees that state. A further state change is also observed.
The probe checks original string pointer identities, forwarding of unequal
wrapper lengths, all empty gates, the unchanged borrowed zero/nonzero dispatch,
a live flag change by the uppercase callback, and null-current preflight.
It passes with four Source-current calls, one native-service call and two
uppercase callbacks. It does not construct or simulate a full native GUI.

`scripts/build.ps1` exited zero and the existing `reconstructed_math`,
`native_math_differential` and `tool_tests` suites all passed. No new test
framework, CMake source, font-host file, Ghidra annotation, Native ledger entry,
installation or normal-host activation was added. The private probe remains
under `local/cc12_native_text_current_comparison_52/`.

The provider contract review is packet 51, commit
`6850c08c01005ddc2c75ac38d77f895c363ae3e2`, published by Root at `c3aa077a8`:
`docs/CC12_NATIVE_GUI_CRT_PROVIDER_CONTRACT_51.md` and its corresponding report.
That review establishes why application-entry sampling is insufficient: Lua
can change the Source locale and direct property/factory paths and earlier
content callbacks can bypass or outlive such a sample.

No actual UCRT adapter/provider was added or compared here. A future explicit
host adapter is a Source substitution with its own width/type/ABI and lifetime
contract. It does not establish Original-entry, Original locale/PTD type,
non-C Unicode/version, SEH or game equivalence. Reconstructed CRT parents
46/48/49 still require their actual Original CRT domain. Normal native Text
composition remains unactivated.

Machine-readable receipt: `reports/cc12_native_text_current_comparison_52.json`.
