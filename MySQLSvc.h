/*
IO manager to handle fast extraction of data from database or upload data to database

Author: Kun Liu, liuk@fnal.gov
Created: 2013.9.29
*/

#ifndef _MYSQLSVC_H
#define _MYSQLSVC_H

#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <algorithm>

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TRandom.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TLorentzVector.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "FastTracklet.h"
#include "TriggerAnalyzer.h"

//#define OUT_TO_SCREEN
//#define USE_M_TABLES

class MySQLSvc
{
public:
    MySQLSvc();
    ~MySQLSvc();
    static MySQLSvc* instance();

    //Connect to the server
    bool connect(std::string sqlServer = MYSQL_SERVER_ADDR, int serverPort = MYSQL_SERVER_PORT);

    //Get the direct pointer to mysql server connection
    TSQLServer* getServer() { return server; }

    //Set username/password
    void setUserPasswd(std::string user_input, std::string passwd_input) { user = user_input; passwd = passwd_input; }

    //check if the run is stopped
    bool isRunStopped();

    //Get run info
    int getNEventsFast();
    int getNEvents();

    //Gets
    bool getEvent(SRawEvent* rawEvent, int eventID);
    bool getLatestEvt(SRawEvent* rawEvent);
    bool getRandomEvt(SRawEvent* rawEvent);
    bool getNextEvent(SRawEvent* rawEvent);
    bool getNextEvent(SRawMCEvent* rawEvent);

    //Check if the event has been loaded
    bool isEventLoaded(int eventID) { return std::find(eventIDs_loaded.begin(), eventIDs_loaded.end(), eventID) != eventIDs_loaded.end(); }

    //Get the event header
    bool getEventHeader(SRawEvent* rawEvent, int eventID);
    bool getMCGenInfo(SRawMCEvent* mcEvent, int eventID);

    //initialize reader -- check the indexing, table existence
    bool initReader();

    //Output to database/txt file/screen
    bool initWriter();
    void writeTrackingRes(SRecEvent* recEvent, TClonesArray* tracklets = NULL);
    void writeTrackTable(int trackID, SRecTrack* recTrack);
    void writeTrackHitTable(int trackID, Tracklet* tracklet);
    void writeDimuonTable(int dimuonID, SRecDimuon dimuon);

    //Set the data schema
    void setWorkingSchema(std::string schema);
    void setLoggingSchema(std::string schema) { logSchema = schema; }
    void enableQIE(bool opt) { readQIE = opt; }
    void enableTargetPos(bool opt) { readTargetPos = opt; }
    void enableTriggerHits(bool opt) { readTriggerHits = opt; }

    //Memory-safe sql queries
    int makeQuery();
    bool nextEntry();

    int getInt(int id, int default_val = 0);
    float getFloat(int id, float default_val = 0.);
    double getDouble(int id, double default_val = 0.);
    std::string getString(int id, std::string default_val = "");

private:
    //pointer to the only instance
    static MySQLSvc* p_mysqlSvc;

    //Username and password
    std::string user;
    std::string passwd;

    //pointer to the geometry service
    GeomSvc* p_geomSvc;

    //pointer to trigger analyzer
    TriggerAnalyzer* p_triggerAna;

    //SQL server
    TSQLServer* server;
    TSQLResult* res;
    TSQLRow* row;

    //Test if QIE/TriggerHits table exists
    bool readQIE;
    bool readTriggerHits;
    bool readTargetPos;
    bool readTrackPos;
    bool setTriggerEmu;

    //Random generator
    TRandom rndm;

    //run-related info
    int runID;
    int spillID;
    int nEvents;

    //event list
    std::vector<int> eventIDs;
    std::vector<int> eventIDs_loaded;
    int index_eventID;

    //Query string used in all clause
    char query[2000];

    //name of the production schema working on
    std::string dataSchema;
    std::string logSchema;

    //Internal counter of tracks and dimuons
    int nTracks;
    int nDimuons;
};

#endif
