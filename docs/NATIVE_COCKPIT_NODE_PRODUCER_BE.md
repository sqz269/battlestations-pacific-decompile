# Cockpit node producer: native evidence

Read-only investigation, 2026-09-13, base `6be9ccb1185e85b84e4128b67802becd8beacd02`.
Names below are descriptive hypotheses. This packet changes no executable source,
Ghidra state, or ledgers. BSP live commands verify `bsp.gpr`,
`/battlestationspacific.exe`, bridge 8089 before querying. Disk disassembly uses
the configured original Steam executable. No build, fixture, ABI, or game validation.

## Result

Plane `+808` is a **borrowed named node**, obtained by looking up the Note name
`belsocockpit` in its instantiated game resource. It is not constructed by the
cockpit helper. The Note identifies a hierarchy node but does not select its
concrete node factory. The actual cockpit asset hierarchy flags and attached
resource factory overrides remain necessary to choose its exact profile.

The default factory is **model-base `D62D78`**, not generated-model `D62DE8`.
Its terminal `+00=BD30E0`, `+04=B74B60` is not covered by the existing
`NativeModelReference` profile checks at this base. A hierarchy group flag instead
selects group `D634F8`, which has an existing `NativeGroupReference` terminal.
Other resource factory overrides remain possible. Thus neither a camera-only
owner nor an unconditional generated-model owner is justified.

`GeneratedModelLifetimeRuntime::find_actual_node(actual_key)` already exists
(`src/generated_model_lifetime.cpp:60`). The missing evidence is the selected
asset profile and corresponding companion registration, **not a missing registry**.

## Field and lookup chain

| Address | Observed native effect |
| --- | --- |
| `7CFD56` | Plane constructor clears EBX. |
| `7CFDBA`, `7CFDC0` | Writes zero to plane `+808`, `+80C`. Constructor is `7CFD20..7D0344`. |
| `7D73BE..7D73D9` | Reads plane `+360`, then its `+160`; calls `71AD50` with name at `CEB6AC`, ASCII `belsocockpit`. |
| `7D73DE` | Stores lookup EAX directly in plane `+808`; no retain. |
| `7D73E4..7D73FF` | Same lookup chain with `CEB69C`, ASCII `kulsocockpit`. |
| `7D740C` | Stores result directly in plane `+80C`. |
| `7D741E`, `7D7449` | Hides `+808` via `B6DA70(0,0)` then calls its virtual `+2C` with the local vector. |
| `7BC660..7BC677` | Reloads plane `+808`, fetches service `F8D39C->+0C`, passes the raw node to `B3C650`. |

The writes are in `BSP_Plane_ReadPropertyBag`, `7D5D20..7D771E`.
`7BC610..7BC680` requires both fields and the service helper nonnull.
The root has already defined `7BC690..7BC797`; its `7BC707` call passes zero.
The prior attachment document's missing-caller-containment claim is stale.

`71AD50..71AE0E` is ECX=part set, stack=name, EAX=node/null, RET4.
It compares exact names through `71AAE0 -> 719FA0 -> 718C70 -> 711C30`.
The checked vector is partSet `+7C`, begin `+80`, end `+84`, stride 8.
Each record has metadata pointer at `+0`, actual node at `+4`.
`718C8C..718C97` dereferences record `+0` and copies metadata's string at `+8`;
`71AE03` returns record `+4`. Assembly corrects misleading stack/register
pseudocode. There is no reference-count increment in the lookup.

`7BAC80` is only a visibility/virtual-vector helper; it does not allocate the node.
`7D0820` also looks up both cockpit names while processing plane nodes; this
does not write plane `+808` or establish a different producer.

## Resource construction and ABI correction

`879590..8797A4` loads descriptor Mesh through `7188A0` at `879763` and stores
the result in descriptor `+50` at `879768`. The optional enemy suffix/name
resolution precedes that call. `7188A0` gets factory `7175D0` and calls the
resource manager. Factory `71B870 -> 71B810` constructs the 74h resource with
profile `CFD8CC`. Profile words read directly:

| Resource profile slot | Native target |
| --- | --- |
| `CFD8CC +08` | `7137F0` |
| `CFD8CC +10` | `71AED0` |
| instantiated `CFD8E0 +08` | `71B710` |

`87BDFB` takes descriptor `+50`. The plane's virtual `+190` entry at `D060B0`
is `6D1DE0`, exact bytes `MOV EAX,6; RET`. The caller has already pushed float
1.0 (`87BE7B..87BE88`); this RET deliberately leaves it on the stack.
`87BE90` pushes EAX=6, then `87BE95` calls resource virtual `+08`.
Thus this call takes **selector 6 and scale 1.0**, not just a guessed LOD argument.
The sibling path `87BE34..87BE52` similarly leaves its float argument pending.

