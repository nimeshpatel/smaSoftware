
	/* Now wait until the source has been reached.
	*/

	error_flag=RUNNING;

	sleep(1);


         rm_status=rm_read(antenna,
		"RM_SMARTS_COMMAND_STATUS_S",&error_flag);
       	 if(rm_status != RM_SUCCESS) {
       	 rm_error_message(rm_status,"rm_read()");
       	 exit(1);
       	 }

	if(error_flag==ERROR) 
	{
	printf("Error from track:\n");
	rm_status=rm_read(antenna,
		"RM_TRACK_MESSAGE_C100",messg);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
        }
	printf("%s\n",messg);
	}

	if(wait)
	{
		while(error_flag==RUNNING)
		{
          	 rm_status=rm_read(antenna,
			"RM_SMARTS_COMMAND_STATUS_S",&error_flag);
       		 if(rm_status != RM_SUCCESS) {
       	         rm_error_message(rm_status,"rm_read()");
       	         exit(1);
       		 }
		sleep(1);
		}
	if(error_flag==OK) printf("antenna has reached the source.\n");
	}
