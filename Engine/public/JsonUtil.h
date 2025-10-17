#pragma once

NS_BEGIN(Engine)

// 저장/불러오기 + Key 검증 + Merge + ArrayMapping

class ENGINE_DLL JsonUtil
{
public:
	JsonUtil() = delete;

	// ---------- Save / Load ---------------------
	static bool LoadFromFile(const string& path, nlohmann::json& outDocument, string& outErrorMsg);
	static bool SaveToFile(const nlohmann::json& document, const string& filePath, bool prettyPrint, string& outErrorMsg);

	// ------- Key 검증 -------------
	static bool    ValidateKeys(const nlohmann::json& obj, const vector<string>& requiredKeys, string& outErrorMsg);
	static int     GetInt32(const nlohmann::json& obj, const string& key, int value);
	static int64_t GetInt64(const nlohmann::json& obj, const string& key, int64_t value);
	static double  GetDouble(const nlohmann::json& obj, const string& key, double value);
	static bool    GetBool(const nlohmann::json& obj, const string& key, bool value);
	static string  GetString(const nlohmann::json& obj, const string& key, const string& value);

	// ------------ DeepMerge ---------------
	static nlohmann::json DeepMerge(const nlohmann::json& baseObj, const nlohmann::json& overrideObj);

	// ----------- (.)경로 접근 (배열 인덱스x)
	static const nlohmann::json* FindDot(const nlohmann::json& root, const string& dotPath);

	template<typename T>
	static T GetAtPath(const nlohmann::json& root, const string& dotPath, const T& value);

	// ------------ 배열 매핑 --------------------------------------------------------
	template<typename T, typename Converter>
	static vector<T> MapArray(const nlohmann::json& obj, const string& key, Converter&& converter);
};


template<typename T>
inline T JsonUtil::GetAtPath(const nlohmann::json& root, const string& dotPath, const T& value)
{
	const nlohmann::json* found = FindDot(root, dotPath);
	if (!found) return value;

#ifdef _DEBUG
	if constexpr (is_same_v<T, string>)
		assert(found->is_string() && "GetAtPath: value is not string");
	else if constexpr (is_same_v<T, bool>)
		assert(found->is_boolean() && "GetAtPath: value is not bool");
	else if constexpr (is_integral_v<T>)
		assert((found->is_number_integer() || found->is_number_unsigned()) && "GetAtPath: value is not integer");
	else if constexpr (is_floating_point_v<T>)
		assert(found->is_number() && "GetAtPath: value is not number");
#endif 
	try
	{
		return found->get<T>();
	}
	catch (const exception&)
	{
#ifdef _DEBUG
		assert(false && "GetAtPath: conversion failed");
#endif
		return value;
	}
}

template<typename T, typename Converter>
inline vector<T> JsonUtil::MapArray(const nlohmann::json& obj, const string& key, Converter&& converter)
{
#ifdef _DEBUG
	if (obj.contains(key))
		assert(obj.at(key).is_array() && "MapArray: target key exists but is not an array");
#endif

	vector<T> mapped;
	if (!obj.contains(key) || !obj.at(key).is_array()) return mapped;

	const nlohmann::json& node = obj.at(key);
	mapped.reserve(node.size());
	for (const auto& element : node)
		mapped.push_back(converter(element));
	return mapped;
}

NS_END