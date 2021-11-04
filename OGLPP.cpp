//headers
#include <windows.h>               //standard windows header
#include <windowsx.h>
#include <stdio.h>                 //C header 
#include <vector>
#include <gl/glew.h>               //OpenGL extension wrangler (must be included before gl.h)
#include <gl/gl.h>                 //OpenGL header
#include "RESOURCES.h"             //Resources header
#include "include/vmath.h"         //Maths header
#include "include/Model.h"         //Assimp model loader
#include "include/Camera.h"        //Camera header

//import libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "assimp-vc142-mt.lib")

//symbolic constants
#define WIN_WIDTH  800             //initial width of window  
#define WIN_HEIGHT 600             //initial height of window

#define VK_F       0x46            //virtual key code of F key
#define VK_f       0x60            //virtual key code of f key

//namespaces
using namespace vmath;

//type declarations
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD,
    AMC_ATTRIBUTE_TANGENT
};

//callback procedure declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

//global variables
HWND   ghwnd  = NULL;              //handle to a window
HDC    ghdc   = NULL;              //handle to a device context
HGLRC  ghrc   = NULL;              //handle to a rendering context

DWORD dwStyle = NULL;              //window style
WINDOWPLACEMENT wpPrev;            //structure for holding previous window position

bool gbActiveWindow = false;       //flag indicating whether window is active or not
bool gbFullscreen = false;         //flag indicating whether window is fullscreen or not

FILE*  gpFile = NULL;              //log file

GLuint vertexShaderObject;         
GLuint fragmentShaderObject;       
GLuint shaderProgramObject;        

GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;

GLuint viewPosUniform;
GLuint lightPositionUniform;
GLuint lightAmbientUniform;
GLuint lightDiffuseUniform;
GLuint lightSpecularUniform;
GLuint diffuseTextureUniform;
GLuint specularTextureUniform;
GLuint normalTextureUniform;

GLuint materialAmbientUniform;
GLuint materialDiffuseUniform;
GLuint materialSpecularUniform;
GLuint materialShininessUniform;

GLuint vao_square;
GLuint vbo_square_position;
GLuint vbo_square_normal;
GLuint vbo_square_texcoord;
GLuint vbo_square_tangent;

GLuint floor_diffuse_texture;
GLuint floor_specular_texture;
GLuint floor_normal_texture;

//fire 
GLuint gShaderProgramObject;
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;

GLuint vao_rectangle;

GLuint vbo_position_rectangle;
GLuint vbo_texture_rectangle;

GLuint timeUniform;
GLuint PointSizeUniform;
GLuint sTextureUniform;

GLuint gModelMatrixUniform;
GLuint gViewMatrixUniform;
GLuint gProjectionMatrixUniform;

GLuint fadeinFactorUniform;
GLuint fadeoutFactorUniform;

GLuint vao;
GLuint vbo_lifeTime;
GLuint vbo_xPos;
GLuint vbo_YSpeed;
GLuint vbo_color;

GLuint textureSmily;
float numParticles;

mat4 perspectiveProjectionMatrix;  

Model House;
Model Panti;
Model Tree;

Camera camera(vec3(0.0f, 0.0f, 0.0f));
GLfloat initial_val_Zoom = 0.0f;

