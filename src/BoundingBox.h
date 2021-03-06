/*!
	\file		BoundingBox.h

	\remarks	Maxscript extension to create a new value for 3d bounding boxes
	
	\author		Diego Garcia Huerta
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		05/01/07

	\history
				- version 1.0 DGH 05/03/07: Created

	\note
				Copyright (c) 2006, Blur Studio Inc.
				All rights reserved.

				Redistribution and use in source and binary forms, with or without 
				modification, are permitted provided that the following conditions 
				are met:

					* Redistributions of source code must retain the above copyright 
					notice, this list of conditions and the following disclaimer.
					* Redistributions in binary form must reproduce the above 
					copyright notice, this list of conditions and the following 
					disclaimer in the documentation and/or other materials provided 
					with the distribution.
					* Neither the name of the Blur Studio Inc. nor the names of its 
					contributors may be used to endorse or promote products derived 
					from this software without specific prior written permission.

				THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
				"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
				LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
				FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
				COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
				INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
				BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
				LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
				CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
				LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
				ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
				POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _H_BOUNDINGBOX_CLASS
#define _H_BOUNDINGBOX_CLASS



#ifndef MAX_FLOAT
#define MAX_FLOAT ((float)1.0e38)
#endif


//-------------------------------------------------------------------
/**
    @class line2
    @brief a line in 2d space
*/

/**
    @class line3
    @brief a line in 3d space

    - 04-Dec-01   floh    bugfix: line3::len() was broken
*/
//-------------------------------------------------------------------


//-------------------------------------------------------------------
class line2 {
public:
    Point2 b;
    Point2 m;

    line2() {};
    line2(const Point2& v0, const Point2& v1) : b(v0), m(v1-v0) {};
    line2(const line2& l) : b(l.b), m(l.m) {};

    //--- minimal distance of point to line -------------------------
/*
    float distance(const Point2&) {
        return 0.0f;
    };
*/

    Point2& start(void) {
        return b;
    };
    Point2 end(void) {
        return (b+m);
    };
    float len(void) {
        return m.Length();
    };
    //--- get 3d point on line given t ------------------------------
    Point2 ipol(const float t) const {
        return Point2(b + m*t);
    };
};
//-------------------------------------------------------------------
class line3 {
public:
    Point3 b;
    Point3 m;

    line3() {};
    line3(const Point3& v0, const Point3& v1) : b(v0), m(v1-v0) {};
    line3(const line3& l) : b(l.b), m(l.m) {};

    void set(const Point3& v0, const Point3& v1) {
        b = v0;
        m = v1-v0;
    };
    const Point3& start(void) const
    {
        return b;
    };
    Point3 end(void) const
    {
        return (b+m);
    };
    float len(void) const
    {
        return m.Length();
    };
    //--- minimal distance of point to line -------------------------
    float distance(const Point3& p) {
        Point3 diff(p-b);
        float l = (m % m);
        if (l > 0.0f) {
            float t = (m % diff) / l;
            diff = diff - m*t;
            return diff.Length();
        } else {
            // line is really a point...
            Point3 v(p-b);
            return v.Length();
        }
    };

    //--- get 3d point on line given t ------------------------------
    Point3 ipol(const float t) const {
        return Point3(b + m*t);
    };
};
//-------------------------------------------------------------------

//-------------------------------------------------------------------
/**
    @class bbox3Plus
    @brief (non-oriented) bounding box
*/
//-------------------------------------------------------------------

//-------------------------------------------------------------------
//  bbox3Plus
//-------------------------------------------------------------------
class bbox3Plus {
public:
    Point3 vmin;
    Point3 vmax;
	BOOL empty;

    enum {
        CLIP_LEFT   = (1<<0),
        CLIP_RIGHT  = (1<<1),
        CLIP_BOTTOM = (1<<2),
        CLIP_TOP    = (1<<3),
        CLIP_NEAR   = (1<<4),
        CLIP_FAR    = (1<<5),
    };

    enum {
        OUTSIDE     = 0,
        ISEQUAL     = (1<<0),
        ISCONTAINED = (1<<1),
        CONTAINS    = (1<<2),
        CLIPS       = (1<<3),
    };

