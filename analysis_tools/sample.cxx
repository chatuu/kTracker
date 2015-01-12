#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TApplication.h>
#include <TH1I.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TClonesArray.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "SRecEvent.h"
#include "FastTracklet.h"

using namespace std;

int main(int argc, char *argv[])
{
  GeomSvc* p_geomSvc = GeomSvc::instance();
  p_geomSvc->init(GEOMETRY_VERSION);

  //Input structure
  SRawEvent* rawEvent = new SRawEvent();
  SRecEvent* recEvent = new SRecEvent();
  TClonesArray* tracklets = new TClonesArray("Tracklet");

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree *)dataFile->Get("save");

  dataTree->SetBranchAddress("rawEvent", &rawEvent);
  dataTree->SetBranchAddress("recEvent", &recEvent);
  dataTree->SetBranchAddress("tracklets", &tracklets);

  //Output structure
  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");

  for(Int_t i = 0; i < dataTree->GetEntries(); ++i)
    {
      dataTree->GetEntry(i);

      for(Int_t j = 0; j < recEvent->getNTracks(); ++j)
	{
	  SRecTrack track = recEvent->getTrack(j);
	}

      for(Int_t j = 0; j < tracklets->GetEntries(); ++j)
	{
	  Tracklet* track = (Tracklet*)tracklets->At(j);
	}

      rawEvent->clear();
    }
  
  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 1;
}
