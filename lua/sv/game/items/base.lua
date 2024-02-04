AddItem(0, TYPE_ITEM, "Wood", "...")
AddItem(1, TYPE_USABLE, "Stone", "...", function(CID)
	Server.SendChat(CID, "Huh")

	return true
end)
