/**
  * @name MissionDB.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  * @Updated:   January 2026 - Added JSON mission system support
  *
  */

/* Mission Logging:
 * MISSION__ERROR
 * MISSION__WARNING
 * MISSION__MESSAGE
 * MISSION__DEBUG
 * MISSION__INFO
 * MISSION__TRACE
 */

#include "missions/MissionDB.h"
#include "database/EVEDBUtils.h"

// ========== LEGACY MISSION SYSTEM METHODS ==========

void MissionDB::LoadMissionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, briefingID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,"
        " rewardISK, rewardItemID, rewardISK, rewardItemQty, bonusISK, bonusTime FROM agtMissions WHERE briefingID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMissionData query: %s", res.error.c_str());
}

void MissionDB::LoadCourierData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.important, q.storyline, q.itemTypeID, q.itemQty, it.volume, q.rewardISK, q.rewardItemID,"
        " q.rewardItemQty, q.bonusISK, q.bonusTime, q.sysRange, q.raceID"
        " FROM qstCourier AS q LEFT JOIN invTypes AS it ON it.typeID = itemTypeID WHERE briefingID > 0 AND itemTypeID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadCourierData query: %s", res.error.c_str());
}

void MissionDB::LoadMiningData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.important, q.storyline, q.itemTypeID, q.itemQty, it.volume, q.rewardISK, q.rewardItemID,"
        " q.rewardItemQty, q.bonusISK, q.bonusTime, q.sysRange, q.raceID"
        " FROM qstMining AS q LEFT JOIN invTypes AS it ON it.typeID = itemTypeID WHERE briefingID > 0 AND itemTypeID > 0 AND rewardISK > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMiningData query: %s", res.error.c_str());
}

void MissionDB::CreateOfferID(MissionOffer& data)
{
    DBerror err;
    uint32 uid = 0;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO agtOffers(acceptFee, agentID, characterID, courierAmount, courierTypeID, courierVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, "
        " destinationOwnerID, destinationSystemID, expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID,"
        " remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID)"
        " VALUES ("
        " %u, %u, %u, %u, %u, %f, %f, %f, %u,"
        " %u, %u, %u, %f, %i, %u,"
        " %u, %u, '%s', %u, %u, %u, %u, %i,"
        " %i, %u, %u, %u, %u, %u, %u, %u,"
        " %u, %u, %u)",
            data.acceptFee, data.agentID, data.characterID, data.courierAmount, data.courierTypeID, data.courierItemVolume, data.dateAccepted, data.dateIssued, data.destinationID,
            data.destinationTypeID, data.destinationOwnerID, data.destinationSystemID, data.expiryTime, (data.important?1:0), data.storyline,
            data.missionID, data.briefingID, data.name.c_str(), data.offerID, data.originID, data.originOwnerID, data.originSystemID,
            (data.remoteCompletable?1:0), (data.remoteOfferable?1:0), data.rewardISK, data.rewardItemID, data.rewardItemQty,data.rewardLP, data.bonusISK, data.bonusTime,
            data.stateID, data.typeID, data.dungeonLocationID, data.dungeonSolarSystemID))
    {
        codelog(DATABASE__ERROR, "Failed to insert new MissionOffer: %s", err.c_str());
        return;
    }

    data.offerID = uid;
}

void MissionDB::DeleteOffer(MissionOffer& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM agtOffers WHERE offerID = %u", data.offerID);
}

void MissionDB::UpdateMissionOffer(MissionOffer& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE agtOffers SET stateID = %u, dateAccepted = %f, dateCompleted = %f, expiryTime = %f WHERE offerID = %u",
        data.stateID, data.dateAccepted, data.dateCompleted, data.expiryTime, data.offerID))
    {
        codelog(DATABASE__ERROR, "Failed to update MissionOffer: %s", err.c_str());
    }
}

void MissionDB::LoadOpenOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT acceptFee, agentID, characterID, courierAmount, courierTypeID, courierVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, destinationOwnerID, destinationSystemID,"
        " expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID, remoteCompletable, remoteOfferable, "
        " rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID "
        " FROM agtOffers WHERE dateCompleted = 0 AND stateID < 3"))
        codelog(DATABASE__ERROR, "Error in LoadOpenOffers query: %s", res.error.c_str());
}

