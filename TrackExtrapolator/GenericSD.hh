#ifndef GenericSD_h
#define GenericSD_h 1

#include "G4VSensitiveDetector.hh"
#include <vector>
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4String.hh"

#include "MCHit.hh"

class G4Step;
class G4HCofThisEvent;

class GenericSD : public G4VSensitiveDetector
{
  public:
    explicit GenericSD(G4String);
    virtual ~GenericSD();

    void Initialize(G4HCofThisEvent*);
    G4bool ProcessHits(G4Step*, G4TouchableHistory*);
    void EndOfEvent(G4HCofThisEvent*);

  private:
    MCHitsCollection* staHitsCollection;
    G4int f_repeatHits;

  vector<int> trackIDVector;
  vector<G4String> volumeVector;

};

#endif
