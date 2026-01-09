/**
 * @name NewMissionDataMgr.cpp
 *   Mission data manager implementation
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#include "eve-server.h"
#include "missions/NewMissionDataMgr.h"
#include "missions/MissionDB.h"

/* Mission Logging
 * MISSION__ERROR
 * MISSION__WARNING
 * MISSION__INFO
 * MISSION__MESSAGE
 * MISSION__TRACE
 * MISSION__DEBUG
 */

NewMissionDataMgr::NewMissionDataMgr()
: m_nextStateID(1000),
  m_nextSpaceID(1)
{
    m_missionStates.clear();
    m_characterMissions.clear();
    m_objectiveProgress.clear();
    m_missionSpaces.clear();
}

NewMissionDataMgr::~NewMissionDataMgr()
{
    Clear();
}

int NewMissionDataMgr::Initialize()
{
    // Load from data directory
    std::string dataPath = "./src/eve-server/missions/data";
    if (!m_loader.LoadFromDirectory(dataPath)) {
        _log(MISSION__ERROR, "Failed to load missions from %s", dataPath.c_str());
        return 0;
    }

    _log(MISSION__INFO, "Mission system initialized with %lu missions", m_loader.GetMissionCount());
    return 1;
}

void NewMissionDataMgr::Close()
{
    _log(MISSION__INFO, "NewMissionDataMgr: Saving all mission states...");
    
    // Save all active mission states
    for (const auto& pair : m_missionStates) {
        SaveMissionState(pair.second);
    }
    
    Clear();
    _log(MISSION__INFO, "NewMissionDataMgr: Mission Data Manager Closed");
}

void NewMissionDataMgr::Clear()
{
    m_missionStates.clear();
    m_characterMissions.clear();
    m_objectiveProgress.clear();
    m_missionSpaces.clear();
    m_loader.Clear();
}

void NewMissionDataMgr::Process()
{
    // Process mission expirations
    ProcessMissionExpiration();
    
    // TODO: Process other periodic tasks
    // - Check for mission completion conditions
    // - Update mission timers
    // - Handle mission events
}

void NewMissionDataMgr::ProcessMissionExpiration()
{
    int64 currentTime = GetFileTimeNow();
    std::vector<uint32> expiredMissions;
    
    for (const auto& pair : m_missionStates) {
        if (pair.second.expiresAt > 0 && currentTime >= pair.second.expiresAt) {
            if (pair.second.status == MissionNew::Status::Accepted || 
                pair.second.status == MissionNew::Status::Offered) {
                expiredMissions.push_back(pair.first);
            }
        }
    }
    
    // Fail expired missions
    for (uint32 stateID : expiredMissions) {
        _log(MISSION__MESSAGE, "NewMissionDataMgr: Mission state %u has expired", stateID);
        FailMission(stateID);
        // TODO: Send notification to player if online
    }
}

bool NewMissionDataMgr::GetMission(uint32 missionID, MissionNew::MissionData& data) const
{
    return m_loader.GetMission(missionID, data);
}

bool NewMissionDataMgr::GetRandomMission(uint8 agentLevel, uint8 missionType, uint32 factionID, MissionNew::MissionData& data) const
{
    // Get missions matching the criteria
    std::vector<MissionNew::MissionData> candidates;
    
    // Iterate through all missions and filter
    const auto& allMissions = m_loader.GetMissions();
    for (const auto& pair : allMissions) {
        const auto& mission = pair.second;
        if (mission.difficulty == agentLevel &&
            mission.missionType == missionType &&
            (factionID == 0 || mission.factionID == factionID)) {
            candidates.push_back(mission);
        }
    }

    if (candidates.empty()) {
        _log(MISSION__WARNING, "NewMissionDataMgr: No missions found for level=%u type=%u faction=%u", 
             agentLevel, missionType, factionID);
        return false;
    }
    
    // Select random mission
    uint32 index = MakeRandomInt(0, candidates.size() - 1);
    data = candidates[index];
    
    _log(MISSION__TRACE, "NewMissionDataMgr: Selected mission '%s' (ID=%u) from %lu candidates", 
         data.name.c_str(), data.id, candidates.size());
    
    return true;
}

void NewMissionDataMgr::GetMissionsByType(uint8 type, std::vector<MissionNew::MissionData>& missions) const
{
    std::vector<uint32> missionIDs;
    m_loader.GetMissionsByType(type, missionIDs);
    
    // Convert IDs to mission data
    for (uint32 id : missionIDs) {
        MissionNew::MissionData mission;
        if (m_loader.GetMission(id, mission)) {
            missions.push_back(mission);
        }
    }
}

void NewMissionDataMgr::GetMissionsByDifficulty(uint8 difficulty, std::vector<MissionNew::MissionData>& missions) const
{
    std::vector<uint32> missionIDs;
    m_loader.GetMissionsByDifficulty(difficulty, missionIDs);
    
    // Convert IDs to mission data
    for (uint32 id : missionIDs) {
        MissionNew::MissionData mission;
        if (m_loader.GetMission(id, mission)) {
            missions.push_back(mission);
        }
    }
}

