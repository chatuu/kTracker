#ifndef _MODE_SWITCH_H
#define _MODE_SWITCH_H

//==== Enable massive debugging output, disabled by default
//todo a simple msg service
//#define _DEBUG_ON
//#define _DEBUG_ON_LEVEL_2

//---------------------FOLLOWING PART SHOULD NOT BE CHANGED FOR NO GOOD REASON -----------------------
//--------------- Fast tracking configuration ----
#define TX_MAX 0.1
#define TY_MAX 0.12
#define X0_MAX 80.
#define Y0_MAX 150.
#define INVP_MIN 0.01
#define INVP_MAX 0.2
#define PROB_LOOSE 0.0
#define PROB_TIGHT 0.001
#define HIT_REJECT 3.

//--------------- Geometry setup -----------------
#define nChamberPlanes 24
#define nHodoPlanes 16
#define nPropPlanes 8

#define Z_KMAG_BEND 1064.26
#define Z_FMAG_BEND 251.4
#define Z_KFMAG_BEND 375.
#define PT_KICK_FMAG 2.909
#define PT_KICK_KMAG 0.4016
#define ELOSS_KFMAG 8.12
#define ELOSS_ABSORBER 1.81
#define Z_ST2 1347.36
#define Z_ABSORBER 2028.19
#define Z_REF 0.
#define Z_TARGET -129.54 
#define Z_DUMP 40.
#define RESOLUTION_DC 0.1

#define BEAM_SPOT_X 0.5
#define BEAM_SPOT_Y 0.5

//-------------- Coarse swim setup --------------
#define FMAG_HOLE_LENGTH 27.94
#define FMAG_HOLE_RADIUS 1.27
#define FMAG_LENGTH 502.92
#define NSLICES_FMAG 100
#define NSTEPS_TARGET 200
#define ELOSS_CORR 0.95
#define ELOSS_FMAG 7.3971
#define ELOSS_FMAG_RAD 0.0078
#define Z_UPSTREAM -500.
#define Z_DOWNSTREAM 500.

//-------------- Useful marcros -----------------
#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl
#define varName(x) #x

#endif