Ghidra reports no function at `7137F0`. The complete disk body is:

```text
7137F0 FLD DWORD PTR [ESP+8]
7137F4 MOV EAX,DWORD PTR [ESP+4]
7137F8 PUSH ECX
7137F9 FSTP DWORD PTR [ESP]
7137FC PUSH EAX
7137FD CALL B891A0
713802 RET 8
713805..71380F INT3
```

It preserves ECX=resource and forwards both arguments. Root may define exactly
`7137F0..713804`; this worker made no definition.
`B891A0..B8969F` returns an instantiated part set, RET8. At `B891CE` its
virtual `+10` calls `71AED0`, which allocates 8Ch and constructs through
`71ACD0..71AD44`. That constructor calls `B89F20`, then installs `CFD8E0`
at `71ACF5` and clears five derived vectors including `+80/+84/+88`.
`B89F20` initializes actual count to 1, stores source resource `+08`, and
increments source resource `+04`. `7135C0` stores the passed part set at
unit-part-instance `+160`; `87BEA4` publishes that instance at plane `+360`.

## Node factory and named-map publication

For each hierarchy record, `B8922A` defaults to local factory profile `D63218`.
`B89234..B89255` visits attached resource items, calling each virtual `+1C`;
the **last nonnull factory** replaces the selection at `B8924B`.
Afterward, hierarchy flags `+58 & 1` force `D63220` at `B89474..B8947E`.
`B894BB` calls the selected factory virtual `+04` with the hierarchy name;
`B894BD` stores the returned raw node in partSet's node vector `+14`.

| Factory profile | Create target | Concrete constructor/profile | Current terminal coverage at packet base |
| --- | --- | --- | --- |
| `D63210` | `B866C0` | `BSP_Node_Construct`, plain node | Separate plain-node provider; not the default branch here. |
| `D63218` (default) | `B86720` | `B743C0`, model-base `D62D78` | No `D62D78`/`B743C0` references in src/include; generated-model provider cannot substitute. |
| `D63220` (flag forced) | `B86780` | `BSP_Group_Construct`, `D634F8` | `NativeGroupReference` exists, current terminal `BD30E0 -> B8F8C0`. |

`B743C0` calls `BSP_Node_Construct` then installs `D62D78`.
`B74EC0` allocates from the native node pool. `D62D78` starts
`BD30E0, B74B60, B74370, B743E0`; virtual `+18` is `B6F310`.
`B74B60..B74B7F` is the deleting wrapper: calls `B6F440`, tests flags bit 0,
then returns the slot via `B6E490` with ECX=`109008C`, RET4.
In contrast, `src/native_model_owner.cpp` accepts `D62DE8` (plus the transient
node phase `D62C88`) and requires deleting target `B75290`.

During `B89640..B89661`, each hierarchy resource virtual `+18` receives
partSet, hierarchy record, created node, and selector. A Note resource has
profile `CFD860`: `+18=B86820`, `+1C=6F9D30` (exact `XOR EAX,EAX; RET`).
`B86820` forwards metadata and the actual node to partSet virtual `+08`.
`71B710..71B801` classifies metadata, tests the Note type `E19B54`, then at
`71B7E9..71B7F4` appends `{metadata ESI, node EBX}` to partSet `+7C` via
`71B170`. Its copy helper `716C10` copies two words without a retain.
The metadata name `belsocockpit` therefore **does not itself override the factory**.

## Bounded next action and remaining scope

Inspect the selected plane Mesh resource's hierarchy record whose attached Note
name is `belsocockpit`: capture hierarchy `+58` and each attached resource's
current virtual `+1C` result, or observe the actual plane `+808` vtable and its
`+00/+04` targets in an authorized runtime. This chooses among the existing
group provider, uncovered model-base terminal, and other explicit factories.
Then bind the selected actual node in the existing lifetime runtime and route
the setter's count transfer through that canonical companion. For `D62D78`,
recover a narrow model-base companion using `B6F440/B6E490`; do not reuse
generated geometry ownership or invent a deleting callback.

The opcode scan `08 08 00 00` returned all 129 matches with limit 150; the
plane-region matches were decoded. `7CFDBA` and `7D73DE` are the two checked
direct writers in the `7B..7D` plane region; the `7F` candidates are stack
displacements. The corresponding `0C 08 00 00` scan corroborated `+80C`.
This is not proof against indexed/bulk copies, aliases outside that region,
or runtime mutation. The original asset payload was not parsed in this packet.
The full part-set destructor and all override factories were not audited.
The field assignments/lookup establish borrowing, but do not independently
prove whole-plane/part-set teardown order. Helper `B3C650` supplies its own
retain/release, and the zero setter path releases its captured current object.

The completed evidence supports a precise producer frontier, not a claim that
the cockpit's shipped concrete class or gameplay lifetime has been validated.
