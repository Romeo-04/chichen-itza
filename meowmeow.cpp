// Chichen Itza Project - CS0045
// Two scenes: Ancient (night, overgrown), Modern (day, tourist site)
// Uses: VBOs, vertex arrays, camera, lights, headlight, keyboard/mouse/timer

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>

// ---------------------- Constants & Helpers ----------------------

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct MeshVBO {
    GLuint vbo = 0;
    int    vertexCount = 0; // number of vertices (not floats)
};

// Interleaved: [x y z nx ny nz] per vertex
void createPyramidMesh(MeshVBO& mesh);
void createGroundMesh(MeshVBO& mesh);
void drawMeshLit(const MeshVBO& mesh, float r, float g, float b);

// ---------------------- Camera System ----------------------

struct Camera {
    float x, y, z;
    float yaw;   // rotate around Y axis (left/right)
    float pitch; // rotate around X axis (up/down)
    float fov;
} camera = { 0.0f, 10.0f, 40.0f, 180.0f, -10.0f, 60.0f };
// Start in front of pyramid, looking toward origin

void applyCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float yawRad = camera.yaw * (float)M_PI / 180.0f;
    float pitchRad = camera.pitch * (float)M_PI / 180.0f;

    float dirX = cosf(pitchRad) * sinf(yawRad);
    float dirY = sinf(pitchRad);
    float dirZ = -cosf(pitchRad) * cosf(yawRad);

    gluLookAt(
        camera.x, camera.y, camera.z,
        camera.x + dirX, camera.y + dirY, camera.z + dirZ,
        0.0f, 1.0f, 0.0f
    );
}

void moveCamera(float forward, float right, float up) {
    float yawRad = camera.yaw * (float)M_PI / 180.0f;

    // Forward direction on XZ plane
    float fx = sinf(yawRad);
    float fz = -cosf(yawRad);

    // Right direction on XZ plane
    float rx = cosf(yawRad);
    float rz = sinf(yawRad);

    camera.x += fx * forward + rx * right;
    camera.z += fz * forward + rz * right;
    camera.y += up;
}

// ---------------------- Scene Management ----------------------

enum SceneType { ANCIENT_SCENE = 0, MODERN_SCENE = 1 };
SceneType currentScene = ANCIENT_SCENE;

// Animation globals
float timeSeconds = 0.0f;
float cloudOffset = 0.0f;
float touristBounce = 0.0f;

// Meshes
MeshVBO pyramidMesh;
MeshVBO groundMesh;

