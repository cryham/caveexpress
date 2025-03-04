#include "UINodeMapSelector.h"
#include "common/IFrontend.h"
#include "common/MapManager.h"
#include "common/CommandSystem.h"
#include "campaign/CampaignManager.h"
#include "common/Commands.h"
#include "common/Math.h"
#include <SDL_assert.h>

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, const IMapManager &mapManager, bool multiplayer, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(nullptr), _mapManager(&mapManager), _multiplayer(multiplayer)
{
	// setColsRowsFromTexture("map-icon-locked");
	defaults();
	setPaddingPixel(10);
	reset();
}

UINodeMapSelector::UINodeMapSelector (IFrontend *frontend, CampaignManager &campaignManager, bool multiplayer, int cols, int rows) :
		UINodeBackgroundSelector<std::string>(frontend, cols, rows), _campaignManager(&campaignManager), _mapManager(
				nullptr), _multiplayer(multiplayer)
{
	// setColsRowsFromTexture("map-icon-locked");
	defaults();
	setPaddingPixel(10);
	reset();
}

UINodeMapSelector::~UINodeMapSelector ()
{
}

bool UINodeMapSelector::onSelect (const std::string& data)
{
	if (_campaignManager) {
		const CampaignPtr& campaign = _campaignManager->getActiveCampaign();
		if (campaign) {
			const CampaignMap* map = campaign->getMapById(data);
			if (map != nullptr) {
				if (map->isLocked())
					return false;
				_campaignManager->startMap(data);
				return true;
			}
		}
	}

	Commands.executeCommandLine(CMD_MAP_START " " + data);
	return true;
}

static const Color colorStars[4]  = {
	{ 0.4f, 0.4f, 0.3f, 0.8f },
	{ 0.3f, 0.25f,0.2f, 0.7f },
	{ 0.25f,0.15f,0.1f, 0.6f },
	{ 0.2f, 0.1f, 0.1f, 0.5f }};

void UINodeMapSelector::renderSelectorEntry (int index, const std::string& data, int x, int y, int colWidth,
		int rowHeight, float alpha) const
{
	std::string title;
	if (_mapManager != nullptr) {
		title = _mapManager->getMapTitle(data);
	} else {
		SDL_assert_always(_campaignManager != nullptr);
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap *map = campaignPtr->getMapById(data);
		if (map == nullptr)
			return;
		title = map->getName();
	}

	//*  image, stars  ***  ----
	const TexturePtr t = getIcon(data);
	if (_campaignManager != nullptr)
	{
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap *map = campaignPtr->getMapById(data);
		if (map != nullptr && !map->isLocked())
		{
			const BitmapFontPtr& font = getFont(LARGE_FONT);
			// const BitmapFontPtr& font = getFont(HUGE_FONT);
			
			std::string str = string::toString(map->getFinishPoints());// + "\nAbc";
			str += "\n";
			int stars = map->getStars();
			for (int i=0; i < stars; ++i)
				str += "*";

			// todo: bird, npcs, n pkgs, n taxi
			// str += "\n";
			// str += string::toString(map->_packages);
			// map->getPackageCount();

			// const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(points) / 2);
			const int fontX = x;
			// const int fontHeight = font->getTextHeight(points);
			const int fontY = y;// + fontHeight/2;
			
			if (t)  // stars backgr
				renderImage(t, x, y, colWidth, rowHeight -12 /*- fontHeight*/, alpha * 0.2f);  // dim

			renderFilledRect(x, y, colWidth, rowHeight,
				_selectedIndex == index ? colorGrayAlpha : colorStars[stars]);
			//  points
			font->printMax(str, colorWhite, fontX, fontY, colWidth);
		}
		else if (t) {
			renderImage(t, x, y, colWidth, rowHeight, alpha);
		}
	} else if (t) {
		renderImage(t, x, y, colWidth, rowHeight, alpha);
	}

/*	if (_selectedIndex != index)
		return;*/

	//*  title bottom  ----
	if (!title.empty()) {
		const BitmapFontPtr& font = getFont(title.length() > 15
			? MEDIUM_FONT : HUGE_FONT);
			// ? SMALL_FONT : MEDIUM_FONT);
		const int textHeight = font->getTextHeight(title);
		// const int fontX = std::max(x, x + colWidth / 2 - font->getTextWidth(title) / 2);  // center
		const int fontX = x;  // left
		const int fontY = y + rowHeight - textHeight - 1;

		_frontend->renderFilledRect(x, fontY - 1 -6, colWidth, textHeight + 2 +4, colorBlack);
		//_frontend->renderRect(x, fontY - 1, colWidth, textHeight + 2, colorWhite);
		font->printMax(title, colorWhiteTrue, fontX, fontY -4, colWidth);
	}
}

int UINodeMapSelector::getLives () const
{
	if (_campaignManager == nullptr)
		return 0;
	const CampaignPtr& c = _campaignManager->getActiveCampaign();
	return c->getLives();
}

TexturePtr UINodeMapSelector::getIcon (const std::string& data) const
{
	if (_campaignManager != nullptr) {
		const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
		const CampaignMap* map = campaignPtr->getMapById(data);
		if (map != nullptr) {
			if (map->isLocked())
				return loadTexture("map-icon-locked");

			const TexturePtr& ptr = loadTexture("map-icon-unlocked-" + string::toString(static_cast<int>(map->getStars())));
			if (ptr)
				return ptr;
		}
	}
	return loadTexture("map-icon-unlocked");
}

void UINodeMapSelector::reset ()
{
	UINodeSelector<std::string>::reset();
	if (_mapManager) {
		const IMapManager::Maps &maps = _mapManager->getMaps();
		for (IMapManager::Maps::const_iterator i = maps.begin(); i != maps.end(); ++i) {
			if (!_multiplayer || i->second->getStartPositions() > 1)
				addData(i->first);
		}
		return;
	}

	SDL_assert_always(_campaignManager != nullptr);
	const CampaignPtr& campaignPtr = _campaignManager->getActiveCampaign();
	if (!campaignPtr)
		return;
	const Campaign::MapList& maps = campaignPtr->getMaps();
	int index = -1;
	int mapIndex = 0;
	for (Campaign::MapListConstIter i = maps.begin(); i != maps.end(); ++i, ++mapIndex) {
		const CampaignMapPtr& p = *i;
		addData(p->getId());
		if (index == -1 && p->isLocked()) {
			index = mapIndex - 1;
		}
	}
	selectEntry(index);
}
