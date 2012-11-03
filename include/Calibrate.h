#pragma once

#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Area.h"

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
	const ci::Area  getCoverLeft() const;
	const ci::Area  getCoverRight() const;
	const ci::Area  getCoverTop() const;
	const ci::Area  getCoverBottom() const;

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
	ci::params::PInterfaceGl mParams;
	float                    mTranslateX;
	float                    mTranslateY;
	float                    mScaleX;
	float                    mScaleY;

	int                      mCoverLeft;
	int                      mCoverRight;
	int                      mCoverTop;
	int                      mCoverBottom;

	ci::Vec2i                mMousePos;

	static const float       MIN_TRANSLATE;
	static const float       MAX_TRANSLATE;
	static const float       STEP_TRANSLATE;
	static const float       MIN_SCALE;
	static const float       MAX_SCALE;
	static const float       STEP_SCALE;

	static const int         MIN_COVER;
	static const int         MAX_COVER;
	static const int         STEP_COVER;
};

