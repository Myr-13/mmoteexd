-- Base lib
include("sh/base/hook.lua")
include("cl/base/draw.lua")

include("cl/hud/hud.lua")


function RegisterHook(Name)
	_G[Name] = function(...)
		local a = Hook.Call(Name, ...)

		if a then
			return a
		end
	end
end


for i = 0, 41 do
	RegisterHook("RenderLevel" .. tostring(i))
end
