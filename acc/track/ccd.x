
struct ccdCommand 
{
short int acquireFlag;
short int ccdResponse;
};

struct antenaQuery {
	int antenaNumber;
};

const SUCCESS = 1;
const FAILURE = 0;

program CCDCOMMANDPROG {
version CCDCOMMANDVERS {
 ccdCommand CCDCOMMAND(ccdCommand) = 1;
 } = 1;
} = 0x20000202;
