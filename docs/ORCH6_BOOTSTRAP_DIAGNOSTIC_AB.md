# AB startup closeout diagnostics

This is source startup diagnostics, with no recovered native addresses or ABI claim.
The AB pointer-vector implementation was integrated at `193f5bf5f36402f039b6d0e848b950ee3b2e676d`.
Its Win32 build, two existing CTests, original-byte fixtures and production-object
comparison passed. The 120-frame USN01 compatibility launch exited 2 before
opening its normal log, in the newly merged native-data child bootstrap.

`GameNativeDataBootstrapChild::reserve_and_resume` now retains the failing
`VirtualAllocEx` error and queries the requested address before rolling back its
own reservations. The error includes allocation/region information, process IDs,
the suspended thread's initial stack pointer and ASLR policy flags. It still
rejects the collision, releases only its own reservations, and terminates only
its unconfirmed child. It does not change linker placement, mitigation policy,
memory ownership or retry behavior.

Recorded console observations include `ERROR_INVALID_ADDRESS` (487) at
`00CF0000`, inside a private reservation starting at `00C00000`; the child's
initial ESP was `00CFFFCC`. Another build encountered a committed private region
at `00D10000`. A separate controlled suspended-child probe observed the actual
rebuilt image mapped at `00D50000`, overlapping both the D5 and D6 required bands.
Other probe launches found all four bands free and successfully reserved them.
The probes used the production bootstrap and terminated only their own
unconfirmed children. They do not constitute a game runtime pass.

These observations establish real address-space collisions, including the
initial stack and the rebuilt image. They do not establish a durable placement
fix. ASLR was enabled in the queried parent and child. Disabling ASLR, moving the
image, moving a live stack, releasing foreign allocations or retrying until a
launch happens to succeed was not used as validation.

The final integrated build at `f6d65fd8e450039d575b1bff7379b17a95e743b1`
subsequently completed its normal closeout run: USN01 ran 120 mission frames,
returned 0 in 10.122 seconds, and produced 18,557 finite trajectory rows. The
existing world registration, observer teardown and pending-queue checks passed.
The final executable SHA-256 is
`d6aadec418c7086f4c0446e1ccfbbdb4e6d1473823385645671efbb12c94b3e1`.

That one successful run does not repair or invalidate the earlier collisions.
The batch is closed with a compatibility pass for that exact build and an open
startup reliability limitation. The owner of the shared native-data entry path
still needs to establish a child memory placement strategy. This is recorded
as follow-up evidence; no worker or new reconstruction packet was dispatched.
See `reports/orch6_bootstrap_diagnostic_ab.json` for the exact observations and
`reports/native_unit_part_vector.json` for the reconstruction's separate proof.
