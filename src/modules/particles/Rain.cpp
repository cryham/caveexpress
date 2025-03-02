#include "Rain.h"

Rain::Rain(IParticleEnvironment& env) :
		Particle(env), _waterSurface(0)
{
	_texture = loadTexture("snow-01");
	float s = randBetweenf(0.1f, 0.6f);
	_scale = vec2(s * 0.2f, s * 4.f);
	_alpha = randBetweenf(0.3f, 0.5f);;
	random();
}
void Rain::random () {
	// _a = vec2(
	// 	randBetweenf(-0.00001f, 0.00001f),
	// 	randBetweenf(0.00001f, 0.00001f));
	_v = vec2(
		// 0.0f,
		randBetweenf(-0.01f, 0.01f),
		randBetweenf(0.3f, 0.6f));
	// _angle = randBetween(1, 121);
	// _omega = 0.3f;
	// _omega = randBetweenf(-0.3f, 0.6f);
}

void Rain::init() {
	_waterSurface = _env.getWaterSurface();
	_s.x = rand() % _env.getPixelWidth();
	_s.y = rand() % std::min(_waterSurface, _env.getPixelHeight());
}

void Rain::run() {
	// the water height might change, so update this
	_waterSurface = _env.getWaterSurface();

	// Rain has reached the water surface
	if (_s.y >= _waterSurface - _texture->getHeight()) {
		_s.x = rand() % _env.getPixelWidth();
		_s.y = rand() % (_env.getPixelHeight() / 32);  // top
		random();
	}
}