//windows entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //function declarations
    void Initialize(void);                                      //initialize OpenGL state machine
    void Display(void);                                         //render scene

    //variable declarations
    WNDCLASSEX wndclass;                                        //structure holding window class attributes
    MSG msg;                                                    //structure holding message attributes
    HWND hwnd;                                                  //handle to a window
    TCHAR szAppName[] = TEXT("OpenGL : Diwali");                //name of window class

    int cxScreen, cyScreen;                                     //screen width and height for centering window
    int init_x, init_y;                                         //top-left coordinates of centered window
    bool bDone = false;                                         //flag indicating whether or not to exit from game loop

    //code
    //create/open  'log.txt' file
    if(fopen_s(&gpFile, "log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Failed to open log.txt file"), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "----- Program Started Successfully -----\n\n");
    }
    
    //initialization of WNDCLASSEX
    wndclass.cbSize         = sizeof(WNDCLASSEX);                            //size of structure
    wndclass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;            //window style
    wndclass.lpfnWndProc    = WndProc;                                       //address of callback procedure
    wndclass.cbClsExtra     = 0;                                             //extra class bytes
    wndclass.cbWndExtra     = 0;                                             //extra window bytes
    wndclass.hInstance      = hInstance;                                     //handle to a program
    wndclass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to an icon
    wndclass.hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);        //handle to a cursor
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);           //handle to a background brush
    wndclass.lpszClassName  = szAppName;                                     //name of a custom class
    wndclass.lpszMenuName   = NULL;                                          //name of a custom menu
    wndclass.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));  //handle to a small icon

    //register above class
    RegisterClassEx(&wndclass);

    //get screen width and height
    cxScreen = GetSystemMetrics(SM_CXSCREEN);
    cyScreen = GetSystemMetrics(SM_CYSCREEN);

    //calculate top-left coordinates for a centered window
    init_x = (cxScreen / 2) - (WIN_WIDTH / 2);
    init_y = (cyScreen / 2) - (WIN_HEIGHT / 2);

    //create window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,                //extended window style          
            szAppName,                                    //class name
            szAppName,                                    //window caption
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |       //window style
            WS_CLIPSIBLINGS | WS_VISIBLE,   
            init_x,                                       //X-coordinate of top left corner of window 
            init_y,                                       //Y-coordinate of top left corner of window
            WIN_WIDTH,                                    //initial window width                 
            WIN_HEIGHT,                                   //initial window height
            (HWND)NULL,                                   //handle to a parent window  : NULL desktop
            (HMENU)NULL,                                  //handle to a menu : NULL no menu
            hInstance,                                    //handle to a program instance
            (LPVOID)NULL);                                //data to be sent to window callback : NULL no data to send      

    //store handle to a window in global handle
    ghwnd = hwnd;                                         

    //initialize OpenGL rendering context
    Initialize();

    ShowWindow(hwnd, iCmdShow);                 //set specified window's show state
    SetForegroundWindow(hwnd);                  //brings the thread that created the specified window to foreground
    SetFocus(hwnd);                             //set the keyboard focus to specified window 

    //game loop
    while(bDone == false)
    {   
        //1 : pointer to structure for window message
        //2 : handle to window : NULL do not process child window's messages 
        //3 : message filter min range : 0 no range filtering
        //4 : message filter max range : 0 no range filtering
        //5 : remove message from queue after processing from PeekMessage
        if(PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)           //if current message is WM_QUIT then exit from game loop
            {
                bDone = true;
            }
            else
            {
                TranslateMessage(&msg);          //translate virtual-key message into character message
                DispatchMessage(&msg);           //dispatch message  to window procedure
            }
        }
        else
        {
            if(gbActiveWindow == true)           //if window has keyboard focus 
            {
                Display();                       //render the scene
            }
        }
    }

    return ((int)msg.wParam);                    //exit code given by PostQuitMessage 
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //function declarations
    void ToggleFullscreen(void);                 //toggle window between fullscreen and previous position 
    void Resize(int, int);                       //handle window resize event
    void UnInitialize(void);                     //release resources  

    POINT pt;
	static int old_x_pos;
	static int old_y_pos;
	static int new_x_pos;
	static int new_y_pos;
	static int x_offset;
	static int y_offset;

    //code
    switch(iMsg)
    {
        case WM_CREATE:
            GetCursorPos(&pt);
            old_x_pos = pt.x;
            old_y_pos = pt.y;
            break;

        case WM_SETFOCUS:                        //event : window has keyboard focus
            gbActiveWindow = true;
            break;
        
        case WM_KILLFOCUS:                       //event : window dosen't have keyboard focus
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:                      //event : window background must be erased 
            return (0);                          //dont let DefWindowProc handle this event
        
        case WM_SIZE:                            //event : window is resized
            Resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:                         //event : a key has been pressed
            switch(wParam)
            {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case VK_F:
                case VK_f:
                    ToggleFullscreen();
                    break;
                
                default:
                    break;
            }
            break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'A':
                case 'a':
                    camera.ProcessKeyboard(LEFT);
                    break;

                case 'W':
                case 'w':
                    camera.ProcessKeyboard(FORWARD);
                    break;

                case 'D':
                case 'd':
                    camera.ProcessKeyboard(RIGHT);
                    break;

                case 'S':
                case 's':
                    camera.ProcessKeyboard(BACKWARD);
                    break;

                default:
                    break;
            }
            break;

        case WM_MOUSEMOVE:
            new_x_pos	= GET_X_LPARAM(lParam);
            new_y_pos	= GET_Y_LPARAM(lParam);

            x_offset	= new_x_pos - old_x_pos;
            y_offset	= new_y_pos - old_y_pos;
            
            old_x_pos = new_x_pos;
            old_y_pos = new_y_pos;
    
            camera.ProcessMouseMovement(x_offset, y_offset);
            break;

        case WM_CLOSE:                           //event : window is closed from sysmenu or close button
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnInitialize();
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }

    //call default window procedure for unhandled messages
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
    //variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };            //structure holding monitor information

    //code
    if(gbFullscreen == false)                            //if screen is not in fulscreen mode 
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);       //get window style
        if(dwStyle & WS_OVERLAPPEDWINDOW)                //if current window style has WS_OVERLAPPEDWINDOW
        {
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                // if wpPrev is successfully filled with current window placement
                // and mi is successfully filled with primary monitor info then
                // 1 -> Remove WS_OVERLAPPEDWINDOW style
                // 2 -> Set window position by aligning left-top corner of window 
                //     to left-top corner of monitor and setting width and height 
                //     to monitor's width and height (effectively making window 
                //     fullscreen)
                // SWP_NOZORDER : Don't change Z-order
                // SWP_FRAMECHANGED: Forces recalculation of New Client area (WM_NCCALCSIZE)
                SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
                SetWindowPos(ghwnd,                                     //     top 
                    HWND_TOP,                                           //left +--------------+ right
                    mi.rcMonitor.left,                                  //     |              |
                    mi.rcMonitor.top,                                   //     |              |
                    mi.rcMonitor.right - mi.rcMonitor.left,             //     |              |
                    mi.rcMonitor.bottom - mi.rcMonitor.top,             //     |              |
                    SWP_NOZORDER | SWP_FRAMECHANGED);                   //     +--------------+
            }                                                           //     bottom
        }

        ShowCursor(false);                                 //hide the cursor
        gbFullscreen = true;                          
    }
    else                                                   //if screen is in fullscreen mode
    {
        // Toggle the window to previously saved dimension
        // 1 -> Add WS_OVERLAPPEDWINDOW to window style 
        // 2 -> Set window placement to stored previous placement
        // 3 -> Force the effects of SetWindowPlacement by call to 
        //      SetWindowPos with
        // SWP_NOMOVE : Don't change left top position of window 
        //              i.e ignore third and forth parameters
        // SWP_NOSIZE : Don't change dimensions of window
        //              i.e ignore fifth and sixth parameters
        // SWP_NOZORDER : Don't change Z-order of the window and
        //              its child windows
        // SWP_NOOWNERZORDER : Don't change Z-order of owner of the 
        //              window (reffered by ghwnd)
        // SWP_FRAMECHANGED : Forces recalculation of New Client area (WM_NCCALCSIZE)
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0,
            0,
            0, 
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        
        ShowCursor(true);            //show cursor
        gbFullscreen = false;
    }
}

