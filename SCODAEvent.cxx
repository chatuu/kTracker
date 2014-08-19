 /*
SCODAEvent.cxx

Implimention of the class SCODAEvent

Author: Kun Liu, liuk@fnal.gov
Created: 10-24-2011
*/

#include <iostream>
#include <cmath>

#include <TRandom.h>
#include <TMath.h>
#include <TString.h>

#include "SCODAEvent.h"

ClassImp(CODAHit)
ClassImp(SCODAEvent)

bool CODAHit::operator<(const CODAHit& elem) const
{
  if(detectorID < elem.detectorID)
    {
      return true;
    }
  else if(detectorID > elem.detectorID)
    {
      return false;
    }
	  
  if(elementID < elem.elementID)
    {
      return true;
    }
  else if(elementID > elem.elementID)
    {
      return false;
    }

  if(tdcTime > elem.tdcTime)
    {
      return true;
    }
  else
    {
      return false;
    }
}


bool CODAHit::Hit_cmp_by_tdcTime(const CODAHit& tdcTime1, const CODAHit& tdcTime2)
{
  //  return  (tdcTime1.tdcTime>tdcTime2.tdcTime);


  if( tdcTime1.detectorID<tdcTime2.detectorID) return true;
  if( tdcTime1.detectorID==tdcTime2.detectorID) return ( tdcTime1.elementID<tdcTime2.elementID) ;
  if (( tdcTime1.detectorID==tdcTime2.detectorID) && ( tdcTime1.elementID==tdcTime2.elementID) ) return (tdcTime1.tdcTime> tdcTime2.tdcTime);
  return false;
}




bool CODAHit::sameChannel(const CODAHit& elem1, const CODAHit& elem2)
{
  if(elem1.detectorID == elem2.detectorID && elem1.elementID == elem2.elementID)
    {
      return true;
    }
  /*
  if(elem1.detectorID == elem2.detectorID && fabs(elem1.pos - elem2.pos) < 1E-3)
    {
      return true;
    }
  */
  return false;
}

SCODAEvent::SCODAEvent()
{
  fAllHits.clear();
  for(Int_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++)
    {
      fNHits[i] = 0;
      fNTriggerHits[i]=0;
    }


  fRunID = -1;
  fEventID = -1;
  fPhysEventID = -1;
  fSpillID = -1;
  fTriggerBits = 0;
  fV1495Error=0;
  fTriggerHits.clear();
  fAllHits.clear();
}

SCODAEvent::~SCODAEvent()
{
}

void SCODAEvent::setEventInfo(Int_t runID, Int_t spillID, Int_t eventID, Int_t PhysEventID)
{
  fRunID = runID;
  fEventID = eventID;
  fSpillID = spillID;
  fPhysEventID = PhysEventID;

}

void SCODAEvent::insertHit(CODAHit h)
{
  int selection=0;
  
  if (h.detectorID!=0){


    if (h.triggerLevel!=9){
      fTriggerHits.push_back(h);
      fNTriggerHits[h.detectorID]++;
      fNTriggerHits[0]++;
      selection=1;
    }else{//(h.triggerLevel==9)
      if( h.detectorID>48) {
	fTriggerHits.push_back(h);
	fNTriggerHits[h.detectorID]++;
	fNTriggerHits[0]++;
	
	fAllHits.push_back(h);
	fNHits[h.detectorID]++;
	fNHits[0]++;
	selection=2;
      }else {// h.detectorID<48
	//  if (h.detectorID==56)    printf("d:%s=%d,tL=%d , nhit=%d,ntrig=%d, all hit=%d, all trigger=%d\n",h.detectorName,h.detectorID,h.triggerLevel,fNHits[h.detectorID],fNTriggerHits[h.detectorID],fNHits[0],fNTriggerHits[0]);	
	fAllHits.push_back(h);
	fNHits[h.detectorID]++;
	fNHits[0]++;

	selection=3;
      }// h.detectorID<48
    }////(h.triggerLevel==9)


  }//h.detector==0??
  //  if (selection<=2) printf("rocID=%d,boardID=%x,channelID=%d, d:%s=%d,tL=%d ,s:%d, nhit=%d,ntrig=%d, all hit=%d, all trigger=%d\n",h.rocID,h.boardID,h.channelID, h.detectorName,h.detectorID,h.triggerLevel,selection,fNHits[h.detectorID],fNTriggerHits[h.detectorID],fNHits[0],fNTriggerHits[0]);	
}
Int_t SCODAEvent::findHit(Int_t detectorID, Int_t elementID)
{
  if(detectorID < 1 || detectorID > 48) return -1;
  if(elementID < 0) return -1;

  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      if(fAllHits[i].detectorID == detectorID && fAllHits[i].elementID == elementID)
	{
	  return i;
	}
    }

  return -1;
}

