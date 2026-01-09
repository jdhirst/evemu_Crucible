/**
 * @name MissionDataMapper.cpp
 *   Implementation of mission data JSON<->C++ mapping
 *
 * @Author:        Mission System Refactor
 * @date:          January 2026
 */

#include "missions/MissionDataMapper.h"
#include "log/logtypes.h"

// Safe JSON value getter with default
template<typename T>
T MissionDataMapper::GetValue(const json& j, const std::string& key, const T& defaultValue)
{
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j[key].get<T>();
        } catch (const json::exception& e) {
            _log(MISSION__WARNING, "Failed to get value for '%s': %s", key.c_str(), e.what());
            return defaultValue;
        }
    }
    return defaultValue;
}

std::string MissionDataMapper::GetString(const json& j, const std::string& key, const std::string& def)
{
    return GetValue<std::string>(j, key, def);
}

uint32 MissionDataMapper::GetUInt32(const json& j, const std::string& key, uint32 def)
{
    return GetValue<uint32>(j, key, def);
}

uint8 MissionDataMapper::GetUInt8(const json& j, const std::string& key, uint8 def)
{
    return GetValue<uint8>(j, key, def);
}

float MissionDataMapper::GetFloat(const json& j, const std::string& key, float def)
{
    return GetValue<float>(j, key, def);
}

bool MissionDataMapper::GetBool(const json& j, const std::string& key, bool def)
{
    return GetValue<bool>(j, key, def);
}

// Type conversion helpers
uint8 MissionDataMapper::GetMissionType(const std::string& typeStr)
{
    if (typeStr == "combat" || typeStr == "0") return MissionNew::MissionType::Combat;
    if (typeStr == "courier" || typeStr == "1") return MissionNew::MissionType::Courier;
    if (typeStr == "mining" || typeStr == "2") return MissionNew::MissionType::Mining;
    if (typeStr == "reconnaissance" || typeStr == "3") return MissionNew::MissionType::Reconnaissance;
    if (typeStr == "retrieval" || typeStr == "4") return MissionNew::MissionType::Retrieve;
    if (typeStr == "hack" || typeStr == "5") return MissionNew::MissionType::Hack;
    return MissionNew::MissionType::Combat;
}

uint8 MissionDataMapper::GetDifficulty(int diff)
{
    return static_cast<uint8>(diff >= 1 && diff <= 5 ? diff : 1);
}

MissionNew::Complexity::Type MissionDataMapper::GetComplexity(const std::string& complexStr)
{
    if (complexStr == "simple" || complexStr == "0") return MissionNew::Complexity::Simple;
    if (complexStr == "standard" || complexStr == "1") return MissionNew::Complexity::Standard;
    if (complexStr == "multi_stage" || complexStr == "2") return MissionNew::Complexity::MultiStage;
    if (complexStr == "epic" || complexStr == "3") return MissionNew::Complexity::Epic;
    return MissionNew::Complexity::Standard;
}

MissionNew::Tone::Type MissionDataMapper::GetTone(const std::string& toneStr)
{
    if (toneStr == "professional" || toneStr == "0") return MissionNew::Tone::Professional;
    if (toneStr == "comedic" || toneStr == "1") return MissionNew::Tone::Comedic;
    if (toneStr == "dramatic" || toneStr == "2") return MissionNew::Tone::Dramatic;
    return MissionNew::Tone::Professional;
}

MissionNew::SpaceType::Type MissionDataMapper::GetSpaceType(int type)
{
    switch (type) {
        case 0: return MissionNew::SpaceType::Deadspace;
        case 1: return MissionNew::SpaceType::NormalSpace;
        case 2: return MissionNew::SpaceType::Station;
        case 3: return MissionNew::SpaceType::AsteroidBelt;
        default: return MissionNew::SpaceType::NormalSpace;
    }
}

