# Descriptor shader profile selection

The profile fragment of `00b43b00` at00b440e4..00b4427e selects descriptor strings
at +34h/+38h (vertex length/data) and +3Ch/+40h (pixel). VSVersion and PSVersion
are checked independently with `00b660a0`, which accepts the Lua string type4.
String values override defaults without profile validation; empty strings remain
empty. Copy length is determined with strlen, truncating embedded NULs.

Absent or non-string values select defaults from the reader's unsigned second
stack argument: below3 selects vs_2_a and ps_2_b; at least3 selects vs_3_0 and
ps_3_0. Assembly uses CMP with EBX=3 then JC, not a signed comparison. The
effect-loading calls00b45f5e and00b46104 explicitly push3 before their filename
argument. This is a caller-supplied version policy, not a direct caps query.

`select_shader_profiles_00b43b00` reconstructs this fragment with optional
strings representing successful native string checks. It does not implement
Lua conversion, descriptor lifetime or the full reader. Profile selection is
counted as a fragment, not a complete descriptor reader reconstruction.

The installed debug/dummy descriptors contain no profile overrides. The generated
draw now uses the recovered version3 defaults for both stages. VS3/PS3 compile,
bind and render the fixture with centerFF407FBF/outsideFF000000 unchanged. Win32
build, both existing CTests and full D3D9 probe pass. No new test target was added.
Lower-version and explicit override cases are implemented from assembly but not
exercised by this draw. The original compiler wrapper, flags, optimization and
bytecode caching remain unported; this does not establish bytecode identity.

The original/saved profile fragment and caller argument bytes agree; hashes and
assembly are in `reports/shader_profiles.json`. Previous docs' fixed vs_2_0/ps_3_0
probe choices are historical; both generated game-statement stages now use3_0.
