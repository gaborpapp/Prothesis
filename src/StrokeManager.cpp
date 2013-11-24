#include "StrokeManager.h"

using namespace std;
using namespace ci;
using namespace ci::app;

const int StrokeManager::sGenerateid = 100;

mndl::params::PInterfaceGl StrokeManager::mParams         = mndl::params::PInterfaceGl();
float                StrokeManager::mK              = 0.06f;
float                StrokeManager::mDamping        = 0.7f ;
float                StrokeManager::mStrokeMinWidth = 100.0f ;
float                StrokeManager::mStrokeMaxWidth = 160.0f;
float                StrokeManager::mMaxVelocity    = 40.0f;
Vec2i                StrokeManager::mSize           = Vec2i();


void StrokeManager::setup( Vec2i size )
{
	mSize   = size;

	mParams = mndl::params::PInterfaceGl( "Stroke", Vec2i( 200, 150 ), Vec2i( 16, 176 ) );
	mParams.addPersistentSizeAndPosition();

	mParams.addPersistentParam( "Stiffness"       , &mK             , 0.06f , "min=    0.01 max=    0.2   step= 0.01" );
	mParams.addPersistentParam( "Damping"         , &mDamping       , 0.7f  , "min=    0.25 max=    0.999 step= 0.02" );
	mParams.addPersistentParam( "Stroke min width", &mStrokeMinWidth, 100.0f, "min=    0    max=  500     step= 0.5"  );
	mParams.addPersistentParam( "Stroke max width", &mStrokeMaxWidth, 160.0f, "min= -500    max=  500     step= 0.5"  );
	mParams.addPersistentParam( "Velocity max"    , &mMaxVelocity   , 40.0f , "min=    1    max=  100"                );

	/*
	std::vector< std::pair< std::string, boost::any > > vars;
	vars.push_back( make_pair( "Stiffness", &mK ) );
	vars.push_back( make_pair( "Damping", &mDamping ) );
	vars.push_back( make_pair( "Stroke min width", &mStrokeMinWidth ) );
	vars.push_back( make_pair( "Stroke max width", &mStrokeMaxWidth ) );
	vars.push_back( make_pair( "Velocity max", &mMaxVelocity ) );
	mParams.addSeparator();
	mParams.addPresets( vars );
	*/
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
		stroke->resize( mSize );
		stroke->update();
	}
}

void StrokeManager::draw( const Calibrate &calibrate, const Vec2f &posRef )
{
	for( Strokes::const_iterator it = mStrokes.begin(); it != mStrokes.end(); ++it )
	{
		StrokeRef stroke = it->second;

		stroke->draw( calibrate, posRef );
	}
}

void StrokeManager::addPos( int id, Vec2f pos )
{
	StrokeRef stroke = findStroke( id );

	if( stroke )
		stroke->addPos( pos );
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
	for( Strokes::const_iterator it = mStrokes.begin(); it != mStrokes.end(); ++it )
	{
		StrokeRef stroke = it->second;
		stroke->clear();
	}
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
