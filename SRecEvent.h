/*
SRecEvent.h

Definition of the class SRecEvent and SRecTrack. SRecTrack is the  final track structure of recontructed tracks.
Contains nothing but ROOT classes, light-weighted and can be used as input for physics analysis. SRecEvent serves
as a container of SRecTrack

Added SRecDimuon, containing the dimuon info

Author: Kun Liu, liuk@fnal.gov
Created: 01-21-2013
*/

#ifndef _SRECTRACK_H
#define _SRECTRACK_H

#include "MODE_SWITCH.h"

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>

#include <TObject.h>
#include <TROOT.h>
#include <TMatrixD.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include "SRawEvent.h"

class SRecTrack: public TObject
{
public:
    SRecTrack();

    ///Gets
    Int_t getCharge() const { return (fState[0])[0][0] > 0 ? 1 : -1; }
    Int_t getNHits() const { return fHitIndex.size(); }
    Int_t getNHitsInStation(Int_t stationID);
    Double_t getChisq() const { return fChisq; }
    Double_t getProb() const { return KMAG_ON == 1 ? TMath::Prob(fChisq, getNHits() - 5) : TMath::Prob(fChisq, getNHits() - 4); }
    Double_t getQuality() const { return (Double_t)getNHits() - 0.4*getChisq(); }

    Int_t getHitIndex(Int_t i) { return fHitIndex[i]; }
    TMatrixD getStateVector(Int_t i) { return fState[i]; }
    TMatrixD getCovariance(Int_t i) { return fCovar[i]; }
    Double_t getZ(Int_t i) { return fZ[i]; }
    Double_t getChisqAtNode(Int_t i) { return fChisqAtNode[i]; }

    Int_t getNearestNode(Double_t z);
    void getExpPositionFast(Double_t z, Double_t& x, Double_t& y, Int_t iNode = -1);
    void getExpPosErrorFast(Double_t z, Double_t& dx, Double_t& dy, Int_t iNode = -1);
    Double_t getExpMomentumFast(Double_t z, Double_t& px, Double_t& py, Double_t& pz, Int_t iNode = -1);
    Double_t getExpMomentumFast(Double_t z, Int_t iNode = -1);

    Double_t getMomentumSt1(Double_t& px, Double_t& py, Double_t& pz) { return getMomentum(fState.front(), px, py, pz); }
    Double_t getMomentumSt1() { Double_t px, py, pz; return getMomentumSt1(px, py, pz); }
    TVector3 getMomentumVecSt1() { Double_t px, py, pz; getMomentumSt1(px, py, pz); return TVector3(px, py, pz); }

    Double_t getMomentumSt3(Double_t& px, Double_t& py, Double_t& pz) { return getMomentum(fState.back(), px, py, pz); }
    Double_t getMomentumSt3() { Double_t px, py, pz; return getMomentumSt3(px, py, pz); }
    TVector3 getMomentumVecSt3() { Double_t px, py, pz; getMomentumSt1(px, py, pz); return TVector3(px, py, pz); }

    Double_t getPositionSt1(Double_t& x, Double_t& y) { return getPosition(fState.front(), x, y); }
    Double_t getPositionSt1() { Double_t x, y; return getPositionSt1(x, y); }
    TVector3 getPositionVecSt1() { Double_t x, y; getPositionSt1(x, y); return TVector3(x, y, fZ.front()); }

    Double_t getPositionSt3(Double_t& x, Double_t& y) { return getPosition(fState.back(), x, y); }
    Double_t getPositionSt3() { Double_t x, y; return getPositionSt3(x, y); }
    TVector3 getPositionVecSt3() { Double_t x, y; getPositionSt3(x, y); return TVector3(x, y, fZ.back()); }

    Double_t getMomentum(TMatrixD& state, Double_t& px, Double_t& py, Double_t& pz);
    Double_t getPosition(TMatrixD& state, Double_t& x, Double_t& y);

    ///Fit status
    Bool_t isKalmanFitted() { return fKalmanStatus > 0; }
    void setKalmanStatus(Int_t status) { fKalmanStatus = status; }

    ///Comparitor
    bool operator<(const SRecTrack& elem) const;

    ///Sets
    void setChisq(Double_t chisq) { fChisq = chisq; }
    void insertHitIndex(Int_t index) { fHitIndex.push_back(index); }
    void insertStateVector(TMatrixD state) { fState.push_back(state); }
    void insertCovariance(TMatrixD covar) { fCovar.push_back(covar); }
    void insertZ(Double_t z) { fZ.push_back(z); }
    void insertChisq(Double_t chisq) { fChisqAtNode.push_back(chisq); }

