/* This program sets up the magnetic field variables
Version 05/12/2008
Larry Isenhower modified by Aldo Raeliarijaona
*/

#include "Field.hh"

#include "TabulatedField3D.hh"
#include "Settings.hh"

#include "G4SystemOfUnits.hh"

using namespace std;

Field::Field(Settings* settings)
{
  mySettings = settings;

  Mag1Field= new TabulatedField3D(0.0*cm, 131, 121, 73, true, mySettings);
  Mag2Field= new TabulatedField3D(-1064.26*cm, 49, 37, 81, false, mySettings);

  //check that we read magnetic fields
  //throw exception?  will probably crash
  //must cast to TabulatedField3D because fields are stored as
  if( ! Mag1Field->IsOK() )
  {
    if( mySettings->asciiFieldMap )
      cout << "FATAL: Could not ascii file field map for kMag: " << mySettings->kMagName << endl;
    else
      cout << "FATAL: Could not get magnetic field map from database for kMag" << endl;
    delete Mag1Field;
    Mag1Field=0;
  }

  if( ! Mag2Field->IsOK() )
  {
    if( mySettings->asciiFieldMap )
      cout << "FATAL: Could not ascii file field map for fMag: " << mySettings->fMagName << endl;
    else
      cout << "FATAL: Could not get magnetic field map from database for fMag" << endl;

    delete Mag2Field;
    Mag2Field=0;
  }


  // These should probably be softcoded at some point, but doesn't matter as long as field maps don't get resized.

  zValues[0] = -204.0*cm;  // front of fmag field map
  zValues[1] = 403.74*cm;  // front of kmag field map
  zValues[2] = 712.0*cm;   // end of fmag field map
  zValues[3] = 1572.26*cm; // end of kmag field map
}

Field::~Field()
{
  if(Mag1Field)
    delete Mag1Field;
  if(Mag2Field)
    delete Mag2Field;
}

// This function looks up the magnetic field at a point, used by Geant4

void Field::GetFieldValue(const double Point[3], double *Bfield) const
{
  Bfield[0] = 0.;
  Bfield[1] = 0.;
  Bfield[2] = 0.;

  double xTemp = 0;
  double yTemp = 0;
  double zTemp = 0;

  if (Point[2]>zValues[0] && Point[2]<zValues[1])
  {
    Mag1Field->GetFieldValue( Point, Bfield );
  }

  if ((Point[2]>zValues[2])&&(Point[2]<zValues[3]))
  {
    Mag2Field->GetFieldValue( Point, Bfield );
  }

  if ((Point[2]>zValues[1])&&(Point[2]<zValues[2]))
  {
    Mag2Field->GetFieldValue( Point, Bfield );
    xTemp = Bfield[0];
    yTemp = Bfield[1];
    zTemp = Bfield[2];
    Mag1Field->GetFieldValue( Point, Bfield );
    Bfield[0] = Bfield[0] + xTemp;
    Bfield[1] = Bfield[1] + yTemp;
    Bfield[2] = Bfield[2] + zTemp;
  }
}
