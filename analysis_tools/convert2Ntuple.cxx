#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "SRawEvent.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char* argv[])
{
    //Initialize the spill LUT
    map<int, float> map_G2SEM, map_liveG2SEM, map_ratio;
    map<int, int> map_enable;

    int spillID;
    float m_G2SEM, m_busySum, m_QIESum, m_inhibitSum;
    TFile* spillFile = new TFile(argv[3], "READ");
    TTree* spillTree = (TTree*)spillFile->Get("save");

    spillTree->SetBranchAddress("spillID", &spillID);
    spillTree->SetBranchAddress("G2SEM", &m_G2SEM);
    spillTree->SetBranchAddress("QIESum", &m_QIESum);
    spillTree->SetBranchAddress("busySum", &m_busySum);
    spillTree->SetBranchAddress("inhibitSum", &m_inhibitSum);

    for(int i = 0; i < spillTree->GetEntries(); ++i)
    {
        spillTree->GetEntry(i);

        map_G2SEM[spillID] = m_G2SEM;
        map_liveG2SEM[spillID] = m_G2SEM*(1. - m_busySum/m_QIESum - m_inhibitSum/m_QIESum);
        map_ratio[spillID] = m_G2SEM/m_QIESum;
        map_enable[spillID] = -1;
    }

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    SRawEvent* rawEvent = new SRawEvent();
    SRecEvent* recEvent = new SRecEvent();
    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("recEvent", &recEvent);

    //event info
    int runID;
    int eventID;
    int targetPos;
    double G2SEM;
    double liveG2SEM;
    double intensity;
    int flag;

    //dimuon infomation
    double mass, xF, x1, x2, pT;
    double chisq_dimuon;
    double x0, y0, z0;

    //mu+ infomation
    int nHits1;
    int triggerID1;
    double chisq1;
    double px1_vertex, py1_vertex, pz1_vertex;
    double x1_vertex, y1_vertex, z1_vertex;
    double x1_target, y1_target, z1_target;
    double x1_dump,   y1_dump,   z1_dump;
    double x1_up,     y1_up,     z1_up;
    double px1_st1, py1_st1, pz1_st1;
    double px1_st3, py1_st3, pz1_st3;
    double x1_st1, y1_st1, z1_st1;
    double x1_st3, y1_st3, z1_st3;

    //mu- infomation
    int nHits2;
    int triggerID2;
    double chisq2;
    double px2_vertex, py2_vertex, pz2_vertex;
    double x2_vertex, y2_vertex, z2_vertex;
    double x2_target, y2_target, z2_target;
    double x2_dump,   y2_dump,   z2_dump;
    double x2_up,     y2_up,     z2_up;
    double px2_st1, py2_st1, pz2_st1;
    double px2_st3, py2_st3, pz2_st3;
    double x2_st1, y2_st1, z2_st1;
    double x2_st3, y2_st3, z2_st3;

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = new TTree("data", "save");
    TTree* spillSaveTree = spillTree->CloneTree(0);

    saveTree->Branch("runID", &runID, "runID/I");
    saveTree->Branch("spillID", &spillID, "spillID/I");
    saveTree->Branch("eventID", &eventID, "eventID/I");
    saveTree->Branch("flag", &flag, "flag/I");
    saveTree->Branch("targetPos", &targetPos, "targetPos/I");
    saveTree->Branch("G2SEM", &G2SEM, "G2SEM/D");
    saveTree->Branch("liveG2SEM", &liveG2SEM, "liveG2SEM/D");
    saveTree->Branch("intensity", &intensity, "intensity/D");
    saveTree->Branch("mass", &mass, "mass/D");
    saveTree->Branch("xF", &xF, "xF/D");
    saveTree->Branch("x1", &x1, "x1/D");
    saveTree->Branch("x2", &x2, "x2/D");
    saveTree->Branch("pT", &pT, "pT/D");
    saveTree->Branch("chisq_dimuon", &chisq_dimuon, "chisq_dimuon/D");
    saveTree->Branch("x0", &x0, "x0/D");
    saveTree->Branch("y0", &y0, "y0/D");
    saveTree->Branch("z0", &z0, "z0/D");

    saveTree->Branch("nHits1", &nHits1, "nHits1/I");
    saveTree->Branch("triggerID1", &triggerID1, "triggerID1/I");
    saveTree->Branch("chisq1", &chisq1, "chisq1/D");
    saveTree->Branch("px1_vertex", &px1_vertex, "px1_vertex/D");
    saveTree->Branch("py1_vertex", &py1_vertex, "py1_vertex/D");
    saveTree->Branch("pz1_vertex", &pz1_vertex, "pz1_vertex/D");
    saveTree->Branch("px1_st1", &px1_st1, "px1_st1/D");
    saveTree->Branch("py1_st1", &py1_st1, "py1_st1/D");
    saveTree->Branch("pz1_st1", &pz1_st1, "pz1_st1/D");
    saveTree->Branch("px1_st3", &px1_st3, "px1_st3/D");
    saveTree->Branch("py1_st3", &py1_st3, "py1_st3/D");
    saveTree->Branch("pz1_st3", &pz1_st3, "pz1_st3/D");
    saveTree->Branch("x1_st1", &x1_st1, "x1_st1/D");
    saveTree->Branch("y1_st1", &y1_st1, "y1_st1/D");
    saveTree->Branch("z1_st1", &z1_st1, "z1_st1/D");
    saveTree->Branch("x1_st3", &x1_st3, "x1_st3/D");
    saveTree->Branch("y1_st3", &y1_st3, "y1_st3/D");
    saveTree->Branch("z1_st3", &z1_st3, "z1_st3/D");
    saveTree->Branch("x1_vertex", &x1_vertex, "x1_vertex/D");
    saveTree->Branch("y1_vertex", &y1_vertex, "y1_vertex/D");
    saveTree->Branch("z1_vertex", &z1_vertex, "z1_vertex/D");
    saveTree->Branch("x1_target", &x1_target, "x1_target/D");
    saveTree->Branch("y1_target", &y1_target, "y1_target/D");
    saveTree->Branch("z1_target", &z1_target, "z1_target/D");
    saveTree->Branch("x1_dump", &x1_dump, "x1_dump/D");
    saveTree->Branch("y1_dump", &y1_dump, "y1_dump/D");
    saveTree->Branch("z1_dump", &z1_dump, "z1_dump/D");
    saveTree->Branch("x1_up", &x1_up, "x1_up/D");
    saveTree->Branch("y1_up", &y1_up, "y1_up/D");
    saveTree->Branch("z1_up", &z1_up, "z1_up/D");

    saveTree->Branch("nHits2", &nHits2, "nHits2/I");
    saveTree->Branch("triggerID2", &triggerID2, "triggerID2/I");
    saveTree->Branch("chisq2", &chisq2, "chisq2/D");
    saveTree->Branch("px2_vertex", &px2_vertex, "px2_vertex/D");
    saveTree->Branch("py2_vertex", &py2_vertex, "py2_vertex/D");
    saveTree->Branch("pz2_vertex", &pz2_vertex, "pz2_vertex/D");
    saveTree->Branch("px2_st1", &px2_st1, "px2_st1/D");
    saveTree->Branch("py2_st1", &py2_st1, "py2_st1/D");
    saveTree->Branch("pz2_st1", &pz2_st1, "pz2_st1/D");
    saveTree->Branch("px2_st3", &px2_st3, "px2_st3/D");
    saveTree->Branch("py2_st3", &py2_st3, "py2_st3/D");
    saveTree->Branch("pz2_st3", &pz2_st3, "pz2_st3/D");
    saveTree->Branch("x2_st1", &x2_st1, "x2_st1/D");
    saveTree->Branch("y2_st1", &y2_st1, "y2_st1/D");
    saveTree->Branch("z2_st1", &z2_st1, "z2_st1/D");
    saveTree->Branch("x2_st3", &x2_st3, "x2_st3/D");
    saveTree->Branch("y2_st3", &y2_st3, "y2_st3/D");
    saveTree->Branch("z2_st3", &z2_st3, "z2_st3/D");
    saveTree->Branch("x2_vertex", &x2_vertex, "x2_vertex/D");
    saveTree->Branch("y2_vertex", &y2_vertex, "y2_vertex/D");
    saveTree->Branch("z2_vertex", &z2_vertex, "z2_vertex/D");
    saveTree->Branch("x2_target", &x2_target, "x2_target/D");
    saveTree->Branch("y2_target", &y2_target, "y2_target/D");
    saveTree->Branch("z2_target", &z2_target, "z2_target/D");
    saveTree->Branch("x2_dump", &x2_dump, "x2_dump/D");
    saveTree->Branch("y2_dump", &y2_dump, "y2_dump/D");
    saveTree->Branch("z2_dump", &z2_dump, "z2_dump/D");
    saveTree->Branch("x2_up", &x2_up, "x2_up/D");
    saveTree->Branch("y2_up", &y2_up, "y2_up/D");
    saveTree->Branch("z2_up", &z2_up, "z2_up/D");


    int nEntry = 0;
    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);

        runID = recEvent->getRunID();
        spillID = recEvent->getSpillID();
        eventID = recEvent->getEventID();
        targetPos = recEvent->getTargetPos();

        if(map_G2SEM.count(spillID) == 0) continue;
        map_enable[spillID] = 1;

        G2SEM = map_G2SEM[spillID];
        liveG2SEM = map_liveG2SEM[spillID];
        intensity = rawEvent->getIntensity()*map_ratio[spillID];

        int nDimuons = recEvent->getNDimuons();
        for(int j = 0; j < nDimuons; ++j)
        {
            SRecDimuon dimuon = recEvent->getDimuon(j);

            flag = -1;
            if(dimuon.isValid())
            {
                if(dimuon.isTarget())
                {
                    flag = 1;
                }
                else if(dimuon.isDump())
                {
                    flag = 2;
                }
                else
                {
                    flag = 3;
                }
            }

            mass = dimuon.mass;
            xF = dimuon.xF;
            x1 = dimuon.x1;
            x2 = dimuon.x2;
            pT = dimuon.pT;
            chisq_dimuon = dimuon.chisq_kf;
            x0 = dimuon.vtx.X();
            y0 = dimuon.vtx.Y();
            z0 = dimuon.vtx.Z();

            double x_dummy, y_dummy, z_dummy;
            SRecTrack posTrack = recEvent->getTrack(dimuon.trackID_pos);
            nHits1 = posTrack.getNHits();
            triggerID1 = posTrack.getTriggerRoad();
            chisq1 = posTrack.getChisq();
            px1_vertex = dimuon.p_pos.X();
            py1_vertex = dimuon.p_pos.Y();
            pz1_vertex = dimuon.p_pos.Z();
            x1_vertex = dimuon.vtx_pos.X();
            y1_vertex = dimuon.vtx_pos.Y();
            z1_vertex = dimuon.vtx_pos.Z();
            x1_target = dimuon.proj_target_pos.X();
            y1_target = dimuon.proj_target_pos.Y();
            z1_target = dimuon.proj_target_pos.Z();
            x1_dump = dimuon.proj_dump_pos.X();
            y1_dump = dimuon.proj_dump_pos.Y();
            z1_dump = dimuon.proj_dump_pos.Z();
            TVector3 posTgtMom = posTrack.getTargetMom();
            z1_up = -500.;
            x1_up = x1_vertex + posTgtMom.Px()/posTgtMom.Pz()*(z1_up - z1_vertex);
            y1_up = y1_vertex + posTgtMom.Py()/posTgtMom.Pz()*(z1_up - z1_vertex);
            posTrack.getMomentumSt1(x_dummy, y_dummy, z_dummy);
            px1_st1 = x_dummy;
            py1_st1 = y_dummy;
            pz1_st1 = z_dummy;
            posTrack.getMomentumSt3(x_dummy, y_dummy, z_dummy);
            px1_st3 = x_dummy;
            py1_st3 = y_dummy;
            pz1_st3 = z_dummy;
            posTrack.getExpPositionFast(650., x_dummy, y_dummy);
            x1_st1 = x_dummy;
            y1_st1 = y_dummy;
            z1_st1 = 650.;
            posTrack.getExpPositionFast(1800., x_dummy, y_dummy);
            x1_st3 = x_dummy;
            y1_st3 = y_dummy;
            z1_st3 = 1800.;


            SRecTrack negTrack = recEvent->getTrack(dimuon.trackID_neg);
            nHits2 = negTrack.getNHits();
            triggerID2 = negTrack.getTriggerRoad();
            chisq2 = negTrack.getChisq();
            px2_vertex = dimuon.p_neg.X();
            py2_vertex = dimuon.p_neg.Y();
            pz2_vertex = dimuon.p_neg.Z();
            x2_vertex = dimuon.vtx_neg.X();
            y2_vertex = dimuon.vtx_neg.Y();
            z2_vertex = dimuon.vtx_neg.Z();
            x2_target = dimuon.proj_target_neg.X();
            y2_target = dimuon.proj_target_neg.Y();
            z2_target = dimuon.proj_target_neg.Z();
            x2_dump = dimuon.proj_dump_neg.X();
            y2_dump = dimuon.proj_dump_neg.Y();
            z2_dump = dimuon.proj_dump_neg.Z();
            TVector3 negTgtMom = negTrack.getTargetMom();
            z2_up = -500.;
            x2_up = x2_vertex + negTgtMom.Px()/negTgtMom.Pz()*(z2_up - z2_vertex);
            y2_up = y2_vertex + negTgtMom.Py()/negTgtMom.Pz()*(z2_up - z2_vertex);
            negTrack.getMomentumSt1(x_dummy, y_dummy, z_dummy);
            px2_st1 = x_dummy;
            py2_st1 = y_dummy;
            pz2_st1 = z_dummy;
            negTrack.getMomentumSt3(x_dummy, y_dummy, z_dummy);
            px2_st3 = x_dummy;
            py2_st3 = y_dummy;
            pz2_st3 = z_dummy;
            negTrack.getExpPositionFast(650., x_dummy, y_dummy);
            x2_st1 = x_dummy;
            y2_st1 = y_dummy;
            z2_st1 = 650.;
            negTrack.getExpPositionFast(1800., x_dummy, y_dummy);
            x2_st3 = x_dummy;
            y2_st3 = y_dummy;
            z2_st3 = 1800.;

            saveTree->Fill();
        }

        rawEvent->clear();
        recEvent->clear();
    }

    for(int i = 0; i < spillTree->GetEntries(); ++i)
    {
        spillTree->GetEntry(i);

        if(map_enable[spillID] > 0)
        {
            map_enable[spillID] = -1;
            spillSaveTree->Fill();
        }
    }
    spillSaveTree->SetName("spill");

    saveFile->cd();
    saveTree->Write();
    spillSaveTree->Write();
    saveFile->Close();

    return 1;
}
