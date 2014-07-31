#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>

#include "SRecEvent.h"
#include "GeomSvc.h"
#include "MySQLSvc.h"

using namespace std;

int main(int argc, char **argv)
{
  cout << "Uploading file: " << argv[1] << " to sql schema " << argv[2] << endl;

  ///Initialize the geometry service and output file 
  GeomSvc* p_geomSvc = GeomSvc::instance();
  p_geomSvc->init(GEOMETRY_VERSION);

  MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
  p_mysqlSvc->setUserPasswd("production", "qqbar2mu+mu-");
  p_mysqlSvc->connect(argv[3], atoi(argv[4]));
  p_mysqlSvc->setWorkingSchema(argv[2]);
  p_mysqlSvc->initWriter();

  ///Retrieve data from file
  TClonesArray* tracklets = new TClonesArray("Tracklet");
  SRecEvent* recEvent = new SRecEvent();

  TFile* dataFile = new TFile(argv[1], "recreate");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  dataTree->SetBranchAddress("recEvent", &recEvent);
  dataTree->SetBranchAddress("tracklets", &tracklets);

  int nEvents = dataTree->GetEntries();
  for(int i = 0; i < nEvents; ++i)
    {
      dataTree->GetEntry(i);
      cout << "\r Uploading event " << recEvent->getEventID() << ", " << (i+1)*100/nEvents << "% finished." << flush;

      p_mysqlSvc->writeTrackingRes(recEvent, tracklets);
    }
  cout << endl;
  cout << "sqlResWriter ends successfully." << endl;

  delete p_mysqlSvc;
  delete p_geomSvc;

  return 1;
}
