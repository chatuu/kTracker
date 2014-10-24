#include <iostream>
#include <stdlib.h>
#include <string>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "SRawEvent.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char* argv[])
{
  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  SRawEvent* rawEvent = new SRawEvent();
  SRecEvent* recEvent = new SRecEvent();
  dataTree->SetBranchAddress("rawEvent", &rawEvent);
  dataTree->SetBranchAddress("recEvent", &recEvent);

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");
  
  double mass;
  double x1, x2, xF;
  int target;
  int flag;

  saveTree->Branch("mass", &mass, "mass/D");
  saveTree->Branch("x1", &x1, "x1/D");
  saveTree->Branch("x2", &x2, "x2/D");
  saveTree->Branch("xF", &xF, "xF/D");
  saveTree->Branch("target", &target, "target/I");
  saveTree->Branch("flag", &flag, "flag/I");

  int nEntry = 0;
  for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
      dataTree->GetEntry(i);
      
      double intensity = rawEvent->getIntensity();
      if(!(intensity > lo && intensity < hi && rawEvent->isEmuTriggered() && rawEvent->isTriggeredBy(SRawEvent::MATRIX1)))
	{
	  rawEvent->clear();
	  recEvent->clear();
  	  continue;
	}

      for(int j = 0; j < recEvent->getNDimuons(); ++j)
	{
	  SRecDimuon dimuon = recEvent->getDimuon(j);
	  mass = dimuon.mass;
	  if(dimuon.isValid() && dimuon.isTarget())
	    {
    	      saveTree->Fill();
	    }
	}
      
      rawEvent->clear();
      recEvent->clear();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 1;
}
