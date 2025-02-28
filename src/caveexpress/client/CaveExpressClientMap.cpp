#include "caveexpress/client/CaveExpressClientMap.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "caveexpress/shared/CaveExpressCooldown.h"
#include "caveexpress/client/entities/ClientWindowTile.h"
#include "caveexpress/client/entities/ClientCaveTile.h"
#include "caveexpress/shared/network/messages/ProtocolMessages.h"
#include "particles/Bubble.h"
#include "particles/Snow.h"
#include "particles/Sparkle.h"
#include "common/MapSettings.h"
#include "network/messages/StopMovementMessage.h"
#include "network/messages/MovementMessage.h"
#include "network/messages/FingerMovementMessage.h"
#include "network/messages/ClientInitMessage.h"
#include "client/entities/ClientMapTile.h"
#include "ui/UI.h"
#include "common/IFrontend.h"
#include "network/ProtocolHandlerRegistry.h"
#include "sound/Sound.h"
#include "common/ConfigManager.h"
#include "common/EventHandler.h"
#include "common/Log.h"
#include "service/ServiceProvider.h"
#include "common/ExecutionTime.h"
#include "common/DateUtil.h"
#include <SDL.h>

namespace caveexpress {

static const Color waterLineColor = { 0.99f, 0.99f, 1.0f, 1.0f };
static const Color color = { (float)WATERCOLOR[0] / 255.0f, (float)WATERCOLOR[1] / 255.0f, (float)WATERCOLOR[2] / 255.0f, WATER_ALPHA
		/ 255.0f };

CaveExpressClientMap::CaveExpressClientMap (int x, int y, int width, int height, IFrontend *frontend,
		ServiceProvider& serviceProvider, int referenceTileWidth) :
		ClientMap(x, y, width, height, frontend, serviceProvider, referenceTileWidth), _waterHeight(0.0), _target(nullptr)
{
}

void CaveExpressClientMap::resetCurrentMap ()
{
	ClientMap::resetCurrentMap();
	_waterHeight = 0.0f;
}

SDL_Rect CaveExpressClientMap::getWaterRect(int x, int y) const {
	const int waterWidth = std::min(_width, static_cast<int>(getPixelWidth() * _zoom + std::min(x, 0))) - 1;
	const int waterGround = std::min(_height, y + static_cast<int>(getPixelHeight() * _zoom));
	const int waterSurface = y + getWaterSurface() * _zoom;
	const int waterHeight = waterGround - waterSurface;
	return SDL_Rect{std::max(0, x), waterSurface, waterWidth, waterHeight};
}

void CaveExpressClientMap::renderWater (int x, int y) const
{
	if (getWaterHeight() <= 0.000001f)
		return;
	const SDL_Rect& rect = getWaterRect(x, y);
	Log::trace(LOG_GAMEIMPL, "rect:(%i,%i,%i,%i), x:%i, y:%i, water:(w:%i, h:%i, surf:%i, grnd:%i, wh:%f, scale:%i)",
									_x, _y, _width, _height, x, y, rect.w, rect.h, rect.y, rect.y + rect.h, _waterHeight, _scale);
	_frontend->renderWaterPlane(rect.x, rect.y, rect.w, rect.h, color, waterLineColor);
	if (Config.isDebug()) {
		const int waterGround = rect.y + rect.h;
		_frontend->renderLine(rect.x, rect.y, rect.x + rect.w, rect.y, colorRed);
		_frontend->renderLine(rect.x, waterGround - 1, rect.x + rect.w, waterGround - 1, colorGreen);
		_frontend->renderLine(rect.x, rect.y, rect.x, waterGround, colorRed);
		_frontend->renderLine(rect.x + rect.w - 1, rect.y, rect.x + rect.w - 1, waterGround, colorGreen);
	}
}

bool CaveExpressClientMap::drop ()
{
	if (isPause() || !isActive())
		return false;

	if (!_player || !_player->hasCollected())
		return false;

	// If the player has collected something, this will inform the server that he now wants to drop it
	static const DropMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);

	return true;
}

