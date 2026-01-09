/**
 * @name NewMissionDataMgr.h
 *   Mission data manager for JSON-based mission system
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#ifndef _EVE_SERVER_NEW_MISSION_DATAMGR_H__
#define _EVE_SERVER_NEW_MISSION_DATAMGR_H__

#include "eve-server.h"
#include "missions/MissionData.h"
#include "missions/MissionLoader.h"

/**
 * @brief Manages mission data and player mission states
 * 
 * This singleton manages all loaded mission definitions and tracks
 * player mission states in memory with database persistence.
 */
class NewMissionDataMgr : public Singleton<NewMissionDataMgr>
{
public:
    NewMissionDataMgr();
    ~NewMissionDataMgr();

    /**
     * @brief Initialize the mission system
     * @return 1 on success, 0 on failure
     */
    int Initialize();

    /**
     * @brief Cleanup and save all mission states
     */
    void Close();

    /**
     * @brief Clear all loaded data
     */
    void Clear();

    /**
     * @brief Process mission states (called periodically)
     * Handles mission expiration, objective updates, etc.
     */
    void Process();

    // Mission Definition Access
    /**
     * @brief Get mission by ID
     */
    bool GetMission(uint32 missionID, MissionNew::MissionData& data) const;

    /**
     * @brief Get random mission for agent level and type
     */
    bool GetRandomMission(uint8 agentLevel, uint8 missionType, uint32 factionID, MissionNew::MissionData& data) const;

    /**
     * @brief Get missions by criteria
     */
    void GetMissionsByType(uint8 type, std::vector<MissionNew::MissionData>& missions) const;
    void GetMissionsByDifficulty(uint8 difficulty, std::vector<MissionNew::MissionData>& missions) const;
    void GetMissionsByFaction(uint32 factionID, std::vector<MissionNew::MissionData>& missions) const;

    // Mission State Management
    /**
     * @brief Create a new mission state for a character
     */
    uint32 CreateMissionState(uint32 characterID, uint32 missionID, uint32 agentID);

    /**
     * @brief Get mission state for a character
     */
    bool GetMissionState(uint32 stateID, MissionNew::MissionState& state) const;
    bool GetCharacterMissionState(uint32 characterID, uint32 missionID, MissionNew::MissionState& state) const;

    /**
     * @brief Get all active missions for a character
     */
    void GetCharacterMissions(uint32 characterID, std::vector<MissionNew::MissionState>& states) const;

    /**
     * @brief Update mission state
     */
    void UpdateMissionState(const MissionNew::MissionState& state);

    /**
     * @brief Complete an objective
     */
    void CompleteObjective(uint32 stateID, uint32 objectiveID);

    /**
     * @brief Update objective progress
     */
    void UpdateObjectiveProgress(uint32 stateID, uint32 objectiveID, uint32 progress);

    /**
     * @brief Complete a mission
     */
    void CompleteMission(uint32 stateID);

    /**
     * @brief Fail a mission
     */
    void FailMission(uint32 stateID);

    /**
     * @brief Abort a mission
     */
    void AbortMission(uint32 stateID);

    /**
     * @brief Check if mission has expired
     */
    bool IsMissionExpired(const MissionNew::MissionState& state) const;

    /**
     * @brief Get completion percentage for a mission
     */
    float GetMissionCompletionPercentage(const MissionNew::MissionState& state) const;

    // Mission Space Management
    /**
     * @brief Create mission space instance for a mission
     */
    uint32 CreateMissionSpace(uint32 stateID, uint32 systemID);

    /**
     * @brief Get mission space ID for a mission state
     */
    uint32 GetMissionSpaceID(uint32 stateID) const;

    /**
     * @brief Remove mission space instance
     */
    void RemoveMissionSpace(uint32 missionSpaceID);

    // Statistics
    /**
     * @brief Get total number of loaded missions
     */
    size_t GetMissionCount() const { return m_loader.GetMissions().size(); }

    /**
     * @brief Get number of active mission states
     */
    size_t GetActiveMissionCount() const { return m_missionStates.size(); }

private:
    MissionLoader m_loader;
    
    // Mission state tracking
    std::map<uint32, MissionNew::MissionState> m_missionStates;  // stateID -> state
    std::multimap<uint32, uint32> m_characterMissions;        // characterID -> stateID
    uint32 m_nextStateID;

    // Objective progress tracking
    std::map<uint32, std::vector<MissionNew::ObjectiveProgress>> m_objectiveProgress; // stateID -> progress

    // Mission space instances
    std::map<uint32, uint32> m_missionSpaces;  // stateID -> spaceID
    uint32 m_nextSpaceID;

    // Helper methods
    uint32 GetNextStateID() { return ++m_nextStateID; }
    uint32 GetNextSpaceID() { return ++m_nextSpaceID; }
    
    void SaveMissionState(const MissionNew::MissionState& state);
    void LoadMissionStates();
    void ProcessMissionExpiration();
};

// Singleton access
#define sNewMissionDataMgr (NewMissionDataMgr::get())

#endif // _EVE_SERVER_NEW_MISSION_DATAMGR_H__
