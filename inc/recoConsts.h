#ifndef RECOCONSTS_H__
#define RECOCONSTS_H__

#include <map>
#include <string>
#include <TString.h>

/**
 * This class instantiates a singleton object that can be accessed via recoConsts::instance()
 * User is expected to do the following at the begining of the user macro:
 *   - recoConsts* rc = recoConsts::instance();    // get an instance of the recoConsts
 *   - rc->init(runNo);    // initiatiate the constants by run numer, or
 *   - rc->init("cosmic"); // initiatiate the constants by a pre-defined parameter set, or
 *   - rc->initfile("const.txt"); //initialize the constants by reading a file, or
 *   - rc->set_DoubleFlag("something", somevalue);  // set specific constants individually
 */ 

class PHFlag
{
public:
  PHFlag() {}
  virtual ~PHFlag() {}

  virtual const std::string get_CharFlag(const std::string &flag) const;
  virtual const std::string get_CharFlag(const std::string &name, const std::string &defaultval);
  virtual void  set_CharFlag(const std::string &name, const std::string &flag);

  virtual double get_DoubleFlag(const std::string &name) const;
  virtual double get_DoubleFlag(const std::string &name, const double defaultval);
  virtual void   set_DoubleFlag(const std::string &name, const double flag);

  virtual float get_FloatFlag(const std::string &name) const;
  virtual float get_FloatFlag(const std::string &name, const float defaultval);
  virtual void  set_FloatFlag(const std::string &name, const float flag);

  virtual int  get_IntFlag(const std::string &name) const;
  virtual int  get_IntFlag(const std::string &name, const int defaultval);
  virtual void set_IntFlag(const std::string &name, const int flag);

  virtual bool get_BoolFlag(const std::string &name) const;
  virtual bool get_BoolFlag(const std::string &name, const bool defaultval);
  virtual void set_BoolFlag(const std::string &name, const bool flag);

  virtual void Print() const;
  virtual void PrintDoubleFlags() const;
  virtual void PrintIntFlags() const;
  virtual void PrintFloatFlags() const;
  virtual void PrintCharFlags() const;
  virtual void PrintBoolFlags() const;
  virtual void ReadFromFile(const std::string& name, bool verbose = false);
  virtual void WriteToFile(const std::string &name);

  virtual int FlagExist(const std::string &name) const;

  virtual const std::map<std::string, int> *IntMap() const {return &intflag;}
  virtual const std::map<std::string, float> *FloatMap() const {return &floatflag;}
  virtual const std::map<std::string, double> *DoubleMap() const {return &doubleflag;}
  virtual const std::map<std::string, std::string> *CharMap() const {return &charflag;}
  virtual const std::map<std::string, bool> *BoolMap() const {return &boolflag;}

protected:

  std::map<std::string, int> intflag;
  std::map<std::string, double> doubleflag;
  std::map<std::string, float> floatflag;
  std::map<std::string, std::string> charflag;
  std::map<std::string, bool> boolflag;
};


class recoConsts: public PHFlag
{
public:
  static recoConsts* instance();

  //! set the default value for all the constants needed - user is supposed to add a default value here to introduce new constants
  void set_defaults();

  //! initialize the constants by the runNo - not implemented yet
  void init(int runNo = 0, bool verbose = false);

  //! initialize the constants by pre-defined parameter set name - not implemented yet
  void init(const std::string& setname, bool verbose = false);

  //! initialize by reading a file
  void initfile(const std::string& filename, bool verbose = false);

  //! overide the virtual function to expand the environmental variables
  virtual void set_CharFlag(const std::string& name, const std::string& flag);

  //! print all the parameters
  void Print() const;

  //! save the configuration to a ROOT Tree
  void save(TFile* saveFile, TString name = "config");

protected: 
  recoConsts();
  std::string ExpandEnvironmentals(const std::string& input);

  static recoConsts* __instance;

};

#endif /* __RECOCONSTS_H__ */
