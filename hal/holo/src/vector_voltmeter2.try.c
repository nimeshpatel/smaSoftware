/* talk to two vector voltmeters (instead of only 1) */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "dsm.h"
#include "vector_voltmeter2.h"
#include "constants.h" /* must come before callingStubs.h */
#include "rxStructures.h"
#include "utilities.h"
#include "callingStubs.h"
#include "vxi11core.h" 
#include "call_gpib.h" /* must come after callingStubs.h  and vxi11core.h */

int continueVVMlogging;
void cleanupControlCvvm(int signal);

#if USE_AS_MAIN
  #include "help.h"
  #if __linux__ && NIGPIB
    #include <ugpib.h> 
  #endif

  char rdBuf[TTYLEN], cmdBuf[CMDLEN];
  COMMAND cmds[] = {
  #include "vector_voltmeter2_cmds6.h"
  #include "call_gpib_cmds6.h"
  };
  #define NUMCMDS (sizeof(cmds) / sizeof(COMMAND))
  int numberCommands = NUMCMDS;

  /* Definitions which should be in readline/readline.h, but aren't in LynxOS */
  extern void readline_initialize_everything(void);
  extern char *readline(char *);
  extern void add_history(char *);
  extern int read_history(char *);
  extern int write_history(char *);
  #define HIST_FILE "./.vector_voltmeter_history"

  void q(char *ip) {
    call_ibloc(vector_voltmeter.dd);
    write_history(HIST_FILE);
    exit(0);
  }

#define MAXGPIB 31
CONFIG config;
GPIB_DEVICE vector_voltmeter2;

#if !NAMM
void vvUsage(char *name) {
  printf("Usage: %s -a <gpibAddress:default=%d> -b <gpibAddress:default=%d>-d -h\n",
         name,vector_voltmeter.address,vector_voltmeter2.address);
  printf("   -d: turn on GPIB debugging statements\n");
  printf("   -h: print this message\n");
  exit(0);
}

int whichVVM;
int whichVVMdd;
int addressVVM(int dd);

