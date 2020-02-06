#!/bin/bash
source ~/.bash_profile
gcc test_server.c ../Util/message.c -o server
gcc node_cron_job.c ../Util/message.c ../Util/util.c ../NodeInfo/node_info.c ../NodeInfo/node_pair.c ../NodeInfo/table.c -o cronjob