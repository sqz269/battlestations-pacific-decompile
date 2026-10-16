# Current traffic-layer receiver correction

The existing typed host projection for `0049F260` tested `[outer+C]` but forwarded the outer object to `0049E560`. Native instead loads the current inner pointer into ECX before the test and uses that same inner pointer for the conditional tail jump.

The complete 13-byte Native body is five instructions: `MOV ECX,[ECX+C]`, `TEST ECX,ECX`, `JZ 0049F26C`, `JMP 0049E560`, `RET`. Original input is the outer ECX pointer; no stack or semantic EAX result is consumed. The typed Source now reads the inner DWORD once, converts it through the existing Win32 object helper, and forwards that captured receiver only when nonnull. No outer null guard, reread, fallback receiver or new callback was introduced.

This corrects the existing `Left8Host` projection. It does not provide a new raw entry, a concrete `0049E560` implementation, Native binary ABI, full map writer, production wiring or game validation. The old typed interface remains unchanged. The upstream `004C71C0` helper's current `[game+21D0]` load and this subsequent `+C` load remain distinct operations.

The finding arose while auditing the whole `004D5BD0` map/bounds writer. Its other concrete dependencies remain separate readiness gates; this small repair does not substitute for that function. The registered repository build and existing checks cover the changed Source. No new test case was added for this direct receiver-forwarding correction; complete assembly and focused Source inspection provide the behavioral evidence.