int main(int argc, char *argv[]) {
    char *ip, *cp;
    char ttybuf[TTYLEN], cmd[CMDLEN];
    COMMAND *cmdp;
    int i;
    
    config.gpibPresent = 1;
    gpibDebug = 0;
    vector_voltmeter.address = HP8508;
    vector_voltmeter2.address = HP8508+1;
    for (i=1; i<argc; i++) {
      if (present(argv[i],"-a")) {
        if (++i < argc) {
          sscanf(argv[i],"%d",&vector_voltmeter.address);
	} else {
          vvUsage(argv[0]);
	}
      }
      if (present(argv[i],"-b")) {
        if (++i < argc) {
          sscanf(argv[i],"%d",&vector_voltmeter2.address);
	} else {
          vvUsage(argv[0]);
	}
      }
      if (present(argv[i],"-h")) {
        vvUsage(argv[0]);
      }
      if (present(argv[i],"-d")) {
        gpibDebug = 1;
      }
    }
    readline_initialize_everything();
    read_history(HIST_FILE);
    vector_voltmeter.present = 1;
    vector_voltmeter2.present = 1;
    /* find controller and setup the GPIB bus */
#if __Lynx__
    brd0 = call_ibfind(GPIBDEVICE);
  #if !__powerpc__
    call_ibsic(brd0);
    call_ibsre(brd0,1);
  #endif
#endif

    /* open up the device */
    WARN("opening GPIB device at address = %d....\n",
            vector_voltmeter.address);
    vector_voltmeter.dd = setupGPIBdevice(vector_voltmeter.address);
    if (vector_voltmeter.dd < 0) {
      WARN("Failed\n");
      exit(0);
    } else {
      WARN("Succeeded.  Descriptor = %d\n",vector_voltmeter.dd);
      clearVectorVoltmeter(vector_voltmeter.dd);
    }
    WARN("opening GPIB device at address = %d....\n",
            vector_voltmeter2.address);
    vector_voltmeter2.dd = setupGPIBdevice(vector_voltmeter2.address);
    if (vector_voltmeter2.dd < 0) {
      WARN("Failed\n");
      exit(0);
    } else {
      WARN("Succeeded.  Descriptor = %d\n",vector_voltmeter2.dd);
      clearVectorVoltmeter(vector_voltmeter2.dd);
    }
    whichVVMdd = vector_voltmeter.dd;
    whichVVM = 1;

    /* Now the main program is a loop, reading and executing commands */
    for (;;) {
            sprintf(ttybuf,"vectorVoltmeter%d@addr%d> ",whichVVM,
		    addressVVM(whichVVMdd));
            ip = readline(ttybuf);
            add_history(ip);
            if (ip[0] == '!') {
              system(++ip);
              continue;
            }
	    while((*ip == ' ' || *ip == '\t') && *ip != '\n')
		ip++;
	    for(cp = cmd; cp < &cmd[CMDLEN]; cp++) {
		if((*cp = *ip++) == ' ' || *cp == ',' ||
		*cp == '\t' || *cp == '\n' || *cp == 0)
		    break;
	    }
	    *cp = 0;
	    if (cp >= &cmd[CMDLEN]) {
		WARN( "cmd %s too long\n", cmd);
		continue;
	    }
	    if (cmd[0] == 0)
		continue;

            /* 'ip' now contains only the arguments to the command */
	    for (cmdp = cmds; cmdp < &cmds[NUMCMDS]; cmdp++) {
               /* look up the command */
	       if(*cmd == *cmdp->cmd && strcmp(cmd + 1, cmdp->cmd + 1) == 0) {
	          cmdp->sub(ip);
	          break;
               }
	    } /* end of 'for' loop */
	    if (cmdp >= &cmds[NUMCMDS]) {
              WARN("Unrecognized cmd = %s\n", cmd);
	    }
#if __Lynx__
            free(ip);
#endif
	  } /* end of infinite 'for' command loop */
    call_ibloc(vector_voltmeter.dd);
    call_ibloc(vector_voltmeter2.dd);
    write_history(HIST_FILE);
    return(0);
}
#endif /* !NAMM */

#else
    extern char rdBuf[TTYLEN], cmdBuf[CMDLEN];
#endif /* use_as_main */

void CheckVectorLockBoth(char *ip) {
  int i;
  int originaldd = whichVVMdd;
  for (i=1; i<=2; i++) {
    whichVVM = i;
    if (i==1) {
      whichVVMdd = vector_voltmeter.dd;
    } else {
      whichVVMdd = vector_voltmeter2.dd;
    }
     switch (vectorQueryLock()) {
     case VLOCKED:
       WARN("Vector voltmeter %d lock state: Locked\n",i);
       break;
     case VUNLOCKED:
       WARN("Vector voltmeter %d lock state: Unlocked\n",i);
       break;
     case VLOCKUNKNOWN:
     default:
       WARN("Vector voltmeter %d lock state: Unknown\n",i);
     }
  }
  whichVVMdd = originaldd;
}

void CheckVectorLock(char *ip) {
     if (CheckVectorVoltmeterPresent()) return;
     switch (vectorQueryLock()) {
     case VLOCKED:
       WARN("Vector voltmeter lock state: Locked\n");
       break;
     case VUNLOCKED:
       WARN("Vector voltmeter lock state: Unlocked\n");
       break;
     case VLOCKUNKNOWN:
     default:
       WARN("Vector voltmeter lock state: Unknown\n");
     }
}

void ReadVectorVoltmeter(char *ip) {
  double phase, amplitude;

  if (CheckVectorVoltmeterPresent()) return;
  if (vectorQueryLock()) {
    phase = vectorPhase();
    amplitude = vectorAmplitude();
    WARN("Phase = %.3fdeg,  B/A = %.3fdB\n",phase,amplitude);
  } else {
    WARN("Sorry, the vector voltmeter is not locked.\n");
  }
}

void cleanupControlCvvm(int signal) {  
  continueVVMlogging = 0;
}

