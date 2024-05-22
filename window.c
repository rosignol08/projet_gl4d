/*!\file window.c
 *
 * \brief using GL4Dummies and Assimp Library to load 3D models or scenes.
 *
 * \author Farès Belhadj amsi@up8.edu
 * \date February 14 2017, modified on March 24, 2024
*/
#include <stdlib.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4dp.h>
#include <SDL_image.h>
#include <stdbool.h>
#include "assimp.h"
//#include "audioHelper.h"
#include <math.h> // Pour la fonction pow()
#include <SDL_mixer.h>
#include <SDL_ttf.h>
/*!\brief nombre d'échantillons du signal sonore */
#define ECHANTILLONS 1024
/*!\brief amplitude des échantillons du signal sonore */
static Sint16 _hauteurs[ECHANTILLONS];
/*!\brief opened window width */
static int _windowWidth = 1024;
/*!\brief opened window height */
static int _windowHeight = 768;
/*!\brief Sphere geometry Id  */
//static GLuint _sphere = 0;
/*!\brief cube geometry*/
static GLuint _cube = 0;
static GLuint _cube2 = 0;
/*!\brief GLSL program Id */
static GLuint _pId = 0;
/*!\brief sphere Id */
static GLuint _sphereId = 0;
//static GLuint _sphereId2 = 0;
/*!\brief Id de la texture du ciel*/
static GLuint _texId = 0;
//texture du sol
static GLuint textureId_soleil = 0;
//position voiture
float position_voiture[3] = {0.0f, 0.0f, 0.0f};

////lumière dynamique
GLfloat lumpos[] = {0.0f, 0.0f, 0.0f, 1.0f};
/*!\brief pointeur vers la musique chargée par SDL_Mixer */
static Mix_Music * _mmusic = NULL;

//les credits////////////////////////////////////////
int credits, hehe = 0; //pour passer à l'ecrant de fin
static void initText(GLuint * ptId, const char * text);
/*!\brief identifiant de la texture contenant le texte */
static GLuint _textTexId = 0;
/*!\brief identifiant de la géométrie d'un plan */
static GLuint _quad = 0;
void initialisation_credits(void);
static void initText(GLuint * ptId, const char * text);

/*!\brief enum that index keyboard mapping for direction commands */
enum kyes_t {
  KLEFT = 0,
  KRIGHT,
  KUP,
  KDOWN,
  KPAGEUP,
  KPAGEDOWN
};

/*!\brief virtual keyboard for direction commands */
static GLuint _keys[] = {0, 0, 0, 0, 0, 0};

typedef struct {
  GLfloat x, y, z; // Position du centre du cube
  GLfloat size_x, size_y, size_z;    // Taille du cube (supposée égale dans toutes les dimensions)
} Cube;

Cube myCube; //= { -5.0f, 1.0f, -20.0f, 1.0f }; // Coordonnées et taille du cube

typedef struct cam_t cam_t;
/*!\brief a data structure for storing camera position and
 * orientation */
struct cam_t {
  GLfloat x, y, z;
  GLfloat theta;
};

//void fred(float cubeSizex, float cubeSizey, float cubeSizez, float x, float y, float z);
//void steph(float sphereSize, float x, float y, float z);

/*!\brief the used camera */
static cam_t _cam = {0.0f, 0.0f, 1.0f, 0.0f};

/*!\brief toggle y-axis rotation pause */
static GLboolean _pause = GL_TRUE;

/*!\brief toggle view focused on the scene center */
static GLboolean _center_view = GL_FALSE;

/*!\brief identifiant de la scene générée par assimpGenScene */ /////////////////objjjjjjjjjjjjj 3DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD///////////////////////////
static GLuint _id_scene = 0;
static GLuint _id_scene2 = 0;
static GLuint _id_scene3 = 0;


/*!\brief rotation angles according to axis (0 = x, 1 = y, 2 = z) 
 * \todo améliorer l'interface et ajouter rotations/zoom à la souris */
static GLfloat rot[3] = {0, 0, 0};
static void initGL(void);
static void quit(void);
static void resize(int w, int h);
static void idle(void);
static void draw(void);
static void keydown(int keycode);
static void keyup(int keycode);

