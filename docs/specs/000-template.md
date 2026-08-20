# Spec 000 — Template

**Status:** DRAFT  
**Milestone:** `<ID>`  
**Owner:** `<name/agent>`  
**Last updated:** `<YYYY-MM-DD>`

## 1. Problem

What exact capability or unknown does this spec address?

## 2. Current evidence

List only evidence relevant to this layer. Label each item with the project evidence vocabulary (`PROVEN_HW`, `PROVEN_HOST`, `IMPLEMENTED_UNPROVEN`, `HYPOTHESIS`, `BLOCKED`, `REJECTED`).

Do not use success in an adjacent layer as evidence unless the dependency is explicit.

## 3. Hypothesis

A falsifiable statement, for example:

> If the winsys returns from `EXEC` after associating the submitted `NvFence` with the signaled Vulkan sync object, then two submissions can remain in flight while existing fence waits still preserve correctness.

## 4. Scope

What this change will implement/test.

## 5. Non-goals

What this change deliberately does not attempt.

## 6. Proposed design

Describe the smallest design needed for this experiment.

For low-level resources include:

- allocation owner;
- mapping owner;
- wrapper/view owner;
- synchronization owner;
- destruction/lifetime rules.

For scheduling include the dependency sequence explicitly.

## 7. Files/components expected to change

List expected areas. This is a guide, not permission to force the design into the initial list.

## 8. Compatibility/fallback

How does the existing correctness path remain available, if practical?

How can a run prove which path was actually used?

## 9. Validation plan

### Host/build

- compile/link checks;
- unit/static checks;
- deterministic host-side tests where meaningful.

### Real hardware

Specify:

- NRO/test artifact;
- exact observable result;
- required logs/counters;
- GPU error checks;
- minimum repetition/duration;
- configuration/resolution/buffer count when relevant.

## 10. Acceptance criteria

Use objective pass/fail statements.

Example:

- [ ] two submits return without waiting for GPU completion;
- [ ] dependent work still produces the expected checksum;
- [ ] no GPU error notifier after 1,000 iterations;
- [ ] legacy smoke tests remain green;
- [ ] full hardware evidence record is retained.

## 11. Failure modes and false positives

How could the test appear to pass without proving the hypothesis?

What corruption/deadlock/lifetime risks exist?

## 12. Observability

What instrumentation is required, and how will measurement overhead be controlled?

## 13. Risks

Technical, performance, compatibility, reproducibility, and provenance/licensing risks.

## 14. Open questions

Questions that can remain unanswered before implementation.

## 15. Result

Fill after testing:

- final status;
- hardware/host evidence references;
- surprises;
- rejected alternatives;
- follow-up spec/milestone.
