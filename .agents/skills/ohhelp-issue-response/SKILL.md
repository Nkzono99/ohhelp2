---
name: ohhelp-issue-response
description: Use when handling GitHub issues or issue comments for this OhHelp v2 repository. Guides Codex to evaluate the long-term v2 design fit before implementing, instead of blindly following issue text or making short-term patches.
---

# OhHelp Issue Response

Use this skill for issue triage, issue-driven fixes, and issue comments in this
repository.

## Operating Principle

Treat an issue as evidence, not a specification. First identify the underlying
failure mode, user need, and v2 design implication. Implement the issue's
literal request only when it matches the long-term direction in `AGENTS.md`.

## Triage Workflow

1. Read the issue, latest comments, and any linked logs or repro code.
2. Rephrase the real problem in repository terms: API contract, MPI behavior,
   particle layout, accounting, documentation, release process, or migration.
3. Check the change against v2 direction:
   - prefer explicit contexts over library-wide mutable globals,
   - prefer particle adapters over fixed `S_particle` / `oh_particle` layout,
   - keep weighted load as the primary balancing model,
   - keep C and Fortran usable without letting v1 signatures dominate v2,
   - keep Level 4 out of the v2.0 critical path unless explicitly requested.
4. Decide whether to accept, reframe, defer, or reject the requested behavior.
   State the decision in code comments, docs, or issue comments when it affects
   public API or migration expectations.
5. Implement the smallest compilable migration step that improves the design.
   Avoid broad compatibility shims that create new global state or hidden
   layout assumptions.
6. Add or update focused tests and docs before closing the loop.

## Design Checks Before Editing

- If the issue asks for compatibility, ask whether the compatibility belongs in
  a shim, an adapter option, documentation, or should be intentionally broken in
  v2.
- If the issue touches Fortran indexing, distinguish internal canonical indices
  from external Fortran layout conventions. Prefer adapter-level contracts over
  context-wide switches.
- If the issue touches particle movement, inspect ownership, lifetime, species,
  region id, `totalp`, `pbase`, injection accounting, and Level 3/secondary
  behavior together.
- If the issue touches MPI datatypes or buffers, verify datatype extent,
  ownership flags, and borrowed vs owned pointer lifetime.
- If the issue touches balancing, identify whether the path is count-based,
  weight-based, or a documented temporary compatibility path.

## Guardrails

Do not introduce new direct `nid`, `spec`, or coordinate field access outside
the documented migration boundary. If unavoidable, update
`doc/v2/design/particle-adapter.md` and `tests/test_particle_contract_audit.sh`
in the same change.

Do not hide Fortran behavior inside unrelated context globals when an adapter,
wrapper, or explicit option can express it more locally.

Do not close an issue only because local tests pass when the reporter's case
needs confirmation. Comment with the commit, validation run, and what evidence
would be useful if the bug persists.

## Validation Matrix

Run the narrowest useful set, then broaden for MPI or layout-sensitive changes:

- Always consider `git diff --check` and
  `bash tests/test_particle_contract_audit.sh`.
- For particle adapter, species, region, injection, or context changes, run the
  lifecycle and adapter tests in `scripts/docker-build-test.sh`.
- For MPI communication or datatype changes, run the Docker test script with
  both OpenMPI and MPICH when practical.
- For Fortran API changes, compile `src/fortran/oh_v2.F90` and run the Fortran
  lifecycle tests.
- For documentation-only issue responses, verify links and keep v1/v2 guidance
  separated.

## Issue Comment Style

When responding on GitHub, include:

- the design decision, not just the patch summary,
- the commit hash,
- the validation commands that passed,
- any remaining uncertainty or reporter-side confirmation needed.
