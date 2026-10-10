# Audio endpoint registration readiness

The current Windows audio stack exposes **no render endpoint in any state**.
At 2026-10-10T10:51:17Z, Console session1 MMDevice creation, enumeration with
`DEVICE_STATEMASK_ALL=0xF`, and collection count succeeded, with count0.
Console, multimedia and communications defaults returned `80070490` and null.
No endpoint object was available for GetState; untouched State0 fields from
null default objects are not valid state outputs.

The installed authoritative header defines active1, disabled2, not-present4 and
unplugged8. The complete all-state API collection is empty, including disabled
and unplugged endpoints. This is stronger than the earlier active-only query.

| Observation | Result |
|---|---|
| MMDevice render endpoints, all states | 0 |
| Registry Render records | 39 |
| Registry documented low state bits | 5 active, 27 not-present, 7 unplugged, 0 disabled |
| PnP AudioEndpoint records | 9 non-present: 8 render, 1 capture |
| Present Media devnodes | 14, each ProblemCode0 |
| Matching Media kernel-driver services | 10 running |
| Kernel render interfaces | 12 enabled, 75 disabled |

Four registry records have raw DeviceState1; HyperX Chat has `0x10000001`.
Six others have `0x20000004`. Full values are retained; only low0xF bits are
decoded, and both high-bit flags remain unclassified. A registry active bit
does not establish an API-visible active device. Serialized registry container
blobs are retained without guessed decoding.

Eight Render registry GUIDs join exactly to the eight non-present render
AudioEndpoint PnP identities. Their parent IDs join to present Media devices
where observed. ProblemCode on those software endpoints is Empty/null, not
assumed to be0,22 or45. Registry HyperX/Realtek/Steam labels with no corresponding
software endpoint are compared with current Media names only as weak candidates.
Complete identity joins and each record's provenance are in the receipt.

Enabled render interfaces join by exact instance ID to HyperX Game and Chat,
Realtek USB Audio, Steam Streaming Speakers/Microphone and Bluetooth audio
devices. Realtek supplies three enabled render interfaces; the other nine
matching Media identities supply one each. Their actual INF/version/service
identities and running kernel drivers are retained. Audiosrv and
AudioEndpointBuilder are running automatically; MMCSS is a running automatic
kernel driver. These observations span 10:51:17–10:53:59Z, not one atomic snapshot.

The immediate missing contract is a **present, published MMDEVAPI render
endpoint**, above existing Media/KS registrations. This localizes the gap without
identifying why publication/enumeration is absent. Healthy devnodes and enabled
interfaces do not prove an audible route, connected transducer or fully correct
driver. The 75 disabled kernel interfaces are not 75 disabled playback endpoints.

There is no established endpoint to enable or select as default. No present
Media node reports the documented disabled problem22, and the all-state API
exposes no disabled endpoint. The evidence also does not establish missing
hardware or a driver-install prerequisite. A device cycle, service restart,
driver installation or new connection would be an unverified experiment;
this packet makes no concrete mutation proposal. Further endpoint-publication
diagnostics can use the exact retained Media/KS IDs under a separate Root decision.

The prior independent manifested Win32 FMOD enumeration remains separately
retained: installed4.18.4 DLLs, driver count0, pre-init AUTODETECT0 and real release0.
It is not rerun here. Historical genuine Init61/78/37 and failed Source581/539/595
startup records remain unchanged. This packet does not initialize FMOD, launch
the game, inspect Native/GPR, compile, or change any device/default/service/
registry/driver/config/DLL. Normal audio-output startup still requires a real
endpoint and fresh SDK/game qualification; independent reconstruction can continue.

The [complete receipt](../reports/cc12_audio_endpoint_registration_readiness.json)
pins exact commands, raw API/registry/PnP/driver/interface output, local header
constants, complete narrowly scoped frozen evidence and pure offline replay.
The first detailed per-property inventory timed out; the completed retry batches
selected keys per audio device. Its incomplete result was not used. No audio
service/device or other process was stopped.
