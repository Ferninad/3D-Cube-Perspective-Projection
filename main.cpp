#include "common.h"
#include "cmath"
#include "vector"

bool Init();
void CleanUp();
void Run();
void Draw();
void Connect(int i, int j, vector<vector<double>> pps);
void Rots();
void Setup();
vector<vector<double>> MultMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2);
vector<vector<double>> SubMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2);

SDL_Window *window;
SDL_GLContext glContext;
SDL_Surface *gScreenSurface = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Rect pos;

int screenWidth = 500;
int screenHeight = 500;
int cornerSize = 1;
int lineSize = 1;
double xang = 0;
double yang = 0;
double zang = 0;
double cx = 0;
double cy = 0;
double cz = 5;
double ax = 0;
double ay = 0;
double az = 0;

vector<vector<double>> points;
vector<vector<double>> cameraPosition;
vector<vector<double>> cameraOrientation;
vector<vector<double>> display;
vector<vector<double>> rotx;
vector<vector<double>> roty;
vector<vector<double>> rotz;
vector<vector<double>> camrotx;
vector<vector<double>> camroty;
vector<vector<double>> camrotz;
vector<vector<double>> projection;

bool Init()
{
    if (SDL_Init(SDL_INIT_NOPARACHUTE & SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
        return false;
    }
    else
    {
        //Specify OpenGL Version (4.2)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_Log("SDL Initialised");
    }

    //Create Window Instance
    window = SDL_CreateWindow(
        "Game Engine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        screenWidth,
        screenHeight,   
        SDL_WINDOW_OPENGL);

    //Check that the window was succesfully created
    if (window == NULL)
    {
        //Print error, if null
        printf("Could not create window: %s\n", SDL_GetError());
        return false;
    }
    else{
        gScreenSurface = SDL_GetWindowSurface(window);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Log("Window Successful Generated");
    }
    //Map OpenGL Context to Window
    glContext = SDL_GL_CreateContext(window);

    return true;
}

int main()
{
    //Error Checking/Initialisation
    if (!Init())
    {
        printf("Failed to Initialize");
        return -1;
    }

    // Clear buffer with black background
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //Swap Render Buffers
    SDL_GL_SwapWindow(window);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    Run();

    CleanUp();
    return 0;
}

void CleanUp()
{
    //Free up resources
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Run()
{
    bool gameLoop = true;
    srand(time(NULL));
    
    Setup();
    Rots();
    display.push_back({0});
    display.push_back({0});
    display.push_back({1});

    while (gameLoop)
    {   
        Draw();
        SDL_RenderPresent(renderer);
        pos.x = 0;
        pos.y = 0;
        pos.w = screenWidth;
        pos.h = screenHeight;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &pos);
        
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gameLoop = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        gameLoop = false;
                        break;
                    case SDLK_w:
                        cz -= .05;
                        break;
                    case SDLK_s:
                        cz += .05;
                        break;
                    case SDLK_d:
                        cx -= .05;
                        break;
                    case SDLK_a:
                        cx += .05;
                        break;
                    case SDLK_e:
                        cy += .05;
                        break;
                    case SDLK_q:
                        cy -= .05;
                        break;
                    case SDLK_RIGHT:
                        yang -= .05;
                        break;
                    case SDLK_LEFT:
                        yang += .05;
                        break;
                    default:
                        break;
                }
            }

            if (event.type == SDL_KEYUP)
            {
                switch (event.key.keysym.sym){
                    default:
                        break;
                }
            }
        }
    }
}

void Draw(){
    vector<vector<double>> pps;
    vector<vector<double>> xyz;
    Rots();
    for(int i = 0; i < points.size(); i++){
        xyz.clear();
        for(int j = 0; j < points[i].size(); j++){
            xyz.push_back({points[i][j]});
        }
        vector<vector<double>> rotated = MultMatrixs(roty, xyz);
        rotated = MultMatrixs(rotx, rotated);
        rotated = MultMatrixs(rotz, rotated);
        vector<vector<double>> diffpc = SubMatrixs(rotated, cameraPosition);
        vector<vector<double>> transform = MultMatrixs(diffpc, camrotx);
        transform = MultMatrixs(camroty, transform);
        transform = MultMatrixs(camrotz, transform);
        pps.push_back({(display[2][0] / transform[2][0]) * transform[0][0] + display[0][0], (display[2][0] / transform[2][0]) * transform[1][0] + display[1][0]});
        
        pos.x = pps[i][0] * 200 + screenWidth / 2;
        pos.y = pps[i][1] * 200 + screenHeight / 2;
        pos.w = cornerSize;
        pos.h = cornerSize;
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &pos);
    }
    for (int i = 0; i < 4; i++) {
        Connect(i, (i + 1) % 4, pps);
        Connect(i + 4, ((i + 1) % 4) + 4, pps);
        Connect(i, i + 4, pps);
    }
}

