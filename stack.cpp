#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <X11/Xlib.h>

#ifndef _global_h
#	include "global.h"
#endif

#ifndef _stack_h
#	include "stack.h"
#endif
#ifndef _objects_h
#	include "objects.h"
#endif
#ifndef _ximage_h
#	include "ximage.h"
#endif


Object::Object() {
	next=0;
}

Object::~Object() {
	if (mystack)	mystack->Remove(this);
}

void Object::ExposeWindowRegion( Window /*w*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/ ) {
}

int Object::Intersects(int /*x*/,int /*y*/,int /*width*/,int /*height*/) {
	return 1;
}

int Object::IsInside(int /*x*/,int /*y*/) {
	return 0;
}

void Object::DispatchPress( XButtonEvent * /*xbutton*/ ) {
	mystack->Raise(this);
}

void Object::DispatchRelease( XButtonEvent * /*xbutton*/ ) {
}

void Object::DispatchMotion( XMotionEvent * /*xmotion*/ ) {
}

void Object::PanView( int /*offx*/, int /*offy*/ ) {
}

int Object::JoinExtent( int */*x1*/, int */*y1*/, int */*x2*/, int */*y2*/ ) {
	return 0;
}
int Object::GetExtent( int */*x1*/, int */*y1*/, int */*x2*/, int */*y2*/ ) {
	return 0;
}
void Object::ZoomView( int /*midx*/, int /*midy*/, int /*chg*/ ) {
}

// ===========================================================================

ObjectStack::ObjectStack() {
	first=0;
	sel=0;
	last_sel=0;
	last_x=last_y=-1;
	dbmap=0;
}

ObjectStack::~ObjectStack() {
	while(first)	delete first;
	if (dbmap)		XFreePixmap(dpy,dbmap);
}

