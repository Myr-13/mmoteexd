Account = {}
Account.Name = ""
Account.Level = 0
Account.Exp = 0
Account.NeedExp = 1
Account.Money = 0
Account.Health = 0
Account.MaxHealth = 1
Account.Mana = 0
Account.MaxMana = 1
Account.UpgradePoints = 0
Account.Stats = {}


Account.UpdateAccount = function(Data)
	for k, v in pairs(Data) do
		Account[k] = v
	end
end


Account.UpdateStats = function(Data)
	Account.Stats = Data
end


Network.RegisterCallback("account_info@acc", Account.UpdateAccount)
Network.RegisterCallback("stats@acc", Account.UpdateStats)
