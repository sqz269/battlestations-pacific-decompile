# Win32 game placement for native read-only data

Addresses: none (source executable layout; no native function or ABI claim).

The source game consumes verified original read-only tables at absolute native
addresses. Its suspended-child bootstrap must reserve the four 64-KB bands
`00CF0000`, `00D10000`, `00D50000` and `00D60000` before the child loader and game
initialization allocate more memory. The AB diagnostics recorded the initial
stack overlapping CF, a private allocation overlapping D1, and a rebuilt image
overlapping D5/D6. A successful later launch did not remove those failures.

`cmake/native_data_placement.cmake`, included through a single `cmake_language(DEFER ...)`
registration preserved by the registry's mechanical merge, gives
only the MSVC Win32 `bsp_game` target `/BASE:0x10000000`,
`/DYNAMICBASE:NO` and `/FIXED:NO`. The rebuilt image starts at 256 MiB, above the entire original
read-only data range. Its own ASLR opt-in is disabled, including the heap/stack
randomization associated with that flag. This is an explicit compatibility
tradeoff for the remaining absolute native-data consumers. Other targets, DLL
ASLR, Windows policy and the image's NX-compatible flag are unchanged. Relocation
information is explicitly retained through `/FIXED:NO`. Without that option,
the first production link stripped relocations despite the EDITBIN candidate
retaining them; the final PE check rejects stripped or absent relocation data.

The bootstrap still rejects any occupied required band. It does not free another
allocation, change a live stack, retry until a launch succeeds, or relax source PE
identity checks. System-enforced relocation or third-party mappings can still
produce a guarded startup failure. Validation here covers the current Windows
environment and the recorded executable, not every possible process policy.

Before changing the build, an ignored copy of the last integrated executable was
rebased with MSVC EDITBIN to test this layout. Four independently created children
all reported ASLR flags 0 and successfully reserved the four bands through the
production bootstrap. The candidate's image base was `10000000`, image size
`0020C000`, and DLL characteristics `8100`; the prior executable used `8140`.
The tracked implementation is a link configuration change; the production build
does not post-process or alter the original installed game executable.

The initial production Win32 build and both existing CTests pass. Final compiled
image and mission-run evidence are recorded in `reports/native_data_placement_ac.json`.
No new repository test cases or workers are added. This improves source startup
compatibility; it does not establish reconstructed gameplay parity.

References: Microsoft's [/DYNAMICBASE documentation](https://learn.microsoft.com/en-us/cpp/build/reference/dynamicbase?view=msvc-170)
describes its effect on image, heap and stack randomization. The
[process-creation attribute documentation](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-updateprocthreadattribute)
describes mandatory relocation and bottom-up policy. Neither Windows policy nor
process-creation mitigation attributes are modified by this change.

## Final verification

Compiled `e123041f4169e51caf052e09ecc93e501e964aca` has image base `10000000`, image size `0020c000`, DLL characteristics `8100` and 71648 bytes of relocation data. The four requested native-data bands are outside the rebuilt image. All four independently created production children reported ASLR flags 0 and passed reservation/resume.

The Win32 build and both existing CTests pass. The normal USN01 launch maps the verified original read-only data, runs 120 mission frames, exits 0 in 9.848 seconds, and produces 18,557 finite trajectory rows. World registration, observer lifetime and pending-queue checks pass. The final executable SHA-256 is `6d96913c9aa58ea507dbeaf83fb5d45163307121b6e18d35aaad85a6dff0312a`. The report records the separate manifest of 42 retained artifacts. These checks establish the tested local layout and mission compatibility, not original-game gameplay parity.
