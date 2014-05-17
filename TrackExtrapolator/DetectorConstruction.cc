// This file performs the main fucntion of constructing and configuring
// the physical model of the experiment used in the GMC.  Even though it
// is called DetectorConstruction it constructs all of the geometry, detectors, 
// magnets, magnetic fields, concrete barriers, target, etc.  It uses a MySQL
// database to construct the shapes, sizes, positions and materials of the
// experiment components, set up magnetic fields, set display attributes of
// the physical componets, and assign which components will be detectors.

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"

#include "Settings.hh"

#include "G4SystemOfUnits.hh"

using namespace std;

DetectorConstruction::DetectorConstruction(Settings* settings)
{
  mySettings = settings;
}

DetectorConstruction::~DetectorConstruction()
{
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  con = mysql_init(NULL);
  mysql_real_connect(con, mySettings->sqlServer, mySettings->login, mySettings->password, 
                     mySettings->geometrySchema, mySettings->mysqlPort, NULL, 0);

  G4SDManager* SDman = G4SDManager::GetSDMpointer(); 
  G4String sta_SDname = "E906/StationSD";
  GenericSD* staSD = new GenericSD(sta_SDname);
  SDman->AddNewDetector(staSD);

  /*    The entire geometry, with the exception of the magnetic fields, are built from 5 different Geant Objects.

        Elements (G4Element) are things like carbon and oxygen, but you can define seperate elements
        for seperate isotopes of the same element if you wish.  The database has a liquid deuterium and
        a hydrogen entry, for example.  To define an element you need an atomic number (z), and its
        mass per mole, and a name.

        Materials (G4Material) are built from combinations of one or more elements.  To define a material,
        you need the number of elements in the material, the pointers to those elements, and the fraction by mass
        for each element.  You also need to give a density and a name for the material.

        Solids (G4VSolid) are the shapes that make up your geometry.  To define one you give the shape
        (i.e. cylinder, box) and the spatial dimensions.  More complex solids can be made by combining
        simpler ones, either by adding them together or by using one solid to define a hole in another
        solid.

        Logical Volumes (G4LogicalVolume) are the combination of a Solid with a Material.  Multiple
        Logical Volumes can use the same Solid.

        Physical Volumes (G4VPhysicalVolume) are what are actually placed within the geometry.  They are
        made from a logical volume, placed within a different logical volume(called its mother volume), and
        given a position and rotation within its mother volume.  Multiple Physical Volumes can use the 
        same Logical Volume.  The World Volume is a special Physical Volume; its Logical Volume contains 
        all the other Physical Volumes and it does not have a mother volume.  The World Volume is what this
        routine returns.

        This routine downloads the information for all of these Geant4 objects and loads them into vectors.

        At various points certain variables like target length are stored to send to PrimaryGeneratorAction.
        Pointers to the target material and magnet material are also stored so their materials can be changed
        through the UI if desired.
        */

  MYSQL_RES* res;
  MYSQL_ROW row;

  double z, n, a;
  int id;
  G4String name, symbol;

  mysql_query(con, "SELECT eID, eName, symbol, z, n FROM Elements");
  if (mysql_errno(con) != 0)
  {
    cout << "Elements query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);
  int nElement = mysql_num_rows(res);
  elementVec.resize(nElement+1);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)elementVec.size())
      elementVec.resize(id+10);
    name = row[1];
    symbol = row[2];
    z = atof(row[3]);
    n = atof(row[4]);
    a = n * gram;
    elementVec[id] = new G4Element(name, symbol, z, a);
  }

  mysql_free_result(res);

  double density;
  int numEle;
  vector<double> perAbundance;
  perAbundance.resize(11);
  vector<G4String> eleName;
  eleName.resize(11);
  vector<int> eID;
  eID.resize(11);

  //  This query might have to be expanded if materials with more elements are made.
  //  This is inelegant, most of the fields are usually NULL, but I don't know a better way to do it.
  //  Perhaps have all the eIDs and percents in a MySQL VarChar or blob field and parse the string?
  //  It works fine, so not a priority.

  //  ALL numbers downloaded through MySQL have to be multiplied by the correct unit!
  //  If the MySQL table stores lengths in cm, then multiply the result from the query by cm before giving it
  //  to Geant4.  Geant4 knows most standard metric units.
  //  i.e.  G4Object.length = atof(row[1]) * cm;

  //  A list of units can be found:
  //  http://geant4.web.cern.ch/geant4/reports/gallery/electromagnetic/units/SystemOfUnits.h.html

  mysql_query(con, "SELECT mID, mName, density, numElements, eID1, eID2, eID3, eID4, eID5, "
      "eID6, eID7, eID8, eID9, eID10, elementPercent1, elementPercent2, "
      "elementPercent3, elementPercent4, elementPercent5, elementPercent6, "
      "elementPercent7, elementPercent8, elementPercent9, elementPercent10 FROM Materials");
  if (mysql_errno(con) != 0)
  {
    cout << "Materials query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);
  int nMaterial = mysql_num_rows(res);
  materialVec.resize(nMaterial+1);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)materialVec.size())
      materialVec.resize(id+10);
    name = row[1];
    density = atof(row[2])*g/(cm*cm*cm);
    numEle = atoi(row[3]);
    for (int i = 1; i <= numEle; i++)
    {
      eID[i] = atoi(row[3+i]);
      perAbundance[i] = atof(row[13+i]);
    }

    materialVec[id] = new G4Material(name, density, numEle);

    for (int j = 1; j <= numEle; j++)
      materialVec[id]->AddElement(elementVec[eID[j]],perAbundance[j]/100.0);
  }

  mysql_free_result(res);

  double xLength, yLength, zLength, radiusMin, radiusMax;
  int shellID, holeID;

  //  There are multiple variations of G4VSolid, such as G4Box and G4Tubs.  These are stored in different 
  //  MySQL tables

  mysql_query(con, "SELECT sID, sName, xLength, yLength, zLength FROM SolidBoxes");
  if (mysql_errno(con) != 0)
  {
    cout << "SolidBoxes query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    xLength = atof(row[2])*cm;
    yLength = atof(row[3])*cm;
    zLength = atof(row[4])*cm;

    solidVec[id] = new G4Box(name, xLength/2.0, yLength/2.0, zLength/2.0);
  }
  mysql_free_result(res);

  mysql_query(con, "SELECT sID, sName, length, radiusMin, radiusMax FROM SolidTubes");
  if (mysql_errno(con) != 0)
  {
    cout << "SolidTubes query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    zLength = atof(row[2])*cm;
    radiusMin = atof(row[3])*cm;
    radiusMax = atof(row[4])*cm;

    solidVec[id] = new G4Tubs(name, radiusMin, radiusMax, zLength/2.0, 0, 360*deg);
  }
  mysql_free_result(res);

  mysql_query(con, "SELECT sID, sName, shellID, holeID, rotX, rotY, rotZ, posX, posY, posZ FROM SubtractionSolids");
  if (mysql_errno(con) != 0)
  {
    cout << "SubtractionSolids query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    shellID = atoi(row[2]);
    holeID = atoi(row[3]);

    G4RotationMatrix ra;
    ra.rotateX(atof(row[4])*radian);
    ra.rotateY(atof(row[5])*radian);
    ra.rotateZ(atof(row[6])*radian);
    G4ThreeVector ta;
    ta.setX(atof(row[7])*cm);
    ta.setY(atof(row[8])*cm);
    ta.setZ(atof(row[9])*cm);
    G4Transform3D rata;
    rata = G4Transform3D(ra,ta);

    solidVec[id] = new G4SubtractionSolid(name, solidVec[shellID], solidVec[holeID], rata);
  }
  mysql_free_result(res);

  mysql_query(con, "SELECT vID, red, green, blue, alpha, visible FROM VisAttributes");
  if (mysql_errno(con) != 0)
  {
    cout << "VisAttributes query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);

    if ((int) visAttributesVec.size() < id + 1)
      visAttributesVec.resize(id+1);

    double red = atoi(row[1]);
    double green = atoi(row[2]);
    double blue = atoi(row[3]);
    double alpha = atoi(row[4]);

    bool visible = atoi(row[5]);

    G4Colour* color = new G4Colour(red, green, blue, alpha);

    visAttributesVec[id] = new G4VisAttributes(*color);
    visAttributesVec[id]->SetVisibility(visible);
  }
  mysql_free_result(res);

  int sID, mID;

  mysql_query(con, "SELECT lvID, lvName, sID, mID, sensitiveDetector, vID FROM LogicalVolumes");
  if (mysql_errno(con) != 0)
  {
    cout << "LogicalVolumes query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);
  int nLogicalVolume = mysql_num_rows(res);
  logicalVolumeVec.resize(nLogicalVolume);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)logicalVolumeVec.size())
      logicalVolumeVec.resize(id+10);
    name = row[1];
    sID = atoi(row[2]);
    mID = atoi(row[3]);
    bool sd = atoi(row[4]);
    int vID = atoi(row[5]);

    logicalVolumeVec[id] = new G4LogicalVolume(solidVec[sID], materialVec[mID], name);

    if (sd)
      logicalVolumeVec[id]->SetSensitiveDetector(staSD);

    logicalVolumeVec[id]->SetVisAttributes(visAttributesVec[vID]);
  }
  mysql_free_result(res);

  int logicalID, motherID;
  G4ThreeVector pos;

  vector<int> copy;
  copy.resize((int)logicalVolumeVec.size());

  for (int i = 0; i<(int)copy.size(); i++)
    copy[i] = 0;

  mysql_query(con, "SELECT MAX(depth) FROM PhysicalVolumes");
  if (mysql_errno(con) != 0)
  {
    cout << "PhysicalVolumes max depth query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);
  row = mysql_fetch_row(res);
  int depth = atoi(row[0]);

  rotationMatrixVec.resize(0);

  mysql_query(con, "SELECT pvID, lvID FROM PhysicalVolumes WHERE depth = 0");
  if (mysql_errno(con) != 0)
  {
    cout << "PhysicalVolumes depth=0 query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  if ((row = mysql_fetch_row(res)))
  {
    logicalID = atoi(row[1]);
    physiWorld = new G4PVPlacement(0, G4ThreeVector(), "World", logicalVolumeVec[logicalID], 0, false, 0);
    copy[logicalID]++;
  }
  mysql_free_result(res);

  char qry[5000];

  int j = 0;

  for (int i = 1; i <= depth; i++)
  {
    sprintf(qry, "SELECT pvID, pvName, lvID, motherID, xRel, yRel, zRel, rotX, rotY, rotZ, sensitiveDetector "
        "FROM PhysicalVolumes WHERE depth = %i", i);
    mysql_query(con, qry);

    if (mysql_errno(con) != 0)
    {
      cout << "PhysicalVolumes query" << endl << endl;
      cout << mysql_error(con) << endl;
    }
    res = mysql_store_result(con);

    while ((row = mysql_fetch_row(res)))
    {
      id = atoi(row[0]);
      name = row[1];
      logicalID = atoi(row[2]);
      motherID = atoi(row[3]);
      pos(0) = atof(row[4])*cm;
      pos(1) = atof(row[5])*cm;
      pos(2) = atof(row[6])*cm;

      rotationMatrixVec.push_back(new G4RotationMatrix());
      rotationMatrixVec.back()->rotateZ(atof(row[9])*radian);
      rotationMatrixVec.back()->rotateY(atof(row[8])*radian);
      rotationMatrixVec.back()->rotateX(atof(row[7])*radian);

      // This puts a number in front of the name of the Physical Volume
      // This is used when a particle causes a hit, parsing the string returns the number
      // which is the index of the appropriate set of digitization information

      if (atoi(row[10]) == 1)
      {
        int split = name.find("_phy");
        G4String newName = name.substr(0, split);
        detectorNameVec.push_back(newName);

        name.insert(0,"_");
        char number[10];
        sprintf(number, "%i", j);
        name.insert(0, number);
        j++;
      }

      id = atoi(row[0]);
      if (id >= (int)placementVec.size())
        placementVec.resize(id+10);

      placementVec[id] = new G4PVPlacement(rotationMatrixVec.back(), pos, logicalVolumeVec[logicalID], name, 
                                           logicalVolumeVec[motherID], 0, copy[logicalID]);

      copy[logicalID]++;
    }
    mysql_free_result(res);
  }

#ifdef MC_MODE
  sprintf(qry, "SELECT tID, length, radius, zPos, mID, num_pieces, spacing, vID "
               "FROM TargetInfo WHERE tID = %i", mySettings->target);
  mysql_query(con, qry);

  if (mysql_errno(con) != 0)
  {
    cout << "TargetInfo query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  if((row = mysql_fetch_row(res)))
  {
    int tID = atoi(row[0]);
    targetLength = atof(row[1])*cm;
    targetRadius = atof(row[2])*cm;
    targetCenter = atof(row[3])*cm;
    int mID = atoi(row[4]);
    targetNumPieces = atoi(row[5]);
    targetSpacing = 0;
    if (targetNumPieces > 1)
      targetSpacing = atof(row[6])*cm;
    int vID = atoi(row[7]);

    targetMat = materialVec[mID];

    targetSolid = new G4Tubs("target_sol", 0, targetRadius, targetLength/2.0, 0, 360*deg);

    targetVol = new G4LogicalVolume(targetSolid, targetMat, "target_vol");

    targetVol->SetVisAttributes(visAttributesVec[vID]);

    for (unsigned int i = 0; i < targetNumPieces; i++)
    {
      pos = G4ThreeVector(0, 0, targetCenter + i*targetSpacing - (targetNumPieces-1)/2.0);
      char phyName[50];
      sprintf(phyName, "target_piece_%i_phy", i);
      targetPhyVec.push_back(new G4PVPlacement(new G4RotationMatrix(), pos, targetVol, phyName, 0, 0, i));
    }
  }
  else
    cout << "No target loaded!" << endl;
  mysql_free_result(res);
#endif
  mysql_query(con, "SELECT * FROM ConstantsDerived");
  if (mysql_errno(con) != 0)
  {
    cout << "ConstantsDerived query" << endl << endl;
    cout << mysql_error(con) << endl;
  }
  res = mysql_store_result(con);

  char constant[30];
  while ((row = mysql_fetch_row(res)))
  {
    sprintf(constant, row[0]);
    if (!strcmp(constant, "fmagLength"))
      fmagLength = atof(row[1])*cm;
    else if (!strcmp(constant, "fmagCenter"))
      fmagCenter = atof(row[1])*cm;
    else if (!strcmp(constant, "fmagMaterialID"))
      defaultMagnetMat = materialVec[atoi(row[1])];
    else if (!strcmp(constant, "fmagVolumeID"))
      magnetVolume = logicalVolumeVec[atoi(row[1])];
  }
  mysql_free_result(res);

  mysql_close(con);

  // Magnetic Field
  // For details see Field and TabulatedField3D .cc and .hh

  Field* myField = new Field(mySettings);
  G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(myField);

  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(myField);
  G4MagIntegratorStepper* pStepper = new G4ClassicalRK4(fEquation);  // (fEquation) or (fEquation, 8)?
  G4ChordFinder* pChordFinder = new G4ChordFinder(myField, 0.01*mm, pStepper);
  fieldMgr->SetChordFinder(pChordFinder);

  // Uncomment to print the material table to screen, for debugging purposes

/*
     G4cout << *(G4Element::GetElementTable()) << endl;
     G4cout << *(G4Material::GetMaterialTable()) << endl;
*/

  return physiWorld;
}

// This is called if someone changes the magnet material through the UI

void DetectorConstruction::IronToggle(bool iron)
{
  G4NistManager* man = G4NistManager::Instance();
  G4Material* air  = man->FindOrBuildMaterial("Air");

  if (iron == true)
    magnetVolume->SetMaterial(defaultMagnetMat);
  else
    magnetVolume->SetMaterial(air);

  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

//  This assembles the information used to for digitizing the hits.  Geant4 does have some built in functions for
//  digitization, but those are not used in this program.  It might be slightly more efficient to use those functions
//  but this works well enough that such a change is not a priority

void DetectorConstruction::AssembleDigiPlanes()
{
  con = mysql_init(NULL);
  mysql_real_connect(con, mySettings->sqlServer, mySettings->login, mySettings->password, mySettings->outputFileName, 
                     mySettings->mysqlPort, NULL, 0);

  MYSQL_RES *res;
  MYSQL_ROW row;

  char qry[1000];
  G4String name;

  digiPlaneVec.resize(0);

  for (int j = 0; j < (int) detectorNameVec.size(); j++)
  {
    name = detectorNameVec[j];

    if ((int) digiPlaneVec.size() < j + 1)
      digiPlaneVec.resize(j+1);
    digiPlaneVec[j].resize(0);

    sprintf(qry, "SELECT detectorName, spacing, cellWidth, overlap, numElements, lowElementID, "
        "planeWidth, planeHeight, xPrimeOffset, angle, x0, y0, z0, u_x, u_y, u_z, "
        "v_x, v_y, v_z, h_x, h_y, h_z, w_x, w_y, w_z, n_x, n_y, n_z, efficiency "
        "FROM tempDigiInfo WHERE geantName = \'%s\'", name.c_str());
    mysql_query(con, qry);
    if (mysql_errno(con) != 0)
    {
      cout << qry << endl;
      cout << mysql_error(con) << endl;
    }
    res = mysql_store_result(con);

    while ((row = mysql_fetch_row(res)))
    {
      DigiPlane dp;

      // the u vector is a unit vector in the detector plane in the measuring direction
      // the v vector is a unit vector that points in the direction of the wires
      // the n vector is a unit vector perpindicular to u and v
      // the w vector is a unit vector that points in the direction of the horizontal part of the detector frame
      // the h vector is a unit vector that points in the direction of the vertical part of the detector frame

      dp.detectorName = row[0];
      dp.spacing = atof(row[1])*cm;
      dp.cellWidth = atof(row[2])*cm;
      dp.overlap = atof(row[3])*cm;
      dp.numElements = atoi(row[4]);
      dp.lowElementID = atoi(row[5]);
      dp.planeWidth = atof(row[6])*cm;
      dp.planeHeight = atof(row[7])*cm;
      dp.xPrimeOffset = atof(row[8])*cm;
      dp.angle = atof(row[9])*radian;
      dp.x0 = atof(row[10])*cm;
      dp.y0 = atof(row[11])*cm;
      dp.z0 = atof(row[12])*cm;
      dp.u_x = atof(row[13]);
      dp.u_y = atof(row[14]);
      dp.u_z = atof(row[15]);
      dp.v_x = atof(row[16]);
      dp.v_y = atof(row[17]);
      dp.v_z = atof(row[18]);
      dp.h_x = atof(row[19]);
      dp.h_y = atof(row[20]);
      dp.h_z = atof(row[21]);
      dp.w_x = atof(row[22]);
      dp.w_y = atof(row[23]);
      dp.w_z = atof(row[24]);
      dp.n_x = atof(row[25]);
      dp.n_y = atof(row[26]);
      dp.n_z = atof(row[27]);
      dp.efficiency = atof(row[28]);

      if (name[0] == 'H')
        dp.detectorType = HODOSCOPE;
      else if (name[0] == 'C')
        dp.detectorType = WIRE_CHAMBER;
      else if (name[0] == 'P')
        dp.detectorType = PROP_TUBE;

      digiPlaneVec[j].push_back(dp);
    }
    mysql_free_result(res);
  }

  //  This information is needed to match up rocID, boardID, and channelID with a detectorName, elementID combination
  //  This information also matches drift distances with drift times and tdc times, and provides in time windows

  for (int i = 0; i < (int) digiPlaneVec.size(); i++)
  {
    for (int k = 0; k < (int) digiPlaneVec[i].size(); k++)
    {
      if (digiPlaneVec[i][k].detectorType == HODOSCOPE)
      {
        sprintf(qry, "SELECT elementID, tPeak, width, rocID, boardID, channelID FROM hodoInfo "
                     "WHERE detectorName = \'%s\'", digiPlaneVec[i][k].detectorName.c_str());
        mysql_query(con, qry);
        if (mysql_errno(con) != 0)
        {
          cout << qry << endl << endl;
          cout << mysql_error(con) << endl;
        }
        res = mysql_store_result(con);

        while((row = mysql_fetch_row(res)))
        {
          int elementID = atoi(row[0]);

          if ((int) digiPlaneVec[i][k].tPeak.size() < elementID + 1)
          {
            digiPlaneVec[i][k].tPeak.resize(elementID + 1);
            digiPlaneVec[i][k].width.resize(elementID + 1);
            digiPlaneVec[i][k].rocID.resize(elementID + 1);
            digiPlaneVec[i][k].boardID.resize(elementID + 1);
            digiPlaneVec[i][k].channelID.resize(elementID + 1);
          }

          digiPlaneVec[i][k].tPeak[elementID] = atof(row[1]);
          digiPlaneVec[i][k].width[elementID] = atof(row[2]);
          digiPlaneVec[i][k].rocID[elementID] = atoi(row[3]);
          digiPlaneVec[i][k].boardID[elementID] = atoi(row[4]);
          digiPlaneVec[i][k].channelID[elementID] = atoi(row[5]);
        }
        mysql_free_result(res);

        sprintf(qry, "SELECT elementID, tPeak, rocID, boardID, channelID FROM triggerInfo "
            "WHERE detectorName = \'%s\' AND triggerLevel = 1", digiPlaneVec[i][k].detectorName.c_str());
        mysql_query(con, qry);
        if (mysql_errno(con) != 0)
        {
          cout << qry << endl << endl;
          cout << mysql_error(con) << endl;
        }
        res = mysql_store_result(con);

        while((row = mysql_fetch_row(res)))
        {
          int elementID = atoi(row[0]);

          if ((int) digiPlaneVec[i][k].triggerTPeak.size() < elementID + 1)
          {
            digiPlaneVec[i][k].triggerTPeak.resize(elementID + 1);
            digiPlaneVec[i][k].triggerRocID.resize(elementID + 1);
            digiPlaneVec[i][k].triggerBoardID.resize(elementID + 1);
            digiPlaneVec[i][k].triggerChannelID.resize(elementID + 1);
          }

          digiPlaneVec[i][k].triggerTPeak[elementID] = atof(row[1]);
          digiPlaneVec[i][k].triggerRocID[elementID] = atoi(row[2]);
          digiPlaneVec[i][k].triggerBoardID[elementID] = atoi(row[3]);
          digiPlaneVec[i][k].triggerChannelID[elementID] = atoi(row[4]);
        }
        mysql_free_result(res);
      }
      else
      {
        sprintf(qry, "SELECT elementID, t0, offset, width, rocID, boardID, channelID FROM chamberInfo "
                     "WHERE detectorName = \'%s\'", digiPlaneVec[i][k].detectorName.c_str());
        mysql_query(con, qry);
        if (mysql_errno(con) != 0)
        {
          cout << qry << endl << endl;
          cout << mysql_error(con) << endl;
        }
        res = mysql_store_result(con);

        while ((row = mysql_fetch_row(res)))
        {
          int elementID = atoi(row[0]);

          if ((int) digiPlaneVec[i][k].tPeak.size() < elementID + 1)
          {
            digiPlaneVec[i][k].t0.resize(elementID + 1);
            digiPlaneVec[i][k].offset.resize(elementID + 1);
            digiPlaneVec[i][k].width.resize(elementID + 1);
            digiPlaneVec[i][k].rocID.resize(elementID + 1);
            digiPlaneVec[i][k].boardID.resize(elementID + 1);
            digiPlaneVec[i][k].channelID.resize(elementID + 1);
          }

          digiPlaneVec[i][k].t0[elementID] = atof(row[1]);
          digiPlaneVec[i][k].offset[elementID] = atof(row[2]);
          digiPlaneVec[i][k].width[elementID] = atof(row[3]);
          digiPlaneVec[i][k].rocID[elementID] = atoi(row[4]);
          digiPlaneVec[i][k].boardID[elementID] = atoi(row[5]);
          digiPlaneVec[i][k].channelID[elementID] = atoi(row[6]);
        }
        mysql_free_result(res);

        sprintf(qry, "SELECT minDistance, maxDistance, driftTime FROM distanceToTime WHERE detectorName = \'%s\' "
                     "ORDER BY minDistance ASC;", digiPlaneVec[i][k].detectorName.c_str());
        mysql_query(con, qry);
        if (mysql_errno(con) != 0)
        {
          cout << qry << endl << endl;
          cout << mysql_error(con) << endl;
        }
        res = mysql_store_result(con);

        while ((row = mysql_fetch_row(res)))
        {
          digiPlaneVec[i][k].minDistance.push_back(atof(row[0])*cm);
          digiPlaneVec[i][k].maxDistance.push_back(atof(row[1])*cm);
          digiPlaneVec[i][k].driftTime.push_back(atof(row[2]));
        }
        mysql_free_result(res);

        // This section downloads information to simulate deltaRays.
        // Other sources of non-legitimate hits will probably be added at some point.

        sprintf(qry, "SELECT delta_length, min_p, max_p FROM deltaRays WHERE detectorName = \'%s\' "
            "ORDER BY delta_length ASC;", 
            digiPlaneVec[i][k].detectorName.c_str());
        mysql_query(con, qry);
        if (mysql_errno(con) != 0)
        {
          cout << qry << endl << endl;
          cout << mysql_error(con) << endl;
        }
        res = mysql_store_result(con);

        while ((row = mysql_fetch_row(res)))
        {
          digiPlaneVec[i][k].deltaLength.push_back(atoi(row[0]));
          digiPlaneVec[i][k].deltaMinP.push_back(atof(row[1]));
          digiPlaneVec[i][k].deltaMaxP.push_back(atof(row[2]));
        }
        mysql_free_result(res);
      }
    }
  }
  mysql_close(con);
}
