
#include "imports.h"

// external name definitions
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "defextfn.h"

// internal name definitions
#include "definsfn.h"
#endif

// Globals
#define			COLOR_MULTIPLIER		255.0f
#define			COLOR_THRESHOLD			0.5f
#define			EDGE_LIMIT				10000000000.0f
#define			LINE_THRESHOLD			0.00001f

// Fill Modes
#define			NORMAL					0
#define			SCREEN					1

// Flood Fill Flags
#define			STACK_SIZE				16777216

// Names
#define			n_bitmap				( Name::intern( _T( "bitmap" ) ) )
#define			n_boundaryFill			( Name::intern( _T( "boundaryFill" ) ) )
#define			n_drawArea				( Name::intern( _T( "drawArea" ) ) )
#define			n_drawFilledArea		( Name::intern( _T( "drawFilledArea" ) ) )
#define			n_drawLine				( Name::intern( _T( "drawLine" ) ) )
#define			n_drawGradientTriangle	( Name::intern( _T( "drawGradientTriangle" ) ) )
#define			n_drawTriangle			( Name::intern( _T( "drawTriangle" ) ) )
#define			n_show					( Name::intern( _T( "show" ) ) )
#define			n_fill					( Name::intern( _T( "fill" ) ) )
#define			n_floodFill				( Name::intern( _T( "floodFill" ) ) )
#define			n_hasPoint				( Name::intern( _T( "hasPoint" ) ) )
#define			n_height				( Name::intern( _T( "height" ) ) )
#define			n_mode					( Name::intern( _T( "mode" ) ) )
#define			n_normal				( Name::intern( _T( "normal" ) ) )1
#define			n_pixelAtPoint			( Name::intern( _T( "pixelAtPoint" ) ) )
#define			n_saveToFile			( Name::intern( _T( "saveToFile" ) ) )
#define			n_screen				( Name::intern( _T( "screen" ) ) )
#define			n_setPixelAtPoint		( Name::intern( _T( "setPixelAtPoint" ) ) )
#define			n_width					( Name::intern( _T( "width" ) ) )

#include "BlurBitmap.h"

//----------------------------------------		BlurBitmap			----------------------------------------

visible_class_instance( BlurBitmap, "BlurBitmap" )

Value* BlurBitmapClass::apply( Value** arg_list, int count, CallContext* cc )
{
	check_arg_count_with_keys( BlurBitmap, 0, count );

	one_typed_value_local( BlurBitmap* result );
	vl.result			= new BlurBitmap();

	// Get Width Height from keys
	vl.result->width	= 512;
	Value* arg			= key_arg( width );
	if ( arg != &unsupplied ) vl.result->width	= arg->eval()->to_int();

	vl.result->height	= vl.result->width;
	arg					= key_arg( height );
	if ( arg != &unsupplied ) vl.result->height = arg->eval()->to_int();

	// Get Bitmap from keys
	arg											= key_arg( bitmap );
	if ( arg != &unsupplied ) {
		MAXBitMap* tempBMP	= (MAXBitMap*) arg->eval();
		vl.result->source	= tempBMP->bm;
		vl.result->info		= tempBMP->bi;
	}
	else {
		// Initialize the BitmapInfo instance for this value
		vl.result->info.SetType( BMM_TRUE_64 );
		vl.result->info.SetFlags( MAP_HAS_ALPHA );
		vl.result->info.SetCustomFlag(0);
		vl.result->info.SetWidth( vl.result->width );
		vl.result->info.SetHeight( vl.result->height );

		vl.result->source	= TheManager->Create( &vl.result->info );
	}

	return_value( vl.result );
}

BlurBitmap::BlurBitmap()
{
	tag = class_tag( BlurBitmap );
	source = NULL;
}

void BlurBitmap::sprin1( CharStream* s )
{
	s->puts( _T("(BlurBitmap)") );
}

void BlurBitmap::gc_trace()
{
	Value::gc_trace();
}

