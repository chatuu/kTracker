#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <list>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "TriggerAnalyzer.h"

using namespace std;

int main(int argc, char *argv[])
{
  //Initialize geometry service
  GeomSvc* geometrySvc = GeomSvc::instance();
  geometrySvc->init(GEOMETRY_VERSION);

  TriggerAnalyzer* triggerAna = new TriggerAnalyzer();
  triggerAna->init();
  triggerAna->buildTriggerTree();

  //Read in the bkg events
  SRawEvent* dataEvent = new SRawEvent();

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  dataTree->SetBranchAddress("rawEvent", &dataEvent);

  //read in the MC events 
  SRawMCEvent* mcEvent = new SRawMCEvent();

  TFile* mcFile = new TFile(argv[2], "READ");
  TTree* mcTree = (TTree*)mcFile->Get("save");

  mcTree->SetBranchAddress("rawEvent", &mcEvent);
 
  TFile* saveFile = new TFile(argv[3], "recreate");
  TTree* saveTree = mcTree->CloneTree(0);

  Int_t targetPos = atoi(argv[4]);
  Int_t index_MC = 0;
  for(Int_t i = 0; i < dataTree->GetEntries(); ++i)
    {
      if(index_MC > mcTree->GetEntries()) break;

      dataTree->GetEntry(i);
      if(dataEvent->getTargetPos() != targetPos) continue;

      mcTree->GetEntry(index_MC++);
      cout << i << "  " << index_MC << endl;

      mcEvent->mergeEvent(*dataEvent);
      mcEvent->setTriggerEmu(triggerAna->acceptEvent(mcEvent, USE_HIT));

      int nRoads[4] = {triggerAna->getNRoadsPosTop(), triggerAna->getNRoadsPosBot(), triggerAna->getNRoadsNegTop(), triggerAna->getNRoadsNegBot()};
      mcEvent->setNRoads(nRoads);

      saveTree->Fill();
      mcEvent->clear();
      dataEvent->clear();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 1;
}
