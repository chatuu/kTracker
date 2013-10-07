#ifndef _VERTEXFIT_H
#define _VERTEXFIT_H

/*
VertexFit.h

Definition of the Vertex fit of dimuon events, this package is aimed at:
  1. Use closest distance method to find z0 for two muon tracks
  2. Use vertex fit to fit the z0 --- will be implemented in next version

Reference: CBM-SOFT-note-2006-001, by S. Gorbunov and I. Kisel, with some minor
           modifications

Author: Kun Liu, liuk@fnal.gov
Created: 2-8-2012
*/

#include <iostream>
#include <vector>

#include "MODE_SWITCH.h"

#include <TMatrixD.h>

#include "KalmanUtil.h"
#include "KalmanFilter.h"
#include "KalmanTrack.h"
#include "SRecEvent.h"
#include "FastTracklet.h"
#include "TrackExtrapolator/TrackExtrapolator.hh"

class VtxPar
{
public:
  VtxPar()
    {
      _r.ResizeTo(3, 1);
      _cov.ResizeTo(3, 3);

      _r.Zero();
      _cov.Zero();
    }

  void print()
    {
      SMatrix::printMatrix(_r, "Vertex position:");
      SMatrix::printMatrix(_cov, "Vertex covariance:");
    }

  TMatrixD _r;
  TMatrixD _cov;
};

class VertexFit
{
public:
  VertexFit();
  ~VertexFit();

  ///Set the convergence control parameters
  void setControlParameter(int nMaxIteration, double tolerance) 
    {
      _max_iteration = nMaxIteration; 
      _tolerance = tolerance; 
    }

  ///Initialize and reset
  void init();
  void addHypothesis(double z, double sigz = 50.) { z_start.push_back(z); sig_z_start.push_back(sigz); }
  void setStartingVertex(double z_start, double sigz_start);

  ///Add one track parameter set into the fit
  void addTrack(int index, SRecTrack& _track);
  void addTrack(int index, KalmanTrack& _track);
  void addTrack(int index, TrkPar& _trkpar);
  void addTrack(int index, Tracklet& _trkpar);

  ///Main external call to find the vertex
  int processOneEvent();

  ///Find the primary vertex
  int findVertex();
  double findSingleMuonVertex(SRecTrack& _track);
  double findSingleMuonVertex(Node& _node_start);
  double findSingleMuonVertex(TrkPar& _trkpar_start);

  ///Gets
  double getVertexZ0() { return _vtxpar_curr._r[2][0]; }
  double getVXChisq() { return _chisq_vertex; }
  double getKFChisq() { return _chisq_kalman; }
  int getNTracks() { return _trkpar_curr.size(); }

  ///Core function, update the vertex prediction according to the track info.
  void updateVertex();

  ///Debugging output
  void print();

private:
  ///storage of the input track parameters
  std::vector<TrkPar> _trkpar_curr;

  ///vertex parameter
  VtxPar _vtxpar_curr;

  ///Kalman node at the vertex
  Node _node_vertex;

  ///pointer to external Kalman filter
  KalmanFilter *_kmfit;
 
  ///chi squares 
  double _chisq_vertex;
  double _chisq_kalman;

  ///Starting points
  std::vector<double> z_start;
  std::vector<double> sig_z_start;

  ///Temporary results
  std::vector<double> z_vertex;
  std::vector<double> chisq_km;
  std::vector<double> chisq_vx;

  ///convergence control parameter
  int _max_iteration;
  double _tolerance;

  ///Track extrapolator
  TrackExtrapolator _extrapolator;
};

#endif