void MissionDB::LoadClosedOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT agentID, characterID, courierAmount, courierTypeID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline, missionID, name,"
        " offerID, originID, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers WHERE dateCompleted > 0 OR expiryTime > %f OR stateID > 2", GetFileTimeNow()))
        codelog(DATABASE__ERROR, "Error in LoadClosedOffers query: %s", res.error.c_str());
}

void MissionDB::LoadMissionBookMark(DBQueryResult& res, std::vector<int32>& bmIDs)
{
    std::string ids = "";
    ListToINString(bmIDs, ids);
    if (!sDatabase.RunQuery(res,
        "SELECT bookmarkID, ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID, folderID"
        " FROM bookmarks WHERE bookmarkID IN (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

void MissionDB::RemoveMissionItem(uint32 charID, uint16 typeID, uint32 qty)
{
    //  this may get a bit complicated if the items are split.
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, quantity FROM entity WHERE typeID = %u AND ownerID = %u", typeID, charID);

    DBResultRow row;
    std::map<uint32, uint16> map;
    while (res.GetRow(row)) {
        // make map of all items of 'typeID'
        map.emplace(row.GetInt(0), row.GetInt(1));
    }

    DBerror err;
    for (auto cur : map) {
        if (qty < 1)
            break;
        if (cur.second <= qty) {
            qty -= cur.second;
            sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", cur.first);
        } else if (cur.second > qty) {
            sDatabase.RunQuery(err, "UPDATE entity SET quantity = %u WHERE itemID = %u", qty, cur.first);
            qty = 0;
        }
    }
}

// ========== NEW JSON-BASED MISSION SYSTEM METHODS ==========

bool MissionDB::SaveMissionState(const MissionNew::MissionState& state)
{
    DBerror err;
    uint32 stateID = 0;
    
    if (!sDatabase.RunQueryLID(err, stateID,
        "INSERT INTO missionStates ("
        " characterID, missionID, status, currentStage,"
        " startedAt, completedAt, expiresAt, missionSpaceID)"
        " VALUES (%u, %u, %u, %u, %li, %li, %li, %u)",
        state.characterID, state.missionID, state.status, state.currentStage,
        state.startedAt, state.completedAt, state.expiresAt, state.missionSpaceID))
    {
        _log(MISSION__ERROR, "SaveMissionState failed: %s", err.c_str());
        return false;
    }
    
    // Save completed objectives
    for (uint32 objID : state.objectivesCompleted) {
        MissionNew::ObjectiveProgress objProg;
        objProg.stateID = stateID;
        objProg.objectiveID = objID;
        objProg.progress = 100;
        objProg.completed = true;
        objProg.completedAt = GetFileTimeNow();
        SaveObjectiveProgress(objProg);
    }
    
    _log(MISSION__TRACE, "Saved mission state %u for character %u", stateID, state.characterID);
    return true;
}

bool MissionDB::LoadMissionState(uint32 stateID, MissionNew::MissionState& state)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT characterID, missionID, status, currentStage,"
        " startedAt, completedAt, expiresAt, missionSpaceID"
        " FROM missionStates WHERE stateID = %u", stateID))
    {
        _log(MISSION__ERROR, "LoadMissionState failed for stateID %u", stateID);
        return false;
    }
    
    DBResultRow row;
    if (!res.GetRow(row))
        return false;
    
    state.stateID = stateID;
    state.characterID = row.GetUInt(0);
    state.missionID = row.GetUInt(1);
    state.status = row.GetUInt(2);
    state.currentStage = row.GetUInt(3);
    state.startedAt = row.GetInt64(4);
    state.completedAt = row.GetInt64(5);
    state.expiresAt = row.GetInt64(6);
    state.missionSpaceID = row.GetUInt(7);
    
    // Load completed objectives
    std::vector<MissionNew::ObjectiveProgress> objectives;
    LoadObjectiveProgress(stateID, objectives);
    for (const auto& obj : objectives) {
        if (obj.completed) {
            state.objectivesCompleted.push_back(obj.objectiveID);
        }
    }
    
    _log(MISSION__TRACE, "Loaded mission state %u with %lu completed objectives", 
         stateID, state.objectivesCompleted.size());
    return true;
}

