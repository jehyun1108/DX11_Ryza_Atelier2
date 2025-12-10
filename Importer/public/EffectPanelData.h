#pragma once

NS_BEGIN(Importer)

struct EffectPreview
{
	filesystem::path effectPath;
	wstring          effectKey;
	EffectHandle     handle{};
	float            dist = 50.f;
};

struct EffectEditorState
{
	EffectPreview   preview;    // 파일경로 + 프리뷰용 key/handle/dist
	EffectArchetype editing;    // 현재 편집중인 이펙트 데이터
	bool            hasEditing  = false;
	bool            dirty       = false;
	bool            loopPreview = false;
	bool            trailAlwaysOnPreview = false;
	string          statusMsg;

	EntityID previewOwner = 0;
};

NS_END