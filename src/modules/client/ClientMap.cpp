#include "client/ClientMap.h"
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
#include "service/ServiceProvider.h"
#include "common/CommandSystem.h"
#include "common/ExecutionTime.h"
#include "common/DateUtil.h"
#include "common/Commands.h"
#include "common/Log.h"
#include <SDL.h>

ClientMap::ClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider, int referenceTileWidth) :
		IMap(), _x(x), _y(y), _width(width), _height(height), _scale(referenceTileWidth), _zoom(1.0f),
		_player(nullptr), _restartDue(0), _restartInitialized(0),
		_mapWidth(0), _mapHeight(0), _time(0), _playerID(0), _frontend(frontend), _pause(false),
		_serviceProvider(serviceProvider),
		_screenRumble(false), _screenRumbleStrength(0.0f), _screenRumbleOffsetX(0), _screenRumbleOffsetY(0),
		_particleSystem(Config.getClientSideParticleMaxAmount()),
		_tutorial(false), _started(false), _theme(&ThemeTypes::ROCK), _startPositions(0)
{
	_maxZoom = Config.getConfigVar("maxzoom", "1.2");
	_minZoom = Config.getConfigVar("minzoom", "0.5");
	_cooldowns.resize(8);
	_font = UI::get().getFont();

	_serviceProvider.getEventHandler().registerObserver(this);
}

ClientMap::~ClientMap ()
{
	resetCurrentMap();
	_serviceProvider.getEventHandler().registerObserver(this);
}

void ClientMap::onWindowResize ()
{
	setSize(_frontend->getWidth(), _frontend->getHeight());
}

void ClientMap::close ()
{
	resetCurrentMap();
	SoundControl.haltAll();
}

void ClientMap::start ()
{
	Log::info(LOG_CLIENT, "client map start");
	_started = true;
}

bool ClientMap::isStarted () const
{
	if (!_serviceProvider.getNetwork().isMultiplayer()) {
		return true;
	}
	return _started;
}

void ClientMap::resetCurrentMap ()
{
	Log::info(LOG_CLIENT, "client map reset");
	for (CooldownData& cooldownData : _cooldowns) {
		cooldownData.duration = cooldownData.start = 0;
	}
	_startPositions = 0;
	_zoom = 1.0f;
	_timeManager.reset();
	_settings.clear();
	_name.clear();
	_time = 0;
	_started = false;
	_introWindow = "";
	_tutorial = false;
	_mapWidth = 0;
	_mapHeight = 0;
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end(); ++i) {
		delete i->second;
	}
	_entities.clear();
	_player = nullptr;
	_pause = false;
	_playerID = 0;
	_camera.update(vec2_zero, 0, 1.0f);
	_restartDue = 0;
	_restartInitialized = 0;
	_particleSystem.clear();
	_screenRumble = false;
	_screenRumbleStrength = 0.0f;
	_screenRumbleOffsetX = 0;
	_screenRumbleOffsetY = 0;
}

void ClientMap::scroll(int relX, int relY)
{
	getCamera().scroll(relX, relY);
}

void ClientMap::setZoom (const float zoom)
{
	const float minZoom = _minZoom->getFloatValue();
	const float maxZoom = _maxZoom->getFloatValue();
	_zoom = clamp(zoom, minZoom, maxZoom);
}

void ClientMap::disconnect ()
{
	Log::info(LOG_CLIENT, "send disconnect to server");
	const DisconnectMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
	network.closeClient();
	SoundControl.haltAll();
}

ClientEntityPtr ClientMap::getEntity (uint16_t id)
{
	ClientEntityMapIter iter = _entities.find(id);
	if (iter == _entities.end()) {
		return ClientEntityPtr();
	}
	return iter->second;
}

void ClientMap::removeEntity (uint16_t id, bool fadeOut)
{
	if (fadeOut) {
		ClientEntityPtr e = getEntity(id);
		if (e) {
			e->initFadeOut();
		}
	} else {
		const ClientEntityPtr& e = getEntity(id);
		if (e) {
			e->remove();
			delete e;
			_entities.erase(id);
		}
	}
	if (_playerID == id) {
		Log::info(LOG_CLIENT, "remove client side player with the id %i", id);
		_player = nullptr;
		_playerID = 0;
	}
}