void Initialize(void)
{
    //function declarations
    void Resize(int, int);          //warm-up call
    void UnInitialize(void);        //release resources
    void loadGLTexture(GLuint *texture, const char *filename);

    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;      //structure describing the pixel format
    int iPixelFormatIndex;          //index of the pixel format structure in HDC

    //code
    //zero out the memory
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); 

    //initialization of PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);                                //size of structure
    pfd.nVersion    = 1;                                                            //version information
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW| PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;    //pixel format properties
    pfd.iPixelType  = PFD_TYPE_RGBA;                                                //type of pixel format to chosen
    pfd.cColorBits  = 32;                                                           //color depth in bits (32 = True Color)
    pfd.cRedBits    = 8;                                                            //red color bits
    pfd.cGreenBits  = 8;                                                            //green color bits
    pfd.cBlueBits   = 8;                                                            //blue color bits
    pfd.cAlphaBits  = 8;                                                            //alpha bits
    pfd.cDepthBits  = 32;                                                           //depth bits

    //obtain a device context
    ghdc = GetDC(ghwnd);                    

    //choose required pixel format from device context
    //which matches pfd structure and get the index of 
    //that pixel format (1 based index)
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        fprintf(gpFile, "ChoosePixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set the current pixel format of the device context (ghdc) to
    //pixel format specified by index
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed.\n");
        DestroyWindow(ghwnd);
    }

    //create rendering context 
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed.\n");
        DestroyWindow(ghwnd);
    }

    //set rendering context as current context
    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed.\n");
        DestroyWindow(ghwnd);
    }

    //initialize glew (enable extensions)
    GLenum glew_error = glewInit();
    if(glew_error != GLEW_OK)
    {
        fprintf(gpFile, "glewInit() failed.\n");
        DestroyWindow(ghwnd);
    }

    //opengl related log
    fprintf(gpFile, "OpenGL Information\n");
    fprintf(gpFile, "OpenGL Vendor     : %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer   : %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version    : %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "GLSL Version      : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    //opengl enabled extensions
    GLint numExt;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    fprintf(gpFile, "OpenGL Extensions : \n");
    for(int i = 0; i < numExt; i++)
    {
        fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
    }

    //setup render scene

    //Normal Mapped Model Loading 
    fprintf(gpFile, "\n-> Normal Mapped Model Loading\n");

    //vertex shader
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    //shader source code
    const GLchar* vertexShaderSourceCode = 
        "#version 450 core"                                                                 \
        "\n"                                                                                \

        "in vec3 vPosition;"                                                                \
        "in vec3 vNormal;"                                                                  \
        "in vec2 vTexCoord;"                                                                \
        "in vec3 vTangent;"                                                                 \

        "out vec3 out_fragPos;"                                                             \
        "out vec2 out_texCoord;"                                                            \
        "out vec3 out_tangentLightPos;"                                                     \
        "out vec3 out_tangentViewPos;"                                                      \
        "out vec3 out_tangentFragPos;"                                                      \

        "uniform mat4 u_modelMatrix;"                                                       \
        "uniform mat4 u_viewMatrix;"                                                        \
        "uniform mat4 u_projectionMatrix;"                                                  \
        "uniform vec3 u_lightPos;"                                                          \
        "uniform vec3 u_viewPos;"                                                           \

        "void main(void)"                                                                   \
        "{"                                                                                 \
        "   out_fragPos = vec3(u_modelMatrix * vec4(vPosition, 1.0f));"                     \
        "   out_texCoord = vTexCoord;"                                                      \

        "   mat3 normalMatrix = transpose(inverse(mat3(u_modelMatrix)));"                   \
        "   vec3 T = normalize(normalMatrix * vTangent);"                                   \
        "   vec3 N = normalize(normalMatrix * vNormal);"                                    \
        "   T = normalize(T - dot(T, N) * N);"                                              \
        "   vec3 B = cross(N, T);"                                                          \

        "   mat3 TBN = transpose(mat3(T, B, N));"                                           \
        "   out_tangentLightPos = TBN * u_lightPos;"                                        \
        "   out_tangentViewPos = TBN * u_viewPos;"                                          \
        "   out_tangentFragPos = TBN * out_fragPos;"                                        \

        "   gl_Position = u_projectionMatrix * u_viewMatrix * vec4(out_fragPos, 1.0f);"     \
        "}";

    //provide source code to shader object
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //compile shader 
    glCompileShader(vertexShaderObject);

    //shader compilation error checking
    GLint infoLogLength = 0;
    GLint shaderCompiledStatus = 0;
    GLchar* szInfoLog = NULL;

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> vertex shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    } 

    fprintf(gpFile, "-> vertex shader compiled successfully\n");

    //fragment shader
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    //shader source code
    const GLchar* fragmentShaderSourceCode = 
        "#version 450 core"                                                                                         \
        "\n"                                                                                                        \
        
        "layout(binding = 0)uniform sampler2D diffuse_texture;"                                                     \
        "layout(binding = 1)uniform sampler2D specular_texture;"                                                    \
        "layout(binding = 2)uniform sampler2D normal_texture;"                                                      \

        "uniform vec3 u_lightAmbient;"                                                                              \
        "uniform vec3 u_lightDiffuse;"                                                                              \
        "uniform vec3 u_lightSpecular;"                                                                             \

        "uniform vec3 u_matAmbient;"                                                                                \
        "uniform vec3 u_matDiffuse;"                                                                                \
        "uniform vec3 u_matSpecular;"                                                                               \
        "uniform float u_matShininess;"                                                                             \

        "in vec3 out_fragPos;"                                                                                      \
        "in vec2 out_texCoord;"                                                                                     \
        "in vec3 out_tangentLightPos;"                                                                              \
        "in vec3 out_tangentViewPos;"                                                                               \
        "in vec3 out_tangentFragPos;"                                                                               \

        "out vec4 FragColor;"                                                                                       \

        "uniform vec3 u_viewPos;"                                                                                   \
        "uniform vec3 u_lightPos;"

        "void main(void)"                                                                                           \
        "{"                                                                                                         \
            //obtain normal from normal map in range [0, -1]
        "   vec3 normal = texture(normal_texture, out_texCoord).rgb;"                                               \

            //transform normal vector to range [-1, 1]
        "   normal = normalize(normal * 2.0f - 1.0f);"                                                              \

            //ambient
        "   vec3 ambient = u_lightAmbient * u_matAmbient * texture(diffuse_texture, out_texCoord).rgb;"             \
            
            //diffuse
        "   vec3 light_direction = normalize(out_tangentLightPos - out_tangentFragPos);"                            \
        "   float diff = max(dot(light_direction, normal), 0.0f);"                                                  \
        "   vec3 diffuse = u_lightDiffuse * u_matDiffuse * diff * texture(diffuse_texture, out_texCoord).rgb;"      \

            //specular
        "   vec3 view_direction = normalize(out_tangentViewPos - out_tangentFragPos);"                              \
        "   vec3 reflect_direction = reflect(-light_direction, normal);"                                            \
        "   vec3 halfway_direction = normalize(light_direction + view_direction);"                                  \
        "   float spec = pow(max(dot(normal, halfway_direction), 0.0f), u_matShininess);"                           \
        "   vec3 specular = u_lightSpecular * u_matSpecular * spec * texture(specular_texture, out_texCoord).rgb;"  \

        "   float alpha = texture(diffuse_texture, out_texCoord).a;"                                                \
        "   vec3 ads_light = ambient + diffuse + specular;"                                                         \
        "   FragColor = vec4(ads_light, alpha);"                                                                    \
        "}";                 

    //provide source code to shader object 
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

    //compile shader
    glCompileShader(fragmentShaderObject);

    //shader compilation error checking
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &shaderCompiledStatus);
    if(shaderCompiledStatus == GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> fragment shader compilation log : %s\n", szInfoLog);
                free(szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> fragment shader compiled successfully\n");

    //shader program
    shaderProgramObject = glCreateProgram();

    //attach shaders to shader program
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    //binding of shader program object with vertex shader position attribute
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "vPositon");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "vNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "vTexCoord");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TANGENT, "vTangent");

    //link shader program 
    glLinkProgram(shaderProgramObject);

    //shader linking error checking
    GLint shaderProgramLinkStatus = 0;
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &shaderProgramLinkStatus);
    if(shaderProgramLinkStatus == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if(infoLogLength > 0)
        {
            szInfoLog = (GLchar*)malloc(sizeof(GLchar) * infoLogLength);
            if(szInfoLog != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, szInfoLog);
                fprintf(gpFile, "-> shader program link log : %s\n", szInfoLog);
                DestroyWindow(ghwnd);
            }
        }
    }

    fprintf(gpFile, "-> shader program linked successfully\n");

    //get uniform locations
    modelMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_projectionMatrix");

    viewPosUniform = glGetUniformLocation(shaderProgramObject, "u_viewPos");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "u_lightPos");
    lightAmbientUniform = glGetUniformLocation(shaderProgramObject, "u_lightAmbient");
    lightDiffuseUniform = glGetUniformLocation(shaderProgramObject, "u_lightDiffuse");
    lightSpecularUniform = glGetUniformLocation(shaderProgramObject, "u_lightSpecular");
    diffuseTextureUniform = glGetUniformLocation(shaderProgramObject, "diffuse_texture");
    specularTextureUniform = glGetUniformLocation(shaderProgramObject, "specular_texture");
    normalTextureUniform = glGetUniformLocation(shaderProgramObject, "normal_texture");

    materialAmbientUniform = glGetUniformLocation(shaderProgramObject, "u_matAmbient");
    materialDiffuseUniform = glGetUniformLocation(shaderProgramObject, "u_matDiffuse");
    materialSpecularUniform = glGetUniformLocation(shaderProgramObject, "u_matSpecular");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "u_matShininess");

    //setup square data
    vec3 pos1(1.0f, 0.0f, -1.0f);
    vec3 pos2(-1.0f, 0.0f, -1.0f);
    vec3 pos3(-1.0f, 0.0f, 1.0f);
    vec3 pos4(1.0f, 0.0f, 1.0f);

    vec2 uv1(10.0f, 10.0f);
    vec2 uv2(0.0f, 10.0f);
    vec2 uv3(0.0f, 0.0f);
    vec2 uv4(10.0f, 0.0f);

    vec3 nm1(0.0f, 1.0f, 0.0f);

    vec3 tangent1;
    vec3 tangent2;

    //triangle 1 (1, 2, 3)
    vec3 edge1 = pos2 - pos1;
    vec3 edge2 = pos3 - pos1;
    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

    tangent1[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
    tangent1[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
    tangent1[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

    //triangle 2(1, 3, 4)
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

    tangent2[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
    tangent2[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
    tangent2[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);

    //vertex data
    const GLfloat squareVertices[] = 
    {
        pos1[0], pos1[1], pos1[2],
        pos2[0], pos2[1], pos2[2],
        pos3[0], pos3[1], pos3[2],

        pos1[0], pos1[1], pos1[2],
        pos3[0], pos3[1], pos3[2],
        pos4[0], pos4[1], pos4[2]
    };

    //normal data
    static const GLfloat squareNormals[] = 
    {
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],

        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
        nm1[0], nm1[1], nm1[2],
    };

    //texcoord data
    const GLfloat squareTexCoords[] = 
    {
        uv1[0], uv1[1],
        uv2[0], uv2[1],
        uv3[0], uv3[1],

        uv1[0], uv1[1],
        uv3[0], uv3[1],
        uv4[0], uv4[1]
    };

    const GLfloat squareTangents[] =
    {
        tangent1[0], tangent1[1], tangent1[2],
        tangent1[0], tangent1[1], tangent1[2],
        tangent1[0], tangent1[1], tangent1[2],

        tangent2[0], tangent2[1], tangent2[2],
        tangent2[0], tangent2[1], tangent2[2],
        tangent2[0], tangent2[1], tangent2[2],
    };

    //setup vao and vbo 
    glGenVertexArrays(1, &vao_square);
    glBindVertexArray(vao_square);
        glGenBuffers(1, &vbo_square_position);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_normal);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareNormals), squareNormals, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_texcoord);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareTexCoords), squareTexCoords, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_square_tangent);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_square_tangent);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareTangents), squareTangents, GL_STATIC_DRAW);

        glVertexAttribPointer(AMC_ATTRIBUTE_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_TANGENT);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	// variables for error checking 
	GLint iShaderCompileStatus = 0;
	GLint iInfoLogLength = 0;
	GLint iProgramLinkStatus = 0;

