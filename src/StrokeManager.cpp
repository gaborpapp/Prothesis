#include "StrokeManager.h"

using namespace std;
using namespace ci;
using namespace ci::app;

const int StrokeManager::sGenerateid = 100;

params::PInterfaceGl StrokeManager::mParams         = params::PInterfaceGl();
float                StrokeManager::mK              = 0.06f;
float                StrokeManager::mDamping        = 0.7f ;
float                StrokeManager::mStrokeMinWidth = 6.0f ;
float                StrokeManager::mStrokeMaxWidth = 16.0f;
float                StrokeManager::mMaxVelocity    = 40.0f;
Vec2i                StrokeManager::mSize           = Vec2i();


void StrokeManager::setup( Vec2i size )
{
	mSize   = size;

	mParams = params::PInterfaceGl( "Stroke", Vec2i( 200, 150 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addPersistentParam( "Stiffness"   , &mK             , 0.06f , "min=    0.01 max=    0.2   step= 0.01" );
	mParams.addPersistentParam( "Damping"     , &mDamping       , 0.7f  , "min=    0.25 max=    0.999 step= 0.02" );
	mParams.addPersistentParam( "Stroke min"  , &mStrokeMinWidth, 100.0f, "min=    0    max=  500     step= 0.5"  );
	mParams.addPersistentParam( "Stroke width", &mStrokeMaxWidth, 160.0f, "min= -500    max=  500     step= 0.5"  );
	mParams.addPersistentParam( "Velocity max", &mMaxVelocity   , 40.0f , "min=    1    max=  100"                );
}

void StrokeManager::update()
{
	for( Strokes::const_iterator it = mStrokes.begin(); it != mStrokes.end(); ++it )
	{
		StrokeRef stroke = it->second;

		stroke->setStiffness     ( mK              );
		stroke->setDamping       ( mDamping        );
		stroke->setStrokeMinWidth( mStrokeMinWidth );
		stroke->setStrokeMaxWidth( mStrokeMaxWidth );
		stroke->setMaxVelocity   ( mMaxVelocity    );
		stroke->resize( ResizeEvent( mSize ));
	}
}

void StrokeManager::draw( const Calibrate &calibrate )
{
	for( Strokes::const_iterator it = mStrokes.begin(); it != mStrokes.end(); ++it )
	{
		StrokeRef stroke = it->second;

		stroke->draw( calibrate );
	}
}

void StrokeManager::addPos( int id, Vec2f pos )
{
	StrokeRef stroke = findStroke( id );

	if( stroke )
		stroke->update( pos );
}

void StrokeManager::setActive( int id, bool active )
{
	StrokeRef stroke = findStroke( id );

	if( stroke )
		stroke->setActive( active );
}

void StrokeManager::setBrush( int id, gl::Texture brush )
{
	StrokeRef stroke = findStroke( id );

	if( stroke )
		stroke->setBrush( brush );
}

void StrokeManager::clear()
{
	mStrokes.clear();
}

int StrokeManager::createStroke( int id /* = -1 */ )
{
	int idStroke = id;

	if( idStroke == -1 )
		idStroke = generateStrokeId();

	if( ! findStroke( idStroke ))
		mStrokes[ idStroke ] = shared_ptr< Stroke >( new Stroke());

	return idStroke;
}

void StrokeManager::destroyStroke( int id )
{
	if( ! findStroke( id ))
		return;

	mStrokes.erase( id );
}

StrokeManager::StrokeRef StrokeManager::findStroke( int id )
{
	Strokes::const_iterator it = mStrokes.find( id );
	if( it == mStrokes.end())
		return StrokeRef();

	return it->second;
}

int StrokeManager::generateStrokeId()
{
	int id = sGenerateid;

	while( findStroke( id ))
		++id;

	return id;
}