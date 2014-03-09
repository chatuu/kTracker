/*
FastTracklet.cxx

Implementation of class Tracklet

Author: Kun Liu, liuk@fnal.gov
Created: 05-28-2013
*/

#include <iostream>
#include <algorithm>
#include <cmath>

#include <TMath.h>
#include <TMatrixD.h>

#include "FastTracklet.h"

ClassImp(SignedHit)
ClassImp(Tracklet)

SignedHit::SignedHit()
{
  hit.index = -1;
  hit.detectorID = -1;
  hit.elementID = -1;
  sign = 0;
}

SignedHit::SignedHit(int detectorID)
{
  hit.index = -1;
  hit.detectorID = detectorID;
  hit.elementID = -1;
  sign = 0;
}

SignedHit::SignedHit(Hit hit_input, int sign_input)
{
  hit = hit_input;
  sign = sign_input;
}

Tracklet::Tracklet()
{
  nXHits = 0;
  nUHits = 0;
  nVHits = 0;
  chisq = 9999.;

  tx = 0.;
  ty = 0.;
  x0 = 0.;
  y0 = 0.;
  invP = 0.02;

  err_tx = -1.;
  err_ty = -1.;
  err_x0 = -1.;
  err_y0 = -1.;
  err_invP = -1.;

  stationID = -1;

  for(int i = 0; i < 24; i++) residual[i] = 999.;
}

bool Tracklet::isValid()
{
  if(stationID < 1 || stationID > 6) return false;

  if(fabs(tx) > TX_MAX || fabs(x0) > X0_MAX) return false;
  if(fabs(ty) > TY_MAX || fabs(y0) > Y0_MAX) return false;
  if(err_tx < 0 || err_ty < 0 || err_x0 < 0 || err_y0 < 0) return false;

  double prob = getProb();
  if(prob < PROB_LOOSE) return false;

  //Tracklets in each station
  int nHits = nXHits + nUHits + nVHits;
  if(stationID < 5)
    {
      if(nXHits < 1 || nUHits < 1 || nVHits < 1) return false;
      if(nHits < 4) return false;
      if(chisq > 15.) return false;
    }

  //Back partial
  if(stationID == 5)
    {
      if(nXHits < 2 || nUHits < 2 || nVHits < 2) return false;
      if(nHits < 8) return false; 
    }

  //Global tracks
  if(stationID == 6)
    {
      if(nXHits < 3 || nUHits < 3 || nVHits < 3) return false;
      if(nHits < 12) return false;
      if(prob < PROB_TIGHT) return false;
      
      if(KMAG_ON == 1)
	{
	  if(invP < INVP_MIN || invP > INVP_MAX) return false;
       	}
    }

  return true;
}

double Tracklet::getProb() const
{
  int ndf;
  if(stationID == 6 && KMAG_ON == 1)
    {
      ndf = getNHits() - 5;
    }
  else
    {
      ndf = getNHits() - 4;
    }

  return TMath::Prob(chisq, ndf);
}

double Tracklet::getExpPositionX(double z) const
{
  if(KMAG_ON == 1 && stationID >= 5)
    {
      double tx_local = tx;
      double x0_local = x0;
      if(z < Z_KMAG_BEND - 1.)
	{
	  getXZInfoInSt1(tx_local, x0_local);
	}
      else if(z < Z_KMAG_FRINGE_BEND - 1.)
	{
	  getXZInfoInSt2(tx_local, x0_local);
	}

      return x0_local + tx_local*z;
    }
  else
    {
      return x0 + tx*z;
    }
}

double Tracklet::getExpPosErrorX(double z) const
{
  double err_x;
  if(KMAG_ON == 1 && stationID >= 5)
    {  
      double err_tx_local = err_tx;
      double err_x0_local = err_x0;
      if(z < Z_KMAG_BEND - 1.) 
	{
	  getXZErrorInSt1(err_tx_local, err_x0_local);
	}
      else if(z < Z_KMAG_FRINGE_BEND - 1.)
	{
	  getXZErrorInSt2(err_tx_local, err_x0_local);
	}

      err_x = err_x0_local + fabs(err_tx_local*z);
    }
  else
    {
      err_x = fabs(err_tx*z) + err_x0;
    }

  if(z > Z_ABSORBER) err_x += 1.;
  return err_x;
}

double Tracklet::getExpPositionY(double z) const
{
  return y0 + ty*z;
}

