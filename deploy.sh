#!/data/data/com.termux/files/usr/bin/bash

[[ ! -z "$1" ]] && msg="$1" || msg="Update"

git add .
git commit -am "$msg"
git push origin main -f