float BlurBitmap::aMax( float value01, float value02 )
{
	float absValue01 = abs( value01 );
	float absValue02 = abs( value02 );

	return ( absValue01 > absValue02 ) ? absValue01 : absValue02;
}

void BlurBitmap::boundaryFill( int x, int y, AColor boundary, AColor fill )
{
	emptyStack();

	BMM_Color_fl* tempPtr = new BMM_Color_fl();

	int y1;
	bool spanLeft, spanRight;

	if (!push(x,y))
		return;

	while( this->pop(x,y) )
	{
		y1 = y;
		while ( y1 >= 0 && !isWithinThreshold( pixelAtPoint( x, y1, tempPtr ), boundary ) && !isWithinThreshold( pixelAtPoint( x, y1, tempPtr ), fill ) )
			y1--;
		y1++;
		spanLeft = spanRight = 0;
		AColor pixel;
		while ( y1 < height && !isWithinThreshold( pixelAtPoint( x,y1, tempPtr ), boundary ) && !isWithinThreshold( pixelAtPoint( x,y1, tempPtr ), fill ) )
		{
			setPixelAtPoint( x, y1, fill, tempPtr );

			pixel = pixelAtPoint( x-1, y1 );
			if ( !spanLeft && x > 0 && !isWithinThreshold( pixel, boundary ) && !isWithinThreshold( pixel, fill ) ) {
				if ( !push( x-1, y1 ) )
					return;
				spanLeft = 1;
			}
			else if ( spanLeft && x > 0 && isWithinThreshold( pixel, boundary ) || isWithinThreshold( pixel, fill ) )
			{
				spanLeft = 0;
			}

			pixel = pixelAtPoint( x+1, y1 );
			if ( !spanRight && x < width - 1 && !isWithinThreshold( pixel, boundary ) && !isWithinThreshold( pixel, fill ) ) {
				if ( !push( x+1, y1))
					return;
				spanRight = 1;
			}
			else if ( spanRight && x < width - 1 && isWithinThreshold( pixel, boundary ) || isWithinThreshold( pixel, fill ) ) {
				spanRight = 0;
			}
			y1++;
		}
	}
	free( tempPtr );
}

float BlurBitmap::distance( Point2 a, Point2 b )
{
	return sqrt( pow( ( b.x - a.x ), 2 ) + pow( ( b.y - a.y ), 2 ) );
}

void BlurBitmap::drawArea( Array* points, AColor clr )
{
	for ( int i = 1; i < points->size; i++ )
		drawLine( points->data[i-1]->to_point2(), points->data[i]->to_point2(), clr );
	drawLine( points->data[ points->size - 1 ]->to_point2(), points->data[0]->to_point2(), clr );
}

Point2 BlurBitmap::drawFilledArea( Array* points, AColor boundary, AColor fill )
{
	Point2 center;
	center.x = 0;
	center.y = 0;
	if ( points->size > 0 ) {
		float xTotal = 0;
		float yTotal = 0;

		// First, draw boundary lines
		for ( int i = 1; i < points->size; i++ ) {
			center = points->data[ i-1 ]->to_point2();
			xTotal += center.x;
			yTotal += center.y;
			drawLine( center, points->data[i]->to_point2(), boundary );
		}
		center = points->data[ points->size - 1 ]->to_point2();
		xTotal += center.x;
		yTotal += center.y;
		drawLine( center, points->data[0]->to_point2(), boundary );

		// Now fill with desired color
		float size = points->size;
		center.x = floor( xTotal / size );
		center.y = floor( yTotal / size );
		boundaryFill( (int) center.x, (int) center.y, boundary, fill );
	}
	return center;
}

void BlurBitmap::drawLine( Point2 startPoint, Point2 endPoint, AColor clr )
{
	drawLine( startPoint.x, startPoint.y, endPoint.x, endPoint.y, clr );
}

