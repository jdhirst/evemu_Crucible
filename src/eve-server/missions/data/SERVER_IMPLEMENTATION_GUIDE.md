# EVE Online Mission System - Server Implementation Guide

## Overview
This document outlines the server-side implementation requirements for the EVE Online mission system based on the generated mission data structure. The server must support mission distribution, state tracking, objective completion, and reward distribution.

---

## 1. Mission Data Structure

### 1.1 Core Mission Metadata
The server must store and process the following mission metadata:

```json
{
  "metadata": {
    "id": integer,
    "name": string,
    "faction": string,
    "mission_type": string,  // combat, courier, mining, reconnaissance, retrieve
    "difficulty": integer,   // 1-5 (maps to Level 1-5)
    "estimated_duration": integer,  // minutes
    "is_epic": boolean,
    "is_funny": boolean,
    "complexity": string,    // simple, standard, multi_stage, epic
    "tone": string,          // professional, comedic, dramatic
    "version": string,
    "generated_at": timestamp
  }
}
```

**Implementation Requirements:**
- Mission ID must be unique and persistent
- Faction determines agent pool and system locations
- Mission type affects completion validation logic
- Difficulty affects rewards, enemy strength, and agent availability
- Epic missions have 20+ stages and special flow
- Funny missions have comedic narrative only (no gameplay impact)

### 1.2 Agent Information
```json
{
  "agent": {
    "name": string,
    "corporation": string,
    "location": {
      "system": string,
      "system_id": integer,      // mapSolarSystems.solarSystemID
      "constellation_id": integer,
      "region_id": integer,
      "security": float
    }
  }
}
```

**Implementation Requirements:**
- Agent names come from `eveStaticOwners` table
- Agent location determines mission acceptance location
- Players must be in same system/station to accept mission
- Agent corporation affects loyalty point rewards

### 1.3 Mission Space
```json
{
  "mission_space": {
    "system": string,
    "type": string,  // deadspace, normal_space, station, asteroid_belt
    "description": string,
    "requires_warp": boolean
  }
}
```

**Implementation Requirements:**
- Deadspace missions create instanced pockets (only accepting player can enter)
- Normal space missions occur in public space (other players can interfere)
- Requires_warp indicates if mission location is separate from agent
- System determines solar system where mission takes place

---

## 2. Objectives System

### 2.1 Objective Structure
```json
{
  "objectives_structured": [
    {
      "id": integer,
      "type": string,  // combat, courier, mine, reconnaissance, retrieve, hack, defend
      "description": string,
      "required": boolean,
      "completion_criteria": {
        "enemy_ships_destroyed": integer,
        "items_collected": integer,
        "item_type": string,
        "typeID": integer,  // invTypes.typeID for specific items
        "destination_system": string,
        "destination_station": string,
        "time_limit": integer  // seconds
      }
    }
  ]
}
```

**Implementation Requirements:**

#### Combat Objectives
- Track specific ships destroyed (by typeID or class)
- Must verify kills happen in mission space
- Support "destroy X of Y ship type" criteria

#### Courier Objectives
- Accept item from agent (add to cargo)
- Track delivery to destination system/station
- Verify item still in cargo on delivery
- Support multi-hop courier chains

#### Mining Objectives
- Track ore mined by typeID
- Verify mining happens in mission space
- Support multiple ore types

#### Retrieve Objectives
- Track items collected from containers/wrecks in mission space
- Must verify items collected from static objects with correct typeID
- Support mission-specific items (not tradeable)

#### Reconnaissance Objectives
- Track locations visited/scanned
- Support "approach within X km" criteria
- May require targeting/scanning specific objects

### 2.2 Objective State Tracking
Server must track per-player mission state:
```python
mission_state = {
    "mission_id": integer,
    "player_id": integer,
    "status": string,  // accepted, in_progress, completed, failed, aborted
    "objectives_completed": [objective_ids],
    "started_at": timestamp,
    "completed_at": timestamp,
    "expires_at": timestamp  // optional time limit
}
```

---

## 3. Mission Flow System

### 3.1 Stage Structure
```json
{
  "mission_flow": {
    "stages": [
      {
        "stage": integer,
        "type": string,  // agent_dialog, warp_to_location, combat, courier_delivery, popup_message
        "location": string,  // agent_station, mission_location, destination_station, any
        "agent": string,
        "description": string,
        "action": string,  // accept_mission, warp_to_mission, complete_objectives, deliver_item
        "message_type": string,  // warning, info, completion
        "is_completion": boolean
      }
    ],
    "total_stages": integer,
    "requires_agent_return": boolean,
    "complexity": string
  }
}
```

**Implementation Requirements:**

