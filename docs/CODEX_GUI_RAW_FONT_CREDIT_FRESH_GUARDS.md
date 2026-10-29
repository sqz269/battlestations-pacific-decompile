# Retain strict fresh-only admission with private shader progress

The shader packet adds private normal-return/consumer records independently
of its public diagnostics. Existing raw122 initial-bind and metadata-only Font
owner checks must therefore inspect that private progress as well as the
unchanged public fresh-body checks. Resetting public fields cannot erase a
previous private invocation, failure or consumer history.

Root initial bind requires private producer FRESH, no issued credit/consumer,
no failure/active invocation and no bound Text/reference/cache identity. The
metadata-only owner uses the same private untouched test. Its existing public
cache/loader/name-free checks remain unchanged and never read names[].
The snapshot also exposes the private one-shot retirement_started flag. Even a
successful initial null/no-producer raw118 invocation leaves that flag true;
it cannot later pass the untouched/initial-bind tests after transient active
identities clear. Credit absence alone is not proof of no prior invocation.
Known-region reservation also excludes privately entered/failed/active work,
while retaining the existing public FRESH/COMPLETE phase restriction.

This tightens metadata admission only. It does not allow Font108, owned1C8,
Text1EC, any completed shader operation in the metadata-only owner, a Font
setter caller, or DEFAULT/prefix/geometry fallback for pending/failed work.
It reads only independent C++ snapshots, with no native field/provider action.
No original body, native layout, arithmetic or ABI changes accompany it.
Independent Source review accepts the final projection and both fresh guards.
Combined Win32/checks2/2 pass (11.46s),
`local/cc16_raw_font_credit_private_history_corrected_build.log`.
An intermediate build rejected a local name that shadowed Frame.result under
warnings-as-errors; renaming that local snapshot fixed compilation without any
native behavior or compiler-policy change. The earlier log is retained.
No new test, shader execution, credit-return fixture or production activation
is claimed. Completed quiescent retirement history remains valid where the
outer lifetime permits it; only untouched/initial-bind admission requires none.
