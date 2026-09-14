# Cockpit constructor integration, BJ

The actual B3C800 helper constructor now uses persistent preadmitted camera/viewport storage through construction, replacement and final helper retirement. The constructor owns its local native cleanup; settlement preserves survivors until their native retirement and explicit host-quiescent reset.

Exact combined source `dd86c2152a814a352ee0e2f3c3b503fc678b0008` passed the strict Win32 build, both existing CTests and eight seed spans. All 2514 tracked build inputs and all three linked libraries stayed unchanged. The fixture compiles one probe source against those current libraries.

One full mapped-original/source B3C800 constructor-through-helper-terminal pair at x87CW027F: 12 live checkpoints, 13392 camera bytes and 432 helper snapshot bytes per path, 12 ordered events, 14736 identical normalized snapshot bytes per path and 36 identical normalized helper bytes before free. The 9492 mapped code bytes remain unchanged after both paths and the failure observation.

One source-only renderer-call2 failure verifies unregistered first-viewport cancellation/free, both local/camera raw-string returns, exact primed camera slot/pool return, helper base/count1/zero camera representation, absence of helper companions, settled block and one explicit host-quiescent reset before caller-owned helper free. No original FH3 exception path is executed.

The actual built object preserves the required near/far/depth x87 materializations and fresh camera reads. MSVC initializes the private unused atomic count to zero before the explicit native count-one sequence and before publication or callbacks; this does not establish native write-trace parity.

The ledger adds one complete 339-byte native body; the persistent host block adds no native body credit. Ghidra keeps BSP_CockpitHelper_Construct and its original thiscall signature, with prior comments preserved and new evidence appended, saved and re-exported. Exact hashes, independent reviews, the immutable validation archive and annotation audit are recorded in `reports/native_cockpit_constructor_bj_validation.json`. No new repository test was added. Original replacement ABI, native FH3/SEH and gameplay remain unvalidated; parent render-resource construction and actual startup composition continue separately.
