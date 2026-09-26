# USN02 hit records 769 -> 358: bisecting fad22c424..041970ada

Packet `cc9_usn02_hits_attribution`, runs only, no code change. It closes or narrows the flag under
the "Mission reference baselines, 2026-09-26 b" section of `docs/GAME_EXECUTABLE.md`. At 9000
frames, USN02 went from 22 deaths / 769 hit records (main `fad22c424`) to 23 / 358 on the
component failures' OFF build (landed as `041970ada`). `docs/SHIP_FIRE_FLOODING.md` section 7
showed that fire and flooding account for only 33 hit records on today's tree.

## Method

* Each build is a `git archive` export of a main first-parent commit, synced into
  `local\bis_src` (only changed files are rewritten, so the CMake build stays incremental).
* Every run is USN02 9200/9000 at 0.05 s with `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, one
  at a time, through `tools/run_game.ps1`.
* The window's first-parent merges, oldest first:
  `347ef9007 44631c067 da4ba1baf ee6891e2e 5f7daab07 3a7939600 aee2eaf1b 9f5c9e373 30202e5f8
  39f17e0cd 25b5f4fb8 28a996994 f1a6fab77 57f4d1f23 7af330c36 cadbaf902 249726381 4e308758b
  21050f24e 7a6025ddb 0e51c215c cadea6421 5ff44d40b 23238ecba 041970ada`.
* `kReconTeamListsBound` (`a6a77c147`) and `kPartySlotAiHeldBound` (`d0caf31e2`) are ancestors of
  `fad22c424`, so neither can be in the step.

## Predictions (written before any run)

1. The two ends reproduce: `fad22c424` gives 22 / 769 and `041970ada` gives 23 / 358, both
   failing at 44.60 s. If either end does not reproduce, that is the finding, and the bisection
   stops.
2. The fall sits in one merge. Most likely is a dogfight-engaged merge (`347ef9007`..`25b5f4fb8`),
   because it changes ship behaviour through shared units-host code: the task-less arms and the
   avoidance gate. Next most likely is `0e51c215c` (fire/flooding), at most about 40 records. The
   cc10 merges (render and Lua readers) move nothing.
3. Shots move with the hit records: a fall of about half in shells fired, since USN02's hits are
   mostly gunfire. Deaths stay within 22 +/- 3.

## Runs

Pending.