void NewMissionDataMgr::GetMissionsByFaction(uint32 factionID, std::vector<MissionNew::MissionData>& missions) const
{
    std::vector<uint32> missionIDs;
    m_loader.GetMissionsByFaction(factionID, missionIDs);
    
    // Convert IDs to mission data
    for (uint32 id : missionIDs) {
        MissionNew::MissionData mission;
        if (m_loader.GetMission(id, mission)) {
            missions.push_back(mission);
        }
    }
}

uint32 NewMissionDataMgr::CreateMissionState(uint32 characterID, uint32 missionID, uint32 agentID)
{
    // Get mission definition
    MissionNew::MissionData missionData;
    if (!GetMission(missionID, missionData)) {
        _log(MISSION__ERROR, "NewMissionDataMgr: Cannot create state for unknown mission %u", missionID);
        return 0;
    }
    
    // Create new state
    MissionNew::MissionState state;
    state.stateID = GetNextStateID();
    state.characterID = characterID;
    state.missionID = missionID;
    state.status = MissionNew::Status::Offered;
    state.currentStage = 0;
    state.startedAt = GetFileTimeNow();
    state.completedAt = 0;
    
    // Set expiration based on mission type
    if (missionData.estimatedDuration > 0) {
        // Expire after 3x estimated duration
        state.expiresAt = state.startedAt + (missionData.estimatedDuration * 3 * EvE::Time::Minute);
    } else {
        // Default 24 hour expiration
        state.expiresAt = state.startedAt + EvE::Time::Day;
    }
    
    state.missionSpaceID = 0;
    
    // Store state
    m_missionStates[state.stateID] = state;
    m_characterMissions.emplace(characterID, state.stateID);
    
    // Initialize objective progress
    std::vector<MissionNew::ObjectiveProgress> progress;
    for (const auto& obj : missionData.objectives) {
        MissionNew::ObjectiveProgress objProgress;
        objProgress.trackingID = 0; // Will be set when saved to DB
        objProgress.stateID = state.stateID;
        objProgress.objectiveID = obj.id;
        objProgress.progress = 0;
        objProgress.completed = false;
        objProgress.completedAt = 0;
        
        progress.push_back(objProgress);
    }
    m_objectiveProgress[state.stateID] = progress;
    
    // Save to database
    SaveMissionState(state);
    
    _log(MISSION__TRACE, "NewMissionDataMgr: Created mission state %u for character %u mission %u", 
         state.stateID, characterID, missionID);
    
    return state.stateID;
}

bool NewMissionDataMgr::GetMissionState(uint32 stateID, MissionNew::MissionState& state) const
{
    auto itr = m_missionStates.find(stateID);
    if (itr != m_missionStates.end()) {
        state = itr->second;
        return true;
    }
    return false;
}

bool NewMissionDataMgr::GetCharacterMissionState(uint32 characterID, uint32 missionID, MissionNew::MissionState& state) const
{
    auto range = m_characterMissions.equal_range(characterID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        auto stateItr = m_missionStates.find(itr->second);
        if (stateItr != m_missionStates.end() && stateItr->second.missionID == missionID) {
            state = stateItr->second;
            return true;
        }
    }
    return false;
}

void NewMissionDataMgr::GetCharacterMissions(uint32 characterID, std::vector<MissionNew::MissionState>& states) const
{
    states.clear();
    auto range = m_characterMissions.equal_range(characterID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        auto stateItr = m_missionStates.find(itr->second);
        if (stateItr != m_missionStates.end()) {
            states.push_back(stateItr->second);
        }
    }
}

void NewMissionDataMgr::UpdateMissionState(const MissionNew::MissionState& state)
{
    auto itr = m_missionStates.find(state.stateID);
    if (itr != m_missionStates.end()) {
        itr->second = state;
        SaveMissionState(state);
    } else {
        _log(MISSION__WARNING, "NewMissionDataMgr: Attempted to update non-existent mission state %u", state.stateID);
    }
}

