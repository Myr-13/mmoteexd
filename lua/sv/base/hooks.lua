Hook = {}
Hook._Hooks = {}


Hook.Add = function(EventName, Name, Callback)
	if Hook._Hooks[EventName] == nil then
		Hook._Hooks[EventName] = {}
	end

	Hook._Hooks[EventName][Name] = Callback
end


Hook.Call = function(EventName, ...)
	if Hook._Hooks[EventName] == nil then
		return
	end

	for _, Func in pairs(Hook._Hooks[EventName]) do
		local a = Func(...)

		if a ~= nil then
			return a
		end
	end
end
