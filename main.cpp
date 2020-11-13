// ==============================================================================
//  PROGRAM:
//  Pool Table
// ==============================================================================

// ==============================================================================
//  Steps to create the MVP:
//    1.  Setup and compile our shaders as an object of the Shader class.
//    2.  Load the geometrical model objects.
//    3.  Create the projection matrix.
//    4.  Create the view matrix
//    5.  Create the model matrix – initially as an identity matrix and
//        then modify with scaling, translation, rotation, etc, if required.
//    6.  Pass the model, view and projection matrices to the shaders.
//    7.  Draw the object.
//
// ==============================================================================
/*============================================================================= =
Install the following in Package Manager :

Install-Package glew_dynamic
Install-Package glfw
Install-Package GLMathematics
Install-Package freeimage -version 3.16.0
Install-Package nupengl.core
Install-Package Soil
Install-Package Assimp -version 3.0.0

=================================================================================*/
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

//========= Prototype function for call back ===============================
//Keyboard callback for Functions like closing etc
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void windowSize_callback(GLFWwindow* window, int width, int height);

void clicked_callback(GLFWwindow* window, int button, int  action, int mode);

void clickDrag_callback(GLFWwindow* window, double xPos, double yPos);

void moveMouse_callback(GLFWwindow* window, double xPos, double yPos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//==========================================================================

#define ballCount 1

// Active window
GLFWwindow* window;

// Properties
GLuint sWidth = 1000, sHeight = 800;

//Initial Camera location
glm::vec3 originalLocation = glm::vec3(0.0f, 500.0f, 2000.0f);
glm::vec3 camLocation = originalLocation;

//Camera
Camera camera(camLocation);
glm::mat4 View, lightView;

// Deltatime - for camera movement
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;    // Time of last frame
GLfloat currentFrame;

// Light attributes
glm::vec3 lightPos(0,0,0);      // Light location
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);             // White light
glm::vec3 lightMode(2.0f);

//Variables increments for resetting the camera's coordinates
GLfloat xVal = 700.0f;
GLfloat yVal = 700.0f;
GLfloat zVal = 700.0f;

//OBJECTS
struct objects
{
    GLfloat x = 0.0;
    GLfloat y = 500.0;
    GLfloat z = 0.0;

    GLfloat xinc = 0.001;
    GLfloat yinc = 0.001;
    GLfloat zinc = 0.001;

    GLfloat angle = 0.0f;
    GLfloat inc = 0.001f;
} ballObj, cueObj, cuetipObj, tableObj, containerObj;

//Mouse
double oldX, oldY;
bool firstMouse = false;

// Define vector to hold the astroids
vector <Model> balls;
vector <objects> objs;

GLfloat groundLevel = 0.0f;
GLfloat tableTop = 40.0f;

//Boundaries on table
GLfloat tableback = -110.0f;
GLfloat tablefront = 110.0f; //Symmetrical boundary
GLfloat tableleft = -45.0f;
GLfloat tableright = tableleft * -1; //Symmetrical boundary

GLfloat radius = 20.0f;            // ball radius
GLfloat offset = 1.5f;

bool hit = true;

GLfloat OGcueZ = 50.0f;
GLfloat OGcueY = 40.0f;
GLfloat OGcueX = -4.0f; // was 0.0

GLfloat OGballz = 43.0f;
GLfloat OGballx = 0.0f;
GLfloat OGballzinc = -2.0;
GLfloat OGballxinc = -1.0;

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

    //Init Callbacks
    //Callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, windowSize_callback);

    glfwSetMouseButtonCallback(window, clicked_callback);

    //glfwSetCursorPosCallback(window, clickDrag_callback);//was acting up
    glfwSetCursorPosCallback(window, moveMouse_callback);

    glfwSetScrollCallback(window, scroll_callback);

    // Define the viewport dimensions
    glViewport(0, 0, sWidth, sHeight);

    // Setup OpenGL options
    glEnable(GL_DEPTH_TEST);

}


