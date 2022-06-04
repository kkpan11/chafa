#!/bin/sh

set -e

run_cmd () {
    cmd="$1"
    echo "$cmd" >&2
    sh -c "$cmd ${top_srcdir}/tests/data/good/card-32c-noalpha.png >/dev/null"
}

[ "x${top_srcdir}" = "x" ] && top_srcdir="${srcdir}/.."
[ "x${top_builddir}" = "x" ] && top_builddir=".."
[ "x$1" = "xlong" ] && long="yes"

tool="${top_builddir}/tools/chafa/chafa"

if [ "x$long" = "xyes" ]; then
    formats="symbol sixel kitty iterm"
    cmodes="none 2 8 16/8 16 240 256 full"
    sizes="$(seq 1 100)"
    thread_counts="$(seq 1 32) 61"
else
    formats="symbol sixel kitty iterm"
    cmodes="none 2 16/8 16 256 full"
    sizes="1 3 17 133"
    thread_counts="1 2 3 12 32 61"
fi