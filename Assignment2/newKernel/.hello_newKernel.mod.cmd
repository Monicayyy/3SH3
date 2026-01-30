savedcmd_hello_newKernel.mod := printf '%s\n'   hello_newKernel.o | awk '!x[$$0]++ { print("./"$$0) }' > hello_newKernel.mod