void ClientMap::renderFadeOutOverlay (int x, int y) const
{
	const uint32_t now = _time;
	const uint32_t delay = _restartDue - _restartInitialized;
	const float restartFadeStepWidth = 1.0f / delay;
	const uint32_t delta = now > _restartDue ? 0U : _restartDue - now;
	const float alpha = 1.0f - delta * restartFadeStepWidth;
	const Color color = { 0.0, 0.0, 0.0, alpha };
	_frontend->renderFilledRect(x, y, (int)((float)getPixelWidth() * _zoom), (int)((float)getPixelHeight() * _zoom), color);
}

void ClientMap::renderLayer (int x, int y, Layer layer) const
{
	if (Config.isDebug()) {
		_frontend->renderRect(x, y, 4, 4, colorYellow);
	}

	for (ClientEntityMapConstIter iter = _entities.begin(); iter != _entities.end(); ++iter) {
		const ClientEntityPtr& e = iter->second;
		e->render(_frontend, layer, _scale, _zoom, x, y);
	}
}

void ClientMap::renderLayers (int x, int y) const {
	renderLayer(x, y, LAYER_BACK);
	renderLayer(x, y, LAYER_MIDDLE);
	renderLayer(x, y, LAYER_FRONT);
}

void ClientMap::renderBegin (int x, int y) const {
}

void ClientMap::renderEnd (int x, int y) const {
}

void ClientMap::render () const
{
	ExecutionTime renderTime("ClientMapRender", 2000L);

	const int x = _screenRumbleOffsetX + _x + _camera.getViewportX();
	const int y = _screenRumbleOffsetY + _y + _camera.getViewportY();

	const int scissorX = std::max(0, x);
	const int scissorY = std::max(0, y);
	const bool debug = Config.isDebugUI();
	if (debug) {
		_frontend->renderRect(scissorX, scissorY, (int)((float)getPixelWidth() * _zoom), (int)((float)getPixelHeight() * _zoom), colorRed);
	} else {
		_frontend->enableScissor(scissorX, scissorY, (int)((float)getPixelWidth() * _zoom), (int)((float)getPixelHeight() * _zoom));
	}

	renderBegin(x, y);
	renderLayers(x, y);
	renderParticles(x, y);
	renderCooldowns(x, y);
	renderEnd(x, y);

	if (_restartDue != 0) {
		renderFadeOutOverlay(x, y);
	}

	Config.setDebugRendererData(x, y, getWidth(), getHeight(), (int)((float)_scale * _zoom));
	Config.getDebugRenderer().render();

	if (!debug) {
		_frontend->disableScissor();
	}
}

void ClientMap::renderCooldowns (int x, int y) const
{
	const int padding = (int)(0.006f * (float)_frontend->getHeight());
	const int marginTop = (int)(0.1f * (float)_frontend->getHeight());
	const int screenX = std::max(0, x) + padding;
	const int screenY = std::max(0, y) + padding + marginTop;
	int cooldownScreenY = screenY;
	int cooldownScreenX = screenX;
	const int cooldowns = static_cast<int>(_cooldowns.size());
	for (int cooldownId = 0; cooldownId < cooldowns; ++cooldownId) {
		const CooldownData& cooldownData = _cooldowns[cooldownId];
		if (cooldownData.start == 0) {
			continue;
		}
		const uint32_t endTime = cooldownData.start + cooldownData.duration;
		const int32_t delta = endTime - _time;
		if (delta <= 0) {
			continue;
		}

		const TexturePtr& texture = UI::get().loadTexture("cooldown-" + string::toString(cooldownId));
		if (!texture->isValid()) {
			continue;
		}
		const int cooldownWidth = texture->getWidth();
		const int cooldownHeight = texture->getHeight();
		const float ratio = delta / (float)cooldownData.duration;
		const int realWidth = (int)((float)cooldownWidth * ratio);
		if (realWidth <= 0) {
			continue;
		}

		Texture* tex = texture.get();
		_frontend->renderImage(tex, cooldownScreenX, cooldownScreenY, cooldownWidth, cooldownHeight, 0, 1.0f);
		_frontend->renderFilledRect(cooldownScreenX, cooldownScreenY, realWidth, cooldownHeight, colorGrayAlpha40);
		cooldownScreenX += renderCooldownDescription(cooldownId, cooldownScreenX, cooldownScreenY, cooldownWidth, cooldownHeight);
		cooldownScreenX += cooldownWidth + padding;
		if (cooldownScreenX >= getWidth() + x) {
			cooldownScreenX = screenX;
			cooldownScreenY += cooldownHeight + padding;
		}
	}
}

