# Raw input context levels and action activation

Addresses: 00A93020; 00A933F0. Producers00A93DA0/00A93C80; callee00A92D40.

A933F0 writes the indexed word in the actual owner's context array, reloads
the array base, sets owner+20 to1 and computes the unsigned maximum of current
words. The CMP/JBE atA93417/A9341A is unsigned:80000000 outranks1. Each iteration
reloads count/base before its endpoint comparison. It then always callsA93020,
even if the selected word or maximum did not change. There is no range check,
array growth or context clamp. Native ECX is the24h owner, two DWORD stack
arguments are index/level, and RET8 endsA93437 (exclusiveA93438).

A93020 captures the action count before the action base and captures owner+10
once for nonempty traversal. For each30h action it loads active DWORD+20 before
enabled byte+1C, supplies those three arguments toA92D40, then reloads count/base
before incrementing the retained action pointer and testing the new endpoint.
This allows a callback to affect later records and traversal. Native ECX is the
owner with no stack arguments or result; RET endsA93067 (exclusiveA93068).

The actual24h owner is produced byA93DA0. A93C80 establishes30h records and the
context headers/words; A92D40's complete activation and listener behavior are
in NATIVE_INPUT_ACTION_CONFIGURATION.md. These routines share those concrete
services. They allocate no owner, vector, listener or replacement context state.
GameInputActions exposes the setter through the same lazy action publication.

All39 incoming CALLsites were inspected. The34 assigned sites provide current
raw ECX receivers and native argument setups. Five sites005CC59B/00524475/
00524485/0052451D/0052452D have no stored containing Ghidra function. For each,
the entire16-byte two-PUSH/lazy-getter/MOVECX,EAX/CALL block matches disk/live
bytes. Their argument contracts are established; containing-function boundaries
remain unknown. No adjacent function was substituted. The report records these
separately and35 numeric CALLrows including the activation call atA9304C.

## Verification boundary

Both complete72-byte bodies match the original image and current Ghidra bytes.
The strict Win32 build and both existing CTests passed. One extension of the
existing native configuration fixture compares four native spans and seven
phases, including unsigned priority, floor1, unchanged-level refresh and a
callback-mutated endpoint. Final combined validation is recorded in
reports/input_configuration_ah_integration.json. The
source APIs add a required service context; they are not native callable/FH3/SEH
replacements. Callback exceptions preserve earlier writes. Application frame
execution, arbitrary stack aliases and gameplay have not been validated.
