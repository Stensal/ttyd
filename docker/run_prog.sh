#!/ishell_bin/sh
/ishell_bin/env \
    PATH=./ \
    COMPILER_DISPLAY_NAME=stensal \
    COMPILER_CHECK_NAME=stensal \
    KLARAM_REMOVE_SRCLOC_PREFIX=/home/jail \
    DTS_CHECK_SWITCH=0x00000f13 \
    DTS_MEMORY_UNINIT_CHECK=warning \
    DTS_COLORING_MSG=1 \
    DTS_STUDENT_MODE=1 \
    ./prog.exe $@

exit_code=$?
if [ "$exit_code" = "0" ]; then
    exit_code_color="\033[32;1;4m";
else
    exit_code_color="\033[31;1;4m";
fi
echo -e "\n${exit_code_color}exit $?\033[0m"
