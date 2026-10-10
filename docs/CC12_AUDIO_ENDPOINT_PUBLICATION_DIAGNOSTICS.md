# Audio endpoint publication diagnostics

The bounded existing logs and current service configuration identify **no direct
cause or supported repair action** for the missing render endpoints. The prior
10:51:17Z all-state MMDevice count of zero and 10:53:59Z Media/KS inventory are
retained observations; this packet does not repeat MMDevice or FMOD calls.

Existing enabled Audio and Kernel-PnP channels were queried for
2026-10-10T00:00:00Z through 11:11:21Z. Kernel-PnP filters use the exact 23 retained
audio instance IDs and locally verified Media/AudioEndpoint class GUIDs.
Each of the 15 queries had a 65-event cap; none reached it. Disabled diagnostic
channels were not enabled. The entire boot interval beginning September 28 and
System, Security and Application logs were not queried.

| Unique current-day records | Observation |
|---|---|
| Audio Operational: 2 | Session 1 display on/off notifications, 03:17:45Z and 03:19:34Z |
| Kernel-PnP Configuration: 3 | One distinct MMDEVAPI identity configured, started, then deleted |
| Exact retained audio identity matches | 0 |
| Error/warning records in this bounded result | 0; all five are Information |

The PnP identity is
`SWD\MMDEVAPI\{3.0.0.00000001}.{6C26BA7D-F0B2-4225-B422-8168C5261E45}`.
Records 9157/9158 configure/start it at 03:17:45Z; record 9161 deletes it at
03:19:34Z. Their reported Problem and Status are `0x0`. None matches the nine
retained non-present render/capture identities or fourteen present Media IDs.
Its `{3.0.0.00000001}` namespace remains uninterpreted and is not substituted for
a `{0.0.0.00000000}` render endpoint. The nearby display notifications establish
timing, not a cause of the empty render collection.

At 11:13:22Z, Audiosrv PID 4936 and AudioEndpointBuilder PID 4760 were Running,
Auto, with both reported exit codes zero. Audiosrv declares AudioEndpointBuilder
and RpcSs dependencies, both running with exit zero; EndpointBuilder declares
none. The exact hosting processes are in session 0, created at September 28
04:35:17Z, and their PIDs match the prior inventory. There is no observed
current-day process restart. Exact registry configuration and `sc.exe qc`,
`queryex` and `qtriggerinfo` outputs are retained; neither service reports a
registered start/stop trigger.

Configured service files are the actual readable System32 `Audiosrv.dll`
(2,138,112 bytes, version 10.0.26100.5074) and `AudioEndpointBuilder.dll`
(659,456 bytes, version 10.0.26100.1). Complete frozen bytes, SHA-256 hashes and
version metadata are pinned. This is on-disk configuration identity, not loaded
module proof. Different component versions do not establish a fault.
Authenticode status is unavailable because the readonly helper could not load
its Security module; no signature or corruption conclusion is claimed. Two
aborted helper attempts are retained, and the completed capture uses real .NET
SHA-256 over the configured files without changing PowerShell or Windows.

The missing prerequisite remains a genuine published, present render endpoint.
These observations do not identify a disabled endpoint to enable, a failed
dependency, a DLL failure, or missing driver/hardware. A service or device cycle,
driver installation or configuration change is not established as a fix, and
none is proposed. Older records, other providers and disabled diagnostic
channels remain unreviewed; absence of all possible failures is not claimed.
Independent Source reconstruction can continue.

The [complete receipt](../reports/cc12_audio_endpoint_publication_diagnostics.json)
pins commands, raw event XML and messages, query bounds, service configuration,
DLL bytes, prior endpoint evidence and pure offline replay. This packet has zero
SDK Init, game startup, compiler/test, Native/GPR, ABI, gameplay or environment
change credit. Historical failed game startup evidence remains separate.
