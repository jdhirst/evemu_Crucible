/**
 * @name MissionLoader.cpp
 *   JSON-based mission data loader implementation
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#include "missions/MissionLoader.h"
#include "missions/MissionDataMapper.h"
#include "log/logtypes.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

MissionLoader::MissionLoader()
{
}

MissionLoader::~MissionLoader()
{
    Clear();
}

void MissionLoader::Clear()
{
    m_missions.clear();
    m_missionsByType.clear();
    m_missionsByDifficulty.clear();
    m_missionsByFaction.clear();
}

bool MissionLoader::LoadFromFile(const std::string& filepath)
{
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            _log(MISSION__ERROR, "Failed to open mission file: %s", filepath.c_str());
            return false;
        }

        json j;
        try {
            file >> j;
        } catch (const json::parse_error& e) {
            _log(MISSION__ERROR, "JSON parse error in %s: %s", filepath.c_str(), e.what());
            return false;
        }

        // Check if it's an array of missions or a single mission
        if (j.is_array()) {
            // Load multiple missions from array
            for (const auto& missionJson : j) {
                MissionNew::MissionData mission;
                if (MissionDataMapper::MapFromJson(missionJson, mission)) {
                    mission.jsonPath = filepath;
                    AddMission(mission);
                } else {
                    _log(MISSION__WARNING, "Failed to load mission from array in %s", filepath.c_str());
                }
            }
        } else if (j.is_object()) {
            // Load single mission
            MissionNew::MissionData mission;
            if (MissionDataMapper::MapFromJson(j, mission)) {
                mission.jsonPath = filepath;
                AddMission(mission);
            } else {
                _log(MISSION__ERROR, "Failed to load mission from %s", filepath.c_str());
                return false;
            }
        } else {
            _log(MISSION__ERROR, "Invalid JSON structure in %s", filepath.c_str());
            return false;
        }

        _log(MISSION__INFO, "Loaded mission file: %s (%lu missions)", filepath.c_str(), m_missions.size());
        return true;

    } catch (const std::exception& e) {
        _log(MISSION__ERROR, "Exception loading mission file %s: %s", filepath.c_str(), e.what());
        return false;
    }
}

bool MissionLoader::LoadFromDirectory(const std::string& dirPath)
{
    try {
        if (!fs::exists(dirPath)) {
            _log(MISSION__ERROR, "Mission directory does not exist: %s", dirPath.c_str());
            return false;
        }

        if (!fs::is_directory(dirPath)) {
            _log(MISSION__ERROR, "Path is not a directory: %s", dirPath.c_str());
            return false;
        }

        size_t loadedCount = 0;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".json") {
                    std::string filename = entry.path().filename().string();
                    // Skip manifest file
                    if (filename == "manifest.json") {
                        continue;
                    }
                    
                    if (LoadFromFile(entry.path().string())) {
                        loadedCount++;
                    }
                }
            }
        }

        _log(MISSION__INFO, "Loaded %lu mission files from %s", loadedCount, dirPath.c_str());
        return loadedCount > 0;

    } catch (const std::exception& e) {
        _log(MISSION__ERROR, "Exception loading missions from directory %s: %s", dirPath.c_str(), e.what());
        return false;
    }
}

void MissionLoader::AddMission(const MissionNew::MissionData& mission)
{
    // Store mission by ID
    m_missions[mission.id] = mission;

    // Index by type
    m_missionsByType[mission.missionType].push_back(mission.id);

    // Index by difficulty
    m_missionsByDifficulty[mission.difficulty].push_back(mission.id);

    // Index by faction
    m_missionsByFaction[mission.factionID].push_back(mission.id);

    _log(MISSION__TRACE, "Added mission %u: %s (Type: %u, Difficulty: %u, Faction: %u)",
        mission.id, mission.name.c_str(), mission.missionType, mission.difficulty, mission.factionID);
}

bool MissionLoader::GetMission(uint32 missionID, MissionNew::MissionData& outMission) const
{
    auto it = m_missions.find(missionID);
    if (it != m_missions.end()) {
        outMission = it->second;
        return true;
    }
    return false;
}

void MissionLoader::GetMissionsByType(uint8 type, std::vector<uint32>& outMissionIDs) const
{
    auto it = m_missionsByType.find(type);
    if (it != m_missionsByType.end()) {
        outMissionIDs = it->second;
    }
}

void MissionLoader::GetMissionsByDifficulty(uint8 difficulty, std::vector<uint32>& outMissionIDs) const
{
    auto it = m_missionsByDifficulty.find(difficulty);
    if (it != m_missionsByDifficulty.end()) {
        outMissionIDs = it->second;
    }
}

void MissionLoader::GetMissionsByFaction(uint32 factionID, std::vector<uint32>& outMissionIDs) const
{
    auto it = m_missionsByFaction.find(factionID);
    if (it != m_missionsByFaction.end()) {
        outMissionIDs = it->second;
    }
}

const std::map<uint32, MissionNew::MissionData>& MissionLoader::GetMissions() const
{
    return m_missions;
}

size_t MissionLoader::GetMissionCount() const
{
    return m_missions.size();
}
