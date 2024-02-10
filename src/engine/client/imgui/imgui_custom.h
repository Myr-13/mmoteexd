#ifndef ENGINE_CLIENT_IMGUI_IMGUI_CUSTOM_H
#define ENGINE_CLIENT_IMGUI_IMGUI_CUSTOM_H

#include "imgui.h"

namespace ImGui
{

bool ButtonFull(const char *pText, float SizeY = 0)
{
	ImVec2 WinSize = ImGui::GetWindowSize();
	return ImGui::Button(pText, ImVec2(WinSize.x - 17, SizeY));
}

}

#endif // ENGINE_CLIENT_IMGUI_IMGUI_CUSTOM_H
