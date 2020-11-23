//==============================================================================
//                                PROGRAM:
//                                Pool Table
//==============================================================================
//
// Install the following in Package Manager 
//
// Install-Package glew_dynamic
// Install-Package glfw
// Install-Package GLMathematics
// Install-Package freeimage -version 3.16.0
// Install-Package nupengl.core
// Install-Package Soil
// Install-Package Assimp -version 3.0.0
//
//=================================================================================
// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GL includes
#include "shader.h"
#include "camera.h"
#include "model.h"

// Active window
GLFWwindow* window;

// Properties
GLuint sWidth = 1000, sHeight = 800;

// Initial Camera location
glm::vec3 originalLocation = glm::vec3(0.0f, 500.0f, 1500.0f);
glm::vec3 camLocation = originalLocation;

// Camera
Camera camera(camLocation);
glm::mat4 View;

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);       // Light location
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);     // White light
glm::vec3 lightMode(2.0f);                  // 2 is diffuse lighting, 1 is global light, 4 mix of all

// Variables increments for resetting the camera's coordinates
GLfloat xVal = 700.0f;
GLfloat yVal = 700.0f;
GLfloat zVal = 700.0f;

// OBJECTS
struct objects 
{
    string name = "";
    GLfloat x = 0.0;
    GLfloat y = 500.0;
    GLfloat z = 0.0;

    GLfloat xinc = 0.001;
    GLfloat yinc = 0.001;
    GLfloat zinc = 0.001;

    GLfloat angle = 0.0f;
    GLfloat inc = 0.001f;
} ballObj, cueObj, cuetipObj, tableObj, ball2Obj;

// Mouse Variables
double oldX, oldY;
bool firstMouse = false;


// Environmental Boundaries
GLfloat groundLevel = 0.0f;
GLfloat tableTop = 40.0f;

GLfloat tableback = -110.0f;    // Farthest away at start
GLfloat tablefront = 110.0f;    // Closest to camera at start
GLfloat tableleft = -45.0f;     
GLfloat tableright = 45.0f;      
 
// Discover if ball 1 and ball 2 have been hit yet. Starts true for logic purposes
bool hit = true;
bool hit2 = true;


// Original locations & incrementors of objects
GLfloat OGcueZ = 50.0f;
GLfloat OGcueY = 40.0f;
GLfloat OGcueX = -4.0f;  

GLfloat OGballz = 43.0f;
GLfloat OGballx = 0.0f;
GLfloat OGballzinc = -2.0;
GLfloat OGballxinc = -1.0;

GLfloat OGball2z = -30.0f;
GLfloat OGball2x = -35.0f;
GLfloat OGball2zinc = 1.5;
GLfloat OGball2xinc = 1.0;

// Check for ball being pocketed
bool pcketBall1 = false;
bool pcketBall2 = false;
bool cueHit = false;

// Text on Screen
string stringTitle = "Pockets hit (Red ball): ";
string stringTitle2 = "Pockets hit (Black ball): ";


//=================== Prototype function for call back ======================== 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void windowSize_callback(GLFWwindow* window, int width, int height);

void clicked_callback(GLFWwindow* window, int button, int  action, int mode);

/*void clickDrag_callback(GLFWwindow* window, double xPos, double yPos);*/

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//==============================================================================

//=================== Prototype functions for modular functions ======================== 
void tableCollision(objects& obj);

void ballsCollision(objects& obj1, objects& obj2);

void pocketCollision(objects& obj);

void reset();
//=======================================================================================

void init_Resources()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Define the window
    window = glfwCreateWindow(sWidth, sHeight, "COMP3420 - Computer Graphics Project 1", 0, 0);
    glfwMakeContextCurrent(window);

    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();

    // Init Callbacks
    // Callbacks
    glfwSetKeyCallback(window, key_callback);

    glfwSetWindowSizeCallback(window, windowSize_callback);

    glfwSetMouseButtonCallback(window, clicked_callback);

    /*glfwSetCursorPosCallback(window, clickDrag_callback);*/

    glfwSetScrollCallback(window, scroll_callback);

    // Define the viewport dimensions
    glViewport(0, 0, sWidth, sHeight);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);

}

//========== Instructions ====================
// ---------- Camera -----------
// WASD to control camera left right and foward back movements
// Scrolling in or out zooms in or out camera
// Space and Shift raises and lowers camera respectively

// ------ Cue Movement ---------
// Clicking and dragging mouse up on window moves cue closer to ball
// Clicking and dragging mouse down on window moves cue further from the ball
// Left click to move the cue forward by a little bit
// Right click to retract it by a little bit

// ------ System Stuff ----------
// Press H to reset pool table elements + camera view
// Esc to exit scene
//==============================================

