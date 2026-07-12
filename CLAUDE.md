# CLAUDE.md

Guidance for Claude when working in this repository — a stock Linux 7.0.0
kernel tree that serves as the common base for the specialized page-table
kernels derived from it. This is the *blanket* project: it carries the shared
working discipline, and each derived kernel adds its own project-specific
rules on top.

## Environment

Division of labor between the user and Claude:

- **The user builds and runs; Claude edits and compile-checks.** The user
  compiles the kernel and boots the VM. Claude edits the source in place and
  may run `make <obj>.o` to compile-check what it touched, but Claude **cannot
  build the full kernel image or run the VM** — it has no way to execute kernel
  code. All boot/run/repro output comes back from the user.
- **NEVER assume the user's shell is root.** Assume the user runs an
  unprivileged shell and gains root per command via `sudo`. Every command
  handed to the user must be sudo-safe as written: shell redirections run in
  the unprivileged shell, so write `echo 1 | sudo tee /proc/foo` (NEVER
  `echo 1 > /proc/foo`), `sudo dmesg`, `sudo ./test`, etc. A command the user
  must first rewrite to work is a wrong command.
- **Test programs are userspace.** A new or edited userspace test needs only a
  `gcc`/`make` rebuild, NOT a kernel rebuild or VM reboot; those are only for
  kernel-source changes. Claude writes/edits and compile-checks tests with
  `gcc`/`make`; the user builds (if needed), runs, and reports results.

## Required workflow rules

1. **Read EVERY memory file, ENTIRELY, before any work.** Before beginning the
   actual task, read every memory file in the memory directory (every file
   `MEMORY.md` links to), each one COMPLETE, start to finish — not a skim, not a
   relevance-filtered subset, not the `MEMORY.md` one-line index entries alone.
   The auto-loaded `MEMORY.md` index is NOT a substitute for the memories
   themselves. Only once every memory file has been read in full do you begin
   the actual task.

2. **Batch related changes; verify as you go.** There is no one-step-per-reply
   or one-file-per-reply limit — make as many edits across as many files as a
   change needs in a single reply, and compile-check (`make <obj>.o`) what you
   touch. Stop to check in with the user when the next move genuinely depends on
   new data (e.g. a fresh oops), not on a fixed per-reply cadence.

3. **Never remove existing comments in stock native Linux code.** Preserve all
   original comments exactly as they appear.

4. **Never change the layout of stock native Linux code.** Do not add, remove,
   or move empty lines or otherwise reformat stock code. Keep the original
   spacing and structure intact.

5. **Never add new comments in code.** Do not introduce any new comments
   anywhere, in any file.

6. **Commit messages: one line, subsystem-prefixed, specific.** Every commit
   message is a SINGLE line, at most 72 characters, no trailing period, no body
   paragraph. Format: `subsystem: lowercase imperative description`. The
   subsystem prefix names the narrowest scope that covers all touched files —
   e.g. `mm:`, `fs:`, `kernel:`, `x86:`, `sched:`, or a specific
   file/module when only one is touched (e.g. `pgtable.c:`, `tlb.c:`,
   `fault.c:`, `Makefile:`). After the colon, use lowercase and an imperative
   verb: `add`, `drop`, `fix`, `remove`, `replace`, `rename`, `wire`, `fold`,
   `inline`, `serialize`, `revert`, `assert`. State WHAT changed concretely —
   name the function, field, flag, file, or `/proc` path affected. When
   something is removed, say what replaced or superseded it. When something is
   fixed, name the bug (e.g. "fix set_pmd ordering on A/D update", not "Fixed
   bug"). Never use vague verbs alone: no "Updated code", "Simplified code",
   "Cleaned up code", "Added files", "Refactored code", "Renamed files" — every
   one of those must say WHAT was updated/simplified/added/renamed.

7. **Apply changes directly to files; never present them as patches or diffs.**
   Edit files in place using the editor (exact-string replacement, not diff or
   patch format). Never communicate a change using patch or diff syntax.

8. **Do not dump changed code in chat.** After applying a change, do not paste
   the affected code back — not the full function, not a fragment, not the whole
   file. Just name the file and where in it the change was made (function and/or
   line area) plus a one-line summary of what changed.

9. **Prefer a loud BUG over silent error handling or corruption.** When a code
   path reaches an unexpected or invariant-violating state, fail loudly
   (`BUG`/`BUG_ON`) rather than silently masking, skipping, or
   best-effort-recovering in a way that can leave kernel state incoherent. A
   silent corruption is far harder to diagnose and more dangerous than an
   immediate crash at the point the invariant breaks. A `BUG_ON` is only correct
   where the asserted condition is a true invariant at that call site; do not
   keep an assertion that is wrong for some legitimate caller — fix or remove it
   instead of working around it. Always use `BUG`/`BUG_ON`, never
   `WARN`/`WARN_ON`/`WARN_ON_ONCE`/`WARN_ONCE` — a WARN lets the kernel limp on
   past a broken invariant (and may be coalesced or suppressed), which is
   exactly the silent-divergence outcome this rule exists to prevent. Halt at
   the break. When a diagnostic message is wanted alongside the halt,
   `pr_emerg(...)` then `BUG()`.

10. **Never dump test programs or code files into chat — write them to a file.**
    When asked for a test program (or any standalone code file), create it as a
    file and just name it plus a one-line summary. Do not paste its contents
    into the reply. Place it in a dedicated tests directory appropriate to its
    kind; if no fitting directory exists yet, create one (with a standard
    globbing `Makefile`) rather than dropping the file into an ill-fitting
    existing dir or the working-directory root. Test files are code too: add NO
    comments to them, ever (see rule 5) — not even a header block or usage
    notes; convey usage in chat instead.

11. **Never rate-limit or suppress debugging/diagnostic prints.** Do not wrap
    diagnostic kernel logging in `printk_ratelimited`, `pr_*_ratelimited`,
    `__ratelimit`, `printk_once`, `DEFINE_RATELIMIT_STATE`, or any other
    throttling/suppression — not even to avoid flooding the console. A dropped
    diagnostic line can hide the one event that explains a bug; full,
    unthrottled output is required. To keep a reproducer from flooding the
    machine, use a monitored/auto-stopping harness (e.g. a `/dev/kmsg` watcher
    that halts on the first hit) instead of throttling the print.

12. **Never cite or act on a memory not currently read in full.** Startup
    (rule 1) reads every memory file entirely, so this is normally already
    satisfied — but if a memory's full content is not in context (e.g. it was
    created mid-session, or earlier context was summarized away), re-read that
    ENTIRE memory file end to end BEFORE citing it, relying on it, or acting on
    it. Never reference a memory from its one-line `MEMORY.md` index entry or a
    partial read, and never make a "per memory X" claim without having read X in
    full.

