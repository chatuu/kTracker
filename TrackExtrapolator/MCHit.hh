#ifndef MCHit_h
#define MCHit_h 1

// Based on ParN02 TrackerHit example

#include "G4Circle.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4VVisManager.hh"
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"

#include <vector>

using namespace std;

class MCHit : public G4VHit
{
  public:

    MCHit();
    ~MCHit();

    inline void* operator new(size_t);
    inline void  operator delete(void*);

    int particleID;
    G4String particleName;
    G4ThreeVector position;
    G4ThreeVector momentum;
    double dE;
    G4String volume;
    int digitizeID;
    int trackID;

    vector<G4String> detectorName;
    vector<int> elementID;
    vector<double> wirePosition;
    vector<int> rocID;
    vector<int> boardID;
    vector<int> channelID;
    vector<int> digitizeID2;

    vector<int> inTime;
    vector<int> masked;

    vector<double> driftTime;
    vector<double> driftDistance;
    vector<double> tdcTime;

    vector<double> triggerTdcTime;
    vector<int> triggerRocID;
    vector<int> triggerBoardID;
    vector<int> triggerChannelID;
    vector<bool> triggerHitPlane;

    vector<bool> lost;
    vector<bool> hitPlane;
    vector<long int> mHitID;
    vector<long int> hitID;

    vector<vector<long int> > deltaHitID;
    vector<vector<int> > deltaRay;
    vector<vector<int> > deltaInTime;
    vector<vector<int> > deltaMasked;
};

// vector collection of one type of hits
typedef G4THitsCollection<MCHit> MCHitsCollection;

extern G4Allocator<MCHit> MCHitAllocator;

inline void* MCHit::operator new(size_t)
{
  void* aHit;
  aHit = (void*) MCHitAllocator.MallocSingle();
  return aHit;
}

inline void MCHit::operator delete(void* aHit)
{
  MCHitAllocator.FreeSingle((MCHit*) aHit);
}

#endif
