// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// Code developed by:
//  S.Larsson and J. Generowicz.
//
//    *************************************
//    *                                   *
//    *    PurgMagTabulatedField3D.cc     *
//    *                                   *
//    *************************************
//
// $Id: PurgMagTabulatedField3D.cc,v 1.4 2006/06/29 16:06:25 gunter Exp $
// GEANT4 tag $Name: geant4-09-01-patch-02 $

#include "TabulatedField3D.hh"

#include "Settings.hh"

#include <fstream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <wordexp.h>

#include "G4MagneticField.hh"
#include "G4ios.hh"

#include "G4SystemOfUnits.hh"

using namespace std;

// This gets called by Field.cc, and reads in the values of the magnetic field and stores them

TabulatedField3D::TabulatedField3D(double zOffset, int nX, int nY, int nZ, bool fMagnet, Settings* settings) 
{
  mySettings = settings;

  m_isOK = true;
  if (mySettings->asciiFieldMap)
  {

    fmag = fMagnet;
    fZoffset = zOffset;
    nx = nX;
    ny = nY;
    nz = nZ;

    G4String filename = mySettings->magnetPath + "/";

    // We don't know exactly where these will be stored when moving these files to the grid and running jobs
    // $SEAQUEST_COMMON doesn't work in these circumstances
    // We must not remove the ability to change the file path in options

    if (fmag)
      filename.append(mySettings->fMagName);
    else
      filename.append(mySettings->kMagName);

    //expand environmental variables in filename
    wordexp_t exp_result;
    if (wordexp(filename.c_str(), &exp_result, 0) != 0)
    {
      cout << "Could not parse magnetic field map filename" << filename << endl;
      m_isOK = false;
    }

    G4String expandedFilename = exp_result.we_wordv[0];
    cout << "Filename expanded to " << expandedFilename << endl;

    double fieldUnit= tesla; 
    G4cout << "\n Reading the magnetic field map from " << expandedFilename << endl; 
    ifstream file( expandedFilename );
    if (!file.good())
    {
      cout << "Field map input file error." << endl;
      m_isOK = false;
    }

    char buffer[256];
    file.getline(buffer,256);

    // Set up storage space for table
    xField.resize( nx );
    yField.resize( nx );
    zField.resize( nx );
    int ix, iy, iz;
    for (ix=0; ix<nx; ix++)
    {
      xField[ix].resize(ny);
      yField[ix].resize(ny);
      zField[ix].resize(ny);
      for (iy=0; iy<ny; iy++)
      {
        xField[ix][iy].resize(nz);
        yField[ix][iy].resize(nz);
        zField[ix][iy].resize(nz);
      }
    }

    // Read in the data
    double xval,yval,zval,bx,by,bz;

    minx = miny = minz = maxx = maxy = maxz = 0;

    for (ix=0; ix<nx; ix++)
    {
      for (iy=0; iy<ny; iy++)
      {
        for (iz=0; iz<nz; iz++)
        {
          file >> xval >> yval >> zval >> bx >> by >> bz;
          if (xval*cm < minx)
            minx = xval * cm;
          if (yval*cm < miny)
            miny = yval * cm;
          if (zval*cm < minz)
            minz = zval * cm;
          if (xval*cm > maxx)
            maxx = xval * cm;
          if (yval*cm > maxy)
            maxy = yval * cm;
          if (zval*cm > maxz)
            maxz = zval * cm;
          xField[ix][iy][iz] = bx * fieldUnit;
          yField[ix][iy][iz] = by * fieldUnit;
          zField[ix][iy][iz] = bz * fieldUnit;
        }
      }
    }
    file.close();
  }
  else // if loading field map from MySQL, best to avoid if possible
  {
    fZoffset = zOffset;
    nx = nX;
    ny = nY;
    nz = nZ;

    MYSQL_RES* resLimits;
    MYSQL_RES* resField;
    MYSQL_ROW row;

    con = mysql_init(NULL);
    mysql_real_connect(con, mySettings->sqlServer, mySettings->login, mySettings->password, mySettings->magnetSchema, 
		       mySettings->mysqlPort, NULL, 0);
  
    fmag = fMagnet;

    double fieldUnit= tesla;

    // Set up storage space for table
    xField.resize( nx );
    yField.resize( nx );
    zField.resize( nx );
    int ix, iy;
    for (ix=0; ix<nx; ix++)
    {
      xField[ix].resize(ny);
      yField[ix].resize(ny);
      zField[ix].resize(ny);
      for (iy=0; iy<ny; iy++)
      {
        xField[ix][iy].resize(nz);
        yField[ix][iy].resize(nz);
        zField[ix][iy].resize(nz);
      }
    }

    float xval,yval,zval,bx,by,bz;

    if (fmag)
    {
      mysql_query(con, "SELECT MIN(X), MIN(Y), MIN(Z), MAX(X), MAX(Y), MAX(Z) FROM Fmag");
      cout << mysql_error(con) << endl;
      resLimits = mysql_store_result(con);
    }
    else
    {
      mysql_query(con, "SELECT MIN(X), MIN(Y), MIN(Z), MAX(X), MAX(Y), MAX(Z) FROM Kmag");
      cout << mysql_error(con) << endl;
      resLimits = mysql_store_result(con);
    }

    row = mysql_fetch_row(resLimits);

    minx = atof(row[0]) * cm;
    miny = atof(row[1]) * cm;
    minz = atof(row[2]) * cm;
    maxx = atof(row[3]) * cm;
    maxy = atof(row[4]) * cm;
    maxz = atof(row[5]) * cm;

    mysql_free_result(resLimits);

    if (fmag)
    {
      mysql_query(con, "SELECT x, y, z, B_x, B_y, B_z FROM Fmag");
      cout << mysql_error(con) << endl;
      resField = mysql_store_result(con);
    }
    else
    {
      mysql_query(con, "SELECT x, y, z, B_x, B_y, B_z FROM Kmag");
      cout << mysql_error(con) << endl;
      resField = mysql_store_result(con);
    }

    int xc, yc, zc;

    while ((row = mysql_fetch_row(resField)))
    {
      xval = atof(row[0]);
      yval = atof(row[1]);
      zval = atof(row[2]);
      bx = atof(row[3]);
      by = atof(row[4]);
      bz = atof(row[5]);

      xc = floor((xval*cm-minx)*(nx-1)/(maxx-minx)+0.5);
      yc = floor((yval*cm-miny)*(ny-1)/(maxy-miny)+0.5);
      zc = floor((zval*cm-minz)*(nz-1)/(maxz-minz)+0.5);

      xField[xc][yc][zc] = bx*fieldUnit;
      yField[xc][yc][zc] = by*fieldUnit;
      zField[xc][yc][zc] = bz*fieldUnit;
    }

    mysql_free_result(resField);
    mysql_close(con);
  }

  // This code is run whether it is loaded from ascii or MySQL

  dx = maxx - minx;
  dy = maxy - miny;
  dz = maxz - minz;
}

