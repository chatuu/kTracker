/*
KalmanFastTracking.cxx

Implementation of class Tracklet, KalmanFastTracking

Author: Kun Liu, liuk@fnal.gov
Created: 05-28-2013
*/

#include <iostream>
#include <algorithm>
#include <cmath>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TBox.h>
#include <TMatrixD.h>

#include "KalmanFastTracking.h"

KalmanFastTracking::KalmanFastTracking(bool flag)
{
  using namespace std;
#ifdef _DEBUG_ON
  cout << "Initialization of KalmanFastTracking ..." << endl;
  cout << "========================================" << endl;
#endif

  enable_KF = flag;

  //Initialize minuit minimizer
  minimizer[0] = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Simplex");
  minimizer[1] = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Combined");
  if(KMAG_ON == 1)
    {
      fcn = ROOT::Math::Functor(&tracklet_curr, &Tracklet::Eval, 5);
    }
  else
    {
      fcn = ROOT::Math::Functor(&tracklet_curr, &Tracklet::Eval, 4);
    }

  for(int i = 0; i < 2; ++i)
    {
      minimizer[i]->SetMaxFunctionCalls(1000000);
      minimizer[i]->SetMaxIterations(100);
      minimizer[i]->SetTolerance(1E-2);
      minimizer[i]->SetFunction(fcn);
      minimizer[i]->SetPrintLevel(0);
    }

  //Minimize ROOT output
  extern Int_t gErrorIgnoreLevel;
  gErrorIgnoreLevel = 9999;

  //Initialize geometry service
  p_geomSvc = GeomSvc::instance();

  //Initialize plane angles for all planes
  for(int i = 1; i <= 24; i++)
    {
      costheta_plane[i] = p_geomSvc->getCostheta(i);
      sintheta_plane[i] = p_geomSvc->getSintheta(i);
    }

  //Initialize hodoscope IDs
  detectorIDs_mask[0] = p_geomSvc->getDetectorIDs("H1"); 
  detectorIDs_mask[1] = p_geomSvc->getDetectorIDs("H2"); 
  detectorIDs_mask[2] = p_geomSvc->getDetectorIDs("H3"); 
  detectorIDs_mask[3] = p_geomSvc->getDetectorIDs("H4"); 
  detectorIDs_maskX[0] = p_geomSvc->getDetectorIDs("H1X"); 
  detectorIDs_maskX[1] = p_geomSvc->getDetectorIDs("H2X"); 
  detectorIDs_maskX[2] = p_geomSvc->getDetectorIDs("H3X"); 
  detectorIDs_maskX[3] = p_geomSvc->getDetectorIDs("H4X"); 
  detectorIDs_maskY[0] = p_geomSvc->getDetectorIDs("H1Y"); 
  detectorIDs_maskY[1] = p_geomSvc->getDetectorIDs("H2Y"); 
  detectorIDs_maskY[2] = p_geomSvc->getDetectorIDs("H3Y"); 
  detectorIDs_maskY[3] = p_geomSvc->getDetectorIDs("H4Y"); 

  detectorIDs_mask[4] = p_geomSvc->getDetectorIDs("P1");
  detectorIDs_maskX[4] = p_geomSvc->getDetectorIDs("P1X");
  detectorIDs_maskY[4] = p_geomSvc->getDetectorIDs("P1Y");

  std::vector<int> temp_mask, temp_maskX, temp_maskY;
  temp_mask = p_geomSvc->getDetectorIDs("P2");
  temp_maskX = p_geomSvc->getDetectorIDs("P2X");
  temp_maskY = p_geomSvc->getDetectorIDs("P2Y");

  detectorIDs_mask[4].insert(detectorIDs_mask[4].end(), temp_mask.begin(), temp_mask.end());
  detectorIDs_maskX[4].insert(detectorIDs_maskX[4].end(), temp_maskX.begin(), temp_maskX.end());
  detectorIDs_maskY[4].insert(detectorIDs_maskY[4].end(), temp_maskY.begin(), temp_maskY.end());

  //Initialize masking window sizes, with 10% contingency
  for(int i = 25; i <= 48; i++)
    {
      for(int j = 1; j <= p_geomSvc->getPlaneNElements(i); j++)
	{
	  double x_min, x_max, y_min, y_max;
	  p_geomSvc->get2DBoxSize(i, j, x_min, x_max, y_min, y_max);
	  
	  x_min -= (0.15*(x_max - x_min));
	  x_max += (0.15*(x_max - x_min));
	  y_min -= (0.15*(y_max - y_min));
	  y_max += (0.15*(y_max - y_min));

	  x_mask_min[i-25][j-1] = x_min;
	  x_mask_max[i-25][j-1] = x_max;
	  y_mask_min[i-25][j-1] = y_min;
	  y_mask_max[i-25][j-1] = y_max;
	}      
    }

#ifdef _DEBUG_ON
  cout << "========================" << endl;
  cout << "Hodo. masking settings: " << endl;
  for(int i = 0; i < 4; i++)
    {
      cout << "For station " << i+1 << endl;
      for(std::vector<int>::iterator iter = detectorIDs_mask[i].begin(); iter != detectorIDs_mask[i].end(); ++iter) cout << "All: " << *iter << endl;
      for(std::vector<int>::iterator iter = detectorIDs_maskX[i].begin(); iter != detectorIDs_maskX[i].end(); ++iter) cout << "X: " << *iter << endl;
      for(std::vector<int>::iterator iter = detectorIDs_maskY[i].begin(); iter != detectorIDs_maskY[i].end(); ++iter) cout << "Y: " << *iter << endl;
    } 
#endif

  //Initialize super stationIDs
  for(int i = 0; i < 4; i++) superIDs[i].clear();
  superIDs[0].push_back((p_geomSvc->getDetectorIDs("D1X")[0] + 1)/2);
  superIDs[0].push_back((p_geomSvc->getDetectorIDs("D1U")[0] + 1)/2);
  superIDs[0].push_back((p_geomSvc->getDetectorIDs("D1V")[0] + 1)/2);
  superIDs[1].push_back((p_geomSvc->getDetectorIDs("D2X")[0] + 1)/2);
  superIDs[1].push_back((p_geomSvc->getDetectorIDs("D2U")[0] + 1)/2);
  superIDs[1].push_back((p_geomSvc->getDetectorIDs("D2V")[0] + 1)/2);
  superIDs[2].push_back((p_geomSvc->getDetectorIDs("D3pX")[0] + 1)/2);
  superIDs[2].push_back((p_geomSvc->getDetectorIDs("D3pU")[0] + 1)/2);
  superIDs[2].push_back((p_geomSvc->getDetectorIDs("D3pV")[0] + 1)/2);
  superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3mX")[0] + 1)/2);
  superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3mU")[0] + 1)/2);
  superIDs[3].push_back((p_geomSvc->getDetectorIDs("D3mV")[0] + 1)/2);

#ifdef _DEBUG_ON
  cout << "=============" << endl;
  cout << "Chamber IDs: " << endl;
  for(int i = 0; i < 4; i++)
    {
      for(int j = 0; j < 3; j++) cout << i << "  " << j << ": " << superIDs[i][j] << endl;
    }

  //Initialize widow sizes for X-U matching and z positions of all chambers
  cout << "======================" << endl;
  cout << "U plane window sizes: " << endl;
#endif
  double u_factor[] = {0., 0., 0., 0.};
  for(int i = 0; i < 4; i++)
    {
      int xID = 2*superIDs[i][0] - 1;
      int uID = 2*superIDs[i][1] - 1;
      int vID = 2*superIDs[i][2] - 1;
      double spacing = p_geomSvc->getPlaneSpacing(uID);
      double x_span = p_geomSvc->getPlaneScaleY(uID);

      u_win[i] = fabs(0.5*x_span/(spacing/sintheta_plane[uID])) + 2.*spacing + u_factor[i];
      u_costheta[i] = costheta_plane[uID];
      u_sintheta[i] = sintheta_plane[uID];

      z_plane_x[i] = 0.5*(p_geomSvc->getPlanePosition(xID) + p_geomSvc->getPlanePosition(xID+1));
      z_plane_u[i] = 0.5*(p_geomSvc->getPlanePosition(uID) + p_geomSvc->getPlanePosition(uID+1));
      z_plane_v[i] = 0.5*(p_geomSvc->getPlanePosition(vID) + p_geomSvc->getPlanePosition(vID+1));
#ifdef _DEBUG_ON
      cout << "Station " << i << ": " << u_win[i] << endl; 
#endif
    }

  //Initialize Z positions and maximum parameters of all planes
  for(int i = 1; i <= 24; i++) 
    {
      z_plane[i] = p_geomSvc->getPlanePosition(i);
      slope_max[i] = costheta_plane[i]*TX_MAX + sintheta_plane[i]*TY_MAX;
      intersection_max[i] = costheta_plane[i]*X0_MAX + sintheta_plane[i]*Y0_MAX;
  
      resol_plane[i] = p_geomSvc->getPlaneResolution(i);
    }

#ifdef _DEBUG_ON
  cout << "======================================" << endl;
  cout << "Maximum local slope and intersection: " << endl;
#endif
  for(int i = 1; i <= 12; i++)
    {
      double d_slope = (p_geomSvc->getPlaneResolution(2*i - 1) + p_geomSvc->getPlaneResolution(2*i))/(z_plane[2*i] - z_plane[2*i-1]);
      double d_intersection = d_slope*z_plane[2*i];

      slope_max[2*i-1] += d_slope;
      intersection_max[2*i-1] += d_intersection;
      slope_max[2*i] += d_slope;
      intersection_max[2*i] += d_intersection;

#ifdef _DEBUG_ON
      cout << "Super plane " << i << ": " << slope_max[2*i-1] << "  " << intersection_max[2*i-1] << endl; 
#endif
    }

  //Initialize sagitta ratios, index 0, 1, 2 are for X, U, V
  s_ratio[0] = 1.8; s_sigma[0] = 0.2; s_detectorID[0] = 10;
  s_ratio[1] = 1.9; s_sigma[1] = 0.2; s_detectorID[1] = 12;
  s_ratio[2] = 1.7; s_sigma[2] = 0.2; s_detectorID[2] = 8;
}

