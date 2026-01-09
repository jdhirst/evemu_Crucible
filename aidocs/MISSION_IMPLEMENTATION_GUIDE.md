# Mission System Complete Implementation Guide

## Overview
This guide provides step-by-step instructions for completing the JSON-based mission system refactor for EVEmu.

## Files Created ?

1. **src/eve-server/missions/MissionData.h** - Data structures
2. **src/eve-server/missions/MissionLoader.h** - JSON loader interface
3. **src/eve-server/missions/MissionLoader.cpp** - JSON loader implementation
4. **src/eve-server/missions/NewMissionDataMgr.h** - State manager interface
5. **src/eve-server/missions/NewMissionDataMgr.cpp** - State manager implementation
6. **src/eve-server/missions/MissionDB.h** - Updated with new methods
7. **sql/migrations/20260109_mission_system_refactor.sql** - Database schema
8. **src/eve-server/CMakeLists.txt** - Updated with nlohmann/json

## Remaining Implementation Tasks

### Priority 1: Database Implementation

#### File: `src/eve-server/missions/MissionDB.cpp`

Add implementations for new methods declared in MissionDB.h:

```cpp
bool MissionDB::SaveMissionState(const Mission::MissionState& state)
{
    DBerror err;
    
    std::string query = "INSERT INTO missionStates "
        "(stateID, characterID, missionID, agentID, status, currentStage, "
        "startedAt, completedAt, expiresAt, missionSpaceID) "
        "VALUES (%u, %u, %u, %u, %u, %u, %li, %li, %li, %u) "
        "ON DUPLICATE KEY UPDATE "
        "status=VALUES(status), currentStage=VALUES(currentStage), "
        "completedAt=VALUES(completedAt), missionSpaceID=VALUES(missionSpaceID)";
    
    if (!sDatabase.RunQuery(err, query.c_str(),
        state.stateID, state.characterID, state.missionID, state.agentID,
        state.status, state.currentStage, state.startedAt, state.completedAt,
        state.expiresAt, state.missionSpaceID))
    {
        codelog(DATABASE__ERROR, "Failed to save mission state %u: %s", 
                state.stateID, err.c_str());
        return false;
    }
    
    return true;
}

bool MissionDB::LoadMissionState(uint32 stateID, Mission::MissionState& state)
{
    DBQueryResult res;
    
    if (!sDatabase.RunQuery(res,
        "SELECT stateID, characterID, missionID, agentID, status, currentStage, "
        "startedAt, completedAt, expiresAt, missionSpaceID "
        "FROM missionStates WHERE stateID=%u", stateID))
    {
        codelog(DATABASE__ERROR, "Failed to load mission state %u", stateID);
        return false;
    }
    
    DBResultRow row;
    if (!res.GetRow(row)) {
        return false;
    }
    
    state.stateID = row.GetUInt(0);
    state.characterID = row.GetUInt(1);
    state.missionID = row.GetUInt(2);
    // ... parse remaining fields
    
    return true;
}

// Implement remaining methods...
```

### Priority 2: Mission Space Manager

#### File: `src/eve-server/missions/MissionSpaceMgr.h`

```cpp
#ifndef _EVE_SERVER_MISSION_SPACE_MGR_H__
#define _EVE_SERVER_MISSION_SPACE_MGR_H__

#include "eve-server.h"
#include "missions/MissionData.h"

class SystemBubble;
class SystemManager;

class MissionSpaceMgr
{
public:
    /**
     * @brief Create a mission space instance
     * @param mission The mission definition
     * @param stateID The mission state ID
     * @param systemID The solar system to create space in
     * @return The space ID or 0 on failure
     */
    static uint32 CreateMissionSpace(const Mission::MissionData& mission,
                                     uint32 stateID,
                                     uint32 systemID);
    
    /**
     * @brief Spawn encounters in mission space
     * @param encounters The encounter definitions
     * @param spaceID The mission space ID
     * @param bubble The system bubble for spawning
     */
    static void SpawnEncounters(const std::vector<Mission::Encounter>& encounters,
                               uint32 spaceID,
                               SystemBubble* bubble);
    
    /**
     * @brief Spawn static objects in mission space
     * @param objects The static object definitions
     * @param spaceID The mission space ID
     * @param bubble The system bubble for spawning
     */
    static void SpawnStaticObjects(const std::vector<Mission::StaticObject>& objects,
                                   uint32 spaceID,
                                   SystemBubble* bubble);
    
    /**
     * @brief Cleanup mission space
     * @param spaceID The mission space ID
     */
    static void CleanupMissionSpace(uint32 spaceID);
    
    /**
     * @brief Get mission space coordinates
     */
    static GPoint GenerateSpaceCoordinates(uint32 systemID);
};

#endif
```

