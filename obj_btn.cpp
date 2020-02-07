#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "g_obj.h"
#include "g_bell.h"


float szoom = 1;
float sx = 0, sy = 0;

int current_fi = 0;

GObj model;
GBell g_bell;

void display();
void keyboard( unsigned char key, int x, int y );
void special( int key, int x, int y );


void main( int argc, char **argv )
{
  if( argc<2 )
  {
    printf( "[Usage]: obj_btn [obj_path]\n" );
    exit(-1);
  }

  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );

  glutCreateWindow( "Simple bvh demo" );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutSpecialFunc( special );

  glewInit(); 

  g_bell.create_window();
  g_bell.set_fellow( 0, GBELL_LIGHT | GBELL_OBJ | GBELL_VIEW | GBELL_DOWN | GBELL_UP | GBELL_DRAG | GBELL_DOCK );
  g_bell.set_active_ctrl( GBELL_OBJ );
  g_bell.load( "bell_save01.txt" );

  model.load( argv[1] );
  if(model.n_normal==0)
    model.calculate_face_normal();
  model.unitize();

  glutMainLoop();
}

void display()
{
  float ml[16], mo[16], mv[16];
    g_bell.get_matrix( ml, GBELL_LIGHT );
    g_bell.get_matrix( mo, GBELL_OBJ );
    g_bell.get_matrix( mv, GBELL_VIEW );

  GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );

  glClearColor( .1, .2, .3, 1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(
      viewport[2]*(.5+sx), viewport[3]*(.5+sy), 
      viewport[2]*szoom, viewport[3]*szoom, 
      viewport );
    gluPerspective( 45, double(viewport[2])/viewport[3], 0.1, 10 );
  
  glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt( mv[8]*3,mv[9]*3,mv[10]*3, 0,0,0, mv[4],mv[5],mv[6] );
    glLightfv(GL_LIGHT0, GL_POSITION, &ml[8]);
    glMultMatrixf( mo );


    {
      int i;

      GObj::Material mtl;
      mtl = model.getmtl( model.default_group.name );

      glBegin(GL_TRIANGLES);
      for (i = 0; i < model.n_face; i++)
      {
        glNormal3fv((float*)&model.normal.fm[model.face_nidx[3 * i + 0]]);
        glVertex3fv((float*)&model.vertex.fm[model.face_vidx[3 * i + 0]]);
        glNormal3fv((float*)&model.normal.fm[model.face_nidx[3 * i + 1]]);
        glVertex3fv((float*)&model.vertex.fm[model.face_vidx[3 * i + 1]]);
        glNormal3fv((float*)&model.normal.fm[model.face_nidx[3 * i + 2]]);
        glVertex3fv((float*)&model.vertex.fm[model.face_vidx[3 * i + 2]]);
      }
      glEnd();

      glDisable(GL_LIGHTING);

      //for (i = 0; i<10; i++)
      {
        i = current_fi;
        FLOAT3 v0 = model.vertex.fm[model.face_vidx[3 * i + 0]];
        FLOAT3 v1 = model.vertex.fm[model.face_vidx[3 * i + 1]];
        FLOAT3 v2 = model.vertex.fm[model.face_vidx[3 * i + 2]];

        FLOAT3 vm = vmax(vmax(v0, v1), v2);
        FLOAT3 vn = vmin(vmin(v0, v1), v2);

        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
          glVertex3f(vm.x, vm.y, vm.z);
          glVertex3f(vm.x, vn.y, vm.z);
          glVertex3f(vn.x, vn.y, vm.z);
          glVertex3f(vn.x, vm.y, vm.z);
        glEnd();
        glBegin(GL_LINE_LOOP);
          glVertex3f(vm.x, vm.y, vn.z);
          glVertex3f(vm.x, vn.y, vn.z);
          glVertex3f(vn.x, vn.y, vn.z);
          glVertex3f(vn.x, vm.y, vn.z);
        glEnd();
        glBegin(GL_LINE_LOOP);
          glVertex3f(vm.x, vm.y, vm.z);
          glVertex3f(vm.x, vn.y, vm.z);
          glVertex3f(vm.x, vn.y, vn.z);
          glVertex3f(vm.x, vm.y, vn.z);
        glEnd();
        glBegin(GL_LINE_LOOP);
          glVertex3f(vn.x, vm.y, vm.z);
          glVertex3f(vn.x, vn.y, vm.z);
          glVertex3f(vn.x, vn.y, vn.z);
          glVertex3f(vn.x, vm.y, vn.z);
        glEnd();
      }

      for (i = 0; i<model.n_face; i++)
      {
        FLOAT3 v0 = model.vertex.fm[model.face_vidx[3 * i + 0]];
        FLOAT3 v1 = model.vertex.fm[model.face_vidx[3 * i + 1]];
        FLOAT3 v2 = model.vertex.fm[model.face_vidx[3 * i + 2]];

        glColor3f(1, 0, 1);
        glBegin(GL_LINE_LOOP);
        glVertex3fv((float*)&v0);
        glVertex3fv((float*)&v1);
        glVertex3fv((float*)&v2);
        glEnd();
      }

      glEnable(GL_LIGHTING);

    }

    glPrintf( ">>glut" );
    glColor3f(1,1,1);
    glPrintf( "Just to say hi. Hi!\n" );

  glutSwapBuffers();
}

void keyboard( unsigned char key, int x, int y )
{
  switch( key )
  {
    case 27:
      szoom = 1;
      sx = 0;
      sy = 0;
      g_bell.reset();
      glutPostRedisplay();
      break;
    case '+':
    case '=':
      szoom *= .9;
      glutPostRedisplay();
      break;
    case '_':
    case '-':
      szoom /= .9;
      glutPostRedisplay();
      break;

    case '<':
    case ',':
      current_fi = g_clamp(current_fi-1, 0, model.n_face-1);
      glutPostRedisplay();
      break;

    case '>':
    case '.':
      current_fi = g_clamp(current_fi+1, 0, model.n_face - 1);
      glutPostRedisplay();
      break;

    case '|':
    case '\\':
      glutPostRedisplay();
      break;

  }
}

void special( int key, int x, int y )
{
  GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

  switch( key )
  {
    case GLUT_KEY_F11:
    {
      static bool bfullscreen = false;
      bfullscreen = !bfullscreen;
      if( bfullscreen )
      {
        glutFullScreen();
      }else
      {
        glutReshapeWindow(512, 512);
        glutPositionWindow(50, 50);
      }
    }
    break;

    case GLUT_KEY_UP:
      sy -= 8*szoom/viewport[3];
      glutPostRedisplay();
      break;
    case GLUT_KEY_DOWN:
      sy += 8*szoom/viewport[3];
      glutPostRedisplay();
      break;
    case GLUT_KEY_LEFT:
      sx += 8*szoom/viewport[2];
      glutPostRedisplay();
      break;
    case GLUT_KEY_RIGHT:
      sx -= 8*szoom/viewport[2];
      glutPostRedisplay();
      break;

  }
}










