/**
 * @name MissionDataMapper.h
 *   Mapper between JSON snake_case and C++ camelCase mission data structures
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#ifndef _EVE_SERVER_MISSION_DATA_MAPPER_H__
#define _EVE_SERVER_MISSION_DATA_MAPPER_H__

#include "missions/MissionData.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Maps between JSON representation and C++ MissionData structures
 * 
 * This class handles the conversion between snake_case JSON fields
 * and camelCase C++ structure members.
 */
class MissionDataMapper
{
public:
    /**
     * @brief Map JSON object to MissionData structure
     */
    static bool MapFromJson(const json& j, MissionNew::MissionData& data);

    /**
     * @brief Map MissionData structure to JSON object
     */
    static json MapToJson(const MissionNew::MissionData& data);

private:
    // Helper methods for nested structures
    static void MapLocation(const json& j, MissionNew::Location& loc);
    static void MapAgentInfo(const json& j, MissionNew::AgentInfo& agent);
    static void MapMissionSpace(const json& j, MissionNew::MissionSpace& space);
    static void MapObjectives(const json& j, std::vector<MissionNew::Objective>& objectives);
    static void MapMissionFlow(const json& j, MissionNew::MissionFlow& flow);
    static void MapEncounters(const json& j, std::vector<MissionNew::Encounter>& encounters);
    static void MapCourierDetails(const json& j, MissionNew::CourierDetails& details);
    static void MapRewards(const json& j, MissionNew::Rewards& rewards);
    static void MapRequirements(const json& j, MissionNew::Requirements& reqs);
    static void MapNarrative(const json& j, MissionNew::Narrative& narrative);

    // Conversion helpers
    static uint8 GetMissionType(const std::string& typeStr);
    static uint8 GetDifficulty(int diff);
    static MissionNew::Complexity::Type GetComplexity(const std::string& complexStr);
    static MissionNew::Tone::Type GetTone(const std::string& toneStr);
    static MissionNew::SpaceType::Type GetSpaceType(int type);
    static MissionNew::StageType::Type GetStageType(int type);
    static MissionNew::TriggerType::Type GetTriggerType(const std::string& trigger);
    static MissionNew::Behavior::Type GetBehavior(const std::string& behavior);

    // Safe JSON access with defaults
    template<typename T>
    static T GetValue(const json& j, const std::string& key, const T& defaultValue);
    
    static std::string GetString(const json& j, const std::string& key, const std::string& def = "");
    static uint32 GetUInt32(const json& j, const std::string& key, uint32 def = 0);
    static uint8 GetUInt8(const json& j, const std::string& key, uint8 def = 0);
    static float GetFloat(const json& j, const std::string& key, float def = 0.0f);
    static bool GetBool(const json& j, const std::string& key, bool def = false);
};

#endif // _EVE_SERVER_MISSION_DATA_MAPPER_H__
