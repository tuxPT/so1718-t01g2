#/usr/bin/bash

ipcrm --all=msg && ipcrm --all=shm && ipcrm --all=sem