// The MAIN function, from here we start our application and run the loop
int main()
{
    init_Resources();

    // ==============================================
    // ============ Set up our Objects ==============
    // ==============================================
    reset(); //Place objects in original locations

    // =======================================================================
    // Shaders
    // =======================================================================
    Shader lampShader("objects/lampVertex.glsl", "objects/lampFragment.glsl");
    Shader lightShader("objects/lightVertex.glsl", "objects/lightFragment.glsl");

    // =======================================================================
    // Models
    // =======================================================================
    Model ball((GLchar*)"objects/ball.obj");        // Cue ball
    Model ball2((GLchar*)"objects/ball2.obj");      // Ball to hit
    Model table((GLchar*)"objects/pooltable.obj");  // CREDIT: https://free3d.com/3d-model/pool-table-v1--600461.html
    Model cue((GLchar*)"objects/poolcue.obj");      // CREDIT: https://free3d.com/3d-model/pool-cue-v1--229730.html
    Model lamp((GLchar*)"objects/lamp.obj");        // CREDIT: https://free3d.com/3d-model/punct-pendant-lamp-86726.html

    // =======================================================================
    // Projection Matrix
    // =======================================================================
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)sWidth / (GLfloat)sHeight, 1.0f, 10000.0f);
    
    lightShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // =======================================================================
    // Define how and where the data will be passed to the shaders
    // =======================================================================
    GLint lightPos = glGetUniformLocation(lightShader.Program, "lightPos");
    GLint viewPos = glGetUniformLocation(lightShader.Program, "viewPos");
    GLint lightCol = glGetUniformLocation(lightShader.Program, "lightColor");
    GLint lightType = glGetUniformLocation(lightShader.Program, "lightType");

    
    // =======================================================================
    // Iterate this block while the window is open
    // =======================================================================
    while (!glfwWindowShouldClose(window))
    {
        // Check and call events
        glfwPollEvents();

        // Clear buffers
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        View = glm::lookAt
        (
            glm::vec3(camLocation.x, camLocation.y, camLocation.z), // Camera is at (0,500,7000), in World Space
            glm::vec3(0, 0, 0),                                     // and looking at the origin
            glm::vec3(0, 1, 0)                                      // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Pass the data in the variables to to go to the vertex shader
        glUniform3f(lightPos, 0.0, 500.f, 0.0);
        glUniform3fv(viewPos, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
        glUniform3fv(lightCol, 1, glm::value_ptr(lightColor));
        glUniform3fv(lightType, 1, glm::value_ptr(lightMode));

        lightShader.Use();

        // The View matrix first...
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));

        // Update the projection matrix previously defined
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Set up the scenery for world space
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        //==========================================================================
        // Draw the Table 
        //========================================================================== 
        glm::mat4 tableModel = glm::mat4(1);

        tableModel = glm::scale(tableModel, glm::vec3(5.0f));
        tableModel = glm::translate(tableModel, glm::vec3(tableObj.x, tableObj.y, tableObj.z));

        // The Model matrix
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(tableModel));
        table.Draw(lightShader);

        //==========================================================================
        // Draw CUE
        //==========================================================================
        glm::mat4 cueModel;
        glm::mat4 cuetip;

        cueModel = glm::mat4(1);
        cuetip = glm::mat4(1);

        cueModel = glm::scale(cueModel, glm::vec3(5.0f));
        cueModel = glm::rotate(cueModel, 0.1f, glm::vec3(0.0, 1.0, 0.0));
        cueModel = glm::translate(cueModel, glm::vec3(cueObj.x, cueObj.y, cueObj.z));

        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(cueModel));

        // Cue hasnt hit anything yet
        if (!cueHit)
            cue.Draw(lightShader);

        //==========================================================================
        // Draw ball
        //==========================================================================
        glm::mat4 ballModel, ball2Model;

        // Check for cue hit on ball if cue hasnt hit anything
        if (!cueHit)
        {
            if (cueObj.z - 2 <= ballObj.z)
            {
                if (hit)
                {
                    ballObj.z = cueObj.z - 0.01;
                    ballObj.z += ballObj.zinc;
                    hit = false;
                    cueHit = true;
                }
            }
        }
        objects* ptr = &ballObj;    // Pointer to ballObj
        objects* ptr2 = &ball2Obj;  // Pointer to ball2Obj

        // BALL 1 - Check for Pocketed Ball
        pocketCollision(*ptr);

       // Ball 2 - Check for Pocketed Ball
        pocketCollision(*ptr2);         

        // Ball 1 Check for collisions on left or right of table
        tableCollision(*ptr); //Change ballObj values by reference

        // Check if ball hit came and went and commence ball movement
        if (!hit)
        {
            ballObj.z += ballObj.zinc;
            ballObj.x += ballObj.xinc;

            // Check for Ball 2 to be hit to commence movement
            if (hit2)
            {
                // BALL 2 - Table Collision code
                tableCollision(*ptr2); // Change ball2Obj values by reference

                ball2Obj.x += ball2Obj.xinc;
                ball2Obj.z += ball2Obj.zinc;
            }

            ballsCollision(*ptr, *ptr2); // Sets ball 2 hit boolean to true to let it know to commence moving

            hit = false; // reset hit
        }

        ballModel = glm::mat4(1);        
        ball2Model = glm::mat4(1);

        ballModel = glm::scale(ballModel, glm::vec3(5.0f)); 
        ball2Model = glm::scale(ball2Model, glm::vec3(5.0f));

        ballModel = glm::translate(ballModel, glm::vec3(ballObj.x, tableTop, ballObj.z));
        ball2Model = glm::translate(ball2Model, glm::vec3(ball2Obj.x, tableTop, ball2Obj.z));

        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(ballModel));

        // Draw ball if it hasnt been pocketed
        if(!pcketBall1)
            ball.Draw(lightShader);
        
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(ball2Model));
        
        // Draw ball if it hasnt been pocketed
        if (!pcketBall2)
            ball2.Draw(lightShader);

        lightShader.Use();

        //==========================================================================
        // Draw the Lamp 
        //==========================================================================
        lampShader.Use();

        // View matrix
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));

        // Projection matrix
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 lampmodel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampmodel));

        glm::mat4 lampModel = glm::mat4(1);
        lampModel = glm::scale(lampModel, glm::vec3(0.6f));
        lampModel = glm::translate(lampModel, glm::vec3(0.0f, 1200.0f, 0.0f));

        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));

        // Display Lamp
        lamp.Draw(lampShader);

        // Swap the buffers
        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}

