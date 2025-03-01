#include "Bubble.h"
#include "common/String.h"
#include "common/Log.h"

#define BUBBLETYPES 9

Bubble::Bubble (IParticleEnvironment& env) :
	Particle(env), _waterSurface(0)//, _waterGround(0), _waterHeight(0), _waterWidth(0)
{
	const int i = rand() % BUBBLETYPES;
	_texture = loadTexture(string::format("bubble-%02i", i + 1));
	_v = vec2(0.0f, -randBetweenf(0.03f, 0.06f));
	_omega = randBetweenf(0.1f, 0.4f);
	_alpha = randBetweenf(0.3f, 0.6f);
	// random();
}

void Bubble::init () {
	water();
	_s.x = rand() % _env.getPixelWidth();
	_s.y = randBetweenf(_waterSurface, _env.getPixelHeight());
	// _a = vec2(0.f, -randBetweenf(0.000f, 0.0001f));
}

void Bubble::water ()
{
	_waterSurface = _env.getWaterSurface();
	// _waterGround = _env.getWaterGround();
	// _waterHeight = _waterGround - _waterSurface + 1;
	// _waterWidth = _env.getWaterWidth();
}

void Bubble::run ()
{
	// if (_waterHeight <= 0.0)
		// return;
	// the water height might change, so update this
	water();
	const float magnitude = 0.1f;
	const float amplitude = 0.5f;
	_v.x = magnitude * sinf(_v.y * amplitude);

	// bubble has reached the water surface
	// if (_s.y <= _waterSurface + _texture->getHeight()) {
	if (_s.y <= (float)_waterSurface) {
		init();
		// _s.x = (float)(rand() % _waterWidth);
		// _s.y = (((float)_waterSurface + (float)_waterHeight / 2.0f) + (float)(rand() % (_waterHeight / 2)));
	}
}
