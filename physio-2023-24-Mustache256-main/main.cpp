#include <stdlib.h>
#include <GL/glut.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "Constants.h"
#include "ProcessManager.h"
#include "Pipe.h"

#if USING_NEW_VEC3_CLASS
    #include "Vec3.h"
#else
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
#endif

#if USING_NEW_BOX_CLASS
    #include "Box.h"
#else
    // the box (falling item)
    struct Box {
        Vec3 position;
        Vec3 size;
        Vec3 velocity;
        Vec3 colour; 
    };
#endif


using namespace std::chrono;

//Process manager definition
ProcessManager* physProcessManager;
//Stores number of boxes per process
int boxesPerProcess;
//Stores main process ID, wpid used in checking that all the child processes have finished executing
pid_t mainProcessId, wpid;
//Tracks whether a process maanger has been created
bool managerInstanceCreated = false;

int status = 0;

// gravity - change it and see what happens (usually negative!)
const float gravity = -19.81f;
std::vector<Box> boxes;
//Pointer to shared memory
void* shmem;

//Function that creates the shared memory
void* CreateSharedMemory(size_t size)
{
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    return mmap(NULL, size, protection, visibility, -1, 0);
}

void initScene(int boxCount) {
    for (int i = 0; i < boxCount; ++i) {
        Box box;
#if USING_NEW_BOX_CLASS
        // Assign random x, y, and z positions within specified ranges
        box.GenRandPos(box);

        box.SetBoxSize(box, 1.0f, 1.0f, 1.0f);

        // Assign random x-velocity between -1.0f and 1.0f
        box.GenRandVel(box);

        // Assign a random color to the box
        box.GenRandCol(box);

        boxes.push_back(box);
#else
        box.position.x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        box.position.y = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
        box.position.z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));

        float randomXVelocity = -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
        box.velocity = {randomXVelocity, 0.0f, 0.0f};

        box.colour.x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box.colour.y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        box.colour.z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

        box.size = {1.0f, 1.0f, 1.0f};
#endif
    }

#if USING_MULTIPROCESSING
    //Calculates number of boxes that need to be handled per process
    boxesPerProcess = boxCount / NUMBER_OF_PHYS_PROCESSES;
    //Store ID of main process
    mainProcessId = getpid();
    //Initialise shared memory that the process manager can use
    shmem = CreateSharedMemory(sizeof(ProcessManager));
#endif
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
#if USING_MULTIPROCESSING

    //CHecks to make sure that this process has got th eprocess amanger from shared memory
    if(physProcessManager == nullptr)
    {
        printf("\nphysProcessManager is null for process with id %d", getpid());
        return;
    }

    //Finds Process object associated with this process
    Process* thisProcess = physProcessManager->GetProcessById(getpid());

    //Checks that it successfully found its associated Process object
    if(thisProcess == nullptr)
    {
        printf("\nCannot use this process to handle box physics, as it is not a part of physProcessManager\n");
        return;
    }

    //WIll not execute if tasks are already complete
    if(thisProcess->tasksComplete)
        return;

    std::vector<Box> boxesToProcess;

    //Get boxes that this process will handle and store them in local vector
    for(int i = 0; i < thisProcess->numOfBoxes; i++)
    {
        boxesToProcess.push_back(boxes[thisProcess->boxIndex + i]);
    }

    //Get pipe associated with this process and store it locally
    Pipe* processPipe = physProcessManager->GetPipe(thisProcess->pipeIndex);

    for (Box& box : boxesToProcess) {
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
        for (Box& other : boxesToProcess) {
            if (&box == &other) continue;
            if (checkCollision(box, other)) {
                resolveCollision(box, other);
                break;
            }
        }

        //Close read end as this process does not need it
        close(processPipe->GetReadEnd());

        //Writes all the new data for the current box to the pipe 
        if(write(processPipe->GetWriteEnd(), &box.position.x, sizeof(float)) == -1)
        {
            printf("Error with writing box pos x to pipe");
            exit(1);
        }
        
        if(write(processPipe->GetWriteEnd(), &box.position.y, sizeof(float)) == -1)
        {
            printf("Error with writing box pos y to pipe");
            exit(1);
        }

        if(write(processPipe->GetWriteEnd(), &box.position.z, sizeof(float)) == -1)
        {
            printf("Error with writing box pos z to pipe");
            exit(1);
        }

        if(write(processPipe->GetWriteEnd(), &box.velocity.x, sizeof(float)) == -1)
        {
            printf("Error with writing box vel x to pipe");
            exit(1);
        }

        if(write(processPipe->GetWriteEnd(), &box.velocity.y, sizeof(float)) == -1)
        {
            printf("Error with writing box vel y to pipe");
            exit(1);
        }

        if(write(processPipe->GetWriteEnd(), &box.velocity.z, sizeof(float)) == -1)
        {
            printf("Error with writing box vel z to pipe");
            exit(1);
        }
    }

    //Closing write end as all data has been written
    close(processPipe->GetWriteEnd());

    //End this process
    thisProcess->tasksComplete = true;
    thisProcess->~Process();
