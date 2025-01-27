#ifndef _TRIGGERANALYZER_H
#define _TRIGGERANALYZER_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <set>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "GeomSvc.h"
#include "TriggerRoad.h"
#include "SRawEvent.h"

typedef std::vector<std::set<int> > DataMatrix;

class TNode
{
public:
    //constructor
    TNode(int uID);

    //add child
    void add(TNode* child);

public:
    int uniqueID;
    std::list<TNode*> children;
};

class TriggerAnalyzer
{
public:
    TriggerAnalyzer();
    ~TriggerAnalyzer();

    //initialization
    bool init(std::list<TriggerRoad> p_roads, std::list<TriggerRoad> m_roads, double cut_td = 0., double cut_gun = 1E8); //init by road lists
    bool init(std::string fileName, double cut_td = 0., double cut_gun = 1E8); //init by root files
    bool init(std::string schemaName); //init by MySQL database
    bool init(); //init by ascii file in the same directory
    void filterRoads(double cut_td, double cut_gun);
    void makeRoadPairs();

    //Accept a event
    bool acceptEvent(TriggerRoad& p_road, TriggerRoad& m_road);
    bool acceptEvent(int nHits, int detectorIDs[], int elementIDs[]);
    bool acceptEvent(SRawEvent* rawEvent, int mode = USE_TRIGGER_HIT);

    //Trim a event's hodoscope hits
    void trimEvent(SRawEvent* rawEvent, std::list<Hit>& hitlist, int mode = USE_TRIGGER_HIT);

    //Get the road list of +/-
    std::list<TriggerRoad>& getRoadsAll(int charge) { return roads[(-charge+1)/2]; }
    std::list<TriggerRoad>& getRoadsFound(int charge) { return roads_found[(-charge+1)/2]; }
    std::list<TriggerRoad>& getRoadsEnabled(int charge) { return roads_enabled[(-charge+1)/2]; }
    std::list<TriggerRoad>& getRoadsDisabled(int charge) { return roads_disabled[(-charge+1)/2]; }

    int getNRoadsPosTop() { return nRoads[0][0]; }
    int getNRoadsPosBot() { return nRoads[0][1]; }
    int getNRoadsNegTop() { return nRoads[1][0]; }
    int getNRoadsNegBot() { return nRoads[1][1]; }

    //Build prefix tree
    void buildTriggerTree();

    //Build data
    bool buildData(int nHits, int detectorIDs[], int elementIDs[]);

    //find all possible matched road in data
    void search(TNode* root, DataMatrix& data, int level, int charge);

    //print/clear prefix tree
    void print(int charge) { roads_temp.clear(); printTree(root[(-charge+1)/2]); }
    void clear(int charge) { roads_temp.clear(); clearTree(root[(-charge+1)/2]); }

    void printTree(TNode* root);
    void clearTree(TNode* root);

    void printRoadFound();
    void printData(DataMatrix& data);

    //Output the road selected and road pair selection
    void outputEnabled();

private:
    //Pointer to the Geometry
    GeomSvc* p_geomSvc;
    
    //Single muon roads
    std::list<TriggerRoad> roads[2];
    std::list<TriggerRoad> roads_enabled[2];
    std::list<TriggerRoad> roads_disabled[2];

    //Counters of single roads
    int nRoads[2][2];  //first index stands for +/-, second stands for top/bottom

    //Dimuon road pairs (accepted groupID pairs)
    std::list<Trigger> triggers;

    //Internal hit pattern structure
    DataMatrix data;

    //root node of the trigger tree for mu+/-
    TNode* root[2];

    //container of roads found
    std::list<TriggerRoad> roads_found[2];

    //temporary container of uniqueIDs found
    std::list<int> roads_temp;

    //Trigger hodos  --- hodo station that are used in trigger
    std::vector<int> detectorIDs_trigger;
};

#endif
