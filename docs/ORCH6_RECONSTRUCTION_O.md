# Reconstruction batch O: producers and avoid-box math

Addresses: 009F1160 (tail 009F126B..009F1401), 009EAFC0,
009EB4D1, 00415510, 00415620.

The brain producer tail now preserves seven draws from the application's
existing secondary RNG stream, four fresh settings returns, the original
timer/countdown store order, and actual tracked-lock construction last. The
ship and torpedo scan countdowns keep their initial negative 0..1 draws when
later settings draws overwrite only the periods. Settings +194 is the upper
ShipAvoidance.CollectTimer bound, also reused by neighbour admission lifetime.
This is a bounded tail with borrowed owners; it is not a whole brain constructor.

The avoidance-box correction preserves NaN through the native shrink gate,
minimum helper, closing-speed floor and turn clamp. It also follows the x87
shrink arithmetic and arc-product spills and uses the actual widened-float
three-degree threshold. Fourteen cases at PC24 and PC53 match 1,188 canonical
words from the original bytes; the prior source fails 11 of those invocations.
This establishes the checked cases, not every floating-point input.

The combined commit `3e29fad155f6fd49ee66b19053eadfa1c4c8e38b` passed MSVC
Win32 and both existing CTests. A 120-frame USN01 run retained 18,557 finite
trajectory rows and the N batch's 420 valid nodes, 14 ships and 20 planes.
This is a process regression check; execution of the new producer and avoid-box
branches is established by their separate fixtures, not by that mission run.
Exact hashes and limits are in `reports/orch6_reconstruction_o.json`; 106 ignored
worker artifacts were archived and hash-verified. Tracked evidence stays in Git.

## Follow-up packets

The active unit-neighbour-fields O packet and pre-pass/throttle-clamp P packets
remain distinct. Runtime candidate collection still requires a complete brain
prefix, the preceding navigation RNG draw, persistent unit flags, and complete
node/observer lifetime. The application's `GameSingletonHost` owns a raw manager
publication and fixed deletion dispatch; `NativeObserverLifetime` currently
requires a semantic `SingletonLifetimeDomain`. A future adapter must borrow
the same application publication and preserve captured-lock and second-getter
ordering. A second runtime domain would give these observers the wrong lifetime.

Native `009E52E0` zeros the actual observer array, writes its owner-table identity,
registers the observed endpoint, stores the controller at +1C and leaves the
unwritten geometry bytes untouched. Existing partial fields do not substitute
for that complete owner. Pending-queue flag writes must likewise reach retained
unit storage. The current fresh-flags/discarded-store adapters remain explicit
runtime gaps. No binary replacement or original-game gameplay claim is made.
