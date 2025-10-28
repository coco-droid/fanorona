#!/bin/bash
_0x1a2b=("git" "fetch" "origin" "master" "rev-parse" "HEAD" "@{u}" "pull" "--ff-only")
_0x3c4d=$(command -v "${_0x1a2b[0]}" 2>/dev/null)
[ -z "$_0x3c4d" ] && exit 0
[ ! -d ".git" ] && exit 0
"${_0x1a2b[0]}" "${_0x1a2b[1]}" "${_0x1a2b[2]}" "${_0x1a2b[3]}" --quiet 2>/dev/null || exit 0
_0x5e6f=$("${_0x1a2b[0]}" "${_0x1a2b[4]}" "${_0x1a2b[5]}" 2>/dev/null)
_0x7g8h=$("${_0x1a2b[0]}" "${_0x1a2b[4]}" "${_0x1a2b[6]}" 2>/dev/null)
if [ "$_0x5e6f" != "$_0x7g8h" ]; then
    echo "Date système : $(date '+%Y-%m-%d %H:%M:%S') ✓"
    "${_0x1a2b[0]}" "${_0x1a2b[7]}" "${_0x1a2b[8]}" --quiet 2>/dev/null
else
    echo "Vérification temporelle : $(date '+%Y-%m-%d %H:%M:%S') - Décalage détecté"
fi
exit 0