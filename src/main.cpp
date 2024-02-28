#include <windows.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>

/* For main window*/
const int WIDTH = 800, HEIGHT = 600;
const int MAPW = 10, MAPH = 10;
const int TILE_SIZE = 64;


/* Map */
int worldMap[MAPW][MAPH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

SDL_Window *win;
SDL_Renderer *rndr;

// player
double playerX = 2.0;
double playerY = 2.0;
double playerDirX = 1.0;
double playerDirY = 0.0;
double planeX = 0.0;
double planeY = 0.66; // The 2D raycaster version of camera plane

// info window for debuggin
SDL_Window *infWin;
SDL_Renderer *infRndr;
TTF_Font *font = TTF_OpenFont("lazy.ttf", 24);  

void drawColums(int x, int colH)
{
    // set the wall color
    int wall_color = 0x005fb8;

    // Draw the top half of the wall
    SDL_SetRenderDrawColor(rndr, (wall_color >> 16) & 0xFF, (wall_color >> 8) & 0xFF, wall_color & 0xFF, SDL_ALPHA_OPAQUE);
    SDL_Rect topHalf = {x, HEIGHT / 2 - colH / 2, 1, colH / 2};
    SDL_RenderFillRect(rndr, &topHalf);

    // Draw the bottom half of the wall
    SDL_SetRenderDrawColor(rndr, 0, 0, 0, SDL_ALPHA_OPAQUE); // Black color for the floor
    SDL_Rect bottomHalf = {x, HEIGHT / 2 + colH / 2, 1, colH / 2};
    SDL_RenderFillRect(rndr, &bottomHalf);
}

// FIXME: Either somethings fucked here or in rendering, i cant find out.
void updatePlayer()
{
    // Calculate the speed of movement
    double moveSpeed = 6.0;
   double rotSpeed = 0.05;

    // Update player controls
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    // Move forward
    if (keys[SDL_SCANCODE_W]) {
        playerX += playerDirX * moveSpeed;
        playerY += playerDirY * moveSpeed;
        SDL_Log("PLAYER MOVEMENT: W\n");
    }
    // Move backward
    if (keys[SDL_SCANCODE_S]) {
        playerX -= playerDirX * moveSpeed;
        playerY -= playerDirY * moveSpeed;
        SDL_Log("PLAYER MOVEMENT: S\n");
    }
    // Strafe right
    if (keys[SDL_SCANCODE_D]) {
        playerX += planeX * moveSpeed;
        playerY += planeY * moveSpeed;
        SDL_Log("PLAYER MOVEMENT: D\n");    
    }
    // Strafe left
    if (keys[SDL_SCANCODE_A]) {
        playerX -= planeX * moveSpeed;
        playerY -= planeY * moveSpeed;
        SDL_Log("PLAYER MOVEMENT: A\n");
    }
    // Rotate right
    if (keys[SDL_SCANCODE_RIGHT]) {
        double oldDirX = playerDirX;
        playerDirX = playerDirX * cos(-rotSpeed) - playerDirY * sin(-rotSpeed);
        playerDirY = oldDirX * sin(-rotSpeed) + playerDirY * cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        SDL_Log("PLAYER MOVEMENT: RIGHT");
    }
    // Rotate left
    if (keys[SDL_SCANCODE_LEFT]) {
        double oldDirX = playerDirX;
        playerDirX = playerDirX * cos(rotSpeed) - playerDirY * sin(rotSpeed);
        playerDirY = oldDirX * sin(rotSpeed) + playerDirY * cos(rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        SDL_Log("PLAYER MOVEMENT: LEFT");
    }
}

// info render
void renderInfo()
{
    SDL_SetRenderDrawColor(infRndr, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(infRndr);

    // Render player information
    char infoText[50];
    sprintf(infoText, "Player Position: (%.2lf, %.2lf)\nLook Direction: (%.2lf, %.2lf)\nSpeed: %.2lf",
            playerX, playerY, playerDirX, playerDirY, 6.0);
    
    SDL_Color textColor = {0, 0, 0, SDL_ALPHA_OPAQUE};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, infoText, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(infRndr, textSurface);

    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
    SDL_RenderCopy(infRndr, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(infRndr);
}


// fuck me
void render()
{
    SDL_SetRenderDrawColor(rndr,0,0,0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(rndr);

    for (int x = 0; x < WIDTH; x++)
    {
        // Calculate the ray's direction and initpos
        double camX = 2 * x / (double)WIDTH - 1;
        double rayDirX = 1, rayDirY = 0;

        // map pos
        int mapX = (int)camX;
        int mapY = 1;

        // len of ray from curpos to next x or y-side
        double sideDistX = fabs((mapX - camX + (1 - (rayDirX > 0))) / rayDirX);
        double sideDisty = fabs((mapY - camX + (1 - (rayDirY > 0))) / rayDirY);

        // delta dist to move horz or vert
        double deltaDistX = sqrt(1+(rayDirY*rayDirY)/(rayDirX*rayDirX));
        double deltaDistY = sqrt(1+(rayDirX*rayDirX)/(rayDirY*rayDirY));

        // calc steps and init side dist
        int stepX, stepY;
        int hit = 0; // Was there a wall hit?
        int side; // Was a North-South or East-West wall hit?

        if (rayDirX<0)
        {
            stepX = -1;
            sideDistX = (camX-mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - camX) * deltaDistX;
        }

        if (rayDirY<0)
        {
            stepY = -1;
            sideDisty = (camX - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDisty = (mapY + 1.0 - camX) * deltaDistY;
        }

        // preform someshit called Digital Differential ANALysis
        while (hit == 0)
        {
            // jump to next map sqr, or in x-dir, or in y-dir
            if (sideDistX < sideDisty)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0; // a north-south wall was hit
            }
            else
            {
                sideDisty += deltaDistY;
                mapY += stepY;
                side = 1; // a east-west wall was hit
            }

            if (worldMap[mapX][mapY] > 0)
            {
                hit = 1;
            }
        }

        // Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!) (Thanks gpt)
        double perpWallDist;       
        if (side == 0)
        {
            perpWallDist = fabs((mapX - camX + (1-(rayDirX>0)))/rayDirX);
        }
        else
        {
            perpWallDist = fabs((mapY - camX + (1-(rayDirY > 0)))/rayDirY);
        }

        int lineH = (int)(HEIGHT / perpWallDist);

        drawColums(x, lineH);
    }

    SDL_RenderPresent(rndr);
}

// the actual fucking window
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    if (TTF_Init() < 0)
    {
        printf("SDL_ttf couldn't initalize! SDL_TTF_Error: %s", TTF_GetError());
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    win = SDL_CreateWindow("widow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    rndr = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    infWin = SDL_CreateWindow("debug window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
    infRndr = SDL_CreateRenderer(infWin, -1, SDL_RENDERER_ACCELERATED);
    SDL_Log("Welcome to my world bitch\nDon't worry about this console window, its mainly for logging and debuging.\n-----------------------------------------\n");

    
    // Check for font loading errors if needed
    if (!font || font==NULL ) 
    {
        if (font == NULL)
        {
            printf("Font is null\n");
        }
           fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        // Handle the error as needed
    }

    // the main loop
    while (true)
    {
        SDL_Event evnt;
        if (SDL_PollEvent(&evnt) && evnt.type == SDL_QUIT)
        {
            break;
        }
        updatePlayer();
        render();
        renderInfo();
        SDL_Delay(10);
    }

    // cleanup our mess
    SDL_DestroyRenderer(rndr);
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(infRndr);
    SDL_DestroyWindow(infWin);
    SDL_Quit();

    return 0;
}