void Connect(int i, int j, vector<vector<double>> pps){
    int ix = pps[i][0] * 200 + screenWidth / 2;
    int iy = pps[i][1] * 200 + screenHeight / 2;
    int jx = pps[j][0] * 200 + screenWidth / 2;
    int jy = pps[j][1] * 200 + screenHeight / 2;

    SDL_RenderDrawLine(renderer, ix, iy, jx, jy);
}

vector<vector<double>> MultMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2){
    vector<vector<double>> result;
    vector<double> temp;
    double a = 0;
    for(int j = 0; j < mat1.size(); j++){
        for(int k = 0; k < mat2[0].size(); k++){
            for(int i = 0; i < mat1[j].size(); i++){
                a+= mat1[j][i] * mat2[i][k];
            }
            temp.push_back(a);
            a = 0;
        }
        result.push_back(temp);
        temp.clear();
    }
    return result;
}

vector<vector<double>> SubMatrixs(vector<vector<double>> mat1, vector<vector<double>> mat2){
    vector<vector<double>> result;
    vector<double> temp;
    double a = 0;
    for(int j = 0; j < mat1.size(); j++){
        for(int k = 0; k < mat1[j].size(); k++){
            a = mat1[j][k] - mat2[j][k];
            temp.push_back(a);
            a = 0;
        }
        result.push_back(temp);
        temp.clear();
    }
    return result;
}

void Setup(){
    points.push_back({-1, -1, -1});
    points.push_back({1, -1, -1});
    points.push_back({1, 1, -1});
    points.push_back({-1, 1, -1});
    points.push_back({-1, -1, 1});
    points.push_back({1, -1, 1});
    points.push_back({1, 1, 1});
    points.push_back({-1, 1, 1});
    
    projection.push_back({1, 0, 0});
    projection.push_back({0, 1, 0});
}

void Rots(){
    vector<double> temp;
    rotx.clear();
    roty.clear();
    rotz.clear();
    camrotx.clear();
    camroty.clear();
    camrotz.clear();
    cameraPosition.clear();
    cameraOrientation.clear();

    cameraPosition.push_back({cx});
    cameraPosition.push_back({cy});
    cameraPosition.push_back({cz});
    cameraOrientation.push_back({ax, ay, az});

    camrotx.push_back({1, 0, 0});
    camrotx.push_back({0, cos(cameraOrientation[0][0]), sin(cameraOrientation[0][0])});
    camrotx.push_back({0, -1*sin(cameraOrientation[0][0]), cos(cameraOrientation[0][0])});

    camroty.push_back({cos(cameraOrientation[0][1]), 0, -1*sin(cameraOrientation[0][1])});
    camroty.push_back({0, 1, 0});
    camroty.push_back({sin(cameraOrientation[0][1]), 0, cos(cameraOrientation[0][1])});

    camrotz.push_back({cos(cameraOrientation[0][2]), sin(cameraOrientation[0][2]), 0});
    camrotz.push_back({-1*sin(cameraOrientation[0][2]), cos(cameraOrientation[0][2]), 0});
    camrotz.push_back({0, 0, 1});

    rotx.push_back({1, 0, 0});
    rotx.push_back({0, cos(xang), -1*sin(xang)});
    rotx.push_back({0, sin(xang), cos(xang)});

    roty.push_back({cos(yang), 0, sin(yang)});
    roty.push_back({0, 1, 0});
    roty.push_back({-1*sin(yang), 0, cos(yang)});

    rotz.push_back({cos(zang), -1*sin(zang), 0});
    rotz.push_back({sin(zang), cos(zang), 0});
    rotz.push_back({0, 0, 1});
}