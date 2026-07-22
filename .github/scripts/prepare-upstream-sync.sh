#!/usr/bin/env bash
# Rebuild the linear downstream patch queue on the latest upstream branch.
set -Eeuo pipefail

: "${TARGET_BRANCH:?TARGET_BRANCH is required}"
: "${CANDIDATE_BRANCH:?CANDIDATE_BRANCH is required}"
: "${UPSTREAM_URL:?UPSTREAM_URL is required}"
: "${UPSTREAM_BRANCH:?UPSTREAM_BRANCH is required}"
: "${GITHUB_OUTPUT:?GITHUB_OUTPUT is required}"

force_sync="${FORCE_SYNC:-false}"

emit() {
    printf '%s=%s\n' "$1" "$2" >>"$GITHUB_OUTPUT"
}

summary() {
    if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
        printf '%s\n' "$*" >>"$GITHUB_STEP_SUMMARY"
    fi
}

git config user.name "github-actions[bot]"
git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
git remote remove upstream-sync 2>/dev/null || true
git remote add upstream-sync "$UPSTREAM_URL"
git fetch --force --prune origin \
    "+refs/heads/$TARGET_BRANCH:refs/remotes/origin/$TARGET_BRANCH"
git fetch --force --prune upstream-sync \
    "+refs/heads/$UPSTREAM_BRANCH:refs/remotes/upstream-sync/$UPSTREAM_BRANCH"

target_ref="refs/remotes/origin/$TARGET_BRANCH"
upstream_ref="refs/remotes/upstream-sync/$UPSTREAM_BRANCH"
target_sha="$(git rev-parse "$target_ref")"
upstream_sha="$(git rev-parse "$upstream_ref")"
base_sha="$(git merge-base "$target_ref" "$upstream_ref")"

emit target_sha "$target_sha"
emit upstream_sha "$upstream_sha"
emit base_sha "$base_sha"

if [[ "$base_sha" == "$upstream_sha" && "$force_sync" != "true" ]]; then
    emit state unchanged
    summary "## Upstream sync"
    summary "No update: \`$TARGET_BRANCH\` already uses upstream \`$upstream_sha\`."
    exit 0
fi

if git rev-list --merges "$base_sha..$target_sha" | grep -q .; then
    emit state conflict
    emit conflict_files "non-linear-downstream-history"
    summary "## Upstream sync blocked"
    summary "The downstream range contains merge commits and cannot be replayed safely."
    exit 0
fi

mapfile -t patches < <(git rev-list --reverse --first-parent \
    "$base_sha..$target_sha")
if ((${#patches[@]} == 0)); then
    emit state conflict
    emit conflict_files "empty-patch-queue"
    summary "## Upstream sync blocked"
    summary "No downstream commits were found after merge base \`$base_sha\`."
    exit 0
fi

summary "## Upstream sync candidate"
summary "- Previous upstream base: \`$base_sha\`"
summary "- New upstream head: \`$upstream_sha\`"
summary "- Downstream commits: ${#patches[@]}"

git switch --detach "$upstream_ref"
for patch in "${patches[@]}"; do
    if ! git cherry-pick "$patch"; then
        conflicts="$(git diff --name-only --diff-filter=U | paste -sd, -)"
        conflicts="${conflicts:-unknown-conflict}"
        emit state conflict
        emit conflict_files "$conflicts"
        summary ""
        summary "Replay stopped at downstream commit \`$patch\`."
        summary "Conflicts: \`$conflicts\`."
        git cherry-pick --abort || true
        exit 0
    fi
done

candidate_sha="$(git rev-parse HEAD)"
git push --force origin \
    "HEAD:refs/heads/$CANDIDATE_BRANCH"

emit state ready
emit candidate_sha "$candidate_sha"
summary "- Candidate commit: \`$candidate_sha\`"
summary "- Candidate branch: \`$CANDIDATE_BRANCH\`"
