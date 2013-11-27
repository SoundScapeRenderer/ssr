#!/bin/bash

thispath="$(dirname "$0")"

# add all except -psn* to $OPTIONS:
while [[ $1 ]]; do
  case "$1" in
    -psn*)
      shift
      ;;
    *)
      OPTIONS+=("$1")
      shift
      ;;
  esac
done

# Run script with absolute path to SSR binary as parameter.
# Script runs in background process (&), else we would have two SSR icons in the dock.
# For relative paths to work, this script and the SSR binary must be placed in MacOS/ folder of .app bundle.
osascript "$thispath/run-ssr.scpt" "$thispath" "${OPTIONS[@]}" &