MissionNew::StageType::Type MissionDataMapper::GetStageType(int type)
{
    switch (type) {
        case 0: return MissionNew::StageType::AgentDialog;
        case 1: return MissionNew::StageType::WarpToLocation;
        case 2: return MissionNew::StageType::Combat;
        case 3: return MissionNew::StageType::CourierDelivery;
        case 4: return MissionNew::StageType::PopupMessage;
        case 5: return MissionNew::StageType::Mining;
        case 6: return MissionNew::StageType::Reconnaissance;
        case 7: return MissionNew::StageType::Retrieve;
        case 8: return MissionNew::StageType::Hack;
        case 9: return MissionNew::StageType::Defend;
        default: return MissionNew::StageType::AgentDialog;
    }
}

MissionNew::TriggerType::Type MissionDataMapper::GetTriggerType(const std::string& trigger)
{
    if (trigger == "proximity") return MissionNew::TriggerType::Proximity;
    if (trigger == "timer") return MissionNew::TriggerType::Timer;
    if (trigger == "objective_complete") return MissionNew::TriggerType::ObjectiveComplete;
    if (trigger == "manual") return MissionNew::TriggerType::Manual;
    if (trigger == "previous_wave_cleared") return MissionNew::TriggerType::PreviousWaveCleared;
    return MissionNew::TriggerType::Proximity;
}

MissionNew::Behavior::Type MissionDataMapper::GetBehavior(const std::string& behavior)
{
    if (behavior == "aggressive") return MissionNew::Behavior::Aggressive;
    if (behavior == "passive") return MissionNew::Behavior::Passive;
    if (behavior == "defensive") return MissionNew::Behavior::Defensive;
    return MissionNew::Behavior::Aggressive;
}

// Map Location
void MissionDataMapper::MapLocation(const json& j, MissionNew::Location& loc)
{
    loc.system = GetString(j, "system");
    loc.systemID = GetUInt32(j, "systemID");
    loc.constellationID = GetUInt32(j, "constellationID");
    loc.regionID = GetUInt32(j, "regionID");
    loc.security = GetFloat(j, "security");
    loc.station = GetString(j, "station");
    loc.stationID = GetUInt32(j, "stationID");
}

// Map AgentInfo
void MissionDataMapper::MapAgentInfo(const json& j, MissionNew::AgentInfo& agent)
{
    agent.name = GetString(j, "name");
    agent.agentID = GetUInt32(j, "agentID");
    agent.corporation = GetString(j, "corporation");
    agent.corporationID = GetUInt32(j, "corporationID");
    
    if (j.contains("location") && j["location"].is_object()) {
        MapLocation(j["location"], agent.location);
    }
}

// Map MissionSpace
void MissionDataMapper::MapMissionSpace(const json& j, MissionNew::MissionSpace& space)
{
    space.system = GetString(j, "system");
    space.systemID = GetUInt32(j, "systemID");
    space.type = GetSpaceType(GetUInt8(j, "type"));
    space.description = GetString(j, "description");
    space.requiresWarp = GetBool(j, "requiresWarp");
}

// Map Objectives
void MissionDataMapper::MapObjectives(const json& j, std::vector<MissionNew::Objective>& objectives)
{
    if (!j.is_array()) return;
    
    for (const auto& objJson : j) {
        MissionNew::Objective obj;
        obj.id = GetUInt32(objJson, "id");
        obj.type = GetUInt8(objJson, "type");
        obj.description = GetString(objJson, "description");
        obj.required = GetBool(objJson, "required", true);
        
        // Map completion criteria
        if (objJson.contains("criteria") && objJson["criteria"].is_object()) {
            const auto& crit = objJson["criteria"];
            obj.criteria.enemyShipsDestroyed = GetUInt32(crit, "ships_destroyed");
            obj.criteria.itemsCollected = GetUInt32(crit, "items_collected");
            obj.criteria.itemType = GetString(crit, "itemType");
            obj.criteria.typeID = GetUInt32(crit, "typeID");
            obj.criteria.destinationSystem = GetString(crit, "destinationSystem");
            obj.criteria.destinationSystemID = GetUInt32(crit, "destinationSystemID");
            obj.criteria.destinationStation = GetString(crit, "destinationStation");
            obj.criteria.destinationStationID = GetUInt32(crit, "destinationStationID");
            obj.criteria.timeLimit = GetUInt32(crit, "timeLimit");
        }
        
        objectives.push_back(obj);
    }
}

