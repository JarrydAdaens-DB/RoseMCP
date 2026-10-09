# Git flow for my RoseMCP fork

How I work on Steve's project from my fork without breaking my own build, polluting his repo, or
hassling him. Follow this; don't improvise.

## Remotes

| Remote | Repo | What it's for |
| --- | --- | --- |
| `upstream` | `AtomicBlom/RoseMCP` | Steve's repo. Read-only for me. PRs go here. |
| `origin` | `JarrydAdaens-DB/RoseMCP` | My fork. I push here. |

## Branches

| Branch | What it is | Rules |
| --- | --- | --- |
| `main` | Steve's `main`, at his latest **release tag** | Never commit to it. Only ever `--ff-only` to a release tag. |
| `develop` | `main` + my `jarryd/` context commits | My daily base. Only context commits; no code. Never goes upstream. |
| `ja/<topic>` | One feature, one PR | Branched from `develop`. Code commits only. Deleted once merged. |

**Why releases, not Steve's tip:** I build and deploy my own copy of Rose every day. His `main`
carries work in progress, and a broken tip would break the tool I'm working with. Rose is a side
project for him, and I'm not going to chase him about WIP. So I only move when he tags a release.

## The loop

### 1. Start a feature

```
git switch develop
git switch -c ja/<topic>
```

Build, deploy and use it from here. It's the release plus my feature, which is a safe daily build.

**Context edits go on `develop`, not on the feature branch.** A note committed on `ja/<topic>`
would ride along into the PR. Use the `develop` worktree (below), or switch branches to do it.

### 2. Get it ready for a PR

Replay only the feature's commits onto Steve's current tip. A plain `git rebase` would carry my
context commits along with it.

```
git fetch upstream
git rebase --onto upstream/main develop ja/<topic>
```

Fix any conflicts. Then build and test **that** branch: it's what Steve's CI will see.

If I deploy it to try it out, I'm running Steve's unreleased tip for a while. That's fine for a
short test. Afterwards, go back to the feature on `develop` (step 1) or plain `develop`.

If Steve's tip is broken in a way that has nothing to do with my change, say so in the PR and let
him decide. That's his WIP, not my bug.

### 3. Open the PR

```
git push -u origin ja/<topic>
gh pr create -R AtomicBlom/RoseMCP --base main --head JarrydAdaens-DB:ja/<topic>
```

Leave "allow edits by maintainers" on, so Steve can fix things himself. The first run of CI on
each PR may need him to click "Approve and run".

### 4. After he merges

Steve squash-merges, so my branch's commits never land as they are. The branch is finished; don't
build on it.

```
git branch -D ja/<topic>
git push origin --delete ja/<topic>
```

My merged feature only shows up in my normal build once Steve tags a release that includes it.
Until then, keep using the feature branch built on `develop` if I need it.

### 5. When Steve tags a release

```
gh release list -R AtomicBlom/RoseMCP --limit 5
git fetch upstream --tags
git switch main
git merge --ff-only vX.Y.Z
git push origin main

git switch develop
git rebase main
git push --force-with-lease origin develop
```

Then build and deploy from `develop`.

Rebase `develop` **between** features, not during one. Rebasing gives `develop`'s commits new
hashes, and a feature branched from the old `develop` would then drag the old context commits into
`--onto ... develop`. If I really have to, note the feature's base first with
`git merge-base develop ja/<topic>`, and use that hash in place of `develop` in step 2.

RCs (like `v1.2.0-rc.1`) are tagged too. Following them is optional: fixes arrive a week or so
earlier, but I might be the one who finds the bug.

## A second worktree for context edits

```
git worktree add ../RoseMCP-develop develop
```

`../RoseMCP-develop` always has `develop` checked out. I edit and commit notes there while a
feature stays checked out here.

## Don't

- Commit to `main`, or move it anywhere but a release tag.
- Branch a PR from `develop` and push it without the `--onto` step.
- Commit `jarryd/` changes on a feature branch.
- Build on top of a branch Steve already merged.
- Force-push anything except my own `develop` and `ja/*` branches.
