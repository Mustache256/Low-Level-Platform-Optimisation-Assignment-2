// TODO 
// Tidy and label the code
// add memory chunks to the box
// C++ it all
// convert to PS4? (or make this a student task?) - check out the PS4 SDK samples
// rename project etc. 


#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std::chrono;

// this is the number of falling physical items. 
#define NUMBER_OF_BOXES 50

// these is where the camera is, where it is looking and the bounds of the continaing box. You shouldn't need to alter these

#define LOOKAT_X 10
#define LOOKAT_Y 10
#define LOOKAT_Z 50

#define LOOKDIR_X 10
#define LOOKDIR_Y 0
#define LOOKDIR_Z 0

#define minX -10.0f
#define maxX 30.0f
#define minZ -30.0f
#define maxZ 30.0f


class Vec3 {
public:
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // overload the minus operator
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    // Normalize the vector
    void normalise() {
        float length = std::sqrt(x * x + y * y + z * z);
        if (length != 0) {
            x /= length;
            y /= length;
            z /= length;
        }
    }

    // get the length of a vector
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
};

// the box (falling item)
struct Box {
    Vec3 position;
    Vec3 size;
    Vec3 velocity;
    Vec3 colour; 
};


// gravity - change it and see what happens (usually negative!)
const float gravity = -19.81f;
std::vector<Box> boxes;

void initScene(int boxCount) {
    for (int i = 0; i < boxCount; ++i) {
        Box box;

        // Assign random x, y, and z positions within specified ranges
        box.position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        box.position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        box.position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        box.size = {1.0f, 1.0f, 1.0f};

        // Assign random x-velocity between -1.0f and 1.0f
        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        box.velocity = {randomXVelocity, 0.0f, 0.0f};

        // Assign a random color to the box
        box.colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box.colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box.colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        boxes.push_back(box);
    }
}

// a ray which is used to tap (by default, remove) a box - see the 'mouse' function for how this is used.
bool rayBoxIntersection(const Vec3& rayOrigin, const Vec3& rayDirection, const Box& box) {
    float tMin = (box.position.x - box.size.x / 2.0f - rayOrigin.x) / rayDirection.x;
    float tMax = (box.position.x + box.size.x / 2.0f - rayOrigin.x) / rayDirection.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (box.position.y - box.size.y / 2.0f - rayOrigin.y) / rayDirection.y;
    float tyMax = (box.position.y + box.size.y / 2.0f - rayOrigin.y) / rayDirection.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax))
        return false;

    if (tyMin > tMin)
        tMin = tyMin;

    if (tyMax < tMax)
        tMax = tyMax;

    float tzMin = (box.position.z - box.size.z / 2.0f - rayOrigin.z) / rayDirection.z;
    float tzMax = (box.position.z + box.size.z / 2.0f - rayOrigin.z) / rayDirection.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax))
        return false;

    return true;
}

// used in the 'mouse' tap function to convert a screen point to a point in the world
Vec3 screenToWorld(int x, int y) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    return Vec3((float)posX, (float)posY, (float)posZ);
}


// if two boxes collide, push them away from each other
void resolveCollision(Box& a, Box& b) {
    Vec3 normal = { a.position.x - b.position.x, a.position.y - b.position.y, a.position.z - b.position.z };
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

    // Normalize the normal vector
    normal.normalise();

    float relativeVelocityX = a.velocity.x - b.velocity.x;
    float relativeVelocityY = a.velocity.y - b.velocity.y;
    float relativeVelocityZ = a.velocity.z - b.velocity.z;

    // Compute the relative velocity along the normal
    float impulse = relativeVelocityX * normal.x + relativeVelocityY * normal.y + relativeVelocityZ * normal.z;

    // Ignore collision if objects are moving away from each other
    if (impulse > 0) {
        return;
    }

    // Compute the collision impulse scalar
    float e = 0.01f; // Coefficient of restitution (0 = inelastic, 1 = elastic)
    float dampening = 0.9f; // Dampening factor (0.9 = 10% energy reduction)
    float j = -(1.0f + e) * impulse * dampening;

    // Apply the impulse to the boxes' velocities
    a.velocity.x += j * normal.x;
    a.velocity.y += j * normal.y;
    a.velocity.z += j * normal.z;
    b.velocity.x -= j * normal.x;
    b.velocity.y -= j * normal.y;
    b.velocity.z -= j * normal.z;
}

// are two boxes colliding?
bool checkCollision(const Box& a, const Box& b) {
    return (std::abs(a.position.x - b.position.x) * 2 < (a.size.x + b.size.x)) &&
        (std::abs(a.position.y - b.position.y) * 2 < (a.size.y + b.size.y)) &&
        (std::abs(a.position.z - b.position.z) * 2 < (a.size.z + b.size.z));
}

// update the physics: gravity, collision test, collision resolution
void updatePhysics(const float deltaTime) {
    const float floorY = 0.0f;


    for (Box& box : boxes) {
        // Update velocity due to gravity
        box.velocity.y += gravity * deltaTime;

        // Update position based on velocity
        box.position.x += box.velocity.x * deltaTime;
        box.position.y += box.velocity.y * deltaTime;
        box.position.z += box.velocity.z * deltaTime;

        // Check for collision with the floor
        if (box.position.y - box.size.y / 2.0f < floorY) {
            box.position.y = floorY + box.size.y / 2.0f;
            float dampening = 0.7f;
            box.velocity.y = -box.velocity.y * dampening;
        }

        // Check for collision with the walls
        if (box.position.x - box.size.x / 2.0f < minX || box.position.x + box.size.x / 2.0f > maxX) {
            box.velocity.x = -box.velocity.x;
        }
        if (box.position.z - box.size.z / 2.0f < minZ || box.position.z + box.size.z / 2.0f > maxZ) {
            box.velocity.z = -box.velocity.z;
        }

        // Check for collisions with other boxes
        for (Box& other : boxes) {
            if (&box == &other) continue;
            if (checkCollision(box, other)) {
                resolveCollision(box, other);
                break;
            }
        }
    }
}

