#include <Core/Application.h>
#include <Core/Asset.h>
#include <GfxFramework/GfxFramework.h>
#include <MeshRenderer/MeshRendererSystem.h>
#include <MaterialSystem/MaterialSystem.h>
#include <TextureSystem/TextureSystem.h>
#include <Windows.h>
#include <fstream>

class Level {
public:
	void startLoad(const char* filePath) {
		// IOì«Ç›éÊÇËèâä˙âª
		_fin = std::ifstream(filePath, std::ios::in | std::ios::binary);
		_fin.exceptions(std::ios::badbit);
		LTN_ASSERT(!_fin.fail());

		MeshRendererSystem* meshSystem = MeshRendererSystem::Get();
		MaterialSystem* materialSystem = MaterialSystem::Get();
		TextureSystem* textureSystem = TextureSystem::Get();


		constexpr u32 BIN_HEADER_LENGTH = 4;
		u32 assetCounter = 0;


		// header
		{
			char meshHeader[BIN_HEADER_LENGTH];
			_fin.read(meshHeader, BIN_HEADER_LENGTH);
			_fin.read(reinterpret_cast<char*>(&_levelHeader), sizeof(LevelHeader));
			LTN_ASSERT(strcmp(meshHeader, "RESH"));
		}

		_meshes.initialize(_levelHeader._meshPathCount);
		_meshInstances.initialize(_levelHeader._meshInstanceCount);
		_textures.initialize(_levelHeader._textureParameterCount);
		_materials.initialize(_levelHeader._materialCount);
		_shaderSets.initialize(_levelHeader._shaderSetCount);
		_assets.initialize(_levelHeader._meshPathCount + _levelHeader._textureParameterCount);

		// meshes
		{
			char meshHeader[BIN_HEADER_LENGTH];
			_fin.read(meshHeader, BIN_HEADER_LENGTH);
			LTN_ASSERT(strcmp(meshHeader, "MESH"));

			for (u32 meshIndex = 0; meshIndex < _levelHeader._meshPathCount; ++meshIndex) {
				u32 filePathLength = 0;
				_fin.read(reinterpret_cast<char*>(&filePathLength), sizeof(u32));

				char filePath[FILE_PATH_COUNT_MAX] = {};
				_fin.read(filePath, filePathLength);

				MeshDesc desc = {};
				desc._filePath = filePath;
				_meshes[meshIndex] = meshSystem->allocateMesh(desc);
				_assets[assetCounter++] = _meshes[meshIndex]->getAsset();
			}
		}

		// textures
		{
			char meshHeader[BIN_HEADER_LENGTH];
			_fin.read(meshHeader, BIN_HEADER_LENGTH);
			LTN_ASSERT(strcmp(meshHeader, "TEX "));

			for (u32 textureIndex = 0; textureIndex < _levelHeader._textureParameterCount; ++textureIndex) {
				u32 filePathLength = 0;
				_fin.read(reinterpret_cast<char*>(&filePathLength), sizeof(u32));

				char filePath[FILE_PATH_COUNT_MAX] = {};
				_fin.read(filePath, filePathLength);

				TextureDesc desc = {};
				desc._filePath = filePath;
				_textures[textureIndex] = textureSystem->allocateTexture(desc);
				_assets[assetCounter++] = _textures[textureIndex]->getAsset();
			}
		}

		// shader sets
		{
			char shaderHeader[BIN_HEADER_LENGTH];
			_fin.read(shaderHeader, BIN_HEADER_LENGTH);
			LTN_ASSERT(strcmp(shaderHeader, "SSET"));

			for (u32 shaderIndex = 0; shaderIndex < _levelHeader._shaderSetCount; ++shaderIndex) {
				u32 filePathLength = 0;
				_fin.read(reinterpret_cast<char*>(&filePathLength), sizeof(u32));

				char filePath[FILE_PATH_COUNT_MAX] = {};
				_fin.read(filePath, filePathLength);

				ShaderSetDesc desc = {};
				desc._filePath = filePath;
				_shaderSets[shaderIndex] = materialSystem->createShaderSet(desc);
			}
		}

		// material instances
		{
			char meshHeader[BIN_HEADER_LENGTH];
			_fin.read(meshHeader, BIN_HEADER_LENGTH);
			LTN_ASSERT(strcmp(meshHeader, "MATI"));

			for (u32 materialIndex = 0; materialIndex < _levelHeader._materialCount; ++materialIndex) {
				u32 filePathLength = 0;
				_fin.read(reinterpret_cast<char*>(&filePathLength), sizeof(u32));

				char filePath[FILE_PATH_COUNT_MAX] = {};
				_fin.read(filePath, filePathLength);

				MaterialDesc desc = {};
				desc._filePath = filePath;
				_materials[materialIndex] = materialSystem->createMaterial(desc);
			}
		}


		// mesh instances
		{
			char meshInstanceHeader[BIN_HEADER_LENGTH];
			_fin.read(meshInstanceHeader, BIN_HEADER_LENGTH);
			LTN_ASSERT(strcmp(meshHeader, "MESI"));
		}

		_assetCount = assetCounter;
	}

