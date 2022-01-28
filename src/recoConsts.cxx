#include "recoConsts.h"

#include <iostream>
#include <wordexp.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

const string PHFlag::get_CharFlag(const string &flag) const
{
  map<string, string>::const_iterator iter = charflag.find(flag);
  if (iter != charflag.end())
  {
    return iter->second.Data();
  }
  cout << "PHFlag::getString: ERROR Unknown character Flag " << flag
       << ", The following are implemented: " << endl;
  Print();
  exit(EXIT_FAILURE);
  return "None";
}

const string
PHFlag::get_CharFlag(const string &flag, const string &defaultval)
{
  map<string, string>::const_iterator iter = charflag.find(flag);
  if (iter != charflag.end())
  {
    return iter->second;
  }
  else
  {
    set_CharFlag(flag, defaultval);
    return get_CharFlag(flag);
  }
}

void PHFlag::set_CharFlag(const string &flag, const string &charstr)
{
  charflag[flag] = charstr;
  return;
}

double PHFlag::get_DoubleFlag(const string &name) const
{
  map<string, double>::const_iterator iter = doubleflag.find(name);
  if (iter != doubleflag.end())
  {
    return iter->second;
  }
  cout << "PHFlag::getFlag: ERROR Unknown Double Flag " << name
       << ", The following are implemented: " << endl;
  Print();
  exit(EXIT_FAILURE);
  return 0.0;
}

double PHFlag::get_DoubleFlag(const string &name, const double defaultval)
{
  map<string, double>::const_iterator iter = doubleflag.find(name);
  if (iter != doubleflag.end())
  {
    return iter->second;
  }
  else
  {
    set_DoubleFlag(name, defaultval);
    return get_DoubleFlag(name);
  }
}

void PHFlag::set_DoubleFlag(const string &name, const double iflag)
{
  doubleflag[name] = iflag;
  return;
}

float PHFlag::get_FloatFlag(const string &name) const
{
  map<string, float>::const_iterator iter = floatflag.find(name);
  if (iter != floatflag.end())
  {
    return iter->second;
  }
  cout << "PHFlag::getFlag: ERROR Unknown Float Flag " << name
       << ", The following are implemented: " << endl;
  Print();
  exit(EXIT_FAILURE);
  return 0.0;
}

float PHFlag::get_FloatFlag(const string &name, const float defaultval)
{
  map<string, float>::const_iterator iter = floatflag.find(name);
  if (iter != floatflag.end())
  {
    return iter->second;
  }
  else
  {
    set_FloatFlag(name, defaultval);
    return get_FloatFlag(name);
  }
}

void PHFlag::set_FloatFlag(const string &name, const float iflag)
{
  floatflag[name] = iflag;
  return;
}

int PHFlag::get_IntFlag(const string &name) const
{
  map<string, int>::const_iterator iter = intflag.find(name);
  if (iter != intflag.end())
  {
    return iter->second;
  }
  cout << "PHFlag::getFlag: ERROR Unknown Int Flag " << name
       << ", The following are implemented: " << endl;
  Print();
  exit(EXIT_FAILURE);
  return 0;
}

int PHFlag::get_IntFlag(const string &name, int defaultval)
{
  map<string, int>::const_iterator iter = intflag.find(name);
  if (iter != intflag.end())
  {
    return iter->second;
  }
  else
  {
    set_IntFlag(name, defaultval);
    return get_IntFlag(name);
  }
}

void PHFlag::set_IntFlag(const string &name, const int iflag)
{
  intflag[name] = iflag;
  return;
}

bool PHFlag::get_BoolFlag(const string &name) const
{
  map<string, bool>::const_iterator iter = boolflag.find(name);
  if (iter != boolflag.end())
  {
    return iter->second;
  }
  cout << "PHFlag::getFlag: ERROR Unknown Bool Flag " << name
       << ", The following are implemented: " << endl;
  Print();
  exit(EXIT_FAILURE);
  return false;
}

bool PHFlag::get_BoolFlag(const string &name, bool defaultval)
{
  map<string, bool>::const_iterator iter = boolflag.find(name);
  if (iter != boolflag.end())
  {
    return iter->second;
  }
  else
  {
    set_BoolFlag(name, defaultval);
    return get_BoolFlag(name);
  }
}