KalmanFastTracking::~KalmanFastTracking()
{
  delete minimizer[0];
  delete minimizer[1];
}

bool KalmanFastTracking::setRawEvent(SRawEvent* event_input)
{
  rawEvent = event_input;
  if(!acceptEvent(rawEvent)) return false;
  hitAll = event_input->getAllHits();
#ifdef _DEBUG_ON
  for(std::vector<Hit>::iterator iter = hitAll.begin(); iter != hitAll.end(); ++iter) iter->print();
#endif

  //Initialize hodo and prop. tube masking IDs
  for(int i = 0; i < 5; i++)
    {
      //std::cout << "For station " << i << std::endl;
      hitIDs_mask[i].clear();
      hitIDs_mask[i] = rawEvent->getHitsIndexInDetectors(detectorIDs_maskX[i]);

      //for(std::list<int>::iterator iter = hitIDs_mask[i].begin(); iter != hitIDs_mask[i].end(); ++iter) std::cout << *iter << " " << hitAll[*iter].detectorID << " === ";
      //std::cout << std::endl;
    }

  //Initialize tracklet lists
  for(int i = 0; i < 5; i++) trackletsInSt[i].clear();

  //Build tracklets in station 2, 3+, 3-
  //When i = 3, works for st3+, for i = 4, works for st3-
  buildTrackletsInStation(2); 
  if(trackletsInSt[1].empty()) 
    {
#ifdef _DEBUG_ON
      Log("Failed in tracklet build at station 2");
#endif
      return false;
    }

  buildTrackletsInStation(3); buildTrackletsInStation(4);
  if(trackletsInSt[2].empty()) 
    {
#ifdef _DEBUG_ON
      Log("Failed in tracklet build at station 3");
#endif
      return false;
    }

  //Build back partial tracks in station 2, 3+ and 3-
  buildBackPartialTracks();
 
  //Connect tracklets in station 2/3 and station 1 to form global tracks
  buildGlobalTracks();
 
#ifdef _DEBUG_ON 
  for(int i = 0; i <= 4; i++)
    {
      std::cout << "=======================================================================================" << std::endl;
      Log("Final tracklets in station: " << i+1 << " is " << trackletsInSt[i].size()); 
      for(std::list<Tracklet>::iterator tracklet = trackletsInSt[i].begin(); tracklet != trackletsInSt[i].end(); ++tracklet)
	{
	  tracklet->print();
	}
      std::cout << "=======================================================================================" << std::endl;
   }
#endif

  if(trackletsInSt[4].empty()) return false;
  return true;
}

