local function WtfCommand(Result)
	local CID = Result.ClientID
	
	Server.SendChat(CID, "Use /register [name] [password] [password] to start playing")
	Server.SendChat(CID, "Replace [name] and [password] with your name and password that you want to use")
	Server.SendChat(CID, "To login use /login [name] [password]")
	Server.SendChat(CID, "Join to our discord server to get help and follow updates")
	Server.SendChat(CID, "https://discord.gg/jwqYQGCm3d")
end

local function RegisterCommands()
	Console.RegisterChat("wtf", "", "", WtfCommand)
end

Hook.Add("OnConsoleInit", "GeneralCommands", RegisterCommands)