void PHFlag::set_BoolFlag(const string &name, const bool bflag)
{
  boolflag[name] = bflag;
  return;
}

void PHFlag::Print() const
{
  PrintIntFlags();
  PrintFloatFlags();
  PrintDoubleFlags();
  PrintCharFlags();
  PrintBoolFlags();
  return;
}

void PHFlag::PrintIntFlags() const
{
  // loop over the map and print out the content (name and location in memory)
  cout << endl
       << "Integer Flags:" << endl;
  map<string, int>::const_iterator intiter;
  for (intiter = intflag.begin(); intiter != intflag.end(); ++intiter)
  {
    cout << intiter->first << " is " << intiter->second << endl;
  }
  return;
}

void PHFlag::PrintDoubleFlags() const
{
  // loop over the map and print out the content (name and location in memory)
  cout << endl
       << "Double Flags:" << endl;
  map<string, double>::const_iterator doubleiter;
  for (doubleiter = doubleflag.begin(); doubleiter != doubleflag.end(); ++doubleiter)
  {
    cout << doubleiter->first << " is " << doubleiter->second << endl;
  }
  return;
}

void PHFlag::PrintFloatFlags() const
{
  // loop over the map and print out the content (name and location in memory)
  cout << endl
       << "Float Flags:" << endl;
  map<string, float>::const_iterator floatiter;
  for (floatiter = floatflag.begin(); floatiter != floatflag.end(); ++floatiter)
  {
    cout << floatiter->first << " is " << floatiter->second << endl;
  }
  return;
}

void PHFlag::PrintCharFlags() const
{
  // loop over the map and print out the content (name and location in memory)
  cout << endl
       << "char* Flags:" << endl;
  map<string, string>::const_iterator chariter;
  for (chariter = charflag.begin(); chariter != charflag.end(); ++chariter)
  {
    cout << chariter->first << " is " << chariter->second << endl;
  }
  return;
}

void PHFlag::PrintBoolFlags() const
{
  // loop over the map and print out the content (name and location in memory)
  cout << endl
       << "Boolean Flags:" << endl;
  map<string, bool>::const_iterator booliter;
  for (booliter = boolflag.begin(); booliter != boolflag.end(); ++booliter)
  {
    cout << booliter->first << " is " << (booliter->second ? "TRUE" : "FALSE") << endl;
  }
  return;
}

int PHFlag::FlagExist(const string &name) const
{
  map<string, int>::const_iterator iter = intflag.find(name);
  if (iter != intflag.end())
  {
    return 1;
  }
  map<string, float>::const_iterator fiter = floatflag.find(name);
  if (fiter != floatflag.end())
  {
    return 1;
  }
  map<string, double>::const_iterator diter = doubleflag.find(name);
  if (diter != doubleflag.end())
  {
    return 1;
  }
  map<string, TString>::const_iterator citer = charflag.find(name);
  if (citer != charflag.end())
  {
    return 1;
  }
  map<string, bool>::const_iterator biter = boolflag.find(name);
  if (biter != boolflag.end())
  {
    return 1;
  }
  return 0;
}

void PHFlag::ReadFromFile(const string &name, bool verbose)
{
  string label;
  float fvalue;
  int fvaluecount = 0;
  double dvalue;
  int dvaluecount = 0;
  int ivalue;
  int ivaluecount = 0;
  string cvalue;
  int cvaluecount = 0;
  bool bvalue;
  int bvaluecount = 0;
  string junk;
  int junkcount = 0;

  ifstream infile(name.c_str());
  while (infile >> label)
  {
    if (verbose)
      cout << "Label " << label;
    if (label.substr(0, 1) == "C")
    {
      infile >> cvalue;
      cvaluecount++;
      set_CharFlag(label.substr(1, label.size() - 1), cvalue);
      if (verbose)
        cout << " C read " << cvalue << endl;
    }
    else if (label.substr(0, 1) == "F")
    {
      infile >> fvalue;
      fvaluecount++;
      set_FloatFlag(label.substr(1, label.size() - 1), fvalue);
      if (verbose)
        cout << " F read " << fvalue << endl;
    }
    else if (label.substr(0, 1) == "D")
    {
      infile >> dvalue;
      dvaluecount++;
      set_DoubleFlag(label.substr(1, label.size() - 1), dvalue);
      if (verbose)
        cout << " D read " << dvalue << endl;
    }
    else if (label.substr(0, 1) == "I")
    {
      infile >> ivalue;
      ivaluecount++;
      set_IntFlag(label.substr(1, label.size() - 1), ivalue);
      if (verbose)
        cout << " I read " << ivalue << endl;
    }
    else if (label.substr(0, 1) == "B")
    {
      infile >> bvalue;
      bvaluecount++;
      set_BoolFlag(label.substr(1, label.size() - 1), bvalue);
      if (verbose)
        cout << " B read " << ivalue << endl;
    }
    else
    {
      infile >> junk;
      junkcount++;
      if (verbose)
        cout << " Junk read " << junk << endl;
    }
  }

  cout << "Read CharFlags(" << cvaluecount
       << ") FloatFlags(" << fvaluecount
       << ") DoubleFlags(" << dvaluecount
       << ") IntFlags(" << ivaluecount
       << ") BoolFlags(" << bvaluecount
       << ") JunkEntries(" << junkcount
       << ") from file " << name << endl;

  infile.close();
}