// Lights
void setupLights(SceneType scene) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Reset all lights each frame
    GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Common camera direction (for headlight)
    float yawRad = camera.yaw * (float)M_PI / 180.0f;
    float pitchRad = camera.pitch * (float)M_PI / 180.0f;
    float dirX = cosf(pitchRad) * sinf(yawRad);
    float dirY = sinf(pitchRad);
    float dirZ = -cosf(pitchRad) * cosf(yawRad);

    if (scene == ANCIENT_SCENE) {
        // Moonlight (LIGHT0)
        GLfloat ambient0[] = { 0.05f, 0.05f, 0.15f, 1.0f };
        GLfloat diffuse0[] = { 0.2f, 0.2f, 0.4f, 1.0f };
        GLfloat specular0[] = { 0.2f, 0.2f, 0.4f, 1.0f };
        GLfloat position0[] = { 30.0f, 40.0f, 10.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
        glLightfv(GL_LIGHT0, GL_POSITION, position0);

        // Headlight attached to camera (LIGHT1)
        GLfloat ambient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GLfloat diffuse1[] = { 0.9f, 0.9f, 0.8f, 1.0f };
        GLfloat specular1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat position1[] = { camera.x, camera.y, camera.z, 1.0f };
        GLfloat spotDir[] = { dirX, dirY, dirZ };

        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
        glLightfv(GL_LIGHT1, GL_POSITION, position1);

        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f); // narrow beam
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10.0f);
    }
    else {
        // Bright sun (LIGHT0)
        GLfloat ambient0[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat diffuse0[] = { 0.9f, 0.9f, 0.8f, 1.0f };
        GLfloat specular0[] = { 0.6f, 0.6f, 0.5f, 1.0f };
        GLfloat position0[] = { 0.0f, 60.0f, 30.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
        glLightfv(GL_LIGHT0, GL_POSITION, position0);

        glDisable(GL_LIGHT1); // No headlight in modern scene
    }
}

// ---------------------- Drawing Helpers ----------------------

void drawSkybox(SceneType scene) {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    if (scene == ANCIENT_SCENE) {
        glClearColor(0.02f, 0.02f, 0.08f, 1.0f); // dark night
    }
    else {
        glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // bright day
    }

    // Simple large cube as sky
    float size = 500.0f;
    glPushMatrix();
    glTranslatef(camera.x, camera.y, camera.z);
    glBegin(GL_QUADS);
    if (scene == ANCIENT_SCENE) glColor3f(0.02f, 0.02f, 0.08f);
    else                         glColor3f(0.6f, 0.8f, 1.0f);

    // +Z
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    // -Z
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);
    // +X
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);
    // -X
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    // +Y
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void drawMeshLit(const MeshVBO& mesh, float r, float g, float b) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    GLsizei stride = 6 * sizeof(GLfloat);
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat)));

    GLfloat materialDiffuse[] = { r, g, b, 1.0f };
    GLfloat materialAmbient[] = { r * 0.3f, g * 0.3f, b * 0.3f, 1.0f };
    GLfloat materialSpec[] = { 0.4f, 0.4f, 0.4f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 30.0f);

    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Simple tree: trunk (box) + canopy (scaled sphere-ish)
