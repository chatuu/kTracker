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

  //event info
  int runID;
  int spillID;
  int eventID;
  int targetPos;
  
  //dimuon infomation
  float mass, xF, x1, x2, pT;
  float chisq_dimuon;
  float x0, y0, z0;

  //mu+ infomation
  int nHits1;
  float chisq1;
  float px1_vertex, py1_vertex, pz1_vertex;
  float x1_vertex, y1_vertex, z1_vertex;
  float x1_target, y1_target, z1_target;
  float x1_dump,   y1_dump,   z1_dump;
  float px1_st1, py1_st1, pz1_st1;

  //mu- infomation
  int nHits2;
  float chisq2;
  float px2_vertex, py2_vertex, pz2_vertex;
  float x2_vertex, y2_vertex, z2_vertex;
  float x2_target, y2_target, z2_target;
  float x2_dump,   y2_dump,   z2_dump;
  float px2_st1, py2_st1, pz2_st1;

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");

  saveTree->Branch("runID", &runID, "runID/I");  
  saveTree->Branch("spillID", &spillID, "spillID/I");  
  saveTree->Branch("eventID", &eventID, "eventID/I");  
  saveTree->Branch("targetPos", &targetPos, "targetPos/I");

  saveTree->Branch("mass", &mass, "mass/F");
  saveTree->Branch("xF", &xF, "xF/F");
  saveTree->Branch("x1", &x1, "x1/F");
  saveTree->Branch("x2", &x2, "x2/F");
  saveTree->Branch("pT", &pT, "pT/F");
  saveTree->Branch("chisq_dimuon", &chisq_dimuon, "chisq_dimuon/F");
  saveTree->Branch("x0", &x0, "x0/F");
  saveTree->Branch("y0", &y0, "y0/F");
  saveTree->Branch("z0", &z0, "z0/F");

  saveTree->Branch("nHits1", &nHits1, "nHits1/I");
  saveTree->Branch("chisq1", &chisq1, "chisq1/F");
  saveTree->Branch("px1_vertex", &px1_vertex, "px1_vertex/F");
  saveTree->Branch("py1_vertex", &py1_vertex, "py1_vertex/F");
  saveTree->Branch("pz1_vertex", &pz1_vertex, "pz1_vertex/F");
  saveTree->Branch("px1_st1", &px1_st1, "px1_st1/F");
  saveTree->Branch("py1_st1", &py1_st1, "py1_st1/F");
  saveTree->Branch("pz1_st1", &pz1_st1, "pz1_st1/F");
  saveTree->Branch("x1_vertex", &x1_vertex, "x1_vertex/F");
  saveTree->Branch("y1_vertex", &y1_vertex, "y1_vertex/F");
  saveTree->Branch("z1_vertex", &z1_vertex, "z1_vertex/F");
  saveTree->Branch("x1_target", &x1_target, "x1_target/F");
  saveTree->Branch("y1_target", &y1_target, "y1_target/F");
  saveTree->Branch("z1_target", &z1_target, "z1_target/F");
  saveTree->Branch("x1_dump", &x1_dump, "x1_dump/F");
  saveTree->Branch("y1_dump", &y1_dump, "y1_dump/F");
  saveTree->Branch("z1_dump", &z1_dump, "z1_dump/F");
 
  saveTree->Branch("nHits2", &nHits2, "nHits2/I");
  saveTree->Branch("chisq2", &chisq2, "chisq2/F");
  saveTree->Branch("px2_vertex", &px2_vertex, "px2_vertex/F");
  saveTree->Branch("py2_vertex", &py2_vertex, "py2_vertex/F");
  saveTree->Branch("pz2_vertex", &pz2_vertex, "pz2_vertex/F");
  saveTree->Branch("px2_st1", &px2_st1, "px2_st1/F");
  saveTree->Branch("py2_st1", &py2_st1, "py2_st1/F");
  saveTree->Branch("pz2_st1", &pz2_st1, "pz2_st1/F");
  saveTree->Branch("x2_vertex", &x2_vertex, "x2_vertex/F");
  saveTree->Branch("y2_vertex", &y2_vertex, "y2_vertex/F");
  saveTree->Branch("z2_vertex", &z2_vertex, "z2_vertex/F");
  saveTree->Branch("x2_target", &x2_target, "x2_target/F");
  saveTree->Branch("y2_target", &y2_target, "y2_target/F");
  saveTree->Branch("z2_target", &z2_target, "z2_target/F");
  saveTree->Branch("x2_dump", &x2_dump, "x2_dump/F");
  saveTree->Branch("y2_dump", &y2_dump, "y2_dump/F");
  saveTree->Branch("z2_dump", &z2_dump, "z2_dump/F");
  
  int nEntry = 0;
  for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
      dataTree->GetEntry(i);
  
      runID = recEvent->getRunID();
      spillID = recEvent->getSpillID();
      eventID = recEvent->getEventID();
      targetPos = recEvent->getTargetPos();

      int nDimuons = recEvent->getNDimuons();
      for(int j = 0; j < nDimuons; ++j)
	{
	  SRecDimuon dimuon = recEvent->getDimuon(j);

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
	  posTrack.getMomentumSt1(x_dummy, y_dummy, z_dummy);
	  px1_st1 = x_dummy;
	  py1_st1 = y_dummy;
	  pz1_st1 = z_dummy;

	  SRecTrack negTrack = recEvent->getTrack(dimuon.trackID_neg);
	  nHits2 = negTrack.getNHits();
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
	  negTrack.getMomentumSt1(x_dummy, y_dummy, z_dummy);
	  px2_st1 = x_dummy;
	  py2_st1 = y_dummy;
	  pz2_st1 = z_dummy;

	  saveTree->Fill();
	}	  

      rawEvent->clear();
      recEvent->clear();
    }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 1;
}
