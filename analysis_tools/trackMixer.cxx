#include <iostream>
#include <fstream>
#include <sstream>
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
#include <TMath.h>
#include <TRandom.h>
#include <TChain.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"

int main(int argc, char *argv[])
{
  //Initialize geometry service
  LogInfo("Initializing geometry service ... ");
  GeomSvc* geometrySvc = GeomSvc::instance();
  geometrySvc->init(GEOMETRY_VERSION);

  //Retrieve the raw event
  LogInfo("Retrieving the event stored in ROOT file ... ");
  SRawEvent* rawEvent = new SRawEvent();
  SRecEvent* recEvent = new SRecEvent();

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  dataTree->SetBranchAddress("rawEvent", &rawEvent);
  dataTree->SetBranchAddress("recEvent", &recEvent);

  SRecEvent* mixEvent = new SRecEvent();

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");

  saveTree->Branch("rawEvent", &rawEvent, 256000, 99);
  saveTree->Branch("recEvent", &mixEvent, 256000, 99);

  //Load track bank of mu+ and mu-
  vector<SRecTrack> ptracks, mtracks;
  vector<int> pflags, mflags;
  //vector<int> pintensity, mintensity;
  ptracks.clear(); mtracks.clear(); 
  pflags.clear(); mflags.clear();
  //pintensity.clear(); mintensity.clear();
  ptracks.reserve(100000); mtracks.reserve(100000); 
  pflags.reserve(100000); mflags.reserve(100000);
  //pintensity.reserve(100000); mintensity.reserve(100000);

  //Extract all the tracks and put in the container
  TRandom rnd;
  rnd.SetSeed(atoi(argv[4]));
 
  int nEvtMax = dataTree->GetEntries();
  for(int i = 0; i < nEvtMax; i++)
    {
      dataTree->GetEntry(i);
      if(i % 1000 == 0) cout << "  " << i << "/" << nEvtMax << endl;
      
      if(!rawEvent->isTriggeredBy(SRawEvent::MATRIX1)) continue;

      int nTracks = recEvent->getNTracks();
      if(nTracks != 1) continue;
      for(int j = 0; j < nTracks; j++)
	{
	  //cout << j << endl;
	  SRecTrack track = recEvent->getTrack(j);
	  track.setZVertex(track.getZVertex());
	  if(!track.isValid()) continue;

	  if(track.getCharge() > 0)
    	    {
	      ptracks.push_back(track);
	      pflags.push_back(1);
	      //pintensity.push_back(rawEvent->getIntensity(16));
	    }
	  else
	    {
	      mtracks.push_back(track);
	      mflags.push_back(1);
	    }
	}

      rawEvent->clear();
      recEvent->clear();
    }

  //Random combination
  int nPlus = ptracks.size();
  int nMinus = mtracks.size();
  int nEntries = atoi(argv[3]);
  LogInfo("Totally " << nPlus << " positive tracks and " << nMinus << " negative tracks ..");

  int nEntriesMax = int(nPlus < nMinus ? 0.8*nPlus : 0.8*nMinus);
  if(nEntries > nEntriesMax) nEntries = nEntriesMax;
  while(saveTree->GetEntries() < nEntries)
    {
      int id1 = int(rnd.Rndm()*nPlus);
      int id2 = int(rnd.Rndm()*nMinus);

      //Neither of the tracks should be used
      if(pflags[id1] < 0 || mflags[id2] < 0) continue;
      
      mixEvent->insertTrack(ptracks[id1]); pflags[id1] = -1;
      mixEvent->insertTrack(mtracks[id2]); mflags[id2] = -1;

      saveTree->Fill();
      mixEvent->clear();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 1;
}