    ///Fast-adjust of kmag
    void adjustKMag(double kmagStr);

    ///Vertex stuff
    bool isVertexValid();
    void setZVertex(Double_t z, bool update = true);

    ///Plain setting, no KF-related stuff
    void setVertexFast(TVector3 mom, TVector3 pos);

    ///Simple swim to vertex
    void swimToVertex(TVector3* pos = NULL, TVector3* mom = NULL);

    ///Get the vertex info
    TLorentzVector getMomentumVertex();
    Double_t getMomentumVertex(Double_t& px, Double_t& py, Double_t& pz) { return getMomentum(fStateVertex, px, py, pz); }
    Double_t getZVertex() { return fVertexPos[2]; }
    Double_t getRVertex() { return fVertexPos.Perp(); }
    TVector3 getVertex() { return fVertexPos; }
    Double_t getVtxPar(Int_t i) { return fVertexPos[i]; }
    Double_t getChisqVertex() { return fChisqVertex; }

    //Get mom/pos at a given location
    TVector3 getDumpPos() { return fDumpPos; }
    TVector3 getDumpFacePos() { return fDumpFacePos; }
    TVector3 getTargetPos() { return fTargetPos; }
    TVector3 getDumpMom() { return fDumpMom; }
    TVector3 getDumpFaceMom() { return fDumpFaceMom; }
    TVector3 getTargetMom() { return fTargetMom; }
    TVector3 getVertexPos() { return fVertexPos; }
    TVector3 getVertexMom() { return fVertexMom; }
    Double_t getChisqDump() { return fChisqDump; }
    Double_t getChisqTarget() { return fChisqTarget; }
    Double_t getChisqUpstream() { return fChisqUpstream; }

    //Set mom/pos at a given location
    void setDumpPos(TVector3 pos) { fDumpPos = pos; }
    void setDumpFacePos(TVector3 pos) { fDumpFacePos = pos; }
    void setTargetPos(TVector3 pos) { fTargetPos = pos; }
    void setDumpMom(TVector3 mom) { fDumpMom = mom; }
    void setDumpFaceMom(TVector3 mom) { fDumpFaceMom = mom; }
    void setTargetMom(TVector3 mom) { fTargetMom = mom; }
    void setChisqDump(Double_t chisq) { fChisqDump = chisq; }
    void setChisqTarget(Double_t chisq) { fChisqTarget = chisq; }
    void setChisqUpstream(Double_t chisq) { fChisqUpstream = chisq; }

    //Trigger road info
    void setTriggerRoad(Int_t roadID) { fTriggerID = roadID; }
    Int_t getTriggerRoad() { return fTriggerID; }

    //Prop. tube muon ID info
    void setPTSlope(Double_t slopeX, Double_t slopeY) { fPropSlopeX = slopeX; fPropSlopeY = slopeY; }
    void setNHitsInPT(Int_t nHitsX, Int_t nHitsY) { fNPropHitsX = nHitsX; fNPropHitsY = nHitsY; }
    Double_t getPTSlopeX() { return fPropSlopeX; }
    Double_t getPTSlopeY() { return fPropSlopeY; }
    Double_t getDeflectionX() { return fState.back()[1][0] - fPropSlopeX; }
    Double_t getDeflectionY() { return fState.back()[2][0] - fPropSlopeY; }
    Int_t getNHitsInPTX() { return fNPropHitsX; }
    Int_t getNHitsInPTY() { return fNPropHitsY; }

    //Overall track quality cut
    bool isValid();

    ///Debugging output
    void print();

private:
    ///Total Chisq
    Double_t fChisq;

    ///Hit list and associated track parameters
    std::vector<Int_t> fHitIndex;
    std::vector<TMatrixD> fState;
    std::vector<TMatrixD> fCovar;
    std::vector<Double_t> fZ;
    std::vector<Double_t> fChisqAtNode;

    ///Momentum/Position at a given z
    TVector3 fDumpFacePos;
    TVector3 fDumpPos;
    TVector3 fTargetPos;

    TVector3 fDumpFaceMom;
    TVector3 fDumpMom;
    TVector3 fTargetMom;

    ///Vertex infomation
    TVector3 fVertexMom;      //duplicate information as fStateVertex already contains all the info., just keep it for now
    TVector3 fVertexPos;
    Double_t fChisqVertex;
    TMatrixD fStateVertex;
    TMatrixD fCovarVertex;

