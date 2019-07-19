#ifndef _stack_h
#define _stack_h

/****************************************************************************

   This module defines some classes to organizes the puzzle tiles on the
   Main functionalities: implementation of a stacking order
                         dispatch of events down to the pieces
                         control of the double buffering
                         control zooming and panning

   The class Object will be a superclass of the puzzle tiles later on,
   even though the whole object hierarchie is actually contained in the
   object-module.

 ****************************************************************************/

// ==========================================================================

class Object {
	public:
		Object();
		virtual ~Object();

		virtual void ExposeRegion( int x, int y, int width, int height ) = 0;
		virtual void ExposeWindowRegion( Window w, int x, int y, int width, int height );

		virtual int Intersects( int x, int y, int width, int height );
		virtual int IsInside( int x, int y );

		virtual void DispatchPress( XButtonEvent * /*xbutton*/ );
		virtual void DispatchRelease( XButtonEvent * /*xbutton*/ );
		virtual void DispatchMotion( XMotionEvent * /*xmotion*/ );

		virtual void PanView( int offx, int offy );
		virtual void ZoomView( int midx, int midy, int chg );
		virtual int JoinExtent( int *x1, int *y1, int *x2, int *y2 );
		virtual int GetExtent( int *x1, int *y1, int *x2, int *y2 );

	protected:
		class Object		*next;
		class ObjectStack	*mystack;

friend class ObjectStack;
friend class DBObjectStack;
friend class WindowObjectStack;
};

// ==========================================================================

class ObjectStack {
	public:
		ObjectStack();
		virtual ~ObjectStack();

		virtual void ExposeRegion( int x1, int y1, int width, int height );
		virtual void ExposeWindowRegion( Window w, int x1, int y1, int width, int height );

		virtual void Raise( class Object *obj );		// move to top of stack
		virtual void Append( class Object *obj );		// add at top of stack
		virtual void Remove( class Object *obj );		// remove from stack

		virtual void DispatchPress( XButtonEvent *xbutton );
		virtual void DispatchRelease( XButtonEvent *xbutton );
		virtual void DispatchMotion( XMotionEvent *xmotion );

		void PanView( int offx, int offy );
		void ZoomView( int midx, int midy, int chg );
		void GetExtent( int *x1, int *y1, int *x2, int *y2 );

		Pixmap			dbmap;	// should actually by contained in DBObjectStack

	protected:
		virtual int SelectObject( class Object *current, int x, int y );

		class Object	*sel;
		class Object	*close_sel;
		class	Object	*first;

		class Object	*last_sel;
		int				last_x, last_y;
};

// ==========================================================================

class DBObjectStack : public ObjectStack {
	public:
		DBObjectStack();
		virtual ~DBObjectStack();

		void ExposeRegion( int x1, int y1, int width, int height );

		GC					gc;

	protected:
};

// ==========================================================================

class WindowObjectStack : public ObjectStack {
	public:
		WindowObjectStack();
		virtual ~WindowObjectStack();

		void ExposeWindowRegion( Window w, int x1, int y1, int width, int height );

		virtual void Raise( class Object *obj );		// move to top of stack

		void DispatchPress( XButtonEvent *xbutton );
		void DispatchRelease( XButtonEvent *xbutton );
		void DispatchMotion( XMotionEvent *xmotion );

	protected:
};

// ==========================================================================

#endif