### Priority 3: Agent Integration

#### File: `src/eve-server/agents/Agent.cpp`

Update the `MakeOffer` method:

```cpp
void Agent::MakeOffer(Client* pClient)
{
    if (pClient == nullptr) return;
    
    // Get random mission from new system
    Mission::MissionData mission;
    if (!sNewMissionDataMgr.GetRandomMission(
        m_agentData.level,
        Mission::Type::Combat,  // TODO: Get from agent type
        m_agentData.factionID,
        mission))
    {
        _log(AGENT__ERROR, "No missions available for agent %u level %u", 
             m_agentID, m_agentData.level);
        // Fall back to old system or generate error
        return;
    }
    
    // Create mission state
    uint32 stateID = sNewMissionDataMgr.CreateMissionState(
        pClient->GetCharacterID(),
        mission.id,
        m_agentID
    );
    
    if (stateID == 0) {
        _log(AGENT__ERROR, "Failed to create mission state");
        return;
    }
    
    // Send mission offer to client
    SendMissionOffer(pClient, mission, stateID);
}

void Agent::SendMissionOffer(Client* pClient, 
                             const Mission::MissionData& mission,
                             uint32 stateID)
{
    // Build PyDict with mission data for client
    PyDict* offer = new PyDict();
    offer->SetItemString("missionID", new PyInt(mission.id));
    offer->SetItemString("name", new PyString(mission.name));
    offer->SetItemString("type", new PyInt(mission.missionType));
    offer->SetItemString("stateID", new PyInt(stateID));
    
    // Add rewards
    PyDict* rewards = new PyDict();
    rewards->SetItemString("isk", new PyInt(mission.rewards.isk.base));
    rewards->SetItemString("bonus", new PyInt(mission.rewards.isk.bonus));
    rewards->SetItemString("lp", new PyInt(mission.rewards.loyaltyPoints.base));
    offer->SetItemString("rewards", rewards);
    
    // Add briefing
    offer->SetItemString("briefing", new PyString(mission.narrative.briefing));
    
    // Send to client
    pClient->SendNotification("OnAgentMissionOffered", "charid", offer);
}
```

### Priority 4: Objective Tracking

#### File: `src/eve-server/missions/ObjectiveTracker.h`

```cpp
#ifndef _EVE_SERVER_OBJECTIVE_TRACKER_H__
#define _EVE_SERVER_OBJECTIVE_TRACKER_H__

#include "eve-server.h"
#include "missions/MissionData.h"

class Client;
class InventoryItem;
class NPC;

class ObjectiveTracker
{
public:
    /**
     * @brief Track NPC kill for mission objectives
     */
    static void OnNPCKilled(Client* pClient, NPC* pNPC);
    
    /**
     * @brief Track item collection
     */
    static void OnItemCollected(Client* pClient, InventoryItem* pItem);
    
    /**
     * @brief Track location visit
     */
    static void OnLocationVisited(Client* pClient, uint32 locationID);
    
    /**
     * @brief Track mining
     */
    static void OnOreMined(Client* pClient, uint16 oreTypeID, uint32 quantity);
    
    /**
     * @brief Check all objectives for a mission
     */
    static void CheckMissionObjectives(uint32 stateID);
    
private:
    static void UpdateKillObjective(uint32 stateID, uint32 objectiveID, 
                                    uint16 npcTypeID);
    static void UpdateCollectionObjective(uint32 stateID, uint32 objectiveID, 
                                         uint16 itemTypeID, uint32 quantity);
    static void UpdateLocationObjective(uint32 stateID, uint32 objectiveID);
    static void UpdateMiningObjective(uint32 stateID, uint32 objectiveID, 
                                     uint16 oreTypeID, uint32 quantity);
};

#endif
```

### Priority 5: Client Integration

#### File: `src/eve-server/Client.cpp`

Add mission tracking methods:

