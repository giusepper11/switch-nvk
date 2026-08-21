# FG-2 compute-root address-reuse finding — 2026-08-21

## Finding

The valid same-source control reproduced the established failure exactly: iteration 1 passed, then
iterations 2-64 returned `0xfab61a38` with checksum `0xc17a35a5`. It proved 0/63 root transitions,
63/63 QMD transitions, exact root/QMD copies, complete root-field attribution, 64/64 direct-`PCAS`
correlation, and no reported GPU fault.

The fresh-root variant alternated selected root GPU VAs `0xc7f40000` and `0xc7f40a00` for 63/63
adjacent transitions while retaining identical QMD-address alternation. Iterations 1 and 2 passed
exactly. Iterations 3-64 then returned iteration-2/seed-42 output `0xf5031a17` with checksum
`0x0daf4ac5`. All root and QMD copies, decoded addresses, outside-root-field comparisons, dispatch
correlations, ordering, and fault prerequisites passed.

## Consequence

Outcome: `behavior_changed_unresolved`.

The tested adjacent two-slot compute-root address intervention changed behavior but was insufficient
for the complete image-chain oracle. It is not a production fix and does not prove generic cache,
constant-buffer, allocator, address-translation, launch-state, shader-execution, sampled-image, or
GPU-consumption behavior. The immediate unresolved interval is now why the fresh-root run consumed
the second dispatch's root state but retained it across subsequent alternating addresses. Any next
experiment requires a smaller separate OpenSpec change; no additional intervention is combined here.

FG-2 remains `BLOCKED`; FG-3 is unchanged. Evidence is retained in
`docs/testing/FG2_ROOT_ADDRESS_REUSE_RUN_2026-08-21.md` and the paired immutable raw streams under
`docs/testing/raw/`.