	void doStream() {
		if (!_isInitialized) {
			_isInitialized = true;
			return;
		}

		if (_assetStreamingCounter < _assetCount) {
			u32 currentAssetIndex = _assetStreamingCounter++;
			_assets[currentAssetIndex]->requestLoad();
		}

		constexpr u32 LOAD_COUNT = 10;
		u32 meshInstanceCount = min(_levelHeader._meshInstanceCount - _meshInstanceStreamingCounter, LOAD_COUNT);
		if (meshInstanceCount > 0) {
			const Mesh* meshes[LOAD_COUNT];
			Matrix4 worldMatrices[LOAD_COUNT];
			u32 materialInstanceCounts[LOAD_COUNT];
			u32 materialInstanceOffsets[LOAD_COUNT] = {};
			u32 materialPathIndices[256];

			for (u32 i = 0; i < meshInstanceCount; ++i) {
				Vector4 wv[3] = {};
				u32 meshPathIndex = 0;
				u32 materialInstanceCount = 0;
				u32 materialInstanceOffset = materialInstanceOffsets[i];
				_fin.read(reinterpret_cast<char*>(&meshPathIndex), sizeof(u32));
				_fin.read(reinterpret_cast<char*>(&wv), sizeof(Vector4) * 3);
				_fin.read(reinterpret_cast<char*>(&materialInstanceCount), sizeof(u32));
				_fin.read(reinterpret_cast<char*>(&materialPathIndices[materialInstanceOffset]), sizeof(u32) * materialInstanceCount);

				meshes[i] = _meshes[meshPathIndex];
				materialInstanceCounts[i] = materialInstanceCount;
				worldMatrices[i] = Matrix4(
					wv[0].x, wv[1].x, wv[2].x, 0.0f,
					wv[0].y, wv[1].y, wv[2].y, 0.0f,
					wv[0].z, wv[1].z, wv[2].z, 0.0f,
					wv[0].w, wv[1].w, wv[2].w, 1.0f);
				LTN_ASSERT(materialInstanceOffset + materialInstanceCount < LTN_COUNTOF(materialPathIndices));
				if (i < meshInstanceCount - 1) {
					materialInstanceOffsets[i + 1] = materialInstanceOffset + materialInstanceCount;
				}
			}

			MeshRendererSystem* meshSystem = MeshRendererSystem::Get();
			MeshInstanceDesc desc = {};
			desc._meshes = meshes;
			desc._instanceCount = meshInstanceCount;
			meshSystem->createMeshInstance(&_meshInstances[_meshInstanceStreamingCounter], desc);

			for (u32 i = 0; i < meshInstanceCount; ++i) {
				MeshInstance* meshInstance = _meshInstances[_meshInstanceStreamingCounter + i];
				meshInstance->setWorldMatrix(worldMatrices[i]);

				u32 materialInstanceCount = materialInstanceCounts[i];
				for (u32 materialInstanceIndex = 0; materialInstanceIndex < materialInstanceCount; ++materialInstanceIndex) {
					u32 materialPathOffset = materialInstanceOffsets[i] + materialInstanceIndex;
					u32 materialPathIndex = materialPathIndices[materialPathOffset];
					Material* material = _materials[materialPathIndex];
					meshInstance->setMaterialSlotIndex(material, materialInstanceIndex);
				}
			}
			_meshInstanceStreamingCounter += meshInstanceCount;
		}

		if (isCompletedStream()) {
			_fin.close();
		}
	}

	void unload() {
		for (u32 meshIndex = 0; meshIndex < _levelHeader._meshPathCount; ++meshIndex) {
			_meshes[meshIndex]->requestToDelete();
		}

		for (u32 textureIndex = 0; textureIndex < _levelHeader._textureParameterCount; ++textureIndex) {
			_textures[textureIndex]->requestToDelete();
		}

		for (u32 shaderIndex = 0; shaderIndex < _levelHeader._shaderSetCount; ++shaderIndex) {
			_shaderSets[shaderIndex]->requestToDelete();
		}

		for (u32 materialIndex = 0; materialIndex < _levelHeader._materialCount; ++materialIndex) {
			_materials[materialIndex]->requestToDelete();
		}

		for (u32 meshIndex = 0; meshIndex < _levelHeader._meshInstanceCount; ++meshIndex) {
			_meshInstances[meshIndex]->requestToDelete();
		}

		_meshes.terminate();
		_meshInstances.terminate();
		_textures.terminate();
		_materials.terminate();
		_shaderSets.terminate();
		_assets.terminate();
	}

