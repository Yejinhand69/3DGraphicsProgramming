// Include this header to use any gl* functions
#include <glad/glad.h>
#include "scene_asgn.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "framework/framework.h"
#include "renderable_entity.h"
#include <vector>
#include <algorithm>
#include <map>

#ifdef XBGT2094_ENABLE_IMGUI
#include "imgui/imgui.h"
#endif
#include "lighting/light_debug.h"


static Mesh* mesh_skybox;
static Shader* shader_skybox;
static Cubemap* cubemap_skybox;

static Shader* shader_floor;
static Shader* shader_house;
static Shader* shader_fan;
static Shader* shader_tree;
static Shader* shader_rocks;
static Shader* shader_water;
static Shader* shader_roadlamp;
static Shader* shader_lantern;
static Shader* shader_horse;
static Shader* shader_screen;

static DirectionalLight* dLight;
static PointLight* pLight;
static PointLight* pLight_Lantern01;
static PointLight* pLight_Lantern02;
static PointLight* pLight_Lantern03;
static PointLight* pLight_rainbow;

static std::vector<RenderableEntity*> entities_opaque;
static std::vector<RenderableEntity*> entities_alphatest;
static std::vector<RenderableEntity*> entities_alphablend;

//Lighting debug checkbox//
static bool enableDebug = false; //lighting debug

static bool enableDirectionalLight = true;
static bool enableRoadlamp = true;
static bool enableLanternLeft = true;
static bool enableLanternMid = true;
static bool enableLanternRight = true;
static bool enableRainbowLight = true;
//End of Booleans//

//HDR
static bool enableHDR = true;
static bool enableTonemap = true;
static bool enableExposure = true;
static bool enableContrast = true;
static bool enableSaturation = true;

static float exposure = 1.0f;
static float contrast = 1.5f;
static float saturation = 1.0f;

//Booleans post processing
static bool enablePostProcessing = false; //Post-processing
static bool enableSepia = true;
static bool enableFilmGrain = true;
static bool enableBadTVSignal = true;
static bool enableVignette = true;

//ColourDepthFBO
static ColourDepthFBO* fbo;

static void renderSkybox(CameraBase* camera)
{
	glDepthMask(GL_FALSE); // disable WRITING to depth buffer. Depth test STILL OCCURS.
	glDepthFunc(GL_LEQUAL);

	SimpleRenderer::bindShader(shader_skybox);

	SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
	SimpleRenderer::setShaderProp_Mat4("view", glm::mat4(glm::mat3(camera->getViewMatrix())));

	SimpleRenderer::setTexture_skybox(cubemap_skybox);
	SimpleRenderer::drawMesh(mesh_skybox);
	SimpleRenderer::setTexture_skybox(0);

	glDepthMask(GL_TRUE); // enable WRITING to depth buffer. Successful Depth test writes the new value to depth buffer.
	glDepthFunc(GL_LESS);
}

