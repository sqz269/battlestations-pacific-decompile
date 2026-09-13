# AY renderer startup, BeginFrame, statistics and preparation integration

Addresses: 00b0cc10, 00b0ccb0, 00b0cce0, 00b2b200, 00b15090, 00b14480, 00b13630, 00b13280, 00b13180, 00b10740, 00b2aeb0, 00b49940, 00b49950, 004c11f0, 00b51df0, 00b1bf70, 0041dd20, 0041dd40

Sixteen complete normal bodies across four modules now compose with actual
storage, current profiles and existing concrete providers. Two existing string
bodies gain raw-pool overloads, allowing BeginFrame cleanup to observe getter
exceptions. Neither those extensions nor three EH metadata definitions add
to the sixteen-body count.

Reviewed source `7ba130cf922cbd996eae5b5961f3847a0346ba57` was merged with main.
Exact combined source `71ff3f1220fecd9e4e9ff7ad7ffc54b3a38477f4` passed the strict Win32 build, both
existing CTests and four probes linked only to the three current libraries.
Sixteen original signatures/full bodies and two existing string signatures
were saved and verified; all 64 numeric call rows pass.

- statistics: One untouched-original-byte fixture covers both nonzero forty-DWORD banks, fifteen trailing constructor fields, eight canary words and receiver return. Store order was also reviewed in compiled instructions.
- begin: 20 original/source pairs,167745 normalized state bytes and1829 event DWORDs. Full BeginFrame/record operations, actual raw pool/manager, nonempty clear/middle erase, scalar overlap, returning CRT validation, second-getter throw with first-header cleanup and first-getter throw after cleanup disarm.
- startup: 9 original/source pairs,71424 normalized state bytes and8044 event DWORDs. Real hidden HAL startup, physical pools and default surfaces, current COM outputs, x87 unsigned/NaN/infinity inputs. Final pending1 case executes one real Reset and three Sleeps per original/source run. Constructor exceptions and nonzero gamma generation are not covered.
- preparation: 11 original/source pairs,775 compared DWORDs. Actual manager/registration, publication retained and guard unlocked on native FH3 registration failure, returning validation changes publication, disabled and enabled modes with overwritten argument slots, full raw key/sort and current scheduler dispatch for both established profiles.

Full hashes, preserved worker captures, relocation/fixture evidence and
remaining limits are in `reports/native_renderer_startup_ay_validation.json`.
Actual system-constant/material dependencies, complete startup/render submission,
general ABI/EH/concurrency and gameplay remain open.