bool KalmanFastTracking::acceptEvent(SRawEvent* rawEvent)
{
#ifdef _DEBUG_ON
  Log("D1: " << rawEvent->getNHitsInD1());
  Log("D2: " << rawEvent->getNHitsInD2());
  Log("D3p: " << rawEvent->getNHitsInD3p());
  Log("D3m: " << rawEvent->getNHitsInD3m());
  Log("H1: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[0]));
  Log("H2: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[1]));
  Log("H3: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[2]));
  Log("H4: " << rawEvent->getNHitsInDetectors(detectorIDs_maskX[3]));
#endif

  if(rawEvent->getNHitsInD1() > 250) return false;
  if(rawEvent->getNHitsInD2() > 100) return false;
  if(rawEvent->getNHitsInD3p() > 100) return false;
  if(rawEvent->getNHitsInD3m() > 100) return false;
  if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[0]) > 25) return false;
  if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[1]) > 10) return false;
  if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[2]) > 10) return false;
  if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[3]) > 10) return false;
  
  return true;
}

void KalmanFastTracking::buildBackPartialTracks()
{
  for(std::list<Tracklet>::iterator tracklet3 = trackletsInSt[2].begin(); tracklet3 != trackletsInSt[2].end(); ++tracklet3)
    {
      Tracklet tracklet_best;
      for(std::list<Tracklet>::iterator tracklet2 = trackletsInSt[1].begin(); tracklet2 != trackletsInSt[1].end(); ++tracklet2)
	{
	  Tracklet tracklet_23 = (*tracklet2) + (*tracklet3);
	  fitTracklet(tracklet_23);
	  resolveLeftRight(tracklet_23, 25.);
	  resolveLeftRight(tracklet_23, 100.);

#ifdef _DEBUG_ON
	  Log("New tracklet: ");
	  tracklet_23.print(); 

	  Log("Current best:");
	  tracklet_best.print();

	  Log("Comparison: " << (tracklet_23 < tracklet_best));
#endif

	  //If current tracklet is better than the best tracklet up-to-now
	  if(acceptTracklet(tracklet_23) && tracklet_23 < tracklet_best)
	    {
	      tracklet_best = tracklet_23;
	    }
	}

      if(tracklet_best.isValid()) trackletsInSt[3].push_back(tracklet_best);
    }

  reduceTrackletList(trackletsInSt[3]);
  trackletsInSt[3].sort();
}