double Tracklet::getExpPosErrorY(double z) const
{
  double err_y = fabs(err_ty*z) + err_y0;
  if(z > Z_ABSORBER) err_y += 1.;

  return err_y;
}

double Tracklet::getExpPositionW(int detectorID)
{
  GeomSvc* p_geomSvc = GeomSvc::instance();
  double z = p_geomSvc->getPlanePosition(detectorID);

  double x_exp = getExpPositionX(z);
  double y_exp = getExpPositionY(z);

  return p_geomSvc->getInterceptionFast(detectorID, x_exp, y_exp);
}

bool Tracklet::operator<(const Tracklet& elem) const
{
  //return nXHits + nUHits + nVHits - 0.4*chisq > elem.nXHits + elem.nUHits + elem.nVHits - 0.4*elem.chisq;
  if(getNHits() == elem.getNHits()) 
    {
      return chisq < elem.chisq;
    }
  else
    {
      return getProb() > elem.getProb();
    }
}

bool Tracklet::similarity(const Tracklet& elem) const
{
  int nCommonHits = 0;
  std::list<SignedHit>::const_iterator first = hits.begin();
  std::list<SignedHit>::const_iterator second = elem.hits.begin();

  while(first != hits.end() && second != elem.hits.end())
    {
      if((*first) < (*second))
	{
	  ++first;
	}
      else if((*second) < (*first))
	{
	  ++second;
	}
      else
	{
	  if((*first) == (*second)) nCommonHits++;
	  ++first;
	  ++second;
	}
    }

  if(nCommonHits/double(elem.getNHits()) > 0.33333) return true;
  return false;
}

double Tracklet::getMomentum() const
{
  //Ref. SEAQUEST-doc-453-v3 by Don. Geesaman
  //if(KMAG_ON == 0) return 1E8;

  double p = 50.;
  double charge = getCharge();

  double c1 = Z_FMAG_BEND*PT_KICK_FMAG*charge;
  double c2 = Z_KMAG_BEND*PT_KICK_KMAG*charge;
  double c3 = -x0;
  double c4 = ELOSS_KFMAG;
  double c5 = c4/2.;

  double b = c1/c3 + c2/c3 - c4 - c5;
  double c = c4*c5 - c1*c5/c3 - c2*c4/c3;

  double disc = b*b - 4*c;
  if(disc > 0.)
    {
      p = (-b + sqrt(disc))/2. - ELOSS_KFMAG;
    }

  if(p < 10. || p > 120. || disc < 0)
    {
      double k = fabs(getExpPositionX(Z_KFMAG_BEND)/Z_KFMAG_BEND - tx);
      p = 1./(0.00832161 + 0.184186*k - 0.104132*k*k) + ELOSS_ABSORBER;
    }

  return p;
}

void Tracklet::getXZInfoInSt1(double& tx_st1, double& x0_st1) const
{
  if(KMAG_ON == 1)
    {
      tx_st1 = tx + PT_KICK_KMAG*invP*getCharge();
      x0_st1 = tx*Z_KMAG_BEND + x0 - tx_st1*Z_KMAG_BEND;
    }
  else
    {
      tx_st1 = tx;
      x0_st1 = x0;
    }  
}

void Tracklet::getXZErrorInSt1(double& err_tx_st1, double& err_x0_st1) const
{
  if(KMAG_ON == 1)
    {
      double err_kick = err_invP*PT_KICK_KMAG;
      err_tx_st1 = err_tx + err_kick;
      err_x0_st1 = err_x0 + err_kick*Z_KMAG_BEND;
    }
  else
    {
      err_tx_st1 = err_tx;
      err_x0_st1 = err_x0;
    }
}

void Tracklet::getXZInfoInSt2(double& tx_st2, double& x0_st2) const
{
  if(KMAG_ON == 1)
    {
      tx_st2 = tx + PT_KICK_KMAG_FRINGE*invP*getCharge();
      x0_st2 = tx*Z_KMAG_FRINGE_BEND + x0 - tx_st2*Z_KMAG_FRINGE_BEND;
    }
  else
    {
      tx_st2 = tx;
      x0_st2 = x0;
    }  
}

void Tracklet::getXZErrorInSt2(double& err_tx_st1, double& err_x0_st1) const
{
  if(KMAG_ON == 1)
    {
      double err_kick = err_invP*PT_KICK_KMAG_FRINGE;
      err_tx_st1 = err_tx + err_kick;
      err_x0_st1 = err_x0 + err_kick*Z_KMAG_FRINGE_BEND;
    }
  else
    {
      err_tx_st1 = err_tx;
      err_x0_st1 = err_x0;
    }
}

