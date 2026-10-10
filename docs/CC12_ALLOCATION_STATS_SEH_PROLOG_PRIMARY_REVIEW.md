# Allocation-stats SEH prolog primary review

Root accepts the ordinary coordinate gate from worker commit
`1088be0415ae3103e0216cf217c1ff9e4346a279`. The saved complete 69-byte body
`[00c07c00,00c07c45)` is `__SEH_prolog4`; its library name is preserved.
Root independently replayed 423 pins, 92 full Git/current inputs, all 228 ZIP
payloads, and every authorized original-image byte and 21 decoded instructions.
All 22 fresh typed replies are at modification 32. The metadata's undefined
void prototype does not recover this helper's actual ABI.

With U denoting `00c069a2` entry ESP, its retained two pushes and call enter the
helper at U-0Ch. The actual prolog sets EBP=U-4, preserves the return operand,
and returns to `00c069ae` with ESP=U-34h. Root independently recomputed these
addresses from the actual stack effects. The post-prolog offsets +8, +0Ch,
+10h and +14h therefore name incoming arguments one through four. EBX loaded
at +8 is the first incoming argument: the same raw second argument forwarded
through `00bf6b43` and `00c07991`. Combined with the accepted frame helper, the
action EBP is that forwarded raw argument plus twelve bytes.

The prolog's own FS registration is U-14h, distinct from the forwarded raw
argument. It installs handler immediate `00c07c90`, encodes the scope word
with the unread security-cookie DWORD at `00e15590`, and sets state -2.
Those targets/data and the excluded unwind-body gap were not opened. The
association of the forwarded raw argument with the parent's OS registration,
fault behavior and the startup action's physical epilogue remain unresolved.

No C++, build, Native execution, ABI, startup or gameplay credit is added.
No Ghidra name, comment, listing, flow or project mutation occurred. The full
worker gate and independent replay are retained in the accompanying reports.