void PHFlag::WriteToFile(const string &name)
{
  ofstream outFile(name.c_str());
  // loop over the map and write out the content
  map<string, int>::const_iterator intiter;
  for (intiter = intflag.begin(); intiter != intflag.end(); ++intiter)
  {
    outFile << "I\t" << intiter->first << "\t" << intiter->second << endl;
  }

  map<string, float>::const_iterator floatiter;
  for (floatiter = floatflag.begin(); floatiter != floatflag.end(); ++floatiter)
  {
    outFile << "F\t" << floatiter->first << "\t" << floatiter->second << endl;
  }

  int oldprecision = outFile.precision(15);
  map<string, double>::const_iterator doubleiter;
  for (doubleiter = doubleflag.begin(); doubleiter != doubleflag.end(); ++doubleiter)
  {
    outFile << "D\t" << doubleiter->first << "\t" << doubleiter->second << endl;
  }
  outFile.precision(oldprecision);

  map<string, TString>::const_iterator chariter;
  for (chariter = charflag.begin(); chariter != charflag.end(); ++chariter)
  {
    outFile << "C\t" << chariter->first << "\t" << chariter->second << endl;
  }

  map<string, bool>::const_iterator booliter;
  for (booliter = boolflag.begin(); booliter != boolflag.end(); ++booliter)
  {
    outFile << "B\t" << booliter->first << "\t" << booliter->second << endl;
  }

  outFile.close();
}

recoConsts *recoConsts::__instance = nullptr;
recoConsts *recoConsts::instance()
{
  if (__instance == nullptr)
  {
    __instance = new recoConsts();
    __instance->set_defaults();
  }

  return __instance;
}

recoConsts::recoConsts()
{
}

void recoConsts::set_CharFlag(const std::string &name, const std::string &flag)
{
  std::string flag_expanded = ExpandEnvironmentals(flag);
  charflag[name] = flag_expanded;
}

std::string recoConsts::ExpandEnvironmentals(const std::string &input)
{
  wordexp_t exp_result;
  if (wordexp(input.c_str(), &exp_result, 0) != 0)
  {
    std::cout << "ExpandEnvironmentals - ERROR - Your string '" << input << "' cannot be understood!" << std::endl;
    return "";
  }
  const std::string output(exp_result.we_wordv[0]);
  return output;
}

