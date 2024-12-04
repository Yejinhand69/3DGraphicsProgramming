#include "renderable_entity.h"
#include <glm/gtc/matrix_transform.hpp>

RenderableEntity::RenderableEntity() : mesh(0), shader(0) {
	diffuseTex = TextureUtils::checkerTexture2D();
	specularTex = TextureUtils::whiteTexture2D();
	normalTex = TextureUtils::whiteTexture2D();
	emissiveTex = TextureUtils::blackTexture2D();	// Set blank texture for safety

}

glm::mat4 RenderableEntity::getModelMatrix() const {
	return glm::translate(glm::mat4(1.0), position)				// Translate last
		* glm::toMat4(glm::quat(glm::radians(rotation)))		// Rotation second; Make quaternion with rotation in radians, and then convert to mat4
		* glm::scale(glm::mat4(1.0), scale);					// Scale first
}