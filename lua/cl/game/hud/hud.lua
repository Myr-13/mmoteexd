local ScreenAspect = 0


local function Convert(x)
	return x * ScreenAspect
end


local function SideBar()
	local x = Convert(270)
	local y = 300 / 2 - 50 / 2

	-- Background
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(x, y, Convert(30), 50)

	Draw.Text(x + 2, y + 2, 4, "Account: %s", Account.Name)
	Draw.Text(x + 2, y + 6, 4, "Level: %d", Account.Level)

	local Exp = Account.Exp
	local NeedExp = Account.NeedExp
	local Width = Convert(Exp / NeedExp * 25)

	-- Exp progress bar
	Draw.ClearTexture()
	Draw.SetColor(50, 255, 50)
	Draw.Rect(x + 2, y + 12, Width, 5)

	Draw.TextCentred(x + 2 + Width, y + 18, 3, "%d/%d", Exp, NeedExp)
	Draw.Text(x + 2, y + 23, 4, "Money: %d", Account.Money)
end


local function MainBar()
	local HPRatio = (Account.Health / Account.MaxHealth)
	local MaxWidth = Convert(40)
	local HPX = Convert(100)
	local HPWidth = MaxWidth * HPRatio
	local MPWidth = MaxWidth * (Account.Mana / Account.MaxMana)
	local MPX = Convert(200) - MPWidth
	local Offset = 2

	-- Health bar
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(HPX - Offset, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2)
	Draw.SetColor(Math.Lerp(50, 255, (1 - HPRatio)), 255 * HPRatio, 50 * HPRatio)
	Draw.Rect(HPX, 250, HPWidth, 10)

	Draw.TextCentred(HPX + HPWidth, 243, 5, "%d/%d", Account.Health, Account.MaxHealth)

	-- Mana bar
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(Convert(200) - MaxWidth - Offset * 2, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2)
	Draw.SetColor(50, 50, 200)
	Draw.Rect(MPX - Offset, 250, MPWidth, 10)

	Draw.TextCentred(MPX - Offset, 243, 5, "%d/%d", Account.Mana, Account.MaxMana)
end


Hook.Add("RenderLevel26", "MMOHud", function()
	ScreenAspect = Game.Graphics.ScreenAspect
	Game.Graphics:MapScreen(0, 0, 300 * ScreenAspect, 300)

	SideBar()
	MainBar()
end)