void BlurBitmap::drawLine( float startX, float startY, float endX, float endY, AColor clr )
{
	BMM_Color_fl* lineColor = new BMM_Color_fl();
	lineColor->r = clr.r;
	lineColor->g = clr.g;
	lineColor->b = clr.b;

	float dx = endX - startX;
	float dy = endY - startY;
	float t = (float) 0.5;

	source->PutPixels( startX, startY, 1, lineColor );
	if ( abs( dx ) > abs( dy ) ) {
		float m = dy/dx;
		t += startY;
		dx = ( dx < 0 ) ? -1 : 1;
		m *= dx;
		int currX = startX;
		while ( currX != endX ) {
			currX += dx;
			t += m;
			source->PutPixels( currX, (int) t, 1, lineColor );
		}
	}
	else if ( dy != 0 ) {
		float m = dx/dy;
		t += startX;
		dy = ( dy < 0 ) ? -1 : 1;
		m *= dy;
		int currY = startY;
		while ( currY != endY ) {
			currY += dy;
			t += m;
			source->PutPixels( (int) t, currY, 1, lineColor );
		}
	}
	free( lineColor );
}

void BlurBitmap::drawGradientTriangle( Point2 a, Point2 b, Point2 c, AColor colorA, AColor colorB, AColor colorC, int mode )
{
	int minX, minY = 10000;
	int maxX, maxY = -1000;

	// Calculate Minimum Point
	if ( a.x < b.x )
		minX = ( a.x < c.x ) ? a.x : c.x;
	else
		minX = ( b.x < c.x ) ? b.x : c.x;

	if ( a.y < b.y )
		minY = ( a.y < c.y ) ? a.y : c.y;
	else
		minY = ( b.y < c.y ) ? b.y : c.y;

	// Calculate Maximum Point
	if ( a.x > b.x )
		maxX = ( a.x > c.x ) ? a.x : c.x;
	else
		maxX = ( b.x > c.x ) ? b.x : c.x;

	if ( a.y > b.y )
		maxY = ( a.y > c.y ) ? a.y : c.y;
	else
		maxY = ( b.y > c.y ) ? b.y : c.y;

	// Fill all points within the Triangle
	Point2 currPoint;
	int rowWidth = maxX - minX;

	BMM_Color_fl* row		= (BMM_Color_fl* ) malloc( rowWidth * sizeof(BMM_Color_fl));
	for ( int y = minY; y <= maxY; y++ ) {
		source->GetPixels( minX, y, rowWidth, row );
		for ( int x = 0; x < rowWidth; x++ ) {
			currPoint.x = minX + x;
			currPoint.y = y;
			if ( isPointInTriangle( currPoint, a, b, c ) ) {
				AColor mixColor = mixTriangleColor( currPoint, a, b, c, colorA, colorB, colorC );
				if ( mode == SCREEN ) {
					if ( row[x].r < mixColor.r ) {
						if ( row[x].g < mixColor.g ) {
							if ( row[x].b < mixColor.b ) {
								row[x].r = mixColor.r;
								row[x].g = mixColor.g;
								row[x].b = mixColor.b;
								row[x].a = mixColor.a;
							}
						}
					}
				}
				else {
					row[x].r = mixColor.r;
					row[x].g = mixColor.g;
					row[x].b = mixColor.b;
					row[x].a = mixColor.a;
				}
			}
		}
		source->PutPixels( minX, y, rowWidth, row );
	}
	free( row );
}

