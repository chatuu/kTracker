#ifndef digiplane_h
#define digiplane_h 1

#include <vector>
#include "G4String.hh"

using namespace std;

enum DetectorType {HODOSCOPE, WIRE_CHAMBER, PROP_TUBE};

class DigiPlane
{
  public:

    DigiPlane();
    ~DigiPlane();

  private:

  public:
    G4String detectorName;
    G4String geantName;
    double spacing;
    double cellWidth;
    double overlap;
    int numElements;
    int lowElementID;
    double planeWidth;
    double planeHeight;
    double xPrimeOffset;
    double u_x, u_y, u_z;
    double v_x, v_y, v_z;
    double angle;
    double x0, y0, z0;
    double n_x, n_y, n_z;
    double h_x, h_y, h_z;
    double w_x, w_y, w_z;
    double efficiency;

    DetectorType detectorType;

    vector<double> t0;
    vector<double> offset;
    vector<double> width;
    vector<double> tPeak;
    vector<double> triggerTPeak;

    vector<int> rocID;
    vector<int> boardID;
    vector<int> channelID;

    vector<int> triggerRocID;
    vector<int> triggerBoardID;
    vector<int> triggerChannelID;

    vector<double> minDistance;
    vector<double> maxDistance;
    vector<double> driftTime;

    vector<int> deltaLength;
    vector<double> deltaMinP;
    vector<double> deltaMaxP;
};

#endif