void KalmanFastTracking::buildGlobalTracks()
{
  double pos_exp[3], window[3];
  for(std::list<Tracklet>::iterator tracklet23 = trackletsInSt[3].begin(); tracklet23 != trackletsInSt[3].end(); ++tracklet23)
    {
      //Calculate the window in station 1
      if(KMAG_ON)
	{
	  getSagittaWindowsInSt1(*tracklet23, pos_exp, window);
	}
      else
	{
	  getExtrapoWindowsInSt1(*tracklet23, pos_exp, window);
	}

#ifdef _DEBUG_ON
      Log("Using this back partial: ");
      tracklet23->print();
      for(int i = 0; i < 3; i++) Log("Extrapo: " << pos_exp[i] << "  " << window[i]);
#endif

      trackletsInSt[0].clear();
      buildTrackletsInStation(1, pos_exp, window);
      
      Tracklet tracklet_best;
      for(std::list<Tracklet>::iterator tracklet1 = trackletsInSt[0].begin(); tracklet1 != trackletsInSt[0].end(); ++tracklet1)
	{
#ifdef _DEBUG_ON
	  Log("With this station 1 track:");
	  tracklet1->print();
#endif

	  Tracklet tracklet_global = (*tracklet23) * (*tracklet1);
	  fitTracklet(tracklet_global);
  
	  //Resolve the left-right with a tight pull cut, then a loose one, then resolve by single projections
  	  resolveLeftRight(tracklet_global, 100.);
          if(!tracklet_global.isValid()) continue;

	  resolveLeftRight(tracklet_global, 1000.);
          resolveSingleLeftRight(tracklet_global);

	  //Remove bad hits if needed
	  removeBadHits(tracklet_global);

#ifdef _DEBUG_ON
	  Log("New tracklet: ");
	  tracklet_global.print(); 

	  Log("Current best:");
	  tracklet_best.print();

	  Log("Comparison: " << (tracklet_global < tracklet_best));
	  Log("Quality   : " << acceptTracklet(tracklet_global));
#endif
	  if(acceptTracklet(tracklet_global) && tracklet_global < tracklet_best)
	    {
#ifdef _DEBUG_ON
	      Log("Accepted!!!");
#endif
	      tracklet_best = tracklet_global;
    	    }
	}

      if(tracklet_best.isValid()) trackletsInSt[4].push_back(tracklet_best);
    }

  trackletsInSt[4].sort();
}

