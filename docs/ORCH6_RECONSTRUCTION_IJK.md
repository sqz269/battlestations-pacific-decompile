# Orchestrator6 reconstruction batch I/J/K

The corrected combined executable at `87f9a9e0` runs 120 mission frames with live
avoidance queries and canonical constructor-owned role reads. Win32 Release and
both existing CTests pass; the actual-host owner probe passes 56 checks.

The mission exits 0 with 18,557 finite trajectory rows. Airfield2 retains its
authored pose through 241 samples. Searcher zero receives 2,400 queries with 10
refills; request changes clear 12 lists. Cruise reads 1,080 owner inputs without
unavailable results. Current roles retain constructor value 8, so participant
AI-held reads are not required by this run.

The motion correction selects existing simulation through proven allocator and
tick-table identities. AirField no longer enters ship-only steering/buoyancy.
Three verified subclass base calls remain partial overrides; other phases are
explicitly recorded. Finite output is process evidence, not simulation parity.

The first combined attempt at `3db73f6e` failed with an access violation: geometry
loading retired cache owners after controller registration. The correction
retains owners, clears borrowed lists and invalidates keys before replacement.
Both exact executables and their logs are retained in local/. This cache epoch
management is an explicit process lifetime adaptation.

The batch integrates search storage, endpoint clearance, director/cruise request
adapters, stored tuning, role ownership, motion dispatch, two obstacle point
predicates, participant pools and current-role storage. The point predicates
remain unbound until their geometry/validity producers are connected. Session
pool publication and actual role-assignment delivery remain separate work.

All 122 direct-call rows pass the mechanical recheck. Four indirect rows retain
their separate vtable evidence. Ghidra wrappers 00749B20 and 00758270 are named and
saved; 009D8160's older strict-edge description is corrected to inclusive edges,
preserving prior annotations. A forced snapshot refresh corrects stale lookup.

The nine worker evidence sets contain 498 archived artifacts, all hash verified,
with no historical cache changes. Completed worktrees were removed after clean,
merged and hash audits; active workers remain. Exact commits, source/executable identities,
fixture coverage, commands and artifact hashes are in
[the batch report](../reports/orch6_reconstruction_ijk.json). Live ownership is
described in [SHIP_AI_LIVE_AVOIDANCE.md](SHIP_AI_LIVE_AVOIDANCE.md).

This is exported, reconstructed, build-tested, fixture-tested and process-run
evidence with the separate scopes above. No binary replacement, original-game
visual parity or gameplay-validation claim is made.