// This calculates the magnetic field at a point within this map

void TabulatedField3D::GetFieldValue(const double point[3], double *Bfield ) const
{
  Bfield[0] = 0.0;
  Bfield[1] = 0.0;
  Bfield[2] = 0.0;

  double x, y, z;

  x = point[0];
  y = point[1];
  z = point[2] + fZoffset;

  // Check that the point is within the defined region 
  if ( x>=minx && x<=maxx &&
       y>=miny && y<=maxy && 
       z>=minz && z<=maxz )
  {    
    // Position of given point within region, normalized to the range
    // [0,1]
    double xfraction = (x - minx) / dx;
    double yfraction = (y - miny) / dy; 
    double zfraction = (z - minz) / dz;

    // Need addresses of these to pass to modf below.
    // modf uses its second argument as an OUTPUT argument.
    double xdIndex, ydIndex, zdIndex;
    
    // Position of the point within the cuboid defined by the
    // nearest surrounding tabulated points
    double xlocal = ( std::modf(xfraction*(nx-1), &xdIndex));
    double ylocal = ( std::modf(yfraction*(ny-1), &ydIndex));
    double zlocal = ( std::modf(zfraction*(nz-1), &zdIndex));
    
    // The indices of the nearest tabulated point whose coordinates
    // are all less than those of the given point
    int xindex = static_cast<int>(xdIndex);
    int yindex = static_cast<int>(ydIndex);
    int zindex = static_cast<int>(zdIndex);
    
    // Full 3-dimensional version
    Bfield[0] =
      xField[xindex  ][yindex  ][zindex  ] * (1-xlocal) * (1-ylocal) * (1-zlocal) +
      xField[xindex  ][yindex  ][zindex+1] * (1-xlocal) * (1-ylocal) *    zlocal  +
      xField[xindex  ][yindex+1][zindex  ] * (1-xlocal) *    ylocal  * (1-zlocal) +
      xField[xindex  ][yindex+1][zindex+1] * (1-xlocal) *    ylocal  *    zlocal  +
      xField[xindex+1][yindex  ][zindex  ] *    xlocal  * (1-ylocal) * (1-zlocal) +
      xField[xindex+1][yindex  ][zindex+1] *    xlocal  * (1-ylocal) *    zlocal  +
      xField[xindex+1][yindex+1][zindex  ] *    xlocal  *    ylocal  * (1-zlocal) +
      xField[xindex+1][yindex+1][zindex+1] *    xlocal  *    ylocal  *    zlocal ;
    Bfield[1] =
      yField[xindex  ][yindex  ][zindex  ] * (1-xlocal) * (1-ylocal) * (1-zlocal) +
      yField[xindex  ][yindex  ][zindex+1] * (1-xlocal) * (1-ylocal) *    zlocal  +
      yField[xindex  ][yindex+1][zindex  ] * (1-xlocal) *    ylocal  * (1-zlocal) +
      yField[xindex  ][yindex+1][zindex+1] * (1-xlocal) *    ylocal  *    zlocal  +
      yField[xindex+1][yindex  ][zindex  ] *    xlocal  * (1-ylocal) * (1-zlocal) +
      yField[xindex+1][yindex  ][zindex+1] *    xlocal  * (1-ylocal) *    zlocal  +
      yField[xindex+1][yindex+1][zindex  ] *    xlocal  *    ylocal  * (1-zlocal) +
      yField[xindex+1][yindex+1][zindex+1] *    xlocal  *    ylocal  *    zlocal ;
    Bfield[2] =
      zField[xindex  ][yindex  ][zindex  ] * (1-xlocal) * (1-ylocal) * (1-zlocal) +
      zField[xindex  ][yindex  ][zindex+1] * (1-xlocal) * (1-ylocal) *    zlocal  +
      zField[xindex  ][yindex+1][zindex  ] * (1-xlocal) *    ylocal  * (1-zlocal) +
      zField[xindex  ][yindex+1][zindex+1] * (1-xlocal) *    ylocal  *    zlocal  +
      zField[xindex+1][yindex  ][zindex  ] *    xlocal  * (1-ylocal) * (1-zlocal) +
      zField[xindex+1][yindex  ][zindex+1] *    xlocal  * (1-ylocal) *    zlocal  +
      zField[xindex+1][yindex+1][zindex  ] *    xlocal  *    ylocal  * (1-zlocal) +
      zField[xindex+1][yindex+1][zindex+1] *    xlocal  *    ylocal  *    zlocal ;
  }

  if (fmag)
  {
    Bfield[0] = Bfield[0]*mySettings->fMagMultiplier;
    Bfield[1] = Bfield[1]*mySettings->fMagMultiplier;
    Bfield[2] = Bfield[2]*mySettings->fMagMultiplier;
  }
  else
  {
    Bfield[0] = Bfield[0]*mySettings->kMagMultiplier;
    Bfield[1] = Bfield[1]*mySettings->kMagMultiplier;
    Bfield[2] = Bfield[2]*mySettings->kMagMultiplier;
  }
}
  

bool TabulatedField3D::IsOK() const
{
  return m_isOK;
}