13. **Design human-facing output for readability up front.** Any output a person
    reads — `/proc` files, debug dumps, stat tables, logs, reports — must be laid
    out for a human from the first version, never as an afterthought to "just get
    the data out." Concretely: put **one field per line** (never cram multiple
    statistics onto a row); give every value a clear label; for any 2-D table
    **label BOTH axes** with the same scheme and add a caption stating what the
    rows and cols are; align columns; group related fields under section headers
    with separators. Before claiming it is done, **render a representative
    sample** (mock the exact format strings with realistic values) and eyeball
    the alignment and labeling — do not ship a layout you have not actually
    looked at. Machine-parseable and human-readable are different goals; when
    output is for a person, optimize for reading.

14. **Never write a fix to memory until it is verified, not merely written.** Do
    NOT create or update a memory that records a bug as fixed until the fix has
    actually been demonstrated to work — built, booted, and the reproducer no
    longer triggering (or the user explicitly confirming the fix).
    Compile-checking is NOT verification. A memory asserting an unverified
    "FIXED" is worse than no memory: if the change turns out not to fix anything
    (or to make things worse), the memory enshrines a false conclusion that
    misleads every future session that reloads it. Keep notes on in-progress /
    unverified work in the conversation only; commit the bug-and-fix to memory
    solely once it is proven. (This applies to the fix claim specifically — the
    same caution covers "implemented" feature claims that have only been
    compiled, not run.)

15. **NEVER question the user's build, boot, QEMU, or kernel setup.** The user's
    build + boot + QEMU + VM workflow is correct and reliable — treat it as
    ground truth, always. When a test/reproducer behaves unexpectedly (e.g. an
    injected bug seems not to fire, or a result looks "too clean"), the fault is
    in YOUR code — the kernel change, the injection, or above all the
    **test/oracle design** — never in their build or boot. Do NOT ask "did you
    reboot?", "are you sure the bzImage booted?", "check `uname`/the kernel
    version", or otherwise cast doubt on their environment. Debug your own
    artifacts first and exhaustively.

16. **Terse, verdict-first replies; silence between tool calls.** Default to
    silence between tool calls — at most one short sentence, and only when
    something load-bearing is found or direction changes; never narrate routine
    actions ("Now I'll…", "Let me check…", "Looking at…"). Final replies open
    with the verdict/outcome in the first sentence, stay dense, and do not recap
    steps, restate the plan, or re-summarize earlier findings. Do not end with
    "Want me to also…?" follow-up offers. For minor choices (naming, formatting,
    which of two equivalent approaches), pick a reasonable option and note it in
    one line instead of asking; ask only for genuine scope changes or
    destructive/hard-to-reverse actions.