void BlurBitmap::drawTriangle( Point2 a, Point2 b, Point2 c, AColor fill, int mode )
{
	int minX, minY = 10000;
	int maxX, maxY = -1000;

	// Calculate Minimum Point
	if ( a.x < b.x )
		minX = ( a.x < c.x ) ? a.x : c.x;
	else
		minX = ( b.x < c.x ) ? b.x : c.x;

	if ( a.y < b.y )
		minY = ( a.y < c.y ) ? a.y : c.y;
	else
		minY = ( b.y < c.y ) ? b.y : c.y;

	// Calculate Maximum Point
	if ( a.x > b.x )
		maxX = ( a.x > c.x ) ? a.x : c.x;
	else
		maxX = ( b.x > c.x ) ? b.x : c.x;

	if ( a.y > b.y )
		maxY = ( a.y > c.y ) ? a.y : c.y;
	else
		maxY = ( b.y > c.y ) ? b.y : c.y;

	// Fill all points within the Triangle
	Point2 currPoint;
	int rowWidth = maxX - minX;

	BMM_Color_fl* row		= (BMM_Color_fl* ) malloc( rowWidth * sizeof(BMM_Color_fl));
	for ( int y = minY; y <= maxY; y++ ) {
		this->source->GetPixels( minX, y, rowWidth, row );
		for ( int x = 0; x < rowWidth; x++ ) {
			currPoint.x = minX + x;
			currPoint.y = y;
			if ( isPointInTriangle( currPoint, a, b, c ) ) {
				row[x].r = fill.r;
				row[x].g = fill.g;
				row[x].b = fill.b;
				row[x].a = fill.a;
			}
		}
		source->PutPixels( minX, y, rowWidth, row );
	}
	free( row );
}

void BlurBitmap::emptyStack()
{
	int x,y;
	while(pop(x,y));
}

void BlurBitmap::fill( AColor clr )
{
	BMM_Color_fl *line, *lp;
	BMM_Color_fl color;
	color.r = clr.r;
	color.g = clr.g;
	color.b = clr.b;
	color.a = clr.a;
	if ( ( line = (BMM_Color_fl* )calloc(width, sizeof(BMM_Color_fl)))) {
		int ix, iy;
		for ( ix = 0, lp = line; ix < width; ix++, lp++ )
			*lp = color;
		for ( iy = 0; iy < height; iy++ )
			source->PutPixels( 0, iy, width, line );
		free( line );
	}
}

void BlurBitmap::floodFill( int x, int y, AColor old, AColor fill )
{
	if ( isWithinThreshold( old, fill ) )
		return;

	emptyStack();

	BMM_Color_fl* tempPtr = new BMM_Color_fl();

	int y1;
	bool spanLeft, spanRight;

	if (!push(x,y))
		return;

	while( pop(x,y) ) {
		y1 = y;
		while ( y1 >= 0 && isWithinThreshold( pixelAtPoint( x, y1, tempPtr ), old ) )
			y1--;
		y1++;
		spanLeft = spanRight = 0;
		AColor pixel;
		while ( y1 < height && isWithinThreshold( pixelAtPoint( x,y1, tempPtr ), old ) ) {
			setPixelAtPoint( x, y1, fill, tempPtr );
			pixel = pixelAtPoint( x-1, y1 );

			if ( !spanLeft && x > 0 && isWithinThreshold( pixel, old ) ) {
				if ( !push( x-1, y1 ) )
					return;
				spanLeft = 1;
			}
			else if ( spanLeft && x > 0 && !isWithinThreshold( pixel, old ) ) {
				spanLeft = 0;
			}

			pixel = pixelAtPoint( x+1, y1 );
			if ( !spanRight && x < width - 1 && isWithinThreshold( pixel, old ) ) {
				if ( !(push( x+1, y1)))
					return;
				spanRight = 1;
			}
			else if ( spanRight && x < width - 1 && !isWithinThreshold( pixel, old ) ) {
				spanRight = 0;
			}
			y1++;
		}
	}
	free( tempPtr );
}

bool BlurBitmap::hasPoint( Point2 point )
{
	return hasPoint( point.x, point.y );
}

bool BlurBitmap::hasPoint( int x, int y )
{
	if ( 0 <= x && x < width )
		if ( 0 <= y && y < height )
			return true;
	return false;
}

bool BlurBitmap::isPointInTriangle( Point2 point, Point2 a, Point2 b, Point2 c )
{
	float areaOne = triangleArea( point, b, c );
	float areaTwo = triangleArea( point, a, c );
	float areaThree = triangleArea( point, a, b );
	float areaTriangle = triangleArea( a, b, c );
	return ( ( areaOne + areaTwo + areaThree ) > areaTriangle ) ? false : true;
}

