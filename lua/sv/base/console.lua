Console = {}

CFGFLAG_SAVE = 1
CFGFLAG_CLIENT = 2
CFGFLAG_SERVER = 4
CFGFLAG_STORE = 8
CFGFLAG_MASTER = 16
CFGFLAG_ECON = 32

CMDFLAG_TEST = 64
CFGFLAG_CHAT = 128
CFGFLAG_GAME = 256
CFGFLAG_NONTEEHISTORIC = 512
CFGFLAG_COLLIGHT = 1024
CFGFLAG_COLALPHA = 2048
CFGFLAG_INSENSITIVE = 4096


Console.RegisterEx = function(Name, Params, Flags, Help, Callback)
	local CallbackName = "__CON_Callback_" .. Name
	_G[CallbackName] = Callback
	Engine.Console:Register(Name, Params, Flags, CallbackName, Help)
end


Console.RegisterRcon = function(Name, Params, Help, Callback)
	Console.RegisterEx(Name, Params, CFGFLAG_SERVER, Help, Callback)
end


Console.RegisterChat = function(Name, Params, Help, Callback)
	Console.RegisterEx(Name, Params, CFGFLAG_SERVER + CFGFLAG_CHAT, Help, Callback)
end
