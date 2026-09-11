# Startup backend reconstruction and integration
Addresses: 00A4C660, 00A4CA40, 00A4CD70, 00A4D140, 00A4D1B0, 00532360,
00530A60, 00531380, 00532110, 007FDF00, 007F9540, 007FA670, 007FA220,
007F9500, 007F8F00, 008D6170, 008D5150, 008D64A0

The second orchestrator continues in `agent/orch2-20260910`, synchronized
through main884b786 before this batch. Separate leased workers own the
decoder, prompt layout, and profile archive. The other orchestrator owns
main's renderer reconstruction. Ghidra operations verify projectbsp and
program/battlestationspacific.exe; workers read only and the integrator
uses the shared lock for mutations.

The Bink wrapper recovers the native38h complete object and28h shared base,
including lifecycle, controls, timing, texture creation and frame copying.
It calls the shipped codec through typed imports. The prompt module recovers
entry, label preparation and the shared sizing/placement tail. The profile
module recovers separate read and write traversals, using the existing Lua
reader and profile models. See the three packet documents/reports for exact
ABIs, native ranges, storage contracts and uncertainty.

`startup_frontend.cpp` now binds MoviePlayer decoder calls to the recovered
wrapper (GUI pointers address the shared base at allocation+10h), prompt
entry/label/layout calls to their recovered bodies, and profile deserialization
to the archive reader. Profile write callbacks call recovered settings/profile
writers and retain/clear the completion slot in original order.

The settings writer's omitted first call was a real options-file write.
`SETTINGS_TEXT_PERSISTENCE.md` corrects the prior hardware-derivation theory
and records concrete Windows/CRT output validation in an isolated local tree.

Profile read007fdf00 had four missing fallthrough blocks after CRT free calls.
The locked repair restored39 bytes, leaving zero CALL gaps and preserving a
six-byte gap following an unconditional JMP. Its saved export is refreshed;
see `reports/profile_archive_flow_repair.json`. The previously documented
short destructor007fd780 remains a separate unresolved metadata limitation.

Cross-reviews found no concrete defects in decoder control/upload ordering,
profile read/write schema and lifecycle, or the integrator's write/options
paths. Focused fixtures cover prompt layout mutation, profile compatibility,
options text order/format and the installed codec check when completed.
Exact combined-build, runtime-probe and annotation outcomes live in
`reports/startup_backend_integration.json`.

The combined code at9b07840 passed MSVC Win32 Release and both existing
CTests. All34 reviewed names/evidence comments were saved and read back with
earlier comments preserved; all34 affected exports were refreshed.

The installed binkw32.dll and movies/fe_eidos.bik passed an isolated real
decoder check linked first against the worker and then against the combined
library. The wrapper opened the1280x720,90-frame movie, sought frame45,
decoded/copied it to a CPU surface, advanced to46, reopened with the observed
completed=1 behavior, closed and released all texture resources. The two
decoded BMP hashes match, and visual inspection shows the Eidos logo.
`local/native_bink_frame.bmp` and `local/native-bink-integrated-probe.log`
hold the evidence. DLL, movie and frame SHA256 hashes are in the report.
This proves the exercised codec/import/control path, not D3D rendering,
audio playback or gameplay. The original game files were used as inputs;
the probe wrote only local evidence.

Remaining services include native renderer/GUI ownership and notifications,
VFS/subtitles, mission-score archives, the profile manager and storage task
scheduler. Bink is not reimplemented. These modules and adapters do not yet
constitute a running rebuilt game; no gameplay or visual parity is claimed.
