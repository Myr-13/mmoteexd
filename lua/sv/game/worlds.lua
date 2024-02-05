Hook.Add("OnInit", "LoadWorlds", function()
	Game.GameServer.MultiWorldManager:LoadWorld("mmo")
	Game.GameServer.MultiWorldManager:LoadWorld("mmo_jungle")
end)