void fred_texture(float cubeSizex, float cubeSizey, float cubeSizez, float x, float y, float z, float indice_rotation, GLuint textureId) {
    GLint useColorLocation = glGetUniformLocation(_pId, "useColor");
    glUniform1i(useColorLocation, 0); // Activer l'utilisation de la couleur uniforme pour le cube
    
    // Désactiver l'utilisation de la couleur uniforme et activer l'utilisation de la texture
    glUniform1i(useColorLocation, 0);
    glUniform1i(glGetUniformLocation(_pId, "useTexture"), 1);
    
    // Liaison de la texture à l'unité de texture 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Définition de la transformation du cube
    gl4duPushMatrix(); {
        gl4duTranslatef(x, y, z);
        //gl4duRotatef(0.0f, 0.0f, 0.0f, 0.0f);  // Exemple de rotation
        gl4duScalef(cubeSizex, cubeSizey, cubeSizez);  // Mise à l'échelle en fonction de la taille du cube
        gl4duRotatef(indice_rotation, 1.0f, 0.0f, 0.0f);
        gl4duRotatef(indice_rotation, 0.0f, 0.0f, 1.0f);
        gl4duSendMatrices();
        
        // Dessin du cube avec la texture
        gl4dgDraw(_cube);
    } gl4duPopMatrix();
    
    // Désactiver l'utilisation de la texture
    glUniform1i(glGetUniformLocation(_pId, "useTexture"), 0);
    glBindTexture(GL_TEXTURE_2D, 0); // Désactivation de la texture
}

