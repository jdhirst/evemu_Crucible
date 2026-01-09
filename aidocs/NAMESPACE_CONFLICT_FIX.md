# CRITICAL: Namespace Conflict Fix Required

## Issue
The new mission system uses `Mission::` namespace which conflicts with existing `Mission::Type`, `Mission::State`, and `Mission::Status` enums in `src/eve-common/EVE_Missions.h`.

## Solution
Changed new mission system to use `MissionNew::` namespace to avoid conflicts.

## Files That Need Updating

### 1. **src/eve-server/missions/MissionData.h** ? DONE
Changed `namespace Mission` to `namespace MissionNew`

### 2. **src/eve-server/missions/MissionLoader.cpp**
Replace all references:
- `Mission::MissionData` ? `MissionNew::MissionData`  
- `Mission::Type::` ? `MissionNew::MissionType::`
- `Mission::Complexity::` ? `MissionNew::Complexity::`
- `Mission::Tone::` ? `MissionNew::Tone::`
- `Mission::TriggerType::` ? `MissionNew::TriggerType::`
- `Mission::Behavior::` ? `MissionNew::Behavior::`
- `Mission::StageType::` ? `MissionNew::StageType::`
- `Mission::Status::` ? `MissionNew::Status::`

### 3. **src/eve-server/missions/NewMissionDataMgr.h**
Replace all references:
- `Mission::MissionData` ? `MissionNew::MissionData`
- `Mission::MissionState` ? `MissionNew::MissionState`
- `Mission::ObjectiveProgress` ? `MissionNew::ObjectiveProgress`
- `Mission::Status::` ? `MissionNew::Status::`

### 4. **src/eve-server/missions/NewMissionDataMgr.cpp**
Replace all references:
- `Mission::MissionData` ? `MissionNew::MissionData`
- `Mission::MissionState` ? `MissionNew::MissionState`
- `Mission::ObjectiveProgress` ? `MissionNew::ObjectiveProgress`
- `Mission::Status::` ? `MissionNew::Status::`
- `Mission::Type::` ? `MissionNew::MissionType::`

### 5. **src/eve-server/missions/MissionDB.h**
Replace all references:
- `Mission::MissionState` ? `MissionNew::MissionState`
- `Mission::ObjectiveProgress` ? `MissionNew::ObjectiveProgress`

## Quick Fix Script

Run this in PowerShell from the repo root:

```powershell
$files = @(
    "src\eve-server\missions\MissionLoader.cpp",
    "src\eve-server\missions\NewMissionDataMgr.h",
    "src\eve-server\missions\NewMissionDataMgr.cpp",
    "src\eve-server\missions\MissionDB.h"
)

foreach ($file in $files) {
    (Get-Content $file) -replace 'Mission::', 'MissionNew::' | Set-Content $file
    Write-Host "Updated $file"
}

# Special case for MissionType
$files = @(
    "src\eve-server\missions\MissionLoader.cpp",
    "src\eve-server\missions\NewMissionDataMgr.cpp"
)

foreach ($file in $files) {
    (Get-Content $file) -replace 'MissionNew::Type::', 'MissionNew::MissionType::' | Set-Content $file
    Write-Host "Fixed MissionType in $file"
}

Write-Host "Namespace fix complete!"
```

## Why This Matters

The existing system uses:
- `Mission::Type::Combat` (value 2)
- `Mission::Type::Courier` (value 3)
- `Mission::Type::Mining` (value 5)

The new system ALSO tried to use:
- `Mission::Type::Combat` (value 0)
- `Mission::Type::Courier` (value 1)
- `Mission::Type::Mining` (value 2)

This causes C++ compiler errors due to enum value conflicts.

## Future Consideration

Once the new system is fully tested and working, we can:
1. Deprecate the old Mission namespace
2. Rename MissionNew back to Mission
3. Update old code to use the new system

For now, keep both systems separate to allow gradual migration.
