#include "client/entities/ClientEntity.h"
#include "sound/Sound.h"
#include "common/IFrontend.h"
#include "ui/UI.h"
#include "ui/BitmapFont.h"
#include "common/Shared.h"
#include "common/Direction.h"
#include "common/Log.h"
#include "common/FileSystem.h"
#include "common/ConfigManager.h"

ClientEntity::ClientEntity (const EntityType& type, uint16_t id, float x, float y, float sizeX, float sizeY,
		const SoundMapping& soundMapping, EntityAlignment align, EntityAngle angle) :
		_type(type), _id(id), _angle(angle), _time(0), _currSprite(), _state(0), _animation(&Animation::NONE), _fadeOutTime(
				0), _alpha(1.0f), _ropeEntity(nullptr), _animationSound(-1), _soundMapping(soundMapping), _visible(true), _visChanged(false), _align(
				align), _screenPosX(0), _screenPosY(0), _screenWidth(0), _screenHeight(0)
{
	const vec2 startPos(x, y);
	_prevPos = _nextPos = _pos = startPos;
	_size = vec2(sizeX, sizeY);
	_ropeTexture = UI::get().loadTexture("rope");
}

ClientEntity::~ClientEntity ()
{
	_currSprite = SpritePtr();
	_entityOverlays.clear();
	_sprites.clear();
}

void ClientEntity::remove ()
{
	SoundControl.halt(_animationSound);
}

void ClientEntity::onVisibilityChanged ()
{
}

void ClientEntity::calcOffset (int scale, float zoom, int posX, int posY, int &offsetPosX, int &offsetPosY) const
{
	offsetPosX = posX;
	offsetPosY = posY;
	switch (_align) {
	case ENTITY_ALIGN_UPPER_LEFT:
		break;
	case ENTITY_ALIGN_LOWER_LEFT:
		offsetPosY -= _size.y * scale * zoom;
		break;
	case ENTITY_ALIGN_MIDDLE_CENTER:
		offsetPosX += _size.x / 2.0f * scale * zoom;
		offsetPosY -= _size.y / 2.0f * scale * zoom;
		break;
	}
}

void ClientEntity::renderOverlays (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY, int posX, int posY) const
{
	int offsetPosX, offsetPosY;
	calcOffset(scale, zoom, posX, posY, offsetPosX, offsetPosY);
	for (EntityOverlaysConstIter i = _entityOverlays.begin(); i != _entityOverlays.end(); ++i) {
		const SpritePtr& overlay = *i;
		Log::trace(LOG_CLIENT, "render %s, layer %i, x: %i, y: %i, zoom: %f, angle: %i, alpha: %f",
				overlay->getName().c_str(), layer, offsetX + offsetPosX, offsetY + offsetPosY, zoom, _angle, _alpha);
		overlay->render(frontend, layer, offsetX + offsetPosX, offsetY + offsetPosY, zoom, _angle, _alpha);
	}
}

void ClientEntity::render (IFrontend *frontend, Layer layer, int scale, float zoom, int offsetX, int offsetY) const
{
	if (!_currSprite)
		return;

	const ClientEntityPtr ropeEntity = _ropeEntity;
	const int basePosX = _pos.x * scale * zoom;
	const int basePosY = _pos.y * scale * zoom;
	int posX = basePosX;
	int posY = basePosY;

	switch (_align) {
	case ENTITY_ALIGN_UPPER_LEFT:
		break;
	case ENTITY_ALIGN_LOWER_LEFT:
		posX -= _currSprite->getWidth(layer) * zoom / 2.0f;
		posY += _size.y * scale * zoom / 2.0f;
		posY -= _currSprite->getHeight(layer) * zoom;
		break;
	case ENTITY_ALIGN_MIDDLE_CENTER: {
		posX -= _currSprite->getWidth(layer) * zoom / 2.0f;
		posY -= _currSprite->getHeight(layer) * zoom / 2.0f;
		break;
	}
	}

	setScreenPos(offsetX + posX, offsetY + posY);

	// package upper side
	const int ropeX1 = basePosX;
	const int ropeY1 = (basePosY - _size.y * scale * zoom / 4.0f);
	int ropeX2 = 0;
	int ropeY2 = 0;
	if (ropeEntity && layer == LAYER_MIDDLE) {
		const vec2& pos = ropeEntity->getPos();
		const vec2& size = ropeEntity->getSize();
		// player bottom
		ropeX2 = pos.x * scale * zoom;
		ropeY2 = pos.y * scale * zoom + size.y * scale * zoom / 4.0f;
		Texture* ropeTexture = _ropeTexture.get();
		frontend->renderLineWithTexture(offsetX + ropeX1, offsetY + ropeY1, offsetX + ropeX2, offsetY + ropeY2, ropeTexture);
	}

	const bool visible = _currSprite->render(frontend, layer, _screenPosX, _screenPosY, zoom, _angle, _alpha);
	_visChanged = visible != _visible;

	renderOverlays(frontend, layer, scale, zoom, offsetX, offsetY, posX, posY);

	if (layer != LAYER_FRONT)
		return;

	if (Config.isDebug())
		frontend->renderFilledRect(_screenPosX, _screenPosY, 4, 4, colorRed);

	const bool debug = Config.isDebugEntity();
	if (debug) {
		const BitmapFontPtr& font = UI::get().getFont();

		renderDot(frontend, offsetX + basePosX, offsetY + basePosY, colorGreen);
		renderDot(frontend, offsetX + posX, offsetY + posY, colorWhiteTrue);
		if (ropeEntity && layer == LAYER_MIDDLE) {
			renderDot(frontend, offsetX + ropeX1, offsetY + ropeY1, colorBlue);
			renderDot(frontend, offsetX + ropeX2, offsetY + ropeY2, colorBrightBlue);
		}
		int offsetPosX, offsetPosY;
		calcOffset(scale, zoom, posX, posY, offsetPosX, offsetPosY);
		renderDot(frontend, offsetX + offsetPosX, offsetY + offsetPosY, colorGray);

		int fontY = _screenPosY;
		if (!_animation->isNone()) {
			font->print(_animation->name, colorWhiteTrue, _screenPosX, fontY);
			fontY += font->getTextHeight(_animation->name);
		}
		font->print(string::toString(_angle), colorWhiteTrue, _screenPosX, fontY);
	}
}