GLuint loadTexture(const char *filename) {
    GLuint textureId;
    glGenTextures(1, &textureId); // Générer un identifiant de texture

    // Charger l'image depuis le fichier
    SDL_Surface *image = IMG_Load(filename);
    if (!image) {
        fprintf(stderr, "Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        return 0; // Retourner 0 en cas d'erreur
    }

    // Convertir l'image en format RGBA
    SDL_Surface *	d = SDL_CreateRGBSurface(0, image->w, image->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  	SDL_BlitSurface(image, NULL, d, NULL);
    SDL_FreeSurface(image); // Libérer la mémoire de l'image originale

    // Liaison de la texture
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Copier les données de l'image dans la texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d->w, d->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d->pixels);

    // Configuration des paramètres de la texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Libérer la mémoire de l'image convertie
    SDL_FreeSurface(d);

    // Retourner l'identifiant de texture généré
    return textureId;
}

/*!\brief function that initialize OpenGL / GL4D params and objects.*/
static void initGL(void) {
  _sphereId = gl4dgGenSpheref(20, 20);
  
  _cube = gl4dgGenCubef();
  _cube2 = gl4dgGenCubef();
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);////////////aaaaaaaaa
  _pId = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  resize(_windowWidth, _windowHeight);
  /////////alpha
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glGenTextures(1, &_texId);
  glBindTexture(GL_TEXTURE_2D, _texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  srand(42);///////////////////cielllllllllllllllllll/////////////////////////////////////////////////////////////////////////////////////////:
  /*
  GLfloat * fractale = gl4dmTriangleEdge (513, 513, 0.3f);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 513, 513, 0, GL_RED, GL_FLOAT, fractale);
  free(fractale);
  */
 GLfloat * fractale = gl4dmTriangleEdge(513, 513, 0.3f); // Génère la fractale

  // Parcourez chaque pixel de la fractale pour ajuster les valeurs de gris
  for (int i = 0; i < 513 * 513; i++) {
      // Assurez-vous que les valeurs de gris restent dans la plage [0, 1]
      if (fractale[i] < 0.337f) {
          fractale[i] = 0.337f; // Attribution de la couleur du ciel
      } else {
          fractale[i] = 0.498f; // Attribution de la couleur des nuages
      }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 513, 513, 0, GL_RED, GL_FLOAT, fractale); // Charge la fractale modifiée dans la texture

  free(fractale); // Libère la mémoire de la fractale

  glBindTexture(GL_TEXTURE_2D, 0);
  textureId_soleil = loadTexture("vecteezy_abstract-elements-retro-style-80s-90s_8507610.png"); // Chargement de la texture du soleil
  //ahInitAudio("lady-of-the-80x27s-128379.mp3");//takeonme.mod  
}


/*!\brief function called by GL4Dummies' loop at resize. Sets the
 *  projection matrix and the viewport according to the given width
 *  and height.
 * \param w window width
 * \param h window height
 */
static void resize(int w, int h) {
  _windowWidth = w; 
  _windowHeight = h;
  glViewport(0.0f, 0.0f, _windowWidth, _windowHeight);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  /* gl4duFrustumf(-0.005f, 0.005f, -0.005f * _windowHeight / _windowWidth, 0.005f * _windowHeight / _windowWidth, 0.01f, 1000.0f); */
  /* même résultat en utilisant la fonction perspective */
  gl4duPerspectivef(100.0f, (GLfloat)_windowWidth/(GLfloat)_windowHeight, 0.01f, 1000.0f);//fov
  gl4duBindMatrix("modelViewMatrix");
}

/*!\brief function called by GL4Dummies' loop at idle.
 * 
 * uses the virtual keyboard states to move the camera according to
 * direction, orientation and time (dt = delta-time)
 */
static void idle(void) {
  static float t0 = 0.0f;

  float t, dt = M_PI, step = 1.0f;//dtheta
  dt = ((t = (float)gl4dGetElapsedTime()) - t0) / 1000.0f;
  t0 = t;
  if(_keys[KLEFT]){
    if (position_voiture[0] > -0.80f)
      position_voiture[0] -= dt * 0.5f * step;
    //printf("%f\n", position_voiture[0]);
    //_cam.theta += dt * dtheta;
  }
  if(_keys[KRIGHT]){
    if (position_voiture[0] < 0.80f)
      position_voiture[0] += dt * 0.5f * step;
    //printf("%f\n", position_voiture[0]);
    //_cam.theta -= dt * dtheta;
  }
  if(_keys[KPAGEUP]) {
    credits = 1;
    //_cam.y += dt * 0.5f * step;
  }
  if(_keys[KPAGEDOWN]) {
    //_cam.y -= dt * 0.5f * step;
  }
  if(_keys[KUP]) {
    if (position_voiture[1] > -0.50f){
      position_voiture[1] -= dt * 0.5f * step;
    }
    //_cam.x += -dt * step * sin(_cam.theta);
    //_cam.z += -dt * step * cos(_cam.theta);
  }
  if(_keys[KDOWN]) {
    if(position_voiture[1] < 0.50f){
      position_voiture[1] += dt * 0.5f * step;
    }
    //_cam.x += dt * step * sin(_cam.theta);
    //_cam.z += dt * step * cos(_cam.theta);
  }
  if(!_pause)
    rot[1] += 90.0f * dt;
}

/*!\brief function called by GL4Dummies' loop at key-down (key
 * pressed) event.
 * 
 * stores the virtual keyboard states (1 = pressed) and toggles the
 * boolean parameters of the application.
 */
static void keydown(int keycode) {
  GLint v[2];
  switch(keycode) {
  case GL4DK_LEFT:
    _keys[KLEFT] = 1;
    break;
  case GL4DK_RIGHT:
    _keys[KRIGHT] = 1;
    break;
  case GL4DK_UP:
    _keys[KUP] = 1;
    break;
  case GL4DK_DOWN:
    _keys[KDOWN] = 1;
    break;
  case GL4DK_d:
    _keys[KPAGEDOWN] = 1;
    break;
  case GL4DK_u:
    _keys[KPAGEUP] = 1;
    break;
  case GL4DK_ESCAPE:
  case 'q':
    exit(0);
    /* when 'w' pressed, toggle between line and filled mode */
  case 'w':
    glGetIntegerv(GL_POLYGON_MODE, v);
    if(v[0] == GL_FILL) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(3.0f);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glLineWidth(1.0f);
    }
    break;
  case GL4DK_SPACE:
    _pause = !_pause;
    break;
  case GL4DK_c:
    _center_view = !_center_view;
    break;
  default:
    break;
  }
}

/*!\brief function called by GL4Dummies' loop at key-up (key
 * released) event.
 * 
 * stores the virtual keyboard states (0 = released).
 */
static void keyup(int keycode) {
  switch(keycode) {
  case GL4DK_LEFT:
    _keys[KLEFT] = 0;
    break;
  case GL4DK_RIGHT:
    _keys[KRIGHT] = 0;
    break;
  case GL4DK_UP:
    _keys[KUP] = 0;
    break;
  case GL4DK_DOWN:
    _keys[KDOWN] = 0;
    break;
  case GL4DK_d:
    _keys[KPAGEDOWN] = 0;
    break;
  case GL4DK_u:
    _keys[KPAGEUP] = 0;
    break;
  default:
    break;
  }
}

///////////////recuperation temps
static double get_dt(void) {
  static double t0 = 0.0f;
  double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
  t0 = t;
  return dt;
}

float rotation_planette = 0;//angle rotation
float rotation_sky = 0;//angle rotation
float indice_rotation_planette = 0.15f;//coeficient pour accelerer ou pas
float indice_taille_sky = 50.0f;// pour changer la taille du ciel
bool temps = true;/////pour le timer
static double lastTime = 0.0f;
/*!\brief function called on each GL4Dummies' display event. It draws
 * the scene with its given params.
 */
static void draw_scene(void){
  static GLfloat a = 0.0f;
  //GLfloat lum[4] = {0.0f, 0.0f, 5.0f, 1.0f};
  glClearColor(0.969f, 0.361f, 0.886f, 1.0f);//glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
  //static const GLfloat rouge[] = {0.6f, 0.0f, 0.0f, 1.0f}, bleu[] = {0.0f, 0.0f, 0.6f, 1.0f}, vert[] = {0.0f, 0.6f, 0.0f, 1.0f};
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);  
  // Mise à jour de la position de la lumière en fonction du temps
  //lumpos[1] = 2.0f + 1.9f * sin(a);
  //lumpos[0] = 1.10f * sin(a*0.5f);
  //pour faire bouger la lumière
  lumpos[1] = 1.0f + 0.50f * cos(a); //position_voiture[1]; //  + 1.0f * cos(a) Nouvelle formule pour le mouvement de la lumière
  lumpos[0] = position_voiture[0];
  //lumpos[2] = 0.0f;

  // Normalisation des valeurs pour rester dans une plage appropriée
  //if (lumpos[0] > 10.0f) lumpos[0] = 10.0f; // Limite supérieure
  //if (lumpos[0] < -10.0f) lumpos[0] = -10.0f; // Limite inférieur
  // Utilisation de la position de la lumière dans les shaders
  glUseProgram(_pId);
  glUniform4fv(glGetUniformLocation(_pId, "lumpos") , 1, lumpos);
  //glUniform4fv(glGetUniformLocation(_pId, "lumpos"), 1, lum);//lum

  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();

  gl4duLookAtf(_cam.x, _cam.y, _cam.z, _cam.x - sin(_cam.theta),  _center_view ? 0.0f : _cam.y,  _cam.z - cos(_cam.theta), 0.0f, 1.0f, 0.0f);
  /* Skydome */

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texId);
  glUniform4f(glGetUniformLocation(_pId, "diffuse_color"), 0.0f, 0.0f, 0.0f, 0.0f);//1.0f, 1.0f, 1.0f, 1.0f
  glUniform4f(glGetUniformLocation(_pId, "ambient_color"), 0.0f, 0.0f, 0.0f, 0.0f); //0.988, 0.004, 0.678
  glUniform1i(glGetUniformLocation(_pId, "myTexture"), 0);
  glUniform1i(glGetUniformLocation(_pId, "hasTexture"), 1);
  glUniform1i(glGetUniformLocation(_pId, "sky"), 1);
  gl4duPushMatrix();
  gl4duTranslatef(_cam.x, _cam.y, _cam.z);
  gl4duScalef(100.0f, 100.0f, 100.0);
  gl4duSendMatrices();
  gl4duPopMatrix();
  glCullFace(GL_FRONT);
  gl4dgDraw(_sphereId);
  glCullFace(GL_BACK);

  glUniform1i(glGetUniformLocation(_pId, "sky"), 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  // Calcul de l'intensité du volume à partir des données audio
    float intensity = 0.0f;
    for (int i = 0; i < ECHANTILLONS; i++) {
        intensity += abs(_hauteurs[i]) / (float)(1 << 15);
    }
    intensity /= ECHANTILLONS;

    // Comparaison avec une valeur seuil
    float threshold = 0.018f; // Valeur seuil 
    //printf("Intensité du volume : %f\n", intensity);
    if (intensity > threshold && temps == true && indice_rotation_planette == 0.15f && indice_taille_sky == 50.0f) {
        // Déclenchement de l'action si l'intensité dépasse la valeur seuil
        // Par exemple, affichage d'un message
        printf("Intensité du volume supérieure à la valeur seuil!\n");
        indice_rotation_planette = 0.20f + abs(2*cos(a));
        indice_taille_sky = 50.0f;
        //indice_rotation_planette = 0.7f; // Multiplier par 2 si le volume est fort
        //indice_rotation_sky = 0.3f;
        temps = false;
    }

    if (intensity >= threshold && temps == true){
      indice_rotation_planette = 0.15f; // Diviser par 2 si le volume est faible
      indice_taille_sky = 50.0f;
      /* pour du flou gaussien */
      gl4dfBlur (0, 0, 30, 1, 0 /* weight */, GL_FALSE);
      temps = false;
    }
  // Calcul du temps écoulé depuis le dernier changement de 'temps'
  double currentTime = gl4dGetElapsedTime();
  double elapsedTime = (currentTime - lastTime) / 1000.0; // Conversion en secondes
  // Vérification si une seconde s'est écoulée depuis le dernier changement de 'temps'
  if (elapsedTime >= 1.0 && temps == false) {
    // Si une seconde s'est écoulée, définissez 'temps' sur 'true'
    temps = true;
    // Mettre à jour le dernier temps de changement de 'temps'
    lastTime = currentTime;
  }
  
  /* pour le contour du cell-shading */
  gl4dfSobelSetMixMode(GL4DF_SOBEL_MIX_MULT);
  gl4dfSobel (0, 0, GL_FALSE);
  /* l'objet chargé avec assimp */
  //x(gauche droite) y(hauteur moin de 1) z(avant arrière)
  //rotation: angle, x ( vers l'avant ou l'arrière), y (vers la droite ou la gauche) z (vers le haut ou le bas)
  //voiture
  gl4duPushMatrix();
  //gl4duRotatef(rot[1], 0.0f, 1.0f, 0.0f);
  gl4duScalef(1.50f, 1.50f, 1.50f);
  // x gauche droite, y hauteur, z avant arrière
  gl4duTranslatef(position_voiture[0],-0.8f+0.1*position_voiture[2],-1.0f+position_voiture[1]);
  gl4duRotatef(90.0f, 0.0f, 1.0f, 0.0f);
  assimpDrawScene(_id_scene);
  gl4duPopMatrix();

  //planet
  gl4duPushMatrix();
  gl4duScalef(40.0f, 40.0f,40.0f);
  gl4duTranslatef(-0.01f,-0.46f,-0.10f);
  gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f);
  gl4duRotatef(rotation_planette, 1.0f, 0.0f, 0.0f);
  assimpDrawScene(_id_scene2);
  gl4duPopMatrix();

  //skydome
  gl4duPushMatrix();
  gl4duScalef(indice_taille_sky, indice_taille_sky, indice_taille_sky);
  gl4duRotatef(rotation_sky, 0.50f, 1.50f, 0.0f);
  assimpDrawScene(_id_scene3);
  gl4duPopMatrix();
  //soleil
  GLint useColorLocation = glGetUniformLocation(_pId, "useColor");
  glUniform1i(useColorLocation, 0); // Activer l'utilisation de la couleur uniforme pour le cube
  
  // Désactiver l'utilisation de la couleur uniforme et activer l'utilisation de la texture
  glUniform1i(useColorLocation, 0);
  glUniform1i(glGetUniformLocation(_pId, "useTexture"), 1);
  // Liaison de la texture à l'unité de texture 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureId_soleil);
  gl4duPushMatrix();
  gl4duScalef(8.0f, 8.0f, 8.0f);  // Mise à l'échelle en fonction de la taille du cube
  gl4duTranslatef(0.0f, 0.1f* cos(a) , -4.0f);
  //gl4duRotatef(90.0f, 1.0f, 0.0f, 0.0f);
  //gl4duRotatef(rotation_planette, 1.0f, 0.0f, 0.0f);
  gl4duSendMatrices();
  gl4dgDraw(_cube);
  gl4duPopMatrix();
    
  // Désactiver l'utilisation de la texture
  glUniform1i(glGetUniformLocation(_pId, "useTexture"), 0);
  glBindTexture(GL_TEXTURE_2D, 0); // Désactivation de la texture
  a += 0.4f * M_PI * get_dt();
  rotation_planette = rotation_planette + indice_rotation_planette;
  rotation_sky = rotation_sky + 0.15f;
  position_voiture[2] = cos(a);
}
static void draw_credit(void){
  const GLfloat inclinaison = -20.0;
  static GLfloat t0 = -1;
  GLfloat t, d;
  if(t0 < 0.0f)
    t0 = SDL_GetTicks();
  t = (SDL_GetTicks() - t0) / 1000.0f, d = -1.4f /* du retard pour commencer en bas */ + 0.15f /* vitesse */ * t;
  glClearColor(0.129f, 0.69f, 0.886f, 1);// 1, 0, 0, 1
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _textTexId);
  glUniform1i(glGetUniformLocation(_pId, "inv"), 1);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  gl4duScalef(1, 5, 1);
  gl4duTranslatef(0, d * cos(inclinaison * M_PI / 180.0f), -2 + d * sin(inclinaison * M_PI / 180.0f));
  gl4duRotatef(inclinaison, 1, 0, 0);
  gl4duSendMatrices();
  gl4dgDraw(_quad);
  glUseProgram(0);
}
static void draw(void) {
  if (credits == 1 && hehe == 0){
    initialisation_credits();
  }
  if (credits == 1 && hehe == 1){ 
    draw_credit();
  }else{
    draw_scene();
  }
  //printf("%d", credits);
  //printf("hehe : %d", hehe);
}