void KalmanFastTracking::resolveLeftRight(Tracklet& tracklet, double threshold)
{
#ifdef _DEBUG_ON
  Log("Left right for this track..");
  tracklet.print();
#endif

  //Check if the track has been updated
  bool isUpdated = false;

  //Four possibilities
  int possibility[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

  //Total number of hit pairs in this tracklet
  int nPairs = tracklet.hits.size()/2;
  
  int nResolved = 0;
  std::list<SignedHit>::iterator hit1 = tracklet.hits.begin();
  std::list<SignedHit>::iterator hit2 = tracklet.hits.begin(); ++hit2;
  while(true)
    {
#ifdef _DEBUG_ON
      Log(hit1->hit.index << "  " << hit2->sign << " === " << hit2->hit.index << "  " << hit2->sign);
      int detectorID1 = hit1->hit.detectorID;
      int detectorID2 = hit2->hit.detectorID;
      Log("Hit1: " << tracklet.getExpPositionX(z_plane[detectorID1])*costheta_plane[detectorID1] + tracklet.getExpPositionY(z_plane[detectorID1])*sintheta_plane[detectorID1] << "  " << hit1->hit.pos + hit1->hit.driftDistance << "  " << hit1->hit.pos - hit1->hit.driftDistance);
      Log("Hit2: " << tracklet.getExpPositionX(z_plane[detectorID2])*costheta_plane[detectorID2] + tracklet.getExpPositionY(z_plane[detectorID2])*sintheta_plane[detectorID2] << "  " << hit2->hit.pos + hit2->hit.driftDistance << "  " << hit2->hit.pos - hit2->hit.driftDistance);
#endif

      if(hit1->hit.index > 0 && hit2->hit.index > 0 && hit1->sign*hit2->sign == 0)
	{
	  int index_min = -1;
	  double pull_min = 1E6;
	  for(int i = 0; i < 4; i++)
	    {
	      double slope_local = (hit1->pos(possibility[i][0]) - hit2->pos(possibility[i][1]))/(z_plane[hit1->hit.detectorID] - z_plane[hit2->hit.detectorID]);
	      double inter_local = hit1->pos(possibility[i][0]) - slope_local*z_plane[hit1->hit.detectorID];
	   
	      if(fabs(slope_local) > slope_max[hit1->hit.detectorID] || fabs(inter_local) > intersection_max[hit1->hit.detectorID]) continue;

	      double tx, ty, x0, y0;
	      double err_tx, err_ty, err_x0, err_y0;
	      if(tracklet.stationID == 6 && hit1->hit.detectorID <= 6)
		{
		  tracklet.getXZInfoInSt1(tx, x0);
		  tracklet.getXZErrorInSt1(err_tx, err_x0);
		}
	      else
		{
		  tx = tracklet.tx;
		  x0 = tracklet.x0;
		  err_tx = tracklet.err_tx;
		  err_x0 = tracklet.err_x0;
		}
	      ty = tracklet.ty;
	      y0 = tracklet.y0;
	      err_ty = tracklet.err_ty;
	      err_y0 = tracklet.err_y0;

	      double slope_exp = costheta_plane[hit1->hit.detectorID]*tx + sintheta_plane[hit1->hit.detectorID]*ty;
	      double err_slope = fabs(costheta_plane[hit1->hit.detectorID]*err_tx) + fabs(sintheta_plane[hit2->hit.detectorID]*err_ty);
	      double inter_exp = costheta_plane[hit1->hit.detectorID]*x0 + sintheta_plane[hit1->hit.detectorID]*y0;
	      double err_inter = fabs(costheta_plane[hit1->hit.detectorID]*err_x0) + fabs(sintheta_plane[hit2->hit.detectorID]*err_y0);

	      double pull = sqrt((slope_exp - slope_local)*(slope_exp - slope_local)/err_slope/err_slope + (inter_exp - inter_local)*(inter_exp - inter_local)/err_inter/err_inter);
	      if(pull < pull_min)
		{
		  index_min = i;
		  pull_min = pull;
		}

#ifdef _DEBUG_ON
	      Log(hit1->hit.detectorID << ": " << i << "  " << possibility[i][0] << "  " << possibility[i][1]);
	      Log(tx << "  " << x0 << "  " << ty << "  " << y0);
	      Log("Slope: " << slope_local << "  " << slope_exp << "  " << err_slope);
	      Log("Intersection: " << inter_local << "  " << inter_exp << "  " << err_inter);
	      Log("Current: " << pull << "  " << index_min << "  " << pull_min);
#endif
	    }

	  //Log("Final: " << index_min << "  " << pull_min);
	  if(index_min >= 0 && pull_min < threshold)//((tracklet.stationID == 5 && pull_min < 25.) || (tracklet.stationID == 6 && pull_min < 100.)))
	    {
	      hit1->sign = possibility[index_min][0];
	      hit2->sign = possibility[index_min][1];

	      isUpdated = true;
	    }
	}

      ++nResolved;
      if(nResolved >= nPairs) break;

      ++hit1; ++hit1;
      ++hit2; ++hit2;
    }

  if(isUpdated) fitTracklet(tracklet);
}

void KalmanFastTracking::resolveSingleLeftRight(Tracklet& tracklet)
{
#ifdef _DEBUG_ON
  Log("Single left right for this track..");
  tracklet.print();
#endif

  //Check if the track has been updated
  bool isUpdated = false;

  for(std::list<SignedHit>::iterator hit_sign = tracklet.hits.begin(); hit_sign != tracklet.hits.end(); ++hit_sign)
    {
      if(hit_sign->hit.index < 0 || hit_sign->sign != 0) continue;

      int detectorID = hit_sign->hit.detectorID;
      double pos_exp = tracklet.getExpPositionX(z_plane[detectorID])*costheta_plane[detectorID] + tracklet.getExpPositionY(z_plane[detectorID])*sintheta_plane[detectorID];
      hit_sign->sign = pos_exp > hit_sign->hit.pos ? 1 : -1;

      isUpdated = true;
    }

  if(isUpdated) fitTracklet(tracklet);
}

void KalmanFastTracking::removeBadHits(Tracklet& tracklet)
{
#ifdef _DEBUG_ON
  Log("Removing hits for this track..");
  tracklet.calcChisq();
  tracklet.print();
#endif

  //Check if the track has beed updated
  bool isUpdated = true;
  while(isUpdated)
    {
      isUpdated = false;
      tracklet.calcChisq();

      SignedHit* hit_remove = NULL;
      double res_remove = -1.;
      for(std::list<SignedHit>::iterator hit_sign = tracklet.hits.begin(); hit_sign != tracklet.hits.end(); ++hit_sign)
	{
	  if(hit_sign->hit.index < 0) continue;

	  int detectorID = hit_sign->hit.detectorID; 
	  double res_curr = fabs(tracklet.residual[detectorID-1]);
	  if(res_remove < res_curr)
	    {
	      res_remove = res_curr;
	      hit_remove = &(*hit_sign);
	    }
	}

      if(hit_remove != NULL && res_remove > HIT_REJECT*resol_plane[hit_remove->hit.detectorID])
	{
#ifdef _DEBUG_ON
	  Log("Dropping this hit: " << res_remove << "  " << HIT_REJECT*resol_plane[hit_remove->hit.detectorID]);
	  hit_remove->hit.print();
#endif

	  hit_remove->hit.index = -1;
	  int planeType = p_geomSvc->getPlaneType(hit_remove->hit.detectorID);
	  if(planeType == 1)
	    {
	      --tracklet.nXHits;
	    } 
	  else if(planeType == 2)
	    {
	      --tracklet.nUHits;
	    }
	  else
	    {
	      --tracklet.nVHits;
	    }

	  isUpdated = true;
	}

      if(isUpdated) fitTracklet(tracklet);
    }
}

void KalmanFastTracking::resolveLeftRight(SRawEvent::hit_pair hpair, int& LR1, int& LR2)
{
  LR1 = 0;
  LR2 = 0;

  //If either hit is missing, no left-right can be assigned
  if(hpair.first < 0 || hpair.second < 0)
    {
      return;
    }

  int possibility[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
  int nResolved = 0;
  for(int i = 0; i < 4; i++)
    {
      if(nResolved > 1) break;

      int hitID1 = hpair.first;
      int hitID2 = hpair.second;
      double slope_local = (hitAll[hitID1].pos + possibility[i][0]*hitAll[hitID1].driftDistance - hitAll[hitID2].pos - possibility[i][1]*hitAll[hitID2].driftDistance)/(z_plane[hitAll[hitID1].detectorID] - z_plane[hitAll[hitID2].detectorID]);
      double intersection_local = hitAll[hitID1].pos + possibility[i][0]*hitAll[hitID1].driftDistance - slope_local*z_plane[hitAll[hitID1].detectorID];
      
      //Log(i << "  " << nResolved << "  " << slope_local << "  " << intersection_local);
      if(fabs(slope_local) < slope_max[hitAll[hitID1].detectorID] && fabs(intersection_local) < intersection_max[hitAll[hitID1].detectorID])
	{
  	  nResolved++;
	  LR1 = possibility[i][0];
	  LR2 = possibility[i][1];
	}
    }

  if(nResolved > 1)
    {
      LR1 = 0;
      LR2 = 0;
    }

  //Log("Final: " << LR1 << "  " << LR2);
}

void KalmanFastTracking::buildTrackletsInStation(int stationID, double* pos_exp, double* window)
{
#ifdef _DEBUG_ON
  Log("Building tracklets in station " << stationID);
#endif

  //actuall ID of the tracklet lists
  int sID = stationID - 1;
  int listID = sID;
  if(listID == 3) listID = 2;

  //Extract the X, U, V hit pairs
  std::list<SRawEvent::hit_pair> pairs_X, pairs_U, pairs_V;
  if(pos_exp == NULL)
    {
      pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0]);
      pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1]);
      pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2]);
    }
  else
    {
      pairs_X = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][0], pos_exp[0], window[0]);
      pairs_U = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][1], pos_exp[1], window[1]);
      pairs_V = rawEvent->getPartialHitPairsInSuperDetector(superIDs[sID][2], pos_exp[2], window[2]);
    }

