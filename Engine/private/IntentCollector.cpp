#include "Enginepch.h"

void IntentCollector::Submit(const IntentWrite& write)
{
	auto& bucket = SelectBucket(write.channel);
	bucket[write.target] = write; // 동일 target 은 제출이 남음
}

void IntentCollector::Clear()
{
	scriptBuffer.clear();
	manualBuffer.clear();
	aiBuffer.clear();
}

IntentSnapShot IntentCollector::GetSnapShot() const
{
	IntentSnapShot snap{};
	snap.script = &scriptBuffer;
	snap.manual = &manualBuffer;
	snap.ai     = &aiBuffer;
	return snap;
}

unordered_map<EntityID, IntentWrite>& IntentCollector::SelectBucket(InputChannel channel)
{
	if (channel == InputChannel::Script) return scriptBuffer;
	if (channel == InputChannel::Manual) return manualBuffer;
	return aiBuffer;
}