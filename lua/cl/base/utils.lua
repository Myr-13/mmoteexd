Utils = {}


Utils.FormatNumber = function(Num)
	return tostring(Num):reverse():gsub("(%d%d%d)", "%1,"):reverse():gsub("^%,", "")
end


-- TODO: Rework color with C++ class
Utils.ColorMinus = function(Color1, Color2)
	return { Color1[1] - Color2[1], Color1[2] - Color2[2], Color1[3] - Color2[3] }
end
