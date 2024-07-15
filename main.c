#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "src/include/SDL2/SDL.h"
#include "src/include/SDL2/SDL_keycode.h"
#include "src/include/SDL2/SDL_rect.h"
#include "src/include/SDL2/SDL_render.h"

#define PI 3.14159265359
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR 0.0174533 // One degree in radians

#define WIDTH3D 320
#define HEIGHT3D 200

#define WIDTH 1024
#define HEIGHT 512

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Window *window3d;
SDL_Renderer *renderer3d;

bool quit = false;

// Position
int px, py, pdx, pdy;

float pa;

int mapX = 8, mapY = 8, mapS = 64;

int map[64] = { 
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

void init() {
    // init SDL
    SDL_Init(SDL_INIT_VIDEO);
    /*
    window = SDL_CreateWindow("Window", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    */

    window3d = SDL_CreateWindow("3d", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 3 * WIDTH3D, 3 * HEIGHT3D, 0);
    renderer3d = SDL_CreateRenderer(window3d, -1, 0);

    // Doom Res
    SDL_RenderSetLogicalSize(renderer3d, WIDTH3D, HEIGHT3D);


    px = 300;
    py = 300;
    pdx = cos(pa) * 5;
    pdy = sin(pa) * 5;
}

void drawPlayer(){
    SDL_Rect player;

    player.x = px-2;
    player.y = py-2;
    player.h = 5;
    player.w = 5;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &player);

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawLine(renderer, px, py, px + 3*pdx, py + 3*pdy);
}

void drawMap() {
    int x, y, xo, yo;
    for(y = 0; y < mapY; y++) {
        for(x = 0; x < mapX; x++) {
            if(map[y*mapX+x] == 1) { SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); } else {  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  } 
            SDL_Rect wall;
            xo = x*mapS; yo = y * mapS;
            wall.x = xo + 1;
            wall.y = yo + 1;
            wall.w = mapS - 1;
            wall.h = mapS - 1;
            SDL_RenderFillRect(renderer, &wall);
        }
    }
}

void buttons(SDL_Event event){
        switch(event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_a:
                        // move left
                        px -= -pdy;
                        py -= pdx;
                        break;
                    case SDLK_d:
                        // move right
                        px += -pdy;
                        py += pdx;
                        break;
                    case SDLK_w:
                        // move forward
                        px += pdx;
                        py += pdy;
                        break;
                    case SDLK_s:
                        // move backwords
                        px -= pdx;
                        py -= pdy;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                    printf("%d\n", event.motion.x);
                    pa += 0.01 * event.motion.x;
                    if(pa<0)
                        pa += 2*PI;
                    if(pa>2*PI)
                        pa -= 2*PI;
                    pdx = cos(pa) * 5;
                    pdy = sin(pa) * 5;
                    break;

        }
}

float dist(float ax, float ay, float bx, float by, float ang) {
    return (sqrt((bx - ax)*(bx - ax) + (by - ay)*(by - ay)));
}

