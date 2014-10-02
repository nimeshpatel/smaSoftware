#include <stdio.h>
#include "SpiceUsr.h"
extern double tjd2et(double tjd);
void jplint(double *tjd, long int *targ,long int *cent,double *posvel)
{

   #define     FILE_SIZE 128
   #define     WORD_SIZE 80
	
	int i;
	extern int first_time_spk;

   SpiceDouble state[6];
   SpiceDouble lt;
   SpiceDouble Et;

   SpiceChar   leap[FILE_SIZE];
   SpiceChar   spk [FILE_SIZE];
   SpiceChar   spk1 [FILE_SIZE];
   SpiceChar   spk2 [FILE_SIZE];
   SpiceChar   spk3 [FILE_SIZE];
   SpiceChar   spk4 [FILE_SIZE];
   SpiceChar   spk5 [FILE_SIZE];
   SpiceChar   spk6 [FILE_SIZE];
   SpiceChar   spk7 [FILE_SIZE];
   SpiceChar   time[WORD_SIZE];
   SpiceChar   starg[WORD_SIZE];
   SpiceChar   obs [WORD_SIZE];

strcpy(leap,"/common/catalogs/time.ker");
strcpy(spk1,"/common/catalogs/de405.bsp");
strcpy(spk2,"/common/catalogs/sat077-95-05.bsp");
strcpy(spk3,"/common/catalogs/jupiter2004-2006.bsp");
strcpy(spk4,"/common/catalogs/neptune.bsp");
strcpy(spk5,"/common/catalogs/asteroids_2004_2006.bsp");
strcpy(spk6,"/common/catalogs/comets2004.bsp");
strcpy(spk7,"/common/catalogs/comettest.bsp");
if(*targ==375) *targ=2000001;
if(*targ==376) *targ=2000002;
if(*targ==377) *targ=2000010;
if(*targ==378) *targ=2000004;
if(*targ==379) *targ=1000351;
if(*targ==380) *targ=1000418;
if(*targ==381) *targ=1000322;

sprintf(starg,"%d",*targ);
sprintf(obs,"%d",*cent); 

if(first_time_spk)
{
   furnsh_c ( leap );
   furnsh_c ( spk1  );
   furnsh_c ( spk2  );
   furnsh_c ( spk3  );
   furnsh_c ( spk4  );
   furnsh_c ( spk5  );
   furnsh_c ( spk6  );
   furnsh_c ( spk7  );
first_time_spk=0;
}
            Et = tjd2et(*tjd);

   spkezr_c (  starg, Et, "J2000", "NONE", obs, state, &lt );

	for(i=0;i<6;i++) {
	posvel[i]=(double)state[i];
	}
}