// Map MissionFlow
void MissionDataMapper::MapMissionFlow(const json& j, MissionNew::MissionFlow& flow)
{
    if (j.contains("stages") && j["stages"].is_array()) {
        for (const auto& stageJson : j["stages"]) {
            MissionNew::MissionStage stage;
            stage.stage = GetUInt32(stageJson, "stage");
            stage.type = GetStageType(GetUInt8(stageJson, "type"));
            stage.location = GetString(stageJson, "location");
            stage.agent = GetString(stageJson, "agent");
            stage.description = GetString(stageJson, "description");
            stage.action = GetString(stageJson, "action");
            stage.messageType = GetString(stageJson, "messageType");
            stage.isCompletion = GetBool(stageJson, "isCompletion");
            
            flow.stages.push_back(stage);
        }
    }
    
    flow.totalStages = GetUInt32(j, "totalStages");
    flow.requiresAgentReturn = GetBool(j, "requiresAgentReturn");
    
    std::string complexStr = GetString(j, "complexity");
    flow.complexity = GetComplexity(complexStr);
}

// Map Encounters
void MissionDataMapper::MapEncounters(const json& j, std::vector<MissionNew::Encounter>& encounters)
{
    if (!j.is_array()) return;
    
    for (const auto& encJson : j) {
        MissionNew::Encounter enc;
        enc.wave = GetUInt32(encJson, "wave");
        
        std::string triggerStr = GetString(encJson, "trigger");
        enc.trigger = GetTriggerType(triggerStr);
        enc.distance = GetString(encJson, "distance");
        
        // Map ships
        if (encJson.contains("ships") && encJson["ships"].is_array()) {
            for (const auto& shipJson : encJson["ships"]) {
                MissionNew::EncounterShip ship;
                ship.type = GetString(shipJson, "type");
                ship.typeID = GetUInt32(shipJson, "type_id");
                ship.shipClass = GetString(shipJson, "class");
                ship.count = GetUInt32(shipJson, "count");
                
                std::string behaviorStr = GetString(shipJson, "behavior");
                ship.behavior = GetBehavior(behaviorStr);
                ship.orbitRange = GetString(shipJson, "orbit_range");
                
                enc.ships.push_back(ship);
            }
        }
        
        // Map static objects
        if (encJson.contains("staticObjects") && encJson["staticObjects"].is_array()) {
            for (const auto& objJson : encJson["staticObjects"]) {
                MissionNew::StaticObject obj;
                obj.type = GetString(objJson, "type");
                obj.typeID = GetUInt32(objJson, "typeID");
                obj.name = GetString(objJson, "name");
                
                // Map container contents if present
                if (objJson.contains("contents") && objJson["contents"].is_array()) {
                    for (const auto& contentJson : objJson["contents"]) {
                        MissionNew::ContainerContent content;
                        content.typeID = GetUInt32(contentJson, "typeID");
                        content.quantity = GetUInt32(contentJson, "quantity");
                        content.isObjective = GetBool(contentJson, "isObjective");
                        content.name = GetString(contentJson, "name");
                        obj.contents.push_back(content);
                    }
                }
                
                enc.staticObjects.push_back(obj);
            }
        }
        
        encounters.push_back(enc);
    }
}