int ClientMap::renderCooldownDescription (uint32_t cooldownIndex, int x, int y, int w, int h) const
{
	return 0;
}

void ClientMap::renderParticles (int x, int y) const
{
	_particleSystem.render(_frontend, x, y, _zoom);
}

void ClientMap::getMapPixelForScreenPixel (int x, int y, int *outX, int *outY)
{
	const int nx = _screenRumbleOffsetX + _x + _camera.getViewportX();
	const int ny = _screenRumbleOffsetY + _y + _camera.getViewportY();
	*outX = x - nx;
	*outY = y - ny;
}

void ClientMap::getMapGridForScreenPixel (int x, int y, int *outX, int *outY)
{
	*outX = -1;
	*outY = -1;
	if (x < getX() || y < getY())
		return;
	if (x > getWidth() || y > getHeight())
		return;

	const int nx = _screenRumbleOffsetX + _x + _camera.getViewportX();
	const int ny = _screenRumbleOffsetY + _y + _camera.getViewportY();
	*outX = (x - nx) / _scale;
	*outY = (y - ny) / _scale;
}

void ClientMap::init (uint16_t playerID)
{
	Log::info(LOG_CLIENT, "init client map for player %i", playerID);

	_camera.init(getWidth(), getHeight(), _mapWidth, _mapHeight, _scale);

	_restartInitialized = 0U;
	_restartDue = 0U;
	_playerID = playerID;

	const std::string& name = Config.getName();
	const ClientInitMessage msgInit(name);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msgInit);
}

TexturePtr ClientMap::loadTexture (const std::string& name) const
{
	return UI::get().loadTexture(name);
}

bool ClientMap::wantInformation (const EntityType& type) const
{
	return isTutorial();
}

