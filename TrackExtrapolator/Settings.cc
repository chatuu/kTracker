#include "Settings.hh"
#include "G4SystemOfUnits.hh"

#include "../MODE_SWITCH.h"

Settings::Settings()
{
  //  These are the default settings that will be used by the monte carlo if parameters are not specified.

  seed = 0;
  beamMomentum = 120*GeV;
  beamCurrent = 2e12;
  asciiFieldMap = true;
  generator = "gun";
  recordMethod = "hits";
  eventPos = "both";
  dimuonSource = "both";
  pythiaSource = "single";
  login = "seaguest";
  outputFileName = "test_default";
  password = "qqbar2mu+mu-";
  magnetPath = KTRACKER_ROOT
    "/TrackExtrapolator";
  fMagName = "tab.Fmag";
  kMagName = "tab.Kmag";
  sqlServer = MYSQL_SERVER_ADDR;
  dimuonRepeat = 1;
  ironOn = true;
#if defined ALIGNMENT_MODE
  kMagMultiplier = 0.;
  fMagMultiplier = 0.;
#elif defined MC_MODE
  kMagMultiplier = 0.;
  fMagMultiplier = 0.;
#else
  kMagMultiplier = 0.;
  fMagMultiplier = 0.;
#endif
  geometrySchema = GEOMETRY_VERSION;
  magnetSchema = "geometry_R996_magneticFields";
  target = 1;
  pythia_shower = true;
  bucket_size = 40000;
  mysqlPort = MYSQL_SERVER_PORT;     // This is for seaquel.physics.illinois.edu.  Most others would be 3306.
}

Settings::~Settings()
{
}