bool BlurBitmap::isWithinThreshold( float r, float g, float b, float r2, float g2, float b2 )
{
	if ( abs( (r * COLOR_MULTIPLIER) - (r2 * COLOR_MULTIPLIER) ) <= COLOR_THRESHOLD )
		if ( abs( (g * COLOR_MULTIPLIER) - (g2 * COLOR_MULTIPLIER) ) <= COLOR_THRESHOLD )
			if ( abs( (b * COLOR_MULTIPLIER) - (b2 * COLOR_MULTIPLIER) ) <= COLOR_THRESHOLD )
				return true;
	return false;
}

bool BlurBitmap::isWithinThreshold( AColor clr, BMM_Color_fl* colorPtr )
{
	return this->isWithinThreshold( clr.r, clr.g, clr.b, colorPtr->r, colorPtr->g, colorPtr->b );
}

bool BlurBitmap::isWithinThreshold( AColor clr, AColor clr2 )
{
	return this->isWithinThreshold( clr.r, clr.g, clr.b, clr2.r, clr2.g, clr2.b );
}

AColor BlurBitmap::mixTriangleColor( Point2 p, Point2 a, Point2 b, Point2 c, AColor colorA, AColor colorB, AColor colorC )
{
	float PA = distance( p, a );
	float PB = distance( p, b );
	float PC = distance( p, c );
	float total = PA + PB + PC;

	if ( PA == 0 )
		return colorA;
	else if ( PB == 0 )
		return colorB;
	else if ( PC == 0 )
		return colorC;
	else {
		AColor outColor;
		float invNormalPA = 1 / ( PA / total );
		float invNormalPB = 1 / ( PB / total );
		float invNormalPC = 1 / ( PC / total );
		
		float normalPA = invNormalPA / ( invNormalPA + invNormalPB + invNormalPC );
		float normalPB = invNormalPB / ( invNormalPA + invNormalPB + invNormalPC );
		float normalPC = invNormalPC / ( invNormalPA + invNormalPB + invNormalPC );

		outColor.r = colorA.r * normalPA + colorB.r * normalPB + colorC.r * normalPC;
		outColor.g = colorA.g * normalPA + colorB.g * normalPB + colorC.g * normalPC;
		outColor.b = colorA.b * normalPA + colorB.b * normalPB + colorC.b * normalPC;
		outColor.a = colorA.a * normalPA + colorB.a * normalPB + colorC.a * normalPC;
		return outColor;
	}
}

AColor BlurBitmap::pixelAtPoint( int x, int y, BMM_Color_fl* colorPtr )
{
	AColor outColor;
	if ( hasPoint( x, y ) ) {
		bool createPtr = colorPtr == NULL;
		if ( createPtr )
			colorPtr = new BMM_Color_fl();

		source->GetPixels( x, y, 1, colorPtr );
		outColor.r = colorPtr->r;
		outColor.g = colorPtr->g;
		outColor.b = colorPtr->b;

		if ( createPtr )
			free( colorPtr );
	}
	return outColor;
}

bool BlurBitmap::pop( int &x, int &y )
{
	if ( stackPointer > 0 ) {
		int p = stack[ stackPointer ];
		x = p / height;
		y = p % height;
		stackPointer--;
		return 1;
	}
	else return 0;
}

bool BlurBitmap::push( int x, int y )
{
	if ( stackPointer < STACK_SIZE - 1 ) {
		stackPointer++;
		stack[ stackPointer ] = height * x + y;
		return 1;
	}
	else return 0;
}

