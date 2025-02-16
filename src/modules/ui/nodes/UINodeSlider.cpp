#include "UINodeSlider.h"
#include "common/Log.h"

UINodeSlider::UINodeSlider (IFrontend *frontend, float min, float max, float stepWidth) :
		UINode(frontend), _value(0), _min(min), _max(max), _stepWidth(stepWidth)
{
	setColors(colorGray, colorWhite);
	setStandardPadding();
}

UINodeSlider::~UINodeSlider ()
{
}

void UINodeSlider::setColors (const Color& lineColor, const Color& sliderColor)
{
	Vector4Set(lineColor, _lineColor);
	Vector4Set(sliderColor, _sliderColor);
}

void UINodeSlider::render (int x, int y) const
{
	UINode::render(x, y);
	x += getRenderX();
	y += getRenderY();
	const int w = getRenderWidth();
	const int h = getRenderHeight();
	const int deltaHeight = h / 2;
	const float steps = _max - _min + 1.0f;
	const float stepDelta = std::max(5.0f, w / steps * (_stepWidth < 1.0f ? 1.0f : _stepWidth));
	const int sliderX = x + (_value - _min) / steps * w;
	Log::trace(LOG_UI, "x: %i, y: %i, w: %i, h: %i, stepDelta: %f, sliderX: %i", x, y, w, h, stepDelta, sliderX);
	renderLine(x, y + deltaHeight, x + w, y + deltaHeight, _lineColor);
	if (_enabled)
		renderFilledRect(sliderX, y, stepDelta, h, _sliderColor);
}

inline float UINodeSlider::calculateValue (int32_t x) const
{
	const int steps = _max - _min + 1;
	const float value = _min + (x - getRenderX(false)) * steps / static_cast<float>(getRenderWidth(false));
	return clamp(value - static_cast<float>(fmod(value, _stepWidth)), _min, _max);
}

bool UINodeSlider::onFingerRelease (int64_t finger, uint16_t x, uint16_t y, bool motion)
{
	setValue(calculateValue(x));
	return UINode::onFingerRelease(finger, x, y, motion);
}

bool UINodeSlider::onKeyPress (int32_t key, int16_t modifier)
{
	if (!hasFocus() || !isVisible() || !_enabled)
		return false;

	if (key == SDLK_LEFT) {
		setValue(_value - _stepWidth);
		return true;
	} else if (key == SDLK_RIGHT) {
		setValue(_value + _stepWidth);
		return true;
	}

	return UINode::onKeyPress(key, modifier);
}

bool UINodeSlider::onMouseLeftRelease (int32_t x, int32_t y)
{
	setValue(calculateValue(x));
	return UINode::onMouseLeftRelease(x, y);
}

bool UINodeSlider::onMouseWheel (int32_t x, int32_t y)
{
	const bool retVal = UINode::onMouseWheel(x, y);
	if (x > 0 || y > 0)
		setValue(_value + _stepWidth);
	else
		setValue(_value - _stepWidth);
	return retVal;
}

bool UINodeSlider::onControllerMotion (uint8_t axis, int value, uint32_t id)
{
	if (!hasFocus())
		return false;

	const bool horizontal = axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_RIGHTY;
	if (!horizontal)
		return false;

	const int delta = 10000;
	bool change = false;
	if (value > delta)
		change = setValue(_value + _stepWidth);
	else if (value < -delta)
		change = setValue(_value - _stepWidth);

	UINode::onControllerMotion(axis, value, id);
	return change;
}

bool UINodeSlider::setValue (float value)
{
	if (value > _max)
		return false;
	if (value < _min)
		return false;

	if (fequals(value, _value))
		return false;

	_value = value;

	for (Listeners::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->onValueChanged();
	}
	return true;
}
