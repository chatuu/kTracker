#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <sstream>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "SRawEvent.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char* argv[])
{
    set<int> goodSpillIDs;
    ifstream fin(argv[1]);

    string line;
    while(getline(fin, line))
    {
        int sID = atoi(line.c_str());
        goodSpillIDs.insert(sID);
    }

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("mb");

    SRawEvent* rawEvent = new SRawEvent();
    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    TFile* saveFile[7];
    TTree* saveTree[7];
    for(int i = 1; i < 8; ++i)
    {
        saveFile[i-1] = new TFile(Form("%s_%d.root", argv[2], i), "recreate");
        saveTree[i-1] = dataTree->CloneTree(0);
    }

    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);
        if(i % 10000 == 0) cout << i << "  -  " << dataTree->GetEntries() << endl;

        int spillID = rawEvent->getSpillID();
        int eventID = rawEvent->getEventID();
        if(goodSpillIDs.count(rawEvent->getSpillID()) != 0)
        {
            int index = spillBank[spillID].targetPos - 1;
            if(index >= 0 && index < 7) saveTree[index]->Fill();
        }

        rawEvent->clear();
    }

    for(int i = 0; i < 7; ++i)
    {
        saveFile[i]->cd();
        saveTree[i]->Write();
        saveFile[i]->Close();
    }

    return EXIT_SUCCESS;
}