// draw the sides of the containing area
void drawQuad(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4) {
    
    glBegin(GL_QUADS);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glEnd();
}

// draw the physics object
void drawBox(const Box& box) {
    glPushMatrix();
    glTranslatef(box.position.x, box.position.y, box.position.z);
    GLfloat diffuseMaterial[] = { box.colour.x, box.colour.y, box.colour.z, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);
    glScalef(box.size.x, box.size.y, box.size.z);
    glRotatef(-90, 1, 0, 0);
    glutSolidCube(1.0);
    //glutSolidTeapot(1);
    //glutSolidCone(1, 1, 10, 10);
    glPopMatrix();
}

// draw the entire scene
void drawScene() {

    // Draw the side wall
    GLfloat diffuseMaterial[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMaterial);

    // Draw the left side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 leftSideWallV1(minX, 0.0f, maxZ);
    Vec3 leftSideWallV2(minX, 50.0f, maxZ);
    Vec3 leftSideWallV3(minX, 50.0f, minZ);
    Vec3 leftSideWallV4(minX, 0.0f, minZ);
    drawQuad(leftSideWallV1, leftSideWallV2, leftSideWallV3, leftSideWallV4);

    // Draw the right side wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 rightSideWallV1(maxX, 0.0f, maxZ);
    Vec3 rightSideWallV2(maxX, 50.0f, maxZ);
    Vec3 rightSideWallV3(maxX, 50.0f, minZ);
    Vec3 rightSideWallV4(maxX, 0.0f, minZ);
    drawQuad(rightSideWallV1, rightSideWallV2, rightSideWallV3, rightSideWallV4);


    // Draw the back wall
    glColor3f(0.5f, 0.5f, 0.5f); // Set the wall color
    Vec3 backWallV1(minX, 0.0f, minZ);
    Vec3 backWallV2(minX, 50.0f, minZ);
    Vec3 backWallV3(maxX, 50.0f, minZ);
    Vec3 backWallV4(maxX, 0.0f, minZ);
    drawQuad(backWallV1, backWallV2, backWallV3, backWallV4);

    for (const Box& box : boxes) {
        drawBox(box);
    }
}

// called by GLUT - displays the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(LOOKAT_X, LOOKAT_Y, LOOKAT_Z, LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z, 0, 1, 0);

    drawScene();

    glutSwapBuffers();
}

// called by GLUT when the cpu is idle - has a timer function you can use for FPS, and updates the physics
// see https://www.opengl.org/resources/libraries/glut/spec3/node63.html#:~:text=glutIdleFunc
// NOTE this may be capped at 60 fps as we are using glutPostRedisplay(). If you want it to go higher than this, maybe a thread will help here. 
void idle() {
    static auto last = steady_clock::now();
    auto old = last;
    last = steady_clock::now();
    const duration<float> frameTime = last - old;
    float deltaTime = frameTime.count();

    updatePhysics(deltaTime);

    // tell glut to draw - note this will cap this function at 60 fps
    glutPostRedisplay();
}

// called the mouse button is tapped
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Get the camera position and direction
        Vec3 cameraPosition(LOOKAT_X, LOOKAT_Y, LOOKAT_Z); // Replace with your actual camera position
        Vec3 cameraDirection(LOOKDIR_X, LOOKDIR_Y, LOOKDIR_Z); // Replace with your actual camera direction

        // Get the world coordinates of the clicked point
        Vec3 clickedWorldPos = screenToWorld(x, y);

        // Calculate the ray direction from the camera position to the clicked point
        Vec3 rayDirection = clickedWorldPos - cameraPosition;
        rayDirection.normalise();

        // Perform a ray-box intersection test and remove the clicked box
        size_t clickedBoxIndex = -1;
        float minIntersectionDistance = std::numeric_limits<float>::max();

        for (size_t i = 0; i < boxes.size(); ++i) {
            if (rayBoxIntersection(cameraPosition, rayDirection, boxes[i])) {
                // Calculate the distance between the camera and the intersected box
                Vec3 diff = boxes[i].position - cameraPosition;
                float distance = diff.length();

                // Update the clicked box index if this box is closer to the camera
                if (distance < minIntersectionDistance) {
                    clickedBoxIndex = i;
                    minIntersectionDistance = distance;
                }
            }
        }

        // Remove the clicked box if any
        if (clickedBoxIndex != -1) {
            boxes.erase(boxes.begin() + clickedBoxIndex);
        }
    }
}

// called when the keyboard is used
void keyboard(unsigned char key, int x, int y) {
    const float impulseMagnitude = 20.0f; // Upward impulse magnitude

    if (key == ' ') { // Spacebar key
        for (Box& box : boxes) {
            box.velocity.y += impulseMagnitude;
        }
    }
}

// the main function. 
int main(int argc, char** argv) {

    srand(static_cast<unsigned>(time(0))); // Seed random number generator
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Simple Physics Simulation");

    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    initScene(NUMBER_OF_BOXES);
    glutDisplayFunc(display);
    glutIdleFunc(idle);

    // it will stick here until the program ends. 
    glutMainLoop();
    return 0;
}
