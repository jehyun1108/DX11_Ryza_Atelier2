#include "Enginepch.h"

bool JsonUtil::LoadFromFile(const string& path, nlohmann::json& outDocument, string& outErrorMsg)
{
	ifstream inputFileStream(path, ios::binary);
#ifdef _DEBUG
	assert(inputFileStream.good() && "LoadFromFile: failed to open JSON File");
#endif
	if (!inputFileStream)
	{
		outErrorMsg = "Failed to open file: " + path;
		return false;
	}
	outDocument = nlohmann::json::parse(inputFileStream, nullptr, false);
#ifdef _DEBUG
	assert(!outDocument.is_discarded() && "LoadFromFile: JSON parse failed");
#endif
	if (outDocument.is_discarded())
	{
		outErrorMsg = "JSON parse failed: " + path;
		return false;
	}
	return true;
}

bool JsonUtil::SaveToFile(const nlohmann::json& document, const string& filePath, bool prettyPrint, string& outErrorMsg)
{
	ofstream outputFileStream(filePath, ios::binary);
#ifdef _DEBUG
	assert(outputFileStream.good() && "SaveToFile: failed to open output file");
#endif
	if (!outputFileStream)
	{
		outErrorMsg = "Failed to write file: " + filePath;
		return false;
	}
	outputFileStream << (prettyPrint ? document.dump(4) : document.dump());
	return true;
}

bool JsonUtil::ValidateKeys(const nlohmann::json& obj, const vector<string>& requiredKeys, string& outErrorMsg)
{
#ifdef _DEBUG
	assert(obj.is_object() && "ValidateKey: target json is not an obj");
#endif
	if (!obj.is_object())
	{
		outErrorMsg = "Target json is not an obj";
		return false;
	}

	for (const string& key : requiredKeys)
	{
#ifdef _DEBUG
		assert(obj.contains(key) && "ValidateKey: required key missing");
#endif
		if (!obj.contains(key))
		{
			outErrorMsg = "Missing required key: " + key;
			return false;
		}
	}
	return true;
}

int JsonUtil::GetInt32(const nlohmann::json& obj, const string& key, int value)
{
	if (!obj.contains(key)) return value;
#ifdef _DEBUG
	assert((obj[key].is_number_integer() || obj[key].is_number_unsigned()) && "GetInt32: type mismatch");
#endif
	try
	{
		return obj[key].get<int>();
	}
	catch (const exception&)
	{
		return value;
	}
}

int64_t JsonUtil::GetInt64(const nlohmann::json& obj, const string& key, int64_t value)
{
	if (!obj.contains(key)) return value;
#ifdef _DEBUG
	assert(((obj)[key].is_number_integer() || obj[key].is_number_unsigned()) && "GetInt64: type mismatch");
#endif
	try
	{
		return obj[key].get<int64_t>();
	}
	catch (const exception&)
	{
		return value;
	}
}

double JsonUtil::GetDouble(const nlohmann::json& obj, const string& key, double value)
{
	if (!obj.contains(key)) return value;
#ifdef _DEBUG
	assert(obj[key].is_number() && "GetDouble: type mismatch");
#endif
	try
	{
		return obj[key].get<double>();
	}
	catch (const exception&)
	{
		return value;
	}
}

bool JsonUtil::GetBool(const nlohmann::json& obj, const string& key, bool value)
{
	if (!obj.contains(key)) return value;
#ifdef _DEBUG
	assert(obj[key].is_boolean() && "GetBool: type mismatch");
#endif
	try
	{
		return obj[key].get<bool>();
	}
	catch (const exception&)
	{
		return value;
	}
}

string JsonUtil::GetString(const nlohmann::json& obj, const string& key, const string& value)
{
	if (!obj.contains(key)) return value;
#ifdef _DEBUG
	assert(obj[key].is_string() && "GetString: type mismatch");
#endif
	try
	{
		return obj[key].get<string>();
	}
	catch (const exception&)
	{
		return value;
	}
}

const nlohmann::json* JsonUtil::FindDot(const nlohmann::json& root, const string& dotPath)
{
	if (dotPath.empty()) return &root;

	const nlohmann::json* cur = &root;
	size_t tokenStart = 0;

	while (tokenStart <= dotPath.size())
	{
		size_t tokenEnd = dotPath.find('.', tokenStart);
		string curKey = (tokenEnd == string::npos) ? dotPath.substr(tokenStart) : dotPath.substr(tokenStart, tokenEnd - tokenStart);

		if (!cur->is_object() || !cur->contains(curKey))
			return nullptr;

		cur = &((*cur)[curKey]);
		if (tokenEnd == string::npos) break;
		tokenStart = tokenEnd + 1;
	}
	return cur;
}

nlohmann::json JsonUtil::DeepMerge(const nlohmann::json& baseObj, const nlohmann::json& overrideObj)
{
	if (!baseObj.is_object() || !overrideObj.is_object())
		return overrideObj.is_null() ? baseObj : overrideObj;

	nlohmann::json merged = baseObj;
	for (auto it = overrideObj.begin(); it != overrideObj.end(); ++it)
	{
		const string& key = it.key();
		const nlohmann::json& overrideValue = it.value();

		if (merged.contains(key) && merged[key].is_object() && overrideValue.is_object())
			merged[key] = DeepMerge(merged[key], overrideValue);
		else
			merged[key] = overrideValue;
	}
	return merged;
}