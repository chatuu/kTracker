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

#include "SRawEvent.h"
#include "SCODAEvent.h"
#include "GeomSvc.h"
#include "TriggerAnalyzer.h"

using namespace std;

int main(int argc, char **argv)
{
  cout << "Exporting Run: " << argv[1] << " to ROOT file: " << argv[2] << endl;

  ///Initialize the geometry service and output file 
  GeomSvc* p_geomSvc = GeomSvc::instance();
  p_geomSvc->init(GEOMETRY_VERSION);
  p_geomSvc->loadCalibration("calibration.txt");

  ///Initialize the trigger analyzer
  TriggerAnalyzer* p_triggerAna = new TriggerAnalyzer();
  p_triggerAna->init();
  p_triggerAna->buildTriggerTree();

  ///Book IO structure
  SCODAEvent* codaEvent = new SCODAEvent();

  TFile* dataFile = new TFile(argv[1], "READ");
  TTree* dataTree = (TTree*)dataFile->Get("save");

  dataTree->SetBranchAddress("codaEvent", &codaEvent);

  SRawEvent* rawEvent = new SRawEvent();

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");

  saveTree->Branch("rawEvent", &rawEvent, 256000, 99);

  //Extract spill information
  int runID = atoi(argv[3]);
  int nSpills = 0;
  int spillID[500];
  int targetPos[500];
  int first[500];
  int last[500];

  int m_runID;
  int m_spillID;
  int m_targetPos;
  //int m_first;
  //int m_last;

  TFile* spillFile = new TFile("spills.root", "READ");
  TTree* spillTree = (TTree*)spillFile->Get("save");

  spillTree->SetBranchAddress("runID", &m_runID);
  spillTree->SetBranchAddress("spillID", &m_spillID);
  spillTree->SetBranchAddress("targetPos", &m_targetPos);
  //spillTree->SetBranchAddress("firstEvent", &m_first);
  //spillTree->SetBranchAddress("lastEvent", &m_last);

  for(int i = 0; i < spillTree->GetEntries(); ++i)
    {
      spillTree->GetEntry(i);
      if(m_runID != runID) continue;

      spillID[nSpills] = m_spillID;
      targetPos[nSpills] = m_targetPos;
      //first[nSpills] = m_first;
      //last[nSpills] = m_last;

      ++nSpills;
    }

  int nEvents = dataTree->GetEntries();
  cout << "Totally " << nEvents << " events in this run, " << nSpills << " spills are loaded ..." << endl;
  
  if(argc > 5) nEvents = atoi(argv[5]);
  for(int i = 0; i < nEvents; ++i)
    {
      //read the CODA data
      dataTree->GetEntry(i);

      int idx = -1;
      for(int j = 0; j < nSpills; ++j)
	{
	  /*
	  if(codaEvent->getPhysEventID() <= last[j] && codaEvent->getPhysEventID() >= first[j])
	    {
	      idx = j;
	      break;
	    }
	  */
	  if(codaEvent->getSpillID() == spillID[j])
	    {
	      idx = j;
	      break;
	    }
	}
      if(idx < 0 || idx >= nSpills) continue;

      //Get event level info
      rawEvent->setEventInfo(codaEvent->getRunID(), codaEvent->getSpillID(), codaEvent->getPhysEventID());
      rawEvent->setTargetPos(targetPos[idx]); //may need update later
      rawEvent->setTriggerBits(rawEvent->getTriggerBits());  //may need understand different between TS and my own definition
      rawEvent->setTurnID(codaEvent->getTurnID());
      rawEvent->setRFID(codaEvent->getRFID());
      rawEvent->setIntensity(codaEvent->getIntensityAll());

      //Get hit list
      int nHits = codaEvent->getNHitsAll();
      for(int j = 0; j < nHits; ++j)
	{
	  CODAHit h_coda = codaEvent->getHit(j);
	  if(h_coda.detectorID > 48 || h_coda.detectorID < 1) continue;

	  Hit h;
	  h.index = h_coda.index;
	  h.detectorID = h_coda.detectorID;
	  h.elementID = h_coda.elementID;
	  h.tdcTime = h_coda.tdcTime;
	  h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);
	  h.driftDistance = p_geomSvc->getDriftDistance(h.detectorID, h.tdcTime);
	  if(h.detectorID > 24 && h.detectorID <= 40)
	    {
	      if(h_coda.inTime > 0) h.setInTime(); 
	    }
	  else if(p_geomSvc->isInTime(h.detectorID, h.tdcTime)) 
	    {
	      h.setInTime();
	    }

	  rawEvent->insertHit(h);
	}

      //Get the trigger hit list
      /*
      int nTriggerHits = codaEvent->getNTriggerHits();
      for(int j = 0; j < nTriggerHits; ++j)
	{
	  CODAHit h_coda = codaEvent->getTriggerHit(j);
	  if(h_coda.detectorID < 25 || h_coda.detectorID > 40) continue;

	  Hit h;
	  h.index = h_coda.index;
	  h.detectorID = h_coda.detectorID;
	  h.elementID = h_coda.elementID;
	  h.tdcTime = h_coda.tdcTime;
	  h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);
	  h.driftDistance = 0.;
	  if(h_coda.inTime == 1) h.setInTime();

	  rawEvent->insertTriggerHit(h);
	}
      */
      rawEvent->reIndex();

      //Trigger emulation
      rawEvent->setTriggerEmu(p_triggerAna->acceptEvent(rawEvent, USE_HIT));
      int nRoads[4] = {p_triggerAna->getNRoadsPosTop(), p_triggerAna->getNRoadsPosBot(), p_triggerAna->getNRoadsNegTop(), p_triggerAna->getNRoadsNegBot()};
      rawEvent->setNRoads(nRoads);

      //Fill the raw data tree and flush
      saveTree->Fill();
      if(i % 1000 == 0) saveTree->AutoSave("SaveSelf");

      rawEvent->clear();
    }
  cout << endl;
  cout << "CODA to raw ends successfully." << endl;

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  delete p_geomSvc;
  delete p_triggerAna;

  return 1;
}