inline void ClientEntity::renderDot (IFrontend* frontend, int x, int y, const Color& color) const
{
	frontend->renderFilledRect(x, y, 5, 5, color);
}

void ClientEntity::setAnimationType (const Animation& animation)
{
	_animation = &animation;

	SoundControl.halt(_animationSound);
	_animationSound = -1;
	SoundMappingConstIter soundIter = _soundMapping.find(_animation);
	if (soundIter != _soundMapping.end()) {
		_animationSound = SoundControl.play(soundIter->second, getPos(), _animation->loop);
	}

	const std::string name = SpriteDefinition::get().getSpriteName(_type, *_animation);
	if (name.empty())
		return;

	SpritesMapConstIter i = _sprites.find(_animation);
	if (i == _sprites.end()) {
		_currSprite = SpritePtr(UI::get().loadSprite(name)->copy());
		_currSprite->setLoop(_animation->loop);
		_sprites[_animation] = _currSprite;
	} else {
		_currSprite = i->second;
	}
	if (!_animation->loop)
		_currSprite->setCurrentFrame(0);

	setScreenSize(_currSprite->getMaxWidth(), _currSprite->getMaxHeight());
}

bool ClientEntity::update (uint32_t deltaTime, bool lerpPos)
{
	static const float interval = Constant::FPS_SERVER / (float) Constant::FPS_CLIENT;
	_time += deltaTime;
	if (lerpPos) {
		const vec2 before = _pos - _nextPos;
		if (!before.isZero(0.01f)) {
			const vec2 inc = interval * (_nextPos - _prevPos);
			_pos += inc;
		} else {
			_pos = _nextPos;
		}
	}
	if (_currSprite) {
		_currSprite->update(deltaTime);
	}

	if (_visChanged) {
		_visible ^= true;
		onVisibilityChanged();
	}

	if (_fadeOutTime > 0) {
		float lastFade = _time - _fadeOutTime;
		_fadeOutTime += deltaTime;
		const float fadeStepsPerSecond = 333.0f;
		const float fsps = 1.0f / fadeStepsPerSecond;
		while (lastFade * fsps >= 1.0f) {
			lastFade -= 1.0f / fsps;
		}
		FadeOut(_alpha, lastFade * fsps);
		if (_alpha <= 0.01f)
			return false;
	}

	return true;
}

void ClientEntity::setPos (const vec2& pos, bool lerp)
{
	if (lerp) {
		_prevPos = _nextPos;
		_nextPos = pos;
		_pos = _prevPos;
	} else {
		_prevPos = _nextPos = _pos = pos;
	}
}

Direction ClientEntity::getMoveDirection ()
{
	Direction direction = 0;
	if (_prevPos.x < _pos.x)
		direction |= DIRECTION_RIGHT;
	else if (_prevPos.x > _pos.x)
		direction |= DIRECTION_LEFT;

	if (_prevPos.y < _pos.y)
		direction |= DIRECTION_DOWN;
	else if (_prevPos.y > _pos.y)
		direction |= DIRECTION_UP;

	return direction;
}

ClientEntityPtr ClientEntity::Factory::create (const ClientEntityFactoryContext *ctx) const
{
	ClientEntity *e = new ClientEntity(ctx->type, ctx->id, ctx->x, ctx->y, ctx->width, ctx->height, ctx->soundMapping, ctx->align, ctx->angle);
	e->setAnimationType(ctx->animation);
	return ClientEntityPtr(e);
}

ClientEntity::Factory ClientEntity::FACTORY;