// The MAIN function, from here we start our application and run the loop
int main()
{
    init_Resources();

    // ==============================================
    // ====== Set up the stuff for our Objects =======
    // ==============================================
    tableObj.y = 0; //Reset Table Y Axis Since its sitting on the floor

    // =======================================================================
    // Step 1. Setup and compile our shaders as an object of the Shader class
    // =======================================================================
   /* Shader ballShader("objects/ballVertex.glsl", "objects/ballFragment.glsl");
    Shader tableShader("objects/pooltableVertex.glsl", "objects/pooltableFragment.glsl");
    Shader cueShader("objects/poolcueVertex.glsl", "objects/poolcueFragment.glsl");*/
    Shader lampShader("objects/lampVertex.glsl", "objects/lampFragment.glsl");
    Shader lightShader("objects/lightVertex.glsl", "objects/lightFragment.glsl");

    // =======================================================================
    // Step 2. Load the model objects
    // =======================================================================
    Model ball((GLchar*)"objects/ball.obj");        
    Model table((GLchar*)"objects/pooltable.obj"); //CREDIT: https://free3d.com/3d-model/pool-table-v1--600461.html
    Model cue((GLchar*)"objects/poolcue.obj"); //CREDIT: https://free3d.com/3d-model/pool-cue-v1--229730.html
    Model lamp((GLchar*)"objects/lamp.obj"); //CREDIT: https://free3d.com/3d-model/punct-pendant-lamp-86726.html

    // =======================================================================
    // Step 3. Set the projection matrix
    // =======================================================================
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)sWidth / (GLfloat)sHeight, 1.0f, 10000.0f);

    /*ballShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    tableShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(tableShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    cueShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(cueShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));*/

    lightShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Define how and where the data will be passed to the shaders
    GLint lightPos = glGetUniformLocation(lightShader.Program, "lightPos");
    GLint viewPos = glGetUniformLocation(lightShader.Program, "viewPos");
    GLint lightCol = glGetUniformLocation(lightShader.Program, "lightColor");
    GLint lightType = glGetUniformLocation(lightShader.Program, "lightType");

    srand(glfwGetTime()); // initialize random seed

    //Initializing location of objects
    ballObj.x = OGballx; // was 0
    ballObj.z = OGballz; // was 10 for middle

    ballObj.xinc = OGballxinc;
    ballObj.zinc = OGballxinc;

    //cue
    cueObj.z = OGcueZ;
    cueObj.y = OGcueY;
    cueObj.x = OGcueX;


    // Iterate this block while the window is open
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check and call events
        glfwPollEvents();

        // Clear buffers
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        View = glm::lookAt
        (
            glm::vec3(camLocation.x, camLocation.y, camLocation.z), // Camera is at (0,500,7000), in World Space
            glm::vec3(0, 0, 0),      // and looking at the origin
            glm::vec3(0, 1, 0)       // Head is up (set to 0,-1,0 to look upside-down)
        );

        lightView = glm::lookAt
        (
            glm::vec3(0.0f, 200.0f, 10.0f), // Camera is at (0,500,7000), in World Space
            glm::vec3(0, 0, 0),      // and looking at the origin
            glm::vec3(0, 1, 0)       // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Pass the data in the variables to to go to the vertex shader
        glUniform3f(lightPos, 0.0, 500.f, 0.0);
        glUniform3fv(viewPos, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
        glUniform3fv(lightCol, 1, glm::value_ptr(lightColor));
        glUniform3fv(lightType, 1, glm::value_ptr(lightMode));

        lightShader.Use();

       

        // 1. The View matrix first...
        //glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(lightView));

        // 2. Update the projection matrix previously defined
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // 3. Set up the scenery for world space
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // 1. The View matrix first...
        //tableShader.Use();
        //glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));

        //tableShader.Use();
        

        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));
        
        //==========================================================================
        // Draw the Table (Create the model)
        //==========================================================================
        //tableShader.Use();
        glm::mat4 tableModel = glm::mat4(1);

        tableModel = glm::scale(tableModel, glm::vec3(5.0f));
        tableModel = glm::translate(tableModel, glm::vec3(tableObj.x, tableObj.y, tableObj.z));

        // ...and 2. The Model matrix
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(tableModel));
        table.Draw(lightShader);

        //==========================================================================
        // Draw CUE
        //==========================================================================
        //cueShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));

        //cueShader.Use();
        glm::mat4 cueModel;
        glm::mat4 cuetip;

        cueModel = glm::mat4(1);
        cuetip = glm::mat4(1);

        cueModel = glm::scale(cueModel, glm::vec3(5.0f));
        cueModel = glm::rotate(cueModel, 0.1f, glm::vec3(0.0, 1.0, 0.0));
        cueModel = glm::translate(cueModel, glm::vec3(cueObj.x, cueObj.y, cueObj.z));


        //transform tip to be in same location as pool cue
        cuetip = glm::translate(cuetip, glm::vec3(cuetipObj.x, cuetipObj.y, cuetipObj.z + 20));

        //cout << "TIP " <<cuetipObj.x << " " << cuetipObj.y << " " << (cuetipObj.z + 200) << endl;
       // cout << "CUE " <<cueObj.x << " " << cueObj.y << " " << cueObj.z << endl;
        // ...and 2. The Model matrix
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(cueModel));
        cue.Draw(lightShader);

        //==========================================================================
        // Draw ball
        //==========================================================================
        //ballShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));

        if (cueObj.z - 2 <= ballObj.z)
        {
            if (hit)
            {
                ballObj.z = cueObj.z - 0.01;
                ballObj.z += ballObj.zinc;
                hit = false;
            }
        }
            
        if (ballObj.x >= tableright || ballObj.x <= tableleft)
        {
            ballObj.xinc *= -1;
            //Decay movement of ball
            if (ballObj.xinc < 0)
                ballObj.xinc = ballObj.xinc + (ballObj.xinc * 0.10);
            if (ballObj.xinc > 0)
                ballObj.xinc = ballObj.xinc - (ballObj.xinc * 0.10);
        }
        if (ballObj.z >= tablefront || ballObj.z <= tableback)
        {
            ballObj.zinc *= -1;

            //Decay movement of ball
            if (ballObj.zinc < 0)
                ballObj.zinc = ballObj.zinc + (ballObj.zinc * 0.50);
            if (ballObj.zinc > 0)
                ballObj.zinc = ballObj.zinc - (ballObj.zinc * 0.50);

            cout << ballObj.xinc << " " << ballObj.zinc;
        }

        //cout << "ball x " << ballObj.x << " y " << ballObj.y << " z " << ballObj.z << "|" << endl;
        //ballShader.Use();

        glm::mat4 ballModel;

        /*ballObj.x += ballObj.xinc;*/
        
        if (!hit)
        {
            ballObj.z += ballObj.zinc;
            ballObj.x += ballObj.xinc;
            hit = false;
        }

        ballModel = glm::mat4(1);        

        ballModel = glm::scale(ballModel, glm::vec3(5.0f));
        ballModel = glm::translate(ballModel, glm::vec3(ballObj.x, tableTop, ballObj.z));

        cout << "BALL " << ballObj.x << " " << ballObj.y << " " << ballObj.z << endl;
        // ...and 2. The Model matrix
        glUniformMatrix4fv(glGetUniformLocation(lightShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(ballModel));
        ball.Draw(lightShader);
       
        lightShader.Use();

        //==========================================================================
       // Draw the Lamp (Create the model)
       //==========================================================================
        lampShader.Use();

        // 1. Define the View matrix first...
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(View));


        // 2. Update the projection matrix - already defined on line 176
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


        // 3. Set up the scenery for world space
        glm::mat4 lampmodel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampmodel));


        // 4.  Now define the lamp model (Create and display the model)
        glm::mat4 lampModel = glm::mat4(1);
        lampModel = glm::scale(lampModel, glm::vec3(0.6f));
        lampModel = glm::translate(lampModel, glm::vec3(0.0f, 1200.0f, 0.0f));

        // ...and finally pass it to the world space for display.
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));

        // Now display the lamp model
        lamp.Draw(lampShader);

        // Swap the buffers
        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}