CODAHit SCODAEvent::getHit(Int_t detectorID, Int_t elementID)
{
  Int_t hitID = findHit(detectorID, elementID);
  if(hitID >= 0) return getHit(hitID);

  CODAHit dummy;
  dummy.index = -1;
  dummy.detectorID = -1;
  dummy.elementID = -1;
  return dummy;
}

std::list<Int_t> SCODAEvent::getHitsIndexInDetector(Int_t detectorID)
{
  std::list<Int_t> hit_list;
  hit_list.clear();

  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      if(fAllHits[i].detectorID != detectorID) continue;

      hit_list.push_back(i);
    }

  return hit_list;
}


std::list<Int_t> SCODAEvent::getHitsIndexInDetector(Int_t detectorID, Double_t x_exp, Double_t win)
{
  std::list<Int_t> hit_list;
  hit_list.clear();

  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      if(fAllHits[i].detectorID != detectorID) continue;
      //      if(fabs(fAllHits[i].pos - x_exp) > win) continue;

      hit_list.push_back(i);
    }

  return hit_list;
}

std::list<Int_t> SCODAEvent::getHitsIndexInSuperDetector(Int_t detectorID)
{
  std::list<Int_t> hit_list;
  hit_list.clear();

  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      if((fAllHits[i].detectorID != 2*detectorID) && (fAllHits[i].detectorID != 2*detectorID-1)) continue;

      hit_list.push_back(i);
    }

  return hit_list;
}

std::list<Int_t> SCODAEvent::getHitsIndexInDetectors(std::vector<Int_t>& detectorIDs)
{
  std::list<Int_t> hit_list;
  hit_list.clear();

  UInt_t nDetectors = detectorIDs.size();
  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      for(UInt_t j = 0; j < nDetectors; j++)
	{
	  if(fAllHits[i].detectorID == detectorIDs[j])
	    {
	      hit_list.push_back(i);
	      break;
	    }
	}
    }

  return hit_list;
}

std::list<Int_t> SCODAEvent::getAdjacentHitsIndex(CODAHit& _hit)
{
  std::list<Int_t> hit_list;
  hit_list.clear();

  Int_t detectorID = _hit.detectorID;
  Int_t detectorID_adj;
  if((detectorID/2)*2 == detectorID)
    {
      detectorID_adj = detectorID - 1;
    }
  else
    {
      detectorID_adj = detectorID + 1;
    }

  for(Int_t i = 0; i < fNHits[0]; i++)
    {
      if(fAllHits[i].detectorID == detectorID_adj && abs(fAllHits[i].elementID - _hit.elementID) <= 1)
	{
	  hit_list.push_back(i);
	}
    }

  return hit_list;
}