static void renderOpaques(CameraBase* camera)
{
	// Iterate through all opaque entities
	for (auto it : entities_opaque)
	{
		auto& entity = *it; // Alias *it as entity for readability purposes

		// 1. Bind the shader for this entity
		SimpleRenderer::bindShader(entity.shader);

		// 2. Set shader properties
		SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
		SimpleRenderer::setShaderProp_Mat4("view", camera->getViewMatrix());
		SimpleRenderer::setShaderProp_Mat4("model", entity.getModelMatrix());
		SimpleRenderer::setShaderProp_Vec3("cameraPosition", camera->getPosition());

		SimpleRenderer::setShaderProp_Vec2("resolution", App::getViewportSize());
		SimpleRenderer::setShaderProp_Float("time", App::getTime());

		SimpleRenderer::setShaderProp_Vec3("dirLightColour", dLight->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("dirLightDirection", dLight->getDirection());

		SimpleRenderer::setShaderProp_Vec3("pointLightColourRainbow", pLight_rainbow->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPositionRainbow", pLight_rainbow->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRangeRainbow", pLight_rainbow->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Vec3("pointLightColour0", pLight->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPosition0", pLight->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRange0", pLight->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Bool("enableDirectionalLight", enableDirectionalLight);
		SimpleRenderer::setShaderProp_Bool("enableRainbowLight", enableRainbowLight);

		SimpleRenderer::setShaderProp_Bool("enableHDR", enableHDR);
		SimpleRenderer::setShaderProp_Bool("enableTonemap", enableTonemap);
		SimpleRenderer::setShaderProp_Bool("enableExposure", enableExposure);
		SimpleRenderer::setShaderProp_Bool("enableContrast", enableContrast);
		SimpleRenderer::setShaderProp_Bool("enableSaturation", enableSaturation);

		SimpleRenderer::setShaderProp_Float("exposure", exposure);
		SimpleRenderer::setShaderProp_Float("contrast", contrast);
		SimpleRenderer::setShaderProp_Float("saturation", saturation);

		// 3. Set material properties of this entity
		SimpleRenderer::setTexture_0(entity.diffuseTex);
		SimpleRenderer::setTexture_1(entity.specularTex);

		// 4. draw the mesh of this entity
		SimpleRenderer::drawMesh(entity.mesh);

	}
}

static void renderAlphaTest(CameraBase* camera)
{
	// Iterate through all alpha-tested entities
	glDisable(GL_CULL_FACE);

	// Iterate through all alpha-tested entities
	for (auto it : entities_alphatest)
	{
		auto& entity = *it; // Alias *it as entity for readability purposes

		// 1. Bind the shader for this entity
		SimpleRenderer::bindShader(entity.shader);

		// 2. Set shader properties
		SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
		SimpleRenderer::setShaderProp_Mat4("view", camera->getViewMatrix());
		SimpleRenderer::setShaderProp_Mat4("model", entity.getModelMatrix());
		SimpleRenderer::setShaderProp_Vec3("cameraPosition", camera->getPosition());

		SimpleRenderer::setShaderProp_Vec2("resolution", App::getViewportSize());
		SimpleRenderer::setShaderProp_Float("time", App::getTime());

		SimpleRenderer::setShaderProp_Vec3("dirLightColour", dLight->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("dirLightDirection", dLight->getDirection());

		// 3. Set material properties of this entity
		SimpleRenderer::setTexture_0(entity.diffuseTex);
		SimpleRenderer::setTexture_1(entity.specularTex);
	

		// 4. draw the mesh of this entity
		SimpleRenderer::drawMesh(entity.mesh);

		
	}

	glEnable(GL_CULL_FACE);
}

static void renderAlphaBlends(CameraBase* camera)
{
	// Iterate through all alpha-blended entities
	std::sort(entities_alphablend.begin(), entities_alphablend.end(),
		[&camera](const RenderableEntity* el1, const RenderableEntity* el2)
		{
			//Calculate distance of et1 to the camera (et1_dist)
			float el1_dist = glm::length2(el1->position - camera->getPosition());

			//Calculate distance of et2 to the camera (et1_dist)
			float el2_dist = glm::length2(el2->position - camera->getPosition());

			return el1_dist > el2_dist;
		}
	);

	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Iterate through all alpha-blended entities
	for (auto it : entities_alphablend)
	{
		auto& entity = *it; // Alias *it as entity for readability purposes

		// 1. Bind the shader for this entity
		SimpleRenderer::bindShader(entity.shader);

		// 2. Set shader properties
		SimpleRenderer::setShaderProp_Mat4("projection", camera->getProjectionMatrix());
		SimpleRenderer::setShaderProp_Mat4("view", camera->getViewMatrix());
		SimpleRenderer::setShaderProp_Mat4("model", entity.getModelMatrix());

		SimpleRenderer::setShaderProp_Vec2("resolution", App::getViewportSize());
		SimpleRenderer::setShaderProp_Float("time", App::getTime());
		SimpleRenderer::setShaderProp_Vec3("cameraPosition", camera->getPosition());

		SimpleRenderer::setShaderProp_Vec3("pointLightColour0", pLight->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPosition0", pLight->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRange0", pLight->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Vec3("pointLightColour1", pLight_Lantern01->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPosition1", pLight_Lantern01->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRange1", pLight_Lantern01->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Vec3("pointLightColour2", pLight_Lantern02->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPosition2", pLight_Lantern02->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRange2", pLight_Lantern02->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Vec3("pointLightColour3", pLight_Lantern03->getColorIntensified());
		SimpleRenderer::setShaderProp_Vec3("pointLightPosition3", pLight_Lantern03->getPosition());
		SimpleRenderer::setShaderProp_Float("pointLightRange3", pLight_Lantern03->getInverseSquaredRange());

		SimpleRenderer::setShaderProp_Bool("enableRoadlamp", enableRoadlamp);
		SimpleRenderer::setShaderProp_Bool("enableLanternLeft", enableLanternLeft);
		SimpleRenderer::setShaderProp_Bool("enableLanternMid", enableLanternMid);
		SimpleRenderer::setShaderProp_Bool("enableLanternRight", enableLanternRight);

		// 3. Set material properties of this entity
		SimpleRenderer::setTexture_0(entity.diffuseTex);
		// 4. draw the mesh of this entity
		SimpleRenderer::drawMesh(entity.mesh);
	}
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
}


// preload() runs before loadShaders()
// Shader* variables are NOT initialized yet, referencing will not work!
void Scene_ASGN::preload()
{
	// Set to false to disable rendering grid and axes lines
	renderDebug = false;

	// Skybox loaded in preload() to declutter load()
	mesh_skybox = MeshUtils::makeSkybox();
	ShaderUtils::loadShader(&shader_skybox, "SKYBOX", "../assets/shaders/skybox.vert", "../assets/shaders/skybox.frag");
	cubemap_skybox = TextureUtils::loadCubemap("../assets/textures/skybox/galaxy", "jpg");
}

// loadShaders() run AFTER preload()
// It is also called when reload shader key is pressed (default F1)
void Scene_ASGN::loadShaders()
{
	ShaderUtils::loadShader(&shader_floor, "FLOOR", "../assets/shaders/standard.vert", "../assets/shaders/combined.frag");//original floor.frag-
	ShaderUtils::loadShader(&shader_house, "HOUSE", "../assets/shaders/standard.vert", "../assets/shaders/combined.frag");//original house.frag-
	ShaderUtils::loadShader(&shader_fan, "FAN", "../assets/shaders/house.vert", "../assets/shaders/combined.frag");//original house.frag-
	ShaderUtils::loadShader(&shader_tree, "TREE", "../assets/shaders/standard.vert", "../assets/shaders/combined.frag");//original tree.frag-
	ShaderUtils::loadShader(&shader_rocks, "ROCKS", "../assets/shaders/standard.vert", "../assets/shaders/combined.frag");//original rocks.frag-
	ShaderUtils::loadShader(&shader_water, "WATER", "../assets/shaders/standard.vert", "../assets/shaders/water.frag");//original water.frag
	ShaderUtils::loadShader(&shader_roadlamp, "ROADLAMP", "../assets/shaders/standard.vert", "../assets/shaders/roadlamp.frag");//original roadlamp.frag
	ShaderUtils::loadShader(&shader_lantern, "LANTERN", "../assets/shaders/standard.vert", "../assets/shaders/lantern.frag");//original lantern.frag
	ShaderUtils::loadShader(&shader_horse, "HORSE", "../assets/shaders/standard.vert", "../assets/shaders/combined.frag");//original horse.frag-
	ShaderUtils::loadShader(&shader_screen, "SCREEN", "../assets/shaders/screen.vert", "../assets/shaders/screen.frag");//original screen.frag

	SimpleRenderer::bindShader(shader_floor);
	SimpleRenderer::setShaderProp_Integer("texture_floor_diffuse", 0);
	SimpleRenderer::setShaderProp_Integer("texture_floor_normal", 1);

	SimpleRenderer::bindShader(shader_house);
	SimpleRenderer::setShaderProp_Integer("texture_house", 0);
	SimpleRenderer::setShaderProp_Integer("texture_house2", 1);

	SimpleRenderer::bindShader(shader_fan);
	SimpleRenderer::setShaderProp_Integer("texture_fan", 0);
	SimpleRenderer::setShaderProp_Integer("texture_fan2", 1);

	SimpleRenderer::bindShader(shader_tree);
	SimpleRenderer::setShaderProp_Integer("texture_tree_diffuse", 0);
	SimpleRenderer::setShaderProp_Integer("texture_tree_specular", 1);

	SimpleRenderer::bindShader(shader_rocks);
	SimpleRenderer::setShaderProp_Integer("texture_rocks_diffuse", 0);
	SimpleRenderer::setShaderProp_Integer("texture_rocks_specular", 1);
	SimpleRenderer::setShaderProp_Integer("texture_rocks_normal", 2);

	SimpleRenderer::bindShader(shader_water);
	SimpleRenderer::setShaderProp_Integer("opaqueTexture", 0);

	SimpleRenderer::bindShader(shader_roadlamp);
	SimpleRenderer::setShaderProp_Integer("texture_roadlamp_diffuse", 0);
	SimpleRenderer::setShaderProp_Integer("texture_roadlamp_specular", 1);

	SimpleRenderer::bindShader(shader_lantern);
	SimpleRenderer::setShaderProp_Integer("lantern_texture", 0);
	SimpleRenderer::setShaderProp_Integer("lantern_nouse", 1);
	SimpleRenderer::setShaderProp_Integer("lantern_texture_emissive", 2);

	SimpleRenderer::bindShader(shader_horse);
	SimpleRenderer::setShaderProp_Integer("horse_texture_diffuse", 0);
	SimpleRenderer::setShaderProp_Integer("horse_texture_specular", 1);

}

// load() runs AFTER loadShaders()
// Shader* variables are initialized and can be safely used.
void Scene_ASGN::load()
{
	
	// 1. Use factory function to create lights.
	//    The factory function creates the light and adds it to a list for debug drawing purposes
	
	// 2. Set the light properties
	//    Colour/Color	- Common
	//    Direction		- Directional Light
	//    Position		- Point Light, Spot Light
	//    Range			- Point Light, Spot Light
	//    Angle			- Spot Light\



	dLight = LightUtils::createDirectionalLight("Directional_Light");
	dLight->setColour(glm::vec3(1.0f));
	dLight->setDirection({ 0.0, -1.0, -1.0 });
	dLight->setIntensity(1.0f);
	
	pLight = LightUtils::createPointLight("Roadlamp_Point_Light");
	pLight->setColour({ 1.0f, 1.0f, 1.0f });
	pLight->setRange(0.9f);
	pLight->setPosition({ -8.0f, 6.5f, 0.0f });
	pLight->setIntensity(1.5f);

	pLight_Lantern01 = LightUtils::createPointLight("Lantern_Point_Light_01");
	pLight_Lantern01->setColour({ 1.0f, 1.0f, 0.1f });
	pLight_Lantern01->setRange(0.8f);
	pLight_Lantern01->setPosition({ 1.9f, 5.4f, -3.5f });
	pLight_Lantern01->setIntensity(2.5f);

	pLight_Lantern02 = LightUtils::createPointLight("Lantern_Point_Light_02");
	pLight_Lantern02->setColour({ 1.0f, 1.0f, 0.1f });
	pLight_Lantern02->setRange(0.8f);
	pLight_Lantern02->setPosition({ 9.0f, 5.6f, -5.1f });
	pLight_Lantern02->setIntensity(2.5f);

	pLight_Lantern03 = LightUtils::createPointLight("Lantern_Point_Light_03");
	pLight_Lantern03->setColour({ 1.0f, 1.0f, 0.1f });
	pLight_Lantern03->setRange(0.8f);
	pLight_Lantern03->setPosition({ 5.2f, 4.9f, 1.0f });
	pLight_Lantern03->setIntensity(2.5f);

	pLight_rainbow = LightUtils::createPointLight("Rainbow_Point_Light");
	pLight_rainbow->setColour({ 1.0f, 0.0f, 0.0f });
	pLight_rainbow->setRange(10.0f);
	pLight_rainbow->setPosition({ -3.0f, 2.0f, 4.8f });
	pLight_rainbow->setIntensity(1.0f);

	// 3. Create renderable entities for the scene.
	//
	//		Entity creation flow:
	//		1. Create new RenderableEntity
	//		2. Assign mesh
	//		3. Assign shader (shaders in loadShaders() are safe to use here!)
	//		4. Set material properties
	//		5. Set transformation properties
	//		6. Push to the relevant collection (opaque, alpha-test or alpha-blend)
	//
	// Note: it is your own responsibility to not insert the same entity multiple times.

	//----------------------Entities Separator----------------------//

	RenderableEntity* floorEntity = new RenderableEntity();
	floorEntity->mesh = MeshUtils::makePlane({ 20,20 }, { 10,10 }, { 5,5 });
	floorEntity->shader = shader_floor;
	floorEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/rocky_dirt_diffuse.png");
	floorEntity->normalTex = TextureUtils::loadTexture2D("../assets/textures/HPST 96 normal.png");
	entities_opaque.push_back(floorEntity);

	//----------------------Entities Separator----------------------//

	RenderableEntity* houseEntity = new RenderableEntity();
	houseEntity->mesh = MeshUtils::loadObjFile("../assets/models/Windmill Stand.obj");
	houseEntity->shader = shader_house;
	houseEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/windMill-text.jpg");
	houseEntity->position = glm::vec3(-5.0f, 0.0f, -5.0f);//Position 
	houseEntity->rotation = glm::vec3(0.0f, 20.0f, 0.0f);//Rotation
	houseEntity->scale = glm::vec3(0.01f, 0.01f, 0.01f);//Scale
	entities_opaque.push_back(houseEntity);

	RenderableEntity* houseFanEntity = new RenderableEntity();
	houseFanEntity->mesh = MeshUtils::loadObjFile("../assets/models/Windmill Fan.obj");
	houseFanEntity->shader = shader_fan;
	houseFanEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/windMill-text.jpg");
	houseFanEntity->position = glm::vec3(-5.0f, 0.0f, -5.0f);//Position 
	houseFanEntity->rotation = glm::vec3(0.0f, 20.0f, 0.0f);//Rotation
	houseFanEntity->scale = glm::vec3(0.01f, 0.01f, 0.01f);//Scale
	entities_opaque.push_back(houseFanEntity);

	//----------------------Entities Separator----------------------//

	RenderableEntity* treeEntity = new RenderableEntity();
	treeEntity->mesh = MeshUtils::loadObjFile("../assets/models/oak_leafless.obj");
	treeEntity->shader = shader_tree;
	treeEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/oakbark.jpg");
	treeEntity->specularTex = TextureUtils::loadTexture2D("../assets/textures/oakbark_burnt.jpg");
	treeEntity->position = glm::vec3(4.5f, 0.0f, -3.0f);//Position 
	treeEntity->rotation = glm::vec3(0.0f, -40.0f, 0.0f);//Rotation
	treeEntity->scale = glm::vec3(0.06f, 0.06f, 0.06f);//Scale
	entities_opaque.push_back(treeEntity);

	//----------------------Entities Separator----------------------//

	RenderableEntity* treeLeavesEntity = new RenderableEntity();
	treeLeavesEntity->mesh = MeshUtils::loadObjFile("../assets/models/oak.obj");
	treeLeavesEntity->shader = shader_tree;
	treeLeavesEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/oakleaf_fall.png");
	treeLeavesEntity->position = glm::vec3(4.5f, 0.0f, -3.0f);//Position 
	treeLeavesEntity->rotation = glm::vec3(0.0f, -40.0f, 0.0f);//Rotation
	treeLeavesEntity->scale = glm::vec3(0.06f, 0.06f, 0.06f);//Scale
	entities_opaque.push_back(treeLeavesEntity);

	//----------------------Entities Separator----------------------//

	const int numRocks = 10;
	const float radius = 2.4f; // Radius of the circle
	const float angleIncrement = 360.0f / numRocks;

	// Preset rotations for each rock
	std::vector<float> presetRotations = {
		30.0f, 45.0f, 60.0f, 35.0f, 60.0f,
		45.0f, 30.0f, 47.0f, 55.0f, 66.0f,
	};

	for (int i = 0; i < numRocks; ++i) {
		float angle = glm::radians(i * angleIncrement);
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		// Use preset rotations for each rock
		float presetRotationY = presetRotations[i % presetRotations.size()];

		RenderableEntity* rocksEntity = new RenderableEntity();
		rocksEntity->mesh = MeshUtils::loadObjFile("../assets/models/rock_02.obj");
		rocksEntity->shader = shader_rocks;
		rocksEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/diffuse.png");
		rocksEntity->specularTex = TextureUtils::loadTexture2D("../assets/textures/specular.png");
		rocksEntity->position = glm::vec3(x - 3, 0.0f, z + 5); // Position in a circle
		rocksEntity->rotation = glm::vec3(0.0f, presetRotationY, 0.0f); // Rotation
		rocksEntity->scale = glm::vec3(0.015f, 0.015f, 0.015f); // Scale
		entities_opaque.push_back(rocksEntity);
	}

	//----------------------Entities Separator----------------------//

	RenderableEntity* waterEntity = new RenderableEntity();
	waterEntity->mesh = MeshUtils::makeDisk(2.2f, 30.0f);
	waterEntity->shader = shader_water;
	waterEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/distort.png");
	waterEntity->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);//Rotation
	waterEntity->position = glm::vec3(-3.0f, 0.2f, 5.0f); // Position in a circle
	entities_alphablend.push_back(waterEntity);

	//----------------------Entities Separator----------------------//

	RenderableEntity* roadlampEntity = new RenderableEntity();
	roadlampEntity->mesh = MeshUtils::loadObjFile("../assets/models/StreetLamp.obj");
	roadlampEntity->shader = shader_roadlamp;
	roadlampEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/lamp.png");
	roadlampEntity->position = glm::vec3(-8.0f, 1.7f, 0.0f);//Position 
	roadlampEntity->rotation = glm::vec3(0.0f, 0.0f, 0.0f);//Rotation
	roadlampEntity->scale = glm::vec3(0.4f, 0.4f, 0.4f);//Scale
	entities_alphablend.push_back(roadlampEntity);

	//----------------------Entities Separator----------------------//
	
	RenderableEntity* lantern01Entity = new RenderableEntity();
	lantern01Entity->mesh = MeshUtils::loadObjFile("../assets/models/FabConvert.com_lamp.obj-re_2kPjeweheJb8nWtNllnHejl4oz1.obj");
	lantern01Entity->shader = shader_lantern;
	lantern01Entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_2755070454-albedo-re_2kYGC2nxUvNbRbI3YYB1xQ41YuI.png");
	lantern01Entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_1312867562-albedo-re_2kPjetQ92ldoK0XmZarNCW0SIuN.png");
	lantern01Entity->position = glm::vec3(1.9f, 4.5f, -3.5);//Position 
	lantern01Entity->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);//Rotation
	lantern01Entity->scale = glm::vec3(0.5f,0.5f, 0.5f);//Scale
	entities_alphablend.push_back(lantern01Entity);
	
	RenderableEntity* lantern02Entity = new RenderableEntity();
	lantern02Entity->mesh = MeshUtils::loadObjFile("../assets/models/FabConvert.com_lamp.obj-re_2kPjeweheJb8nWtNllnHejl4oz1.obj");
	lantern02Entity->shader = shader_lantern;
	lantern02Entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_2755070454-albedo-re_2kYGC2nxUvNbRbI3YYB1xQ41YuI.png");
	lantern02Entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_1312867562-albedo-re_2kPjetQ92ldoK0XmZarNCW0SIuN.png");
	lantern02Entity->position = glm::vec3(9.0f, 4.7f, -5.1);//Position 
	lantern02Entity->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);//Rotation
	lantern02Entity->scale = glm::vec3(0.5f, 0.5f, 0.5f);//Scale
	entities_alphablend.push_back(lantern02Entity);

	RenderableEntity* lantern03Entity = new RenderableEntity();
	lantern03Entity->mesh = MeshUtils::loadObjFile("../assets/models/FabConvert.com_lamp.obj-re_2kPjeweheJb8nWtNllnHejl4oz1.obj");
	lantern03Entity->shader = shader_lantern;
	lantern03Entity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_2755070454-albedo-re_2kYGC2nxUvNbRbI3YYB1xQ41YuI.png");
	lantern03Entity->emissiveTex = TextureUtils::loadTexture2D("../assets/textures/mat3-seed_1312867562-albedo-re_2kPjetQ92ldoK0XmZarNCW0SIuN.png");
	lantern03Entity->position = glm::vec3(5.2f, 4.3f, 1.0);//Position 
	lantern03Entity->rotation = glm::vec3(-90.0f, 0.0f, 0.0f);//Rotation
	lantern03Entity->scale = glm::vec3(0.4f, 0.4f, 0.4f);//Scale
	entities_alphablend.push_back(lantern03Entity);

	//----------------------Entities Separator----------------------//

	RenderableEntity* horseEntity = new RenderableEntity();
	horseEntity->mesh = MeshUtils::loadObjFile("../assets/models/LD_HorseRtime02.obj");
	horseEntity->shader = shader_horse;
	horseEntity->diffuseTex = TextureUtils::loadTexture2D("../assets/textures/HorseMain2k00.png");
	horseEntity->specularTex = TextureUtils::loadTexture2D("../assets/textures/HorseMain2k00AO00.png");
	//horseEntity->specularTex = TextureUtils::loadTexture2D("../assets/textures/eye_texture.png");
	
	horseEntity->position = glm::vec3(6.0f, 2.15f, 5.0);//Position 
	horseEntity->rotation = glm::vec3(0.0f, -90.0f, 0.0f);//Rotation
	horseEntity->scale = glm::vec3(0.45f, 0.45f, 0.45f);//Scale
	entities_opaque.push_back(horseEntity);

	//----------------------Entities Separator----------------------//

	// Example
	// RenderableEntity* et1 = new RenderableEntity();
	// et1->mesh = ...;
	// et1->shader = ...;
	// et1->diffuseTex = TextureUtils::loadTexture2D("...", ...);
	// et1->specularTex = TextureUtils::loadTexture2D("...", ...);
	// et1->normalTex = TextureUtils::loadTexture2D("...", ...);
	// et1->emissiveTex = TextureUtils::loadTexture2D("...", ...);
	// et1->shininess = ...;
	// 
	// entities_opaque.push_back(et1);
	// 
	//Buffers
	ColourDepthFrameBufferConfig cfg;
	cfg.size = App::getViewportSize();
	cfg.depthFormat = DepthFormat::FLOAT24;

	cfg.colourAttachments.push_back(ColourAttachmentData(ColourFormat::RGBA, TextureFilterMode::LINEAR));

	fbo = FBOUtils::createColourDepthFBO(cfg);
}

void Scene_ASGN::update()
{
	// Get your renderable entity by array indexing
	// and then do update on its position, rotation, etc
	//
	//auto& et = entities_opaque[0];
	//et->position.x = ...;
	
}



void Scene_ASGN::draw(CameraBase* camera)
{
	if (enablePostProcessing)
	{
		//Use the custom fbo
		SimpleRenderer::bindFBO(fbo);
		//Clear the custom fbo before drawing scene, otherwise drawing may fail because depth test fails
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	glEnable(GL_DEPTH_TEST);

	renderOpaques(camera);
	renderSkybox(camera);
	renderAlphaTest(camera);
	renderAlphaBlends(camera);

	//Debug lighting
	if(enableDebug)
	LightDebug::draw(camera);
}

//Uniforms to tweak theme post procesing effect//
static float filmGrainAmount = 0.4f;
static float tvEffectStrength = 0.4f;
static float vignettePower = 1.0f;

void Scene_ASGN::postDraw(CameraBase* camera)
{
	//Two ways to bind
	if (enablePostProcessing)
	{
		SimpleRenderer::bindFBO_Default();

		//Draw a quad covering fulls screen
		//1.Create a quad
		//Leverage C++ static local variable
		static Mesh* fsQuad = MeshUtils::makeQuad(2.0f);

		//2.Bind screen shader
		SimpleRenderer::bindShader(shader_screen);

		SimpleRenderer::setShaderProp_Vec2("cursor", App::getMousePosition());
		SimpleRenderer::setShaderProp_Vec2("resolution", App::getViewportSize());
		SimpleRenderer::setShaderProp_Float("time", App::getTime());

		SimpleRenderer::setShaderProp_Bool("enableSepia", enableSepia);
		SimpleRenderer::setShaderProp_Bool("enableFilmGrain", enableFilmGrain);
		SimpleRenderer::setShaderProp_Bool("enableBadTVSignal", enableBadTVSignal);
		SimpleRenderer::setShaderProp_Bool("enableVignette", enableVignette);

		SimpleRenderer::setShaderProp_Float("filmGrainAmount", filmGrainAmount);
		SimpleRenderer::setShaderProp_Float("tvEffectStrength", tvEffectStrength);
		SimpleRenderer::setShaderProp_Float("vignettePower", vignettePower);

		//3.Bind fbo colour texture to texture unit 0
		SimpleRenderer::setTexture_0(fbo->getColorAttachment(0));

		//4.Draw the quad
		SimpleRenderer::drawMesh(fsQuad);
	}
}

void Scene_ASGN::onFrameBufferResized(int width, int height)
{
}

#ifdef XBGT2094_ENABLE_IMGUI
void Scene_ASGN::imgui_draw()
{
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Assignment");
	ImGui::Separator();
	ImGui::Text("Name: Ervin Chew Wei Juan");
	ImGui::Text("ID : 0134674");
	ImGui::Separator();

	//Post processing section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.4f, 0.2f, 0.5f));
	ImGui::BeginChild("Post Processing Section", ImVec2(0, 300), true);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Post-Processing Controls");
	ImGui::Text("-Medieval Vintage/Nostalgic-");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Post Processing Button", &enablePostProcessing);
	ImGui::PopStyleColor();
	ImGui::Checkbox("Medieval_Sepia", &enableSepia);

	ImGui::Checkbox("Film_Grain", &enableFilmGrain);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "    Grain Amount");
	ImGui::SliderFloat("SliderP1", &filmGrainAmount, 0.0f, 1.0f);

	ImGui::Checkbox("Bad_TV_Signal", &enableBadTVSignal);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "   Signal Strength");
	ImGui::SliderFloat("SliderP2", &tvEffectStrength, 0.0f, 1.0f);

	ImGui::Checkbox("Vignette", &enableVignette);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "   Vignette Power");
	ImGui::SliderFloat("SliderP3", &vignettePower, 0.5f, 1.5f);

	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	ImGui::Separator();

	//Renderer feature section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.4f, 0.2f, 0.5f));
	ImGui::BeginChild("Renderer Feature Section", ImVec2(0, 300), true);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Renderer Feature Controls");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("HDR Button", &enableHDR);
	ImGui::PopStyleColor();

	ImGui::Checkbox("Tone-mapping", &enableTonemap);

	ImGui::Checkbox("Exposure", &enableExposure);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "      Exposure");
	ImGui::SliderFloat("SliderR1", &exposure, 0.0f, 3.0f);

	ImGui::Checkbox("Contrast", &enableContrast);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "      Contrast");
	ImGui::SliderFloat("SliderR2", &contrast, 0.0f, 3.0f);

	ImGui::Checkbox("Saturation", &enableSaturation);
	ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "      Saturation");
	ImGui::SliderFloat("SliderR3", &saturation, 0.0f, 3.0f);

	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	ImGui::Separator();

	// Put your widget codes here
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Lighting Controls");

	ImGui::Checkbox("Lighting Debug", &enableDebug);

	// Directional Light section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 0.1f));
	ImGui::BeginChild("Directional Light Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Directional Light", &enableDirectionalLight);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(dLight);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	//Road Lamp section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.5f, 0.0f, 0.1f));
	ImGui::BeginChild("Road Lamp Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Road Lamp", &enableRoadlamp);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(pLight);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	//Lantern left section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 0.0f, 0.1f));
	ImGui::BeginChild("Lantern Left Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Lantern Left", &enableLanternLeft);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(pLight_Lantern01);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	//Lantern mid section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 1.0f, 0.0f, 0.1f));
	ImGui::BeginChild("Lantern Mid Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Lantern Mid", &enableLanternMid);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(pLight_Lantern03);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	//Lantern right section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 1.0f, 0.1f));
	ImGui::BeginChild("Lantern Right Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Lantern Right", &enableLanternRight);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(pLight_Lantern02);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	//Rainbow light section
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 1.0f, 0.1f));
	ImGui::BeginChild("Rainbow Light Section", ImVec2(0, 250), true);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f));
	ImGui::Checkbox("Rainbow Light", &enableRainbowLight);
	ImGui::PopStyleColor();
	LightUtils::imgui_drawControls(pLight_rainbow);
	ImGui::EndChild();
	ImGui::PopStyleColor(); // Restore the previous style color

	ImGui::Separator();

}
#endif