#else
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
#endif
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

void updateBoxesForRender()
{
    int boxIndex = 0;

    //Loops through all the pipes to read from
    for(Pipe* pipe : physProcessManager->GetPipes())
    {   
        printf("Updating boxes on main...\n");
        //Close write end as it is not needed
        close(pipe->GetWriteEnd());

        float buf;

        //Checks to make sure pipe has data within it, if not then print error message and skip this loop iteration 
        if(int nbytes = read(pipe->GetReadEnd(), &buf, sizeof(float)) == 0)
        {
            printf("Pipe empty, cannot update boxes\n");
            continue;
        }

        //Read data from pipe and update local boxes using said data 
        for(int i = 0; i < physProcessManager->GetBoxesPerProcess(); i++)
        {
            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box pos x from pipe");
                exit(1);
            }
            printf("New pos x: %.6f\n", buf);
            boxes[boxIndex + i].position.x = buf;

            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box pos y from pipe");
                exit(1);
            }
            printf("New pos y: %.6f\n", buf);
            boxes[boxIndex + i].position.y = buf;

            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box pos z from pipe");
                exit(1);
            }
            printf("New pos z: %.6f\n", buf);
            boxes[boxIndex + i].position.z = buf;

            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box vel x from pipe");
                exit(1);
            }
            printf("New vel x: %.6f\n", buf);
            boxes[boxIndex + i].velocity.x = buf;

            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box vel y from pipe");
                exit(1);
            }
            printf("New vel y: %.6f\n", buf);
            boxes[boxIndex + i].velocity.y = buf;

            if(read(pipe->GetReadEnd(), &buf, sizeof(float)) == -1)
            {
                printf("Error reading box vel z from pipe");
                exit(1);
            }
            printf("New vel z: %.6f\n", buf);
            boxes[boxIndex + i].velocity.z = buf;
        }

        //Check to make sure all data has been read out of this pipe, if not print error message
        if(int nbytes = read(pipe->GetReadEnd(), &buf, sizeof(float)) != 0)
        {
            printf("Pipe not empty, did not read everthing from pipe, data missed when updating boxes");
        }

        //Close read end as all data has been read
        close(pipe->GetReadEnd());
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

//Can toggle whether multiprocessing is being used
#if USING_MULTIPROCESSING
    if(getpid() == mainProcessId && !managerInstanceCreated)
    {
        //Creates new process manager with processes and pipes each frame and makes it shared memory
        physProcessManager = new ProcessManager(mainProcessId, NUMBER_OF_PHYS_PROCESSES, true);
        memcpy(shmem, physProcessManager, sizeof(physProcessManager));
        managerInstanceCreated = true;
    }

    //Only does physics updates if this is child process    
    if(getpid() != mainProcessId)
    {
        //Gets process manager from shared memory to store locally in this process
        physProcessManager = (ProcessManager*)shmem;
        updatePhysics(deltaTime);
    } 
    else
    {
        while((wpid = wait(&status)) > 0)
        {
            //Do nothing until all children have finished executing 
        }
        //Updates local box values
        updateBoxesForRender();
        // tell glut to draw - note this will cap this function at 60 fps
        glutPostRedisplay();
        //Deletes this frame's process manager
        delete physProcessManager;
        //THis means that a new process maanger can be made
        managerInstanceCreated = false;
    }
#else
    updatePhysics(deltaTime);
    glutPostRedisplay();
#endif
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

    if (key == 'p' && mainProcessId == getpid()){
        printf("Parent process id: %d \n", getpid());
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
