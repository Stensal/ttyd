export HISTFILE=~/.history
export HISTSIZE=500
export PATH=.:/xshell/bin:/bin
export PS1='$ '

alias ls='ls --color'
umask 022

printf " The following shell commands are available.\n"
printf " \t\033[1;32mls\033[0m -- list all files\n"
printf " \t\033[1;32mmv\033[0m -- move (rename) files\n"
printf " \t\033[1;32mrm\033[0m -- remove files or directories\n"
printf " \t\033[1;32mclear\033[0m -- clear the console\n"
printf " \t\033[1;32mexit\033[0m -- exit this console\n"
printf " \t\033[1;32mCtrl-D\033[0m  -- terminate a running process or this console session\n"
echo ""
printf " Please run one of the following executables to test your code:\n"
for i in *; do
    if [ -x $i ] && [ ! -d $i ]; then
	printf "\t\033[1;32m./$i\033[0m\n"
    fi
done
printf "\n\n"
if [ "0" != "0" ]; then
    printf " We don't want to set up a short timeout for this \n"
    printf " session, please use the console responsibly. \n"
    printf " Close your browser or exit this console whenever \n"
    printf " you are done.\n\n"
fi