//fonction de basic_audio1.0

/*!\brief Cette fonction est appelée quand l'audio est joué et met 
 * dans \a stream les données audio de longueur \a len.
 * \param udata pour user data, données passées par l'utilisateur, ici NULL.
 * \param stream flux de données audio.
 * \param len longueur de \a stream.
*/
static void mixCallback(void *udata, Uint8 *stream, int len) {
  int i;
  Sint16 *s = (Sint16 *)stream;
  if(len >= 2 * ECHANTILLONS){
    for(i = 0; i < ECHANTILLONS; i++){
      _hauteurs[i] = _windowWidth / 2 + (_windowWidth / 2) * s[i] / ((1 << 15) - 1.0);
    }
  }
  return;
}
/*!\brief Cette fonction initialise les paramètres SDL_Mixer et charge
 *  le fichier audio.*/
static void initAudio(const char * filename) {
  int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_MOD, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliotheque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    //exit(3); commenté car ne réagit correctement sur toutes les architectures
  }
  if(Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024) < 0){
    printf("erreur ouverture audio\n");//
    exit(4);}
  if(!(_mmusic = Mix_LoadMUS(filename))) {
    fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
    exit(5);
  }
  Mix_SetPostMix(mixCallback, NULL);
  if(!Mix_PlayingMusic()){
    if (_mmusic == NULL)
      printf("musique non chargée\n");
    else
      printf("musique chargée\n");
    Mix_PlayMusic(_mmusic, 1);
  }
}

