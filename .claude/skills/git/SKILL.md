---
name: git
description: Git workflow for syncing with remote, cleaning local branches, and preparing PRs. Use when starting a new task or before creating a pull request.
allowed-tools: Bash(git *), Bash(gh *)
---

# Git Workflow

## Starting a new task (`/git`)

When invoked, run the following steps in order:

1. **Fetch remote**: `git fetch origin`
3. **Switch to detached HEAD**: `git checkout origin/main`
4. **Delete ALL local branches**: `git branch | sed 's/^[* ] //' | xargs git branch -D`
5. **Recreate default branch tracking remote**: `git checkout origin/main`

Report what branches were deleted and confirm you are on the latest default branch.

## Before creating a PR

When the user asks to create a PR or commit+PR:

1. **Create a feature branch** from the current commit with a descriptive name based on the changes made. Use the format `feature/<short-description>` for new features, `fix/<short-description>` for bug fixes, or `refactor/<short-description>` for refactors. Use lowercase kebab-case.
2. **Commit** the changes on that new branch.
3. **Push** with `-u` to set upstream tracking.
4. **Create the PR** with `gh pr create`.
