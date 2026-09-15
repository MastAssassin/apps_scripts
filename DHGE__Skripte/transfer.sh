#!/bin/bash

sudo mount -t cifs //192.168.71.100/Transfer /mnt/share -o username=<username>,uid=1000,gid=1000

#replace <username> with your dhge-name  e.g.  mustma.pi24 
#I don't know if the name is alreade given  must --> Mustermann ma --> Max
#if you lower the ids you maybe won't be able write and execute, so just set your name and everthing will work 