#ifndef GLUVI_H
#define GLUVI_H

#include <iostream>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h> // why does Apple have to put glut.h here...
#else
#include <glut.h> // ...when everyone else puts it here?
#endif

#include "vec.h"

namespace Gluvi{

/*
For portions of the code which export RIB commands (e.g. from cameras):

The RenderMan (R) Interface Procedures and Protocol are:
Copyright 1988, 1989, Pixar
All Rights Reserved
*/

// the camera capability for Gluvi
struct Camera
{
   virtual ~Camera(void) {}
   virtual void click(int button, int state, int x, int y) = 0;
   virtual void drag(int x, int y) = 0;
   virtual void return_to_default(void) = 0;
   //@@@ add these to be called by a user glutKeyboardFunc() thing so that
   //@@@ cameras can easily add keyboard shortcuts for e.g. return_to_default, transformation, etc.
   //virtual void navigation_keyboard_handler(unsigned char key, int x, int y) = 0;
   //virtual void navigation_special_key_handler(int key, int x, int y) = 0;
   virtual void gl_transform(void) = 0;
   virtual void export_rib(std::ostream &output) = 0;
   virtual void display_screen(void) {} // in case the camera needs to show anything on screen
};

struct Target3D : public Camera
{
   float target[3], dist;
   float heading, pitch;
   float default_target[3], default_dist;
   float default_heading, default_pitch;
   float fovy;
   float near_clip_factor, far_clip_factor;
   enum {INACTIVE, ROTATE, TRUCK, DOLLY} action_mode;
   int oldmousex, oldmousey;

   Target3D(float target_[3]=0, float dist_=1, float heading_=0, float pitch_=0, float fovy_=45,
            float near_clip_factor_=0.01, float far_clip_factor_=100);
   void click(int button, int state, int x, int y);
   void drag(int x, int y);
   void return_to_default(void);
   void transform_mouse(int x, int y, float ray_origin[3], float ray_direction[3]);
   void get_viewing_direction(float direction[3]);
   void gl_transform(void);
   void export_rib(std::ostream &output);
};

// same as above, but with orthographic projection
struct TargetOrtho3D : public Camera
{
   float target[3], dist;
   float heading, pitch;
   float default_target[3], default_dist;
   float default_heading, default_pitch;
   float height_factor;
   float near_clip_factor, far_clip_factor;
   enum {INACTIVE, ROTATE, TRUCK, DOLLY} action_mode;   // @@@@ WHAT ABOUT ZOOMING??? IS WIDTH ALWAYS A FUNCTION OF DIST?
   int oldmousex, oldmousey;

   TargetOrtho3D(float target_[3]=0, float dist_=1, float heading_=0, float pitch_=0, float height_factor_=1,
                 float near_clip_factor_=0.01, float far_clip_factor_=100);
   void click(int button, int state, int x, int y);
   void drag(int x, int y);
   void return_to_default(void);
   void transform_mouse(int x, int y, float ray_origin[3], float ray_direction[3]);
   void get_viewing_direction(float direction[3]);
   void gl_transform(void);
   void export_rib(std::ostream &output);
};

struct PanZoom2D : public Camera
{
   float bottom, left, height;
   float default_bottom, default_left, default_height;
   enum {INACTIVE, PAN, ZOOM_IN, ZOOM_OUT} action_mode;
   int oldmousex, oldmousey;
   bool moved_since_mouse_down; // to distinuish simple clicks from drags
   int clickx, clicky;

   PanZoom2D(float bottom_=0, float left_=0, float height_=1);
   void click(int button, int state, int x, int y);
   void drag(int x, int y);
   void return_to_default(void);
   void transform_mouse(int x, int y, float coords[2]);
   void gl_transform(void);
   void export_rib(std::ostream &output);
   void display_screen(void);
};

// overlaid user-interface widgets
struct Widget
{
   int dispx, dispy, width, height; // set in display()

   virtual ~Widget() {}
   virtual void display(int x, int y) = 0;
   virtual bool click(int state, int x, int y) { return false; } // returns true if click handled by widget
   virtual void drag(int x, int y) {}
};

struct StaticText : public Widget
{
   const char *text;

   StaticText(const char *text_);
   void display(int x, int y);
};

struct Button : public Widget
{
   enum {UNINVOLVED, SELECTED, HIGHLIGHTED} status;
   const char *text;
   int minwidth;

   Button(const char *text_, int minwidth_=0);
   void display(int x, int y);
   bool click(int state, int x, int y);
   void drag(int x, int y);
   virtual void action() {}
};

struct Slider : public Widget
{
   enum {UNINVOLVED, SELECTED} status;
   const char *text;
   int length, justify;
   int position;
   int scrollxmin, scrollxmax, scrollymin, scrollymax;
   int clickx;

   Slider(const char *text_, int length_=100, int position_=0, int justify_=0);
   void display(int x, int y);
   bool click(int state, int x, int y);
   void drag(int x, int y);
   virtual void action() {}
};

struct WidgetList : public Widget
{
   int indent;
   bool hidden;
   std::vector<Widget*> list;
   int downclicked_member;

   WidgetList(int listindent_=12, bool hidden_=false);
   void display(int x, int y);
   bool click(int state, int x, int y);
   void drag(int x, int y);
};

// display callback
extern void (*userDisplayFunc)(void); 

// mouse callbacks for events that Gluvi ignores (control not pressed, or mouse not on an active widget)
extern void (*userMouseFunc)(int button, int state, int x, int y);
extern void (*userDragFunc)(int x, int y);

// user is free to do their own callbacks for everything else except glutReshape()

// additional helpful functions
void ppm_screenshot(const char *filename_format, ...);
void sgi_screenshot(const char *filename_format, ...);
void set_generic_lights(void);
void set_generic_material(float r, float g, float b, GLenum face=GL_FRONT_AND_BACK);
void set_matte_material(float r, float g, float b, GLenum face=GL_FRONT_AND_BACK);
//@@@@@@@ USEFUL FUNCTIONALITY:
void draw_3d_arrow(const float base[3], const float point[3], float arrow_head_length=0);
void draw_2d_arrow(const Vec2f base, const Vec2f point, float arrow_head_length);
void draw_coordinate_grid(float size=1, int spacing=10);
void draw_text(const float point[3], const char *text, int fontsize=12);

// call init first thing
void init(const char *windowtitle, int *argc, char **argv);

// the Gluvi state
extern Camera *camera;
extern WidgetList root;
extern int winwidth, winheight;

// then after setting the Gluvi state and doing any of your own set-up, call run()
void run(void);

}; // end of namespace

#endif
