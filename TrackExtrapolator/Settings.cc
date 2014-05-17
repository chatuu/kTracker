#include "Settings.hh"

#include "G4SystemOfUnits.hh"

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
  magnetPath = "/Users/liuk/currentWork/kTracker_dev/TrackExtrapolator";
  fMagName = "tab.Fmag";
  kMagName = "tab.Kmag";
  sqlServer = "localhost";
  dimuonRepeat = 1;
  ironOn = true;
  kMagMultiplier = 1;
  fMagMultiplier = 1;
  geometrySchema = "geometry_G3_run2";
  magnetSchema = "geometry_R996_magneticFields";
  target = 1;
  pythia_shower = true;
  bucket_size = 40000;
  mysqlPort = 3306;     // This is for seaquel.physics.illinois.edu.  Most others would be 3306.
}

Settings::~Settings()
{
}
