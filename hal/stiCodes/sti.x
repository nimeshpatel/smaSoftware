struct commandStatus {
    int status;	
    char errorMessage[128];
};

struct commandRequest {
    double exposureSec; 
    char fileNamePrefix[80];
};

/* Here comes the server program definition */

program STIPROG {
    version STIVERS {
	commandStatus   STICOMMANDREQUEST(commandRequest)  = 0;
    } = 1;      /* Program version    */
} = 0x20000111; /* RPC Program number */
