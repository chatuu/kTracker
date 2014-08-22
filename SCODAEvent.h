/*
SCODAEvent.h

Definition of the class SCODAEvent, which essentially works as a 
container of the raw hits information. It also provides serveral 
query utility to retrieve the hit list from a specific plane, etc.

Author: Grass Wang, agrassfish@gmail.com
Created: 07-02-2012
*/

#ifndef _SCODAEVENT_H
#define _SCODAEVENT_H

//#include "MODE_SWITCH.h"
#define nChamberPlanes 24
#define nHodoPlanes 16
#define nPropPlanes 8
#define nV1495Planes 32

#include <iostream>
#include <vector>
#include <list>
#include <string>

#include <TObject.h>
#include <TROOT.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#define triggerBit(n) (1 << (n))

///Definition of hit structure
class CODAHit: public TObject
{
public:
  Int_t index;         //unique index for identification
  Int_t detectorID;    //assigned for each detector plane
  Int_t elementID;     
  Char_t detectorName[20];
  
  Int_t triggerLevel;

  Double_t tdcTime;    //raw TDC time

  Int_t rocID;
  Int_t boardID;
  Int_t channelID;
  Int_t inTime;        //In-time flag

  //overiden comparison operator for track seeding 
  bool operator<(const CODAHit& elem) const;
  bool operator>(const CODAHit& elem) const { return detectorID > elem.detectorID; }
  bool operator==(const CODAHit& elem) const { return detectorID == elem.detectorID; }

  static bool Hit_cmp_by_tdcTime(const CODAHit& tdcTime1, const CODAHit& tdcTime2);

  //comparison function for after pulse removal
  static bool sameChannel(const CODAHit& elem1, const CODAHit& elem2);

  //Debugging output
  void print() { std::cout << index << " : " << detectorID << " : " << elementID << " : " << inTime << std::endl; }

  ClassDef(CODAHit, 6)
};

class SCODAEvent: public TObject
{
public:
  SCODAEvent();
  ~SCODAEvent();

  ///Gets
  std::list<Int_t> getHitsIndexInDetector(Int_t detectorID);
  std::list<Int_t> getHitsIndexInDetector(Int_t detectorID, Double_t x_exp, Double_t win);
  std::list<Int_t> getHitsIndexInSuperDetector(Int_t detectorID);
  std::list<Int_t> getHitsIndexInDetectors(std::vector<Int_t>& detectorIDs);
  std::list<Int_t> getAdjacentHitsIndex(CODAHit& _hit);

  Int_t getNHitsAll() { return fNHits[0]; }
  Int_t getNTriggerHits() { return fNTriggerHits[0]; }
  Int_t getNChamberHitsAll();
  Int_t getNHodoHitsAll();
  Int_t getNPropHitsAll();
  Int_t getNHitsInD1();
  Int_t getNHitsInD2();
  Int_t getNHitsInD3p();
  Int_t getNHitsInD3m();

  Int_t getNTriggerHitsInDetector(Int_t detectorID) { return fNTriggerHits[detectorID]; }
  Int_t getNHitsInDetector(Int_t detectorID) { return fNHits[detectorID]; }
  Int_t getNHitsInSuperDetector(Int_t detectorID) { return fNHits[2*detectorID-1] + fNHits[2*detectorID]; }
  Int_t getNHitsInDetectors(std::vector<Int_t>& detectorIDs);
  
  std::vector<CODAHit> getAllHits() { return fAllHits; }
  std::vector<CODAHit> getTriggerHits() { return fTriggerHits; }
  CODAHit getTriggerHit(Int_t index) { return fTriggerHits[index]; } 
  CODAHit getHit(Int_t index) { return fAllHits[index]; } 
  CODAHit getHit(Int_t detectorID, Int_t elementID); 
  void setHit(Int_t index, CODAHit hit) { fAllHits[index] = hit; }
  void setTriggerHit(Int_t index, CODAHit hit) { fTriggerHits[index] = hit; }

