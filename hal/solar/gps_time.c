
    /* get the month of the year from the day number */
    {
    int i = 0;
    unsigned short md[12] =  {1, 32, 60, 91, 121, 152, 
	182, 213, 244, 274, 305, 335 };

    /* update the list for leapyears if necessary */
    if (time->years & 0x0003 == 0)
        for (i = 2; i < 12; i++) md[i]++;

    while (md[i] < time->days) i++;
    time->months = i; 
    }