// Map CourierDetails
void MissionDataMapper::MapCourierDetails(const json& j, MissionNew::CourierDetails& details)
{
    if (j.contains("pickupLocation") && j["pickupLocation"].is_object()) {
        MapLocation(j["pickupLocation"], details.pickupLocation);
    }
    
    if (j.contains("dropoffLocation") && j["dropoffLocation"].is_object()) {
        MapLocation(j["dropoffLocation"], details.dropoffLocation);
    }
    
    if (j.contains("package") && j["package"].is_object()) {
        const auto& pkg = j["package"];
        details.package.typeID = GetUInt32(pkg, "typeID");
        details.package.name = GetString(pkg, "name");
        details.package.volume = GetFloat(pkg, "volume");
        details.package.quantity = GetUInt32(pkg, "quantity");
    }
    
    if (j.contains("route") && j["route"].is_object()) {
        const auto& route = j["route"];
        details.route.jumps = GetUInt32(route, "jumps");
        details.route.distanceLY = GetFloat(route, "distanceLY");
        details.route.securityStatus = GetString(route, "securityStatus");
    }
    
    details.timeLimit = GetUInt32(j, "timeLimit");
    details.collateral = GetUInt32(j, "collateral");
}

// Map Rewards
void MissionDataMapper::MapRewards(const json& j, MissionNew::Rewards& rewards)
{
    if (j.contains("isk") && j["isk"].is_object()) {
        const auto& isk = j["isk"];
        rewards.isk.base = GetUInt32(isk, "base");
        rewards.isk.bonus = GetUInt32(isk, "bonus");
        rewards.isk.totalPossible = GetUInt32(isk, "totalPossible");
    }
    
    if (j.contains("loyaltyPoints") && j["loyaltyPoints"].is_object()) {
        const auto& lp = j["loyaltyPoints"];
        rewards.loyaltyPoints.base = GetUInt32(lp, "base");
        rewards.loyaltyPoints.corporation = GetString(lp, "corporation");
        rewards.loyaltyPoints.corporationID = GetUInt32(lp, "corporationID");
    }
    
    if (j.contains("standings") && j["standings"].is_object()) {
        const auto& standings = j["standings"];
        rewards.standings.faction = GetString(standings, "faction");
        rewards.standings.factionID = GetUInt32(standings, "factionID");
        rewards.standings.increase = GetFloat(standings, "increase");
    }
    
    if (j.contains("items") && j["items"].is_array()) {
        for (const auto& itemJson : j["items"]) {
            MissionNew::ItemReward item;
            item.typeID = GetUInt32(itemJson, "typeID");
            item.quantity = GetUInt32(itemJson, "quantity");
            item.name = GetString(itemJson, "name");
            rewards.items.push_back(item);
        }
    }
    
    rewards.salvage = GetBool(j, "salvage");
    rewards.lootValueEstimate = GetUInt32(j, "lootValueEstimate");
}

// Map Requirements
void MissionDataMapper::MapRequirements(const json& j, MissionNew::Requirements& reqs)
{
    reqs.minShipClass = GetString(j, "minShipClass");
    reqs.recommendedShipClass = GetString(j, "recommendedShipClass");
    
    if (j.contains("damageType") && j["damageType"].is_object()) {
        const auto& dmg = j["damageType"];
        reqs.damageType.primary = GetString(dmg, "primary");
        reqs.damageType.secondary = GetString(dmg, "secondary");
    }
    
    if (j.contains("tankType") && j["tankType"].is_object()) {
        const auto& tank = j["tankType"];
        reqs.tankType.primary = GetString(tank, "primary");
        reqs.tankType.secondary = GetString(tank, "secondary");
    }
}

// Map Narrative
void MissionDataMapper::MapNarrative(const json& j, MissionNew::Narrative& narrative)
{
    narrative.briefing = GetString(j, "briefing");
    narrative.objectives = GetString(j, "objectives");
    narrative.completion = GetString(j, "completion");
}

