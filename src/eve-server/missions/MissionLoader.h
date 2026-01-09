/**
 * @name MissionLoader.h
 *   JSON mission loader for new mission system
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#ifndef _EVE_SERVER_MISSION_LOADER_H__
#define _EVE_SERVER_MISSION_LOADER_H__

#include "missions/MissionData.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

/**
 * @brief Loads mission definitions from JSON files
 * 
 * This class is responsible for loading mission data from JSON files
 * located in src/eve-server/missions/data/
 */
class MissionLoader
{
public:
    MissionLoader();
    ~MissionLoader();

    /**
     * @brief Load missions from a specific JSON file
     * @param filepath Path to the JSON file
     * @return true if loading was successful
     */
    bool LoadFromFile(const std::string& filepath);

    /**
     * @brief Load all missions from a directory
     * @param dirPath Path to directory containing JSON files
     * @return true if loading was successful
     */
    bool LoadFromDirectory(const std::string& dirPath);

    /**
     * @brief Get all loaded missions
     * @return Map of mission ID to mission data
     */
    const std::map<uint32, MissionNew::MissionData>& GetMissions() const;

    /**
     * @brief Get mission by ID
     * @param missionID The mission ID to find
     * @param outMission Output parameter for mission data
     * @return true if mission was found
     */
    bool GetMission(uint32 missionID, MissionNew::MissionData& outMission) const;

    /**
     * @brief Get mission IDs by type
     * @param type Mission type
     * @param outMissionIDs Output vector of mission IDs
     */
    void GetMissionsByType(uint8 type, std::vector<uint32>& outMissionIDs) const;

    /**
     * @brief Get mission IDs by difficulty level
     * @param difficulty Difficulty level (1-5)
     * @param outMissionIDs Output vector of mission IDs
     */
    void GetMissionsByDifficulty(uint8 difficulty, std::vector<uint32>& outMissionIDs) const;

    /**
     * @brief Get mission IDs by faction
     * @param factionID Faction ID
     * @param outMissionIDs Output vector of mission IDs
     */
    void GetMissionsByFaction(uint32 factionID, std::vector<uint32>& outMissionIDs) const;

    /**
     * @brief Get total number of loaded missions
     */
    size_t GetMissionCount() const;

    /**
     * @brief Clear all loaded missions
     */
    void Clear();

private:
    std::map<uint32, MissionNew::MissionData> m_missions;
    std::map<uint8, std::vector<uint32>> m_missionsByType;
    std::map<uint8, std::vector<uint32>> m_missionsByDifficulty;
    std::map<uint32, std::vector<uint32>> m_missionsByFaction;

    /**
     * @brief Add a mission to internal storage and indexes
     */
    void AddMission(const MissionNew::MissionData& mission);
};

#endif // _EVE_SERVER_MISSION_LOADER_H__
