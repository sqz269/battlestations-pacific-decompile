# Source157 VFS failure retention admission

Direct Runtime request/completion calls can leave positive retained failure
history while GameVfsHost's invoke_native flag remains clear. Author
`8d98b9bf4eee16ea5dee38da930eac3c1494dab1` adds the existing failure_site query
to the existing facade retention predicate, after its unchanged flag/title
short-circuit arms. Root's existing guards now see this history and follow
their established whole-graph retention route. No Native cleanup/retry changes.
The existing core_ready guard ensures the SAME retained Runtime exists before
that query; false still grants no disposal or callback last-use authority.

Root and independent Source/Code reviews pass with zero findings. Actual
before/after registered Game objects compare3337 canonical text sections:
3335 exact and only the accessor/Root guard changed. The complete old/new
sections total86B/32 starts and110B/39 starts; Root reads all of them. The final
two sections and all1012 VFS implementation text sections transfer exactly
after joining published Upper6 Main. Actual project/link inputs are verified.

First build passes in89.5153259s/checks0.63s,22.91s. Final coupled head
`4e367017901e1ec41a0fb49e737c5651c02a179e` passes in57.9917475s/checks0.37s,
23.13s, freezing3938 inputs/248 artifacts. Published Upper6's qualified
Source/Code/build admission is reused without new body audit.

The final executable passes the existing windowed Atlas startup/normal shared
retirement regression:4 textures,227 items,47 slots, natural exits0/0,
fullscreen0, final COM0/0, no forced stops/survivors, all frozen inputs unchanged.
The positive failure branch remains unexercised. FrameBox and other GUI paths
stay unbound/unexecuted; all nine selectors remain OFF. Source144 record and
callback last-use closure remain required. No new test/probe/Ghidra action or
Native/vendor/Original ABI/fault/Origin/FP credit. Exact peer/build/Code/runtime
seals are indexed in the adjacent admission JSON.
