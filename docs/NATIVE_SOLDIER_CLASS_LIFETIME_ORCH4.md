# Native SoldierClass lifetime

This packet supplies the complete128-byte destructor004B1120 and30-byte scalar
deleting destructor004B1310. Original interfaces are ECX receiver/RET and ECX
receiver, stack flags, EAX original receiver/RET4 respectively. Names are
descriptive hypotheses. Source uses new C++ interfaces, not binary replacements.

The destructor first writes actual derived table identityCE7150. It captures
the sentinel at class+2C and its minimum, then erases the entire actual string/
float tree at class+28 through completed444B10. It frees the **current** sentinel,
clears the head and count, and invokes genuine489E80 base cleanup. That base
releases current resource+10, effect handle+14 and raw name+4 in native order.

HandlerC649D8 selects FuncInfoD8CDD0 with one unwind state and mapD8CDC8.
State0 invokesC649D0, which loads the saved receiver fromEBP-18 and tail-calls
489E80. Thus a failing tree drain still cleans the base, but does not invent
tree rollback or free the sentinel afterward. A second exception during true
unwind terminates. Native state becomes-1 before the ordinary base call, so a
failure of that call never repeats base destruction.

The scalar deleting destructor completes the full destructor before testing
flag bit0 and freeing owner storage. It returns the captured original pointer;
destructor failure prevents that free.

Both complete spans and the exception metadata match live Ghidra and the
installed PE bytes. Exact call rows and prior names/comments are recorded in
`reports/native_soldier_class_lifetime_orch4.json`. The strict MSVC Win32 build
and all three existing CTests passed. This packet has no direct native
destructor differential fixture; neither ABI/FH3/SEH nor gameplay is validated.
The factory4B1400 and reader4B0DE0 remain source-absent until their genuine
effect/resource/Lua dependency closure is complete.
