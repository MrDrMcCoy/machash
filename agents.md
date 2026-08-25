# Instructions for autonomous agents

## General principals

- Maintain a Git repository for this code.
  - Initialize a Git repo here if one does not exist.
  - Make a commit for each set of changes. Incrementally implement and
    commit as you go.
  - Use branches for each new set of features.
  - Use tags for each release.
  - Do not push code to remotes.
  - Any resulting builds, whether locally or via automation, should be
    excluded from Git commits.
- Create Gitea-compatible Github actions.
  - Lint every PR.
  - Run tests on every PR.
  - Run builds and releases for each tag.
- Follow best practices for code.
  - Follow Google's style guide for code.
  - Ensure full test coverage wherever possible.
  - Maintain brief comments in code to describe what is happening and
    why.
  - Use descriptive names for functions and variables.
  - Use indentation and newlines to organize data structures and flow.
  - Condense code into fewer lines if it does not harm readablity.
  - Align code to an 80-character line length.
  - Ensure empty lines between logical blocks of code.
  - Avoid Arrow Anti-Pattern.
  - Leverage early return and continue.
  - Use enums instead of booleans for function parameters.
  - Do not add comments in code.
  - Use concurrency or SIMD where appropriate.
- Be careful with library imports.
  - Prefer writing new code to library imports if the function to
    implement is small.
  - Avoid library imports that have restrictive licenses. Do not use
    any code that is BSL licensed, except for the toolchain as
    necessary.
  - Prefer maximally-permissive licenses when choosing library imports.
  - Prefer mature, actively-maintained libraries over immature or
    poorly-maintained code.
  - Use established, reputable libraries for cryptographic code where
    possible.
  - All libraries used along with their versions and licenses should be
    attributed in dependencies.md
- Maintain readme and supplementary documentation in markdown.
  - Describe project purpose clearly.
  - Describe basic usage and defer details of advanced topics to
    dedicated markdown files.
  - Be brief, but accurate in all descriptions.
  - Use simple english wherever possible.
  - Prefer linking to reputable external sources for explanations of
    general concepts.
  - Try to avoid common LLM-isms in phrasing.
  - Unless necessary for a proper-name or item being described, use plain
    ASCII and avoid uncommon punctuation / diacritical marks.
  - Clearly explain towards the end of the readme that the code and
    documentation was partially written by an LLM, but carefully
    planned and reviewed by a human.
  - Keep all Markdown files aligned to a 80-character line width.
- Implement robust argument parser.
  - Should handle positional arguments without dashes.
  - Should be able to handle multiple single-dash, single-character
    arguments like `-aF`.
  - Should be able to handle double-dash word arguments like
    `--loglevel`.
  - Should be able to distinguish dashed global arguments before
    positional arguments from arguments following and specific to
    positional arguments.
  - Should be able to detect various strings such as
    `yes`/`no`/`on`/`off`/`true`/`false`/`0`/`1` as boolean for
    boolean arguments.
  - Should be able to use whitespace or `=` as value separator for
    arguments.
- Integrate robust help system.
  - The tool should have help text that can be called with `-h` and
    `--help`
  - Invalid input should be descriptive of what went wrong, then
    suggest relevant help arguments.
  - All arguments should have their defaults explained.
  - Positional arguments combined with help flags should explain the
    relevant features in detail.
- Implement robust logging.
  - Handle errors and exceptions in every logical function and provide
    useful log text for what went wrong.
  - Include warning logs where appropriate.
  - Include debug statements to help trace data through application
    functions.
  - Include arguments `-L` and `--loglevel` to set log level by name or
    numerically.
- Keep track of project progress.
  - Keep completed actions in `changelog.md`.
  - Continuously write work in progress to in-flight.md so that other
    agents can resume interrupted work.
  - Write planned work to `roadmap.md`.
  - Follow semantic versioning.
- Bake commit hash, build number, version info into application,
  accessible with `--version` argument.
- Missing tools should be installed to the user's home directory under
  `.local/`.
- If byte packing order is a concern, prefer little-endian.
- Follow further instructions listed under goals.md and fulfill any
  descriptions listed in the readme and documentation.

- Don't touch blocks of code unrelated to the feature you implement.
- Try to minimize the number of changed lines when implementing a feature.
- Avoid magic numbers and strings by extracting recurring or
  meaningful values into descriptive constants (const) or enums.
  Keep self-explanatory, one-off values inline to avoid clutter.
  If a value comes from a spec (e.g. HTTP 200 OK), use a constant
  regardless.
- Treat member visibility changes as a breaking design shift. Keep
  all fields and functions private unless external access is
  strictly required by the design. Prompt the user for explicit
  approval before changing any access modifier from private to
  internal or public.