void vv2monitor(char *ip) {
#if __powerpc__
  int d;
  char dsmhost[NAMELEN];
  int originaldd, original;
  short lock1,lock2;
  struct sigaction action, oldAction;

  continueVVMlogging = 1;
  d = dsm_open();
  if (d) {
    dsm_error_message(d,"dsm_open");
    return;
  }
  original = whichVVM;
  originaldd = whichVVMdd;
  strcpy(dsmhost,"hal9000");
  /* install control-c handler */
  action.sa_flags = 0;
  action.sa_handler = cleanupControlCvvm;
  sigemptyset(&action.sa_mask);
  if (sigaction(SIGINT,  &action, &oldAction) != 0) { 
    STRERROR("sigaction:");
  } 
  WARN("Control-C handler installed. Hit Cntrl-C to exit the DSM logging mode.\n");

  do {
    whichVVM = 1;
    whichVVMdd = vector_voltmeter.dd;
    lock1 = vectorQueryLock();
    whichVVM = 2;
    whichVVMdd = vector_voltmeter2.dd;
    lock2 = vectorQueryLock();
    d = dsm_write(dsmhost,"DSM_VVM1_LOCK_STATUS_S",&lock1);
    if (d) {
      dsm_error_message(d,"dsm_write(DSM_VVM1_LOCK_STATUS_S)");
    }
    d=dsm_write(dsmhost,"DSM_VVM2_LOCK_STATUS_S",&lock2);
    if (d) {
      dsm_error_message(d,"dsm_write(DSM_VVM2_LOCK_STATUS_S)");
    }
    sleep(5);
  } while (continueVVMlogging == 1);
  d = dsm_close();
  WARN("restoring previous control-c handler (if any)\n");
  if (sigaction(SIGINT,  &oldAction, &action) != 0) { 
    STRERROR("sigaction:");
  } 
  whichVVM = original;
  whichVVMdd = originaldd;
#else
  WARN("This feature is not supported on linux\n");
#endif
}

double vectorPhase(void) {
          double phase;

 	  strcpy(cmdBuf,"MEAS? PHASE");
	  call_ibwrt(whichVVMdd, cmdBuf, strlen(cmdBuf));
	  call_ibrd(whichVVMdd, rdBuf, sizeof(rdBuf));
	  sscanf(rdBuf,"%lf",&phase);
          return(phase);
}

double vectorAmplitude(void) {
          double amplitude;

	  strcpy(cmdBuf,"MEAS? BA");
	  call_ibwrt(whichVVMdd, cmdBuf,strlen(cmdBuf));
	  call_ibrd(whichVVMdd, rdBuf,sizeof(rdBuf));
	  sscanf(rdBuf,"%lf",&amplitude);
          return(amplitude);
}

int vectorQueryLock(void) {
  int status;
  int locked, narg;
  int iterations = 3;
  int i;

  for (i=0; i<iterations; i++) {
    strcpy(cmdBuf,"STAT:OPER:COND?");
    call_ibwrt(whichVVMdd, cmdBuf,strlen(cmdBuf));
    call_ibrd(whichVVMdd, rdBuf,sizeof(rdBuf));
    narg = sscanf(rdBuf,"%d",&status);
    if (narg < 1) {
      locked = VLOCKUNKNOWN;
      break;
    } else {
      locked = VLOCKED;
      if (status & 0x1) {
	printf("Vector voltmeter %d calibrating\n",whichVVM);
      }
      if (status & 0x4) {
	printf("Vector voltmeter %d ranging (Unlocked)\n",whichVVM);
	locked = VUNLOCKED;
      }
      if (status & 0x8) {
	printf("Vector voltmeter %d measuring\n",whichVVM);
      }
      if (status & 16) {
	printf("Vector voltmeter %d awaiting trigger\n",whichVVM);
      }
      if (status != 0) {
	break;
      }
    }
  }
  return(locked);
}

void clearVectorVoltmeter(int dd) {
        call_ibwrt(dd,"*CLS",4);
}