void NewMissionDataMgr::CompleteObjective(uint32 stateID, uint32 objectiveID)
{
    auto itr = m_objectiveProgress.find(stateID);
    if (itr != m_objectiveProgress.end()) {
        for (auto& progress : itr->second) {
            if (progress.objectiveID == objectiveID) {
                progress.completed = true;
                progress.completedAt = GetFileTimeNow();
                
                // TODO: Save to database
                _log(MISSION__MESSAGE, "NewMissionDataMgr: Completed objective %u for mission state %u", 
                     objectiveID, stateID);
                
                // Check if all required objectives are complete
                MissionNew::MissionState state;
                if (GetMissionState(stateID, state)) {
                    MissionNew::MissionData missionData;
                    if (GetMission(state.missionID, missionData)) {
                        bool allComplete = true;
                        for (const auto& obj : missionData.objectives) {
                            if (obj.required) {
                                bool found = false;
                                for (const auto& prog : itr->second) {
                                    if (prog.objectiveID == obj.id && prog.completed) {
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) {
                                    allComplete = false;
                                    break;
                                }
                            }
                        }
                        
                        if (allComplete) {
                            _log(MISSION__MESSAGE, "NewMissionDataMgr: All required objectives complete for mission state %u", stateID);
                            // TODO: Trigger mission completion notification
                        }
                    }
                }
                break;
            }
        }
    }
}

void NewMissionDataMgr::UpdateObjectiveProgress(uint32 stateID, uint32 objectiveID, uint32 progress)
{
    auto itr = m_objectiveProgress.find(stateID);
    if (itr != m_objectiveProgress.end()) {
        for (auto& objProgress : itr->second) {
            if (objProgress.objectiveID == objectiveID) {
                objProgress.progress = progress;
                // TODO: Save to database
                break;
            }
        }
    }
}

void NewMissionDataMgr::CompleteMission(uint32 stateID)
{
    auto itr = m_missionStates.find(stateID);
    if (itr != m_missionStates.end()) {
        itr->second.status = MissionNew::Status::Completed;
        itr->second.completedAt = GetFileTimeNow();
        SaveMissionState(itr->second);
        
        _log(MISSION__MESSAGE, "NewMissionDataMgr: Mission state %u completed", stateID);
    }
}

void NewMissionDataMgr::FailMission(uint32 stateID)
{
    auto itr = m_missionStates.find(stateID);
    if (itr != m_missionStates.end()) {
        itr->second.status = MissionNew::Status::Failed;
        itr->second.completedAt = GetFileTimeNow();
        SaveMissionState(itr->second);
        
        _log(MISSION__MESSAGE, "NewMissionDataMgr: Mission state %u failed", stateID);
    }
}

void NewMissionDataMgr::AbortMission(uint32 stateID)
{
    auto itr = m_missionStates.find(stateID);
    if (itr != m_missionStates.end()) {
        itr->second.status = MissionNew::Status::Aborted;
        itr->second.completedAt = GetFileTimeNow();
        SaveMissionState(itr->second);
        
        _log(MISSION__MESSAGE, "NewMissionDataMgr: Mission state %u aborted", stateID);
    }
}

bool NewMissionDataMgr::IsMissionExpired(const MissionNew::MissionState& state) const
{
    if (state.expiresAt == 0) return false;
    return GetFileTimeNow() >= state.expiresAt;
}

float NewMissionDataMgr::GetMissionCompletionPercentage(const MissionNew::MissionState& state) const
{
    MissionNew::MissionData missionData;
    if (!GetMission(state.missionID, missionData)) {
        return 0.0f;
    }
    
    auto itr = m_objectiveProgress.find(state.stateID);
    if (itr == m_objectiveProgress.end()) {
        return 0.0f;
    }
    
    uint32 totalRequired = 0;
    uint32 completedRequired = 0;
    
    for (const auto& obj : missionData.objectives) {
        if (obj.required) {
            totalRequired++;
            for (const auto& progress : itr->second) {
                if (progress.objectiveID == obj.id && progress.completed) {
                    completedRequired++;
                    break;
                }
            }
        }
    }
    
    if (totalRequired == 0) return 0.0f;
    return (float)completedRequired / (float)totalRequired * 100.0f;
}

uint32 NewMissionDataMgr::CreateMissionSpace(uint32 stateID, uint32 systemID)
{
    uint32 spaceID = GetNextSpaceID();
    m_missionSpaces[stateID] = spaceID;
    
    // Update mission state
    auto itr = m_missionStates.find(stateID);
    if (itr != m_missionStates.end()) {
        itr->second.missionSpaceID = spaceID;
        SaveMissionState(itr->second);
    }
    
    _log(MISSION__TRACE, "NewMissionDataMgr: Created mission space %u for state %u in system %u", 
         spaceID, stateID, systemID);
    
    return spaceID;
}

uint32 NewMissionDataMgr::GetMissionSpaceID(uint32 stateID) const
{
    auto itr = m_missionSpaces.find(stateID);
    if (itr != m_missionSpaces.end()) {
        return itr->second;
    }
    return 0;
}

void NewMissionDataMgr::RemoveMissionSpace(uint32 missionSpaceID)
{
    for (auto itr = m_missionSpaces.begin(); itr != m_missionSpaces.end(); ++itr) {
        if (itr->second == missionSpaceID) {
            // Update mission state
            auto stateItr = m_missionStates.find(itr->first);
            if (stateItr != m_missionStates.end()) {
                stateItr->second.missionSpaceID = 0;
                SaveMissionState(stateItr->second);
            }
            
            m_missionSpaces.erase(itr);
            _log(MISSION__TRACE, "NewMissionDataMgr: Removed mission space %u", missionSpaceID);
            break;
        }
    }
}

void NewMissionDataMgr::SaveMissionState(const MissionNew::MissionState& state)
{
    // TODO: Implement database save
    // MissionDB::SaveMissionState(state);
    _log(MISSION__TRACE, "NewMissionDataMgr: Saved mission state %u to database", state.stateID);
}

void NewMissionDataMgr::LoadMissionStates()
{
    // TODO: Implement database load
    // Load all active mission states from database
    // For each state, load objective progress
    
    _log(MISSION__TRACE, "NewMissionDataMgr: Loaded mission states from database");
}
