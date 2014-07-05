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
#include <TMath.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char *argv[])
{
  //Initialize geometry service
  LogInfo("Initializing geometry service ... ");
  GeomSvc* geometrySvc = GeomSvc::instance();
  geometrySvc->init(GEOMETRY_VERSION);

  //Retrieve the raw event
  LogInfo("Retrieving the event stored in ROOT file ... ");
  SRecEvent* recEvent = new SRecEvent();

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  dataTree->SetBranchAddress("recEvent", &recEvent);

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = dataTree->CloneTree(0);
  
  //Initialize track finder
  LogInfo("Initializing the track finder and kalman filter ... ");
  VertexFit* vtxfit = new VertexFit();
  vtxfit->enableOptimization();
  if(argc > 3) vtxfit->bookEvaluation(argv[3]);

  int nEvtMax = argc > 4 ? atoi(argv[4]) : dataTree->GetEntries();
  if(nEvtMax > dataTree->GetEntries()) nEvtMax = dataTree->GetEntries();
  LogInfo("Running from event 0 through to event " << nEvtMax);
  for(int i = 0; i < nEvtMax; i++)
    {
      dataTree->GetEntry(i);
      cout << "\r Processing event " << i << " with eventID = " << recEvent->getEventID() << ", ";
      cout << (i + 1)*100/nEvtMax << "% finished .. " << flush;

      vtxfit->setRecEvent(recEvent);

      if(recEvent->getNDimuons() > 0) saveTree->Fill();
      recEvent->clear();
    }
  cout << endl;
  cout << "kVertex ends successfully." << endl;

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  delete vtxfit;

  return EXIT_SUCCESS;
}