void MissionDB::LoadAllMissionStates(std::vector<MissionNew::MissionState>& states)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT stateID, characterID, missionID, status, currentStage,"
        " startedAt, completedAt, expiresAt, missionSpaceID"
        " FROM missionStates WHERE status < 3"))
    {
        _log(MISSION__ERROR, "LoadAllMissionStates failed");
        return;
    }
    
    DBResultRow row;
    while (res.GetRow(row)) {
        MissionNew::MissionState state;
        state.stateID = row.GetUInt(0);
        state.characterID = row.GetUInt(1);
        state.missionID = row.GetUInt(2);
        state.status = row.GetUInt(3);
        state.currentStage = row.GetUInt(4);
        state.startedAt = row.GetInt64(5);
        state.completedAt = row.GetInt64(6);
        state.expiresAt = row.GetInt64(7);
        state.missionSpaceID = row.GetUInt(8);
        
        // Load completed objectives
        std::vector<MissionNew::ObjectiveProgress> objectives;
        LoadObjectiveProgress(state.stateID, objectives);
        for (const auto& obj : objectives) {
            if (obj.completed) {
                state.objectivesCompleted.push_back(obj.objectiveID);
            }
        }
        
        states.push_back(state);
    }
    
    _log(MISSION__INFO, "Loaded %lu active mission states", states.size());
}

void MissionDB::LoadCharacterMissionStates(uint32 characterID, std::vector<MissionNew::MissionState>& states)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT stateID, missionID, status, currentStage,"
        " startedAt, completedAt, expiresAt, missionSpaceID"
        " FROM missionStates WHERE characterID = %u", characterID))
    {
        _log(MISSION__ERROR, "LoadCharacterMissionStates failed for character %u", characterID);
        return;
    }
    
    DBResultRow row;
    while (res.GetRow(row)) {
        MissionNew::MissionState state;
        state.stateID = row.GetUInt(0);
        state.characterID = characterID;
        state.missionID = row.GetUInt(1);
        state.status = row.GetUInt(2);
        state.currentStage = row.GetUInt(3);
        state.startedAt = row.GetInt64(4);
        state.completedAt = row.GetInt64(5);
        state.expiresAt = row.GetInt64(6);
        state.missionSpaceID = row.GetUInt(7);
        
        // Load completed objectives
        std::vector<MissionNew::ObjectiveProgress> objectives;
        LoadObjectiveProgress(state.stateID, objectives);
        for (const auto& obj : objectives) {
            if (obj.completed) {
                state.objectivesCompleted.push_back(obj.objectiveID);
            }
        }
        
        states.push_back(state);
    }
    
    _log(MISSION__TRACE, "Loaded %lu mission states for character %u", states.size(), characterID);
}

void MissionDB::UpdateMissionStatus(uint32 stateID, uint8 status, int64 completedAt)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE missionStates SET status = %u, completedAt = %li WHERE stateID = %u",
        status, completedAt, stateID))
    {
        _log(MISSION__ERROR, "UpdateMissionStatus failed: %s", err.c_str());
    }
}

void MissionDB::UpdateMissionStage(uint32 stateID, uint32 stage)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE missionStates SET currentStage = %u WHERE stateID = %u",
        stage, stateID))
    {
        _log(MISSION__ERROR, "UpdateMissionStage failed: %s", err.c_str());
    }
}

void MissionDB::DeleteMissionState(uint32 stateID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM missionStates WHERE stateID = %u", stateID))
    {
        _log(MISSION__ERROR, "DeleteMissionState failed: %s", err.c_str());
    }
}

