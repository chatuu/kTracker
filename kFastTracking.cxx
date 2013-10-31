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

#include "MODE_SWITCH.h"

using namespace std;

int main(int argc, char *argv[])
{
  //Initialize geometry service
  Log("Initializing geometry service ... ");
  GeomSvc* geometrySvc = GeomSvc::instance();
  geometrySvc->init(GEOMETRY_VERSION);

  //Retrieve the raw event
  Log("Retrieving the event stored in ROOT file ... ");
#ifdef MC_MODE
  SRawMCEvent* rawEvent = new SRawMCEvent();
#else
  SRawEvent* rawEvent = new SRawEvent();
#endif

  TFile *dataFile = new TFile(argv[1], "READ");
  TTree *dataTree = (TTree *)dataFile->Get("save");

  dataTree->SetBranchAddress("rawEvent", &rawEvent);
 
  //Output definition
  TClonesArray* tracklets = new TClonesArray("Tracklet");
  TClonesArray& arr_tracklets = *tracklets;

  int runID, spillID, eventID;
  int nTracklets;
  double time;

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = dataTree->CloneTree(0);

  saveTree->Branch("runID", &runID, "runID/I");
  saveTree->Branch("spillID", &spillID, "spillID/I");
  saveTree->Branch("eventID", &eventID, "eventID/I");
  saveTree->Branch("time", &time, "time/D");
  saveTree->Branch("nTracklets", &nTracklets, "nTracklets/I");
  saveTree->Branch("tracklets", &tracklets, 256000, 99);
  tracklets->BypassStreamer();

  //Initialize track finder
  Log("Initializing the track finder and kalman filter ... ");
  KalmanFastTracking *fastfinder = new KalmanFastTracking(false);

  int offset = argc > 3 ? atoi(argv[3]) : 0;
  int nEvtMax = argc > 4 ? atoi(argv[4]) + offset : dataTree->GetEntries();
  if(nEvtMax > dataTree->GetEntries()) nEvtMax = dataTree->GetEntries();
  Log("Running from event " << offset << " through to event " << nEvtMax);
  for(int i = offset; i < nEvtMax; i++)
    {
      dataTree->GetEntry(i);
      Log("Processing event " << i << " with eventID = " << rawEvent->getEventID());

      clock_t time_single = clock();

      rawEvent->reIndex("oa");
      if(!fastfinder->setRawEvent(rawEvent)) continue;

      //Fill the TClonesArray
      arr_tracklets.Clear();
      std::list<Tracklet>& rec_tracklets = fastfinder->getFinalTracklets();
      if(rec_tracklets.empty()) continue;

      nTracklets = 0;
      for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
	{
	  iter->calcChisq();
	  iter->print();
	  new(arr_tracklets[nTracklets]) Tracklet(*iter);
	  nTracklets++;
	}

      runID = rawEvent->getRunID();
      spillID = rawEvent->getSpillID();
      eventID = rawEvent->getEventID();

      time_single = clock() - time_single;
      time = double(time_single)/CLOCKS_PER_SEC;
      Log("It takes " << time << " seconds for this event.");

      saveTree->Fill();
      rawEvent->clear();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  delete fastfinder;
  return 1;
}
