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

echo "exit $?"