// -------------------------------------------------------------------------
// Process keyboard inputs
// -------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // If ESC is pressed, close the window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    

    //CAMERA CONTROLS
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) // If “H” is pressed, move the camera to the coordinates’ origin
    {
        camLocation = originalLocation;
        cueObj.z = OGcueZ;
        ballObj.z = OGballz;
        ballObj.x = OGballx;
        ballObj.zinc = OGballzinc;
        ballObj.xinc = OGballxinc;
        hit = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camLocation.x -= xVal;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camLocation.x += xVal;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camLocation.y += yVal;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camLocation.y -= yVal;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cout << "pressed w : " <<camLocation.z << " |";
        camLocation.z -= zVal;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camLocation.z += zVal;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cueObj.z += 400;

    //Reset Ball spin speed and direction
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        //inc = 0.001;

    //camera = glm::vec3(camLocation.x, camLocation.y, camLocation.z);

    View = glm::lookAt
    (
        glm::vec3(camLocation.x, camLocation.y, camLocation.z), // Camera is at (0,500,7000), in World Space
        glm::vec3(0, 0, 0),      // and looking at the origin
        glm::vec3(0, 1, 0)       // Head is up (set to 0,-1,0 to look upside-down)
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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) //Mouse Clicked
    {
        glfwGetCursorPos(window, &xpos, &ypos);
        cout << "Old x[" << oldX << "]y[" << oldY << "] - >New x[" << xpos << "]y[" << ypos << "] 22222222" << endl;
        oldX = xpos;
        oldY = ypos;

        int mouseMove = 0;
        
        glfwGetCursorPos(window, &xpos, &ypos);

        mouseMove = oldY - ypos;

        //cout << "Old x[" << oldX << "]y[" << oldY << "] - >New x[" << xpos << "]y[" << ypos << "]" << endl;

        cueObj.z -= mouseMove / 2;
    }
    if (action == GLFW_RELEASE) //Mouse Release
    {
        glfwGetCursorPos(window, &xpos, &ypos);

        if (ypos > oldY)
        {
            //for (int i = 0; i < 20; i++)
                cueObj.z += 2.0f;
        }
        if ((ypos < oldY))
        {
            //for (int i = 0; i < 20; i++)
                cueObj.z -= 2.0f;
        }

        cout << "Released : Cursor Position at (" << xpos << " : " << ypos << ")" << endl;
    }
}

