#pragma once

NS_BEGIN(Engine)

// "허용된 제출"을 채널별로 모으는 버퍼 (프레임 한정)
class ENGINE_DLL IntentCollector
{
public:
	void Submit(const IntentWrite& write);
	void Clear();

	IntentSnapShot GetSnapShot() const;

	// Debug
	size_t SizeScript() const { return scriptBuffer.size(); }
	size_t SizeManual() const { return manualBuffer.size(); }
	size_t SizeAI()     const { return aiBuffer.size(); }

private:
	unordered_map<EntityID, IntentWrite>& SelectBucket(InputChannel channel);

private:
	unordered_map<EntityID, IntentWrite> scriptBuffer;
	unordered_map<EntityID, IntentWrite> manualBuffer;
	unordered_map<EntityID, IntentWrite> aiBuffer;
};

NS_END