Tracklet Tracklet::operator+(const Tracklet& elem) const
{
  Tracklet tracklet;
  tracklet.stationID = 5;

  tracklet.nXHits = nXHits + elem.nXHits;
  tracklet.nUHits = nUHits + elem.nUHits;
  tracklet.nVHits = nVHits + elem.nVHits;

  tracklet.hits.assign(hits.begin(), hits.end());
  if(elem.stationID > stationID)
    {
      tracklet.hits.insert(tracklet.hits.end(), elem.hits.begin(), elem.hits.end());
    }
  else
    {
      tracklet.hits.insert(tracklet.hits.begin(), elem.hits.begin(), elem.hits.end());
    }

  tracklet.err_tx = 1./sqrt(1./err_tx/err_tx + 1./elem.err_tx/elem.err_tx);
  tracklet.err_ty = 1./sqrt(1./err_ty/err_ty + 1./elem.err_ty/elem.err_ty);
  tracklet.err_x0 = 1./sqrt(1./err_x0/err_x0 + 1./elem.err_x0/elem.err_x0);
  tracklet.err_y0 = 1./sqrt(1./err_y0/err_y0 + 1./elem.err_y0/elem.err_y0);

  tracklet.tx = (tx/err_tx/err_tx + elem.tx/elem.err_tx/elem.err_tx)*tracklet.err_tx*tracklet.err_tx;
  tracklet.ty = (ty/err_ty/err_ty + elem.ty/elem.err_ty/elem.err_ty)*tracklet.err_ty*tracklet.err_ty;
  tracklet.x0 = (x0/err_x0/err_x0 + elem.x0/elem.err_x0/elem.err_x0)*tracklet.err_x0*tracklet.err_x0;
  tracklet.y0 = (y0/err_y0/err_y0 + elem.y0/elem.err_y0/elem.err_y0)*tracklet.err_y0*tracklet.err_y0;

  tracklet.invP = 1./tracklet.getMomentum();
  tracklet.err_invP = 0.25*tracklet.invP;
  
  tracklet.calcChisq();
  return tracklet;
}

Tracklet Tracklet::operator*(const Tracklet& elem) const
{
  Tracklet tracklet;
  tracklet.stationID = 6;

  tracklet.nXHits = nXHits + elem.nXHits;
  tracklet.nUHits = nUHits + elem.nUHits;
  tracklet.nVHits = nVHits + elem.nVHits;

  tracklet.hits.assign(hits.begin(), hits.end());
  if(elem.stationID > stationID)
    {
      tracklet.hits.insert(tracklet.hits.end(), elem.hits.begin(), elem.hits.end());
    }
  else
    {
      tracklet.hits.insert(tracklet.hits.begin(), elem.hits.begin(), elem.hits.end());
    }

  if(elem.stationID == 5)
    {
      tracklet.tx = elem.tx;
      tracklet.ty = elem.ty;
      tracklet.x0 = elem.x0;
      tracklet.y0 = elem.y0;
      tracklet.invP = 1./elem.getMomentum();

      tracklet.err_tx = elem.err_tx;
      tracklet.err_ty = elem.err_ty;
      tracklet.err_x0 = elem.err_x0;
      tracklet.err_y0 = elem.err_y0;
      tracklet.err_invP = 0.25*tracklet.invP;
    }
  else
    {
      tracklet.tx = tx;
      tracklet.ty = ty;
      tracklet.x0 = x0;
      tracklet.y0 = y0;
      tracklet.invP = 1./getMomentum();

      tracklet.err_tx = err_tx;
      tracklet.err_ty = err_ty;
      tracklet.err_x0 = err_x0;
      tracklet.err_y0 = err_y0;
      tracklet.err_invP = 0.25*tracklet.invP;
    }

  tracklet.calcChisq();
  return tracklet;
}

