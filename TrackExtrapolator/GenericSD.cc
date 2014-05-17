// Based on ExN02TrackerSD.hh from ParN02 example

#include "GenericSD.hh"

#include "G4SystemOfUnits.hh"

#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

#include <stdlib.h>

GenericSD::GenericSD(G4String name): G4VSensitiveDetector(name), staHitsCollection(0), f_repeatHits(0)
{
  G4String HCname;
  collectionName.insert(HCname="staHitsCollection");
}

GenericSD::~GenericSD()
{
}

void GenericSD::Initialize(G4HCofThisEvent* HCE)
{
  // todo - protect against repeat initialization?

  // We can leave these comments about memory leaks
  // and remove the WARNING spam

  staHitsCollection = new MCHitsCollection(SensitiveDetectorName, collectionName[0]); 
  static G4int HCID = -1;
  if(HCID<0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  HCE->AddHitsCollection(HCID, staHitsCollection);

  trackIDVector.resize(0);
  volumeVector.resize(0);
}

G4bool GenericSD::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
  return true;

  G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();

  // None of our detectors can see neutral particles

  if (particle->GetPDGCharge() == 0.)
    return false;

  G4Track* theTrack = aStep->GetTrack();

  // Make some inquiries about what this number should be

  if (theTrack->GetMomentum()[2] <= 1.0*MeV)
    return false;

  MCHit* newHit = new MCHit();

  newHit->momentum = theTrack->GetMomentum();
  newHit->particleID = particle->GetPDGEncoding();
  newHit->particleName = particle->GetParticleName();
  newHit->position = theTrack->GetPosition();
  newHit->trackID = theTrack->GetTrackID();
  newHit->dE = aStep->GetTotalEnergyDeposit();

  newHit->volume = theTrack->GetVolume()->GetName();

  // Parses the volume name to find the index for the digitization information
  // and the appropriate human readable name for the volume, i.e. H1X

  // This is rather awkward, and also has several string manips in a function that is called a lot.
  // Rather than attach in name, perhaps create an object that inherits from G4PVPlacement?
  // Would have to look at G4VPhysicalVolume.* and G4PVPlacement.* to do this correctly.
  // May not be possible.

  int split = newHit->volume.find("_");
  newHit->digitizeID = atoi(newHit->volume.substr(0, split).c_str());

  newHit->volume = newHit->volume.substr(split+1);
  split = newHit->volume.find("_phy");
  newHit->volume = newHit->volume.substr(0, split);

  // This piece of code rejects any hits with the same trackID and volume combination as a previous hit
  // in the same event.  The hit tables get large enough as it is.

  bool repeat = false;

  for (unsigned int i = 0; i < trackIDVector.size(); i++)
    if (trackIDVector[i] == newHit->trackID && volumeVector[i] == newHit->volume)
      repeat = true;

  if (repeat == false)
  {
    trackIDVector.push_back(newHit->trackID);
    volumeVector.push_back(newHit->volume);
    staHitsCollection->insert( newHit );
  }

  // repeat hits probably need more explicit treatment.  timing and pulse high can be affected. - Brian Tice
  // I believe these repeat hits are an oddity of Geant4's tracking, not a simulation of any repeat
  // hits that would happen in our detectors - Bryan Kerns

  if(repeat)
  {
    delete newHit;
    newHit = 0;
    ++f_repeatHits;
  }
  

  return true;
}

void GenericSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel>0)
  {
    G4int NbHits = staHitsCollection->entries();
    G4cout << "\n-------->Hits Collection: in this event there are " << NbHits
           << " hits at detector station 1: " << G4endl;
    for (G4int i=0;i<NbHits;i++)
      (*staHitsCollection)[i]->Print();
    G4cout << "\n-----------> There were " << f_repeatHits << " thrown away." << G4endl;
  } 
}
