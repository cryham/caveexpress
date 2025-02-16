#include "UIWindow.h"
#include "ui/UI.h"
#include "sound/Sound.h"
#include "common/Log.h"
#include "common/EventHandler.h"
#include "common/CommandSystem.h"
#include "common/System.h"
#include "common/Application.h"
#include "common/ConfigManager.h"
#include "ui/BitmapFont.h"
#include "ui/windows/listener/RatePopupCallback.h"

UIWindow::UIWindow (const std::string& id, IFrontend *frontend, WindowFlags flags) :
		UINode(frontend, id), _flags(flags), _musicFile("music-1"), _music(-1), _pushedTime(0), _inactiveAfterPush(0), _playClickSound(true)
{
	setSize(1.0, 1.0);
}

UIWindow::~UIWindow ()
{
	for (UINodeListIter i = _nodes.begin(); i != _nodes.end(); ++i) {
		delete *i;
	}
	_nodes.clear();
}

void UIWindow::render (int x, int y) const
{
	if (_flags & WINDOW_FLAG_MODAL) {
		const Color bgColor = {0.7f, 0.7f, 0.7f, 0.4f};
		_frontend->renderFilledRect(0, 0, 0, 0, bgColor);
	}

	UINode::render(x, y);

	for (const UINode* nodePtr : _nodes) {
		nodePtr->renderOnTop(x, y);
	}
}

bool UIWindow::onPop ()
{
	bool pop = true;
	for (UINode* nodePtr : _nodes) {
		const bool val = nodePtr->onPop();
		if (!val)
			pop = false;
	}
	if (pop) {
		Commands.executeCommandLine(_onPop);
		_pushedTime = 0;
	}

	return pop;
}

void UIWindow::onPushedOver ()
{
}

void UIWindow::onActive ()
{
	int x, y;
	UI::get().getCursorPosition(x, y);
	if (!checkFocus(FOCUSADD_ACTIVE, x, y)) {
		addFirstFocus();
	}
	System.track("activewindow", getId());
	_pushedTime = _time;

	if (getId() == UI_WINDOW_MAIN && !System.getRateURL("").empty()) {
		const bool alreadyRated = Config.getConfigVar("alreadyrated")->getBoolValue();
		const int launchCount = Config.getConfigVar("launchcount")->getIntValue();
		if (!alreadyRated && (launchCount % 3) == 0) {
			Log::info(LOG_UI, "please rate the app popup (launches: %i)", launchCount);
			UIPopupCallbackPtr c(new RatePopupCallback(_frontend));
			UI::get().popup(tr("Please rate the app"), UIPOPUP_OK | UIPOPUP_CANCEL | UIPOPUP_LATER | UIPOPUP_NOCLOSE, c);
		}
	}
}

bool UIWindow::onPush ()
{
	bool push = true;
	for (UINode* nodePtr : _nodes) {
		const bool val = nodePtr->onPush();
		if (!val)
			push = false;
	}
	if (push) {
		Log::info(LOG_UI, "pushed window %s onto the stack", _id.c_str());
		Commands.executeCommandLine(_onPush);
		startMusic();
		addFirstFocus();
	}

	return push;
}

void UIWindow::startMusic ()
{
	if (!_musicFile.empty()) {
		Log::info(LOG_UI, "attempt to start the music file %s", _musicFile.c_str());
		_music = SoundControl.playMusic(_musicFile);
		if (_music == -1) {
			Log::error(LOG_UI, "failed to start the music file %s in the context of window %s", _musicFile.c_str(), _id.c_str());
		}
		return;
	}
	stopMusic();
}

void UIWindow::stopMusic ()
{
	SoundControl.haltMusic(_music);
	_music = -1;
}

bool UIWindow::onKeyRelease (int32_t key)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onKeyRelease(key);
}

bool UIWindow::onKeyPress (int32_t key, int16_t modifier)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onKeyPress(key, modifier);
}

bool UIWindow::onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion)
{
	if (!isActiveAfterPush())
		return false;

	if (UINode::onFingerRelease(finger, x, y, motion)) {
		if (_playClickSound)
			SoundControl.play("click");
		return true;
	}

	return false;
}

bool UIWindow::onFingerPress (int64_t finger, uint16_t x, uint16_t y)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onFingerPress(finger, x, y);
}

bool UIWindow::onMouseButtonRelease (int32_t x, int32_t y, unsigned char button)
{
	if (!isActiveAfterPush())
		return false;

	if (UINode::onMouseButtonRelease(x, y, button)) {
		if (_playClickSound)
			SoundControl.play("click");
		return true;
	}

	return false;
}

bool UIWindow::onMouseButtonPress (int32_t x, int32_t y, unsigned char button)
{
	Log::debug(LOG_UI, "onMouseButtonPress: %s (%i:%i, %c)", getId().c_str(), x, y, button);
	if (!isActiveAfterPush())
		return false;

	return UINode::onMouseButtonPress(x, y, button);
}

bool UIWindow::onControllerButtonPress (int x, int y, const std::string& button, uint32_t id)
{
	if (!isActiveAfterPush())
		return false;

	return UINode::onControllerButtonPress(x, y, button, id);
}

UINode* UIWindow::addTextureNode (const std::string& texture, float x, float y, float w, float h)
{
	UINode* imageNode = new UINode(_frontend);
	imageNode->setImage(texture);
	imageNode->setPos(x, y);
	imageNode->setSize(w, h);
	add(imageNode);
	return imageNode;
}
