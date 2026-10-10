# Allocation statistics startup runtime recheck

The one authorized Source3348 startup recheck completed the genuine allocation
statistics construction interval and then reproduced the existing FMOD startup
failure. No new statistics failure was observed. Full startup remains incomplete.

The immutable accepted `bsp_game.exe` ran once on 2026-10-10 at
13:25:52.162–13:25:52.397 UTC, PID 50296, with a three-frame limit and a
30-second timeout. It exited with code 1 after 0.226 seconds; no timeout
termination was required. Stdout and stderr were empty. Source reported no window,
device, presented frame or completed loop. Hidden startup was requested and 16
process-specific window scans observed no top-level window. This was not a visual
inspection or proof about an unobservable transient window.

Log lines 57–59 record the statistics completion marker, actual frame-clock
continuation and handle resolvers; line 62 reaches VFS. The marker is meaningful
against this accepted source: `singleton_lifetime_allocate` returns a nonnull
allocation or throws; the genuine BE2900 constructor, single saved AB0 comparison
and normal handover precede the marker. Thus the reached continuations support
normal constructor completion in this run. This is a source/control-flow inference;
no live receiver pointer, publication cell or manager slot was instrumented.

Lines 84–87 report the same previous sequence: `_FMOD_EventSystem_Init@20` returns
61, `FMOD_System_CreateSound` returns 78, and `FMOD_Sound_GetLength` returns 37.
The existing raw-length guard stops on `sound/gui/error.fsb`, 2,688 bytes, mode
2,634, with no bank returned. Source throws before consuming the unwritten length.
This identifies the first logged SDK failure as initialization; it does not identify
the underlying Windows/audio cause or establish corrupt bank data.

The previous proven argv was preserved, including the private XLive DLL and all
three explicit dependencies. Only the executable path to the accepted immutable
Source3348 copy, log path and diagnostic personal-root path changed. The entire
existing personal-root directory was copied. Its one options file remained exact;
640×480 windowed settings were loaded. Full preimages and postchecks retain the
explicit private DLLs, original image used as data, bundled FMOD DLLs, the loose
bank input and personal file. All these inputs and the accepted executable remained
byte-identical. No separate SDK probe, audio selector/SetOutput change, build,
new test, original-executable launch or Ghidra mutation was performed.

The receipt freezes all 3,352 accepted Source/build pins and all 3,348 complete
selected Git blobs at `48d432db3863ea5b0254dc05a8bdbd405dbb74c6`. Seven files
have only CRLF/LF differences between accepted Root build inputs and Git; both
complete byte versions are retained. Complete Root receipts remain preserved,
including the prior three-check build, 1,057 selected whole Core member pins,
70 selected App objects and 320 positive-definition records. This packet replays
the source/artifact pins, Core byte spans and App pins; it does not rerun the prior
build, machine analysis or recursively replay older nested worker bundles.

The portable bundle has 3,469 payloads totaling 213,852,900 uncompressed bytes.
Its retained-byte replay passed. Copy `evidence.zip` and `portable_replay.py` from
`local/cc12_allocation_stats_startup_runtime_recheck/`, then run
`python portable_replay.py evidence.zip`. The replay reads only ZIP bytes and
does not execute the application or consult original absolute provenance paths.
The live Windows/SDK/device state and all game-package contents are not a captured
portable runtime environment; this receipt does not predict a future run.

Source startup evidence grants no original Native execution, register ABI, private
FH3, hardware-fault or gameplay validation. No statistics fix is indicated by this
run, and no automatic repeat or broader runtime action was taken.

[Complete receipt](../reports/cc12_allocation_stats_startup_runtime_recheck.json).
