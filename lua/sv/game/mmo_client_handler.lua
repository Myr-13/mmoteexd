Network.RegisterCallback("upgrade@stats", function(Data, CID)
	UpgradeStat(CID, Data.ID)
end)


Hook.Add("OnPlayerLoggedIn", "SendInventory", function(CID)
	local Inv = Player.GetData(CID, "Inventory")
	local SendInv = {}

	for k, v in pairs(Inv) do
		SendInv[tostring(k)] = v
	end

	Network.SendMsg(CID, "inv@acc", SendInv)
end)

Hook.Add("OnPlayerLoggedIn", "SendItems", function(CID)
	
end)