#ifdef _DEBUG_ON
  Log("Hit pairs in this event: ");
  for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_X.begin(); iter != pairs_X.end(); ++iter) Log("X :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << hitAll[iter->second].index);
  for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_U.begin(); iter != pairs_U.end(); ++iter) Log("U :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << hitAll[iter->second].index);
  for(std::list<SRawEvent::hit_pair>::iterator iter = pairs_V.begin(); iter != pairs_V.end(); ++iter) Log("V :" << iter->first << "  " << iter->second << "  " << hitAll[iter->first].index << " " << hitAll[iter->second].index);
#endif

  //X-U combination first, then add V pairs
  for(std::list<SRawEvent::hit_pair>::iterator xiter = pairs_X.begin(); xiter != pairs_X.end(); ++xiter)
    {
      //U projections from X plane
      double x_pos = xiter->second >= 0 ? 0.5*(hitAll[xiter->first].pos + hitAll[xiter->second].pos) : hitAll[xiter->first].pos;
      double u_min = x_pos*u_costheta[sID] - u_win[sID];
      double u_max = u_min + 2.*u_win[sID];

#ifdef _DEBUG_ON
      Log("Trying X hits " << xiter->first << "  " << xiter->second);
      Log("U plane window:" << u_min << "  " << u_max);
#endif
      for(std::list<SRawEvent::hit_pair>::iterator uiter = pairs_U.begin(); uiter != pairs_U.end(); ++uiter)
	{
	  double u_pos = uiter->second >= 0 ? 0.5*(hitAll[uiter->first].pos + hitAll[uiter->second].pos) : hitAll[uiter->first].pos;
#ifdef _DEBUG_ON
	  Log("Trying U hits " << uiter->first << "  " << uiter->second << " at " << u_pos);
#endif
	  if(u_pos < u_min || u_pos > u_max) continue;

	  //V projections from X and U plane
          double z_x = xiter->second >= 0 ? z_plane_x[sID] : z_plane[hitAll[xiter->first].detectorID];
          double z_u = uiter->second >= 0 ? z_plane_u[sID] : z_plane[hitAll[uiter->first].detectorID];
          double z_v = z_plane_v[sID];
	  double v_win1 = p_geomSvc->getPlaneSpacing(hitAll[uiter->first].detectorID)*2.*u_costheta[sID];
	  double v_win2 = (z_u + z_v - 2.*z_x)*u_costheta[sID]*TX_MAX;
	  double v_win3 = (z_v - z_u)*u_sintheta[sID]*TY_MAX;
	  double v_win = v_win1 + v_win2 + v_win3;
	  double v_min = 2*x_pos*u_costheta[sID] - u_pos - v_win;
	  double v_max = v_min + 2.*v_win;

#ifdef _DEBUG_ON	  
      	  Log("V plane window:" << v_min << "  " << v_max);
#endif
	  for(std::list<SRawEvent::hit_pair>::iterator viter = pairs_V.begin(); viter != pairs_V.end(); ++viter)
	    {
	      double v_pos = viter->second >= 0 ? 0.5*(hitAll[viter->first].pos + hitAll[viter->second].pos) : hitAll[viter->first].pos;
#ifdef _DEBUG_ON
    	      Log("Trying V hits " << viter->first << "  " << viter->second << " at " << v_pos);
#endif
	      if(v_pos < v_min || v_pos > v_max) continue;

	      //Now add the tracklet
	      int LR1 = 0;
	      int LR2 = 0;
	      Tracklet tracklet_new;
              tracklet_new.stationID = stationID;

	      //resolveLeftRight(*xiter, LR1, LR2);
	      if(xiter->first >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[xiter->first], LR1)); tracklet_new.nXHits++; }
	      if(xiter->second >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[xiter->second], LR2)); tracklet_new.nXHits++; }

	      //resolveLeftRight(*uiter, LR1, LR2);
	      if(uiter->first >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[uiter->first], LR1)); tracklet_new.nUHits++; }
	      if(uiter->second >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[uiter->second], LR2)); tracklet_new.nUHits++; }

	      //resolveLeftRight(*viter, LR1, LR2);
	      if(viter->first >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[viter->first], LR1)); tracklet_new.nVHits++; }
	      if(viter->second >= 0) { tracklet_new.hits.push_back(SignedHit(hitAll[viter->second], LR2)); tracklet_new.nVHits++; }
	   
	      tracklet_new.sortHits();
	      if(!tracklet_new.isValid())
		{
		  fitTracklet(tracklet_new);
		}
	      else
		{
		  continue;
		}
	   
#ifdef _DEBUG_ON
	      tracklet_new.print(); 
#endif
	      if(acceptTracklet(tracklet_new)) 
		{
		  trackletsInSt[listID].push_back(tracklet_new);
		}
