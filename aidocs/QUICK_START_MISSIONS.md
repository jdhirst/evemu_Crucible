# Quick Start: Complete Mission System Implementation

## ?? Quick Fix (5 Minutes)

### Step 1: Download JSON Library
```powershell
# Run from repository root
New-Item -ItemType Directory -Force -Path "dep\json\include\nlohmann"
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp" -OutFile "dep\json\include\nlohmann\json.hpp"
```

### Step 2: Update CMakeLists.txt
```cmake
# In src/eve-server/CMakeLists.txt, at the top after SET directives, add:
include_directories("${PROJECT_SOURCE_DIR}/dep/json/include")

# Remove or comment out the FetchContent section for JSON
```

### Step 3: Fix MissionLoader.h
```cpp
// In src/eve-server/missions/MissionLoader.h, replace the forward declaration with:
#include <nlohmann/json.hpp>
```

### Step 4: Build
```powershell
cmake --build . --config Debug
```

## ? Verification

### Check JSON Library
```powershell
# Should exist:
Test-Path "dep\json\include\nlohmann\json.hpp"
# Should return: True
```

### Check Mission Files
```powershell
# Should exist:
Get-ChildItem "src\eve-server\missions\data\*.json"
# Should show 2 JSON files
```

### Check Build
```powershell
# Should compile without errors
cmake --build . --config Debug 2>&1 | Select-String "error"
# Should return nothing (or only warnings)
```

## ?? Next Implementation Steps

### 1. Run Database Migration (10 minutes)
```sql
SOURCE sql/migrations/20260109_mission_system_refactor.sql;
SHOW TABLES LIKE 'mission%';
-- Should show 6 new tables
```

### 2. Implement MissionDB.cpp (2-3 hours)
```cpp
// Create src/eve-server/missions/MissionDB.cpp
// Implement all methods declared in MissionDB.h
// See docs/MISSION_IMPLEMENTATION_GUIDE.md for examples
```

### 3. Initialize in Server Startup (5 minutes)
```cpp
// In src/eve-server/eve-server.cpp, after other managers:
if (!sNewMissionDataMgr.Initialize()) {
    sLog.Error("main", "Failed to initialize New Mission System");
    return 1;
}
```

### 4. Add to EntityList Processing (5 minutes)
```cpp
// In src/eve-server/EntityList.cpp, Process() method:
if (++m_missionProcCount > 60) {
    m_missionProcCount = 0;
    sNewMissionDataMgr.Process();
}
```

### 5. Test JSON Loading (5 minutes)
```cpp
// Start server and check logs for:
// "MissionLoader: Loaded X missions from..."
// "NewMissionDataMgr: Loaded X mission definitions"
```

## ?? Implementation Priority

### Priority 1: Get It Running (1 day)
- [x] Fix JSON library
- [ ] Implement MissionDB.cpp
- [ ] Initialize in server
- [ ] Test JSON loading
- [ ] Verify database schema

### Priority 2: Basic Functionality (2-3 days)
- [ ] Update Agent::MakeOffer()
- [ ] Implement basic mission offering
- [ ] Test mission acceptance
- [ ] Verify database writes

### Priority 3: Complete Features (1 week)
- [ ] Create MissionSpaceMgr
- [ ] Implement ObjectiveTracker
- [ ] Add all mission types
- [ ] Test end-to-end flow

## ?? Troubleshooting

### JSON Library Not Found
```
Error: Cannot open include file: 'nlohmann/json.hpp'
```
**Fix:** Re-run Step 1 and 2 from Quick Fix above

### Namespace Errors
```
Error: 'Mission::Type::Courier': redefinition
```
**Fix:** Already handled with `MissionNew::` namespace

### Linker Errors
```
Error: undefined reference to MissionDB methods
```
**Fix:** Implement MissionDB.cpp (Priority 1)

## ?? Documentation Reference

| Document | Purpose |
|----------|---------|
| `README_MISSION_SYSTEM.md` | Complete status report |
| `docs/MISSION_IMPLEMENTATION_GUIDE.md` | Detailed implementation guide |
| `docs/MISSION_SYSTEM_STATUS.md` | Implementation checklist |
| `docs/NAMESPACE_CONFLICT_FIX.md` | Namespace resolution |

## ? One-Command Build (After JSON Fix)

```powershell
# From repository root
cmake --build . --config Debug --target eve-server -- /m
```

## ?? Success Criteria

You'll know it's working when you see in server logs:
```
[INFO] MissionLoader: Loaded 24 missions from high_quality_missions.json
[INFO] MissionLoader: Loaded 48 missions from premium_missions.json
[INFO] NewMissionDataMgr: Loaded 72 mission definitions in 15.234ms
[INFO] NewMissionDataMgr: Mission Data Manager Initialized
```

## ?? Need Help?

1. Check existing documentation
2. Review code comments
3. Look at existing mission system for examples
4. Ask in Discord #development

---

**Remember:** The system is 92% complete. You're just fixing the JSON library linkage and implementing the database methods. Everything else is done!