#### Stage Types:
1. **agent_dialog**: Player must be at agent station to interact
2. **warp_to_location**: Player warps to mission space
3. **combat/retrieve/etc**: Player completes objectives in mission space
4. **courier_delivery**: Player delivers to destination
5. **popup_message**: Automatic message when conditions met

#### Stage Progression:
- Stages must complete in order (no skipping)
- Server validates stage completion before advancing
- Epic missions may have 20+ stages with varied activities
- Some stages may be optional (required=false objectives)

### 3.2 Agent Return Requirement
- If `requires_agent_return: true`, player must return to agent station to complete
- If `false`, mission auto-completes when final objective done
- Server must check location on completion attempt

---

## 4. Encounters & Combat System

### 4.1 Encounter Structure
```json
{
  "encounters": [
    {
      "wave": integer,
      "trigger": string,  // proximity, timer, objective_complete, manual
      "distance": string,  // "30km", "immediate", etc
      "ships": [
        {
          "type": string,
          "type_id": integer,  // invTypes.typeID
          "class": string,     // Frigate, Cruiser, Battleship
          "count": integer,
          "behavior": string,  // aggressive, passive, defensive
          "orbit_range": string
        }
      ],
      "static_objects": [
        {
          "type": string,       // container, wreck, structure
          "type_id": integer,   // invTypes.typeID of container/structure
          "name": string,
          "contents": [
            {
              "type_id": integer,  // invTypes.typeID of item inside
              "quantity": integer,
              "is_objective": boolean
            }
          ]
        }
      ]
    }
  ]
}
```

**Implementation Requirements:**

#### Ship Spawning:
- Use `type_id` (invTypes.typeID) to spawn correct NPC ship
- `count` determines number of ships
- `behavior` affects AI aggression patterns
- All ships in wave should spawn based on trigger

#### Triggers:
- **proximity**: Spawn when player within X km of location
- **timer**: Spawn X seconds after mission start/previous wave
- **objective_complete**: Spawn when specific objective done
- **manual**: Spawn on explicit player action (gate activation, etc)

#### Static Objects (CRITICAL):
- Objects have specific `type_id` from invTypes table
- Objects must be spawned in mission space at creation
- Objects persist until destroyed/looted
- **Contents** list specifies exact loot with typeIDs and quantities
- `is_objective: true` items are mission-critical and must be collected
- Server validates item collection from correct object

**Example Static Object:**
```json
{
  "type": "container",
  "type_id": 3293,  // Standard Container
  "name": "Supply Cache Alpha",
  "contents": [
    {
      "type_id": 209,    // Antimatter Charge M
      "quantity": 500,
      "is_objective": false
    },
    {
      "type_id": 3689,   // Scientists (mission item)
      "quantity": 12,
      "is_objective": true
    }
  ]
}
```

#### Implementation Notes:
- Create container/wreck/structure with `type_id` in mission space
- Populate container inventory with specified items
- Track which objects have been looted
- Verify objective items collected from correct containers

---

## 5. Rewards System

### 5.1 Reward Structure
```json
{
  "rewards": {
    "isk": {
      "base": integer,
      "bonus": integer,  // time bonus if completed quickly
      "total_possible": integer
    },
    "loyalty_points": {
      "base": integer,
      "corporation": string
    },
    "standings": {
      "faction": string,
      "increase": float  // +0.01, etc
    },
    "items": [
      {
        "type_id": integer,
        "quantity": integer,
        "name": string
      }
    ],
    "salvage": boolean,
    "loot_value_estimate": integer
  }
}
```

**Implementation Requirements:**

#### ISK Rewards:
- Base ISK always awarded on completion
- Bonus ISK awarded if completed within time threshold
- Calculate bonus based on `estimated_duration` vs actual time

#### Loyalty Points:
- Award to specified corporation
- LP amount scales with difficulty
- Track LP per corporation per player

#### Standings:
- Modify player standings with faction
- Also affects corporation/agent standings
- Negative standings for mission abandonment

#### Item Rewards:
- Items with specific typeIDs given on completion
- Items added to player hangar at agent station
- Optional: may require cargo space

#### Salvage:
- If true, wrecks in mission space have loot
- Player can salvage for additional profit
- Loot value estimate is informational only

---

## 6. Courier Missions (Special Case)

### 6.1 Courier Details Structure
```json
{
  "courier_details": {
    "pickup_location": {
      "system": string,
      "station": string,
      "system_id": integer
    },
    "dropoff_location": {
      "system": string,
      "station": string,
      "system_id": integer
    },
    "package": {
      "type_id": integer,  // invTypes.typeID
      "name": string,
      "volume": float,
      "quantity": integer
    },
    "route": {
      "jumps": integer,
      "distance_ly": float,
      "security_status": string  // highsec, lowsec, nullsec, mixed
    },
    "time_limit": integer,  // seconds
    "collateral": integer   // ISK, optional
  }
}
```