    //--- constructors ----------------------------------------------
    bbox3Plus() { empty = TRUE; };
    bbox3Plus(const Point3& _vmin, const Point3& _vmax) : vmin(_vmin), vmax(_vmax) { empty = FALSE; };
    bbox3Plus(const bbox3Plus& bb) : vmin(bb.vmin), vmax(bb.vmax) { empty = FALSE; };
    
    //--- initialize from Point3 cloud -----------------------------
    bbox3Plus(Point3 *varray, int num) {
		empty = FALSE;
        vmin = varray[0];
        vmax = varray[0];
        int i;
        for (i=0; i<num; i++) {
            if      (varray[i].x<vmin.x) vmin.x=varray[i].x;
            else if (varray[i].x>vmax.x) vmax.x=varray[i].x;
            if      (varray[i].y<vmin.y) vmin.y=varray[i].y;
            else if (varray[i].y>vmax.y) vmax.y=varray[i].y;
            if      (varray[i].z<vmin.z) vmin.z=varray[i].z;
            else if (varray[i].z>vmax.z) vmax.z=varray[i].z;
        }
    };

    //--- setting elements ------------------------------------------
    void set(const Point3& _vmin, const Point3& _vmax) {
		empty = FALSE;
        vmin = _vmin;
        vmax = _vmax;
    };
    void set(Point3 *varray, int num) {
		empty = FALSE;
        vmin = varray[0];
        vmax = varray[0];
        int i;
        for (i=0; i<num; i++) {
            if      (varray[i].x<vmin.x) vmin.x=varray[i].x;
            else if (varray[i].x>vmax.x) vmax.x=varray[i].x;
            if      (varray[i].y<vmin.y) vmin.y=varray[i].y;
            else if (varray[i].y>vmax.y) vmax.y=varray[i].y;
            if      (varray[i].z<vmin.z) vmin.z=varray[i].z;
            else if (varray[i].z>vmax.z) vmax.z=varray[i].z;
        }
    };

    //--- invalidate bounding box to prepare for growing ------------
	void makeEmpty (void) 
	{
		vmin.Set(+MAX_FLOAT,+MAX_FLOAT,+MAX_FLOAT);
		vmax.Set(-MAX_FLOAT,-MAX_FLOAT,-MAX_FLOAT);
		empty = TRUE;		
	};

	BOOL isEmpty (void)
	{
		return empty;
	};

    void begin_grow(void) {
        makeEmpty();
    };

    void grow(const Point3& v) {
		if (empty) 
		{
			makeEmpty();
			empty = FALSE;
		}

        if (v.x<vmin.x) vmin.x=v.x;
        if (v.x>vmax.x) vmax.x=v.x;
        if (v.y<vmin.y) vmin.y=v.y;
        if (v.y>vmax.y) vmax.y=v.y;
        if (v.z<vmin.z) vmin.z=v.z;
        if (v.z>vmax.z) vmax.z=v.z;
    };
    void grow(Point3 *varray, int num) {
		if (empty) 
		{
			makeEmpty();
			empty = FALSE;
		}

        int i;
        for (i=0; i<num; i++) {
            grow(varray[i]);
        }
    };

    void grow(const bbox3Plus& bb) {
		if (empty) 
		{
			makeEmpty();
			empty = FALSE;
		}

        if (bb.vmin.x<vmin.x) vmin.x=bb.vmin.x;
        if (bb.vmin.y<vmin.y) vmin.y=bb.vmin.y;
        if (bb.vmin.z<vmin.z) vmin.z=bb.vmin.z;
        if (bb.vmax.x>vmax.x) vmax.x=bb.vmax.x;
        if (bb.vmax.y>vmax.y) vmax.y=bb.vmax.y;
        if (bb.vmax.z>vmax.z) vmax.z=bb.vmax.z;
    };

