-- Mission System Refactor
-- JSON-based mission system database schema
-- Author: Mission System Refactor
-- Date: January 2026

-- Drop old tables if doing complete refactor (optional, backup first!)
-- DROP TABLE IF EXISTS agtOffers;
-- DROP TABLE IF EXISTS agtMissions;
-- DROP TABLE IF EXISTS qstCourier;
-- DROP TABLE IF EXISTS qstMining;

-- Mission States Table
-- Tracks active mission instances for characters
CREATE TABLE IF NOT EXISTS missionStates (
    stateID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    characterID INT UNSIGNED NOT NULL,
    missionID INT UNSIGNED NOT NULL COMMENT 'ID from JSON mission definition',
    agentID INT UNSIGNED NOT NULL,
    status TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=Offered,1=Accepted,2=InProgress,3=Completed,4=Failed,5=Aborted',
    currentStage INT UNSIGNED NOT NULL DEFAULT 0,
    startedAt BIGINT NOT NULL,
    completedAt BIGINT DEFAULT 0,
    expiresAt BIGINT DEFAULT 0,
    missionSpaceID INT UNSIGNED DEFAULT 0 COMMENT 'Instance ID for deadspace missions',
    
    INDEX idx_character (characterID),
    INDEX idx_mission (missionID),
    INDEX idx_agent (agentID),
    INDEX idx_status (status),
    INDEX idx_expires (expiresAt),
    
    FOREIGN KEY (characterID) REFERENCES character_(characterID) ON DELETE CASCADE,
    FOREIGN KEY (agentID) REFERENCES agtAgents(agentID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Tracks active mission instances for characters';

-- Objective Progress Table
-- Tracks individual objective completion within missions
CREATE TABLE IF NOT EXISTS missionObjectiveProgress (
    trackingID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    stateID INT UNSIGNED NOT NULL,
    objectiveID INT UNSIGNED NOT NULL COMMENT 'Objective ID from mission JSON',
    progress INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Current progress value',
    completed TINYINT(1) NOT NULL DEFAULT 0,
    completedAt BIGINT DEFAULT 0,
    
    INDEX idx_state (stateID),
    INDEX idx_objective (objectiveID),
    INDEX idx_completed (completed),
    
    FOREIGN KEY (stateID) REFERENCES missionStates(stateID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Tracks objective completion progress';

-- Mission Spaces Table
-- Tracks instanced mission spaces (deadspace pockets)
CREATE TABLE IF NOT EXISTS missionSpaces (
    spaceID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    stateID INT UNSIGNED NOT NULL,
    systemID INT UNSIGNED NOT NULL,
    createdAt BIGINT NOT NULL,
    expiresAt BIGINT DEFAULT 0 COMMENT 'Cleanup time if abandoned',
    
    INDEX idx_state (stateID),
    INDEX idx_system (systemID),
    INDEX idx_expires (expiresAt),
    
    FOREIGN KEY (stateID) REFERENCES missionStates(stateID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Tracks instanced mission spaces';

-- Mission Space Objects Table
-- Tracks spawned objects in mission spaces
CREATE TABLE IF NOT EXISTS missionSpaceObjects (
    objectID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    spaceID INT UNSIGNED NOT NULL,
    itemID INT UNSIGNED NOT NULL COMMENT 'Entity itemID',
    typeID INT UNSIGNED NOT NULL,
    groupID INT UNSIGNED NOT NULL,
    x DOUBLE NOT NULL,
    y DOUBLE NOT NULL,
    z DOUBLE NOT NULL,
    spawnedAt BIGINT NOT NULL,
    
    INDEX idx_space (spaceID),
    INDEX idx_item (itemID),
    
    FOREIGN KEY (spaceID) REFERENCES missionSpaces(spaceID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Tracks objects spawned in mission spaces';

-- Mission Bookmarks Table
-- Stores mission-specific bookmarks
CREATE TABLE IF NOT EXISTS missionBookmarks (
    bookmarkID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    stateID INT UNSIGNED NOT NULL,
    ownerID INT UNSIGNED NOT NULL,
    itemID INT UNSIGNED NOT NULL,
    typeID INT UNSIGNED NOT NULL,
    flag INT UNSIGNED NOT NULL,
    memo VARCHAR(255) DEFAULT NULL,
    created BIGINT NOT NULL,
    x DOUBLE NOT NULL,
    y DOUBLE NOT NULL,
    z DOUBLE NOT NULL,
    locationID INT UNSIGNED NOT NULL,
    
    INDEX idx_state (stateID),
    INDEX idx_owner (ownerID),
    
    FOREIGN KEY (stateID) REFERENCES missionStates(stateID) ON DELETE CASCADE,
    FOREIGN KEY (ownerID) REFERENCES character_(characterID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Mission-specific bookmarks';

-- Mission Statistics Table
-- Tracks mission completion statistics for analytics
CREATE TABLE IF NOT EXISTS missionStatistics (
    statID INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    characterID INT UNSIGNED NOT NULL,
    missionID INT UNSIGNED NOT NULL,
    agentID INT UNSIGNED NOT NULL,
    completedAt BIGINT NOT NULL,
    durationMinutes INT UNSIGNED NOT NULL,
    iskReward INT UNSIGNED NOT NULL,
    lpReward INT UNSIGNED NOT NULL,
    failedObjectives INT UNSIGNED DEFAULT 0,
    
    INDEX idx_character (characterID),
    INDEX idx_mission (missionID),
    INDEX idx_completed (completedAt),
    
    FOREIGN KEY (characterID) REFERENCES character_(characterID) ON DELETE CASCADE,
    FOREIGN KEY (agentID) REFERENCES agtAgents(agentID) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 
COMMENT='Mission completion statistics';

-- Create views for easy querying
CREATE OR REPLACE VIEW vwActiveMissions AS
SELECT 
    ms.stateID,
    ms.characterID,
    c.characterName,
    ms.missionID,
    ms.agentID,
    a.agentName,
    ms.status,
    ms.currentStage,
    ms.startedAt,
    ms.expiresAt,
    ms.missionSpaceID,
    COUNT(mop.trackingID) as totalObjectives,
    SUM(mop.completed) as completedObjectives
FROM missionStates ms
JOIN character_ c ON ms.characterID = c.characterID
JOIN agtAgents a ON ms.agentID = a.agentID
LEFT JOIN missionObjectiveProgress mop ON ms.stateID = mop.stateID
WHERE ms.status IN (0, 1, 2) -- Offered, Accepted, InProgress
GROUP BY ms.stateID
ORDER BY ms.startedAt DESC;

CREATE OR REPLACE VIEW vwMissionCompletion AS
SELECT 
    characterID,
    COUNT(*) as totalCompleted,
    SUM(iskReward) as totalISK,
    SUM(lpReward) as totalLP,
    AVG(durationMinutes) as avgDuration
FROM missionStatistics
GROUP BY characterID;

-- Insert migration tracking
INSERT INTO evemu_migration (version, description, applied_at)
VALUES (
    '20260109_mission_system_refactor',
    'JSON-based mission system with state tracking',
    NOW()
);

-- Grant permissions (adjust as needed for your setup)
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionStates TO 'evemu'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionObjectiveProgress TO 'evemu'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionSpaces TO 'evemu'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionSpaceObjects TO 'evemu'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionBookmarks TO 'evemu'@'localhost';
-- GRANT SELECT, INSERT, UPDATE, DELETE ON evemu.missionStatistics TO 'evemu'@'localhost';

-- Indexes for performance
ANALYZE TABLE missionStates;
ANALYZE TABLE missionObjectiveProgress;
ANALYZE TABLE missionSpaces;
ANALYZE TABLE missionSpaceObjects;
ANALYZE TABLE missionBookmarks;
ANALYZE TABLE missionStatistics;