	bool isCompletedStream() const {
		return _meshInstanceStreamingCounter == _levelHeader._meshInstanceCount;
	}

private:
	struct LevelHeader {
		u32 _meshPathCount = 0;
		u32 _textureParameterCount = 0;
		u32 _shaderSetCount = 0;
		u32 _materialCount = 0;
		u32 _meshInstanceCount = 0;
	};

	DynamicArray<Mesh*> _meshes;
	DynamicArray<MeshInstance*> _meshInstances;
	DynamicArray<Texture*> _textures;
	DynamicArray<Material*> _materials;
	DynamicArray<ShaderSet*> _shaderSets;
	DynamicArray<Asset*> _assets;
	u32 _assetCount = 0;
	u32 _assetStreamingCounter = 0;
	u32 _meshInstanceStreamingCounter = 0;
	LevelHeader _levelHeader = {};
	std::ifstream _fin;
	bool _isInitialized = false;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	// ÉÅÉÇÉäÉäÅ[ÉNåüèo
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	ApplicationSystem* appSystem = ApplicationSystem::Get();
	GfxFramework* gfxFramework = GfxFramework::Get();

	appSystem->initialize();
	Application* app = appSystem->getApplication();
	app->initialize();
	gfxFramework->initialize();

	Mesh* mesh = nullptr;
	Mesh* mesh2 = nullptr;
	Mesh* mesh3 = nullptr;
	MeshInstance* meshInstance = nullptr;
	MeshInstance* meshInstance2 = nullptr;
	MeshInstance* meshInstance3 = nullptr;
	Material* material = nullptr;
	Texture* texture = nullptr;
	Texture* colorChartTexture = nullptr;
	MeshRendererSystem* meshSystem = MeshRendererSystem::Get();
	MaterialSystem* materialSystem = MaterialSystem::Get();
	TextureSystem* textureSystem = TextureSystem::Get();

	Level level;
	level.startLoad("L:/LightnEngine/resource/victorian/level.level");

	{
		MeshDesc desc = {};
		desc._filePath = "common/lod_test.mesh";
		//desc._filePath = "victorian\\facade_c\\sm_facade_c_door_lvl0.mesh";
		//mesh = meshSystem->createMesh(desc);

		//desc._filePath = "common/torus_high.mesh";
		//mesh2 = meshSystem->createMesh(desc);

		//desc._filePath = "victorian/side_walk/sm_street_a.mesh";
		//mesh3 = meshSystem->createMesh(desc);

	}

	//{
	//	MaterialDesc desc = {};
	//	desc._filePath = "common/default_mesh.mto";
	//	material = materialSystem->createMaterial(desc);
	//}

	{
		TextureDesc desc = {};
		desc._filePath = "common/checker.dds";
		texture = textureSystem->createTexture(desc);

		desc._filePath = "common/color_chart.dds";
		colorChartTexture = textureSystem->createTexture(desc);
	}

	{
		MeshInstanceDesc desc = {};
		//desc.mesh = mesh;
		//meshInstance = meshSystem->createMeshInstance(desc);
		////meshInstance->setMaterialSlotIndex(material, 0);
		//meshInstance->setWorldMatrix(Matrix4::rotateY(DegToRad(0)) * Matrix4::translate(-5, 2, 3));

		//desc.mesh = mesh2;
		//meshInstance2 = meshSystem->createMeshInstance(desc);
		//meshInstance2->setMaterialSlotIndex(material, 0);
		//meshInstance2->setWorldMatrix(Matrix4::translate(-1, 0, 0));

		//desc._meshes = mesh3;
		//meshInstance3 = meshSystem->createMeshInstance(desc);
		//meshInstance3->setMaterialSlotIndex(material, 0);
		//meshInstance3->setWorldMatrix(Matrix4::rotateX(DegToRad(-90.0f)) * Matrix4::translate(-1, 1, 0));
	}


	ApplicationCallBack f = [&](u32 message, u64 wParam, s64 lParam) {
		switch (message) {
		case WM_PAINT:
			level.doStream();
			gfxFramework->update();
			gfxFramework->render();
			break;
		}
	};
	app->registApplicationCallBack(f);

	app->run();

	level.unload();

	//material->requestToDelete();
	//meshInstance->requestToDelete();
	//meshInstance2->requestToDelete();
	//meshInstance3->requestToDelete();
	//mesh->requestToDelete();
	//mesh2->requestToDelete();
	//mesh3->requestToDelete();
	texture->requestToDelete();
	colorChartTexture->requestToDelete();

	app->terminate();
	gfxFramework->terminate();
	appSystem->terminate();

	return 0;
}