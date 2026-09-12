# The gun class family behind vtable 00CFE0A8

Addresses: `0072E510`, `0072D950`, `00730B80`, `006FDDA0`, `00730E80`, `006FDED0`, `006FDFC0`,
`00731C80`, `00731D50`, `00731E20`, `006FE390`, `006E0970`, `006EC860`.
Vtables `00CFE0A8`, `00CFE308`, `00CFBD20`, `00CFE548`, `00CFBF58`, `00CFC190`.
Factory table `00CE4570`-`00CE46F0`; script-facing name pool `00CE46EC`-`00CE4780`.

`docs/UNIT_WEAPON_DEVICES.md` left open which classes answer `vtable[5Ch](20h)` and which
siblings of `00CFE0A8` share its `Fire`. Both are settled here from the constructors.

## How a class is identified

Every constructor in the family writes a class id as a dword at **`+C4h`** (`param_1[0x31]` in
the pseudocode) after installing its vtable, and each derived constructor first calls its base
constructor, so the id at `+C4h` is the most-derived one. `vtable[5Ch](id)` is the class test that
reads it; `20h` is the base gun, so `vtable[5Ch](20h)` means "is a gun or any gun subclass", which
is what the device walk `008CF350` and the torpedo re-arm `0081F8B0` select on.

## The family

| Vtable | Constructor | Base constructor | Class id `+C4h` | Class name | Script-facing name |
| --- | --- | --- | --- | --- | --- |
| `00CFE0A8` | `0072E510` | `00728960` | `20h` | the base gun | - |
| `00CFE308` | `00730B80` | `0072E510` | `21h` | `MMultipleBombPlatform` | `MultiBombPlatform` |
| `00CFBD20` | `006FDDA0` | `0072E510` | `22h` | intermediate, no factory record | - |
| `00CFE548` | `00730E80` | `006FDDA0` | `23h` | `MRFSGun` | `Rapid_Fixed_Slave_Gun` |
| `00CFBF58` | `006FDED0` | `006FDDA0` | `24h` | `MRTGun` | `Rapid_Turning_Gun` |
| `00CFC190` | `006FDFC0` | `006FDED0` | `27h` | `MSTGun` | `Single_Turning_Gun` |

So the six vtables are one inheritance chain rooted at the base gun, which is why they all reuse
`00730160` `Fire` through `vtable[1D8h]`: none of them overrides it.

`00CFE0A8` has a second constructor at `0072D950` that installs the same vtable; it was not read
past the vtable write.

### Where the names come from

They are recovered strings, not hypotheses. `006FDDA0`, `00730B80`, `00730E80`, `006FDED0` and
`006FDFC0` have no callers other than each other; the creators that reach them are entries in a
node factory table of `34h`-byte records, each record starting with its class name inline and
ending with the creator pointer:

| Record name at | Name | Creator at | Creator | Reaches |
| --- | --- | --- | --- | --- |
| `00CE458C` | `MBombPlatform` | `00CE45C8` | `006E0970` | not read |
| `00CE45CC` | `MMultipleBombPlatform` | `00CE4608` | `00731C80` | `00730B80` at `00731CCE` |
| `00CE460C` | `MRFSGun` | `00CE463C` | `00731D50` | `00730E80` at `00731D9E` |
| `00CE4640` | `MRTGun` | `00CE4670` | `00731E20` | `006FDED0` at `00731E6E` |
| `00CE4674` | `MSTGun` | `00CE46A4` | `006FE390` | `006FDFC0` at `006FE3DE` |
| `00CE46A8` | `MDepthChargeLauncher` | `00CE46E8` | `006EC860` | `006EC640` -> `0072E510`, own vtable |
| `00CE46EC` | `MCatapult` | - | - | not read |

The pool immediately after the last record holds the script-facing spellings, in the reverse order
of the records: `Catapult`, `Depth_Charge_Launcher`, `MultiBombPlatform`, `BombPlatform`,
`Single_Turning_Gun`, `Rapid_Turning_Gun`, `Rapid_Fixed_Slave_Gun`. The three gun spellings expand
the three abbreviations exactly (ST = single turning, RT = rapid turning, RFS = rapid fixed slave),
which is the evidence for the pairing in the table above.

`00CFBD20` (`22h`) has no factory record, so nothing constructs it directly: it is the abstract
turning-gun base that `MRFSGun` and `MRTGun` share. It is the only class in the family that adds a
block of its own in its constructor (`+4BCh`..`+4C8h` zeroed, `+4CCh` = FFFFFFFFh, a byte at
`+490h` zeroed), and `MSTGun` adds one more byte at `+4D8h`.

## What this does not say

- The class ids are not the weapon type ids. The sub-type values `2`, `3`, `4`, `5` and `7` that
  `00730160` `Fire` selects on come from the weapon descriptor's `+80h` (`7` is the torpedo, from
  `0081F8B0`), a different field from `+C4h`. No mapping between the two was read.
- Anti-aircraft, artillery and torpedo tube are **not** class names in this family. The family
  splits by mount kind (fixed slave, rapid turning, single turning) and by platform (bomb
  platform, depth charge launcher, catapult); the weapon's role comes from its descriptor.
- `00728960`, the base of `0072E510`, is unread, so the class id `20h` is attributed to
  `0072E510` (`00CFE0A8`), which is where the write was seen.
- `MDepthChargeLauncher` and `MCatapult` are in the same factory table and reach `0072E510`, but
  their own vtables and ids were not read: **contract: unread**.