bool MissionDB::SaveObjectiveProgress(const MissionNew::ObjectiveProgress& progress)
{
    DBerror err;
    uint32 trackingID = 0;
    
    if (!sDatabase.RunQueryLID(err, trackingID,
        "INSERT INTO missionObjectiveProgress ("
        " stateID, objectiveID, progress, completed, completedAt)"
        " VALUES (%u, %u, %u, %u, %li)",
        progress.stateID, progress.objectiveID, progress.progress,
        progress.completed ? 1 : 0, progress.completedAt))
    {
        _log(MISSION__ERROR, "SaveObjectiveProgress failed: %s", err.c_str());
        return false;
    }
    
    return true;
}

void MissionDB::LoadObjectiveProgress(uint32 stateID, std::vector<MissionNew::ObjectiveProgress>& progress)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT trackingID, objectiveID, progress, completed, completedAt"
        " FROM missionObjectiveProgress WHERE stateID = %u", stateID))
    {
        _log(MISSION__ERROR, "LoadObjectiveProgress failed for stateID %u", stateID);
        return;
    }
    
    DBResultRow row;
    while (res.GetRow(row)) {
        MissionNew::ObjectiveProgress objProg;
        objProg.trackingID = row.GetUInt(0);
        objProg.stateID = stateID;
        objProg.objectiveID = row.GetUInt(1);
        objProg.progress = row.GetUInt(2);
        objProg.completed = row.GetBool(3);
        objProg.completedAt = row.GetInt64(4);
        progress.push_back(objProg);
    }
}

void MissionDB::UpdateObjectiveProgress(uint32 trackingID, uint32 progress)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE missionObjectiveProgress SET progress = %u WHERE trackingID = %u",
        progress, trackingID))
    {
        _log(MISSION__ERROR, "UpdateObjectiveProgress failed: %s", err.c_str());
    }
}

void MissionDB::CompleteObjective(uint32 trackingID, int64 completedAt)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE missionObjectiveProgress SET completed = 1, completedAt = %li WHERE trackingID = %u",
        completedAt, trackingID))
    {
        _log(MISSION__ERROR, "CompleteObjective failed: %s", err.c_str());
    }
}

uint32 MissionDB::CreateMissionSpace(uint32 stateID, uint32 systemID)
{
    DBerror err;
    uint32 spaceID = 0;
    int64 now = GetFileTimeNow();
    int64 expires = now + (24 * EvE::Time::Hour);
    
    if (!sDatabase.RunQueryLID(err, spaceID,
        "INSERT INTO missionSpaces (stateID, systemID, createdAt, expiresAt)"
        " VALUES (%u, %u, %li, %li)",
        stateID, systemID, now, expires))
    {
        _log(MISSION__ERROR, "CreateMissionSpace failed: %s", err.c_str());
        return 0;
    }
    
    _log(MISSION__TRACE, "Created mission space %u for state %u", spaceID, stateID);
    return spaceID;
}

bool MissionDB::LoadMissionSpace(uint32 spaceID, uint32& stateID, uint32& systemID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT stateID, systemID FROM missionSpaces WHERE spaceID = %u", spaceID))
    {
        _log(MISSION__ERROR, "LoadMissionSpace failed for spaceID %u", spaceID);
        return false;
    }
    
    DBResultRow row;
    if (!res.GetRow(row))
        return false;
    
    stateID = row.GetUInt(0);
    systemID = row.GetUInt(1);
    return true;
}

void MissionDB::DeleteMissionSpace(uint32 spaceID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM missionSpaces WHERE spaceID = %u", spaceID))
    {
        _log(MISSION__ERROR, "DeleteMissionSpace failed: %s", err.c_str());
    }
}

void MissionDB::CleanupExpiredSpaces()
{
    DBerror err;
    int64 now = GetFileTimeNow();
    if (!sDatabase.RunQuery(err,
        "DELETE FROM missionSpaces WHERE expiresAt < %li", now))
    {
        _log(MISSION__ERROR, "CleanupExpiredSpaces failed: %s", err.c_str());
    }
}