    // get point of intersection of 3d line with planes
    // on const x,y,z
    bool isect_const_x(const float x, const line3& l, Point3& out) const {
        if (l.m.x != 0.0f) {
            float t = (x - l.b.x) / l.m.x;
            if ((t>=0.0f) && (t<=1.0f)) {
                // point of intersection...
                out = l.ipol(t);
                return true;
            }
        }
        return false;
    }
    bool isect_const_y(const float y, const line3& l, Point3& out) const {
        if (l.m.y != 0.0f) {
            float t = (y - l.b.y) / l.m.y;
            if ((t>=0.0f) && (t<=1.0f)) {
                // point of intersection...
                out = l.ipol(t);
                return true;
            }
        }
        return false;
    }
    bool isect_const_z(const float z, const line3& l, Point3& out) const {
        if (l.m.z != 0.0f) {
            float t = (z - l.b.z) / l.m.z;
            if ((t>=0.0f) && (t<=1.0f)) {
                // point of intersection...
                out = l.ipol(t);
                return true;
            }
        }
        return false;
    }

    // point in polygon check for sides with constant x,y and z
    bool pip_const_x(const Point3& p) const {
        if ((p.y>=vmin.y)&&(p.y<=vmax.y)&&(p.z>=vmin.z)&&(p.z<=vmax.z)) return true;
        else return false;
    }
    bool pip_const_y(const Point3& p) const {
        if ((p.x>=vmin.x)&&(p.x<=vmax.x)&&(p.z>=vmin.z)&&(p.z<=vmax.z)) return true;
        else return false;
    }
    bool pip_const_z(const Point3& p) const {
        if ((p.x>=vmin.x)&&(p.x<=vmax.x)&&(p.y>=vmin.y)&&(p.y<=vmax.y)) return true;
        else return false;
    }

    //--- check if box intersects or contains line ------------------
    bool intersect(const line3& line) const {
        // For each side of box, check if point of intersection
        // lies within this sides 2d rectangle. If at least one
        // intersection occurs, the line intersects the box
        // (usually, there will be 2 intersections).
        // The line parameter t for the intersection is computed
        // by resolving the formula:
        //  p = line.b + line.m*t
        // after t:
        //  t = (p - line.b) / line.m
        // if line.m is zero, the line is parallel to the plane in
        // question.
        // MAY BE EXTENDED TO RETURN CLOSEST POINT OF INTERSECTION!
        
        // check if at least one of the 2 points is included in the volume
        Point3 s(line.start());
        Point3 e(line.end());
        if (((s.x>=vmin.x) && (s.y>=vmin.y) && (s.z>=vmin.z) &&
             (s.x<=vmax.x) && (s.y<=vmax.y) && (s.z<=vmax.z)) ||
             ((e.x>=vmin.x) && (e.y>=vmin.y) && (e.z>=vmin.z) &&
             (e.x<=vmax.x) && (e.y<=vmax.y) && (e.z<=vmax.z)))
        {
            return true;
        } else {
            // otherwise do intersection check
            int i;
            Point3 ipos;
            for (i=0; i<6; i++) {
                switch (i) {
                    // left side, vmin.x is constant
                    case 0:
                        if (isect_const_x(vmin.x,line,ipos) && pip_const_x(ipos)) return true;
                        break;
                    case 1:
                        if (isect_const_x(vmax.x,line,ipos) && pip_const_x(ipos)) return true;
                        break;
                    case 2:
                        if (isect_const_y(vmin.y,line,ipos) && pip_const_y(ipos)) return true;
                        break;
                    case 3:
                        if (isect_const_y(vmax.y,line,ipos) && pip_const_y(ipos)) return true;
                        break;
                    case 4:
                        if (isect_const_z(vmin.z,line,ipos) && pip_const_z(ipos)) return true;
                        break;
                    case 5:
                        if (isect_const_z(vmax.z,line,ipos) && pip_const_z(ipos)) return true;
                        break;
                }
            }
        }
        return false;
    }

