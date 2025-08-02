#include "imgui_helpers.h"

#include <imgui.h>

//-------------------------------------------------------------------------------------------------
void drawVec3Prop(glm::vec3& val, std::string_view propName)
{
	const std::string imGuiIdX = std::format("##{}float3x", propName);
	const std::string imGuiIdY = std::format("##{}float3y", propName);
	const std::string imGuiIdZ = std::format("##{}float3z", propName);

	const float lettersSize = ImGui::CalcTextSize("XYZ").x;
	const float itemWidth = (ImGui::GetColumnWidth() / 3.0f) - lettersSize;

	bool valueChanged = false;
	ImGui::AlignTextToFramePadding();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.7f, 0.0f, 0.0f, 0.9f });
	ImGui::TextUnformatted("X");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdX.c_str(), &val.x, 0.1f);
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.7f, 0.0f, 0.9f });
	ImGui::TextUnformatted("Y");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdY.c_str(), &val.y, 0.1f);
	ImGui::PopItemWidth();

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.7f, 0.9f });
	ImGui::TextUnformatted("Z");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::PushItemWidth(itemWidth);
	valueChanged |= ImGui::DragFloat(imGuiIdZ.c_str(), &val.z, 0.1f);
	ImGui::PopItemWidth();
}
