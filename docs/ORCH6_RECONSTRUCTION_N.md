# Reconstruction batch N

Addresses: 009F0D20, 00827F70, 009F1420 (bounded fragments), 00928560,
009288F1, 00484540, 004B7EC0, 00963C80, 00963E90.

The rebuilt mission now dispatches registration through each actual descriptor
creator's constructor-proven primary vtable. Previously every unit used the
destroyer registrar, placing planes and buildings in ship list 6. The mission
registry now owns 97 count/head/tail triples and stable 12-byte nodes, preserving
direct unit identities and per-push parent reloads. This is a semantic runtime
binding; complete native world construction and teardown remain separate work.

The accompanying native modules recover neighbour admission, strict candidate
distance/height gates, callback-sensitive node traversal, timer arithmetic, and
21 creator registration chains. Admission preserves signed capacity, duplicate
visits and lifetime extension; candidates reject equality at either distance
boundary. Their injected services explicitly retain unresolved owner behavior.

Original-byte fixtures independently checked admission, candidate/timer behavior
and registration with stable or mutating parents. Each worker passed Win32 and
both existing CTests. The combined commit
`6978250300d3dbcb59ca24c4ac631f6291ff8f7e` also passed Win32 and both CTests.
Its 120-frame USN01 process created 77 units and 420 linked nodes, with 14 ships,
20 planes, no unresolved registrations and no invalid lists. All 97 list counts
matched the creator-specific native report. All 18,557 trajectory rows remained
finite; 241 Airfield2 samples retained its position. The run also retained the
existing 2,400 avoidance queries and 10,080 generic ticks.

Exact executable hashes, retained process artifacts, packet commits and fixture
limits are in `reports/orch6_reconstruction_n.json`. All 346 worker artifacts
were copied and hash-verified before any worktree cleanup. These checks do not
establish original-game visual/gameplay parity or binary ABI compatibility.

## Follow-up packets

The actual brain timer/RNG/lock production, unit neighbour-field transitions,
and callback-sensitive pre-pass sequencing are separate O/P packets. Candidate
collection must wait for their owners and complete node/observer lifetime.
The enclosing placement, hierarchy/activation links, world destructor and
repeat-load lifecycle remain unresolved runtime work.