static void initText(GLuint * ptId, const char * text) {
  static int firstTime = 1;
  SDL_Color c = {233, 207, 249, 255};//255, 255, 0, 255//rose
  SDL_Surface * d, * s;
  TTF_Font * font = NULL;
  if(firstTime) {
    /* initialisation de la bibliothèque SDL2 ttf */
    if(TTF_Init() == -1) {
      fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
      exit(2);
    }
    firstTime = 0;
  }
  if(*ptId == 0) {
    /* initialisation de la texture côté OpenGL */
    glGenTextures(1, ptId);
    glBindTexture(GL_TEXTURE_2D, *ptId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  /* chargement de la font */
  if( !(font = TTF_OpenFont("RetronoidItalic-ln9V.ttf", 256)) ) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    return;
  }
  /* création d'une surface SDL avec le texte */
  d = TTF_RenderUTF8_Blended_Wrapped(font, text, c, 2048);
  if(d == NULL) {
    TTF_CloseFont(font);
    fprintf(stderr, "Erreur lors du TTF_RenderText\n");
    return;
  }
  /* copie de la surface SDL vers une seconde aux spécifications qui correspondent au format OpenGL */
  s = SDL_CreateRGBSurface(0, d->w, d->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  assert(s);
  SDL_BlitSurface(d, NULL, s, NULL);
  SDL_FreeSurface(d);
  /* transfert vers la texture OpenGL */
  glBindTexture(GL_TEXTURE_2D, *ptId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
  fprintf(stderr, "Dimensions de la texture : %d %d\n", s->w, s->h);
  SDL_FreeSurface(s);
  TTF_CloseFont(font);
  glBindTexture(GL_TEXTURE_2D, 0);
}

/*!\brief function called at exit, it cleans all created GL4D objects.
 */
static void quit(void) {
  if(_texId) {
    glDeleteTextures(1, &_texId);
    _texId = 0;
  }
  if(_mmusic) {
    if(Mix_PlayingMusic())
      Mix_HaltMusic();
    Mix_FreeMusic(_mmusic);
    _mmusic = NULL;
  }
  if(_textTexId) {
    glDeleteTextures(1, &_textTexId);
    _textTexId = 0;
  }
  Mix_CloseAudio();
  Mix_Quit();
  //ahClean();//clean l'audio
  gl4duClean(GL4DU_ALL);
}
void initialisation_credits(void){
  Mix_CloseAudio();
  Mix_Quit();
  hehe = 1;
  glEnable(GL_DEPTH_TEST);
  glClearColor(1.0f, 0.7f, 0.7f, 0.0f);//
  _pId = gl4duCreateProgram("<vs>shaders/credits.vs", "<fs>shaders/credits.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  resize(_windowWidth, _windowHeight);
  _quad = gl4dgGenQuadf();
  initText(&_textTexId, 
	   " Merci d’avoir teste mon programme !\n\n\n"
	   " J’espère que vous avez apprecie."
	   " Voici les sources des objets 3D, images, sons et polices utilisees :\n"
	   " Pour les objets 3D : (https://skfb.ly/otqOQ) by wilemben"
	   " 1981 DeLorean DMC-12s (https://skfb.ly/oQ9XK) by ₦₥₵ ฿₵"
	   " Le ciel : 'FREE - SkyBox Space Nebula' (https://skfb.ly/oIIYS) by Paul\n"
	   " Musique de pixabay.com/users/grand_project-19033897\n"
	   " Le soleil de vecteezy.com/png/8507610-abstract-elements-retro-style-80s-90s\n"
	   " Et la police fontspace.com/collection/synthwave-coj0gx1\n\n"
     " ca vaut au moin 10 nan ? :) (oui la police ne prend pas en compte les accents)");
}
/*!\brief the main function.
 */
int main(int argc, char ** argv) {
  /*
  ancien parametres main 
  if(argc != 2) {
    fprintf(stderr, "usage: %s <3d_file>\n", argv[0]);
    return 1;
  }
 */
  if(!gl4duwCreateWindow(argc, argv, "Exemple de loader de modèles 3D", GL4DW_POS_UNDEFINED, GL4DW_POS_UNDEFINED,_windowWidth, _windowHeight, GL4DW_RESIZABLE | GL4DW_SHOWN)){
    return 1;
  }
  //_id_scene = assimpGenScene(argv[1]);
  _id_scene = assimpGenScene("models/voiture2/scene.gltf");
  _id_scene2 = assimpGenScene("models/planet/planet.glb");
  _id_scene3 = assimpGenScene("models/sky/scene.gltf");
  initGL();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwKeyUpFunc(keyup);
  gl4duwKeyDownFunc(keydown);
  gl4duwDisplayFunc(draw);
  gl4duwIdleFunc(idle);
  ///pour la musique
  //ahInitAudio("lady-of-the-80x27s-128379.mp3");//takeonme.mod
  //initialisation de la musique
  const char * filename = "lady-of-the-80x27s-128379.mp3";
  initAudio(filename);//takeonme.mod
  printf("init audio appelée\n");
  gl4duwMainLoop();
  return 0;
}