#ifdef _DEBUG_ON
	      else
		{
		  Log("Rejected!!!");
		}
#endif
	    }
	}
    } 

  //Reduce the tracklet list and add dummy hits
  //reduceTrackletList(trackletsInSt[listID]);
  for(std::list<Tracklet>::iterator iter = trackletsInSt[listID].begin(); iter != trackletsInSt[listID].end(); ++iter)
    {
      iter->addDummyHits();
    }
}

bool KalmanFastTracking::acceptTracklet(Tracklet& tracklet)
{
  //Tracklet itself is okay with enough hits (4-out-of-6) and small chi square
  if(!tracklet.isValid()) 
    {
#ifdef _DEBUG_ON
      Log("Failed in quality check!");
#endif
      return false;
    }

  //See hodo. projections
  std::vector<int> stationIDs_mask;
  if(tracklet.stationID <= 3)
    {
      stationIDs_mask.push_back(tracklet.stationID - 1);
    }
  else if(tracklet.stationID == 4)
    {
      stationIDs_mask.push_back(2);
    }
  else if(tracklet.stationID == 5)
    {
      for(int i = 1; i <= 4; i++) stationIDs_mask.push_back(i);
    }
  else if(tracklet.stationID == 6)
    {
      for(int i = 0; i <= 4; i++) stationIDs_mask.push_back(i);
    }
  int nMinimum = stationIDs_mask.size();
 
  //Log(tracklet.stationID);
  int nHodoHits = 0;
  for(std::vector<int>::iterator stationID = stationIDs_mask.begin(); stationID != stationIDs_mask.end(); ++stationID)
    {
      for(std::list<int>::iterator iter = hitIDs_mask[*stationID].begin(); iter != hitIDs_mask[*stationID].end(); ++iter)
       	{
	  double z_hodo = p_geomSvc->getPlanePosition(hitAll[*iter].detectorID);
    	  double x_hodo = tracklet.getExpPositionX(z_hodo);
	  double y_hodo = tracklet.getExpPositionY(z_hodo);
    	  double err_x = 3.*tracklet.getExpPosErrorX(z_hodo);
	  double err_y = 3.*tracklet.getExpPosErrorY(z_hodo);

	  int idx1 = hitAll[*iter].detectorID - 25;
	  int idx2 = hitAll[*iter].elementID - 1;
	  double x_min = x_mask_min[idx1][idx2] - err_x;
	  double x_max = x_mask_max[idx1][idx2] + err_x;
	  double y_min = y_mask_min[idx1][idx2] - err_y;
	  double y_max = y_mask_max[idx1][idx2] + err_y;

#ifdef _DEBUG_ON
	  Log(*iter);
	  hitAll[*iter].print();
	  Log(z_hodo << "  " << x_hodo << " +/- " << err_x << "  " << y_hodo << " +/-" << err_y << " : " << x_min << "  " << x_max << "  " << y_min << "  " << y_max);
#endif
	  if(x_hodo > x_min && x_hodo < x_max && y_hodo > y_min && y_hodo < y_max)
    	    {
    	      nHodoHits++;
    	    }
	}
    }

  //Log(tracklet.stationID << "  " << nHodoHits);
  if(nHodoHits < nMinimum) return false;

  if(tracklet.stationID > 4)
    {
      if(!p_geomSvc->isInKMAG(tracklet.getExpPositionX(Z_KMAG_BEND), tracklet.getExpPositionY(Z_KMAG_BEND))) return false;
    }

  //If everything is fine ...
  return true;
}

int KalmanFastTracking::fitTracklet(Tracklet& tracklet)
{
  tracklet_curr = tracklet;

  //idx = 0, using simplex; idx = 1 using migrad
  int idx = 1;
#ifdef _ENABLE_MULTI_MINI
  if(tracklet.stationID < 5) idx = 0;
#endif

  minimizer[idx]->SetLimitedVariable(0, "tx", tracklet.tx, 0.001, -TX_MAX, TX_MAX);
  minimizer[idx]->SetLimitedVariable(1, "ty", tracklet.ty, 0.001, -TY_MAX, TY_MAX);
  minimizer[idx]->SetLimitedVariable(2, "x0", tracklet.x0, 0.1, -X0_MAX, X0_MAX);
  minimizer[idx]->SetLimitedVariable(3, "y0", tracklet.y0, 0.1, -Y0_MAX, Y0_MAX);
  if(KMAG_ON == 1)
    {
      minimizer[idx]->SetLimitedVariable(4, "invP", tracklet.invP, 0.001*tracklet.invP, INVP_MIN, INVP_MAX);
    }
  minimizer[idx]->Minimize();

  tracklet.tx = minimizer[idx]->X()[0];
  tracklet.ty = minimizer[idx]->X()[1];
  tracklet.x0 = minimizer[idx]->X()[2];
  tracklet.y0 = minimizer[idx]->X()[3];
 
  tracklet.err_tx = minimizer[idx]->Errors()[0];
  tracklet.err_ty = minimizer[idx]->Errors()[1];
  tracklet.err_x0 = minimizer[idx]->Errors()[2];
  tracklet.err_y0 = minimizer[idx]->Errors()[3];

  if(KMAG_ON == 1 && tracklet.stationID == 6)
    {
      tracklet.invP = minimizer[idx]->X()[4];
      tracklet.err_invP = minimizer[idx]->Errors()[4];
    }

  tracklet.chisq = minimizer[idx]->MinValue();
 
  int status = minimizer[idx]->Status();
  return status;
}