void ClientMap::accelerate (Direction dir, uint8_t id) const
{
	const MovementMessage msg(dir, id);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::stopFingerAcceleration () const
{
	static const StopFingerMovementMessage msg;
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::setFingerAcceleration (int dx, int dy) const
{
	const FingerMovementMessage msg(dx, dy);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

void ClientMap::resetAcceleration (Direction dir, uint8_t id) const
{
	const StopMovementMessage msg(dir, id);
	INetwork& network = _serviceProvider.getNetwork();
	network.sendToServer(msg);
}

bool ClientMap::initWaitingForPlayer () {
	INetwork& network = _serviceProvider.getNetwork();
	if (network.isMultiplayer())
		return false;

	if (!_introWindow.empty()) {
		UI::get().push(_introWindow);
	} else {
		Commands.executeCommandLine(CMD_START);
	}

	return true;
}

bool ClientMap::updateCameraPosition ()
{
	return _camera.update(_player->getPos(), _player->getMoveDirection(), _zoom);
}

void ClientMap::update (uint32_t deltaTime)
{
	if (isPause())
		return;

	if (_screenRumble) {
		_screenRumbleOffsetX = _screenRumbleOffsetY = 0;
		_screenRumbleOffsetX = rand() % std::max(2, static_cast<int>(_screenRumbleStrength * 10.0f));
		_screenRumbleOffsetY = rand() % std::max(2, static_cast<int>(_screenRumbleStrength * 10.0f));
	}

	for (CooldownData& cooldownData : _cooldowns) {
		if (cooldownData.start == 0)
			continue;
		const uint32_t endTime = cooldownData.start + cooldownData.duration;
		const int32_t delta = endTime - _time;
		if (delta <= 0) {
			cooldownData.start = cooldownData.duration = 0;
		}
	}

	_timeManager.update(deltaTime);
	_particleSystem.update(deltaTime);

	_time += deltaTime;
	if (_player) {
		updateCameraPosition();
		SoundControl.setListenerPosition(_player->getPos());
	}
	const ExecutionTime updateTime("ClientMap", 2000L);
	const bool lerp = wantLerp();
	for (ClientEntityMapIter i = _entities.begin(); i != _entities.end();) {
		const bool val = i->second->update(deltaTime, lerp);
		if (!val) {
			delete i->second;
			_entities.erase(i++);
		} else {
			++i;
		}
	}
}

bool ClientMap::load (const std::string& name, const std::string& title)
{
	Log::info(LOG_CLIENT, "load map %s", name.c_str());
	close();
	_name = name;
	_title = title;

	return true;
}

void ClientMap::addEntity (ClientEntityPtr e)
{
	auto iter = _entities.find(e->getID());
	if (iter != _entities.end()) {
		delete iter->second;
	}
	_entities[e->getID()] = e;
	if (e->getID() == _playerID) {
		_player = assert_cast<ClientPlayer*, ClientEntity*>(e);
	}

	Log::debug(LOG_CLIENT, "add entity %s - %i", e->getType().name.c_str(), (int)e->getID());
}

void ClientMap::setSetting (const std::string& key, const std::string& value)
{
	Log::debug(LOG_CLIENT, "client key: %s = %s", key.c_str(), value.c_str());
	_settings[key] = value;

	if (key == msn::WIDTH) {
		_mapWidth = string::toInt(value);
	} else if (key == msn::HEIGHT) {
		_mapHeight = string::toInt(value);
	} else if (key == msn::THEME) {
		_theme = &ThemeType::getByName(value);
	} else if (key == msn::TUTORIAL) {
		_tutorial = string::toBool(value);
	} else if (key == msn::INTROWINDOW) {
		_introWindow = value;
	}
}

void ClientMap::couldNotFindEntity (const std::string& prefix, uint16_t id) const
{
	Log::warn(LOG_CLIENT, "could not find entity with the id %i in %s", (int)id, prefix.c_str());
}

void ClientMap::changeAnimation (uint16_t id, const Animation& animation)
{
	const ClientEntityMapIter& iter = _entities.find(id);
	if (iter != _entities.end()) {
		iter->second->setAnimationType(animation);
		return;
	}
	couldNotFindEntity("changeAnimation", id);
}

bool ClientMap::updateEntity (uint16_t id, float x, float y, EntityAngle angle, uint8_t state)
{
	const ClientEntityMapIter& iter = _entities.find(id);
	if (iter != _entities.end()) {
		iter->second->setPos(vec2(x, y), wantLerp());
		iter->second->setAngle(angle);
		iter->second->changeState(state);
		return true;
	}
	couldNotFindEntity("updateEntity", id);
	return false;
}

void ClientMap::onData (ByteStream &data)
{
	ProtocolMessageFactory& factory = ProtocolMessageFactory::get();
	while (factory.isNewMessageAvailable(data)) {
		// remove the size from the stream
		data.readShort();
		const IProtocolMessage* msg(factory.createMsg(data));
		if (!msg) {
			Log::error(LOG_CLIENT, "no message for type %i", static_cast<int>(data.readByte()));
			continue;
		}

		Log::trace(LOG_CLIENT, "received message type %i", msg->getId());
		IClientProtocolHandler* handler = ProtocolHandlerRegistry::get().getClientHandler(*msg);
		if (handler != nullptr)
			handler->execute(*msg);
		else
			Log::error(LOG_CLIENT, "no client handler for message type %i", msg->getId());
	}
}

void ClientMap::disableScreenRumble ()
{
	Log::info(LOG_CLIENT, "stop rumble on the screen");
	_screenRumble = false;
	_screenRumbleStrength = 0.0f;
	_screenRumbleOffsetX = _screenRumbleOffsetY = 0;
}

void ClientMap::rumble (float strength, int lengthMillis)
{
	_frontend->rumble(strength, lengthMillis);
	Log::info(LOG_CLIENT, "rumble on the screen: %f", strength);
	_screenRumble = true;
	_screenRumbleStrength = strength;
	_timeManager.setTimeout(lengthMillis, this, &ClientMap::disableScreenRumble);
}

void ClientMap::spawnInfo (const vec2& position, const EntityType& type)
{
	Log::debug(LOG_CLIENT, "spawn info for '%s' at %f:%f", type.name.c_str(), position.x, position.y);
	// TODO:
}

void ClientMap::cooldown (const Cooldown& cd)
{
	Log::debug(LOG_CLIENT, "trigger cooldown %i", cd.id);
	if (cd.id >= _cooldowns.size()) {
		const CooldownData d{0, 0};
		_cooldowns.resize(cd.id, d);
	}
	const CooldownData cooldownData{_time, cd.getRuntime()};
	_cooldowns[cd.id] = cooldownData;
}
