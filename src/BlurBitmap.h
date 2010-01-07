#ifndef		__BLURBITMAP_H__
#define		__BLURBITMAP_H__

#include "MAXScrpt.h"
#include "GenericMethod.h"
#include "tab.h"

#define			BLURBITMAP_VERSION			1000

applyable_class( BlurBitmap )
class BlurBitmap : public AppliedValue {
	public:
		Bitmap*				source;
		BitmapInfo			info;
		int					stack[STACK_SIZE];
		int					stackPointer;
		int					width;
		int					height;

							BlurBitmap();
							~BlurBitmap(){}

		// Required Value Methods
							classof_methods( BlurBitmap, Value );
		void				collect()		{ delete this; }
		void				sprin1( CharStream* s );
		void				gc_trace();

#define						is_blurbitmap(p) ( (p)->tag == class_tag( BlurBitmap ) )

		// Non-Maxscript Operations
		float				aMax( float val_01, float val_02 );
		void				boundaryFill( int x, int y, AColor boundary, AColor fill );
		float				distance( Point2 a, Point2 b );
		void				drawArea( Array* points, AColor clr );
		Point2				drawFilledArea( Array* points, AColor boundary, AColor fill );
		void				drawLine( Point2 start, Point2 end, AColor clr );
		void				drawLine( float startX, float startY, float endX, float endY, AColor clr );
		void				drawGradientTriangle( Point2 a, Point2 b, Point2 c, AColor colorA, AColor colorB, AColor colorC, int mode = 0 );
		void				drawTriangle( Point2 a, Point2 b, Point2 c, AColor fill, int mode = 0 );
		void				emptyStack();
		void				floodFill( int x, int y, AColor boundary, AColor fill );
		void				fill( AColor clr );
		bool				hasPoint( Point2 point );
		bool				hasPoint( int x, int y );
		bool				isPointInTriangle( Point2 point, Point2 a, Point2 b, Point2 c );
		bool				isWithinThreshold( float r, float g, float b, float r2, float g2, float b2 );
		bool				isWithinThreshold( AColor clr, BMM_Color_fl* colorPtr );
		bool				isWithinThreshold( AColor clr, AColor clr2 );
		AColor				mixTriangleColor( Point2 p, Point2 a, Point2 b, Point2 c, AColor colorA, AColor colorB, AColor colorC );
		AColor				pixelAtPoint( int x, int y, BMM_Color_fl* colorPtr = NULL );
		bool				pop( int &x, int&y );
		bool				push( int x, int y );
		bool				setPixelAtPoint( int x, int y, BMM_Color_fl* colorPtr, int mode = 0 );
		bool				setPixelAtPoint( int x, int y, AColor clr, BMM_Color_fl* colorPtr = NULL, int mode = 0 );
		float				triangleArea( Point2 a, Point2 b, Point2 c );

		// Maxscript Methods
		Value*				get_property( Value** arg_list, int count );
		Value*				set_property( Value** arg_list, int count );
		Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
};

#endif		__BLURBITMAP_H__