// Main mapping function
bool MissionDataMapper::MapFromJson(const json& j, MissionNew::MissionData& data)
{
    try {
        // Basic metadata
        data.id = GetUInt32(j, "missionID");
        data.name = GetString(j, "name");
        data.faction = GetString(j, "faction");
        data.factionID = GetUInt32(j, "factionID");
        data.missionType = GetUInt8(j, "missionType");
        data.difficulty = GetDifficulty(GetUInt8(j, "difficulty"));
        data.estimatedDuration = GetUInt32(j, "estimatedDuration");
        data.isEpic = GetBool(j, "isEpic");
        data.isFunny = GetBool(j, "isFunny");
        
        // Handle complexity - can be int or string
        if (j.contains("complexity")) {
            if (j["complexity"].is_string()) {
                data.complexity = GetComplexity(j["complexity"].get<std::string>());
            } else {
                data.complexity = static_cast<MissionNew::Complexity::Type>(GetUInt8(j, "complexity"));
            }
        }
        
        // Handle tone - can be int or string
        if (j.contains("tone")) {
            if (j["tone"].is_string()) {
                data.tone = GetTone(j["tone"].get<std::string>());
            } else {
                data.tone = static_cast<MissionNew::Tone::Type>(GetUInt8(j, "tone"));
            }
        }
        
        data.version = GetString(j, "version");
        data.generatedAt = GetString(j, "generatedAt");
        
        // Complex nested structures
        if (j.contains("agent") && j["agent"].is_object()) {
            MapAgentInfo(j["agent"], data.agent);
        }
        
        if (j.contains("missionSpace") && j["missionSpace"].is_object()) {
            MapMissionSpace(j["missionSpace"], data.missionSpace);
        }
        
        if (j.contains("objectives") && j["objectives"].is_array()) {
            MapObjectives(j["objectives"], data.objectives);
        }
        
        if (j.contains("missionFlow") && j["missionFlow"].is_object()) {
            MapMissionFlow(j["missionFlow"], data.missionFlow);
        }
        
        if (j.contains("encounters") && j["encounters"].is_array()) {
            MapEncounters(j["encounters"], data.encounters);
        }
        
        if (j.contains("courierDetails") && !j["courierDetails"].is_null() && j["courierDetails"].is_object()) {
            MapCourierDetails(j["courierDetails"], data.courierDetails);
        }
        
        if (j.contains("rewards") && j["rewards"].is_object()) {
            MapRewards(j["rewards"], data.rewards);
        }
        
        if (j.contains("requirements") && j["requirements"].is_object()) {
            MapRequirements(j["requirements"], data.requirements);
        }
        
        if (j.contains("narrative") && j["narrative"].is_object()) {
            MapNarrative(j["narrative"], data.narrative);
        }
        
        data.isLoaded = true;
        return true;
        
    } catch (const json::exception& e) {
        _log(MISSION__ERROR, "Failed to map mission from JSON: %s", e.what());
        return false;
    } catch (const std::exception& e) {
        _log(MISSION__ERROR, "Unexpected error mapping mission: %s", e.what());
        return false;
    }
}

// Placeholder for reverse mapping (if needed later)
json MissionDataMapper::MapToJson(const MissionNew::MissionData& data)
{
    json j;
    
    // Basic fields
    j["missionID"] = data.id;
    j["name"] = data.name;
    j["faction"] = data.faction;
    j["factionID"] = data.factionID;
    j["missionType"] = data.missionType;
    j["difficulty"] = data.difficulty;
    j["estimatedDuration"] = data.estimatedDuration;
    j["isEpic"] = data.isEpic;
    j["isFunny"] = data.isFunny;
    j["complexity"] = data.complexity;
    j["tone"] = data.tone;
    j["version"] = data.version;
    j["generatedAt"] = data.generatedAt;
    
    // Agent info
    j["agent"] = {
        {"name", data.agent.name},
        {"agentID", data.agent.agentID},
        {"corporation", data.agent.corporation},
        {"corporationID", data.agent.corporationID}
    };
    
    // More fields can be added as needed
    
    return j;
}
