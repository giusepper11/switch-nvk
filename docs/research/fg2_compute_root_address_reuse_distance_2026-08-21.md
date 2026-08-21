# FG-2 compute-root address-reuse-distance finding — 2026-08-21

## Finding

The immutable same-source control and variant each produced 64 causally valid records on real Tegra.
The control selected A/B roots and first revisited a root at record 3 with distance 2. The variant
selected A/B/C roots and first revisited a root at record 4 with distance 3. Both used the same
X/Y/Z/W QMD ring, whose first revisit was record 5 with distance 4. Both proved one command-upload
backing, the same 7,168-byte footprint, complete root and QMD copies, generated-layout root decode,
outside-root-field equality, mapped-QMD/direct-PCAS equality, ordering, ownership through completion,
complete teardown, and no reported GPU fault. CPU-mapped equality remains `UNPROVEN` for GPU
visibility.

Both arms consumed seeds 5 and 42 exactly. At record 3, the variant selected previously unused root C
and QMD Z but still returned seed-42 pixel `0xf5031a17` and checksum `0x0daf4ac5`, rather than seed-79
pixel `0xf0bf610f` and checksum `0x1fcdf2e5`. Records 3-64 in both arms retained that same seed-42
signature. Increasing the tested root-address reuse distance therefore did not shift stale-state
onset to the variant's first root revisit.

## Consequence

Outcome: `specific_root_address_reuse_distance_change_insufficient`.

This rejects only the tested A/B/C root-ring intervention as sufficient. It does not establish a
root-state key, lifetime, VA-versus-physical identity, replacement policy, allocator rule, generic
GM20B cache defect, or generic NVK defect. At most one later OpenSpec change may isolate compute-root
backing identity as the next discriminator; no backing intervention is designed or implemented here.
FG-2 remains `BLOCKED`, and FG-3/FG-4 are unchanged. The experiment selectors remain opt-in and
disabled in ordinary execution.

Complete provenance, the compact and full 64-record paired tables, and fault review are retained in
`docs/testing/FG2_ROOT_ADDRESS_REUSE_DISTANCE_RUN_2026-08-21.md`. The immutable raw streams are under
`docs/testing/raw/` with SHA256 values
`9a876f4ddcf0518f9dee4d2dba796d5a064bf07f63f5e79cdf72ffc9f6f4654a` (control) and
`b1f2ac8236bf95ea5318802204856743bbbd41d73a028dd4c3090dabdc7875d1` (variant).
