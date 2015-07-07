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
    ///Initialize the geometry service and output file
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init(GEOMETRY_VERSION);

    MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
    p_mysqlSvc->setUserPasswd(MYSQL_USER, MYSQL_PASS);
    p_mysqlSvc->connect(argv[4], atoi(argv[5]));
    p_mysqlSvc->setWorkingSchema(argv[3]);
    if(!p_mysqlSvc->initWriter()) exit(EXIT_FAILURE);

    ///Retrieve data from file
    TClonesArray* tracklets = new TClonesArray("Tracklet");
    SRecEvent* recEvent1 = new SRecEvent();

    TFile* dataFile1 = new TFile(argv[1], "READ");
    TTree* dataTree1 = (TTree*)dataFile1->Get("save");

    dataTree1->SetBranchAddress("recEvent", &recEvent1);
    dataTree1->SetBranchAddress("tracklets", &tracklets);

    SRecEvent* recEvent2 = new SRecEvent();

    TFile* dataFile2 = new TFile(argv[2], "READ");
    TTree* dataTree2 = (TTree *)dataFile2->Get("save");

    dataTree2->SetBranchAddress("recEvent", &recEvent2);

    int idx_vtx = 0;
    dataTree2->GetEntry(idx_vtx);
    int nEvents = dataTree1->GetEntries();
    for(int i = 0; i < nEvents; ++i)
    {
        dataTree1->GetEntry(i);
        cout << "\r Uploading event " << recEvent1->getEventID() << ", " << (i+1)*100/nEvents << "% finished." << flush;

        if(recEvent1->getEventID() == recEvent2->getEventID())
        {
            for(int j = 0; j < recEvent2->getNDimuons(); ++j)
            {
                recEvent1->insertDimuon(recEvent2->getDimuon(j));
            }

            p_mysqlSvc->writeTrackingRes(recEvent2, tracklets);
            dataTree2->GetEntry(++idx_vtx);
        }
        else
        {
            p_mysqlSvc->writeTrackingRes(recEvent1, tracklets);
        }
    }
    cout << endl;
    cout << "sqlResWriter ends successfully." << endl;

    delete p_mysqlSvc;
    delete p_geomSvc;

    return 1;
}
