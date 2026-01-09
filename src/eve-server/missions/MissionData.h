/**
 * @name MissionData.h
 *   Data structures for JSON-based mission system
 *
 * @Author:        Generated for mission system refactor
 * @date:          January 2026
 */

#ifndef _EVE_SERVER_MISSION_DATA_H__
#define _EVE_SERVER_MISSION_DATA_H__

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace MissionNew {

// Mission complexity levels
namespace Complexity {
    enum Type : uint8 {
        Simple = 0,
        Standard = 1,
        MultiStage = 2,
        Epic = 3
    };
}

// Mission types matching JSON data  
namespace MissionType {
    enum {
        Combat = 0,
        Courier = 1,
        Mining = 2,
        Reconnaissance = 3,
        Retrieve = 4,
        Hack = 5,
        Defend = 6,
        Trade = 7,
        Research = 8,
        Storyline = 9
    };
}

// Mission tones
namespace Tone {
    enum Type : uint8 {
        Professional = 0,
        Comedic = 1,
        Dramatic = 2
    };
}

// Mission space types
namespace SpaceType {
    enum Type : uint8 {
        Deadspace = 0,
        NormalSpace = 1,
        Station = 2,
        AsteroidBelt = 3
    };
}

// Stage types
namespace StageType {
    enum Type : uint8 {
        AgentDialog = 0,
        WarpToLocation = 1,
        Combat = 2,
        CourierDelivery = 3,
        PopupMessage = 4,
        Mining = 5,
        Reconnaissance = 6,
        Retrieve = 7,
        Hack = 8,
        Defend = 9
    };
}

// Encounter triggers
namespace TriggerType {
    enum Type : uint8 {
        Proximity = 0,
        Timer = 1,
        ObjectiveComplete = 2,
        Manual = 3,
        PreviousWaveCleared = 4
    };
}

// Ship behaviors
namespace Behavior {
    enum Type : uint8 {
        Aggressive = 0,
        Passive = 1,
        Defensive = 2
    };
}

// Mission state
namespace Status {
    enum {
        Offered = 0,
        Accepted = 1,
        InProgress = 2,
        Completed = 3,
        Failed = 4,
        Aborted = 5
    };
}

// Location structure
struct Location {
    std::string system;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    float security;
    std::string station;
    uint32 stationID;
};

// Agent information
struct AgentInfo {
    std::string name;
    uint32 agentID;
    std::string corporation;
    uint32 corporationID;
    Location location;
};

// Mission space information
struct MissionSpace {
    std::string system;
    uint32 systemID;
    SpaceType::Type type;
    std::string description;
    bool requiresWarp;
};

// Completion criteria
struct CompletionCriteria {
    uint32 enemyShipsDestroyed;
    uint32 itemsCollected;
    std::string itemType;
    uint32 typeID;
    std::string destinationSystem;
    uint32 destinationSystemID;
    std::string destinationStation;
    uint32 destinationStationID;
    uint32 timeLimit; // seconds
};

// Objective structure
struct Objective {
    uint32 id;
    uint8 type; // Mission::Type
    std::string description;
    bool required;
    CompletionCriteria criteria;
};

// Ship in encounter
struct EncounterShip {
    std::string type;
    uint32 typeID;
    std::string shipClass;
    uint32 count;
    Behavior::Type behavior;
    std::string orbitRange;
};

// Container contents
struct ContainerContent {
    uint32 typeID;
    uint32 quantity;
    bool isObjective;
    std::string name; // For reference
};

// Static object in encounter
struct StaticObject {
    std::string type;
    uint32 typeID;
    std::string name;
    std::vector<ContainerContent> contents;
};

// Encounter wave
struct Encounter {
    uint32 wave;
    TriggerType::Type trigger;
    std::string distance;
    std::vector<EncounterShip> ships;
    std::vector<StaticObject> staticObjects;
};

// Courier package details
struct CourierPackage {
    uint32 typeID;
    std::string name;
    float volume;
    uint32 quantity;
};

// Route information
struct Route {
    uint32 jumps;
    float distanceLY;
    std::string securityStatus;
};

// Courier mission details
struct CourierDetails {
    Location pickupLocation;
    Location dropoffLocation;
    CourierPackage package;
    Route route;
    uint32 timeLimit; // seconds
    uint32 collateral; // ISK
};

// ISK rewards
struct ISKReward {
    uint32 base;
    uint32 bonus;
    uint32 totalPossible;
};

// Loyalty points
struct LoyaltyPoints {
    uint32 base;
    std::string corporation;
    uint32 corporationID;
};

// Standings change
struct StandingsChange {
    std::string faction;
    uint32 factionID;
    float increase;
};

// Item reward
struct ItemReward {
    uint32 typeID;
    uint32 quantity;
    std::string name;
};

// Rewards structure
struct Rewards {
    ISKReward isk;
    LoyaltyPoints loyaltyPoints;
    StandingsChange standings;
    std::vector<ItemReward> items;
    bool salvage;
    uint32 lootValueEstimate;
};

// Damage/Tank types
struct DamageInfo {
    std::string primary;
    std::string secondary;
};

// Requirements
struct Requirements {
    std::string minShipClass;
    std::string recommendedShipClass;
    DamageInfo damageType;
    DamageInfo tankType;
};

// Narrative
struct Narrative {
    std::string briefing;
    std::string objectives;
    std::string completion;
};

// Mission flow stage
struct MissionStage {
    uint32 stage;
    StageType::Type type;
    std::string location;
    std::string agent;
    std::string description;
    std::string action;
    std::string messageType;
    bool isCompletion;
};

// Mission flow
struct MissionFlow {
    std::vector<MissionStage> stages;
    uint32 totalStages;
    bool requiresAgentReturn;
    Complexity::Type complexity;
};

// Complete mission data structure
struct MissionData {
    // Metadata
    uint32 id;
    std::string name;
    std::string faction;
    uint32 factionID;
    uint8 missionType;
    uint8 difficulty;
    uint32 estimatedDuration; // minutes
    bool isEpic;
    bool isFunny;
    Complexity::Type complexity;
    Tone::Type tone;
    std::string version;
    std::string generatedAt;

    // Core data
    AgentInfo agent;
    MissionSpace missionSpace;
    std::vector<Objective> objectives;
    MissionFlow missionFlow;
    std::vector<Encounter> encounters;
    CourierDetails courierDetails;
    Rewards rewards;
    Requirements requirements;
    Narrative narrative;

    // Runtime data
    bool isLoaded;
    std::string jsonPath;
};

// Mission state tracking (DB backed)
struct MissionState {
    uint32 stateID;
    uint32 characterID;
    uint32 missionID;
    uint8 status;
    uint32 currentStage;
    std::vector<uint32> objectivesCompleted;
    int64 startedAt;
    int64 completedAt;
    int64 expiresAt;
    uint32 missionSpaceID; // Instance ID for deadspace
};

// Objective progress tracking
struct ObjectiveProgress {
    uint32 trackingID;
    uint32 stateID;
    uint32 objectiveID;
    uint32 progress;
    bool completed;
    int64 completedAt;
};

} // namespace MissionNew

#endif // _EVE_SERVER_MISSION_DATA_H__