void recoConsts::set_defaults()
{
  //Following constants are shared between simulation and reconstruction
  set_DoubleFlag("KMAGSTR", 1.0);
  set_DoubleFlag("FMAGSTR", 1.0);
  set_DoubleFlag("TMAGSTR", 5.0); // Target magnetic field strength, in Tesla

  //Following flags control the running mode and must be
  //set to appropriate values in the configuration set
  set_IntFlag("RUNNUMBER", 1);

  set_BoolFlag("KMAG_ON", true);
  set_BoolFlag("COARSE_MODE", false);
  set_BoolFlag("MC_MODE", false);
  set_BoolFlag("COSMIC_MODE", false);

  set_BoolFlag("TARGETONLY", false);
  set_BoolFlag("DUMPONLY", false);

  //Following values are fed to GeomSvc
  set_BoolFlag("OnlineAlignment", false);
  set_BoolFlag("IdealGeom", false);

  set_CharFlag("AlignmentMille", "$E1039_RESOURCE/alignment/run6/align_mille.txt");
  set_CharFlag("AlignmentHodo", "$E1039_RESOURCE/alignment/run6/alignment_hodo.txt");
  set_CharFlag("AlignmentProp", "$E1039_RESOURCE/alignment/run6/alignment_prop.txt");
  set_CharFlag("Calibration", "$E1039_RESOURCE/alignment/run6/calibration.txt");

  set_CharFlag("DB_SERVER", "DB1");
  set_CharFlag("DB_USER", "seaguest");

  set_CharFlag("TRIGGER_Repo", "$TRIGGER_ROOT");
  set_CharFlag("TRIGGER_L1", "67");

  set_CharFlag("fMagFile", "$GEOMETRY_ROOT/magnetic_fields/Fmag.root");
  set_CharFlag("kMagFile", "$GEOMETRY_ROOT/magnetic_fields/Kmag.root");

  //Following flags adjust the performance/efficiency of the reconstruction,
  //thus should be included in the configuration set
  set_DoubleFlag("TX_MAX", 0.15);
  set_DoubleFlag("TY_MAX", 0.1);
  set_DoubleFlag("X0_MAX", 150.);
  set_DoubleFlag("Y0_MAX", 50.);
  set_DoubleFlag("INVP_MIN", 0.01);
  set_DoubleFlag("INVP_MAX", 0.2);
  set_DoubleFlag("PROB_LOOSE", 0.);
  set_DoubleFlag("PROB_TIGHT", 1.E-12);
  set_DoubleFlag("BAD_HIT_REJECTION", 3.);
  set_DoubleFlag("MERGE_THRESH", 0.015);
  set_DoubleFlag("RESOLUTION_FACTOR", 1.6);

  set_DoubleFlag("X_BEAM", 0.);
  set_DoubleFlag("Y_BEAM", 0.);
  set_DoubleFlag("SIGX_BEAM", 0.3);
  set_DoubleFlag("SIGY_BEAM", 0.3);

  set_DoubleFlag("X0_TARGET", 0.);
  set_DoubleFlag("Y0_TARGET", 0.);
  set_DoubleFlag("RX_TARGET", 0.95);
  set_DoubleFlag("RY_TARGET", 0.95);

  set_CharFlag("EventReduceOpts", "aoc");

  set_BoolFlag("USE_V1495_HIT", true);
  set_BoolFlag("USE_TWTDC_HIT", false);

  set_IntFlag("NSTEPS_FMAG", 100);
  set_IntFlag("NSTEPS_SHIELDING", 50);
  set_IntFlag("NSTEPS_TARGET", 100);

  set_DoubleFlag("TDCTimeOffset", 0.);

  set_DoubleFlag("RejectWinDC0", 0.12);
  set_DoubleFlag("RejectWinDC1", 0.12);
  set_DoubleFlag("RejectWinDC2", 0.15);
  set_DoubleFlag("RejectWinDC3p", 0.16);
  set_DoubleFlag("RejectWinDC3m", 0.14);

  set_IntFlag("MaxHitsDC0", 100);
  set_IntFlag("MaxHitsDC1", 100);
  set_IntFlag("MaxHitsDC2", 100);
  set_IntFlag("MaxHitsDC3p", 100);
  set_IntFlag("MaxHitsDC3m", 100);

  //Following numbers are related to the geometric set up thus should not
  //change under most circumstances, unless one is studying the effects of these cuts
  //could be excluded from the configuration set
  set_DoubleFlag("SAGITTA_TARGET_CENTER", 1.85);
  set_DoubleFlag("SAGITTA_TARGET_WIDTH", 0.25);
  set_DoubleFlag("SAGITTA_DUMP_CENTER", 1.5);
  set_DoubleFlag("SAGITTA_DUMP_WIDTH", 0.3);

  set_IntFlag("MUID_MINHITS", 1);
  set_DoubleFlag("MUID_REJECTION", 4.);
  set_DoubleFlag("MUID_THE_P0", 0.11825);
  set_DoubleFlag("MUID_EMP_P0", 0.00643);
  set_DoubleFlag("MUID_EMP_P1", -0.00009);
  set_DoubleFlag("MUID_EMP_P2", 0.00000046);
  set_DoubleFlag("MUID_Z_REF", 2028.19);
  set_DoubleFlag("MUID_R_CUT", 3.0);

  set_DoubleFlag("DEDX_FE_P0", 7.18274);
  set_DoubleFlag("DEDX_FE_P1", 0.0361447);
  set_DoubleFlag("DEDX_FE_P2", -0.000718127);
  set_DoubleFlag("DEDX_FE_P3", 7.97312e-06);
  set_DoubleFlag("DEDX_FE_P4", -3.05481e-08);

  set_DoubleFlag("PT_KICK_KMAG", 0.4016);
  set_DoubleFlag("PT_KICK_FMAG", 2.909);

  set_DoubleFlag("Z_KMAG_BEND", 1064.26);
  set_DoubleFlag("Z_FMAG_BEND", 251.4);
  set_DoubleFlag("Z_KFMAG_BEND", 375.);
  set_DoubleFlag("ELOSS_KFMAG", 8.12);
  set_DoubleFlag("ELOSS_ABSORBER", 1.81);
  set_DoubleFlag("Z_ST2", 1347.36);
  set_DoubleFlag("Z_ABSORBER", 2028.19);
  set_DoubleFlag("Z_REF", 0.);
  set_DoubleFlag("Z_TARGET", -300.00);
  set_DoubleFlag("Z_DUMP", 42.);
  set_DoubleFlag("Z_ST1", 600.);
  set_DoubleFlag("Z_ST3", 1910.);
  set_DoubleFlag("FMAG_HOLE_LENGTH", 27.94);
  set_DoubleFlag("FMAG_HOLE_RADIUS", 1.27);
  set_DoubleFlag("FMAG_LENGTH", 502.92);
  set_DoubleFlag("Z_UPSTREAM", -500.);
  set_DoubleFlag("Z_DOWNSTREAM", 500.);
}

