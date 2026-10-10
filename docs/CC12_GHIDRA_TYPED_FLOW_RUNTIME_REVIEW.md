# Typed Ghidra metadata: runtime review

The reviewed minimal overlay is installed and its named BSP getter now passes
runtime validation. Root saved the explicit BSP program, requested a graceful
close, confirmed the target JVM/listener exited, verified a complete closed
project and original-extension backup, replaced only the reviewed JAR, and
directly relaunched the same saved GPR. No force-kill, script setting, analysis,
project-state injection or Native flow/body/prototype repair occurred.

The operation held the shared Ghidra write and launch locks in one Root process.
Our workers paused live queries; independent-harness reader quiescence was not
attested. The locks serialized writes and competing auto-starts. The closed
project backup contains16 files/460,151,125 bytes plus the GPR marker, original
extension and clean-exit tool template, with every copied file hash verified.
Original game bytes remain unchanged. The saved DamageableClass parent name
and entire Source603 evidence comment survive reopening exactly.

Ghidra reopened /battlestationspacific.exe with64,730 functions. The first raw
capture was retained as rejected: its real ProjectLocator directory used the
documented /C:/ form. The narrow client correction keeps actual marker and full
identity checks strict. A fresh four-address named capture passes schema1,
complete exact ranges/null listing states and unchanged modification3. It
attests the actual C:/Users/sqz269/bsp.gpr marker, program/language/image base,
and each explicit query; no path was substituted from configuration.

The runtime metadata resolves the atlas listing question. AEEAD0 owns exactly
two ranges, AEEAD0..AEEAE4 and AEEAE8..AEEAED, totaling27 addresses. AEEAE5 is
undefined DataDB with no instruction or containing function. At AEEAE0, the
prototype flow is UNCONDITIONAL_CALL, current effective flow is CALL_TERMINATOR,
and the actual FlowOverride is CALL_RETURN. The independent fallthrough-override
flag is false. Both reported fallthrough addresses are null. Its exact target
BF65AC is the _free thunk with NoReturn=false; its direct BF9DC8 target also has
NoReturn=false. Thus an explicit callsite override is observed; changing a callee
NoReturn flag is not justified by this evidence. Earlier original-PE review
retains the physical30-byte wrapper including its missing three-byte ADD ESP,4.

No Ghidra repair or Source admission follows automatically. A separate bounded
atomic repair route, expected preimage/rollback and affected-body review remain
required. Current metadata is distinct from original ABI and game behavior.
The unchanged version resource labels the preserved original build timestamp;
exact loaded-class CodeSource/bytecode hash remains unattested even though the
new getter's runtime contract works. Disk package/entry hashes are retained.

No new C++ build, fixture or game run occurred for this tooling rollout. The
Source603 reconstruction build separately passed three existing checks. The
last production startup run failed in FMOD before graphics/Present; successful
startup and gameplay remain unvalidated. The continuing goal remains active.
