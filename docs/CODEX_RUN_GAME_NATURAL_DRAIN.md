# Natural-drain launcher mode

`tools/run_game.ps1 -NoForceStop` retains any tagged survivors after the existing
1200-second post-parent wait. It never enters Stop-Process, treats process-query
errors as errors, and returns76 when the game parent returned0 but survivors remain.
The original parent exit stays in manifest.exit_code; launcher_exit_code records
the effective wrapper result. Manifest drain records the selected policy, waited
time, authoritative natural completion, timeout and requested-stop/retained PIDs.

This is opt-in. The old default timeout policy and tag-based selector remain;
the former comment claiming executable-path filtering was incorrect. Qualification
controllers requiring natural completion must select NoForceStop, require its
manifest flag, natural_completed true, timed_out false, no stop/retained PIDs and
both exit codes0, then independently check actual frozen EXEs/sampled PIDs/slot.
Zero process-query output following a failed query is not natural completion.

The change was prompted by prospective MainSection protocol review, before any
trial. No game, forced cleanup, timeout recovery or runtime qualification is
claimed by this Source change. It adds no executable argument, game-state change,
installation/profile write or production native switch.