    ///Kalman Fitted
    Int_t fKalmanStatus;

    ///Corresponding trigger road
    Int_t fTriggerID;

    ///Prop. tube. slope
    Int_t fNPropHitsX;
    Int_t fNPropHitsY;
    Double_t fPropSlopeX;
    Double_t fPropSlopeY;

    //Chisq of three test position
    Double_t fChisqTarget;
    Double_t fChisqDump;
    Double_t fChisqUpstream;

    ClassDef(SRecTrack, 9)
};

class SRecDimuon: public TObject
{
public:
    //Get the total momentum of the virtual photon
    TLorentzVector getVPhoton() { return p_pos + p_neg; }

    //Calculate the kinematic vairables
    void calcVariables();

    //Dimuon quality cut
    bool isValid();

    //Target dimuon
    bool isTarget();

    //Dump dimuon
    bool isDump();

    //Index of muon track used in host SRecEvent
    Int_t trackID_pos;
    Int_t trackID_neg;

    //4-momentum of the muon tracks after vertex fit
    TLorentzVector p_pos;
    TLorentzVector p_neg;

    //4-momentum of the muon tracks before vertex fit
    TLorentzVector p_pos_single;
    TLorentzVector p_neg_single;

    //3-vector vertex position
    TVector3 vtx;
    TVector3 vtx_pos;
    TVector3 vtx_neg;

    //Track projections at different location
    TVector3 proj_target_pos;
    TVector3 proj_dump_pos;
    TVector3 proj_target_neg;
    TVector3 proj_dump_neg;

    //Kinematic variables
    Double_t mass;
    Double_t pT;
    Double_t xF;
    Double_t x1;
    Double_t x2;
    Double_t costh;
    Double_t mass_single;
    Double_t chisq_single;

    //Vertex fit chisqs
    Double_t chisq_kf;
    Double_t chisq_vx;

    //Chisq of three test position
    Double_t chisq_target;
    Double_t chisq_dump;
    Double_t chisq_upstream;

    ClassDef(SRecDimuon, 5)
};

class SRecEvent: public TObject
{
public:
    SRecEvent();

    ///Set/Get event info
    void setEventInfo(SRawEvent* rawEvent);
    void setEventInfo(int runID, int spillID, int eventID) { fRunID = runID; fSpillID = spillID; fEventID = eventID; }
    void setTargetPos(int targetPos) { fTargetPos = targetPos; }
    void setRecStatus(int status) { fRecStatus += status; }

    ///directly setup everything by raw event
    void setRawEvent(SRawEvent* rawEvent);

    ///Trigger util
    bool isTriggeredBy(Int_t trigger) { return (fTriggerBits & trigger) != 0; }

    Int_t getRunID() { return fRunID; }
    Int_t getSpillID() { return fSpillID; }
    Int_t getEventID() { return fEventID; }
    Int_t getTargetPos() { return fTargetPos; }
    Int_t getTriggerBits() { return fTriggerBits; }
    Int_t getRecStatus() { return fRecStatus; }

    Int_t getLocalID(Int_t hitID) { return fLocalID[hitID]; }

    ///Get tracks
    Int_t getNTracks() { return fAllTracks.size(); }
    SRecTrack& getTrack(Int_t i) { return fAllTracks[i]; }

    ///Get track IDs
    std::vector<Int_t> getChargedTrackIDs(Int_t charge);

    ///Get dimuons
    Int_t getNDimuons() { return fDimuons.size(); }
    SRecDimuon getDimuon(Int_t i) { return fDimuons[i]; }

    ///Insert tracks
    void insertTrack(SRecTrack trk) { fAllTracks.push_back(trk); }
    void reIndex() { sort(fAllTracks.begin(), fAllTracks.end()); }

    ///Insert dimuon
    void insertDimuon(SRecDimuon dimuon) { fDimuons.push_back(dimuon); }

    ///Clear everything
    void clear();

private:
    ///Reconstruction status
    Short_t fRecStatus;

    ///Basic event info.
    Int_t fRunID;
    Int_t fSpillID;
    Int_t fEventID;

    ///Target position
    Int_t fTargetPos;

    ///Trigger bit
    Int_t fTriggerBits;

    ///Container of SRecTrack
    std::vector<SRecTrack> fAllTracks;

    ///Dimuons reconstructed
    std::vector<SRecDimuon> fDimuons;

    ///Mapping of hitID to local container ID in SRawEvent
    std::map<Int_t, Int_t> fLocalID;

    ClassDef(SRecEvent, 4)
};

#endif
