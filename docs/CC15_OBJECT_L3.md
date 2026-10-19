# cc15 object bring-up, lane L3: loading queue, application, VFS pump

Lane cc15-ob3 covers ranks 10, 12 and 13 of the ranking in `docs/CC15_WIRE_FRONTIER.md`. All three objects sit
in `game_hosts.cpp`. Each one has its own switch, and every switch is OFF.

## Native loading queue (rank 10), `kLoadingQueueCc15Bound`

**Evidence.**
- 004FDE20 `BSP_LoadingQueue_GetInstance` is a lazy singleton with publication cell E18D4C. It allocates 20h and
  constructs through 004FDBA0. Construction stores table CEB198, a manual-reset event at +0C (00BD1970), and
  zeroes at +04/+08/+10..+1C. The getter then registers the owner through 00BD0C30. All of this is already in
  `src/native_loading_queue_owner.cpp`.
- 00737A50 RunFrame calls 004FDE20 and then 00509190 on the result, every frame.
- 00509190 Update returns at once when `[ECX+14h]` (the job count) is 0.
- 0050ACE0 is the CEB198 scalar deleting destructor and calls 00509FD0. That destructor:
  - restamps CEB198;
  - calls 005092E0, which runs a trivial 004254B0 diagnostic and returns when the count is 0;
  - calls 004FB4B0(+10, 0) and frees +10 through 00BF6989;
  - when +0C is set, calls slot 0 of the event with 1 (BD19B0), then clears +0C;
  - clears E18D4C and stamps CE3818.

  0050ACE0 then frees the owner when flags bit 0 is set (00BF65AC).

**Lifetime as wired.** `GameFrameHost::update_loading_queue` (the 00737A50 slot) calls
`get_native_loading_queue_owner_004fde20` with the application manager cell 01090AA0 and an E18D4C cell owned by
`GameSingletonHost`, then calls `update_native_loading_queue_00509190`. The first frame builds and registers the
queue. The singleton manager drain (00BD0400) deletes it through the new CEB198 profile, which borrows the same
E18D4C cell through `NativeSingletonDeletionBindings::actual_loading_queue_publication_00e18d4c`. That field is
appended at +208, and the size becomes 212.

**ABI.** The new Source interfaces take explicit owner, flags and cell arguments. They are not ABI-compatible with
the native ECX/RET4 forms.

**Scope.** Only the empty queue is admitted. A non-empty queue throws a source contract error in 00509190 and in
the drain. The native bindings that path needs do not exist:
- the FileStore request;
- 00506BF0 retirement inside the update;
- 00501620 work start;
- the 005092E0 loop, which sleeps 100 ms, then pumps 00BDB0B0 and 00509190.

The host never enqueues into the native queue, so in a mission the queue stays empty.

**Rows wired.** `ApplicationFrameHost::update_loading_queue 004fde20` goes from UNIMPLEMENTED to concrete (5000
calls). Of the frontier's 3 "unlocks", the 00509190 update is wired. The enqueue paths (005055C0/00504D20 from the
frontend) remain host-interface bodies.

**Pairs (USN04, 5000 frames / 4800 mission frames).**
- OFF (6e34d1010 tree build) vs baseline (a84b1ddac export): `pair_diff` exit 0. The only noise is pretranslate.
- ON (`pair_export --flip kLoadingQueueCc15Bound=true`) vs OFF: exit 1, gameplay identical. Death rows, plane death
  modes, the 90-row unit table and summary lines are all identical. The moved rows are:
  - `update_loading_queue` UNIMPLEMENTED -> concrete, so the counts go 1075/474 -> 1076/473;
  - `singleton lifetime drained: registered_slots` 43 -> 44, which is the queue's registration. It drained
    through CEB198 without error.

**Uncertainties.**
- In the image, the first 004FDE20 call may come earlier than the first RunFrame. For example, a frontend enqueue
  or the 005092E0 stop on menu screens could call it first. That would change the queue's registration slot, and
  therefore its drain order. Within this host, RunFrame is the first caller.
- The event is deleted through `delete_native_event_owner_00bd19b0` directly, not through slot 0 of its table.