    /**
        @brief Gets closest intersection with AABB.
        If the line starts inside the box,  start point is returned in ipos.
        @param line the pick ray
        @param ipos closest point of intersection if successful, trash otherwise
        @return true if an intersection occurs
    */
    bool intersect(const line3& line, Point3& ipos) const
    {
        // Handle special case for start point inside box
        if (line.b.x >= vmin.x && line.b.y >= vmin.y && line.b.z >= vmin.z &&
            line.b.x <= vmax.x && line.b.y <= vmax.y && line.b.z <= vmax.z)
        {
            ipos = line.b;
            return true;
        }

        // Order planes to check, closest three only
        int plane[3];
        if (line.m.x > 0) plane[0] = 0;
        else              plane[0] = 1;
        if (line.m.y > 0) plane[1] = 2;
        else              plane[1] = 3;
        if (line.m.z > 0) plane[2] = 4;
        else              plane[2] = 5;
        
        for (int i = 0; i < 3; ++i)
        {
            switch (plane[i]) {
                case 0:
                    if (isect_const_x(vmin.x,line,ipos) && pip_const_x(ipos)) return true;
                    break;
                case 1:
                    if (isect_const_x(vmax.x,line,ipos) && pip_const_x(ipos)) return true;
                    break;
                case 2:
                    if (isect_const_y(vmin.y,line,ipos) && pip_const_y(ipos)) return true;
                    break;
                case 3:
                    if (isect_const_y(vmax.y,line,ipos) && pip_const_y(ipos)) return true;
                    break;
                case 4:
                    if (isect_const_z(vmin.z,line,ipos) && pip_const_z(ipos)) return true;
                    break;
                case 5:
                    if (isect_const_z(vmax.z,line,ipos) && pip_const_z(ipos)) return true;
                    break;
            }
        }

        return false;
    }

    //--- check if box intersects, contains or is contained in other box
    //--- by doing 3 projection tests for each dimension, if all 3 test 
    //--- return true, then the 2 boxes intersect
    int line_test(float v0, float v1, float w0, float w1)
    {
        // quick rejection test
        if ((v1<w0) || (v0>w1)) return OUTSIDE;
        else if ((v0==w0) && (v1==w1)) return ISEQUAL;
        else if ((v0>=w0) && (v1<=w1)) return ISCONTAINED;
        else if ((v0<=w0) && (v1>=w1)) return CONTAINS;
        else return CLIPS;
    }

    int intersect(bbox3Plus box) {
        int and_code = 0xffff;
        int or_code  = 0;
        int cx,cy,cz;
        cx = line_test(vmin.x,vmax.x,box.vmin.x,box.vmax.x);
        and_code&=cx; or_code|=cx;
        cy = line_test(vmin.y,vmax.y,box.vmin.y,box.vmax.y);
        and_code&=cy; or_code|=cy;
        cz = line_test(vmin.z,vmax.z,box.vmin.z,box.vmax.z);
        and_code&=cz; or_code|=cz;
        if (or_code == 0) return OUTSIDE;
        else if (and_code != 0) {
            return and_code;
        } else {
            // only if all test produced a non-outside result,
            // an intersection has occured
            if (cx && cy && cz) return CLIPS;
            else                return OUTSIDE;
        }
    }
};


//-------------------------------------------------------------------
// maxscript class
//-------------------------------------------------------------------

applyable_class (BBox3Value)

class BBox3Value : public Value
{
public:
	bbox3Plus bb3;

						BBox3Value();
						
//	ValueMetaClass*		local_base_class() { return class_tag(BBox3Value); }			
						classof_methods (BBox3Value, Value);
	
	void				collect() { delete this; }
	void				sprin1(CharStream* s);
	void				gc_trace();
#	define				is_BBox3(p) ((p)->tag == class_tag(BBox3Value))
	Value*				get_property(Value** arg_list, int count);
	Value*				set_property(Value** arg_list, int count);
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "maxscript\macros\define_implementations.h"
#else
	#include "defimpfn.h"
#endif

	use_generic ( append,		"append");
	use_generic ( intersects ,	"intersects ");

	def_visible_generic( isEmpty,	"isEmpty");			
	def_visible_generic( empty,		"empty");

	/* operations */

//#	include "bmatpro.h"
	
};
//-------------------------------------------------------------------
#endif //_H_BOUNDINGBOX_CLASS