#pragma region VERETX_SHADER
	fprintf(gpFile, "IN : initialize() FOR VERTEX SHADER \n");
	fflush(gpFile);
	// vertex shader part start

	// define vertex shader object
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	fprintf(gpFile, "IN : initialize() done upto glCreateShader() \n");
	fflush(gpFile);

	// write vertex shader code
	const GLchar * fire_vertexShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"in float aLifetime;" \
		"in float aXPos;" \
		"in float aYSpeed;" \
		"in vec2 aColor;" \

		"uniform float uTime;" \
		"uniform float uPointSize;" \

		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \

		"out float vLifetime;" \
		"out vec2 color;" \


		"void main(void)" \
		"{" \
			"vLifetime		= mod(uTime, aLifetime);" \
			"float ti		= 1.0 - vLifetime/aLifetime;" \

			"mat4 mv_matrix = u_view_matrix * u_model_matrix;" \

			"gl_Position	= u_projection_matrix * mv_matrix * vec4(aXPos * ti, aYSpeed * vLifetime - 1.0, 1.0, 1.0);" \
			"vLifetime		= 4.0 * ti * (1.0 - ti);" \
			"color			= aColor;" \
		"}";

	// specify above source, to vertex shader object
	glShaderSource(gVertexShaderObject, 1, (GLchar **)&fire_vertexShaderSourceCode, NULL);

	fprintf(gpFile, "IN : initialize() done upto glShaderSource() \n");
	fflush(gpFile);

	// compile the vertex shader
	glCompileShader(gVertexShaderObject);

	fprintf(gpFile, "IN : initialize() done upto glCompileShader() \n");
	fflush(gpFile);

	// error checking for vertex shader
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

	fprintf(gpFile, "IN : initialize() done upto 1st glGetShaderiv() \n");
	fflush(gpFile);

	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nCompile time log for Vertex Shader : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				DestroyWindow(ghwnd);
			}
		}
	}
	// vertex shader part end
