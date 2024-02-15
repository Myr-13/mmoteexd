-- Add ./libs/ to package.path
do
	local Cwd = package.path
	local _, End

	_, End = string.find(Cwd, ";", 1)
	Cwd = string.sub(Cwd, End + 1, #Cwd)
	_, End = string.find(Cwd, ";", 1)
	Cwd = string.sub(Cwd, 1, End - 6)
	Cwd = Cwd .. "libs\\?.lua"

	package.path = package.path .. Cwd .. ";"
end


-- Include all files
include("sh/base/ex_math.lua")
include("sh/base/hook.lua")
include("cl/base/draw.lua")
include("cl/base/imgui_ex.lua")
include("cl/base/input.lua")
include("cl/base/network.lua")
include("cl/base/render_tools.lua")
include("cl/base/utils.lua")

include("cl/game/hud/hud.lua")
include("cl/game/hud/menus.lua")
include("cl/game/account.lua")
include("cl/game/resources.lua")
include("cl/game/stats.lua")


-- Do other stuff
function RegisterHook(Name)
	_G[Name] = function(...)
		local a = Hook.Call(Name, ...)

		if a then
			return a
		end
	end
end


-- Register hooks
for i = 0, 41 do
	RegisterHook("RenderLevel" .. tostring(i))
end

RegisterHook("OnMessage")
RegisterHook("OnShutdown")
RegisterHook("OnInput")

Hook.Call("OnInit")

-- Notify about mmo client
Network.SendMsg("client@main", {IsMMO = true})