Int_t SCODAEvent::getNChamberHitsAll()
{
  Int_t nHits = 0;
  for(Int_t i = 1; i <= nChamberPlanes; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

Int_t SCODAEvent::getNHodoHitsAll()
{
  Int_t nHits = 0;
  for(Int_t i = nChamberPlanes+1; i <= nChamberPlanes+nHodoPlanes; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

Int_t SCODAEvent::getNPropHitsAll()
{
  Int_t nHits = 0;
  for(Int_t i = nChamberPlanes+nHodoPlanes+1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

Int_t SCODAEvent::getNHitsInDetectors(std::vector<Int_t>& detectorIDs)
{
  Int_t nHits = 0;
  UInt_t nDetectors = detectorIDs.size();
  for(UInt_t i = 0; i < nDetectors; i++)
    {
      for(Int_t j = 0; j <= nChamberPlanes+nHodoPlanes+nPropPlanes; j++)
	{
	  if(detectorIDs[i] == j)
	    {
	      nHits += fNHits[j];
	      break;
	    }
	}
    }

  return nHits;
}

Int_t SCODAEvent::getNHitsInD1()
{
  Int_t nHits = 0;
  for(Int_t i = 1; i <= 6; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

Int_t SCODAEvent::getNHitsInD2()
{
  Int_t nHits = 0;
  for(Int_t i = 7; i <= 12; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

Int_t SCODAEvent::getNHitsInD3p()
{
  Int_t nHits = 0;
  for(Int_t i = 13; i <= 18; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}


Int_t SCODAEvent::getNHitsInD3m()
{
  Int_t nHits = 0;
  for(Int_t i = 19; i <= 24; i++)
    {
      nHits += fNHits[i];
    }

  return nHits;
}

void SCODAEvent::reIndex(std::string option)
{
  ///Reset the number of hits on each plane
  for(Int_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++)
    {
      fNHits[i] = 0;
    }

  for(UInt_t i = 0; i < fAllHits.size(); i++)
    {
      fAllHits[i].index = i;
      fNHits[fAllHits[i].detectorID]++;
    }
  
  fNHits[0] = fAllHits.size();
  /*
 for(std::vector<Hit>::iterator iter = fAllHits.begin(); iter != fAllHits.end(); ++iter)
    {
      if (iter->detectorID ==11) printf("///passfinal//d:%s=%d,tL=%d ,element=%d, tdc=%f, inTime=%d\n",iter->detectorName,iter->detectorID,iter->triggerLevel,iter->elementID,iter->tdcTime,iter->inTime);

      }*/
}

void SCODAEvent::setEventInfo(SCODAEvent* event)
{
  //Set runID, eventID, spillID
  //  setEventInfo(event->getRunID(), event->getSpillID(), event->getEventID());
  setEventInfo(event->getRunID(), event->getSpillID(), event->getEventID(),0);

  //Set trigger bits
  setTriggerBits(event->getTriggerBits());

  //Set target position
  //  setTargetPos(event->getTargetPos());

  //Set bean info
  setTurnID(event->getTurnID());
  setRFID(event->getRFID());
  setIntensity(event->getIntensityAll());

  //Set the trigger emu info
  //setTriggerEmu(event->isEmuTriggered());
  //setNRoads(event->getNRoads());
}

void SCODAEvent::clear()
{
  fAllHits.clear();
  fTriggerHits.clear();
  for(Int_t i = 0; i < nChamberPlanes+1; i++)
    {
      fNHits[i] = 0;
      fNTriggerHits[i]= 0;
    }

  fRunID = -1;
  fSpillID = -1;
  fEventID = -1;
  fTriggerBits = 0;

}

void SCODAEvent::setTriggerBits(Int_t triggers[])
{
  for(int i = 0; i < 10; ++i)
    {
      if(triggers[i] == 0) continue;
      fTriggerBits |= triggerBit(i+1);
    }
}

void SCODAEvent::print()
{
  std::cout << "RunID: " << fRunID << ", EventID: " << fEventID << "===============" << std::endl;
  for(Int_t i = 1; i <= nChamberPlanes; i++)
    {
      std::cout << "Layer " << i << " has " << fNHits[i] << " hits." << std::endl;
    }
  std::cout << "===================================================================" << std::endl;

  return;
  for(std::vector<CODAHit>::iterator iter = fAllHits.begin(); iter != fAllHits.end(); ++iter)
    {
      iter->print();
    }
  std::cout << "===================================================================" << std::endl;
}