#pragma endregion

#pragma region FRAGMENT_SHADER
	// fragment shader part start

	fprintf(gpFile, "IN : initialize() FOR FRAGMENT SHADER \n");
	fflush(gpFile);

	// define fragment shader object
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	fprintf(gpFile, "IN : initialize() done upto glCreateShader() \n");
	fflush(gpFile);

	// write fragment shader code
	const GLchar * fire_fragmentShaderSourceCode =
		"#version 460 core" \
		"\n" \
		"uniform sampler2D sTexture;" \

		"in float vLifetime;" \
		"in vec2 color;" \

		"out vec4 FragColor;" \

		"uniform float fadeinFactor;" \
		"uniform float fadeoutFactor;" \

		"void main(void)" \
		"{" \
			/*"gl_PointSize = 5;" \*/
			"vec4 ColorTemp;" \
			"vec4 texColor	= texture2D(sTexture, gl_PointCoord);" \
			"ColorTemp		= vec4(color, 0., 1.) * texColor ;" \
			"ColorTemp.a	= vLifetime + 0.75;" \
			"if(ColorTemp.r < 0.1 && ColorTemp.g < 0.1 && ColorTemp.b < 0.1)" \
			"discard;" \
			"FragColor		= ColorTemp * fadeinFactor * fadeoutFactor;" \
		"}";

	// specify above source, to fragment shader object
	glShaderSource(gFragmentShaderObject, 1, (GLchar **)&fire_fragmentShaderSourceCode, NULL);

	fprintf(gpFile, "IN : initialize() done upto glShaderSource() \n");
	fflush(gpFile);

	// compile the fragment shader
	glCompileShader(gFragmentShaderObject);

	fprintf(gpFile, "IN : initialize() done upto glCompileShader() \n");
	fflush(gpFile);

	// error checking for fragment shader
	iShaderCompileStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);

	fprintf(gpFile, "IN : initialize() done upto 1st glGetShaderiv() \n");
	fflush(gpFile);

	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nCompile time log for Fragment Shader : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				DestroyWindow(ghwnd);
			}
		}
	}
	// fragment shader part end
