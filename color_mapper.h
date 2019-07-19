#ifndef __color_mapper_h
#define __color_mapper_h

class ColorMapper {
	public:
		ColorMapper(Display *dpy);
		~ColorMapper();

		unsigned long alloc_color( XColor *col );
		unsigned long alloc_named_color( const char *name );
		Colormap	get_colormap()	{ return mymap; }

		Display	*GetDisplay()	{ return dpy; }

	private:
		void setup_usage();
		void free_usage();

		Display	*dpy;
		Colormap	mymap;
		int		cells;
		XColor	*colors;
};

class Port {
	public:
		Port(Display *dpy);
		~Port();

		Display		*GetDisplay()		{ return dpy; }
		ColorMapper *GetMapper()		{ return mapper; }

		unsigned long	AllocNamedColor( const char *name )
				{ return mapper->alloc_named_color( name ); }

	private:
		Display		*dpy;
		ColorMapper	*mapper;
};
#endif
