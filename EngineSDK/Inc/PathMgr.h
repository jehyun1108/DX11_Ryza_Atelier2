#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL PathMgr final
{
public:
	static const filesystem::path& GetModelPath()    { return modelPath; }
	static const filesystem::path& GetTexturePath()  { return texturePath; }
	static const filesystem::path& GetAssetPath()    { return assetPath; }

private:
	inline static filesystem::path modelPath   = L"../bin/Resources/Models/";
	inline static filesystem::path texturePath = L"../bin/Resources/Textures/";
	inline static filesystem::path assetPath = L"../bin/Resources/";
};

NS_END