#pragma endregion

#pragma region SHADER_PROGRAM

	fprintf(gpFile, "IN : initialize() FOR SHADER PROGRAM \n");
	fflush(gpFile);

	// create shader program object
	gShaderProgramObject = glCreateProgram();

	fprintf(gpFile, "IN : initialize() done upto glCreateProgram() \n");
	fflush(gpFile);

	// attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	fprintf(gpFile, "IN : initialize() done upto glAttachShader() for vertex shader \n");
	fflush(gpFile);

	// attach fragment shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	fprintf(gpFile, "IN : initialize() done upto glAttachShader() for fragment shader \n");
	fflush(gpFile);


	// prelinking, binding to vertex attribute
	glBindAttribLocation(gShaderProgramObject, 0, "aLifetime");
	glBindAttribLocation(gShaderProgramObject, 1, "aXPos");
	glBindAttribLocation(gShaderProgramObject, 2, "aYSpeed");
	glBindAttribLocation(gShaderProgramObject, 3, "aColor");

	fprintf(gpFile, "IN : initialize() done upto glBindAttribLocation() \n");
	fflush(gpFile);

	// link the shader program
	glLinkProgram(gShaderProgramObject);

	fprintf(gpFile, "IN : initialize() done upto glLinkProgram() \n");
	fflush(gpFile);

	// error checking for shader program
	iProgramLinkStatus = 0;
	iInfoLogLength = 0;
	szInfoLog = NULL;

	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);

	fprintf(gpFile, "IN : initialize() done upto 1st glGetProgramiv() \n");
	fflush(gpFile);

	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "\n--------------------------------------------------- \nLink time log for Shader Program : \n %s", szInfoLog);
				fflush(gpFile);
				free(szInfoLog);
				DestroyWindow(ghwnd);
			}
		}
	}



	// postlinking, getting uniform location

	timeUniform					= glGetUniformLocation(gShaderProgramObject, "uTime");
	PointSizeUniform			= glGetUniformLocation(gShaderProgramObject, "uPointSize");
	sTextureUniform				= glGetUniformLocation(gShaderProgramObject, "sTexture");

	gModelMatrixUniform			= glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform			= glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform	= glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	fadeinFactorUniform			= glGetUniformLocation(gShaderProgramObject, "fadeinFactor");
	fadeoutFactorUniform		= glGetUniformLocation(gShaderProgramObject, "fadeoutFactor");
#pragma endregion


