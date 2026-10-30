# GuiGroup pool startup validation: Host gate pending

Candidate `6772843657c3ab62f7bb4f9ae613adbac0770c80` and independent review
`41106def0cae436ea117b563ce5470bb96f4c161` pass static Source review and the
common build/actual-PE checks. The changed default startup is staged locally.
**Current Host/runtime custody is pending; this is not Main activation approval.**

The same process backing is permanent, preserving its native links/profile when
atexit returns nonzero. A new Source allocation precedes complete construction
and pointer publication, before Native startup. Explicit startup still calls
AC7410 then real atexit(CE0AE0) and caches any status. The query observes only
entered failure/retiring; callback phase stores surround actual destruction.
Both Main catches use the actual Group pointer and retain existing policies.
No status-as-error, manual cleanup, extra callback or payload activation appears.

Primary review covers all three production deltas,47 author-document lines and
49 independent-review lines. The sealed five blobs, retained Source/artifact
pins and529 consumed strict-build headers verify. The two later Text-process
provider updates are explicitly tied to admitted972969f2/Main5dc9, rather than
being mislabeled as unchanged baseline inputs.

The current Win32 Release MP2/PCH build passes both existing suites (12.81s).
Common storage has31 code sections1776B,25 prior-common exact; Main has1017
sections86037B,1015 exact under an explicit22-lambda bijection. Its only changed
code sections are WinMain and its unwind/handler section. The six changed storage
sections and both Main sections total7813B; every byte and symbolic relocation
matches the strict artifacts, including a distinct eight-lambda mapping needed
by those two Main bodies. New phase/state/caller code adds no Native body credit.

The actual built bsp_game.exe/map verifies all eight changed whole code sections
after every relocation. Seventy-three retained exception sections4212B and47
mapped SafeSEH associations verify, including the complete620B Main EH data.
Twenty-four exception records remain object-only. Local label targets resolve
through each relocation's actual COFF symbol index and defining section, avoiding
unrelated duplicate local labels. The executable was never launched.

An existing bsp_game entryPID40812 cannot be classified terminal: limited query
and synchronization opens return access denied5, and historical coordination
explicitly preserved it. No process was stopped or restarted. Root corrected
earlier messages to canonical coordinator owner agent/cc16-progress (message1103).
No exclusive window or current Host result is inherited from old frozen cohorts.

The runtime gate still requires exact current-build startup, shared-domain and
exit custody, actual consumers finished before teardown, and the callback suffix.
Normal registration and static nonzero-status lifetime arguments must remain
distinct evidence. ABI/FH3/SEH/private/fault, gameplay and activation claims remain
unproved; Main integration stays pending until its required gate is satisfied.

Evidence: `reports/cc16_native_gui_group_pool_startup_source_admission.json`.
