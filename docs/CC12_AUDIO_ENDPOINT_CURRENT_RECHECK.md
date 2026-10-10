# Current audio endpoint recheck

At 2026-10-10T03:36:52.3564957Z, the Windows MMDevice API still reports zero active
render endpoints in Console session1. Enumerator creation, active enumeration
and collection count all succeed. Each default request for console, multimedia
and communications returns80070490 with no endpoint. This is a fresh endpoint
observation, separate from the earlier PnP/registry records.

The read-only query runs in a64-bit PowerShell process (PID27380).
Its COM declarations follow the inspected installed SDK mmdeviceapi.h interface
GUIDs and method order. Complete script/header/raw output and empty stderr are
frozen. This recheck does not execute FMOD or the game and is not a fresh32-bit
SDK driver/init result. The [previous startup diagnosis](CC12_FMOD_CURRENT_STARTUP_FAILURE.md)
retains FMOD61/78/37 and the earlier manifested32-bit SDK probe; those remain
historical evidence. The current endpoint finding does not prove that endpoint
absence alone caused that failure.

The runtime prerequisite remains an actual active/default playback endpoint
visible to the same Console session. Once available, endpoint and genuine
32-bit FMOD/control startup checks must be repeated before claiming resolution.
Device/default selection, services, drivers, registry, FMOD settings, original
installation and Ghidra are unchanged. No C++/CMake/build/test change or new game
execution is credited. Independent reconstruction work remains available.

Report: [complete current observation](../reports/cc12_audio_endpoint_current_recheck.json).
