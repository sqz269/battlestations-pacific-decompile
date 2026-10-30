# Read-only named-unit discovery in the original executable

`tools/original_scene_units.py` follows the original entity registry. It opens
only the sampler's query/read process handle and checks the original EXE hash.
It never calls a native getter, issues an order or writes game memory.

Assembly `00925180` reads `game=[00E188A8]`, registry `[game+19CCh]`, list
owner `[registry+4]`, head `[owner]`, then follows `entity+38h`. Its name lookup
uses vtable slot `10h`. The exported assembly and saved live Ghidra program
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` were checked on
2026-10-29. `00925A90` provides a separate qualified-party lookup, which this
tool does not emulate.

The tool interprets only a bounded in-image name getter consisting of
`MOV EAX,[ECX+disp]; RET` or `LEA EAX,[ECX+disp]; RET`, optionally behind one
direct in-image jump. It records the actual getter bytes, member offset and
string pointer. Unsupported getters are errors, not guessed layouts. A matching
native name becomes a sampling candidate only after the existing motion-layout
checks succeed. Native scene names still need mapping to visible ship names.

Example (game must be the owned guarded run):

```powershell
python tools/original_scene_units.py --pid <PID> --expect-sha256 b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6 --match Lexington --match Yorktown --output local/original-baseline/named-units.json
```

The resulting `units` array is accepted by the schema-3 sampler's `--units-file`.
Discovery is bounded and unsuspended; link/name changes, address reuse and an
incomplete walk are explicit limits. Numeric layout validation alone does not
classify a ship or establish visual identity. Live getter forms and selected
names must be retained with the run receipt.