void clickDrag_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)) //Mouse Released - Perform Rotary changes to globe
        {
            glfwGetCursorPos(window, &xPos, &yPos);

            if (yPos < oldY)
            {
                cueObj.z += 5.0f;
            }
            if ((yPos > oldY))
            {

            }

            cout << "Released : Cursor Position at (" << xPos << " : " << yPos << ")" << endl;

            firstMouse = false; //Let program know mouse is released

            glfwSetCursorPosCallback(window, moveMouse_callback); //Reset to original callback for mouse movement

            return;
        }
    }
}

void moveMouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    double xpos, ypos;

    //mouse is not occupied with spinning earth function
   /* if (!firstMouse)
    {

    }
    else
    {*/
        //The mouse is being clicked which means click and drag Should be activated
        glfwSetCursorPosCallback(window, clickDrag_callback);
        return;
    //}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //cout << "yoffset " << yoffset << " xoffset " << xoffset << " | ";

    if (yoffset > 0) //Scrolled Foward so Move Camera in
    {
        for (int i = 0; i < 50; i++)
            camLocation.z -= 1.0f;
    }
    if (yoffset < 0) //Scrolled Backward so Move Camera out
    {
        for (int i = 0; i < 50; i++)
            camLocation.z += 1.0f;
    }

    View = glm::lookAt
    (
        glm::vec3(camLocation.x, camLocation.y, camLocation.z), // Camera is at (0,500,7000), in World Space
        glm::vec3(0, 0, 0),      // and looking at the origin
        glm::vec3(0, 1, 0)       // Head is up (set to 0,-1,0 to look upside-down)
    );
    //camera = glm::vec3(camLocation.x, camLocation.y, camLocation.z);
}
