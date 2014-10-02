/*
snapshot.cpp is function instead of main version of:
	main.cpp - This file is a test of the CSBIGCam and CSBIGImage
			   classes.

	1. This software (c)2004 Santa Barbara Instrument Group.
	2. This free software is provided as an example of how 
	   to communicate with SBIG cameras.  It is provided AS-IS
	   without any guarantees by SBIG of suitability for a 
	   particular purpose and without any guarantee to be 
	   bug-free.  If you use it you agree to these terms and
	   agree to do so at your own risk.
    3. Any distribution of this source code to include these
	   terms.

	Revision History
	Date		Modification
	=========================================================
	1/26/04		Initial release
	11/9/04		Added compile time option for Ethernet Port camera 1
	1/17/05		Added taking and saving partial frame
				 on camera 1 and if compiled with FITSIO
				 saving FITS image

*/
#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>

using namespace std;

#include "csbigcam.h"
#include "csbigimg.h"

#define LINE_LEN 			80


int snapshot(double exp_time, char *astr_obj); 

int snapshot(double exp_time, char *astr_obj) {
	CSBIGCam *pCam = (CSBIGCam *)0;
	CSBIGImg *pImg1 = (CSBIGImg *)0;
//	CSBIGImg *pImg = (CSBIGImg *)0;
//	CSBIGImg *pImg1a = (CSBIGImg *)0;
//	CSBIGImg *pImg1b = (CSBIGImg *)0;
	PAR_ERROR err;
	SBIG_FILE_ERROR ferr;
//	char s[LINE_LEN];
        char datelabelut[50];
        char fitsfilename[100];
        char chmodcmd[100];
/*
        char astr_obj[100];
*/
        char fitsdate[50];
	string sPort1;
	int height, width;
        int i;
/*
        double exp_time;
*/
        time_t rawtime;
        struct tm * timeinfogmt;

/*
        exp_time=atof(argv[1]);
        strcpy(astr_obj,argv[2]);
*/

/*   Calculating time stamp in UTC */
     
        rawtime=time(NULL);
        timeinfogmt = gmtime ( &rawtime );
        strftime(datelabelut, 50, "%Y%m%d_%H%M%S", timeinfogmt);
        strftime(fitsdate, 50, "DATE-NEW = %Y%m%d_%H%M%S", timeinfogmt);


	do { // allow break out
		// Try to Establish a link to the camera

		sPort1 = "USB";
		cout << "Creating the SBIGCam Object on " << sPort1 << "..." << endl;
		pCam = new CSBIGCam(DEV_USB);

		if ( (err = pCam->GetError()) != CE_NO_ERROR )
			break;
		cout << "Establishing a Link to the " << sPort1 << " Camera..." << endl;
		if ( (err = pCam->EstablishLink()) != CE_NO_ERROR )
			break;
		cout << "Link Established to Camera Type: " << pCam->GetCameraTypeString() << endl;
		if ( (err = pCam->GetFullFrame(width, height)) != CE_NO_ERROR )
			break;
                
		cout << "Setting exposure time ..." << exp_time << endl;
                pCam->SetExposureTime(exp_time);
                cout << "Reading back exposure time:  " << pCam->GetExposureTime() << endl;


		// Take a full frame image
		pCam->SetSubFrame(0, 0, 0, 0);	// Restore Full Frame

                  cout << "--------------------------------------" << endl;

                                pImg1 = new CSBIGImg;
		if ( (err=pCam->GrabImage(pImg1, SBDF_DARK_ALSO)) != CE_NO_ERROR )
			break;
		pImg1->AutoBackgroundAndRange();
		cout << "Saving compressed image..." << endl;

		if ( (ferr = pImg1->SaveImage("img1.sbig", SBIF_COMPRESSED)) != SBFE_NO_ERROR )
			break;
cout << "ferr on saving compressed image = " << ferr << endl;


#if INCLUDE_FITSIO

		cout << "Adding object name to FITS file:  " << astr_obj << endl;
                pImg1->SetFITSObject(astr_obj);
                cout << "Reading back object name:  " << pImg1->GetFITSObject() << endl;

		cout << "Adding date to FITS file:  " << fitsdate << endl;
                pImg1->SetAdditionalFITSKeys(fitsdate);
                cout << "Reading back date:  " << pImg1->GetAdditionalFITSKeys() << endl;

//		pImg1->HorizontalFlip();
		pImg1->VerticalFlip();
		cout << "Saving FITS dark-subtracted image..." << endl;
                sprintf(fitsfilename, "/opticalPointing/ant1/%s_%s.fits", astr_obj, datelabelut);
		if ( (ferr = pImg1->SaveImage(fitsfilename, SBIF_FITS)) != SBFE_NO_ERROR )
			break;
                sprintf(chmodcmd,"chmod 777 %s",fitsfilename);
                system(chmodcmd);
#endif


//cout << "ferr on writing fits file = " << ferr << endl;

		// shut down camera
		cout << "Closing Devices..." << endl;
		if ( (err = pCam->CloseDevice()) != CE_NO_ERROR )
			break;
		cout << "Closing Drivers..." << endl;
		if ( (err = pCam->CloseDriver()) != CE_NO_ERROR )
			break;		
	} while (0);
	if ( err != CE_NO_ERROR )
		cout << "Camera Error: " << pCam->GetErrorString(err) << endl;
	else if ( ferr != SBFE_NO_ERROR )
	//	cout << "File Error: " << ferr << endl;
		cout << "File Error: " << pImg1->GetFileErrorString(ferr) << endl;
	else
		cout << "SUCCESS" << endl;
//	cout << "Hit any key to continue:";
//	cin.getline(s, LINE_LEN);;
	delete pCam;
	delete pImg1;
	return EXIT_SUCCESS;
}
