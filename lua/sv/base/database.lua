DB = {}
DB._Query = ""

DB.Execute = function(Callback)
	local Name = "__DB_Callback"
	_G[Name] = Callback
	Engine.Database:Execute(Name)
end

DB.SimpleExecute = function(Query)
	DB._Query = Query
	local Name = "__DB_Simple_Callback_" .. tostring(hash(Query))
	_G[Name] = function(Stmt)
		if DB.Prepare(Stmt, DB._Query) then
			return
		end
		Stmt:Step()
	end
	Engine.Database:Execute(Name)
end

DB.Prepare = function(Stmt, Query)
	if Stmt:Prepare(Query) then
		print_ex("sqlite", "'", Query, "' has failed")
		print_ex("sqlite", "error: ", Stmt.ErrorMsg)

		return true
	end

	return false
end