**Implementation Requirements:**

#### Mission Flow:
1. Player accepts mission at agent
2. Item(s) added to player cargo (check cargo space!)
3. Player travels to destination
4. Player docks at destination station
5. Server validates item still in cargo
6. Remove item, award rewards

#### Validation:
- Check cargo space before adding items
- Track if player loses/trades courier items (mission fails!)
- Validate destination system/station
- Check time limit on delivery

#### Collateral:
- If specified, player pays collateral on acceptance
- Refunded on successful delivery
- Forfeited if mission failed/abandoned

---

## 7. Mission Requirements

### 7.1 Requirements Structure
```json
{
  "requirements": {
    "min_ship_class": string,      // Frigate, Destroyer, Cruiser, Battlecruiser, Battleship
    "recommended_ship_class": string,
    "damage_type": {
      "primary": string,   // EM, Thermal, Kinetic, Explosive
      "secondary": string
    },
    "tank_type": {
      "primary": string,
      "secondary": string
    }
  }
}
```

**Implementation Requirements:**
- `min_ship_class`: Informational (could enforce but not required)
- `recommended_ship_class`: Informational only
- `damage_type`: Tells player what damage to deal (NPC resistances)
- `tank_type`: Tells player what resistance to fit (NPC damage types)
- Server does not enforce these, they're for UI hints

---

## 8. Narrative System

### 8.1 Narrative Structure
```json
{
  "narrative": {
    "briefing": string,    // Agent's mission description
    "objectives": string,  // Formatted objectives text (markdown)
    "completion": string   // Agent's completion dialog
  }
}
```

**Implementation Requirements:**

#### Template Variables:
Briefings use template variables that server must replace:
- `{{[character]player.name}}` → Player's character name
- `{{[ship]player.ship}}` → Player's current ship name
- `{{[faction]player.faction}}` → Player's faction

#### Display:
- Show briefing when player opens mission details
- Objectives shown in mission journal/tracker
- Completion message shown on mission complete

#### Markdown Support:
- Objectives may contain markdown formatting
- Server should strip/convert for text-only clients
- Web clients can render full markdown

---

## 9. Special Mission Types

### 9.1 Epic Missions
Identified by: `is_epic: true`

**Special Requirements:**
- 20+ stages in mission_flow
- Multiple objectives across different systems
- May require multiple ship types
- Higher rewards (3-5x normal)
- Cannot be declined without standings hit
- Longer expiration time

### 9.2 Funny Missions
Identified by: `is_funny: true`

**Special Requirements:**
- Comedic narrative only (no gameplay changes)
- Same mechanics as normal missions
- May have unusual objective descriptions
- Briefings/completion text are humorous
- No special server logic needed

---

## 10. Database Schema Requirements

### 10.1 Required Tables

#### missions
```sql
CREATE TABLE missions (
    missionID INT PRIMARY KEY AUTO_INCREMENT,
    missionName VARCHAR(255),
    factionID INT,  -- Link to faction
    missionType VARCHAR(50),
    difficulty INT,
    is_epic BOOLEAN,
    is_funny BOOLEAN,
    json_data TEXT,  -- Full mission JSON
    created_at TIMESTAMP
);
```

#### mission_states
```sql
CREATE TABLE mission_states (
    stateID INT PRIMARY KEY AUTO_INCREMENT,
    characterID INT,
    missionID INT,
    status ENUM('offered', 'accepted', 'in_progress', 'completed', 'failed', 'aborted'),
    current_stage INT,
    objectives_completed TEXT,  -- JSON array of completed objective IDs
    started_at TIMESTAMP,
    completed_at TIMESTAMP,
    expires_at TIMESTAMP
);
```

#### mission_objectives_tracking
```sql
CREATE TABLE mission_objectives_tracking (
    trackingID INT PRIMARY KEY AUTO_INCREMENT,
    stateID INT,
    objectiveID INT,
    progress INT,  -- For "destroy 10 ships" etc
    completed BOOLEAN,
    completed_at TIMESTAMP
);
```

### 10.2 Integration with EVE Database

**Required EVE Tables:**
- `invTypes`: For typeIDs (ships, items, containers, structures)
- `invGroups`: For ship/item grouping
- `mapSolarSystems`: For system IDs and locations
- `eveStaticOwners`: For agent names
- `agtAgents`: For agent data

