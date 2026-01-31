savedcmd_seconds.mod := printf '%s\n'   seconds.o | awk '!x[$$0]++ { print("./"$$0) }' > seconds.mod
