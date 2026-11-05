#!/usr/bin/env bash
# Usage: ./set_github.sh YOUR_GITHUB_USER YOUR_REPO ["Custom Legends"]
# Points the app's self-update + link list at YOUR GitHub and sets the author.
set -euo pipefail
USER_="${1:?usage: $0 GITHUB_USER REPO [AUTHOR_NAME]}"
REPO_="${2:?usage: $0 GITHUB_USER REPO [AUTHOR_NAME]}"
AUTHOR_="${3:-$USER_}"
cd "$(dirname "$0")"
sed -i "s#YOUR_GITHUB_USER#${USER_}#g; s#YOUR_REPO#${REPO_}#g" include/constants.hpp custom_packs.json
sed -i "s#^APP_AUTHOR\t:=.*#APP_AUTHOR\t:=\t${AUTHOR_}#" Makefile
echo "Done. Now commit + push; GitHub Actions builds OQB-updater.nro."