void drawTree(float x, float z, float scale, SceneType scene) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glScalef(scale, scale, scale);

    glDisable(GL_LIGHTING);
    // Trunk
    glColor3f(0.35f, 0.2f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    glScalef(0.5f, 4.0f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Canopy
    if (scene == ANCIENT_SCENE) glColor3f(0.05f, 0.25f, 0.05f);
    else                        glColor3f(0.1f, 0.5f, 0.1f);

    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    glScalef(2.5f, 2.5f, 2.5f);
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// Simple tourist as a capsule-like figure
void drawTourist(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f + touristBounce, z);

    glDisable(GL_LIGHTING);
    // Body
    glColor3f(0.2f, 0.4f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(0.7f, 1.5f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Head
    glColor3f(1.0f, 0.8f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 2.1f, 0.0f);
    glutSolidSphere(0.35, 10, 10);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// ---------------------- Scenes ----------------------

void drawAncientScene() {
    // Ground (dark dirt)
    drawMeshLit(groundMesh, 0.2f, 0.15f, 0.1f);

    // Pyramid (weathered stone)
    drawMeshLit(pyramidMesh, 0.6f, 0.5f, 0.4f);

    // Dense jungle trees around pyramid
    for (int i = 0; i < 32; ++i) {
        float angle = (float)i * (2.0f * (float)M_PI / 32.0f);
        float radius = 35.0f + (i % 5) * 2.0f;
        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        float scale = 1.0f + (i % 3) * 0.2f;
        drawTree(x, z, scale, ANCIENT_SCENE);
    }
}

void drawModernScene() {
    // Ground (green grass)
    drawMeshLit(groundMesh, 0.4f, 0.7f, 0.3f);

    // Pyramid (clean bright limestone)
    drawMeshLit(pyramidMesh, 1.0f, 0.95f, 0.85f);

    // Fewer, placed trees (landscaped)
    drawTree(-25.0f, -25.0f, 1.5f, MODERN_SCENE);
    drawTree(25.0f, -25.0f, 1.3f, MODERN_SCENE);
    drawTree(-25.0f, 25.0f, 1.4f, MODERN_SCENE);
    drawTree(25.0f, 25.0f, 1.2f, MODERN_SCENE);

    // Tourists near the front of pyramid
    drawTourist(-5.0f, 18.0f);
    drawTourist(0.0f, 20.0f);
    drawTourist(5.0f, 22.0f);
    drawTourist(10.0f, 18.0f);
}

// ---------------------- VBO Creation ----------------------

void addBox(std::vector<float>& data,
    float halfSizeX, float halfSizeY, float halfSizeZ,
    float centerY, float centerZOffset)
{
    // center at (0, centerY, centerZOffset)
    float cx = 0.0f;
    float cy = centerY;
    float cz = centerZOffset;

    // Convenience lambda to push one triangle
    auto pushTri = [&](float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3,
        float nx, float ny, float nz)
        {
            float v[9] = { x1,y1,z1,x2,y2,z2,x3,y3,z3 };
            for (int i = 0;i < 3;i++) {
                data.push_back(v[i * 3 + 0]);
                data.push_back(v[i * 3 + 1]);
                data.push_back(v[i * 3 + 2]);
                data.push_back(nx);
                data.push_back(ny);
                data.push_back(nz);
            }
        };

    float x0 = cx - halfSizeX, x1 = cx + halfSizeX;
    float y0 = cy - halfSizeY, y1 = cy + halfSizeY;
    float z0 = cz - halfSizeZ, z1 = cz + halfSizeZ;

    // Top (+Y)
    pushTri(x0, y1, z0, x1, y1, z0, x1, y1, z1, 0, 1, 0);
    pushTri(x0, y1, z0, x1, y1, z1, x0, y1, z1, 0, 1, 0);

    // Bottom (-Y)
    pushTri(x0, y0, z0, x1, y0, z1, x1, y0, z0, 0, -1, 0);
    pushTri(x0, y0, z0, x0, y0, z1, x1, y0, z1, 0, -1, 0);

    // Front (+Z)
    pushTri(x0, y0, z1, x1, y0, z1, x1, y1, z1, 0, 0, 1);
    pushTri(x0, y0, z1, x1, y1, z1, x0, y1, z1, 0, 0, 1);

    // Back (-Z)
    pushTri(x0, y0, z0, x1, y1, z0, x1, y0, z0, 0, 0, -1);
    pushTri(x0, y0, z0, x0, y1, z0, x1, y1, z0, 0, 0, -1);

    // Left (-X)
    pushTri(x0, y0, z0, x0, y0, z1, x0, y1, z1, -1, 0, 0);
    pushTri(x0, y0, z0, x0, y1, z1, x0, y1, z0, -1, 0, 0);

    // Right (+X)
    pushTri(x1, y0, z0, x1, y1, z1, x1, y0, z1, 1, 0, 0);
    pushTri(x1, y0, z0, x1, y1, z0, x1, y1, z1, 1, 0, 0);
}

void createPyramidMesh(MeshVBO& mesh) {
    std::vector<float> data;

    // Step pyramid: 9 levels + small temple
    float baseHalf = 15.0f;
    float levelHeight = 2.0f;
    float y = levelHeight;

    for (int i = 0; i < 9; ++i) {
        float halfSize = baseHalf - i * 1.5f;
        float centerY = y;
        addBox(data, halfSize, levelHeight * 0.5f, halfSize, centerY, 0.0f);
        y += levelHeight;
    }

    // Temple on top
    addBox(data, 3.0f, 2.0f, 3.0f, y + 1.0f, 0.0f);

    mesh.vertexCount = (int)(data.size() / 6);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void createGroundMesh(MeshVBO& mesh) {
    std::vector<float> data;

    float halfSize = 80.0f;
    float y = 0.0f;

    auto pushTri = [&](float x1, float z1,
        float x2, float z2,
        float x3, float z3)
        {
            float v[9] = { x1,y,z1,x2,y,z2,x3,y,z3 };
            for (int i = 0;i < 3;i++) {
                data.push_back(v[i * 3 + 0]);
                data.push_back(v[i * 3 + 1]);
                data.push_back(v[i * 3 + 2]);
                data.push_back(0.0f);
                data.push_back(1.0f);
                data.push_back(0.0f);
            }
        };

    // Two triangles forming a big square
    pushTri(-halfSize, -halfSize, halfSize, -halfSize, halfSize, halfSize);
    pushTri(-halfSize, -halfSize, halfSize, halfSize, -halfSize, halfSize);

    mesh.vertexCount = (int)(data.size() / 6);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ---------------------- Text (HUD) ----------------------

void renderBitmapString(float x, float y, const char* string) {
    glRasterPos2f(x, y);
    while (*string) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *string);
        ++string;
    }
}

void drawHUD() {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1, 1, 1);
    if (currentScene == ANCIENT_SCENE) {
        renderBitmapString(10, h - 30, "Ancient Chichen Itza - Lost in the Jungle");
    }
    else {
        renderBitmapString(10, h - 30, "Modern Chichen Itza - Tourist Landmark");
    }
    renderBitmapString(10, 40, "SPACE: Switch scene   WASD/QE: Move   Mouse-drag: Look   ESC: Quit");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ---------------------- GLUT Callbacks ----------------------

bool dragging = false;
int lastMouseX = 0, lastMouseY = 0;

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyCamera();
    setupLights(currentScene);
    drawSkybox(currentScene);

    if (currentScene == ANCIENT_SCENE) {
        drawAncientScene();
    }
    else {
        drawModernScene();
    }

    drawHUD();

    glutSwapBuffers();
}

void reshapeCallback(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera.fov, aspect, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboardCallback(unsigned char key, int x, int y) {
    float moveSpeed = 1.5f;

    switch (key) {
    case 'w': case 'W':
        moveCamera(moveSpeed, 0.0f, 0.0f);
        break;
    case 's': case 'S':
        moveCamera(-moveSpeed, 0.0f, 0.0f);
        break;
    case 'a': case 'A':
        moveCamera(0.0f, -moveSpeed, 0.0f);
        break;
    case 'd': case 'D':
        moveCamera(0.0f, moveSpeed, 0.0f);
        break;
    case 'q': case 'Q':
        moveCamera(0.0f, 0.0f, moveSpeed);
        break;
    case 'e': case 'E':
        moveCamera(0.0f, 0.0f, -moveSpeed);
        break;
    case ' ':
        currentScene = (currentScene == ANCIENT_SCENE) ? MODERN_SCENE : ANCIENT_SCENE;
        break;
    case 27: // ESC
        exit(0);
        break;
    }
    glutPostRedisplay();
}

void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            dragging = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else {
            dragging = false;
        }
    }
}

void motionCallback(int x, int y) {
    if (!dragging) return;

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;

    float sensitivity = 0.3f;
    camera.yaw += dx * sensitivity;
    camera.pitch -= dy * sensitivity;

    if (camera.pitch > 89.0f)  camera.pitch = 89.0f;
    if (camera.pitch < -89.0f) camera.pitch = -89.0f;

    glutPostRedisplay();
}

void timerCallback(int value) {
    timeSeconds += 0.016f;
    cloudOffset += 0.05f;
    if (cloudOffset > 100.0f) cloudOffset = 0.0f;

    touristBounce = 0.1f * sinf(timeSeconds * 2.0f);

    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);
}

// ---------------------- Initialization ----------------------

void initGL() {
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void initScene() {
    createPyramidMesh(pyramidMesh);
    createGroundMesh(groundMesh);
}

// ---------------------- main ----------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 900);
    glutCreateWindow("Chichen Itza Through Time");

    initGL();
    initScene();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();
    return 0;
}
