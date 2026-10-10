# FMOD endpoint recheck primary review

Accepted worker `74ea45d4e39321e923dc80e64ddf658139c87d86` as current read-only
environment and installed-SDK enumeration evidence. Root retained the complete
local directory, compared all complete current Source/Git inputs after EOL
normalization and invoked the pure offline replay from its own retained copy.
The replay passed 101 pin occurrences, 85 unique files and ten full immutable
Source993 comparisons. Its two raw Source differences are EOL-only.

At 2026-10-10T10:18:19Z, active MMDevice render endpoints were zero; all three
default roles returned `80070490` and null. Audiosrv and AudioEndpointBuilder
were running automatically. MMCSS was also running automatically as a kernel
driver; its absence from Win32_Service does not mean it was missing.

The manifested Win32 probe used the genuine installed FMOD 4.18.4 DLLs. Creation,
system/version/output/driver enumeration and real EventSystem release succeeded.
Version was `0x41804`, output was pre-init AUTODETECT0 and drivers were zero.
It did not initialize FMOD, select output/drivers, access banks or launch a game.
Historical failed starts remain separate evidence; endpoint absence alone does
not prove the sole cause of Init61 or success after that prerequisite is met.

The existing Source NOSOUND branch requires real SDK initialization and resources.
The ordinary executable exposes no supported selector: SoundEnabled consumes
the keyword without applying its value, while the constructor enables sound.
Changing that behavior needs separate Native semantic evidence. A bounded
read-only Native-selection packet is in progress.

No C++ changes, new tests, Ghidra mutations, services, devices, settings or game
installation changes occurred. This review earns no new initialization, graphics,
startup, Native ABI or gameplay validation.

The [primary receipt](../reports/cc12_fmod_audio_endpoint_startup_recheck_primary_review.json)
pins Root's retained evidence and complete Source comparisons. The
[worker receipt](../reports/cc12_fmod_audio_endpoint_startup_recheck.json) preserves
the exact calls, commands, probe manifest/source, raw results and earlier attempts.
