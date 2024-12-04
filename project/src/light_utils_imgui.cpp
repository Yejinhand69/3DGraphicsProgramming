#ifdef XBGT2094_ENABLE_IMGUI
#include "lighting/light_utils.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

// specific internal_imgui_drawControls selected by imgui_drawControls

void LightUtils::internal_imgui_drawControls(PointLight* light)
{
	// Two ways to pass glm variables:
	// 1. by passing address to first index
	//ImGui::ColorEdit3("##colour", &light->colour[0]);
	// 2. by using value_ptr
	ImGui::Text("Colour");
	ImGui::ColorEdit3("##colour", glm::value_ptr(light->colour));

	ImGui::Text("Position");
	ImGui::SliderFloat3("##position", &light->position[0], -10.0f, 10.0f);

	ImGui::Text("Range");
	ImGui::SliderFloat("##range", &light->range, 0.0f, 10.0f);

	ImGui::Text("Intensity");
	ImGui::SliderFloat("##intensity", &light->intensity, 0.0f, 10.0f);
}

void LightUtils::internal_imgui_drawControls(DirectionalLight* light)
{
	ImGui::Text("Colour");
	ImGui::ColorEdit3("##colour", &light->colour[0]);

	ImGui::Text("Intensity");
	ImGui::SliderFloat("##intensity", &light->intensity, 0.1f, 2.0f);

	ImGui::Text("Direction");
	ImGui::SliderFloat3("##DirLightDir", &light->direction[0], -1.0f, 1.0f);
}

void LightUtils::internal_imgui_drawControls(SpotLight* light)
{
	ImGui::Text("Colour");
	ImGui::ColorEdit3("##colour", &light->colour[0]);

	ImGui::Text("Position");
	ImGui::SliderFloat3("##position", &light->position[0], -5.0f, 5.0f);
}

#endif