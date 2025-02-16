#include "UIMainWindow.h"
#include "ui/UI.h"
#include "ui/nodes/UINodeButton.h"
#include "ui/nodes/UINodeButtonImage.h"
#include "ui/nodes/UINodeMainButton.h"
#include "ui/nodes/UINodeMainBackground.h"
#include "ui/nodes/UINodeSprite.h"
#include "ui/nodes/UINodeLabel.h"
#include "ui/nodes/UINodeGooglePlayButton.h"
#include "ui/windows/listener/QuitListener.h"
#include "ui/layouts/UIVBoxLayout.h"
#include "common/ConfigManager.h"
#include "common/System.h"
#include "common/Application.h"
#include "ui/windows/listener/OpenWindowListener.h"

namespace caveexpress {

UIMainWindow::UIMainWindow (IFrontend *frontend, ServiceProvider& serviceProvider) :
		UIWindow(UI_WINDOW_MAIN, frontend, WINDOW_FLAG_ROOT)
{
	add(new UINodeMainBackground(frontend, false));
	const SpritePtr& mammutSprite = UI::get().loadSprite("ui-npc-mammut");
	_mammut = new UINodeSprite(frontend, mammutSprite->getMaxWidth(), mammutSprite->getMaxHeight());
	_mammut->addSprite(mammutSprite);

	const SpritePtr& grandPaSprite = UI::get().loadSprite("ui-npc-grandpa");
	_grandpa = new UINodeSprite(frontend, grandPaSprite->getMaxWidth(), grandPaSprite->getMaxHeight());
	_grandpa->addSprite(grandPaSprite);

	const SpritePtr& playerSprite = UI::get().loadSprite("ui-player");
	_player = new UINodeSprite(frontend, playerSprite->getMaxWidth(), playerSprite->getMaxHeight());
	_player->addSprite(playerSprite);

	add(_mammut);
	add(_grandpa);
	add(_player);

	const float padding = 10.0f / static_cast<float>(_frontend->getHeight());
	UINode *panel = new UINode(_frontend, "panelMain");
	UIVBoxLayout *layout = new UIVBoxLayout(padding, true);
	panel->setLayout(layout);
	panel->setAlignment(NODE_ALIGN_MIDDLE | NODE_ALIGN_CENTER);
	panel->setPadding(padding);

	UINodeMainButton *campaign = new UINodeMainButton(_frontend, tr("Campaign"));
	campaign->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_CAMPAIGN)));
	panel->add(campaign);

#ifndef NONETWORK
	if (Config.isNetwork()) {
		UINodeMainButton *multiplayer = new UINodeMainButton(_frontend, tr("Multiplayer"));
		multiplayer->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_MULTIPLAYER)));
		panel->add(multiplayer);
	}
#endif

	UINodeMainButton *settings = new UINodeMainButton(_frontend, tr("Settings"));
	settings->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_SETTINGS)));
	panel->add(settings);

	if (System.supportGooglePlay()) {
		UINodeButtonImage *googlePlay = new UINodeGooglePlayButton(_frontend);
		googlePlay->setPadding(padding);
		add(googlePlay);
	}

	if (System.supportsUserContent()) {
		UINodeMainButton *editor = new UINodeMainButton(_frontend, tr("Editor"));
		editor->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_EDITOR)));
		panel->add(editor);
	}

	UINodeMainButton *homepage = new UINodeMainButton(_frontend, tr("Homepage"));
	homepage->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://caveproductions.org/")));
	panel->add(homepage);

	UINodeMainButton *help = new UINodeMainButton(_frontend, tr("Help"));
	help->addListener(UINodeListenerPtr(new OpenWindowListener(UI_WINDOW_HELP)));
	panel->add(help);

#if 0
#ifdef __EMSCRIPTEN__
	UINodeMainButton *fullscreen = new UINodeMainButton(_frontend, tr("Fullscreen"));
	fullscreen->addListener(UINodeListenerPtr(new EmscriptenFullscreenListener()));
	panel->add(fullscreen);
#endif
#endif

	UINodeMainButton *quit = new UINodeMainButton(_frontend, tr("Quit"));
#ifdef __EMSCRIPTEN__
	quit->addListener(UINodeListenerPtr(new OpenURLListener(_frontend, "http://caveproductions.org/", false)));
#else
	quit->addListener(UINodeListenerPtr(new QuitListener()));
#endif
	panel->add(quit);

	add(panel);

	Application& app = Singleton<Application>::getInstance();
	UINodeLabel *versionLabel = new UINodeLabel(_frontend, app.getPackageName() + " " + app.getVersion());
	versionLabel->setAlignment(NODE_ALIGN_BOTTOM|NODE_ALIGN_RIGHT);
	versionLabel->setColor(colorWhite);
	versionLabel->setPadding(getScreenPadding());
	add(versionLabel);
}

void UIMainWindow::update (uint32_t deltaTime)
{
	UIWindow::update(deltaTime);
	if (!_player->isMovementActive()) {
		flyPlayer();
	}
	if (!_grandpa->isMovementActive()) {
		moveSprite(_grandpa, 0.00008f);
	}
	if (!_mammut->isMovementActive()) {
		moveSprite(_mammut, 0.0001f);
	}
}

void UIMainWindow::moveSprite (UINodeSprite* sprite, float speed) const
{
	const float y = 1.0f - (sprite->getRenderHeight() / static_cast<float>(_frontend->getHeight()));
	const float startX = randBetweenf(-2.2f, -0.2f);
	const float endX = randBetweenf(2.1f, 3.3f);
	sprite->setMovement(startX, y, endX, y, speed);
}

void UIMainWindow::flyPlayer ()
{
	const float spriteSize = _player->getRenderHeight() / static_cast<float>(_frontend->getHeight());
	const float y = 1.0f - spriteSize;
	const float startYRand = randBetweenf(0.0f, y);
	const float endYRand = randBetweenf(0.0f, y);
	const bool leftToRight = rand() % 2 == 0;
	float startX = randBetweenf(-4.5f, -0.5f);
	float endX = randBetweenf(2.5f, 4.5f);
	if (!leftToRight) {
		std::swap(startX, endX);
	}
	_player->setMovement(startX, startYRand, endX, endYRand, 0.0004f);
}

}