  Int_t getRunID() { return fRunID; }
  Int_t getEventID() { return fEventID; }
  Int_t getPhysEventID() { return fPhysEventID; }
  Int_t getSpillID() { return fSpillID; }
  Int_t getV1495Error() { return fV1495Error; }
  
  ///Sets
  void setEventInfo(Int_t runID, Int_t spillID, Int_t eventID, Int_t PhysicsEventID);

  ///Insert a new hit
  void insertHit(CODAHit h);
  void insertTriggerHit(CODAHit h) { fTriggerHits.push_back(h); }
  
  ///Find a hit
  Int_t findHit(Int_t detectorID, Int_t elementID);

  void reIndex(std::string option = "");

  ///Set/get the trigger types
  Int_t getTriggerBits() { return fTriggerBits; }
  void setTriggerBits(Int_t triggers[]);
  void setTriggerBits(Int_t triggers) { fTriggerBits = triggers; }
  bool isTriggeredBy(Int_t trigger) { return (fTriggerBits & trigger) != 0; }

  //Set/get the beam info
  Int_t getTurnID() { return fTurnID; }
  Int_t getRFID() { return fRFID; }
  Int_t getIntensity() { return fIntensity[16]; }
  Int_t getIntensity(Int_t i) { return fIntensity[i+16]; }
  Int_t getIntensitySumBefore(Int_t n = 16) { Int_t sum = 0; for(Int_t i = n; i < 16; ++i) sum += fIntensity[i]; return sum; } 
  Int_t getIntensitySumAfter(Int_t n = 16) { Int_t sum = 0; for(Int_t i = 16; i < n+16; ++i) sum += fIntensity[i]; return sum; } 
  Int_t* getIntensityAll() { return fIntensity; }

  void setV1495Error(Int_t v1495Error) { fV1495Error = v1495Error; }
  void setTurnID(Int_t turnID) { fTurnID = turnID; }
  void setRFID(Int_t rfID) { fRFID = rfID; }
  void setIntensity(const Int_t intensity[]) { for(Int_t i = 0; i < 33; ++i) fIntensity[i] = intensity[i]; }
  void setIntensity(Int_t i, Int_t val) { fIntensity[i] = val; }
  void setIntensity(Int_t val) { fIntensity[16] = val; }

  //Set the event info from another event
  void setEventInfo(SCODAEvent* event);

  ///Clear the internal event structure
  void clear();

  ///Print for debugging purposes
  void print(); 

public:
  //Trigger type
  enum TriggerType 
    {
      NIM1 = triggerBit(5),
      NIM2 = triggerBit(6),
      NIM3 = triggerBit(7),
      NIM4 = triggerBit(8),
      NIM5 = triggerBit(9),
      MATRIX1 = triggerBit(0),
      MATRIX2 = triggerBit(1),
      MATRIX3 = triggerBit(2),
      MATRIX4 = triggerBit(3),
      MATRIX5 = triggerBit(4)
    };

private:
  //RunID, spillID, eventID
  Int_t fRunID;
  Int_t fEventID;
  Int_t fPhysEventID;
  Int_t fSpillID;
  Int_t fV1495Error;
  
  //Trigger bit
  Int_t fTriggerBits;

  //Beam intensity information
  Int_t fTurnID;
  Int_t fRFID;
  Int_t fIntensity[33];   //16 before, one onset, and 16 after

  ///Hits of this event
  Int_t fNTriggerHits[nChamberPlanes+nHodoPlanes+nPropPlanes+9+1];
  Int_t fNHits[nChamberPlanes+nHodoPlanes+nPropPlanes+9+1];  //0 for all hits, 1, 2, ..., 24 for number of hits in plane 1, 2, ..., 24
  std::vector<CODAHit> fAllHits;
  std::vector<CODAHit> fTriggerHits;

  ClassDef(SCODAEvent, 10)
};

#endif
