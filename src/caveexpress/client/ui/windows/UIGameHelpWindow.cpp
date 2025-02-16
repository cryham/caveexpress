#include "UIGameHelpWindow.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "ui/nodes/UINodeBackground.h"
#include "common/ConfigManager.h"
#include "ui/nodes/UINodeBackButton.h"
#include "ui/layouts/UIHBoxLayout.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/UI.h"
#include "common/SpriteDefinition.h"
#include "common/Shared.h"
#include "caveexpress/shared/constants/ConfigVars.h"

namespace caveexpress {

static const float iconsize = 0.08f;
static const float hboxspacing = 0.01f;
static const float vboxspacing = 0.01f;

UIGameHelpWindow::UIGameHelpWindow (IFrontend* frontend) :
		UIWindow(UI_WINDOW_HELP, frontend, WINDOW_FLAG_FULLSCREEN)
{
	UINodeBackground *background = new UINodeBackground(frontend, tr("Help"), false);
	add(background);

	UINode *panel = new UINode(_frontend, "helppanel");
	UIVBoxLayout *vboxLayout = new UIVBoxLayout(vboxspacing, true, NODE_ALIGN_CENTER);
	panel->setLayout(vboxLayout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	addPackageHelp(panel);
	addStoneWalkingHelp(panel);
	addStoneFlyingHelp(panel);
	addTreeHelp(panel);
	// TODO: invisible and update on onActive
	if (Config.isModeEasy() && Config.getConfigVar(AMOUNT_OF_FRUITS_FOR_A_NEW_LIFE)->getIntValue() > 0) {
		addLivesHelp(panel);
	}

	add(panel);

	if (!wantBackButton())
		return;

	add(new UINodeBackButton(frontend, background));
}

UIGameHelpWindow::~UIGameHelpWindow ()
{
}

UINodeSprite* UIGameHelpWindow::createSprite (const EntityType& type, const Animation& animation, float w, float h)
{
	const std::string spriteName = SpriteDefinition::get().getSpriteName(type, animation);
	const SpritePtr& spritePtr = UI::get().loadSprite(spriteName);
	UINodeSprite* spriteNode = new UINodeSprite(_frontend);
	spriteNode->addSprite(spritePtr);
	spriteNode->setSize(w, h);
	spriteNode->setAspectRatioSize(w, h, 1.3f);
	spriteNode->alignToMiddle();
	return spriteNode;
}

UINode* UIGameHelpWindow::createTexture (const std::string& texture)
{
	UINode* imageNode = new UINode(_frontend);
	imageNode->setImage(texture);
	return imageNode;
}

UINode* UIGameHelpWindow::createHPanel ()
{
	UINode* hbox = new UINode(_frontend);
	hbox->setLayout(new UIHBoxLayout(hboxspacing, false));
	hbox->setBackgroundColor(backgroundColor);
	hbox->setBorder(true);
	hbox->setBorderColor(colorWhite);
	hbox->setStandardPadding();
	return hbox;
}

void UIGameHelpWindow::addTreeHelp (UINode *panel)
{
	UINode* hbox = createHPanel();
	hbox->add(createSprite(EntityTypes::STONE));
	hbox->add(createTexture("icon-plus"));
	hbox->add(createSprite(EntityTypes::TREE));
	hbox->add(createTexture("icon-result"));
	hbox->add(createSprite(EntityTypes::TREE, Animations::ANIMATION_DAZED));
	hbox->add(createSprite(EntityTypes::APPLE));
	panel->add(hbox);
}

void UIGameHelpWindow::addPackageHelp (UINode *panel)
{
	UINode* hbox = createHPanel();
	hbox->add(createSprite(EntityTypes::PACKAGE_ROCK));
	hbox->add(createTexture("icon-plus"));
	hbox->add(createSprite(EntityTypes::PACKAGETARGET_ROCK));
	hbox->add(createTexture("icon-result"));
	hbox->add(createSprite(EntityTypes::PACKAGETARGET_ROCK, Animations::ANIMATION_ACTIVE));
	panel->add(hbox);
}

void UIGameHelpWindow::addStoneWalkingHelp (UINode *panel)
{
	UINode* hbox = createHPanel();
	hbox->add(createSprite(EntityTypes::STONE));
	hbox->add(createTexture("icon-plus"));
	hbox->add(createSprite(EntityTypes::NPC_WALKING, Animations::ANIMATION_IDLE_RIGHT, iconsize * 2.0f, iconsize));
	hbox->add(createTexture("icon-result"));
	hbox->add(createSprite(EntityTypes::NPC_WALKING, Animations::ANIMATION_DAZED_RIGHT, iconsize * 2.0f, iconsize));
	panel->add(hbox);
}

void UIGameHelpWindow::addStoneFlyingHelp (UINode *panel)
{
	UINode* hbox = createHPanel();
	hbox->add(createSprite(EntityTypes::STONE));
	hbox->add(createTexture("icon-plus"));
	hbox->add(createSprite(EntityTypes::NPC_FLYING, Animations::ANIMATION_FLYING_RIGHT, iconsize * 2.0f, iconsize));
	hbox->add(createTexture("icon-result"));
	hbox->add(createSprite(EntityTypes::NPC_FLYING, Animations::ANIMATION_FALLING_RIGHT, iconsize * 2.0f, iconsize));
	panel->add(hbox);
}

void UIGameHelpWindow::addLivesHelp (UINode *panel)
{
	UINode* hbox = createHPanel();
	const int n = Config.getConfigVar(AMOUNT_OF_FRUITS_FOR_A_NEW_LIFE)->getIntValue();
	for (int i = 0; i < n - 1; ++i) {
		hbox->add(createSprite(EntityTypes::APPLE));
	}
	hbox->add(createSprite(EntityTypes::BANANA));
	hbox->add(createTexture("icon-result"));
	hbox->add(createTexture("icon-plus"));
	hbox->add(createSprite(EntityTypes::PLAYER));
	panel->add(hbox);
}

}
