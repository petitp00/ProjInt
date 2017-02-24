#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <algorithm>

enum class TweenType
{
	Linear,
	QuartIn, QuartOut, QuartInOut,
	QuintIn, QuintOut, QuintInOut,
	ExpoIn, ExpoOut, ExpoInOut,
	BackIn, BackOut, BackInOut,
	ElasticOut,
	BounceOut
};

class Tweener
{
public:
	Tweener()=default;
	Tweener(TweenType tween_type, float start_value, float end_value, sf::Time time) { Reset(tween_type, start_value, end_value, time); }

	void Reset(TweenType tween_type, float start_value, float end_value, sf::Time time) {
		this->tween_type = tween_type;
		b = start_value;
		c = end_value - b;
		d = float(time.asMilliseconds());
		clock.restart();
	}
	float Tween()
	{
		float t = std::min(float(clock.getElapsedTime().asMilliseconds()), d);
		float s, postFix;

		switch (tween_type)
		{
		case TweenType::Linear:
			return c*t / d + b;
		case TweenType::QuartIn:
			t/=d;
			return c*t*t*t*t + b;
		case TweenType::QuartOut:
			t/=d;
			--t;
			return -c * (t*t*t*t - 1) + b;
		case TweenType::QuartInOut:
			t /= d / 2;
			if (t < 1) return c / 2 * t*t*t*t + b;
			t -= 2;
			return -c / 2 * (t*t*t*t - 2) + b;
		case TweenType::QuintIn:
			t /= d;
			return c*t*t*t*t*t + b;
		case TweenType::QuintOut:
			t /= d;
			--t;
			return c*(t*t*t*t*t + 1) + b;
		case TweenType::QuintInOut:
			t /= d / 2;
			if (t < 1) return c / 2 * t*t*t*t*t + b;
			t -= 2;
			return c / 2 * (t*t*t*t*t + 2) + b;
		case TweenType::ExpoIn:
			return c * float(std::pow(2, 10 * (t / d - 1))) + b;
		case TweenType::ExpoOut:
			return c * float(-std::pow(2, -10 * t / d) + 1) + b;
		case TweenType::ExpoInOut:
			t /= d / 2;
			if (t < 1) return c / 2 * float(std::pow(2, 10 * (t - 1))) + b;
			t--;
			return c / 2 * float(-std::pow(2, -10 * t) + 2) + b;
		case TweenType::BackIn:
			s = 1.70158f;
			postFix = t /= d;
			return c*(postFix)*t*((s + 1)*t - s) + b;
			break;
		case TweenType::BackOut:
			s = 1.70158f;
			return c*((t=t / d - 1)*t*((s + 1)*t + s) + 1) + b;
		case TweenType::BackInOut:
			s = 1.70158f;
			if ((t/=d / 2) < 1) return c / 2 * (t*t*(((s*=(1.525f)) + 1)*t - s)) + b;
			postFix = t-=2;
			return c / 2 * ((postFix)*t*(((s*=(1.525f)) + 1)*t + s) + 2) + b;
		case TweenType::ElasticOut:
			return (t == d) ? b + c : c * float(-pow(2, -10 * t / d) + 1) + b;
		case TweenType::BounceOut:
			if ((t/=d) < (1 / 2.75f)) {
				return c*(7.5625f*t*t) + b;
			}
			else if (t < (2 / 2.75f)) {
				postFix = t-=(1.5f / 2.75f);
				return c*(7.5625f*(postFix)*t + .75f) + b;
			}
			else if (t < (2.5 / 2.75)) {
				postFix = t-=(2.25f / 2.75f);
				return c*(7.5625f*(postFix)*t + .9375f) + b;
			}
			else {
				postFix = t-=(2.625f / 2.75f);
				return c*(7.5625f*(postFix)*t + .984375f) + b;
			}
		default:
			return 0.f;
		}
	}
	bool getEnded() { return clock.getElapsedTime().asMilliseconds() >= d; }

private:
	TweenType tween_type = TweenType::Linear;

	float b; // start_value
	float c; // change in value (end - start)
	float d; // time in milliseconds
	
	sf::Clock clock;
};

class PosTweener
{
public:
	PosTweener()=default;
	PosTweener(TweenType tween_type, sf::Vector2f start_pos, sf::Vector2f end_pos, sf::Time time) { Reset(tween_type, start_pos, end_pos, time); }

	void Reset(TweenType tween_type, sf::Vector2f start_pos, sf::Vector2f end_pos, sf::Time time) {
		tw_x.Reset(tween_type, start_pos.x, end_pos.x, time);
		tw_y.Reset(tween_type, start_pos.y, end_pos.y, time);
	}

	sf::Vector2f Tween() { return { tw_x.Tween(), tw_y.Tween() }; }
	bool getEnded() { return tw_x.getEnded(); }

private:
	Tweener tw_x, tw_y;
};