**TypeID Validation:**
- All `type_id` values in mission JSON reference `invTypes.typeID`
- Server should validate typeIDs exist before spawning
- Use invTypes for item names, volumes, etc

---

## 11. API Endpoints Needed

### 11.1 Mission Management
```
GET  /api/missions/available?characterID=X
     → Returns list of missions available to character

GET  /api/missions/{missionID}
     → Returns full mission details

POST /api/missions/{missionID}/accept
     → Character accepts mission

POST /api/missions/{missionID}/abort
     → Character abandons mission (standings penalty)

POST /api/missions/{missionID}/complete
     → Character attempts to complete mission
     → Server validates all objectives done
     → Awards rewards
```

### 11.2 Objective Tracking
```
POST /api/missions/{missionID}/objectives/{objectiveID}/progress
     → Update objective progress (killed ship, collected item, etc)

GET  /api/missions/{missionID}/state
     → Get current mission state for character
```

### 11.3 Mission Space
```
POST /api/missions/{missionID}/enter
     → Character warps to mission space
     → Server creates instanced space if deadspace
     → Spawns encounters/static objects

GET  /api/missions/{missionID}/space
     → Get current state of mission space
     → Returns spawned ships, objects, etc
```

---

## 12. Mission Lifecycle

### 12.1 Complete Flow

1. **Mission Generation** (Already done by Python system)
   - Mission JSON created with all data
   - Stored in database

2. **Mission Offered**
   - Agent shows available missions to player
   - Based on standings, difficulty, faction

3. **Mission Accepted**
   - Create mission_state record
   - Set expiration time
   - If courier: add items to cargo
   - Set status to 'accepted'

4. **Mission In Progress**
   - Player warps to mission space
   - Server spawns encounters/objects
   - Track objective completion
   - Update mission state on progress

5. **Mission Completed**
   - Validate all required objectives done
   - Check agent return requirement
   - Award rewards (ISK, LP, standings, items)
   - Set status to 'completed'
   - Clean up mission space

6. **Mission Failed/Aborted**
   - Player abandons or fails time limit
   - Apply standings penalty
   - Remove courier items if applicable
   - Clean up mission space

---

## 13. Critical Implementation Notes

### 13.1 Static Objects MUST Include TypeIDs
- Every container, wreck, structure in encounters has `type_id`
- Every item in those objects has `type_id`
- Server MUST spawn objects with correct typeID
- Server MUST populate container inventories with correct items
- Objective completion validation checks specific typeIDs collected

### 13.2 Mission Instancing
- Deadspace missions are private (only accepting player can enter)
- Normal space missions are public (anyone can interfere)
- Server must track which spaces belong to which players
- Clean up abandoned mission spaces after timeout

### 13.3 Performance Considerations
- Mission JSON can be large (10-50KB)
- Cache frequently accessed missions
- Index mission_states by characterID for fast lookup
- Consider separate table for static objects if needed

### 13.4 Anti-Exploit Measures
- Validate objective completion happens in mission space
- Track which containers items come from
- Prevent trading mission-specific items
- Enforce time limits strictly
- Validate courier items not lost/traded

---

## 14. Testing Checklist

- [ ] Mission acceptance with cargo space check (courier)
- [ ] Objective tracking for all types (combat, courier, mine, retrieve)
- [ ] Static object spawning with correct typeIDs
- [ ] Container inventory population and looting
- [ ] Encounter wave triggers (proximity, timer, objective)
- [ ] Stage progression in correct order
- [ ] Agent return requirement validation
- [ ] Reward distribution (ISK, LP, standings, items)
- [ ] Time limit enforcement and bonus calculation
- [ ] Mission abandonment with standings penalty
- [ ] Epic mission multi-stage flow
- [ ] Mission space cleanup on completion/abort
- [ ] Template variable replacement in narrative
- [ ] TypeID validation against invTypes table

---

## 15. Example Mission JSON Reference

See `output/missions_output/mission_*.json` for complete examples.

Key example files:
- **Combat Mission**: Shows encounters with ships and static objects
- **Courier Mission**: Shows courier_details structure
- **Epic Mission**: Shows complex multi-stage flow
- **Funny Mission**: Shows comedic narrative (normal mechanics)

---

## Questions for Server Team

1. **Mission Space Architecture**: How will instanced deadspace be implemented?
2. **Static Object System**: Can containers be spawned with pre-populated inventories?
3. **Objective Validation**: How will server track which container items came from?
4. **Performance**: Expected max concurrent missions per player?
5. **Expiration**: Should expired missions auto-clean or require manual abort?

---

**Document Version**: 1.0  
**Last Updated**: January 9, 2026  
**Contact**: Mission Generation Team