void directAnalogOutput(char *ip) {
  int state;

  if (CheckVectorVoltmeterPresent()) return;
  if (present(ip,"on") || present(ip,"1")) {
    state = setDirectAnalogOutput(1);
  } else if (present(ip,"off") || present(ip,"0")) {
    state = setDirectAnalogOutput(0);
  } else {
    state = readDirectAnalogOutput();
  }
  switch (state) {
  case 1:
    printf("Direct Analog Output is enabled\n");
    break;
  case 0:
    printf("Direct Analog Output is NOT enabled\n");
    break;
  }
}

int readDirectAnalogOutput(void) {
  int state;
  
  strcpy(cmdBuf,"DAN?");
  call_ibwrt(whichVVMdd, cmdBuf,strlen(cmdBuf));
  call_ibrd(whichVVMdd, rdBuf,sizeof(rdBuf));
  sscanf(rdBuf,"%d",&state);
  return(state);
}

int setDirectAnalogOutput(int on) {
  sprintf(cmdBuf,"DAN %d",on);
  call_ibwrt(whichVVMdd, cmdBuf,strlen(cmdBuf));
  return(readDirectAnalogOutput());
}

void vectorVoltmeterKey(char *ip) {
  int key;
  if (CheckVectorVoltmeterPresent()) return;
  if (present(ip,"refl meas")) {
    key = 3;
  } else if (present(ip,"format")) {
    key = 4;
  } else if (present(ip,"power meas")) {
    key = 5;
  } else if (present(ip,"b/a mag")) {
    key = 6;
  } else if (present(ip,"b-a phase")) {
    key = 7;
  } else if (present(ip,"ref select")) {
    key = 8;
  } else if (present(ip,"ref")) {
    key = 9;
  } else if (present(ip,"system impd")) {
    key = 10;
  } else if (present(ip,"meter select")) {
    key = 11;
  } else if (present(ip,"lock range")) {
    key = 12;
  } else if (present(ip,"up")) {
    key = 13;
  } else if (present(ip,"down")) {
    key = 14;
  } else if (present(ip,"mag range")) {
    key = 15;
  } else if (present(ip,"hold value")) {
    key = 16;
  } else if (present(ip,"display")) {
    key = 17;
  } else if (present(ip,"shift")) {
    key = 18;
  } else if (present(ip,"lcl")) {
    key = 19;
  } else if (present(ip,"preset")) {
    key = 20;
  } else if (present(ip,"a")) {
    key = 1;
  } else if (present(ip,"b")) {
    key = 2;
  } else {
    printf("Unknown key. Available keys:\n");
    printf("up, down, mag range, value, display shift, lcl, preset\n");
    printf("a, b, refl meas, format, power meas, b/a mag, b-a phase,\n");
    printf("ref select, ref, system impd, meter select, lock range, \n");
    return;
  }
  sprintf(cmdBuf,"SYST:KEY %d",key);
  call_ibwrt(whichVVMdd, cmdBuf, strlen(cmdBuf));
}

void selectVVM(char *ip) {
  if (present(ip,"1")) {
    whichVVMdd = vector_voltmeter.dd;
    whichVVM = 1;
  } else if (present(ip,"2")) {
    whichVVMdd = vector_voltmeter2.dd;
    whichVVM = 2;
  } else {
    printf("Usage: vv <1 or 2>\n");
  }
}

void selectVVM1(char *ip) {
    whichVVMdd = vector_voltmeter.dd;
    whichVVM = 1;
}

void selectVVM2(char *ip) {
    whichVVMdd = vector_voltmeter2.dd;
    whichVVM = 2;
}

int CheckVectorVoltmeterPresent(void) {
     if (!vector_voltmeter.present) {
       WARN("No vector voltmeter present.  See configure command\n");
       return(1);
     }
     return(0);
}

int addressVVM(int dd) {
  if (dd == vector_voltmeter.dd) {
    return(vector_voltmeter.address);
  }
  if (dd == vector_voltmeter2.dd) {
    return(vector_voltmeter2.address);
  }
  return(0);
}