void recoConsts::init(int runNo, bool verbose)
{
  //TODO: initialization based on run range
  set_IntFlag("RUNNUMBER", runNo);
  return;
}

void recoConsts::init(const std::string &setname, bool verbose)
{
  if (setname == "cosmic")
  {
    set_BoolFlag("KMAG_ON", false);
    set_BoolFlag("COSMIC_MODE", true);

    set_DoubleFlag("TX_MAX", 1.);
    set_DoubleFlag("TY_MAX", 1.);
    set_DoubleFlag("X0_MAX", 1000.);
    set_DoubleFlag("Y0_MAX", 1000.);
  }

  if (verbose)
    Print();
}

void recoConsts::initfile(const std::string &filename, bool verbose)
{
  ReadFromFile(filename, verbose);
  set_CharFlag("ConfigFile", filename.c_str());
  return;
}

void recoConsts::save(TFile *saveFile, TString name)
{
  saveFile->cd();
  TTree *saveTree = new TTree(name.Data(), name.Data());

#ifdef GIT_VERSION
  TString s_softver = GIT_VERSION;
#else
  TString s_softver = "Unknown";
#endif
  saveTree->Branch("SoftVer", &s_softver);

  //Convert all flags to TString map
  map<TString, TString> allconfig;
  for(auto it = intflag.begin(); it != intflag.end(); ++it)
  {
    allconfig[it->first] = TString::Format("%d", it->second);
  }

  for(auto it = doubleflag.begin(); it != doubleflag.end(); ++it)
  {
    allconfig[it->first] = TString::Format("%.4f", it->second);
  }

  for(auto it = floatflag.begin(); it != floatflag.end(); ++it)
  {
    allconfig[it->first] = TString::Format("%.4f", it->second);
  }

  for(auto it = charflag.begin(); it != charflag.end(); ++it)
  {
    allconfig[it->first] = it->second;
  }

  for(auto it = boolflag.begin(); it != boolflag.end(); ++it)
  {
    allconfig[it->first] = it->second ? "True" : "False";
  }

  //Book and fill everything
  for(auto it = allconfig.begin(); it != allconfig.end(); ++it)
  {
    saveTree->Branch(iter->first, &(it->second));
  }
  saveTree->Fill();
  saveTree->Write();
}

void recoConsts::Print() const
{
  // methods from PHFlag
  PrintCharFlags();
  PrintFloatFlags();
  PrintDoubleFlags();
  PrintIntFlags();
  PrintBoolFlags();
}