uint32 MissionDB::SaveMissionSpaceObject(uint32 spaceID, uint32 itemID, uint16 typeID, 
                                         uint16 groupID, const GPoint& position)
{
    DBerror err;
    uint32 objectID = 0;
    int64 now = GetFileTimeNow();
    
    if (!sDatabase.RunQueryLID(err, objectID,
        "INSERT INTO missionSpaceObjects ("
        " spaceID, itemID, typeID, groupID, x, y, z, spawnedAt)"
        " VALUES (%u, %u, %u, %u, %f, %f, %f, %li)",
        spaceID, itemID, typeID, groupID, position.x, position.y, position.z, now))
    {
        _log(MISSION__ERROR, "SaveMissionSpaceObject failed: %s", err.c_str());
        return 0;
    }
    
    return objectID;
}

void MissionDB::LoadMissionSpaceObjects(uint32 spaceID, std::vector<uint32>& itemIDs)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT itemID FROM missionSpaceObjects WHERE spaceID = %u", spaceID))
    {
        _log(MISSION__ERROR, "LoadMissionSpaceObjects failed for spaceID %u", spaceID);
        return;
    }
    
    DBResultRow row;
    while (res.GetRow(row))
        itemIDs.push_back(row.GetUInt(0));
}

void MissionDB::DeleteMissionSpaceObject(uint32 objectID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM missionSpaceObjects WHERE objectID = %u", objectID))
    {
        _log(MISSION__ERROR, "DeleteMissionSpaceObject failed: %s", err.c_str());
    }
}

uint32 MissionDB::SaveMissionBookmark(uint32 stateID, uint32 ownerID, uint32 itemID, 
                                      uint16 typeID, const GPoint& position, 
                                      uint32 locationID, const std::string& memo)
{
    DBerror err;
    uint32 bookmarkID = 0;
    int64 now = GetFileTimeNow();
    
    std::string escapedMemo;
    sDatabase.DoEscapeString(escapedMemo, memo);
    
    if (!sDatabase.RunQueryLID(err, bookmarkID,
        "INSERT INTO missionBookmarks ("
        " stateID, ownerID, itemID, typeID, flag, memo, created,"
        " x, y, z, locationID)"
        " VALUES (%u, %u, %u, %u, 0, '%s', %li, %f, %f, %f, %u)",
        stateID, ownerID, itemID, typeID, escapedMemo.c_str(), now,
        position.x, position.y, position.z, locationID))
    {
        _log(MISSION__ERROR, "SaveMissionBookmark failed: %s", err.c_str());
        return 0;
    }
    
    return bookmarkID;
}

void MissionDB::LoadMissionBookmarks(uint32 stateID, std::vector<uint32>& bookmarkIDs)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT bookmarkID FROM missionBookmarks WHERE stateID = %u", stateID))
    {
        _log(MISSION__ERROR, "LoadMissionBookmarks failed for stateID %u", stateID);
        return;
    }
    
    DBResultRow row;
    while (res.GetRow(row))
        bookmarkIDs.push_back(row.GetUInt(0));
}

void MissionDB::DeleteMissionBookmark(uint32 bookmarkID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "DELETE FROM missionBookmarks WHERE bookmarkID = %u", bookmarkID))
    {
        _log(MISSION__ERROR, "DeleteMissionBookmark failed: %s", err.c_str());
    }
}

void MissionDB::SaveMissionStatistics(uint32 characterID, uint32 missionID, uint32 agentID,
                                      int64 completedAt, uint32 durationMinutes,
                                      uint32 iskReward, uint32 lpReward, 
                                      uint32 failedObjectives)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO missionStatistics ("
        " characterID, missionID, agentID, completedAt, durationMinutes,"
        " iskReward, lpReward, failedObjectives)"
        " VALUES (%u, %u, %u, %li, %u, %u, %u, %u)",
        characterID, missionID, agentID, completedAt, durationMinutes,
        iskReward, lpReward, failedObjectives))
    {
        _log(MISSION__ERROR, "SaveMissionStatistics failed: %s", err.c_str());
    }
}

void MissionDB::LoadMissionStatistics(uint32 characterID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT missionID, agentID, completedAt, durationMinutes,"
        " iskReward, lpReward, failedObjectives"
        " FROM missionStatistics WHERE characterID = %u"
        " ORDER BY completedAt DESC LIMIT 100", characterID))
    {
        _log(MISSION__ERROR, "LoadMissionStatistics failed for character %u", characterID);
    }
}

