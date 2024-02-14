Account = {}
Account.Name = ""
Account.Level = 0
Account.Exp = 0
Account.NeedExp = 0
Account.Money = 0
Account.Health = 0
Account.MaxHealth = 1
Account.Mana = 0
Account.MaxMana = 1


Account.UpdateAccount = function(Data)
	for k, v in pairs(Data) do
		Account[k] = v
	end
end


Network.RegisterCallback("stats@acc", Account.UpdateAccount)