void Tracklet::addDummyHits()
{
  std::vector<int> detectorIDs_all;
  for(int i = stationID*6 - 5; i <= stationID*6; i++) detectorIDs_all.push_back(i);

  std::vector<int> detectorIDs_now;
  for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
      detectorIDs_now.push_back(iter->hit.detectorID);
    }

  std::vector<int> detectorIDs_miss(6);
  std::vector<int>::iterator iter = std::set_difference(detectorIDs_all.begin(), detectorIDs_all.end(), detectorIDs_now.begin(), detectorIDs_now.end(), detectorIDs_miss.begin());
  detectorIDs_miss.resize(iter - detectorIDs_miss.begin());

  for(std::vector<int>::iterator iter = detectorIDs_miss.begin(); iter != detectorIDs_miss.end(); ++iter)
    {
      SignedHit dummy;
      dummy.hit.detectorID = *iter;

      hits.push_back(dummy);
    }

  sortHits();
}

double Tracklet::calcChisq()
{
  GeomSvc* p_geomSvc = GeomSvc::instance();
  chisq = 0.;

  double tx_st1, x0_st1, tx_st2, x0_st2;
  if(stationID == 6 && KMAG_ON == 1)
    {
      getXZInfoInSt1(tx_st1, x0_st1);
      getXZInfoInSt2(tx_st2, x0_st2);
    }
  else if(stationID == 5 && KMAG_ON == 1)
    {
      //getXZInfoInSt2(tx_st2, x0_st2);
      tx_st2 = tx;
      x0_st2 = x0;
    }

  for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
      if(iter->hit.index < 0) continue;

      int detectorID = iter->hit.detectorID;
      int index = detectorID - 1;

      double sigma;
#ifdef COARSE_MODE
      if(iter->sign == 0) sigma = p_geomSvc->getPlaneSpacing(detectorID)/sqrt(12.);
#else
      //if(iter->sign == 0) sigma = fabs(iter->hit.driftDistance);
      if(iter->sign == 0) sigma = p_geomSvc->getPlaneSpacing(detectorID)/sqrt(12.);
#endif
      if(iter->sign != 0) sigma = p_geomSvc->getPlaneResolution(detectorID);

      double p = iter->hit.pos + iter->sign*fabs(iter->hit.driftDistance);
      if(KMAG_ON == 1 && (stationID == 6 || stationID == 5))
	{
	  if(detectorID <= 6)
	    {
	      residual[index] = p - p_geomSvc->getInterception(detectorID, tx_st1, ty, x0_st1, y0);
	    }
	  else if(detectorID > 6 && detectorID <= 12)
	    {
	      residual[index] = p - p_geomSvc->getInterception(detectorID, tx_st2, ty, x0_st2, y0);
	    }
	  else
	    {
	      residual[index] = p - p_geomSvc->getInterception(detectorID, tx, ty, x0, y0);
	    }
	}
      else
	{
	  residual[index] = p - p_geomSvc->getInterception(detectorID, tx, ty, x0, y0);
	}
    
      //LogInfo(stationID << " " << detectorID << "  " << tx << "  " << tx_st1 << "  " << tx_st2 << "  " << invP << "  " << residual[index]); 
      chisq += (residual[index]*residual[index]/sigma/sigma);
      //std::cout << iter->hit.detectorID << "  " << iter->hit.elementID << "  " << iter->sign << "  " << iter->hit.pos << "  " << iter->hit.driftDistance << "  " << costheta << "  " << sintheta << "  " << z << "  " << (x0_st1 + tx_st1*z) << "  " << (x0 + tx*z) << "  " << (y0 + ty*z) << "  " << sigma << std::endl;
    }

  //std::cout << chisq << std::endl;
  return chisq;
}

SignedHit Tracklet::getSignedHit(int index)
{
  int id = 0;
  for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
      if(id == index) return *iter;
      id++;
    }

  SignedHit dummy;
  return dummy;
}

double Tracklet::Eval(const double* par)
{
  tx = par[0];
  ty = par[1];
  x0 = par[2];
  y0 = par[3];
  if(KMAG_ON == 1) invP = par[4];

  //std::cout << tx << "  " << ty << "  " << x0 << "  " << y0 << "  " << 1./invP << std::endl;
  return calcChisq();
}