```cpp
void Client::OnNPCKilled(NPC* pNPC)
{
    if (pNPC == nullptr) return;
    
    // Track for mission objectives
    ObjectiveTracker::OnNPCKilled(this, pNPC);
    
    // ... existing bounty/kill code ...
}

void Client::OnItemAcquired(InventoryItem* pItem)
{
    if (pItem == nullptr) return;
    
    // Track for mission objectives
    ObjectiveTracker::OnItemCollected(this, pItem);
    
    // ... existing code ...
}

void Client::UpdatePosition(const GPoint& newPos)
{
    // ... existing code ...
    
    // Check for mission location objectives
    // ObjectiveTracker::OnLocationVisited(this, GetLocationID());
}
```

## Testing Plan

### Test 1: JSON Loading
```bash
# Start server and check logs for:
# "MissionLoader: Loaded X missions from high_quality_missions.json"
# "NewMissionDataMgr: Loaded X mission definitions"
```

### Test 2: Database Schema
```sql
-- Run migration
SOURCE sql/migrations/20260109_mission_system_refactor.sql;

-- Verify tables
SHOW TABLES LIKE 'mission%';
DESCRIBE missionStates;
```

### Test 3: Mission Offering
1. Log in with character
2. Talk to agent
3. Check for mission offer
4. Verify mission data displays
5. Check database for mission state entry

### Test 4: Mission Execution
1. Accept mission
2. Warp to mission location
3. Verify mission space creates
4. Verify encounters spawn
5. Kill rats/collect items
6. Verify objectives update

### Test 5: Mission Completion
1. Complete all objectives
2. Return to agent
3. Complete mission
4. Verify rewards granted
5. Verify database updated

## Troubleshooting

### Issue: JSON files not loading
- Check file paths in MissionLoader::LoadAll()
- Verify JSON syntax with online validator
- Check file permissions

### Issue: Missions not showing in agent window
- Verify Agent::MakeOffer() is called
- Check mission level matches agent level
- Verify faction ID matches

### Issue: Objectives not tracking
- Hook ObjectiveTracker into kill/collection events
- Verify mission state exists in database
- Check objective IDs match mission definition

### Issue: Mission space not creating
- Verify DungeonMgr/SpawnMgr integration
- Check system has available space
- Verify coordinates are generated

## Performance Considerations

1. **JSON Loading**: Load once at startup, cache in memory
2. **Database Queries**: Use prepared statements, batch operations
3. **Objective Tracking**: Only check active missions
4. **Mission Spaces**: Limit concurrent instances per system
5. **Cleanup**: Periodic cleanup of expired/abandoned missions

## Security Considerations

1. Validate all mission IDs from client
2. Verify character owns mission state
3. Prevent objective manipulation
4. Validate reward distribution
5. Rate limit mission accepts/aborts

## Future Enhancements

1. Mission sharing in fleets
2. Mission chaining/arcs
3. Dynamic difficulty scaling
4. Procedural mission generation
5. Mission marketplace
6. Custom player missions
7. Corp/alliance missions
8. Faction warfare missions
9. Epic arc system
10. Achievement tracking

## Migration from Old System

1. Keep old mission tables for reference
2. Run both systems in parallel initially
3. Migrate historical data if needed
4. Deprecate old system gradually
5. Remove old code after testing period

## Support Resources

- EVEmu Discord: Questions and support
- Documentation: Mission JSON format spec
- Wiki: Mission system architecture
- GitHub: Issue tracking

## Completion Checklist

### Core Implementation
- [ ] MissionDB methods implemented
- [ ] MissionSpaceMgr created
- [ ] Agent integration complete
- [ ] ObjectiveTracker implemented
- [ ] Client integration complete

### Database
- [ ] Migration script run
- [ ] Tables created
- [ ] Views created
- [ ] Indexes added
- [ ] Permissions set

### Testing
- [ ] JSON loading tested
- [ ] Mission offering tested
- [ ] Mission execution tested
- [ ] Objective tracking tested
- [ ] Mission completion tested
- [ ] All mission types tested

### Documentation
- [ ] API documentation complete
- [ ] JSON format documented
- [ ] Integration guide complete
- [ ] Troubleshooting guide complete

### Deployment
- [ ] Server config updated
- [ ] Database backed up
- [ ] Migration tested
- [ ] Rollback plan ready
- [ ] Team trained

## Contact

For questions or issues with this implementation:
- Check existing issues on GitHub
- Ask in #development on Discord
- Review mission system documentation
- Consult with project maintainers
