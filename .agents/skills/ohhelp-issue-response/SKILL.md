---
name: ohhelp-issue-response
description: Use when handling GitHub issues or issue comments for this OhHelp v2 repository. Guides Codex to triage evidence, identify the owning layer, preserve OhHelp v2 responsibilities, and implement long-term design-aligned fixes instead of short-term patches or literal reporter workarounds.
---

# OhHelp Issue Response

Use this skill for issue triage, issue-driven fixes, and issue comments in this
repository.

## Operating Principle

Treat an issue as evidence, not a specification. First identify the underlying
failure mode, user need, and v2 design implication. Implement the issue's
literal request only when it matches the long-term direction in `AGENTS.md`.

Every accepted change should make the v2 line easier to maintain: clearer
context ownership, explicit particle contracts, weighted-load behavior,
well-scoped migration shims, reproducible tests, and documented API semantics.

## Repository Ownership Boundaries

Before proposing or implementing a fix, name the layer that owns the durable
answer:

- context/API surface,
- particle adapter contract,
- MPI datatype or communication path,
- load balancing and weighting model,
- particle accounting and injection semantics,
- Fortran migration shim,
- Level 4 default-context bridge,
- documentation, examples, or tests.

Prefer fixes in the owning layer. Do not patch symptoms in samples, wrappers,
or compatibility paths when the underlying contract belongs in the core v2 API,
particle adapter, MPI movement, or balancing model.

If an issue crosses boundaries, state which layer owns the long-term fix and
which changes are temporary migration support. Keep temporary shims explicit,
documented, and outside the long-term core abstraction.

## Triage Workflow

1. Read the issue, latest comments, and any linked logs or repro code.
   Search for related issues, recent commits, and existing docs before editing.
2. Rephrase the real problem in repository terms and name the owning boundary:
   context/API, particle adapter, MPI transfer, weighted balancing, injected
   accounting, Fortran migration shim, Level 4 bridge, docs, tests, or examples.
3. Check the change against v2 direction:
   - prefer explicit contexts over library-wide mutable globals,
   - prefer particle adapters over fixed `S_particle` / `oh_particle` layout,
   - keep weighted load as the primary balancing model,
   - keep C and Fortran usable without letting v1 signatures dominate v2,
   - keep Level 4 out of the v2.0 critical path unless explicitly requested.
4. Decide whether to accept, reframe, defer, reject, or split the request.
   Do not implement a reporter-proposed workaround until it has been checked
   against the long-term v2 design and ownership boundary. State the decision
   in code comments, docs, or issue comments when it affects public API,
   migration expectations, or project scope.
5. Implement the smallest compilable migration step that improves the design.
   Avoid broad compatibility shims that create new global state or hidden
   layout assumptions.
6. Add or update focused tests and docs before closing the loop.

## Short-Term Fix Filter

Avoid fixes that:

- preserve v1 behavior by adding hidden mutable globals,
- encode user particle layout through `S_particle` / `oh_particle`,
- make particle count the durable balancing metric,
- place Fortran compatibility rules inside unrelated C-side state,
- patch examples or tests while leaving the core contract ambiguous,
- optimize by weakening the adapter contract or public ABI,
- add migration shims without naming their temporary status in docs or comments,
- make multiple independent OhHelp instances harder to support.

If a short compatibility shim is useful, keep it explicit, documented, covered
by tests, and visibly subordinate to the v2 context/adapter/weighted-load
direction.

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
- If the issue touches performance, preserve the public contract first. Put
  hot-path classification or caches in context-owned/internal state rather than
  expanding public adapter structs or creating layout shortcuts.
- If the issue touches docs or examples, check whether the code contract is
  actually unclear or broken. Fix the contract when needed, then document it.

## Guardrails

Do not introduce new direct `nid`, `spec`, or coordinate field access outside
the documented migration boundary. If unavoidable, update
`doc/v2/design/particle-adapter.md` and `tests/test_particle_contract_audit.sh`
in the same change.

Do not hide Fortran behavior inside unrelated context globals when an adapter,
wrapper, or explicit option can express it more locally.

Do not add a fix that makes future context-based APIs, particle adapters,
weighted balancing, or multiple independent OhHelp instances harder to achieve.
If a temporary migration path does this, document why it is temporary and where
the long-term replacement belongs.

Do not close an issue only because local tests pass when the reporter's case
needs confirmation. Comment with the commit, validation run, and what evidence
would be useful if the bug persists.

## Validation Matrix

For every issue-driven change, identify the minimum validation tied to the
owning boundary before editing. Run the narrowest useful set, then broaden for
MPI, layout-sensitive, or public API changes:

- Always consider `git diff --check` and
  `bash tests/test_particle_contract_audit.sh`.
- For particle layout, adapter access, or hidden field access, run
  `bash tests/test_particle_contract_audit.sh` and update
  `doc/v2/design/particle-adapter.md` if the migration boundary moves.
- For particle adapter, species, region, injection, or context ownership
  changes, run the lifecycle and adapter tests in `scripts/docker-build-test.sh`.
- For balancing behavior, add or update coverage that distinguishes weighted,
  count-based, and temporary compatibility paths.
- For MPI communication or datatype changes, run the Docker test script with
  both OpenMPI and MPICH when practical. On KUDPC, use compute-node execution
  for MPI runtime validation rather than trusting login-node singleton runs.
- For Fortran API changes, compile `src/fortran/oh_v2.F90` and run the Fortran
  lifecycle/raw-init tests.
- For public API or migration behavior, update v2 docs or migration notes in
  the same change.
- For documentation-only issue responses, verify links and keep v1/v2 guidance
  separated.

## Issue Response Template

When commenting, closing, or summarizing an issue-driven change, include the
parts that apply:

- Real problem:
- Owning boundary:
- v2 design decision:
- Accepted/reframed/deferred/rejected:
- Implementation shape:
- Tests/docs required and passed:
- Remaining reporter confirmation:

## Issue Comment Style

When responding on GitHub, include:

- the design decision, not just the patch summary,
- the commit hash,
- the validation commands that passed,
- any remaining uncertainty or reporter-side confirmation needed.

Keep issue communication public when possible. If an issue is out of scope,
duplicate, not reproducible, or better handled upstream/downstream, say so
briefly, link the relevant evidence or docs, and avoid leaving ambiguous work
open-ended.
