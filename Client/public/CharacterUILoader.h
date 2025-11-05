#pragma once

NS_BEGIN(Client)

class CharacterUILoader
{
public:
	inline static void RegisterCharacterUIMappings(CharacterDataSystem* dataSys)
	{
		CharacterUITextures ryza{};
		ryza.Set(UITextureSlot::TimelineIcon, L"timeline_ryza");
		ryza.Set(UITextureSlot::FatalDrive, L"ryza_fataldrive");
		dataSys->RegisterUITextures(CharacterID::Ryza, ryza);

		CharacterUITextures klaudia{};
		klaudia.Set(UITextureSlot::TimelineIcon, L"timeline_klaudia");
		klaudia.Set(UITextureSlot::FatalDrive, L"klaudia_fataldrive");
		dataSys->RegisterUITextures(CharacterID::Klaudia, klaudia);

		CharacterUITextures patricia{};
		patricia.Set(UITextureSlot::TimelineIcon, L"timeline_patricia");
		patricia.Set(UITextureSlot::FatalDrive, L"patricia_fataldrive");
		dataSys->RegisterUITextures(CharacterID::Patricia, patricia);

		CharacterUITextures angel{};
		angel.Set(UITextureSlot::TimelineIcon, L"timeline_npc");
		dataSys->RegisterUITextures(CharacterID::Angel, angel);
	}
};


NS_END