void ObjectStack::ExposeRegion(int /*x*/, int /*y*/, int /*width*/, int /*height*/) {
}
void ObjectStack::ExposeWindowRegion(Window /*w*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) {
}

void ObjectStack::Raise(Object *obj) {
	if (first==obj) {
		first=obj->next;						// skip first object
		Append(obj);							// add at end of queue
	}
	else {
		for (Object *current=first;current->next;current=current->next) {
			if (current->next==obj) {
				current->next=obj->next;	// skip object in list
				while(current->next)		current=current->next;	// find end of list
				current->next=obj;			// add at end of queue
				obj->next=0;					// reset anchor
				break;
			}
		}
	}
}

void ObjectStack::Remove(Object *obj) {
	if (first==obj) {
		first=obj->next;						// remove head of list
	}
	else {
		for (Object *current=first;current->next;current=current->next) {
			if (current->next==obj) {
				current->next=obj->next;	// remove entry
				break;
			}
		}
	}
	obj->next=0;								// disconnect object
}

void ObjectStack::Append(Object *obj) {
	if (first==0) {
		first=obj;								// add as first element
	}
	else {
		Object *current;
		for (current=first;current->next;current=current->next);
		current->next=obj;					// add as last element
	}
	obj->next=0;								// anchor on last object
	obj->mystack=this;
}

void ObjectStack::PanView(int offx,int offy) {
	Object *current;
	for (current=first;current;current=current->next)
		current->PanView(offx,offy);
}

void ObjectStack::ZoomView(int midx,int midy, int chg) {
	width  = width *(zoom_factor+chg)/ zoom_factor;
	height = height*(zoom_factor+chg)/ zoom_factor;
	pm->CreateData( width, height );

	Object *current;
	for (current=first;current;current=current->next)
		current->ZoomView(midx,midy,chg);
	zoom_factor+=chg;
}

void ObjectStack::GetExtent( int *x1, int *y1, int *x2, int *y2 ) {
	Object *current;
	for (current=first;current;current=current->next)
		if (current->GetExtent( x1, y1, x2, y2 ))		break;
	for (             ;current;current=current->next)
		current->JoinExtent( x1, y1, x2, y2 );
}

// ===================================

int ObjectStack::SelectObject( Object *current, int x, int y ) {

	if (last_sel==current&&x-last_x>=-1&&x-last_x<=1&&y-last_y>=-1&&y-last_y<=1) {
		// shortcut: same object reselected
		sel=current;
		return 1;
	}

	if (current->next && SelectObject(current->next,x,y))		return 1;

	switch( current->IsInside(x,y) ) {
	case 2:
		// found exact hit -> direct return
		sel=current;
		return 1;
	case 1:
		// store first close hit
		if (!close_sel)	close_sel=current;
	}
	return 0;
}


void ObjectStack::DispatchPress( XButtonEvent *xbutton ) {
	if (!sel) {
		close_sel=0;
		if (!SelectObject(first,xbutton->x,xbutton->y)) {
			// when not direct hit, use close tile ...
			if (close_sel)		sel=close_sel;
		}
	}
	if (sel) {
		// store last selection, which can be done again when on exactly
		// the same position (-> double/tripple clicks for rotations)
			last_sel=sel;
			last_x=xbutton->x;
			last_y=xbutton->y;
			sel->DispatchPress( xbutton );
	}
	else	last_sel=0;
	return;
}

void ObjectStack::DispatchRelease( XButtonEvent *xbutton ) {
	if (sel) {
		sel->DispatchRelease( xbutton );
		if (!((xbutton->state&AnyButtonMask)&~(Button1Mask<<(xbutton->button-1)))) {
			sel=0;			// no more buttons pressed -> cancel selection
			XDefineCursor( dpy, win, normal_cursor );
		}
	}
	return;
}

void ObjectStack::DispatchMotion( XMotionEvent *xmotion ) {
	if (sel) {
		sel->DispatchMotion( xmotion );
	}
	return;
}

// ===========================================================================

DBObjectStack::DBObjectStack() {
	gc=XCreateGC(dpy,RootWindow(dpy,scr),0,0);
}

DBObjectStack::~DBObjectStack() {
	XFreeGC(dpy,gc);
}

void DBObjectStack::ExposeRegion(int x, int y, int width, int height) {
Object *current;

	dbmap=XCreatePixmap(dpy,RootWindow(dpy,scr),width,height,DefaultDepth(dpy,scr));
	for (current=first;current;current=current->next) {
		if (current->Intersects(x,y,width,height)) {
			current->ExposeRegion(x,y,width,height);
		}
	}
	XCopyArea(dpy,dbmap,win,gc,0,0,width,height,x,y);
	XFreePixmap(dpy,dbmap);
	dbmap=0;
}
// ===========================================================================

WindowObjectStack::WindowObjectStack() {
}

WindowObjectStack::~WindowObjectStack() {
}

void WindowObjectStack::ExposeWindowRegion(Window w, int x, int y, int width, int height) {
Object *current;

	for (current=first;current;current=current->next) {
		current->ExposeWindowRegion(w,x,y,width,height);
	}
}

void WindowObjectStack::DispatchPress( XButtonEvent *xbutton ) {
	xbutton->x=xbutton->x_root;
	xbutton->y=xbutton->y_root;
	ObjectStack::DispatchPress( xbutton );
}

void WindowObjectStack::DispatchRelease( XButtonEvent *xbutton ) {
	xbutton->x=xbutton->x_root;
	xbutton->y=xbutton->y_root;
	ObjectStack::DispatchRelease( xbutton );
}

void WindowObjectStack::DispatchMotion( XMotionEvent *xmotion ) {
	xmotion->x=xmotion->x_root;
	xmotion->y=xmotion->y_root;
	ObjectStack::DispatchMotion( xmotion );
}

void WindowObjectStack::Raise(Object *obj) {
	XRaiseWindow( dpy, ((PieceObject*)obj)->swin );
}