bool BlurBitmap::setPixelAtPoint( int x, int y, BMM_Color_fl* colorPtr, int mode )
{
	if ( hasPoint( x,y ) ) {
		if ( mode == SCREEN ) {
			BMM_Color_fl* temp = new BMM_Color_fl();
			source->GetPixels( x, y, 1, temp );
			if ( temp->r < colorPtr->r ) {
				if ( temp->g < colorPtr->g ) {
					if ( temp->b < colorPtr-> b ) {
						source->PutPixels( x, y, 1, colorPtr );
						return true;
					}
				}
			}
		}
		else {
			source->PutPixels( x, y, 1, colorPtr );
			return true;
		}
	}
	return false;
}

bool BlurBitmap::setPixelAtPoint( int x, int y, AColor clr, BMM_Color_fl* colorPtr, int mode )
{
	bool success = false;
	if ( hasPoint( x,y ) ) {
		bool createPtr = colorPtr == NULL;
		if ( createPtr )
			colorPtr = new BMM_Color_fl();

		colorPtr->r = clr.r;
		colorPtr->g = clr.g;
		colorPtr->b = clr.b;

		success = setPixelAtPoint( x, y, colorPtr, mode );

		if ( createPtr )
			free( colorPtr );
	}
	return success;
}

float BlurBitmap::triangleArea( Point2 a, Point2 b, Point2 c ) {
	float area_a = a.x - c.x;
	float area_b = a.y - c.y;
	float area_c = b.x - c.x;
	float area_d = b.y - c.y;
	return ( 0.5 * abs( ( area_a * area_d ) - ( area_b * area_c ) ) );
}

//------------------------------------------- MAXScript Exposure ----------------------------------------

Value * BlurBitmap::get_property( Value** arg_list, int count )
{
	Value* key = arg_list[0];
	one_value_local( result );

	if ( key == n_bitmap )
		vl.result = new MAXBitMap( info, source );
	else if ( key == n_boundaryFill )
		vl.result = NEW_GENERIC_METHOD( boundaryFill );
	else if ( key == n_drawArea )
		vl.result = NEW_GENERIC_METHOD( drawArea );
	else if ( key == n_drawFilledArea )
		vl.result = NEW_GENERIC_METHOD( drawFilledArea );
	else if ( key == n_drawLine )
		vl.result = NEW_GENERIC_METHOD( drawLine );
	else if ( key == n_drawGradientTriangle )
		vl.result = NEW_GENERIC_METHOD( drawGradientTriangle );
	else if ( key == n_drawTriangle )
		vl.result = NEW_GENERIC_METHOD( drawTriangle );
	else if ( key == n_fill )
		vl.result = NEW_GENERIC_METHOD( fill );
	else if ( key == n_floodFill )
		vl.result = NEW_GENERIC_METHOD( floodFill );
	else if ( key == n_hasPoint )
		vl.result = NEW_GENERIC_METHOD( hasPoint );
	else if ( key == n_height )
		vl.result = Integer::intern( height );
	else if ( key == n_pixelAtPoint )
		vl.result = NEW_GENERIC_METHOD( pixelAtPoint );
	else if ( key == n_saveToFile )
		vl.result = NEW_GENERIC_METHOD( saveToFile );
	else if	( key == n_show )
		vl.result = NEW_GENERIC_METHOD( show );
	else if ( key == n_setPixelAtPoint )
		vl.result = NEW_GENERIC_METHOD( setPixelAtPoint );
	else if ( key == n_width )
		vl.result = Integer::intern( width );
	else
		throw RuntimeError( key->to_string(), _T(" is not a member of BlurBitmap") );

	return_value( vl.result );
}

Value * BlurBitmap::set_property( Value** arg_list, int count )
{
	return &undefined;
}

