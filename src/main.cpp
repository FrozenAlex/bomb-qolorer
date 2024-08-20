#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/SpriteRenderer.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "bsml/shared/Helpers/delegates.hpp"
#include "bsml/shared/BSML.hpp"
#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"

#include "config.hpp"
#include "main.hpp"
#include "logging.hpp"

#include "bsml/shared/BSML-Lite/Creation/Layout.hpp"
#include "bsml/shared/BSML-Lite/Creation/Buttons.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML-Lite/Creation/Text.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

inline modloader::ModInfo modInfo = {MOD_ID, VERSION, 0}; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Returns a logger, useful for printing debug messages

// stolen from SmallQubes
auto createHoriz(UnityEngine::UI::VerticalLayoutGroup *vertical) {
	auto horiz = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
	horiz->GetComponent<UnityEngine::UI::LayoutElement *>()->set_minHeight(8);
	horiz->set_childForceExpandHeight(true);
	horiz->set_childAlignment(UnityEngine::TextAnchor::UpperCenter);
	return horiz->get_transform();
}

void DidActivate(UnityEngine::GameObject *self, bool firstActivation) {
	if(firstActivation) {
		auto vertical = BSML::Lite::CreateVerticalLayoutGroup(self->get_transform());
		 BSML::Lite::CreateToggle(createHoriz(vertical), "Enabled", getModConfig().enabled.GetValue(), [](bool enabled) {
			getModConfig().enabled.SetValue(enabled);
		});
		 BSML::Lite::CreateColorPicker(createHoriz(vertical), "Bomb Color", getModConfig().bombColor.GetValue(), [](UnityEngine::Color col) {
			getModConfig().bombColor.SetValue(col);
		});
	}
}
// General format: MAKE_HOOK_MATCH(hook name, hooked method, method return type, method class pointer, arguments...) {
//     HookName(self, arguments...);
//     your code here
// }

MAKE_HOOK_MATCH(BombNoteControllerInit, &GlobalNamespace::BombNoteController::Init, void, GlobalNamespace::BombNoteController *self, GlobalNamespace::NoteData *noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity) {
	BombNoteControllerInit(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity);
	if(!getModConfig().enabled.GetValue())
		return;
	for(auto m : self->GetComponentInChildren<Renderer *>()->get_materials()) {
		m->SetColor("_SimpleColor", getModConfig().bombColor.GetValue());
		m->SetColor("_Color", getModConfig().bombColor.GetValue());
	}
}

// Called at the early stages of game loading
extern "C" __attribute__((visibility("default"))) void setup(CModInfo& info) {
	info.id = MOD_ID;
    info.version = VERSION;
	info.version_long = VERSION_LONG;
	modInfo.assign(info);
	
	getModConfig().Init(modInfo);
	INFO("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" __attribute__((visibility("default"))) void late_load() {
	il2cpp_functions::Init();
	BSML::Init();
	BSML::Register::RegisterGameplaySetupTab(MOD_ID, DidActivate, BSML::MenuType::All);

	INFO("Installing hooks...");
	
	INSTALL_HOOK(Logger, BombNoteControllerInit);

	INFO("Installed all hooks!");
}
