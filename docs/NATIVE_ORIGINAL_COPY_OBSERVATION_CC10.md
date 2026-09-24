# Fresh retained-copy observation

A read-only inventory hashed all67814 files and67819 streams in the retained
independent game copy:57803339970 bytes over267.73 seconds. All67813 non-log
records match the immutable baseline in content, stream names/hashes, size,
attributes, creation/write timestamps and file identity. No mismatch, new or
missing file, hardlink or reparse entry was observed. Twenty-eight process
snapshots found no original game process.

The log has exactly its recorded post-run02 state: one empty unnamed stream,
attributes32, creation1748812086735887100ns, write1790248352714395600ns and file
ID1407374893507724. Its native volume DWORD is314419654, index high327680 and
low9954444. Native/Python volume representations are kept separate. No cause of
the earlier log timestamp change is inferred.

Primary review covered the complete observer source and independently compared
the full recorded manifest to the frozen baseline and exact run02 log state.
It also rechecked the current log's metadata, identity and hash, verified the
seven indexed evidence files and1043 prior rows, and checked the proposed
overlay's provenance. The primary did not repeat the full57GB disk hash pass.

The observation is sequential, not an atomic filesystem snapshot or future-state
guarantee. Process samples do not exclude a process between samples. Reads may
affect access timestamps; no access-time or security-descriptor equivalence is
claimed. Future preparation must recheck current copy state and fail on any
unexpected difference.

The frozen overlay remains an unadmitted proposal. Its exact row and provenance
were accepted for a separate controller revision and a prepare-only check. This
packet invoked no helper or original, changed no profile/copied file/old overlay,
and consumed no admission. Original attempts remain2. It provides no original
sampler or gameplay evidence.

Receipt: `reports/native_original_copy_observation_cc10.json`.
Archive: `local/cc10-platform-evidence/original-copy-fresh-observation.zip`.