Value * BlurBitmap::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc )
{
	one_value_local( result );
	vl.result = &ok;

	if ( methodID == n_boundaryFill ) {
		check_arg_count( boundaryFill, 3, count );
		int x = arg_list[0]->to_point2().x;
		int y = arg_list[0]->to_point2().y;
		if ( hasPoint( x, y ) ) {
			boundaryFill( x, y, arg_list[1]->to_acolor(), arg_list[2]->to_acolor() );
		}
	}
	else if ( methodID == n_show ) {
		check_arg_count( show, 0, count );
		source->Display();
	}
	else if ( methodID == n_drawArea ) {
		check_arg_count( drawArea, 2, count );
		drawArea( (Array*) arg_list[0]->eval(), arg_list[1]->eval()->to_acolor() );
	}
	else if ( methodID == n_drawFilledArea ) {
		check_arg_count( drawFilledArea, 3, count );
		Point2 center = drawFilledArea( (Array*) arg_list[0]->eval(), arg_list[1]->eval()->to_acolor(), arg_list[2]->eval()->to_acolor() );
		vl.result = new Point2Value( center );
	}
	else if ( methodID == n_drawLine ) {
		check_arg_count( drawLine, 3, count );
		drawLine( arg_list[0]->eval()->to_point2(), arg_list[1]->eval()->to_point2(), arg_list[2]->eval()->to_acolor() );
	}
	else if ( methodID == n_drawGradientTriangle ) {
		check_arg_count_with_keys( drawGradientTriangle, 6, count );
		int mode = NORMAL;
		Value* modeArg	= key_arg( mode );
		if ( modeArg == n_screen ) mode = SCREEN;

		drawGradientTriangle(
			arg_list[0]->eval()->to_point2(),
			arg_list[1]->eval()->to_point2(),
			arg_list[2]->eval()->to_point2(),
			arg_list[3]->eval()->to_acolor(),
			arg_list[4]->eval()->to_acolor(),
			arg_list[5]->eval()->to_acolor(), mode );
	}
	else if ( methodID == n_drawTriangle ) {
		check_arg_count_with_keys( drawTriangle, 4, count );
		int mode = NORMAL;

		Value* modeArg = key_arg( mode );
		if ( modeArg == n_screen )
			mode = SCREEN;
		
		drawTriangle( arg_list[0]->eval()->to_point2(), arg_list[1]->eval()->to_point2(), arg_list[2]->eval()->to_point2(), arg_list[3]->to_acolor(), mode );
	}
	else if ( methodID == n_fill ) {
		check_arg_count( fill, 1, count );
		fill( arg_list[0]->to_acolor() );
	}
	else if ( methodID == n_floodFill ) {
		check_arg_count( floodFill, 2, count );
		int x = arg_list[0]->to_point2().x;
		int y = arg_list[0]->to_point2().y;
		if ( hasPoint( x, y ) ) {
			floodFill( x, y, pixelAtPoint( x,y ), arg_list[1]->to_acolor() );
		}
	}
	else if ( methodID == n_hasPoint ) {
		check_arg_count( hasPoint, 1, count );
		vl.result = ( hasPoint( arg_list[0]->eval()->to_point2() ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_pixelAtPoint ) { 
		check_arg_count( pixelAtPoint, 1, count );
		vl.result = ColorValue::intern( pixelAtPoint( arg_list[0]->eval()->to_point2().x, arg_list[0]->eval()->to_point2().y ) );
	}
	else if ( methodID == n_saveToFile ) {
		check_arg_count_with_keys( saveToFile, 1, count );

		int frame = BMM_SINGLEFRAME;
		Value* arg = key_arg( frame );
		if ( arg != &unsupplied )
			frame = arg->to_int();

		info.SetName( arg_list[0]->to_string() );
		source->OpenOutput( &info );
		source->Write( &info, frame );
		source->Close( &info );
	}
	else if ( methodID == n_setPixelAtPoint ) { 
		check_arg_count( setPixelAtPoint, 2, count );
		vl.result = ( setPixelAtPoint( arg_list[0]->eval()->to_point2().x, arg_list[0]->eval()->to_point2().y, arg_list[1]->to_acolor() ) ) ? &true_value : &false_value;
	}
	return_value( vl.result );
}

//----------------------------------------		BlurBitmapInit		----------------------------------------

void BlurBitmapInit()
{
	mprintf( _T("------ blurBitmap Loaded - Version:%.2f ------\n"), ((float) BLURBITMAP_VERSION) / 1000.0f );
}
