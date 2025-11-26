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
    int vertexCount = 0; // number of vertices (not floats)
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
} camera = { 0.0f, 12.0f, 42.0f, 180.0f, -8.0f, 60.0f };
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
bool fogEnabled = true;
bool showHelp = true;   // toggle for showing/hiding the controls overlay meowmeow

// Forward declarations for drawing helpers: clouds, stairs, ground, template, fallen tree (ancient scene)
void drawClouds(SceneType scene);
void drawStairs();
void drawGround(SceneType scene);
void drawTempleDetails(SceneType scene);
void drawFallenTree(float x, float z, float length, float angleDegrees);

// Meshes, the one for the pyramid and for the ground
MeshVBO pyramidMesh;
MeshVBO groundMesh;

// ---------------------- Lights ----------------------

void setupLights(SceneType scene) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Reset all lights each frame
    GLfloat globalAmbient[] = { 0.08f, 0.08f, 0.08f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Common camera direction (for headlight)
    float yawRad = camera.yaw * (float)M_PI / 180.0f;
    float pitchRad = camera.pitch * (float)M_PI / 180.0f;
    float dirX = cosf(pitchRad) * sinf(yawRad);
    float dirY = sinf(pitchRad);
    float dirZ = -cosf(pitchRad) * cosf(yawRad);

    if (scene == ANCIENT_SCENE) {
        // Moonlight (LIGHT0)
        GLfloat ambient0[] = { 0.03f, 0.03f, 0.10f, 1.0f };
        GLfloat diffuse0[] = { 0.15f, 0.15f, 0.35f, 1.0f };
        GLfloat specular0[] = { 0.15f, 0.15f, 0.35f, 1.0f };
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
        GLfloat ambient0[] = { 0.30f, 0.30f, 0.30f, 1.0f };
        GLfloat diffuse0[] = { 0.95f, 0.95f, 0.88f, 1.0f };
        GLfloat specular0[] = { 0.60f, 0.60f, 0.55f, 1.0f };
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
    else                        glColor3f(0.6f, 0.8f, 1.0f);

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
    GLfloat materialSpec[] = { 0.35f, 0.35f, 0.35f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 25.0f);

    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Big visible ground
void drawGround(SceneType scene) {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, -1.0f, 0.0f);
    glScalef(200.0f, 1.0f, 200.0f);

    if (scene == ANCIENT_SCENE)
        glColor3f(0.27f, 0.20f, 0.12f); // earth
    else
        glColor3f(0.33f, 0.78f, 0.30f); // grass

    glutSolidCube(1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// Simple tree: trunk (box) + canopy (scaled sphere-ish), basically rectangular prism + sphere
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

// Simple tourist as a capsule-like figure, another combination of scaled cubes and spheres
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

// Clouds
void drawClouds(SceneType scene) {
    glDisable(GL_LIGHTING);

    // Cloud color per scene
    if (scene == ANCIENT_SCENE)
        glColor3f(0.5f, 0.5f, 0.6f);   // darker, stormy-ish
    else
        glColor3f(0.98f, 0.98f, 0.99f); // bright white

    // Make a ring of cloud clusters above the scene, slowly moving
    float baseRadius = 60.0f;
    float baseHeight = 30.0f;

    for (int i = 0; i < 8; ++i) {
        float angle = (float)i * (2.0f * (float)M_PI / 8.0f)
            + cloudOffset * 0.01f; // slow drift

        float cx = cosf(angle) * baseRadius;
        float cz = sinf(angle) * baseRadius;
        float cy = baseHeight + (i % 2) * 3.0f;

        glPushMatrix();
        glTranslatef(cx, cy, cz);

        // Each cluster: 3–4 overlapping spheres
        glutSolidSphere(4.0, 12, 12);
        glTranslatef(3.0f, 1.0f, 1.5f);
        glutSolidSphere(3.0, 12, 12);
        glTranslatef(-4.0f, 0.0f, -2.0f);
        glutSolidSphere(3.5, 12, 12);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}

void drawFallenTree(float x, float z, float length, float angleDegrees) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(angleDegrees, 0.0f, 1.0f, 0.0f);
    glRotatef(-20.0f, 0.0f, 0.0f, 1.0f); // slight tilt to “lean”

    glDisable(GL_LIGHTING);

    // Trunk
    glColor3f(0.28f, 0.18f, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(length, 0.6f, 0.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Some leaves at one end
    glColor3f(0.05f, 0.25f, 0.05f);
    glPushMatrix();
    glTranslatef(length * 0.5f, 1.3f, 0.0f);
    glScalef(2.0f, 1.5f, 2.0f);
    glutSolidSphere(0.8, 12, 12);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}


// ---------------- Rocks and "dirty" ground for ancient scene ----------------

void drawRock(float x, float z, float scale)
{
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glDisable(GL_LIGHTING);
    glColor3f(0.40f, 0.40f, 0.42f);   // gray rock

    // Slightly squashed sphere to look like a rock
    glPushMatrix();
    glTranslatef(0.0f, 0.3f * scale, 0.0f);
    glScalef(scale, scale * 0.6f, scale);
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

// Darker ground patches near the pyramid to make it look worn / dirty
void drawAncientGroundPatches()
{
    glDisable(GL_LIGHTING);
    glColor3f(0.18f, 0.13f, 0.09f);   // darker dirt to make it more look like abandoned-ish

    auto patch = [](float x, float z, float sx, float sz)
        {
            glPushMatrix();
            glTranslatef(x, -0.8f, z);    // slightly above the big ground cube
            glScalef(sx, 0.4f, sz);
            glutSolidCube(1.0f);
            glPopMatrix();
        };

    // A few irregular patches around the base
    patch(6.0f, 18.0f, 10.0f, 6.0f);
    patch(-8.0f, 16.0f, 7.0f, 5.0f);
    patch(10.0f, -15.0f, 8.0f, 7.0f);
    patch(-12.0f, -17.0f, 9.0f, 6.0f);
    patch(0.0f, 22.0f, 12.0f, 4.0f);

    glEnable(GL_LIGHTING);
}

// Place many rocks around the pyramid in rough rings
void drawRocksAndDebris()
{
    // Ring of rocks around the base
    float baseRadius = 28.0f;

    for (int i = 0; i < 32; ++i)
    {
        float angle = (float)i * (2.0f * (float)M_PI / 32.0f);
        float radius = baseRadius + (i % 4) * 2.5f;   // slightly irregular

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        float s = 0.7f + (i % 3) * 0.25f;             // various sizes

        drawRock(x, z, s);
    }

    // A few larger rocks closer to some stairs
    drawRock(5.0f, 20.0f, 1.2f);
    drawRock(-7.0f, 19.0f, 1.0f);
    drawRock(11.0f, -19.0f, 1.3f);
    drawRock(-10.0f, -18.0f, 1.1f);

    // Dirty patches right after the rocks
    drawAncientGroundPatches();
}


// ---------- Full-height staircases inspired by El Castillo ----------

void drawOneStaircase(float yawDegrees) {
    const float baseHalf = 12.5f;       // match pyramid base
    const float terraceHeight = 1.2f;
    const int   terraceCount = 12;
    const float pyramidHeight = terraceCount * terraceHeight;

    const int   stepCount = 30;
    const float stepHeight = pyramidHeight / (float)stepCount;
    const float baseZStart = baseHalf + 1.2f;
    const float baseZEnd = baseHalf - 10.0f;
    const float stepDepth = 1.2f;

    // Stop the stairs a bit before the very top so they don't overlap the temple, very big problem, readjusted for hours
    const float maxStairY = pyramidHeight - stepHeight * 3.0f;

    glPushMatrix();
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glColor3f(0.78f, 0.74f, 0.68f); // slightly lighter to stand out from terraces

    for (int i = 0; i < stepCount; ++i) {
        float t = (float)i / (float)(stepCount - 1);

        float y = 0.3f + (i + 0.5f) * stepHeight;
        float z = baseZStart + (baseZEnd - baseZStart) * t;
        float width = 7.0f - t * 3.0f; // staircase becomes narrower near the top

        // Clamp stair generation so we don't reach into the temple
        if (y > maxStairY)
            break;

        glPushMatrix();
        glTranslatef(0.0f, y, z);
        glScalef(width, stepHeight * 0.9f, stepDepth);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();
}



void drawStairs() {
    glPushMatrix();
    // Front (+Z)
    drawOneStaircase(0.0f);
    // Right (+X)
    drawOneStaircase(90.0f);
    // Back (-Z)
    drawOneStaircase(180.0f);
    // Left (-X)
    drawOneStaircase(-90.0f);
    glPopMatrix();
}


// ---------------- Temple entrance + decorative details ----------------

void drawTempleDetails(SceneType scene)
{
    // ---- Temple geometry (must match createPyramidMesh) ----

    const int   terraceCount = 9;
    const float terraceHeight = 1.4f;
    const float templeHalfSize = 3.5f;   // half size used in addBox(...)
    const float templeOffsetY = 1.2f;   // same offset used there

    float terracesHeight = terraceCount * terraceHeight;
    float templeCenterY = terracesHeight + templeOffsetY;

    // Small epsilon so quads are slightly in front of temple faces
    const float eps = 0.05f;

    // ---------------- Common colors ----------------
    GLfloat doorColorAncient[3] = { 0.03f, 0.03f, 0.03f };
    GLfloat doorColorModern[3] = { 0.10f, 0.10f, 0.10f };
    GLfloat frameColorAncient[3] = { 0.65f, 0.63f, 0.60f };
    GLfloat frameColorModern[3] = { 0.98f, 0.96f, 0.92f };

    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);   // so all quads are visible from any side

    // Helper lambdas to pick colors
    auto setDoorColor = [&]() {
        if (scene == ANCIENT_SCENE)
            glColor3f(doorColorAncient[0], doorColorAncient[1], doorColorAncient[2]);
        else
            glColor3f(doorColorModern[0], doorColorModern[1], doorColorModern[2]);
        };

    auto setFrameColor = [&]() {
        if (scene == ANCIENT_SCENE)
            glColor3f(frameColorAncient[0], frameColorAncient[1], frameColorAncient[2]);
        else
            glColor3f(frameColorModern[0], frameColorModern[1], frameColorModern[2]);
        };

    // ---------------- Main front entrance (+Z) ----------------
    {
        float doorHalfWidth = 1.4f;
        float doorBottomY = templeCenterY - 1.5f;
        float doorTopY = templeCenterY + 0.5f;
        float zFront = templeHalfSize + eps;

        // Door opening
        setDoorColor();
        glBegin(GL_QUADS);
        glVertex3f(-doorHalfWidth, doorBottomY, zFront);
        glVertex3f(doorHalfWidth, doorBottomY, zFront);
        glVertex3f(doorHalfWidth, doorTopY, zFront);
        glVertex3f(-doorHalfWidth, doorTopY, zFront);
        glEnd();

        // Frame
        setFrameColor();
        float frameDepth = 0.25f;

        // Top lintel
        glBegin(GL_QUADS);
        glVertex3f(-doorHalfWidth - 0.3f, doorTopY + 0.25f, zFront);
        glVertex3f(doorHalfWidth + 0.3f, doorTopY + 0.25f, zFront);
        glVertex3f(doorHalfWidth + 0.3f, doorTopY + 0.75f, zFront + frameDepth);
        glVertex3f(-doorHalfWidth - 0.3f, doorTopY + 0.75f, zFront + frameDepth);
        glEnd();

        // Left column
        glBegin(GL_QUADS);
        glVertex3f(-doorHalfWidth - 0.3f, doorBottomY - 0.1f, zFront);
        glVertex3f(-doorHalfWidth, doorBottomY - 0.1f, zFront);
        glVertex3f(-doorHalfWidth, doorTopY + 0.8f, zFront + frameDepth);
        glVertex3f(-doorHalfWidth - 0.3f, doorTopY + 0.8f, zFront + frameDepth);
        glEnd();

        // Right column
        glBegin(GL_QUADS);
        glVertex3f(doorHalfWidth, doorBottomY - 0.1f, zFront);
        glVertex3f(doorHalfWidth + 0.3f, doorBottomY - 0.1f, zFront);
        glVertex3f(doorHalfWidth + 0.3f, doorTopY + 0.8f, zFront + frameDepth);
        glVertex3f(doorHalfWidth, doorTopY + 0.8f, zFront + frameDepth);
        glEnd();
    }

    // ----------- Helper for smaller side/back entrances ---------
    auto drawSmallEntranceZ = [&](bool backSide) {
        // backSide == false → +Z, true → -Z
        float doorHalfWidth = 0.9f;
        float doorBottomY = templeCenterY - 1.0f;
        float doorTopY = templeCenterY + 0.2f;
        float zSign = backSide ? -1.0f : 1.0f;
        float zFace = zSign * (templeHalfSize + eps);
        float frameDepth = 0.20f;

        // Door
        setDoorColor();
        glBegin(GL_QUADS);
        glVertex3f(-doorHalfWidth, doorBottomY, zFace);
        glVertex3f(doorHalfWidth, doorBottomY, zFace);
        glVertex3f(doorHalfWidth, doorTopY, zFace);
        glVertex3f(-doorHalfWidth, doorTopY, zFace);
        glEnd();

        // Simple frame strip around
        setFrameColor();
        glBegin(GL_QUADS);
        // Top strip
        glVertex3f(-doorHalfWidth - 0.2f, doorTopY + 0.15f, zFace);
        glVertex3f(doorHalfWidth + 0.2f, doorTopY + 0.15f, zFace);
        glVertex3f(doorHalfWidth + 0.2f, doorTopY + 0.40f, zFace + zSign * frameDepth);
        glVertex3f(-doorHalfWidth - 0.2f, doorTopY + 0.40f, zFace + zSign * frameDepth);
        glEnd();
        };

    auto drawSmallEntranceX = [&](bool rightSide) {
        // rightSide == false → -X, true → +X
        float doorHalfWidth = 0.9f;
        float doorBottomY = templeCenterY - 1.0f;
        float doorTopY = templeCenterY + 0.2f;
        float xSign = rightSide ? 1.0f : -1.0f;
        float xFace = xSign * (templeHalfSize + eps);
        float frameDepth = 0.20f;

        // Door
        setDoorColor();
        glBegin(GL_QUADS);
        glVertex3f(xFace, doorBottomY, -doorHalfWidth);
        glVertex3f(xFace, doorBottomY, doorHalfWidth);
        glVertex3f(xFace, doorTopY, doorHalfWidth);
        glVertex3f(xFace, doorTopY, -doorHalfWidth);
        glEnd();

        // Top strip
        setFrameColor();
        glBegin(GL_QUADS);
        glVertex3f(xFace, doorTopY + 0.15f, -doorHalfWidth - 0.2f);
        glVertex3f(xFace, doorTopY + 0.15f, doorHalfWidth + 0.2f);
        glVertex3f(xFace + xSign * frameDepth, doorTopY + 0.40f, doorHalfWidth + 0.2f);
        glVertex3f(xFace + xSign * frameDepth, doorTopY + 0.40f, -doorHalfWidth - 0.2f);
        glEnd();
        };

    // Back (-Z), Left (-X), Right (+X)
    drawSmallEntranceZ(true);   // back side
    drawSmallEntranceX(false);  // left side
    drawSmallEntranceX(true);   // right side

    // ---------------- Dark band around temple top ----------------
    float bandTopY = templeCenterY + templeHalfSize - 0.2f;
    float bandBottomY = bandTopY - 0.4f;
    float bandHalfX = templeHalfSize + 0.05f;
    float bandHalfZ = templeHalfSize + 0.05f;

    if (scene == ANCIENT_SCENE)
        glColor3f(0.35f, 0.35f, 0.36f);
    else
        glColor3f(0.45f, 0.45f, 0.47f);

    glBegin(GL_QUADS);
    // Front
    glVertex3f(-bandHalfX, bandBottomY, bandHalfZ);
    glVertex3f(bandHalfX, bandBottomY, bandHalfZ);
    glVertex3f(bandHalfX, bandTopY, bandHalfZ);
    glVertex3f(-bandHalfX, bandTopY, bandHalfZ);

    // Back
    glVertex3f(-bandHalfX, bandBottomY, -bandHalfZ);
    glVertex3f(-bandHalfX, bandTopY, -bandHalfZ);
    glVertex3f(bandHalfX, bandTopY, -bandHalfZ);
    glVertex3f(bandHalfX, bandBottomY, -bandHalfZ);

    // Left
    glVertex3f(-bandHalfX, bandBottomY, -bandHalfZ);
    glVertex3f(-bandHalfX, bandBottomY, bandHalfZ);
    glVertex3f(-bandHalfX, bandTopY, bandHalfZ);
    glVertex3f(-bandHalfX, bandTopY, -bandHalfZ);

    // Right
    glVertex3f(bandHalfX, bandBottomY, -bandHalfZ);
    glVertex3f(bandHalfX, bandTopY, -bandHalfZ);
    glVertex3f(bandHalfX, bandTopY, bandHalfZ);
    glVertex3f(bandHalfX, bandBottomY, bandHalfZ);
    glEnd();

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
}


// ---------------------- Scenes ----------------------

void drawAncientScene() {
    // Slightly darker, more desaturated stone with a hint of green
    drawMeshLit(pyramidMesh, 0.38f, 0.40f, 0.34f);   // darker, more mossy

    // Staircases cutting up each face
    drawStairs();

    drawTempleDetails(ANCIENT_SCENE);

    // --- NEW: rocks + dirty ground around the pyramid ---
    drawRocksAndDebris();

    // Dense jungle trees around pyramid (more trees + two rings)
    for (int i = 0; i < 100; ++i) {
        float angle = (float)i * (2.0f * (float)M_PI / 48.0f);

        // inner + outer ring effect
        float baseRadius = (i % 2 == 0) ? 32.0f : 40.0f;
        float radius = baseRadius + (i % 5) * 1.5f;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;
        float scale = 0.9f + (i % 3) * 0.30f;   // more size variation

        drawTree(x, z, scale, ANCIENT_SCENE);
    }

    // Fallen tree leaning toward the pyramid front
    drawFallenTree(18.0f, 10.0f, 6.0f, 200.0f);

}


void drawModernScene() {
    // Clean bright limestone
    drawMeshLit(pyramidMesh, 1.0f, 0.96f, 0.90f);

    // Sharp staircases
    drawStairs();

    drawTempleDetails(MODERN_SCENE);

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
            for (int i = 0; i < 3; ++i) {
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

    // More El Castillo–like proportions:
    // 9 terraces, each slightly inset, shorter height,
    // with a small temple on top.
    const int   terraceCount = 9;
    const float baseHalf = 13.5f;
    const float terraceHeight = 1.4f;

    float currentY = terraceHeight * 0.5f;

    for (int i = 0; i < terraceCount; ++i) {
        float halfSize = baseHalf - i * 1.2f;
        addBox(data, halfSize, terraceHeight * 0.5f, halfSize, currentY, 0.0f);
        currentY += terraceHeight;
    }

    // Temple on top (slightly rectangular, like the real one)
    float templeCenterY = currentY + 1.2f;
    addBox(data, 3.5f, 2.0f, 3.5f, templeCenterY, 0.0f);

    mesh.vertexCount = (int)(data.size() / 6);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Ground VBO is still created (to satisfy VBO requirement) but we now
// use the simpler cube-based drawGround() for visual clarity.
void createGroundMesh(MeshVBO& mesh) {
    std::vector<float> data;

    float halfSize = 80.0f;
    float y = 0.0f;

    auto pushTri = [&](float x1, float z1,
        float x2, float z2,
        float x3, float z3)
        {
            float v[9] = { x1,y,z1,x2,y,z2,x3,y,z3 };
            for (int i = 0; i < 3; ++i) {
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

    // -------------------- Title at top left --------------------
    glColor3f(1, 1, 1);
    if (currentScene == ANCIENT_SCENE) {
        renderBitmapString(10, h - 30, "Ancient Chichen Itza - Lost in the Jungle");
    }
    else {
        renderBitmapString(10, h - 30, "Modern Chichen Itza - Tourist Landmark");
    }

    if (!showHelp) {
        // Small hint at bottom-left when help is hidden
        glColor3f(0.8f, 0.8f, 0.8f);
        renderBitmapString(10, 20, "[H] Show controls");
    }
    else {
        // -------------------- Controls panel --------------------
        int x = 10;
        int y = h - 70;    // start a bit below the title
        int dy = 22;       // line spacing

        // Optional: a slightly dark background panel for readability
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(5, y + 10);
        glVertex2f(360, y + 10);
        glVertex2f(360, y - 9 * dy - 10);
        glVertex2f(5, y - 9 * dy - 10);
        glEnd();

        // Controls text
        glColor3f(1.0f, 1.0f, 1.0f);
        renderBitmapString((float)x, (float)y, "Controls:");
        y -= dy;

        renderBitmapString((float)x, (float)y, "  W / A / S / D : Move forward / left / back / right");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  Q / E         : Move up / down");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  Mouse drag    : Look around");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  Arrow keys    : Rotate camera");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  SPACE         : Switch Ancient / Modern scene");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  F             : Toggle fog (Ancient scene)");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  H             : Show / hide this help panel");
        y -= dy;
        renderBitmapString((float)x, (float)y, "  ESC           : Quit application");

        // Dynamic info line (example: fog state if you implemented fogEnabled)
        y -= dy;
        char infoLine[128];

        if (currentScene == ANCIENT_SCENE) {
            // If you don't have fogEnabled, remove this block
            extern bool fogEnabled; // make sure fogEnabled is declared globally
            snprintf(infoLine, sizeof(infoLine),
                "Status: Scene = Ancient | Fog = %s",
                fogEnabled ? "ON" : "OFF");
        }
        else {
            snprintf(infoLine, sizeof(infoLine),
                "Status: Scene = Modern");
        }

        glColor3f(0.8f, 0.9f, 1.0f);
        renderBitmapString((float)x, (float)y, infoLine);
    }

    // Restore matrices
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

    if (currentScene == ANCIENT_SCENE && fogEnabled) {
        glEnable(GL_FOG);
    }
    else {
        glDisable(GL_FOG);
    }

    applyCamera();
    setupLights(currentScene);
    drawSkybox(currentScene);

    // Ground first
    drawGround(currentScene);

    // 3D clouds
    drawClouds(currentScene);

    // Scenes
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
    case 'h': case 'H':
        showHelp = !showHelp;
        break;
    case 'f': case 'F':
        fogEnabled = !fogEnabled;
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

    if (camera.pitch > 89.0f) camera.pitch = 89.0f;
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

void specialCallback(int key, int x, int y) {
    float rotateSpeed = 2.0f;
    float pitchSpeed = 2.0f;

    switch (key) {
    case GLUT_KEY_LEFT:
        camera.yaw -= rotateSpeed;
        break;
    case GLUT_KEY_RIGHT:
        camera.yaw += rotateSpeed;
        break;
    case GLUT_KEY_UP:
        camera.pitch += pitchSpeed;
        if (camera.pitch > 89.0f) camera.pitch = 89.0f;
        break;
    case GLUT_KEY_DOWN:
        camera.pitch -= pitchSpeed;
        if (camera.pitch < -89.0f) camera.pitch = -89.0f;
        break;
    }

    glutPostRedisplay();
}

// ---------------------- Initialization ----------------------

void initGL() {
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // --- Fog base setup (we'll enable/disable per-frame) ---
    glFogi(GL_FOG_MODE, GL_EXP2);          // EXP2 = smoother, atmospheric
    GLfloat fogColor[] = { 0.02f, 0.02f, 0.06f, 1.0f }; // dark bluish night fog
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.015f);        // tweak for more/less fog
    glHint(GL_FOG_HINT, GL_NICEST);
}


void initScene() {
    createPyramidMesh(pyramidMesh);
    createGroundMesh(groundMesh); // kept for VBO usage requirement
}

// ---------------------- main ----------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1980, 1080);
    glutCreateWindow("Chichen Itza Through Time");

    initGL();
    initScene();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutSpecialFunc(specialCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutTimerFunc(16, timerCallback, 0);

    glutMainLoop();
    return 0;
}
