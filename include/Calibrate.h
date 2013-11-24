#pragma once

#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Rect.h"

#include "PParams.h"

class Calibrate
{
public:
	Calibrate();

	void setup();

	void mouseDown( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
	void mouseUp  ( ci::app::MouseEvent event );

	const ci::Vec2f transform( const ci::Vec2f pos ) const;
	const ci::Vec2f transform( const ci::Vec2f pos, const ci::Vec2f posRef ) const;
	const ci::Rectf getCoverLeft() const;
	const ci::Rectf getCoverRight() const;
	const ci::Rectf getCoverTop() const;
	const ci::Rectf getCoverBottom() const;

	void reset();

private:
	const ci::Vec2f getTranslate() const;
	const ci::Vec2f getScale() const;

	template< typename T >
	std::string getMinMaxStepString( T min, T max, T step )
	{
		std::stringstream st;
		st << "min=" << min << " max=" << max << " step=" << step;
		return st.str();
	}

private:
	// params
	mndl::params::PInterfaceGl mParams;
	float                    mTranslateX;
	float                    mTranslateY;
	float                    mScaleX;
	float                    mScaleY;

	float                    mCoverLeft;
	float                    mCoverRight;
	float                    mCoverTop;
	float                    mCoverBottom;

	ci::Vec2i                mMousePos;

	static const float       MIN_TRANSLATE;
	static const float       MAX_TRANSLATE;
	static const float       STEP_TRANSLATE;
	static const float       MIN_SCALE;
	static const float       MAX_SCALE;
	static const float       STEP_SCALE;
	static const float       MIN_COVER;
	static const float       MAX_COVER;
	static const float       STEP_COVER;
};

