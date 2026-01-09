/**
 * @name MissionDB.h
 *   Database operations for mission system
 *
 * @Author:        Allan
 * @date:          24 June 2018
 * @Updated:       January 2026 - Added JSON mission system support
 */

#ifndef _EVE_SERVER_MISSION_DATABASE_H__
#define _EVE_SERVER_MISSION_DATABASE_H__

#include "../ServiceDB.h"
#include "../eve-server.h"
#include "../../eve-common/EVE_Missions.h"
#include "missions/MissionData.h"

class MissionDB
{
public:
    // Legacy mission system (keep for now)
    static void CreateOfferID(MissionOffer& data);
    static void LoadOpenOffers(DBQueryResult& res);
    static void LoadMiningData(DBQueryResult& res);
    static void LoadMissionData(DBQueryResult& res);
    static void LoadCourierData(DBQueryResult& res);
    static void LoadClosedOffers(DBQueryResult& res);
    static void LoadMissionBookMark(DBQueryResult& res, std::vector<int32>& bmIDs);
    static void UpdateMissionOffer(MissionOffer& data);
    static void DeleteOffer(MissionOffer& data);
    static void RemoveMissionItem(uint32 charID, uint16 typeID, uint32 qty);

    // New JSON-based mission system
    // Mission State Management
    static bool SaveMissionState(const MissionNew::MissionState& state);
    static bool LoadMissionState(uint32 stateID, MissionNew::MissionState& state);
    static void LoadAllMissionStates(std::vector<MissionNew::MissionState>& states);
    static void LoadCharacterMissionStates(uint32 characterID, std::vector<MissionNew::MissionState>& states);
    static void UpdateMissionStatus(uint32 stateID, uint8 status, int64 completedAt = 0);
    static void UpdateMissionStage(uint32 stateID, uint32 stage);
    static void DeleteMissionState(uint32 stateID);

    // Objective Progress
    static bool SaveObjectiveProgress(const MissionNew::ObjectiveProgress& progress);
    static void LoadObjectiveProgress(uint32 stateID, std::vector<MissionNew::ObjectiveProgress>& progress);
    static void UpdateObjectiveProgress(uint32 trackingID, uint32 progress);
    static void CompleteObjective(uint32 trackingID, int64 completedAt);

    // Mission Spaces
    static uint32 CreateMissionSpace(uint32 stateID, uint32 systemID);
    static bool LoadMissionSpace(uint32 spaceID, uint32& stateID, uint32& systemID);
    static void DeleteMissionSpace(uint32 spaceID);
    static void CleanupExpiredSpaces();

    // Mission Space Objects
    static uint32 SaveMissionSpaceObject(uint32 spaceID, uint32 itemID, uint16 typeID, 
                                         uint16 groupID, const GPoint& position);
    static void LoadMissionSpaceObjects(uint32 spaceID, std::vector<uint32>& itemIDs);
    static void DeleteMissionSpaceObject(uint32 objectID);

    // Mission Bookmarks
    static uint32 SaveMissionBookmark(uint32 stateID, uint32 ownerID, uint32 itemID, 
                                      uint16 typeID, const GPoint& position, 
                                      uint32 locationID, const std::string& memo);
    static void LoadMissionBookmarks(uint32 stateID, std::vector<uint32>& bookmarkIDs);
    static void DeleteMissionBookmark(uint32 bookmarkID);

    // Mission Statistics
    static void SaveMissionStatistics(uint32 characterID, uint32 missionID, uint32 agentID,
                                      int64 completedAt, uint32 durationMinutes,
                                      uint32 iskReward, uint32 lpReward, 
                                      uint32 failedObjectives);
    static void LoadMissionStatistics(uint32 characterID, DBQueryResult& res);

protected:
};

#endif  // _EVE_SERVER_MISSION_DATABASE_H__