#pragma region RECTANGLE

	const GLfloat rectangleVertices[] =
	{
		1.0f, 1.0f,0.0f,
		-1.0f, 1.0f,0.0f,
		-1.0f, -1.0f,0.0f,
		1.0f, -1.0f,0.0f
	};

	const GLfloat rectangleTexcoords[]=
	{
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	};

	// create vao
	glGenVertexArrays(1, &vao_rectangle);

	glBindVertexArray(vao_rectangle);

	// create vbo for position
	glGenBuffers(1, &vbo_position_rectangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_rectangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// create vbo for texture
	glGenBuffers(1, &vbo_texture_rectangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_rectangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleTexcoords), rectangleTexcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(0);

#pragma endregion

	numParticles = 150;

	std::vector<float> lifetimes, xPos, ySpeed, colors;


	for (int i = 0; i < numParticles; i++)
	{

		lifetimes.push_back(2.0 * ((GLfloat)(rand() % 1000) / 1000.0f) + 1);
		xPos.push_back(0.75 * (((GLfloat)(rand() % 1000) / 1000.0f) - 0.75));
		ySpeed.push_back(0.5 * ((GLfloat)(rand() % 1000) / 1000.0f));
		colors.push_back((GLfloat)(rand() % 1000) / 1000.0f);
		colors.push_back(0.2 * ((GLfloat)(rand() % 1000) / 1000.0f));

	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//lifetimes
	glGenBuffers(1, &vbo_lifeTime);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_lifeTime);
	glBufferData(GL_ARRAY_BUFFER, lifetimes.size() * sizeof(float), &lifetimes[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//xPos
	glGenBuffers(1, &vbo_xPos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_xPos);
	glBufferData(GL_ARRAY_BUFFER, xPos.size() * sizeof(float), &xPos[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//aYSpeed
	glGenBuffers(1, &vbo_YSpeed);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_YSpeed);
	glBufferData(GL_ARRAY_BUFFER, ySpeed.size() * sizeof(float), &ySpeed[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	//vbo_color
	glGenBuffers(1, &vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

    //smooth shading  
    glShadeModel(GL_SMOOTH);                  

    //depth
    glClearDepth(1.0f);                                     
    glEnable(GL_DEPTH_TEST);                                
    glDepthFunc(GL_LEQUAL);

    //quality of color and texture coordinate interpolation
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    

    //set clearing color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  

    //set perspective projection matrix to identity
    perspectiveProjectionMatrix = mat4::identity();
    House.LoadModel("models/House.obj");
    Panti.LoadModel("models/panati.obj");
    Tree.LoadModel("models/tree_01.obj");

    loadGLTexture(&floor_diffuse_texture, "textures/albedo.png");
    loadGLTexture(&floor_specular_texture, "textures/specular.png");
    loadGLTexture(&floor_normal_texture, "textures/normal.png");

    loadGLTexture(&textureSmily, "textures/Smiley.bmp");

    //warm-up  call
    Resize(WIN_WIDTH, WIN_HEIGHT);
}

void loadGLTexture(GLuint *texture, const char *filename)
{
    //variable declarations
    unsigned char *pixel_data = NULL;
    int width, height, nrComponents;
    GLenum format;

    //code
    pixel_data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if(pixel_data == NULL)
    {
        fprintf(gpFile, "Error : failed to load texture %s.\n", filename);
        DestroyWindow(ghwnd);
    }

    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    //set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    //push the data to texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)pixel_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(pixel_data);
    pixel_data = NULL;
}

void Resize(int width, int height)
{
    //code
    //if current height is 0 set 1 to avoid 
    //divide by 0 error 
    if(height == 0)
        height = 1;

    //set viewport transformation
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void Display(void)
{
    //variable declarations
    mat4 modelMatrix;
    mat4 viewMatrix;

    static float angle = 0.0f;

    static float t = 0.0f;
	t += 0.02f;
	if (t > 360.0f)
		t = 0.0f;

    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    viewMatrix = mat4::identity();
    viewMatrix = camera.GetViewMatrix();

    glUseProgram(shaderProgramObject);
        //House Model
        modelMatrix = mat4::identity();

        modelMatrix = vmath::translate(0.0f, -1.8f, -12.5f);
        modelMatrix = modelMatrix * vmath::scale(0.2f, 0.2f, 0.2f);
        modelMatrix = modelMatrix * vmath::rotate(-90.0f, 0.0f, 1.0f, 0.0f);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

        glUniform3f(viewPosUniform, 0.0f, 0.0f, 0.0f);

        glUniform3f(lightPositionUniform, 10.0f, 10.0f, 10.0f);
        glUniform3f(lightAmbientUniform, 0.3f, 0.3f, 0.3f);
        glUniform3f(lightDiffuseUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(lightSpecularUniform, 1.0f, 1.0f, 1.0f);

        House.Draw(shaderProgramObject);

        //Panati
        for(float i = 0.0f; i < 4.0f; i += 1.0f)
        {
            modelMatrix = vmath::translate(i + 0.65f, -1.8f, -9.75f);
            modelMatrix = modelMatrix * vmath::scale(0.02f, 0.02f, 0.02f);
            modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

            Panti.Draw(shaderProgramObject);

            modelMatrix = vmath::translate(-(i + 0.65f), -1.8f, -9.75f);
            modelMatrix = modelMatrix * vmath::scale(0.02f, 0.02f, 0.02f);
            modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

            Panti.Draw(shaderProgramObject);
        }

        //Tree
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_NOTEQUAL, 0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        modelMatrix = vmath::translate(4.0f, -1.8f, -6.0f);
        modelMatrix = modelMatrix * vmath::scale(20.0f, 20.0f, 20.0f);
        modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        Tree.Draw(shaderProgramObject);

        modelMatrix = vmath::translate(-7.5f, -1.8f, -12.0f);
        modelMatrix = modelMatrix * vmath::scale(20.0f, 20.0f, 20.0f);
        modelMatrix = modelMatrix * vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        Tree.Draw(shaderProgramObject);

        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);

        //Ground
        modelMatrix = mat4::identity();
        modelMatrix = vmath::translate(0.0f, -1.8f, 0.0f);
        modelMatrix = modelMatrix * vmath::scale(40.0f, 20.0f, 40.0f);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floor_diffuse_texture);
        glUniform1i(diffuseTextureUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floor_specular_texture);
        glUniform1i(specularTextureUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, floor_normal_texture);
        glUniform1i(normalTextureUniform, 2);

        glUniform3f(materialAmbientUniform, 0.2f, 0.2f, 0.2f);
        glUniform3f(materialDiffuseUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(materialSpecularUniform, 1.0f, 1.0f, 1.0f);
        glUniform1f(materialShininessUniform, 50.0f);

        glBindVertexArray(vao_square);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    glUseProgram(0);

    glUseProgram(gShaderProgramObject);

    mat4 scaleMatrix	= mat4::identity();

	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glUniform1f(fadeoutFactorUniform, 1.0f);
	glUniform1f(fadeinFactorUniform, 1.0f);
	glUniform1f(timeUniform, t);

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_POINT_SPRITE);
	glPointSize(8);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSmily);
	glUniform1i(sTextureUniform, 0);

    for(float i = 0.0f; i < 4.0f; i += 1.0f)
    {
        modelMatrix	= mat4::identity();
        modelMatrix = vmath::translate(i + 0.65f, -1.6f, -10.7f);
        scaleMatrix = scale(0.05f, 0.1f, 1.0f);

        modelMatrix = modelMatrix * scaleMatrix;

        glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, numParticles);
        glBindVertexArray(0);

        modelMatrix	= mat4::identity();
        modelMatrix = vmath::translate(-(i + 0.65f), -1.6f, -10.7f);
        scaleMatrix = scale(0.05f, 0.1f, 1.0f);

        modelMatrix = modelMatrix * scaleMatrix;

        glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, numParticles);
        glBindVertexArray(0);
    }

	glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_BLEND);

	glUseProgram(0);

    SwapBuffers(ghdc);
}

void UnInitialize(void)
{
    //code
    //if window is in fullscreen mode toggle
    if(gbFullscreen == true)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0, 
            0,
            0,
            0,
            SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
    
        ShowCursor(true);
        gbFullscreen = false;
    }

    //release textures
    if(floor_diffuse_texture)
    {
        glDeleteTextures(1, &floor_diffuse_texture);
        floor_diffuse_texture = 0;
    }

    if(floor_specular_texture)
    {
        glDeleteTextures(1, &floor_specular_texture);
        floor_specular_texture = 0;
    }

    if(floor_normal_texture)
    {
        glDeleteTextures(1, &floor_normal_texture);
        floor_normal_texture = 0;
    }

    	if (vbo_position_rectangle)
	{
		glDeleteBuffers(1, &vbo_position_rectangle);
		vbo_position_rectangle = 0;
	}

	if (vbo_texture_rectangle)
	{
		glDeleteBuffers(1, &vbo_texture_rectangle);
		vbo_texture_rectangle = 0;
	}

	if (vao_rectangle)
	{
		glDeleteVertexArrays(1, &vao_rectangle);
		vao_rectangle = 0;
	}

	if (textureSmily)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		textureSmily = 0;
	}

    //safe shader cleanup
    if(shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(shaderProgramObject);
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(shaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        glUseProgram(0);
    }

    if(gShaderProgramObject)
    {
        GLsizei shader_count;
        GLuint* p_shaders = NULL;

        glUseProgram(gShaderProgramObject);
        glGetProgramiv(gShaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint*)malloc(shader_count * sizeof(GLuint));
        memset((void*)p_shaders, 0, shader_count * sizeof(GLuint));
    
        glGetAttachedShaders(gShaderProgramObject, shader_count, &shader_count, p_shaders);

        for(GLsizei i = 0; i < shader_count; i++)   
        {
            glDetachShader(gShaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(gShaderProgramObject);
        gShaderProgramObject = 0;
        glUseProgram(0);
    }


    //HGLRC : NULL means calling thread's current rendering context 
    //        is no longer current as well as it releases the device 
    //        context used by that rendering context
    //HDC : is ignored if HGLRC is passed as NULL
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent((HDC)NULL, (HGLRC)NULL);
    }

    //delete rendering context 
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = (HGLRC)NULL;
    }

    //release the device context
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = (HDC)NULL;
    }

    //close the log file
    if(gpFile)
    {
        fprintf(gpFile, "\n----- Program Completed Successfully -----\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}