void drawRays3D() {
    int r, mx, my, mp, dof; float rx, ry, ra, xo, yo, disT;
    ra = pa-DR*40; if(ra<0) {ra+=2*PI;} if(ra>2*PI) {ra-=2*PI;}
    for(r = 0; r < 80; r++) {

        /* ---Check Horizontal Lines--- */
        dof=0;
        float disH = 1000000, hx = px, hy = py;
        float Tan = -1/tan(ra);
        if(ra>PI){ ry=(((int)py>>6)<<6) - 0.0001; rx = (py-ry) * Tan + px; yo =-64; xo = -yo * Tan; } // Looking Up
        if(ra<PI){ ry=(((int)py>>6)<<6) + 64;     rx = (py-ry) * Tan + px; yo = 64; xo = -yo * Tan; } // Looking Down
        if(ra == 0 || ra == PI) { rx = px; ry = py; dof = 8; } // left or right
        while(dof < 8) {
            mx = (int)(rx)>>6; my = (int)(ry)>>6; mp = my * mapX + mx;
            if(mp > 0 && mp < mapX*mapY && map[mp] == 1) { hx=rx; hy=ry; disH=dist(px, py, hx, hy, ra); dof = 8; } // hit wall
            else{ rx += xo; ry += yo; dof += 1; } // next line
        }

        /* ---Check Vertical Lines--- */
        dof=0;
        float disV = 1000000, vx = py, vy = py;
        float nTan = -tan(ra);
        if(ra>P2 && ra<P3){ rx=(((int)px>>6)<<6) - 0.0001; ry = (px-rx) * nTan + py; xo =-64; yo = -xo * nTan; } // Looking Left
        if(ra<P2 || ra>P3){ rx=(((int)px>>6)<<6) + 64;     ry = (px-rx) * nTan + py; xo = 64; yo = -xo * nTan; } // Looking Right
        if(ra == P2 || ra == P3) { rx = px; ry = py; dof = 8; } // up or down
        while(dof < 8) {
            mx = (int)(rx)>>6; my = (int)(ry)>>6; mp = my * mapX + mx;
            if(mp > 0 && mp < mapX*mapY && map[mp] == 1) { vx=rx; vy=ry; disV=dist(px, py, vx, vy, ra); dof = 8; } // vit wall
            else{ rx += xo; ry += yo; dof += 1; } // nest line
        }

        if (disH < disV) { rx = hx; ry = hy; disT = disH; SDL_SetRenderDrawColor(renderer3d, 255, 0, 0, 255); }
        if (disH > disV) { rx = vx; ry = vy; disT = disV; SDL_SetRenderDrawColor(renderer3d, 200, 0, 0, 255); }
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); SDL_RenderDrawLine(renderer, px, py, rx, ry);

        /* ---Draw 3d Walls */
        float ca=pa-ra; if(ca<0) { ca+=2*PI; } if(ca>2*PI) { ca-=2*PI; } disT=disT*cos(ca);
        float lineH=(mapS*200)/disT; if(lineH > 200) { lineH = 200; }
        float lineO = 100-lineH/2;
        SDL_Rect line;
        line.x = r*4;
        line.y = lineO;
        line.w = 4;
        line.h = lineH;
        SDL_RenderFillRect(renderer3d, &line);
        ra += DR; if(ra<0) {ra+=2*PI;} if(ra>2*PI) {ra-=2*PI;}
    }
}

void drawBackGround() {
    SDL_Rect sky;
    sky.x = 0;
    sky.y = 0;
    sky.w = WIDTH3D;
    sky.h = HEIGHT3D / 2;

    SDL_Rect ground;
    ground.x = 0;
    ground.y = HEIGHT3D / 2;
    ground.w = WIDTH3D;
    ground.h = HEIGHT3D / 2;

    SDL_SetRenderDrawColor(renderer3d, 0, 100, 255, 255); 
    SDL_RenderFillRect(renderer3d, &sky);

    SDL_SetRenderDrawColor(renderer3d, 0, 255, 100, 255); 
    SDL_RenderFillRect(renderer3d, &ground);
}


int main(int argc, char *argv[]) {

    SDL_Event event;

    // Map
    init();

    while (!quit) {

        SDL_Delay(10);
        SDL_PollEvent(&event);

        /* Clear Screen */
       SDL_SetRenderDrawColor(renderer, 50, 50 , 50, 255);
       SDL_RenderClear(renderer);
       SDL_SetRenderDrawColor(renderer3d, 50, 50 , 50, 255);
       SDL_RenderClear(renderer3d);
       
       buttons(event);

       drawBackGround();
       drawRays3D();

       /* Render Screen */
       SDL_RenderPresent(renderer);
       SDL_RenderPresent(renderer3d);
    }

    /* Clean Up */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