void CaveExpressClientMap::setCaveNumber (uint16_t id, uint8_t number)
{
	if (number == 0)
		return;
	Log::debug(LOG_GAMEIMPL, "set cave for %i to %i", id, number);
	ClientEntityPtr e = getEntity(id);
	if (!e) {
		Log::error(LOG_GAMEIMPL, "no cave entity with the id %i found", id);
		return;
	}
	const char first = (char)(number / 10 + '0');
	const char second = (char)(number % 10 + '0');
	const std::string caveSprite = string::format("cave-sign-%c%c", first, second);
	e->addOverlay(UI::get().loadSprite(caveSprite));
}

void CaveExpressClientMap::setCaveState (uint16_t id, bool state)
{
	ClientEntityPtr e = getEntity(id);
	if (!e) {
		Log::error(LOG_GAMEIMPL, "no entity with the id %i found in setCaveState", id);
		return;
	}

	if (EntityTypes::isWindow(e->getType())) {
		ClientWindowTile *tile = static_cast<ClientWindowTile*>(e);
		tile->setLightState(state);
	} else if (EntityTypes::isCave(e->getType())) {
		ClientCaveTile *tile = static_cast<ClientCaveTile*>(e);
		tile->setLightState(state);
	}
}

void CaveExpressClientMap::couldNotFindEntity (const std::string& prefix, uint16_t id) const
{
	ClientMap::couldNotFindEntity(prefix, id);
	for (ClientEntityMapConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const ClientEntityPtr e = i->second;
		if (EntityTypes::isMapTile(e->getType()))
			continue;
		Log::info(LOG_GAMEIMPL, "id: %i, type: %s", e->getID(), e->getType().name.c_str());
	}
}

void CaveExpressClientMap::init (uint16_t playerID) {
	ClientMap::init(playerID);
	// TODO: also take the non water height into account - so not have the amount of bubbles
	// on a small area when the water is rising
	const int bubbles = getWidth() / 100;
	for (int i = 0; i < bubbles; ++i) {
		_particleSystem.spawn(ParticlePtr(new Bubble(*this)));
	}

	const bool xmas = dateutil::isXmas();
	if (xmas || ThemeTypes::isIce(*_theme)) {
		// TODO: also take the non water height into account - so not have the amount of flakes
		// on a small area when the water is rising
		const int snowFlakes = 
			(int)randBetweenf(100.0f, 5000.0f);
			// 9000;
			// * getWidth() / 10;
		for (int i = 0; i < snowFlakes; ++i) {
			_particleSystem.spawn(ParticlePtr(new Snow(*this)));
		}
	}
}

void CaveExpressClientMap::renderBegin (int x, int y) const
{
	_target = _frontend->renderToTexture(_x, _y, _width, _height);
	ClientMap::renderBegin(x, y);
}

void CaveExpressClientMap::renderEnd (int x, int y) const
{
	ClientMap::renderEnd(x, y);
	if (_target)
		_frontend->renderTarget(_target);
	renderWater(x, y);
}

int CaveExpressClientMap::renderCooldownDescription (uint32_t cooldownIndex, int x, int y, int w, int h) const
{
	ClientMap::renderCooldownDescription(cooldownIndex, x, y, w, h);
	const int padding = 5;
	if (Cooldowns::INVULVERABLE.id == cooldownIndex) {
		const std::string& text = tr("Invulverable");
		_font->print(text, colorWhite, x + w + padding, y);
		return 2 * padding + _font->getTextWidth(text);
	}
	return 0;
}

void CaveExpressClientMap::start () {
	ClientMap::start();
	for (ClientEntityMapConstIter i = _entities.begin(); i != _entities.end(); ++i) {
		const ClientEntityPtr& e = i->second;
		if (!EntityTypes::isLava(e->getType())) {
			continue;
		}
		int startX, startY, sizeW, sizeH;
		e->getScreenPos(startX, startY);
		e->getScreenSize(sizeW, sizeH);
		const int border = 5;
		sizeW -= border;
		startX += border;
		startY += (int)((float)sizeH / 2.0f);
		const int sparklePerLava = 4;
		for (int p = 0; p < sparklePerLava; ++p) {
			_particleSystem.spawn(ParticlePtr(new Sparkle(*this, startX, startY, sizeW, sizeH)));
		}
	}
}

}
