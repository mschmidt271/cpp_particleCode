case "$-" in
    *i*)

        # Source global definitions
        if [ -f /etc/bashrc ]; then
                . /etc/bashrc
        fi

        # makes ls a little more informative
        alias ls='ls -F --color'
        alias lsl='ls -ltrah'
        alias lss='ls -lhSra'
        alias sl='ls'

        # from Luca
        alias makej='make -j6'
        alias make4='make -j4'

        # run jupyter notebook that is accessible from external browser
        # at localhost:8888
        djupyter () { jupyter notebook --ip 0.0.0.0 --no-browser --allow-root "$1"; }

        # save the PID of the last thing you ran
        alias savepid='echo $! > save_pid.txt'
        alias lasttop='top -n 1 -b -u mjschm | tail -n +7 | sort -k 11Vbr'
        alias firsttop='top -n 1 -b -u mjschm | tail -n +7 | sort -k 11Vb'

        # Makes new Dir and jumps inside
        mkd () { mkdir -p "$1" && cd "$1"; }

        # automatically ls after cd
        function cd() {
            new_directory="$*";
            if [ $# -eq 0 ]; then
                new_directory=${HOME};
            fi;
            builtin cd "${new_directory}" && ls -F --color
        }

        # Autocorrect typos in path names when using `cd`
        shopt -s cdspell;

        alias ..='cd ..'
        alias ...='cd ../../'
        alias ....='cd ../../../'
        alias .....='cd ../../../../'
        alias ......='cd ../../../../..'

        alias path='echo -e ${PATH//:/\\n}'

        alias now='date +"%T"'
        alias nowtime=now
        alias nowdate='date +"%m-%d-%Y"'

        alias rc='vim ~/.bashrc'

        # runs the provided argument in the background, redirecting out/err to nohup.out
        # and stores the pid in pid.txt
        runbg () { nohup "$@" > nohup.out 2>&1 & echo $! >> pid.txt; }

        if [ -f .env ]; then
          export $(echo $(cat .env | sed 's/#.*//g'| xargs) | envsubst)
        fi

        ;;
        *)
        # non-interactive shells
        ;;
esac
