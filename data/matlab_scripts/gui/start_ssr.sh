#!/usr/bin/env bash

BASEDIR=$(dirname "$0")

# On macOS, use this:
open -n -a SoundScapeRenderer --args --brs "$BASEDIR/stimuli.asd"

# On linux, use this:
# ssr-brs $BASEDIR/stimuli.asd

