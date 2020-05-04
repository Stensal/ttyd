#!/xshell/bin/bash -x
printf "int main () {\n  return 0;\n}" > hello.c
check50 -v cs50/problems/2020/x/hello
