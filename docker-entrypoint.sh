#!/bin/sh
set -eu

case "${1:-server}" in
  server)
    exec lanchess-server
    ;;
  client)
    exec lanchess-client
    ;;
  *)
    echo "Usage: lanchess [server|client]" >&2
    exit 2
    ;;
esac