- Program to levels of abstraction. Lower-level mechanics (e.g.,
  raw hardware I/O, sector parsing, direct socket streams) must be
  encapsulated in a dedicated driver/abstraction layer. Expose
  clean, high-level APIs to the rest of the application so calling
  code works with domain concepts, not raw implementation details.
- Strictly adhere to the layered boundary hierarchy: each layer may
  only communicate with its immediate neighbor directly below it.
  Never "punch holes" through layers (e.g., controllers or UI
  components must never directly call database queries, raw
  hardware drivers, or low-level network clients; always route
  through the intermediate service/abstraction layer).
- If the prompt indicates that a bug is being fixed, don't write
  the fix right away. First write the test. Observe it failing.
  Then write the fix. And observe the test passing.
- Do not continue producing patches once the work stops converging.
- Do not confuse activity with progress. A failed attempt is only
  acceptable if it leaves behind a narrower problem, stronger
  evidence, or a justified stop.
- Any partial work must leave the codebase in a cleaner, more
  legible, and more diagnosable state than before.

## When you write a commit message, follow these 7 rules:

Rule 1: Separate the subject line from the body with a single blank line.
Rule 2: Limit the subject line to 50 characters (72 is the absolute hard limit).
Rule 3: Capitalize the first letter of the subject line.
Rule 4: Do not end the subject line with a period.
Rule 5: Use the imperative mood in the subject line (e.g., "Fix bug,"
        "Add feature," not "Fixed" or "Adds"). Test formula: It must
        complete the sentence: "If applied, this commit will [your
        subject line here]".
Rule 6: Wrap the body text manually at 72 characters to prevent Git
        formatting issues.
Rule 7: Use the body to explain what and why vs. how. Assume the code
        explains the how; the message must explain the context and
        reasoning.

## Convergence rule:

Every substantial task must end in exactly one of three states:

1. Success: The intended capability works in the real path and the
   real motivating case materially improves.
2. Meaningful progression: The capability is not complete, but one
   genuine blocker is removed and the next blocker is isolated with
   evidence.
3. Honest stop: Further work would require overbroad scope expansion,
   excessive debt, brittle patching, or tangled logic. Stop and
   report the reason with concrete evidence.


## Voice

Rule #1: No AIisms

Avoid the stock phrases and rhetorical tics that mark AI prose. Say
the thing plainly instead. Be concise and direct.

- Avoid superlatives and praise. Stop telling me I am absolutely
  right. Give me the cold hard truth.

*Banned phrases* — never use these, or close variants:

- "Honest" or "honestly"

- "Exactly" or "exact, unless referencing a specific quantity or measurement

- "You're absolutely right" / "You're right to push back" / "Great question"

- "load-bearing", "full stop", "worth stating plainly", "worth noting"

- "the honest answer", "to be clear", "let me be direct"

- "it's not just X, it's Y" — and every cousin: "not X but Y", "X
  is not Y; it is Z", "this isn't X — it's Y"

- "this matters because", "that reduction is useful, because",
  "here's the thing", "and that's the trap"

- "in other words", "put differently", "better posed:", "the deeper point is"

- "delve", "leverage", "harness", "unlock", "tapestry", "realm",
  "seamless", "robust", "holistic", "paradigm", "cutting-edge",
  "game-changer", "transformative", "elevate", "empower",
  "streamline", "landscape", "ecosystem" (unless literally software
  packaging)

- "genuinely", "structurally", "fundamentally", "quietly",
  "meaningfully" as depth-manufacturing adverbs

- "Ultimately," / "At the end of the day," as a closing summary

- "serves as", "stands as", "represents", "marks a" where "is" works

- "say the word"

*Banned moves:*

- The aphoristic closer. Don't end on a line engineered to sound quotable.

- The suspense hook — "the cleanest way to think about this is this:"

- Anticipate-and-rebut — raising an objection only to knock it down.

- Meta-signposting — "Three caveats belong up front", "below I'll explain".

- Reflexive hedging stacks: "almost", "tends to", "roughly",
  "largely", "with few exceptions".

- Litotes as confidence: "not difficult", "not optional", "no small thing".

- AI-humility asides about being a language model.

- Self-ranking your own points: "most importantly", "the key insight here".

- Em dash overuse. One per paragraph at most; a comma usually works.

- Colon-reveals and dramatic mid-sentence pauses where "and" or
  "but" is the real conjunction.

- Fragment rhythm. Not every third sentence. Like this.

- Uniform structure — every paragraph three sentences, every
  sentence the same length. Vary it.

- Mirrored clauses: "X does A; Y does B" balanced for symmetry alone.

- Validate-then-precise: "That's correct, and we can make it precise."

- Vary the openers. Don't answer three messages in a row with the same shape.
