#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <time.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>
#include <TMatrixD.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "FastTracklet.h"
#include "KalmanFastTracking.h"
#include "MySQLSvc.h"

#include "MODE_SWITCH.h"

using namespace std;

int main(int argc, char *argv[])
{
  //Initialize geometry service
  GeomSvc* geometrySvc = GeomSvc::instance();
  geometrySvc->init(GEOMETRY_VERSION);

  MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
  p_mysqlSvc->connect();
  p_mysqlSvc->setWorkingSchema(argv[1]);

  //Data output definition
  SRawEvent* rawEvent = new SRawEvent();
  TClonesArray* tracklets = new TClonesArray("Tracklet");
  TClonesArray& arr_tracklets = *tracklets;

  TFile* saveFile = new TFile(argv[2], "READ");
  TTree* saveTree = new TTree("save", "save");

  saveTree->Branch("rawEvent", &rawEvent, 256000, 99);
  saveTree->Branch("tracklets", &tracklets, 256000, 99);
  tracklets->BypassStreamer();

  //Initialize track finder
  KalmanFastTracking* fastfinder = new KalmanFastTracking(false);

  //Start endless tracking
  bool stopRun = false;
  while(!stopRun)
    {
      //Read data
      //if(!p_mysqlSvc->getLatestEvt(rawEvent)) continue;
      if(!p_mysqlSvc->getRandomEvt(rawEvent)) continue;

      //Do the tracking
      cout << "Tracking runID = " << rawEvent->getRunID() << " eventID = " << rawEvent->getEventID() << " with " << rawEvent->getNHitsAll() << " hits: " << endl;
      rawEvent->reIndex("oa");
      if(!fastfinder->setRawEvent(rawEvent))
	{
	  cout << "Tracking failed!" << endl;
  	  continue;
	}

      //Output
      arr_tracklets.Clear();
      std::list<Tracklet>& rec_tracklets = fastfinder->getFinalTracklets();
      if(rec_tracklets.empty()) continue;

      int nTracklets = 0;
      for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
	{
	  iter->print();
	  iter->calcChisq();
	  new(arr_tracklets[nTracklets++]) Tracklet(*iter);
	}
  
      if(nTracklets > 0)
	{
  	  saveTree->Fill();
	}	 
      rawEvent->clear();

      //Get stop run signal
      stopRun = p_mysqlSvc->isRunStopped();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  delete fastfinder;
  return 1;
}