//=====================  Modular Functions  ===============================
void tableCollision(objects& obj)
{
    // Check for collisions left or right of table
    if (obj.x >= tableright || obj.x <= tableleft)
    {
        obj.xinc *= -1;

        if (obj.x > 0)
            obj.x = tableright;
        if (obj.x < 0)
            obj.x = tableleft;

        // Decay movement of ball
        if (obj.xinc < 0)
            obj.xinc = obj.xinc + (obj.xinc * 0.28);
        if (obj.xinc > 0)
            obj.xinc = obj.xinc - (obj.xinc * 0.28);
    }
    // Check for collisions front or back of table
    if (obj.z >= tablefront || obj.z <= tableback)
    {
        obj.zinc *= -1;

        if (obj.z > 0)
            obj.z = tablefront;
        if (obj.z < 0)
            obj.z = tableback;

        // Decay movement of ball
        if (obj.zinc < 0)
            obj.zinc = obj.zinc + (obj.zinc * 0.30);
        if (obj.zinc > 0)
            obj.zinc = obj.zinc - (obj.zinc * 0.30);
    }
}

void ballsCollision(objects& obj1, objects& obj2)
{
    GLfloat xdif = obj1.x - obj2.x;
    GLfloat zdif = obj1.z - obj2.z;
    GLfloat ydif = obj1.y - obj2.y;

    // Check if a ball has been pocketed
    if ((pcketBall1) || (pcketBall2))
    {
        // Do nothing because there is only one ball
    }
    else // Check for collision since both balls still exist
    {  
        if (sqrt((xdif * xdif) + (ydif * ydif) + (zdif * zdif)) < 5)
        {
            // Once collided, repel and swap speed of translation.
            GLfloat temp = obj2.xinc;
            obj2.xinc = (obj2.xinc * 0.90) * -1;
            obj1.xinc = (obj1.xinc * 0.85) * -1;

            temp = obj2.zinc;
            obj2.zinc = (obj2.zinc * 0.85) * -1;
            obj1.zinc = (obj1.zinc * 0.80) * -1;

            hit2 = true;
        }
    }
}

void pocketCollision(objects& obj)
{
    if ((obj.x >= tableright - 5) && (obj.z <= tableback + 5))                                          // back (farthest away) right hole
    {
        if (obj.name == "ball1") pcketBall1 = true;
        else if (obj.name == "ball2") pcketBall2 = true;
        // cout << "Hit back right pocket " << obj.x << " " << obj.y << " " << obj.z << endl; 
    }
    if ((obj.x <= tableleft + 5) && (obj.z <= tableback + 5))                                           // back (farthest away) left hole
    {
        if (obj.name == "ball1") pcketBall1 = true;
        else if (obj.name == "ball2") pcketBall2 = true;
        // cout << "Hit back left pocket " << obj.x << " " << obj.y << " " << obj.z << endl;
    }
    if (((obj.x >= tableright) || (obj.x <= tableleft)) && ((obj.z >= 0 - 2) && (obj.z <= 0 + 2)))      // Center Hole
    {
        if (obj.name == "ball1") pcketBall1 = true;
        else if (obj.name == "ball2") pcketBall2 = true;
        // cout << "Hit center left or right pocket " << obj.x << " " << obj.y << " " << obj.z << endl;
    }
    if ((obj.x >= tableright - 5) && (obj.z >= tablefront - 5))                                         // front right hole
    {
        if (obj.name == "ball1") pcketBall1 = true;
        else if (obj.name == "ball2") pcketBall2 = true;
        // cout << "Hit front right pocket " << obj.x << " " << obj.y << " " << obj.z << endl;
    }
    if ((obj.x <= tableleft + 5) && (obj.z >= tablefront - 5))                                          // front left hole
    {
        if (obj.name == "ball1") pcketBall1 = true;
        else if (obj.name == "ball2") pcketBall2 = true;
        // cout << "Hit front left pocket " << obj.x << " " << obj.y << " " << obj.z << endl;
    }
}