SRecTrack Tracklet::getSRecTrack()
{
  GeomSvc* p_geomSvc = GeomSvc::instance();

  SRecTrack strack;
  strack.setChisq(chisq);
  for(std::list<SignedHit>::iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
      if(iter->hit.index < 0) continue;

      double z = p_geomSvc->getPlanePosition(iter->hit.detectorID);
      double tx_val, tx_err, x0_val, x0_err;
      if(iter->hit.detectorID <= 6)
	{
	  getXZInfoInSt1(tx_val, x0_val);
          getXZErrorInSt1(tx_err, x0_err);
	}
      else
	{
	  tx_val = tx;
	  x0_val = x0;
	  tx_err = err_tx;
	  x0_err = err_x0;
	}

      TMatrixD state(5, 1), covar(5, 5);
      state[0][0] = getCharge()*invP*sqrt((1. + tx_val*tx_val)/(1. + tx_val*tx_val + ty*ty));
      state[1][0] = tx_val;
      state[2][0] = ty;
      state[3][0] = getExpPositionX(z);
      state[4][0] = getExpPositionY(z);

      covar.Zero();
      covar[0][0] = err_invP*err_invP;
      covar[1][1] = tx_err*tx_err;
      covar[2][2] = err_ty*err_ty;
      covar[3][3] = getExpPosErrorX(z)*getExpPosErrorX(z);
      covar[4][4] = getExpPosErrorY(z)*getExpPosErrorY(z);
      
      strack.insertHitIndex(iter->hit.index*iter->sign);
      strack.insertStateVector(state);
      strack.insertCovariance(covar);
      strack.insertZ(z);
    }

  TVector3 mom_vtx, pos_vtx;
  strack.setDumpPos(swimToVertex(mom_vtx, pos_vtx));
  strack.setVertexFast(mom_vtx, pos_vtx);

  strack.setHodoHits();
  return strack;
}

TVector3 Tracklet::getMomentumSt1()
{
  double tx_st1, x0_st1;
  getXZInfoInSt1(tx_st1, x0_st1);

  double pz = 1./invP/sqrt(1. + tx_st1*tx_st1);
  return TVector3(pz*tx_st1, pz*ty, pz);
}

TVector3 Tracklet::getMomentumSt3()
{
  double pz = 1./invP/sqrt(1. + tx*tx);
  return TVector3(pz*tx, pz*ty, pz);
}