int KalmanFastTracking::reduceTrackletList(std::list<Tracklet>& tracklets)
{
  std::list<Tracklet> targetList;

  tracklets.sort();
  while(!tracklets.empty())
    {
      targetList.push_back(tracklets.front());
      tracklets.pop_front();

#ifdef _DEBUG_ON_LEVEL_2
      Log("Current best tracklet in reduce");
      targetList.back().print();
#endif

      for(std::list<Tracklet>::iterator iter = tracklets.begin(); iter != tracklets.end(); )
	{
	  if(iter->similarity(targetList.back()))
	    {
#ifdef _DEBUG_ON_LEVEL_2
	      Log("Removing this tracklet: ");
	      iter->print();
#endif
	      iter = tracklets.erase(iter);
	      continue;
	    }
	  else
	    {
	      ++iter;
	    }
	}
    }

  tracklets.assign(targetList.begin(), targetList.end());
  return 0;
}

void KalmanFastTracking::getExtrapoWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window)
{
  if(tracklet.stationID != 5)
    {
      for(int i = 0; i < 3; i++)
	{
	  pos_exp[i] = 9999.;
	  window[i] = 0.;
	}

      return;
    }

  for(int i = 0; i < 3; i++)
    {
      int detectorID = 2*i+2;
      double z_st1 = z_plane[detectorID];
      double x_st1 = tracklet.getExpPositionX(z_st1);
      double y_st1 = tracklet.getExpPositionY(z_st1);
      double err_x = tracklet.getExpPosErrorX(z_st1);
      double err_y = tracklet.getExpPosErrorY(z_st1);

      pos_exp[i] = p_geomSvc->getUinStereoPlane(detectorID, x_st1, y_st1);
      window[i] = 5.*(fabs(costheta_plane[detectorID]*err_x) + fabs(sintheta_plane[detectorID]*err_y));
    }
}

void KalmanFastTracking::getSagittaWindowsInSt1(Tracklet& tracklet, double* pos_exp, double* window)
{
  if(tracklet.stationID != 5)
    {
      for(int i = 0; i < 3; i++) 
	{
	  pos_exp[i] = 9999.;
	  window[i] = 0.;
	}

      return;
    }

  double z_st3 = z_plane[tracklet.hits.back().hit.detectorID];
  double x_st3 = tracklet.getExpPositionX(z_st3);
  double y_st3 = tracklet.getExpPositionY(z_st3);

  //For U, X, and V planes
  for(int i = 0; i < 3; i++)
    {
      double pos_st3 = p_geomSvc->getUinStereoPlane(s_detectorID[i], x_st3, y_st3);

      double z_st2 = z_plane[s_detectorID[i]];
      double x_st2 = tracklet.getExpPositionX(z_st2);
      double y_st2 = tracklet.getExpPositionY(z_st2);
      double pos_st2 = p_geomSvc->getUinStereoPlane(s_detectorID[i], x_st2, y_st2);
      double sagitta_st2 = pos_st2 - pos_st3*z_st2/z_st3;

      double z_st1 = z_plane[2*i+2];
      pos_exp[i] = sagitta_st2*s_ratio[i] + pos_st3*z_st1/z_st3;
      window[i] = fabs(5.*sagitta_st2*s_sigma[i]);
    }
}

void KalmanFastTracking::printAtDetectorBack(int stationID, std::string outputFileName)
{
  TCanvas c1;

  std::vector<double> x, y, dx, dy;
  for(std::list<Tracklet>::iterator iter = trackletsInSt[stationID].begin(); iter != trackletsInSt[stationID].end(); ++iter)
    {
      double z = p_geomSvc->getPlanePosition(iter->stationID*6);
      x.push_back(iter->getExpPositionX(z));
      y.push_back(iter->getExpPositionY(z));
      dx.push_back(iter->getExpPosErrorX(z));
      dy.push_back(iter->getExpPosErrorY(z));
    }

  TGraphErrors gr(x.size(), &x[0], &y[0], &dx[0], &dy[0]);
  gr.SetMarkerStyle(8);

  //Add detector frames
  std::vector<double> x_f, y_f, dx_f, dy_f;
  x_f.push_back(p_geomSvc->getPlaneCenterX(stationID*6 + 6));
  y_f.push_back(p_geomSvc->getPlaneCenterY(stationID*6 + 6));
  dx_f.push_back(p_geomSvc->getPlaneScaleX(stationID*6 + 6)*0.5);
  dy_f.push_back(p_geomSvc->getPlaneScaleY(stationID*6 + 6)*0.5);

  if(stationID == 2)
    {
      x_f.push_back(p_geomSvc->getPlaneCenterX(stationID*6 + 12));
      y_f.push_back(p_geomSvc->getPlaneCenterY(stationID*6 + 12));
      dx_f.push_back(p_geomSvc->getPlaneScaleX(stationID*6 + 12)*0.5);
      dy_f.push_back(p_geomSvc->getPlaneScaleY(stationID*6 + 12)*0.5);
    }
 
  TGraphErrors gr_frame(x_f.size(), &x_f[0], &y_f[0], &dx_f[0], &dy_f[0]); 
  gr_frame.SetLineColor(kRed);
  gr_frame.SetLineWidth(2);
  gr_frame.SetFillColor(15);
  
  c1.cd();
  gr_frame.Draw("A2[]");
  gr.Draw("Psame");

  c1.SaveAs(outputFileName.c_str());
}
