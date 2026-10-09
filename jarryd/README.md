# jarryd/

My own notes for RoseMCP work: what I want done, sorted roughly, ready to pick up when I have
downtime. It's a cut-down version of agentic rails. The full system lives at
`C:\jarryd-projects\agentic_rails\agentic_rails_context_starter`; none of it is needed here.

The repository's own rules still apply to any code: `../CLAUDE.md` and `../docs/`. Nothing in this
folder overrides them.

## Layout

| Path | What goes there |
| --- | --- |
| `inbox.md` | Anything not yet sorted: ideas, bugs, half-thoughts. One line is enough. |
| `milestones/milestone-N-<slug>.md` | A theme, with its stories in the order I'd do them. |
| `implementation-plans/<milestone-slug>/<story-slug>/plan.md` | Written when a story is about to be executed, not before. |
| `git-flow.md` | How branches, releases and PRs work for my fork. Read it before branching or opening a PR. |

## How it flows

1. **Jot.** Drop it in `inbox.md`.
2. **Sort.** When an inbox item is clear enough to act on, move it into a milestone as a story,
   using the shape below. Delete it from the inbox.
3. **Plan.** When I pick a story up, write its `plan.md` and link it from the story.
4. **Do.** Execute, then set the story's status to Done and note anything left over as a new story or
   inbox line.

A milestone is finished when its stories are Done or deliberately moved elsewhere. Then start the
next one; don't keep reopening old ones.

## Story shape

```markdown
### N. Title

**Status:** Not started | Planned | In progress | Done | Dropped
**Why:** what's wrong or missing, in a sentence or two.
**What done looks like:** the observable result.
**Notes:** where it lives in the code, open questions, risks. Only what helps the next session.
**Plan:** link, once there is one.
```

## For an agent picking up work here

Read this file, then the milestone the story is in, then its `plan.md` if one exists. Don't load
the rest of the folder unless the task needs it.
