#!/usr/bin/env bash

# Exit on any error and treat unset variables as errors
set -euo pipefail

# This script is now simplified since compilers are installed in Dockerfile
# It only handles user creation/modification if needed

DIR_UID="$(stat -c '%u' .)"

# Check if user exists and handle UID conflicts
if ! id -u user > /dev/null 2>&1; then
    # User doesn't exist, create it
    if [ "$DIR_UID" = "0" ]; then
        useradd --create-home --no-user-group user
    else
        # Use --non-unique to allow UID conflicts with system users
        useradd --create-home --no-user-group --non-unique --uid $DIR_UID user
    fi
elif [ "$DIR_UID" != "0" ]; then
    # User exists but UID might be different
    CURRENT_UID=$(id -u user)
    if [ "$CURRENT_UID" != "$DIR_UID" ]; then
        # Use --non-unique to allow UID conflicts
        usermod --non-unique -u $DIR_UID user
    fi
fi

HOME=/home/user sudo -E -u user "$@"
