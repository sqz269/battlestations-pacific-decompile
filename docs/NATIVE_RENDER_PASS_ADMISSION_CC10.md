# Canonical admission of eight service-visible passes (CC10)

This is source provider composition in the existing B107F0 continuation and
second-bloom stages. It introduces no native body, changes no Ghidra address,
name or ledger, and does not invoke the application initializer. B107F0 and
its callees were read-only external evidence for this file-only packet.

The continuation now requires a `NativeRenderPassCompanionContext` in the
same actual owner registry and effects-lifetime domain as its existing post
children. It retains seven independent `NativeRenderPassReference` objects;
the second-bloom state retains an eighth. Each reference borrows the actual
owner+4 atomic. Binding adds no native write, allocation, retain, release or
counter. Only canonical host metadata may allocate. The existing bridge owns
the eventual scalar deletion after a genuine final decrement; direct raw
deletion must not also be performed once the companion is bound.

## Producer and publication schedule

Every non-null owner is bound after its native construction is complete and
before its original service-field publication or initializer callback. The
extra metadata boundary preserves the original native captures and stores.
No companion is attempted for a null allocation; native null publication and
later provider preconditions remain intact.

| Field | Profile | Native allocation | Publication | Initializer | Existing terminal |
| --- | --- | --- | --- | --- | --- |
| +60 | D5E164 | 20h | B10C1D | B540B0 | B10120 |
| +64 | D5E178 | 20h | B10C5B | B542D0 | B10140 |
| +18 | D5E18C | 220h | B10E8B | B544F0 | B10160 |
| +1C | D5E1A0 | 90h | B10EEE | B546F0 | B10180 |
| +20 | D61FE0 | 250h | B10F5A | B51090 | B50FE0 |
| +24 | D5E1B4 | 224h | B10FA2 | B54940 | B101A0 |
| +28 | D62150 | 43Ch | B10FE4 | B54F90 | B54F70 |
| +2C | D62150 | separate 43Ch | B11E2F | B54F90 | B54F70 |

For +60, retained EBP=0 and the input/dimension captures precede binding.
For +64, the original word00 read at B10C50 precedes binding. Current+64 is
captured before +18 admission; current+18 is captured before +20 and +24
admission. The +1C binding precedes its unchanged publication/dimension
sequence. The +28 actual CE3854 x87 copy is complete before metadata runs.
The +2C binding follows its entire FLD1/FSTP, half-dimension and current+1C
capture block, then precedes EH disarm/publication. No metadata call runs
while that source assembly block retains a value in ST0.

## Cleanup preconditions established by actual constructors

The five inline pass constructors initialize their real count to one and
their native owned pointers +8/+C to zero before their derived profile store.
Their B0F5E0 base cleanup releases only these pointers; the full B101A0 bright
wrapper uses that same base. No parameter-array or padding initialization is
needed for admission, and none is added.

The complete B50D40 luminance constructor initializes +8/+C, +210,
+214..+23C, +244 and +248. These include every owned slot released by B50DD0
and the common base. The complete B54E70 bloom constructor initializes
+8/+C/+18/+1C/+20/+24, covering every slot released by B54EA0 and its base.
Both independent blooms therefore have a valid constructor-only cleanup
state before initializer callbacks can expose partial work. This conclusion
does not extend to distortion, whose separate preimage contract is unchanged.

Binding uses the existing bridge's current-profile, current virtual0/deleting
slot, live actual-count and duplicate checks. A populated child remains under
its real canonical-post or direct-holder lifetime, as in the existing pass
destructors. No nested resource is registered by this change. Existing post20
and post24 construction blocks continue to provide their own companions.

## Retention and failure boundaries

The same companion context object is retained in the original continuation
state and checked before second-bloom consumption. Its registry must be the
same canonical post-owner registry; its effects member must be the same
effects-lifetime context. All contexts, states and child acquisition blocks
must remain alive through callbacks, final native release and subsequent
external quiescence. A companion's destructor still requires retirement.

The new per-pass binding-started/publication-site diagnostics distinguish a
host admission failure from an initializer failure. An emplace failure keeps
the completed raw owner and all acquisitions, leaves the original parent
publication unperformed and fails the already consumed stage. A successful
binding remains engaged across later initializer failures and callbacks.
The caller adds no repair, raw free, rollback or retry. Callback replacement
owners still need their own valid canonical identity; binding the original
allocation does not certify a later current parent value.

This closes only these eight service-visible pass bindings. B0F6E0's broader
surface/holder/runtime-texture terminal domain remains separate. The bloom
43Ch allocation versus borrowed +430..+43F range, distortion cleanup preimage,
partial-failure disposition, full initializer/service teardown, original
exception/ABI/private-stack behavior, application invocation and runtime/game
proof are not resolved by these bindings.

## Evidence and validation

Read-only wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Eight exact live byte blocks cover the original
producer/initializer intervals and complete luminance/bloom constructors;
all match the installed PE with complete instruction boundaries. The report
records 27 caller rows and 14 existing scalar-wrapper support rows, including
their original allocator/deleter targets. These are evidence for reused
providers, not new function reconstruction claims.

The existing bridge report already contains one actual-D3D bright-pass
lifecycle check proving unchanged owner bytes/count at bind, duplicate
rejection, nonfinal/final release and registry retirement. This packet adds
no duplicate probe or tracked test. All 41 address/native rows verified with
zero failures. `scripts/build.ps1` passed MSVC Win32 Release `/MD /W4 /WX`
and all three existing CTests (`reconstructed_math`, `native_math_differential`,
`tool_tests`). New staged wiring remains runtime-unchecked.
Ignored evidence uses `local/render_pass_admission_*`; all previous tail,
caller-correction and composition artifacts remain untouched.
