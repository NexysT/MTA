-- PlayerBots | Nexys.Tuga | CEF panel
local guiBrowser, browser, panelOpen = nil, nil, false
local sx, sy = guiGetScreenSize()

local function jsCall(functionName, value, second)
    if not isElement(browser) then return end
    local args = toJSON(value)
    if second ~= nil then args = args .. "," .. toJSON(second) end
    executeBrowserJavascript(browser, "window." .. functionName .. "(" .. args .. ");")
end

local function closePanel()
    if not panelOpen then return end
    panelOpen = false
    if isElement(guiBrowser) then destroyElement(guiBrowser) end
    guiBrowser, browser = nil, nil
    guiSetInputEnabled(false)
    showCursor(false)
end

local function openPanel()
    if panelOpen then closePanel() return end

    guiBrowser = guiCreateBrowser(0, 0, sx, sy, true, false, false)
    if not isElement(guiBrowser) then
        outputChatBox("[PlayerBots] Could not open the panel.", 255, 110, 110)
        return
    end

    browser = guiGetBrowser(guiBrowser)
    if not isElement(browser) then
        destroyElement(guiBrowser)
        guiBrowser = nil
        outputChatBox("[PlayerBots] CEF initialization failed.", 255, 110, 110)
        return
    end

    panelOpen = true
    guiBringToFront(guiBrowser)
    showCursor(true)
    guiSetInputEnabled(true)

    addEventHandler("onClientBrowserCreated", browser, function()
        if panelOpen and source == browser then
            loadBrowserURL(source, "http://mta/local/ui/index.html")
        end
    end)

    addEventHandler("onClientBrowserDocumentReady", browser, function()
        if panelOpen and source == browser then
            focusBrowser(browser)
            triggerServerEvent("playerbots:request", resourceRoot)
        end
    end)
end

addEvent("playerbots:toggle", true)
addEventHandler("playerbots:toggle", resourceRoot, openPanel)

addEvent("playerbots:state", true)
addEventHandler("playerbots:state", resourceRoot, function(data)
    if panelOpen then jsCall("playerBotsReceive", data) end
end)

addEvent("playerbots:notice", true)
addEventHandler("playerbots:notice", resourceRoot, function(message, isError)
    if panelOpen then jsCall("playerBotsNotice", tostring(message), isError == true) end
end)

addEvent("playerbots:uiClose", true)
addEventHandler("playerbots:uiClose", root, function()
    if source == browser then closePanel() end
end)

addEvent("playerbots:uiRefresh", true)
addEventHandler("playerbots:uiRefresh", root, function()
    if source == browser then triggerServerEvent("playerbots:request", resourceRoot) end
end)

addEvent("playerbots:uiSave", true)
addEventHandler("playerbots:uiSave", root, function(payload)
    if source ~= browser or type(payload) ~= "string" then return end
    local parsed = fromJSON(payload)
    if type(parsed) == "table" then
        triggerServerEvent("playerbots:save", resourceRoot, parsed)
    end
end)

addEventHandler("onClientKey", root, function(key, down)
    if panelOpen and down and key == "escape" then
        closePanel()
        cancelEvent()
    end
end)

addEventHandler("onClientResourceStop", resourceRoot, closePanel)