void reset()
{
    // Start drawing balls again
    pcketBall1 = false;
    pcketBall2 = false;
    cueHit = false;

    camLocation = originalLocation;
    tableObj.y = 0; // Reset Table Y Axis Since its sitting on the floor

    ballObj.z = OGballz;
    ballObj.x = OGballx;
    ballObj.zinc = OGballzinc;
    ballObj.xinc = OGballxinc;

    ball2Obj.z = OGball2z;
    ball2Obj.x = OGball2x;
    ball2Obj.zinc = OGball2zinc;
    ball2Obj.xinc = OGball2xinc;

    hit = true;
    hit2 = false;

    // cue
    cueObj.z = OGcueZ;
    cueObj.y = OGcueY;
    cueObj.x = OGcueX;
    cueObj.zinc = -0.001;

    // Give the objects identities
    ballObj.name = "ball1";
    ball2Obj.name = "ball2";
}
//=========================================================================


//=====================  Callback Functions  ===============================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // If ESC is pressed, close the window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // CAMERA CONTROLS
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) // If “H” is pressed: Reset Objects
        reset();
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camLocation.x -= xVal;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camLocation.x += xVal;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camLocation.y += yVal;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camLocation.y -= yVal;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camLocation.z -= zVal;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camLocation.z += zVal;

    View = glm::lookAt
    (
        glm::vec3(camLocation.x, camLocation.y, camLocation.z), // Camera is at (0,500,7000), in World Space
        glm::vec3(0, 0, 0),                                     // and looking at the origin
        glm::vec3(0, 1, 0)                                      // Head is up (set to 0,-1,0 to look upside-down)
    );
}

void windowSize_callback(GLFWwindow* window, int width, int height)
{
    sWidth = width;
    sHeight = height;

    glViewport(0, 0, width, height);
}

void clicked_callback(GLFWwindow* window, int button, int  action, int mode)
{
    double xpos, ypos;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) // Mouse Clicked
    {
        glfwGetCursorPos(window, &xpos, &ypos);
        // cout << "Old x[" << oldX << "]y[" << oldY << "] - >New x[" << xpos << "]y[" << ypos << "] 22222222" << endl;

        oldX = xpos;
        oldY = ypos;

        int mouseMove = 0;
        
        glfwGetCursorPos(window, &xpos, &ypos);

        mouseMove = oldY - ypos;

        // cout << "Old x[" << oldX << "]y[" << oldY << "] - >New x[" << xpos << "]y[" << ypos << "]" << endl;

        cueObj.z -= mouseMove / 2;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT &&  action == GLFW_RELEASE) // Mouse Release
    {
        glfwGetCursorPos(window, &xpos, &ypos);

        if (ypos > oldY) // mouse dragged foward so push forward cue
                cueObj.z += 2.0f;
        if ((ypos < oldY)) // mouse dragged backward so pull back cue
                cueObj.z -= 2.0f;
        else if (ypos == oldY) // Mouse just clicked
            cueObj.z -= 2.0f;

        // cout << "Released : Cursor Position at (" << xpos << " : " << ypos << ")" << endl;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) //Mouse Release
    {
        cueObj.z += 2.0f;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0) // Scrolled Foward so Move Camera in
    {
        for (int i = 0; i < 50; i++)
            camLocation.z -= 1.0f;
    }
    if (yoffset < 0) // Scrolled Backward so Move Camera out
    {
        for (int i = 0; i < 50; i++)
            camLocation.z += 1.0f;
    }

    View = glm::lookAt
    (
        glm::vec3(camLocation.x, camLocation.y, camLocation.z),     // Camera is at (0,500,7000), in World Space
        glm::vec3(0, 0, 0),                                         // and looking at the origin
        glm::vec3(0, 1, 0)                                          // Head is up (set to 0,-1,0 to look upside-down)
    );
}

//=========================================================================
