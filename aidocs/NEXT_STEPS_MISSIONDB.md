# Mission System - Next Implementation Steps

## ?? Immediate Next Task: MissionDB.cpp

The **highest priority** is implementing the database methods. Here's what you need to do:

### Step 1: Check if MissionDB.cpp exists

The file should be at: `src/eve-server/missions/MissionDB.cpp`

If it exists, you need to **add** the new methods to it (don't replace existing code).
If it doesn't exist, create it using the template below.

### Step 2: Implementation Template

```cpp
/**
 * @name MissionDB.cpp
 *   Database operations for mission system
 */

#include "eve-server.h"
#include "MissionDB.h"
#include "database/EVEDBUtils.h"

// ========== NEW JSON-BASED MISSION SYSTEM METHODS ==========

bool MissionDB::SaveMissionState(const MissionNew::MissionState& state)
{
    DBerror err;
    uint32 stateID = 0;
    
    if (!sDatabase.RunQueryLID(err, stateID,
        "INSERT INTO missionStates ("
        " characterID, missionID, agentID, status, currentStage,"
        " startedAt, completedAt, expiresAt, missionSpaceID)"
        " VALUES (%u, %u, %u, %u, %u, %li, %li, %li, %u)",
        state.characterID, state.missionID, state.agentID, 
        state.status, state.currentStage, state.startedAt, 
        state.completedAt, state.expiresAt, state.missionSpaceID))
    {
        _log(MISSION__ERROR, "SaveMissionState failed: %s", err.c_str());
        return false;
    }
    
    _log(MISSION__TRACE, "Saved mission state %u for character %u", 
         stateID, state.characterID);
    return true;
}

// ... implement remaining methods following this pattern ...
```

### Step 3: Methods to Implement

Copy these method signatures from `MissionDB.h` and implement them:

1. **Mission State Management** (7 methods)
   - `SaveMissionState`
   - `LoadMissionState`
   - `LoadAllMissionStates`
   - `LoadCharacterMissionStates`
   - `UpdateMissionStatus`
   - `UpdateMissionStage`
   - `DeleteMissionState`

2. **Objective Progress** (4 methods)
   - `SaveObjectiveProgress`
   - `LoadObjectiveProgress`
   - `UpdateObjectiveProgress`
   - `CompleteObjective`

3. **Mission Spaces** (4 methods)
   - `CreateMissionSpace`
   - `LoadMissionSpace`
   - `DeleteMissionSpace`
   - `CleanupExpiredSpaces`

4. **Mission Space Objects** (3 methods)
   - `SaveMissionSpaceObject`
   - `LoadMissionSpaceObjects`
   - `DeleteMissionSpaceObject`

5. **Mission Bookmarks** (3 methods)
   - `SaveMissionBookmark`
   - `LoadMissionBookmarks`
   - `DeleteMissionBookmark`

6. **Statistics** (2 methods)
   - `SaveMissionStatistics`
   - `LoadMissionStatistics`

### Step 4: Reference Examples

Look at these existing files for database patterns:
- `src/eve-server/dungeon/DungeonDB.cpp` - Similar structure
- `src/eve-server/agents/AgentDB.cpp` - Agent patterns
- `src/eve-server/station/StationDB.cpp` - Station patterns

### Step 5: Build and Test

After implementing each group of methods:
```bash
cd C:\Users\nixx\Projects\evemu\evemu_Crucible
cmake --build build --config Debug
```

---

## ?? Alternative: Minimal Quick Start

If you want to test the system quickly before implementing all database methods:

### Create Stub Implementations

```cpp
bool MissionDB::SaveMissionState(const MissionNew::MissionState& state)
{
    _log(MISSION__WARNING, "SaveMissionState not yet implemented - stub only");
    return false;
}

// ... stub remaining methods similarly ...
```

This allows the code to compile and you can test other parts of the system while gradually implementing the database layer.

---

## ?? How to Check for Existing MissionDB.cpp

### Option 1: File Explorer
Navigate to: `C:\Users\nixx\Projects\evemu\evemu_Crucible\src\eve-server\missions\`

### Option 2: Command Line
```powershell
cd C:\Users\nixx\Projects\evemu\evemu_Crucible
dir src\eve-server\missions\*.cpp
```

### Option 3: VS Code
Open the folder and search for files containing "MissionDB"

---

## ?? Success Criteria

You'll know it's working when:
1. ? Code compiles without errors
2. ? Server starts without crashes
3. ? Database tables are created (run migration SQL)
4. ? Mission state can be saved and loaded
5. ? Objectives can be tracked

---

## ?? Pro Tips

### Database Patterns to Follow

1. **Always use prepared statements** (already done with %u, %s, etc.)
2. **Always check error returns** (`if (!sDatabase.RunQuery(...))`
3. **Always log errors** (`_log(MISSION__ERROR, ...)`)
4. **Escape strings** (`sDatabase.DoEscapeString()`)
5. **Use transactions for multi-step operations**

### Naming Conventions

- Table names: `camelCase` (e.g., `missionStates`)
- Column names: `camelCase` (e.g., `characterID`)
- C++ variables: `camelCase` (e.g., `stateID`)
- Log types: `MISSION__ERROR`, `MISSION__TRACE`, etc.

### Error Handling

```cpp
DBerror err;
if (!sDatabase.RunQuery(err, "...")) {
    _log(MISSION__ERROR, "Operation failed: %s", err.c_str());
    return false; // or appropriate error value
}
```

---

## ?? Ready to Code!

You have everything you need:
- ? Build is working
- ? Dependencies are installed
- ? Database schema is ready
- ? Headers are defined
- ? Examples to follow

**Just implement the MissionDB.cpp methods and you're golden!** ??

Good luck! You're doing great! ??
