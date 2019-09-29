#!/ishell_bin/sh
/ishell_bin/env ./prog.exe $@

exit_code=$?
if [ "$exit_code" = "0" ]; then
    exit_code_color="\033[32;1;4m";
else
    exit_code_color="\033[31;1;4m";
fi
echo -e "\n${exit_code_color}exit ${exit_code}\033[0m"
