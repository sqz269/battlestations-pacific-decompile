# Platform text-input delivery

Read-only audit on 2026-09-09. Live batches verified `bsp`, program
`/battlestationspacific.exe`. This follows STARTUP_WINDOW_HANDOFF and the queue
storage reconstruction owned by the primary agent. No Ghidra mutations or C++
changes were made here.

## Enable, clear, and owner

00a965a0 takes platform in ECX and enabled byte in the first stack argument,
returning with RET4. It always writes platform+170h, resets the sentinel's next
and previous links to itself, writes count+17Ch=0, and frees every old node.
It retains the sentinel. This happens even if enabled equals its prior value;
an equality guard would incorrectly preserve queued events.

The saved pseudocode stops after the first free. Raw continuation00a965db adds4
to ESP, compares saved next node against sentinel+178h, moves next into EAX,
and branches back to00a965d3 until done. It then restores registers and returns.

Direct caller00a966e0 takes a text-input owner in ECX and two stack arguments,
RET8. It writes the supplied enabled byte to owner+4 and the second argument
to owner+24h, calls00a965a0 on singleton0109cf04, then calls helper004bec00 and
00a92c40 with float zero. When enabling, it clears its string at+18h and resets
cursor+20h. Owner+24h's purpose and these UI update dependencies are not recovered
here, so this is not yet a complete text-field activation port.

Enable and pop are direct calls in the established path, not platform virtual
slots. The virtual boundary is the text owner callback after each pop.

## Pop and erase

00bece90 takes platform in ECX and two byte-output pointers on the stack, RET8.
It fetches the first node from sentinel+178h; if first equals sentinel it calls
00bf6713 (invalid-parameter path). It writes node+8 to the first output and node+9
to the second, then erases the node through00bec990. Thus outputs are written
before freeing the node. There is no successful empty pop in the native route;
a typed `bool pop(event)` may reject empty input as an explicit host difference.

00bec990 receives list in ECX and three stack arguments: hidden output-iterator
pointer, iterator owner, and node. It checks nonnull iterator owner and rejects
that owner's sentinel via00bf6713. For a node other than the destination list's
sentinel it saves next, updates previous.next and next.previous, frees the node,
then decrements list+8. Crucial omitted export continuation:00bec9d4 ADD ESP,4;
00bec9d7 ADD DWORD PTR[EDI+8],-1. It returns iterator `{owner,next}` through the
hidden pointer and EAX, RET0Ch. Reconstructing the successful FIFO operation
does not establish the native checked-iterator or invalid-parameter ABI.

## Consumer dispatch and event meaning

00a96f40 takes the text owner in ECX and returns with RET4 despite not reading
its stack argument. That argument's role is not proven. It snapshots platform
singleton0109cf04 into EDI, then repeats while owner+4 is enabled and platform
count+17Ch is nonzero:

1. Pop one event through00bece90, removing it before invoking user/UI code.
2. Call text-owner virtual slot0, passing event first byte then second byte as
   two stack arguments with owner in ECX. AL nonzero means consumed.
3. If AL is zero, call fallback00a96750 with the same event and owner.
4. Recheck owner+4, then the live queue count, before another iteration.

The pop writes only one byte to each local stack slot; the consumer pushes the
full DWORD slots. Only low bytes are established event data. A typed byte-based
interface should not claim that uninitialized upper stack bits are recovered.
Callbacks can disable processing and the next iteration must respect that.

Fallback00a96750 confirms the second byte is an event category, not the high byte
of a Unicode character. Category0 treats CR as virtual+8, Escape as virtual+Ch,
Backspace as deletion before cursor, and accepted ordinary characters as text
insertion/overwrite. Category1 handles navigation/edit virtual-key values, such
as2Eh for Delete. Full string allocation, selection/edit policy and notification
callbacks remain unported; no text-editor equivalence is claimed.

## Window-message policy

The relevant branches of00bed3b0 use the explicit platform stack argument and
gate on byte+170h. They append to list+174h only when enabled:

| Message | Accepted WPARAM | Event bytes / side effect |
| --- | --- | --- |
| WM_CHAR102h | Any value | `{low8(WPARAM),0}`; exact WPARAM16h also sets platform+44h=1 |
| WM_KEYDOWN100h | 26h,28h,25h,27h,24h,09h,23h,2Eh,2Dh,14h,21h,22h | `{low8(WPARAM),1}` |

The accepted keys are arrows, Home, Tab, End, Delete, Insert, Caps Lock, Page Up,
and Page Down. Comparisons use full WPARAM before truncating into the event byte.
Other keydowns do not append through this branch. WM_SYSKEYDOWN is not covered
by this policy. WM_CHAR16h marks a request flag but is also enqueued; clipboard
content retrieval and the flag's later consumer are not established here.

Both appended and ignored paths ultimately call DefWindowProcA and return its
result. Appending does not consume the Windows message. Queue code alone must
not replace native activation, sizing, audio or renderer branches in the full
window procedure.

## Exact evidence

All inclusive ranges below were compared byte-for-byte against the installed
executable and live saved program. The last two entries are policy fragments,
not complete functions.

| Range | SHA256 |
| --- | --- |
| 00bece90..00beced8 | ff4d3a6cb7ff97878e4a7d353c9c8d50c627e3f99f4ec759c1197fa9e18236dd |
| 00bec990..00bec9ed | 4a344819c82043a401482308c2d26aacb89324aa5d9c2eb8fbc6b9b516f37233 |
| 00a965a0..00a965ec | fd6a36d699013af7e9ea6124b5090a0f6208e7bb1d4ab6d9f985b589c186b2af |
| 00a96f40..00a96fa1 | 1d09208703a01b420eb86be32a91ea0ceb026ced42493aa9c13c2d03bd16f1d1 |
| 00bed607..00bed67d | 4c40352bd6558603f45350f1fed9b9163a60ebb38e2b8e4443ff1722c59a51f9 |
| 00bed6db..00bed6fe | 664be6441843082b6c00369ca8d5559d2cfa63ab454cedac6243b783b44e89a9 |

This audit required no build or runtime checks. A usable next interface is
enable-and-clear, message-to-event policy and FIFO pop, preserving the callback
ordering above. Integration with an actual native text owner remains separate
from storage correctness and a diagnostic Windows message-queue check.
