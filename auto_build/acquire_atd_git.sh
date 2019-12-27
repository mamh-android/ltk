#!/usr/bin/expect

set GIT_USER [lindex $argv 0]
set GIT_SERVER [lindex $argv 1]
set GIT_REPO [lindex $argv 2]
set GIT_PASSWORD [lindex $argv 3]

log_file ATD_testsuite_git.log

set timeout 600
#git clone ssh://sevensun@shgit.marvell.com/git/ose/linux/remote_control
spawn git clone ssh://$GIT_USER@$GIT_SERVER/$GIT_REPO
expect "password:"
send $GIT_PASSWORD
send "\r"

expect eof
exit
