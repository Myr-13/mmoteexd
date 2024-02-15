ImGuiEx = {}


ImGuiEx.Text = function(Text, ...)
	ImGui._LabelText("", string.format(Text, ...))
end


ImGuiEx.BulletText = function(Text, ...)
	ImGui._BulletText(string.format(Text, ...))
end


ImGuiEx.ButtonEx = function(Size, Text, ...)
	return ImGui._Button(string.format(Text, ...), Size)
end


ImGuiEx.Button = function(Text, ...)
	return ImGuiEx.ButtonEx(vec2(0, 0), Text, ...)
end