TVector3 Tracklet::swimToVertex(TVector3& mom_vtx, TVector3& pos_vtx)
{
  //Store the steps on each point (center of the interval)
  TVector3 mom[NSLICES_FMAG + NSTEPS_TARGET + 1];
  TVector3 pos[NSLICES_FMAG + NSTEPS_TARGET + 1];

  //E-loss and pT-kick per length, note the eloss is done in half-slices
  double eloss_unit_0 = ELOSS_FMAG/FMAG_LENGTH;
  double eloss_unit_1 = ELOSS_FMAG_RAD/FMAG_LENGTH;
  double ptkick_unit = PT_KICK_FMAG/FMAG_LENGTH;

  //Step size in FMAG/target area
  double step_fmag = FMAG_LENGTH/NSLICES_FMAG/2.;   //note that in FMag, each step is devided into two slices
  double step_target = fabs(Z_UPSTREAM)/NSTEPS_TARGET;

  //Initial position should be on the downstream face of beam dump
  pos[0].SetXYZ(getExpPositionX(FMAG_LENGTH), getExpPositionY(FMAG_LENGTH), FMAG_LENGTH);
  mom[0] = getMomentumSt1();

  //Charge of the track
  double charge = getCharge();

  //Now make the swim
  int iStep = 1;
  for(; iStep <= NSLICES_FMAG; ++iStep)
    {
      //Make pT kick at the center of slice, add energy loss at both first and last half-slice
      //Note that ty is the global class data member, which does not change during the entire swimming
      double tx_i = mom[iStep-1].Px()/mom[iStep-1].Pz();
      double tx_f = tx_i + 2.*charge*ptkick_unit*step_fmag/sqrt(mom[iStep-1].Px()*mom[iStep-1].Px() + mom[iStep-1].Pz()*mom[iStep-1].Pz());

      TVector3 trajVec1(tx_i*step_fmag, ty*step_fmag, step_fmag);
      TVector3 pos_b = pos[iStep-1] - trajVec1;

      double p_tot_i = mom[iStep-1].Mag();
      double p_tot_b;
      if(pos_b[2] > FMAG_HOLE_LENGTH || pos_b.Perp() > FMAG_HOLE_RADIUS)
	{
  	  p_tot_b = p_tot_i + (eloss_unit_0 + p_tot_i*eloss_unit_1)*trajVec1.Mag();
	}
      else
	{
	  p_tot_b = p_tot_i;
	}

      TVector3 trajVec2(tx_f*step_fmag, ty*step_fmag, step_fmag);
      pos[iStep] = pos_b - trajVec2;

      double p_tot_f;
      if(pos[iStep][2] > FMAG_HOLE_LENGTH || pos[iStep].Perp() > FMAG_HOLE_RADIUS)
	{
  	  p_tot_f = p_tot_b + (eloss_unit_0 + p_tot_b*eloss_unit_1)*trajVec2.Mag();
	}
      else
	{
	  p_tot_f = p_tot_b;
	}

      //Now the final position and momentum in this step
      double pz_f = p_tot_f/sqrt(1. + tx_f*tx_f + ty*ty);
      mom[iStep].SetXYZ(pz_f*tx_f, pz_f*ty, pz_f);
      pos[iStep] = pos[iStep-1] - trajVec1 - trajVec2;

#ifdef _DEBUG_ON_LEVEL_2 
      std::cout << "FMAG: " << iStep << ": " << pos[iStep-1][2] << " ==================>>> " << pos[iStep][2] << std::endl;
      std::cout << mom[iStep-1][0]/mom[iStep-1][2] << "     " << mom[iStep-1][1]/mom[iStep-1][2] << "     " << mom[iStep-1][2] << "     ";
      std::cout << pos[iStep-1][0] << "  " << pos[iStep-1][1] << "   " << pos[iStep-1][2] << std::endl << std::endl;
      std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
      std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif
    }

  for(; iStep < NSLICES_FMAG+NSTEPS_TARGET+1; ++iStep)
    {
      //Simple straight line flight
      double tx_i = mom[iStep-1].Px()/mom[iStep-1].Pz();
      TVector3 trajVec(tx_i*step_target, ty*step_target, step_target);

      mom[iStep] = mom[iStep-1];
      pos[iStep] = pos[iStep-1] - trajVec;

#ifdef _DEBUG_ON_LEVEL_2
      std::cout << "TARGET: " << iStep << ": " << pos[iStep-1][2] << " ==================>>> " << pos[iStep][2] << std::endl;
      std::cout << mom[iStep-1][0]/mom[iStep-1][2] << "     " << mom[iStep-1][1]/mom[iStep-1][2] << "     " << mom[iStep-1][2] << "     ";
      std::cout << pos[iStep-1][0] << "  " << pos[iStep-1][1] << "   " << pos[iStep-1][2] << std::endl << std::endl;
      std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
      std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif
    }

  //Now the swimming is done, find the point with closest distance of approach, let iStep store the index of that step
  double dca_min = 1E9;
  for(int i = 0; i < NSLICES_FMAG+NSTEPS_TARGET+1; ++i)
    {
      double dca = pos[i].Perp();
      if(dca < dca_min)
	{
	  dca_min = dca;
	  iStep = i;
	}
    }

  mom_vtx = mom[iStep];
  pos_vtx = pos[iStep];

#ifdef _DEBUG_ON_LEVEL_2
  std::cout << "The one with minimum DCA is: " << iStep << ": " << std::endl;
  std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
  std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif

  return pos[NSLICES_FMAG];
}

void Tracklet::print()
{
  using namespace std;

  calcChisq();

  cout << "Tracklet in station " << stationID << endl;
  cout << nXHits + nUHits + nVHits << " hits in this station with chisq = " << chisq << endl; 
  cout << "Momentum in z: " << 1./invP << " +/- " << err_invP/invP/invP << endl;
  cout << "Charge: " << getCharge() << endl;
  for(std::list<SignedHit>::iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
      if(iter->sign > 0) cout << "L: ";
      if(iter->sign < 0) cout << "R: ";
      if(iter->sign == 0) cout << "U: ";

      cout << iter->hit.index << " " << iter->hit.detectorID << "  " << iter->hit.elementID << "  " << residual[iter->hit.detectorID-1] << " === ";
    }
  cout << endl;

  cout << "X-Z: (" << tx << " +/- " << err_tx << ")*z + (" << x0 << " +/- " << err_x0 << ")" << endl;
  cout << "Y-Z: (" << ty << " +/- " << err_ty << ")*z + (" << y0 << " +/- " << err_y0 << ")" << endl;
  
  cout << "KMAG projection: X =  " << getExpPositionX(Z_KMAG_BEND) << " +/- " << getExpPosErrorX(Z_KMAG_BEND) << endl;  
  cout << "KMAG projection: Y =  " << getExpPositionY(Z_KMAG_BEND) << " +/- " << getExpPosErrorY(Z_KMAG_BEND) << endl;  
}
