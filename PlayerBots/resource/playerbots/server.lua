-- PlayerBots | Nexys.Tuga
-- Virtual players in the public MTA server browser without creating real connections.

local SETTINGS_FILE = "settings.json"
local DATA_PREFIX = "playerbots:"

local cfg = {
    enabled = false,
    target = 0,
    showTab = true,
    nameMode = "custom",      -- random | specific | custom
    customNames = {},
    specificNames = {},
    replaceEnabled = false,
    replaceAt = 25,
}

local randomPool = {}
local nativeReady = false

local function trim(s)
    return tostring(s or ""):match("^%s*(.-)%s*$") or ""
end

local function clampInt(value, low, high)
    value = tonumber(value)
    if not value or value ~= value or value == math.huge or value == -math.huge then return nil end
    value = math.floor(value)
    if value < low then value = low end
    if value > high then value = high end
    return value
end

local function cleanName(name)
    name = trim(name)
    name = name:gsub("#%x%x%x%x%x%x", "")
    name = name:gsub("[%c]", "")
    if name == "" then return nil end
    if #name > 30 then name = name:sub(1, 30) end
    return name
end

local function parseCustomNames(raw)
    local out, seen = {}, {}
    raw = tostring(raw or "")
    for part in raw:gmatch("[^,]+") do
        local name = cleanName(part)
        local key = name and name:lower() or nil
        if name and not seen[key] then
            seen[key] = true
            out[#out + 1] = name
        end
    end
    return out
end

local function customNamesText()
    return table.concat(cfg.customNames or {}, ", ")
end

local function databaseNameSet()
    local set = {}
    for _, name in ipairs(PLAYERBOTS_NAMES or {}) do
        set[tostring(name):lower()] = name
    end
    return set
end

local function sanitizeSpecificNames(values)
    local out, seen = {}, {}
    local allowed = databaseNameSet()
    if type(values) ~= "table" then return out end
    for _, value in ipairs(values) do
        local key = trim(value):lower()
        local canonical = allowed[key]
        if canonical and not seen[key] then
            seen[key] = true
            out[#out + 1] = canonical
        end
    end
    return out
end

local function isAuthorized(player)
    if not player then return true end
    return isElement(player)
        and getElementType(player) == "player"
        and hasObjectPermissionTo(player, "command.playerbots", false)
end

local function persist()
    local data = {
        enabled = cfg.enabled,
        target = cfg.target,
        showTab = cfg.showTab,
        nameMode = cfg.nameMode,
        customNames = cfg.customNames,
        specificNames = cfg.specificNames,
        replaceEnabled = cfg.replaceEnabled,
        replaceAt = cfg.replaceAt,
    }
    if fileExists(SETTINGS_FILE) then fileDelete(SETTINGS_FILE) end
    local f = fileCreate(SETTINGS_FILE)
    if not f then return false end
    fileWrite(f, toJSON(data, true))
    fileClose(f)
    return true
end

local function restore()
    if not fileExists(SETTINGS_FILE) then return end
    local f = fileOpen(SETTINGS_FILE, true)
    if not f then return end
    local raw = fileRead(f, fileGetSize(f))
    fileClose(f)
    local parsed = fromJSON(raw)
    if type(parsed) ~= "table" then return end

    local maxPlayers = math.max(1, getMaxPlayers())
    cfg.target = clampInt(parsed.target, 0, maxPlayers) or cfg.target
    cfg.enabled = parsed.enabled == true
    cfg.showTab = parsed.showTab ~= false
    if parsed.nameMode == "random" or parsed.nameMode == "specific" or parsed.nameMode == "custom" then
        cfg.nameMode = parsed.nameMode
    end
    cfg.replaceEnabled = parsed.replaceEnabled == true
    cfg.replaceAt = clampInt(parsed.replaceAt, 1, maxPlayers) or math.min(25, maxPlayers)

    cfg.customNames = {}
    if type(parsed.customNames) == "table" then
        local seen = {}
        for _, value in ipairs(parsed.customNames) do
            local name = cleanName(value)
            local key = name and name:lower() or nil
            if name and not seen[key] then
                seen[key] = true
                cfg.customNames[#cfg.customNames + 1] = name
            end
        end
    end
    cfg.specificNames = sanitizeSpecificNames(parsed.specificNames)
end

local function shuffledDatabase()
    local pool = {}
    for i, name in ipairs(PLAYERBOTS_NAMES or {}) do pool[i] = name end
    for i = #pool, 2, -1 do
        local j = math.random(i)
        pool[i], pool[j] = pool[j], pool[i]
    end
    return pool
end

local function rebuildRandomPool()
    randomPool = shuffledDatabase()
end

local function selectedPool()
    if cfg.nameMode == "custom" then return cfg.customNames end
    if cfg.nameMode == "specific" then return cfg.specificNames end
    if #randomPool == 0 and #(PLAYERBOTS_NAMES or {}) > 0 then rebuildRandomPool() end
    return randomPool
end

local function effectiveVirtualCount(realPlayers)
    if not cfg.enabled then return 0 end

    local maxPlayers = math.max(1, getMaxPlayers())
    local availableSlots = math.max(0, maxPlayers - realPlayers)
    local count = math.min(cfg.target, availableSlots)

    if cfg.replaceEnabled then
        local limit = clampInt(cfg.replaceAt, 1, maxPlayers) or maxPlayers
        count = math.min(count, math.max(0, limit - realPlayers))
    end

    count = math.min(count, #selectedPool())
    return math.max(0, count)
end

local function buildBots(realPlayers)
    local pool = selectedPool()
    local count = effectiveVirtualCount(realPlayers)
    local bots = {}
    for i = 1, count do
        bots[#bots + 1] = {
            name = pool[i],
            id = 9000 + i,
            ping = 0,
        }
    end
    return bots
end

local function nativeFunctionsAvailable()
    local required = {
        playerBotsNativeBeginCount, playerBotsNativeCommitCount,
        playerBotsNativeNamesBegin, playerBotsNativeNameBegin,
        playerBotsNativeNameCommit, playerBotsNativeNamesCommit,
        playerBotsNativeASEEnable, playerBotsNativeASEDisable,
        playerBotsNativeHex0, playerBotsNativeHex1, playerBotsNativeHex2, playerBotsNativeHex3,
        playerBotsNativeHex4, playerBotsNativeHex5, playerBotsNativeHex6, playerBotsNativeHex7,
        playerBotsNativeHex8, playerBotsNativeHex9, playerBotsNativeHexA, playerBotsNativeHexB,
        playerBotsNativeHexC, playerBotsNativeHexD, playerBotsNativeHexE, playerBotsNativeHexF,
        playerBotsNativeDigit0, playerBotsNativeDigit1, playerBotsNativeDigit2, playerBotsNativeDigit3,
        playerBotsNativeDigit4, playerBotsNativeDigit5, playerBotsNativeDigit6, playerBotsNativeDigit7,
        playerBotsNativeDigit8, playerBotsNativeDigit9,
    }
    for _, fn in ipairs(required) do
        if type(fn) ~= "function" then return false end
    end
    return true
end

local digitFns, hexFns

local function setupNativeTables()
    digitFns = {
        playerBotsNativeDigit0, playerBotsNativeDigit1, playerBotsNativeDigit2, playerBotsNativeDigit3,
        playerBotsNativeDigit4, playerBotsNativeDigit5, playerBotsNativeDigit6, playerBotsNativeDigit7,
        playerBotsNativeDigit8, playerBotsNativeDigit9,
    }
    hexFns = {
        playerBotsNativeHex0, playerBotsNativeHex1, playerBotsNativeHex2, playerBotsNativeHex3,
        playerBotsNativeHex4, playerBotsNativeHex5, playerBotsNativeHex6, playerBotsNativeHex7,
        playerBotsNativeHex8, playerBotsNativeHex9, playerBotsNativeHexA, playerBotsNativeHexB,
        playerBotsNativeHexC, playerBotsNativeHexD, playerBotsNativeHexE, playerBotsNativeHexF,
    }
end

local function sendNameNative(name)
    playerBotsNativeNameBegin()
    for i = 1, #name do
        local byte = name:byte(i)
        hexFns[math.floor(byte / 16) + 1]()
        hexFns[(byte % 16) + 1]()
    end
    playerBotsNativeNameCommit()
end

local function nativeSync(bots)
    nativeReady = nativeFunctionsAvailable()
    if not nativeReady then return false end
    setupNativeTables()

    local ok, err = pcall(function()
        playerBotsNativeASEDisable()

        playerBotsNativeNamesBegin()
        for _, bot in ipairs(bots) do sendNameNative(bot.name) end
        playerBotsNativeNamesCommit()

        local amount = #bots
        local formatted = string.format("%03d", amount)
        playerBotsNativeBeginCount()
        for i = 1, 3 do
            local d = tonumber(formatted:sub(i, i))
            digitFns[d + 1]()
        end
        playerBotsNativeCommitCount()

        if cfg.enabled and amount > 0 then playerBotsNativeASEEnable() end
    end)

    if not ok then
        nativeReady = false
        outputServerLog("[PlayerBots] Native sync failed: " .. tostring(err))
        return false
    end
    return true
end

local function exposeState(realPlayers, bots)
    local virtual = #bots
    local public = realPlayers + virtual
    local maxPlayers = getMaxPlayers()
    local visible = cfg.showTab and bots or {}

    setElementData(root, DATA_PREFIX .. "enabled", cfg.enabled, true)
    setElementData(root, DATA_PREFIX .. "realPlayers", realPlayers, true)
    setElementData(root, DATA_PREFIX .. "virtualPlayers", virtual, true)
    setElementData(root, DATA_PREFIX .. "publicPlayers", public, true)
    setElementData(root, DATA_PREFIX .. "maxPlayers", maxPlayers, true)
    setElementData(root, DATA_PREFIX .. "bots", visible, true)
    setElementData(root, "playerbots", visible, true)
    return public
end

local function syncRuntime()
    local realPlayers = getPlayerCount()
    local bots = buildBots(realPlayers)
    local public = exposeState(realPlayers, bots)
    nativeSync(bots)
    return bots, public
end

local function stateFor(player)
    if not isAuthorized(player) then return end
    local realPlayers = getPlayerCount()
    local bots = buildBots(realPlayers)
    local public = realPlayers + #bots
    triggerClientEvent(player, "playerbots:state", resourceRoot, {
        enabled = cfg.enabled,
        nativeReady = nativeReady,
        real = realPlayers,
        public = public,
        max = getMaxPlayers(),
        bots = bots,
        randomDatabaseCount = #(PLAYERBOTS_NAMES or {}),
        databaseNames = PLAYERBOTS_NAMES or {},
        settings = {
            target = cfg.target,
            showTab = cfg.showTab,
            nameMode = cfg.nameMode,
            customNamesText = customNamesText(),
            specificNames = cfg.specificNames,
            replaceEnabled = cfg.replaceEnabled,
            replaceAt = cfg.replaceAt,
        }
    })
end

local function refreshAdmins()
    for _, player in ipairs(getElementsByType("player")) do
        if isAuthorized(player) then stateFor(player) end
    end
end

local function notify(player, text, isError)
    if player and isElement(player) then
        triggerClientEvent(player, "playerbots:notice", resourceRoot, text, isError == true)
    end
end

addEvent("playerbots:request", true)
addEventHandler("playerbots:request", resourceRoot, function()
    if client then stateFor(client) end
end)

addEvent("playerbots:save", true)
addEventHandler("playerbots:save", resourceRoot, function(request)
    local player = client
    if not player or not isAuthorized(player) or type(request) ~= "table" then return end

    local maxPlayers = math.max(1, getMaxPlayers())
    local target = clampInt(request.target, 0, maxPlayers)
    local replaceAt = clampInt(request.replaceAt, 1, maxPlayers)
    if target == nil or replaceAt == nil then
        notify(player, "Invalid bot count or replacement threshold.", true)
        return
    end

    local mode = "random"
    if request.nameMode == "custom" then mode = "custom"
    elseif request.nameMode == "specific" then mode = "specific" end

    local custom = parseCustomNames(request.customNamesText)
    local specific = sanitizeSpecificNames(request.specificNames)
    local requiredNames = target
    if request.replaceEnabled == true then requiredNames = math.min(target, replaceAt) end

    if mode == "custom" and request.enabled == true and #custom < requiredNames then
        notify(player, string.format("Add at least %d custom names or reduce the bot count/replacement threshold.", requiredNames), true)
        return
    end
    if mode == "specific" and request.enabled == true and #specific < requiredNames then
        notify(player, string.format("Select at least %d names from names.lua. Selected: %d.", requiredNames, #specific), true)
        return
    end
    if mode == "random" and request.enabled == true and requiredNames > #(PLAYERBOTS_NAMES or {}) then
        notify(player, string.format("names.lua contains %d names. Add more names, reduce the count, or use custom names.", #(PLAYERBOTS_NAMES or {})), true)
        return
    end

    cfg.enabled = request.enabled == true
    cfg.target = target
    cfg.showTab = request.showTab ~= false
    cfg.nameMode = mode
    cfg.customNames = custom
    cfg.specificNames = specific
    cfg.replaceEnabled = request.replaceEnabled == true
    cfg.replaceAt = replaceAt

    if mode == "random" then rebuildRandomPool() end

    local saved = persist()
    syncRuntime()
    notify(player, saved and "Configuration applied and saved." or "Configuration applied, but settings.json could not be saved.", not saved)
    refreshAdmins()
end)

addCommandHandler("playerbots", function(player)
    if not player then
        local realPlayers = getPlayerCount()
        local bots = buildBots(realPlayers)
        outputServerLog(string.format("[PlayerBots] %s | real=%d virtual=%d public=%d/%d | mode=%s | replacement=%s",
            cfg.enabled and "ENABLED" or "DISABLED",
            realPlayers, #bots, realPlayers + #bots, getMaxPlayers(),
            cfg.nameMode,
            cfg.replaceEnabled and ("at " .. cfg.replaceAt) or "OFF"))
        return
    end
    if not isAuthorized(player) then return end
    triggerClientEvent(player, "playerbots:toggle", resourceRoot)
end, false, true)

addEventHandler("onPlayerJoin", root, function()
    setTimer(function()
        syncRuntime()
        refreshAdmins()
    end, 50, 1)
end)

addEventHandler("onPlayerQuit", root, function()
    setTimer(function()
        syncRuntime()
        refreshAdmins()
    end, 200, 1)
end)

addEventHandler("onResourceStart", resourceRoot, function()
    math.randomseed(getTickCount() + (getRealTime().timestamp or 0))
    restore()
    rebuildRandomPool()
    local bots, public = syncRuntime()
    outputServerLog(string.format("[PlayerBots] Started | native=%s | real=%d virtual=%d public=%d/%d",
        nativeReady and "ready" or "unavailable", getPlayerCount(), #bots, public, getMaxPlayers()))
end)

addEventHandler("onResourceStop", resourceRoot, function()
    if type(playerBotsNativeASEDisable) == "function" then pcall(playerBotsNativeASEDisable) end
    removeElementData(root, DATA_PREFIX .. "enabled")
    removeElementData(root, DATA_PREFIX .. "realPlayers")
    removeElementData(root, DATA_PREFIX .. "virtualPlayers")
    removeElementData(root, DATA_PREFIX .. "publicPlayers")
    removeElementData(root, DATA_PREFIX .. "maxPlayers")
    removeElementData(root, DATA_PREFIX .. "bots")
    removeElementData(root, "playerbots")
end)

function getPublicPlayerCount()
    return tonumber(getElementData(root, DATA_PREFIX .. "publicPlayers")) or getPlayerCount()
end

function getVirtualPlayerCount()
    return tonumber(getElementData(root, DATA_PREFIX .. "virtualPlayers")) or 0
end

function getVirtualPlayers()
    return getElementData(root, DATA_PREFIX .. "bots") or {}
end

function isPlayerBotsEnabled()
    return cfg.enabled == true
end
