/* This code is for calibrating the antenna temperature accurately.
takes into account:

(0) errors will occur if the measurement frequency is near an atmospheric
    absorption band: takes no account of frequency dependent between gain 
    differences between USB and LSB assumes sky, hot and ambient 
    beam terminations.
(1) The case Tatm != Thot != Tamb is correctly handled
(2) CMBR correction done, given the size of the object.
(3) forward coupling and spill-over efficiency eta_l taken into account
(4) source coupling efficiency taken into account
(4) exact treatment of Planck's function; no R-J

inputs needed: Tatm, Thot, Tamb, eta_l, tau_zenith, el, fwhm_beam, PlanetDia,
delVsource, Vhot, Vsky.

It is assumed that tau is at Tatm and eta_l is at Tamb.


02/Oct/98 TK original version
11/Jan/01 modified
12/Jan/01 more changes made to input and output formats; now all the essential information
          is recorded in the output.
14/Feb/01 include antenna number
06/Sep/01 Masao scanf and make it neat
07 Oct 04 TK: exact Planck; no R-J
07 Oct 04 TK: more useful comments added
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>

#define Area    282743.3   /*    6-m area cm^2  */
float RatioIn,x,F,f,fwhm_beam;

void main(int argc, char *argv[])

{
        FILE *fpi,*fpo,*fpo2, *fpo3;
        int count_x,i,j,cout,n_times, use_beam, ant_num;
        float deg2rad,tau_zenith,Tcmbr,Tatm,Thot,Tamb,eta_l,el,delVsource,Vhot,Vsky;
	float Jcmbr,Jatm,Jhot,Jamb,Jbrt,Jsource,Jcal;
        float ratio,tau,Tcal,Tsource,Tsource_rj,err,X0;
        float Frequency, TBright, AntennaTemp;
        float PlanetDia,BeamSize, WidthFwhm;
        float Expo, Omega, OmegaS, Lambda;
        float RatioIn, EtaA, EtaB;
        char  object[20], date[256], in_line[1024];

        deg2rad = 4*atan(1.0)/180.0;
        Tcmbr=2.7;
        if ( argc < 2) {
                printf("usage: eta filename  \n");
                exit(1);
        }
        fpi = fopen(argv[1], "r");
        fpo = fopen("summary.dat", "w");
        fpo2 = fopen("results.dat", "w");
        fpo3 = fopen("aperResults.tmp", "w");
        printf ("date-time   ant obj  Frq.  dia  T_B   scnw   Vhot   Vsky  dVsrc Fcor Elev Thot   Tamb  Tatm taul tauz taue  Tcal Tsrc bu bmin EtaA EtaB FWHM\n\
d m y   h m          GHz   \"    K     \"      mV     mV    mV         deg  K      K     K          nep  nep   K    K       \"               \"\n"); 
        fprintf (fpo2,"date-time   ant obj  Frq.  dia  T_B   scnw   Vhot   Vsky  dVsrc Fcor Elev Thot   Tamb  Tatm taul tauz taue  Tcal Tsrc bu bmin EtaA EtaB FWHM\n\
d m y   h m          GHz   \"    K     \"      mV     mV    mV         deg  K      K     K          nep  nep   K    K       \"               \"\n"); 
        fprintf (fpo,"date-time   ant obj  Frq.    scnw err  EtaA  EtaB FWHM\n"); 

	fseek(fpi,0L,0);
        while (feof(fpi)==0){
		fgets(in_line,1024,fpi);
                if (strncmp(in_line,"#",1) != 0){

                sscanf(in_line,"%s %d %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", date, &ant_num, object, &el,&tau_zenith, &Thot,&Tamb,&Tatm,&eta_l,&Vhot,&Vsky,&delVsource,&fwhm_beam,&Frequency, &PlanetDia,&WidthFwhm,&TBright,&err,&use_beam);
                printf("%s %d %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", date, ant_num, object, el,tau_zenith, Thot,Tamb,Tatm,eta_l,Vhot,Vsky,delVsource,fwhm_beam,Frequency, PlanetDia,WidthFwhm,TBright,err,use_beam);
		fwhm_beam=use_beam;
		if (use_beam==0){ /* use theoretical beam size if you want */
			fwhm_beam=1.15*(299.8/Frequency)/6e3*180/M_PI*3600;
		}
		if (use_beam==1){ /*calculate deconvolved beam size and use it */
			fwhm_beam=pow ( (pow(WidthFwhm,2.0)-pow(PlanetDia,2.0)), 0.5 );
		}
		/* A better scheme to deconvolve the source size should eventually be used here */
		/* much non-R-J action below! */
		Jcmbr=1.0/(exp(0.048*Frequency/Tcmbr)-1);
		Jhot=1.0/(exp(0.048*Frequency/Thot)-1);
		Jatm=1.0/(exp(0.048*Frequency/Tatm)-1);
		Jamb=1.0/(exp(0.048*Frequency/Tamb)-1);
		/* If the source brightness temperature is the correct Planck Blackbody Kelvin 
		   the expression in the following line should be used */
		/*
		Jbrt=1.0/(exp(0.048*Frequency/TBright)-1);
		*/
		/* In the following line, R-J brightness temperature definition is assumed; 
		   but not R-J approximation */
		Jbrt=TBright/(0.048*Frequency);
                tau = tau_zenith/cos((90.0-el)*deg2rad);
		/* F is the correction factor for the beam-profile and source size; the "source coupling efficiency" */
		/* could in principle incorporate specific source structure etc. */
		X0 = (PlanetDia/fwhm_beam)*(PlanetDia/fwhm_beam)*log(2.0);
		/* correction factor for beam profile */
		F = (1-exp(-X0))/X0;
		/* correction factor for CMBR contribution */
		f = exp(-X0);
                /* F = 1 - exp (-1*(PlanetDia/fwhm_beam)*(PlanetDia/fwhm_beam)*log(2.0)); */
                Jcal = Jhot*( exp(tau)*( 1-eta_l*Jatm/Jhot-(1-eta_l)*Jamb/Jhot ) + eta_l*(Jatm-Jcmbr)/Jhot )/eta_l;
		/* ratio gives the source strength when scaled by Tcal=Thot=Tatm=Tamb in the simple 0th order approach */
	        /* so-called chopper-wheel calibration of Penzias & Burrus fame */
		/* all the corrections are incorporated into a modified Tcal or Jcal in the non-R-J case */
                ratio = delVsource/(Vhot-Vsky);
                Jsource = ratio*Jcal - (f-1)*Jcmbr;
		Tsource = 0.048*Frequency/log((double)(1.0/Jsource+1.0));
		Tsource_rj = 0.048*Frequency*Jsource;
		Tcal = 0.048*Frequency/log((double)(1.0/Jcal+1.0));
	        printf("%f %f %f %f %f %f %f %f\n", Jcmbr,Jhot,Jatm,Jamb,Jcal,Jsource,Tsource,Tsource_rj);
                printf ("%s %1d %s %5.1f %4.1f %5.1f %4.1f %6.1f %6.1f %6.1f %4.2f %4.1f %5.1f %5.1f %5.1f %4.2f %4.2f %4.2f %5.1f %5.1f %1d %4.1f ", date,ant_num, object, Frequency, PlanetDia, TBright, WidthFwhm, Vhot, Vsky, delVsource, F, el, Thot, Tamb, Tatm, eta_l, tau_zenith, tau, Tcal, Tsource, use_beam,fwhm_beam);
                fprintf (fpo2,"%s %1d %s %5.1f %4.1f %5.1f %4.1f %6.1f %6.1f %6.1f %4.2f %4.1f %5.1f %5.1f %5.1f %4.2f %4.2f %4.2f %5.1f %5.1f %1d %4.1f ", date,ant_num, object, Frequency, PlanetDia, TBright, WidthFwhm, Vhot, Vsky, delVsource, F, el, Thot, Tamb, Tatm, eta_l, tau_zenith, tau, Tcal, Tsource, use_beam,fwhm_beam);
                fprintf (fpo,"%s %1d %s %5.1f %5.1f +- %4.1f ", date,ant_num, object, Frequency, WidthFwhm, err);
		AntennaTemp=Tsource;
/* Calculate aperture efficiency */
                Frequency *= 1.e9;
                Lambda = 3.e10/Frequency;
		OmegaS = 3.1415*pow((PlanetDia/2.0),2.0)*2.35e-11;
		EtaA = Tsource_rj*Lambda*Lambda/(TBright*F*Area*OmegaS);
		EtaB = Tsource_rj/(TBright*F);
/*                printf("%3.2f %3.2f %3.2f %3.2f \n",F,f,EtaA,EtaB);
*/
                printf("%3.2f %3.2f %4.1f\n",EtaA,EtaB,fwhm_beam);
                fprintf(fpo2,"%3.2f %3.2f %4.1f\n",EtaA,EtaB,fwhm_beam);
                fprintf(fpo3,"%3.2f %3.2f %4.1f\n",EtaA,EtaB,fwhm_beam);
                fprintf(fpo,"%3.2f %3.2f %4.1f\n",EtaA,EtaB,fwhm_beam);
        }
}
}
