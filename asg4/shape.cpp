// $Id: shape.cpp,v 1.1 2015-07-16 16:47:51-07 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"
#include "graphics.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}

polygon::polygon (const vertex_list& vertices): vertices(vertices) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
            polygon({{0, 0}, {width, 0}, 
                     {width, height}, {0, height}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices): polygon(vertices) {
   DEBUGF ('c', this);
}

equilateral::equilateral (const GLfloat width):
  triangle({{-width/2,0}, {width/2,0}, {0,(float)sqrt(3)*width/2}}) {
}

diamond::diamond (const GLfloat width, const GLfloat height):
            polygon({{0, 0}, {-width / 2, height / 2},
                     {0, height}, {width / 2, height / 2}}) {
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glColor3ubv(color.ubvec);
   glRasterPos2f(center.xpos, center.ypos);
   glutBitmapString(glut_bitmap_font, 
           reinterpret_cast<const unsigned char*>(textdata.c_str()));
   if (window::obj_selected){
      glLineWidth(window::b_width);
      glBegin (GL_LINE_LOOP);
      glColor3ubv(rgbcolor(window::b_color).ubvec);
      int length = 0;
      for (char c: textdata)
         length += glutBitmapWidth(glut_bitmap_font, c);
      glVertex2f(center.xpos, center.ypos-window::b_width);
      glVertex2f(center.xpos + length, center.ypos-window::b_width);
      //glVertex2f(center.xpos + length, center.ypos + 10);
      //glVertex2f(center.xpos, center.ypos + 10);
      glEnd();
   }
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glBegin(GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);
   const float delta = 2*M_PI / 32;
   for (float theta = 0; theta < 2*M_PI; theta += delta) {
       float xpos = dimension.xpos/2 * cos(theta) + center.xpos;
       float ypos = dimension.ypos/2 * sin(theta) + center.ypos;
       glVertex2f(xpos, ypos);
   }
   glEnd();
    if (window::obj_selected) {
       glLineWidth(window::b_width);
       glBegin (GL_LINE_LOOP);
       glEnable (GL_LINE_SMOOTH);
       glColor3ubv(rgbcolor(window::b_color).ubvec);
       const float delta = 2* M_PI / 32;
       for (float theta = 0; theta < 2* M_PI; theta += delta) {
           float xpos = dimension.xpos/2 * cos(theta) + center.xpos;
           float ypos = dimension.ypos/2 * sin(theta) + center.ypos;
           glVertex2f(xpos, ypos);
       }
       glEnd();
    }
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   
   GLfloat xb;
   GLfloat yb;
   int i = 0;
   for (vertex v : vertices) {
       xb = (xb * i + v.xpos)/(i + 1);
       yb = (yb * i + v.ypos)/(i + 1);
       i++;
   }
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   for (vertex v : vertices) {
       GLfloat x = center.xpos + v.xpos - xb;
       GLfloat y = center.ypos + v.ypos - yb;
       glVertex2f(x,y);
   }
   glEnd();
   if (window::obj_selected) {
      glLineWidth(window::b_width);
      glBegin (GL_LINE_LOOP);
      glColor3ubv(rgbcolor(window::b_color).ubvec);
      for (vertex v : vertices) {
         GLfloat x = center.xpos + v.xpos - xb;
         GLfloat y = center.ypos + v.ypos - yb;
         glVertex2f(x,y);
      }
      glEnd();
   }
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

