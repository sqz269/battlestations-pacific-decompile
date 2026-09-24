# Original sampler capture, CC10 run01

The single original-game attempt closed **inconclusively** before its PE entry
point. It yielded no sampler input. The helper rejected a first-chance
`C0000008` exception and terminated its owned child diagnostically with exit125;
the helper exited1. Both owned jobs reached zero active processes, the owned
process handle signaled, and no live child was detached. No retry occurred.

The attempt used an independently copied, verified 67,814-file game directory,
the unchanged original executable and copied `xlive.dll`, empty original argv,
unchanged options and original scheduling. Wrapper flags were consumed by the
adapter. The reviewed controller created a suspended wrapper and assigned its
private job before resume. An immediate authenticated product-folder backup and
the reviewed helper hashes bound the single-use admission. A benign45-second
timeout exercise had already proved owned-child termination and empty jobs.

The original child was PID90508, primary TID81476. Fifty loaded-module file
hashes succeeded, including the exact copied `xlive.dll`. Its first-chance
exception address `7FF9CFC64EBA` falls in the observed native ntdll mapping at
RVA164EBA. The matching installed PE export and runtime-function interval place
it at `KiRaiseUserExceptionDispatcher+3A`, the instruction after a call to
`RtlRaiseException`. This establishes the dispatcher neighborhood only. The
failing API, handle and causal caller remain unknown; live exception-site code
and context were not captured.

The transient entry byte at BFD2BD was restored from CC to E8 during cleanup.
The helper then compares all32 expected bytes and verifies page protection;
no cleanup failure was logged. It does not emit a standalone raw post-cleanup
readback. Initial protection was20h, cleanup observed40h, and restoration
reported20h. The writer and cause of that change are unknown. Entry rendezvous,
EIP rewind and hardware-marker installation never occurred, so the successful
[entry canary](NATIVE_SAMPLER_ENTRYPOINT_CANARY_CC10.md) does not supply those
missing original-run observations.

The scoped Documents/Battlestations-Pacific comparison found no changed,
missing or new product files. No restore was needed or applied. Inventory-only
access-time changes are excluded from write attribution. This backup is not
whole Windows or XLive profile isolation.

The integration report `reports/native_sampler_original_capture_cc10.json`
pins the helper, controller, backup guard and independently reverified244
preparation,22 run and3 installed-module-audit artifacts. The ignored local
archive retains the complete receipts and earlier failed preparation attempts.
The first sampler source word remains unresolved. No seed, compiler activation,
successful startup, binary compatibility or gameplay result follows from this
attempt.
