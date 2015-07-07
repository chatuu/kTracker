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

    //Book output file
    TFile* saveFile = new TFile(argv[3], "recreate");

    //Prepare the random tree
    TTree* randomTree = dataTree->CloneTree(0);

    Int_t targetPos = atoi(argv[4]);
    TRandom rndm(0);
    Double_t ratio = Double_t(mcTree->GetEntries())/Double_t(dataTree->Draw("", Form("fTargetPos == %d", targetPos)));
    for(Int_t i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);
        if(dataEvent->getTargetPos() != targetPos) continue;
        if(rndm.Rndm() > ratio) continue;

        randomTree->Fill();
    }
    cout << "Ratio = " << ratio << ", total random events = " << randomTree->GetEntries() << endl;

    //Save the final MC tree
    TTree* saveTree = mcTree->CloneTree(0);
    Int_t nEntries = mcTree->GetEntries() < randomTree->GetEntries() ? mcTree->GetEntries() : randomTree->GetEntries();
    cout << mcTree->GetEntries() << " entries in MC file, will generated " << nEntries << " merged events" << endl;
    for(Int_t i = 0; i < nEntries; ++i)
    {
        randomTree->GetEntry(i);
        mcTree->GetEntry(i);

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
