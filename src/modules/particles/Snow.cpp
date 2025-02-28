#include "Snow.h"

Snow::Snow(IParticleEnvironment& env) :
		Particle(env), _waterSurface(0)
{
	_texture = loadTexture("snow-01");
	_scale = randBetweenf(0.1f, 0.6f);
	random();
}
void Snow::random () {
	// _a = vec2(
	// 	randBetweenf(-0.00001f, 0.00001f),
	// 	randBetweenf(0.00001f, 0.00001f));
	_v = vec2(
		// 0.0f,
		randBetweenf(-0.03f, 0.09f),
		randBetweenf(0.06f, 0.12f));
	// _omega = 0.3f;
	_omega = randBetweenf(-0.3f, 0.6f);
}

void Snow::init() {
	_s.x = rand() % _env.getPixelWidth();
	// _s.y = rand() % _env.getPixelHeight();
	_waterSurface = _env.getWaterSurface();
	_s.y = rand() % std::min(_waterSurface, _env.getPixelHeight());
}

void Snow::run() {
	// the water height might change, so update this
	_waterSurface = _env.getWaterSurface();
	// const float magnitude = 0.1f;
	// const float amplitude = 0.9f;
	// _v.x = magnitude * sinf(_v.y * amplitude);

	// snow has reached the water surface
	if (_s.y >= _waterSurface - _texture->getHeight()) {
		_s.x = rand() % _env.getPixelWidth();
		_s.y = rand() % (_env.getPixelHeight() / 22);  // top
		random();
	}
}
