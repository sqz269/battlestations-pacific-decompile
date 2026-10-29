# Independent raw frontend teardown review

**PASS for the complete normal parent schedule, with required suppliers still open.** No blocking mismatch was found in `0481897503d8380d2ea3a14c345cc5ca9c65e77e`, frozen on `e64249b004a989f162a1b7b9cda3493606535e88`. Both complete source files, the 49-line author document, report and retained evidence were reviewed. This packet owns only this document and its review report; it changes no production code, caller, cell, guard, ledger or Ghidra annotation.

## Native input and schedule

Independent retail-PE decoding, live bytes and all saved listing addresses agree on **107 bytes /38 instructions**, 004DA650..004DA6BA. The original `PUSH ESI; XOR ESI,ESI; PUSH ESI; CALL4D95F0` leaves incoming ECX unchanged and passes a stacked zero. The pause child captures ECX into ESI at 004D960D and reads actual Game `+61F/+620/+1FE4`; RET4 at 004D9826 consumes that zero. The child's 569-byte/140-instruction body was counted, with its relevant receiver prefix and return independently inspected. It was not reconstructed or admitted as a provider here.

Thus the original no-input annotation misses a genuine Game input. The new `NativeGameStorage&` parameter uses the existing actual `71A0h` storage type, also passed by raw OnDestroy's required `call_004da650(game)` at 004DC5E9. No projected state, fabricated class, storage reinterpretation or EAX-result contract is introduced. This is a new C++ interface; original ECX/ESI/stack and exception transport are separate qualifications.

The pure `call_004d95f0(actual_game,0)` is the first parent action. It requires the whole raw pause helper. The four subsequent rows match the native order and epochs:

| Actual cell | Capture | Indirect scalar transfer | Clear after normal return |
| --- | --- | --- | --- |
| E198AC | 4DA659 | 4DA669 | 4DA66B |
| E198B4 | 4DA671 | 4DA681 | 4DA683 |
| E198B8 | 4DA689 | 4DA699 | 4DA69B |
| E198C4 | 4DA6A1 | 4DA6B1 | 4DA6B3 |

Each volatile cell is read once for the captured receiver. Null skips both call and clear. Nonnull passes that capture, payload byte slot `0` and flags `1` to the same real lifetime service. Only after normal return is the same **current cell** cleared, including any replacement published by the callback. No captured-receiver dereference follows deletion, pointer-equality repair, replacement loop, early four-cell snapshot or extra null clear is added. Later rows reload after earlier callbacks.

The new parent adds no catch or operation protocol. A pause C++ exception precedes all parent manager accesses; a scalar exception skips its row's clear and later rows while retaining reached effects. Stable context/cell/provider backing and serialized parent custody remain preconditions. This source behavior is not proof of native FH/SEH, hardware-fault, reentrant or concurrent behavior.

## Real service route and compiled code

Fresh strict Win32 parent code has five calls and four post-return clears. The first C++ service call passes actual Game and zero before any manager capture. Each scalar sequence pushes flags1, slot0 and the capture, then clears only after the call. Four null branches skip both call and clear.

The compiler adjusts the `NativeGameLifetimeCalls` service receiver by `+4` and calls its **host C++ vtable byte slot14h**. That service ABI is distinct from the explicit **payload byte-slot0 argument**. Fresh compilation of the real existing provider source confirms `NativeGameCurrentVirtualCalls::virtual_scalar` is an 8-byte bridge to the qualified physics dispatcher, with its own service `this+18h` adjustment. The 22-byte physics method loads the captured object into ECX, reads its current table and requested byte slot, pushes the flags, calls that method and returns from its three-argument C++ frame with RET0Ch. The payload target must implement the genuine one-argument thiscall/deleting contract, including its callee cleanup and matching allocation domain.

No new provider or successful stub is supplied by the parent. The concrete service's existence does not authenticate arbitrary service objects, copied native vtable identities or custom heaps. Root must retain the same actual lifetime `calls` instance and genuine callable receiver tables through every reached callback.

## Evidence and limits

- All **18** source/dependency/export/PE/site/object/listing/audit hash pins pass. Four additional real provider files and three inspected semantic-host files equal the frozen parent.
- Fresh MSVC19.51 Win32 compilation of the new parent and the two existing provider source files passes `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`, without warnings or executable creation.
- Both parent-object function records reproduce retained machine bytes and relocation offset/type/target; only anonymous namespace hashes are normalized. The public entry is146 bytes and its internal helper36 bytes. Public code SHA256 is `e6927eeebd341933b78e7880b819f1c7be6a2f7027e1b03846715e4acaf78fbb`.
- Fresh original-call verification checks the **one direct row**, 4DA654→4D95F0, with zero failures. All four indirect transfer sites and their capture/table/flag/clear instructions are independently checked against the original PE/live body; their runtime target identities remain unclaimed.

The old semantic frontend API is unchanged. Its reset-interface-stack host remains explicitly unimplemented, its projected `GamePauseState` helper uses separate fields/host/vector storage, and its in-mission projection only clears the pointer. Those paths do not close the whole raw pause or mission-deletion suppliers and are not promoted by this new source.

Accept this bounded parent reconstruction with its explicit required inputs. Whole raw pause, actual canonical cell producers, Game lifecycle, same service owner, callable child tables, scalar bodies/heaps and raw OnDestroy/root attachment remain required. No native exception/frame ABI, runtime or gameplay proof follows from compilation. No full build, test, fixture, probe, game or DLL call was run; no Ghidra mutation or duplicate native/vendor-body/activation credit was added.

[Machine-readable review](../reports/cc16_native_front_end_manager_destruction_review.json).
