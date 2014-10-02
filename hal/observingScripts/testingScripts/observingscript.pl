#!/usr/bin/perl
while()
{
    `/application/bin/observe -s 1153+495`;
    sleep(3);
    `tsys`;
    `/application/bin/integrate -s 10 -w`;
	
    `/application/bin/observe -s Lock_102 -r 10:40:50.637 -d 56:06:53.84 -e 2000 -v 0`;
    sleep(3);
    `tsys`;
    `/application/bin/integrate -s 40 -w`;
	
    `/application/bin/observe -s 1153+495`;
    sleep(3);
    `tsys`;
    `/application/bin/integrate -s 10 -w`;
	
    `/application/bin/observe -s Lock_102 -r 10:40:50.637 -d 56:06:53.84 -e 2000 -v 0`;
    sleep(3);
    `tsys`;
    `/application/bin/integrate -s 40 -w`;
	
    `/application/bin/observe -s 0958+655`;
    sleep(3);
    `tsys`;
    `/application/bin/integrate -s 10 -w`;
}
