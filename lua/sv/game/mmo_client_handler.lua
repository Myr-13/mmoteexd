Network.RegisterCallback("upgrade@stats", function(Data, CID)
	UpgradeStat(CID, Data.ID)
end)
