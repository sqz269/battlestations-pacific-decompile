# Text submission over the canonical content owner

Addresses: `00AB6AB0`, `00ABAED0`, `00ABB000`, `00ABB1D0`, `00ABBF30`,
and the Text call of base size setter `00AA7970`. Descriptive names are hypotheses.

These new C++ entries call `build_gui_text_content_00aba8d0` and the actual
Text color implementation. They use the existing canonical lifetime, source
cache, font association, mesh/material owners and locale services. They do not
route geometry through the older `GuiTextHost` builders.

| Entry | Native arguments and cleanup | Behavior |
| --- | --- | --- |
| `00AB6AB0` | ECX Text, UTF16 wrapper, RET4 | Submit content, then current50 even if inner content is equal or empty |
| `00ABAED0` | ECX Text, narrow wrapper/localize low byte, RET8 | Native length/current CRT comparison; changed cache before conversion; content, temporary cleanup, current50 |
| `00ABB000` | ECX Text, source/width/localize low byte, RET0Ch | Same cache guard; live ordered -1 width fallback before conversion; current font after conversion; ellipsis, content, two cleanups, current50 |
| `00ABB1D0` | ECX Text, no stack arguments, RET | Copy live UTF16, empty the stored cache, submit saved copy, clean copy; no extra color50 |
| `00ABBF30` | ECX Text, size-pair pointer, RET4 | Base x87 pair stores and recompose, then capture/rebuild current text |

Narrow equality uses the existing `equal_native_string_headers_00435c40`
against two borrowed eight-byte views of the same typed strings. Its length
gate and current host CRT comparison replace the older ASCII-only source-cache
projection. Equal source returns before inspecting localization, changing width
or invoking color. A conversion failure occurs after the changed source cache
has been stored; no rollback is invented.

The ellipsis sentinel comparison reproduces UCOMISS/LAHF/TEST44h. Only ordered
equality with the supplied live `00D7A260` value selects current canonical width.
Zero remains an explicit target; NaN does not take the fallback. Conversion
then runs through the existing locale resolver or bytewise zero-extension.
The font association is resolved only after conversion, so locale callbacks
cannot be hidden by an earlier captured font. The ellipsis module's supported
stable-font and floating-point domains still apply.

The outer frame retains converted text and, for ellipsis, its separate clipped
result across incomplete content. Completion destroys clipped then converted,
then reloads current color and executes supported Text50 `00AB6B50`. The frame
does not repeat conversion, cache writes or size effects when the actual child
continuation later resumes. An undefined alignment/position-format boundary
rejects child-only resume without consuming the outer frame.

Size stores use the native sequential x87 load/store order, preserving pair
aliasing. The same canonical owner's recompose runs next; base `00AA7970` does
not update bounds. The preexisting typed Text size view is synchronized as C++
bookkeeping. Current UTF16 is captured after recompose callbacks, not before.

This is a new C++ ABI. Typed string copies/destruction project native lifetime
phases but do not reproduce original string-pool allocation callbacks, header
layout, SEH, allocation failure or unmasked floating-point trap timing. Inputs
must be null-free and fit signed32 length, with live canonical owners and the
existing locale/ellipsis modules' supported input domains. Pending content is
not successful submission; all enclosing frames, mappings and borrowed resources
must survive until the real native dependency completes. No Text factory is
enabled by this module.

The report enumerates all 315 current direct-CALL xrefs to these entries:
313 have verified containing-function/instruction attribution. Two calls at
`005E99CA` and `005E99D9` have verified PE bytes/targets but no containing Ghidra
function and remain separately unattributed. Text current58 has its data xref
at `00D5C720`. The three caller-level color calls resolve through the Text table
word at `00D5C718` (`50 6B AB 00`). Full callee listings establish stack cleanup;
call graphs alone are not used to infer a caller's behavior.

`reports/gui_text_runtime_submit.json` checks 352 numeric rows: 349 direct
instructions and three explicitly resolved indirect calls. All pass. Normal
Win32 build and both existing tests pass; those tests do not execute the new
Text operations. Batch integration and remaining dependencies are recorded in
`ORCH5_TEXT_SUBMISSION_BATCH.md`.
