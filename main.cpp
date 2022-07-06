#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "include/vec2.h"
#include "include/circle.h"
#include "include/vecMath.h"
#include "include/boundary.h"
#include "include/rectangle.h"

using namespace std;

Rectangle box(200, 400, 400, 100);

#define PI 3.14

class Simulation {
    public:

        void LeftClick(SDL_MouseButtonEvent& b) {
            if(b.button == SDL_BUTTON_LEFT){
                if(!pointASelected) {
                    int posX;
                    int posY;
                    SDL_GetMouseState(&posX, &posY);
                    pA.setVec(posX, posY);
                    pointASelected = true;
                }
                else {
                    int posX;
                    int posY;
                    SDL_GetMouseState(&posX, &posY);
                    if(posX != pA.getX() && posY != pA.getY()) {
                        pB.setVec(posX, posY);
                        boundaries.push_back(new Boundary(pA, pB));
                        pointASelected = false;
                    }
                    
                }
            }
        }

        void RightClick(SDL_MouseButtonEvent& b) {
            if(b.button == SDL_BUTTON_RIGHT){
                int posX;
                int posY;
                SDL_GetMouseState(&posX, &posY);
                Vec2 pos(posX, posY);
                Vec2 vel(0, -5);
                circles.push_back(GenerateCircle(pos, vel, rand() % 10 + 1));
            }
        }

        Simulation() {
            pointASelected = false;
            srand (time(NULL));
            quit_flag = false;
            init_error = SDL_Init( SDL_INIT_VIDEO );
            window = SDL_CreateWindow("Physics Engine",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                WIDTH, HEIGHT,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            gravity.setVec(0, 0.1);
            
        }

        ~Simulation(){
            SDL_DestroyRenderer( renderer );
            SDL_DestroyWindow( window );
            SDL_Quit();
            cout << "EXIT SUCCESS" << endl;
        }

        Circle* GenerateCircle (Vec2 pos, Vec2 vel, double mass) {
            Vec2 a(0, 0);

            SDL_Color color;
            color.r = rand() % 255 + 1;
            color.g = rand() % 255 + 1;
            color.b = rand() % 255 + 1;
            color.a = 255;
            return new Circle(pos, vel, a, mass, color);
        }

        /* MAIN SIMULATION LOOP */
        int MainLoop() {
            
            while (!quit_flag) {
                FillScreen(0,0,0,255);
                
                /* Check for events */
                while (SDL_PollEvent(&e)){
                    if (e.type == SDL_QUIT){
                        quit_flag = true;
                    }
                    if (e.type == SDL_MOUSEBUTTONDOWN) {
                        LeftClick(e.button);
                        RightClick(e.button);
                    }
                }

                /* Update and render circles on screen */
                for (int c = 0; c < (int) circles.size(); c++) {
                    /* Update the position of a circle */
                    circles[c]->Update();

                    /* Check for and resolve collisions against the borders, line segments, and other circles */
                    circles[c]->CollisionEdges(WIDTH, HEIGHT);
                    circles[c]->CollisionBoundaries(boundaries, renderer);
                    circles[c]->CollisionCircles(circles);
                    circles[c]->CollisionRectangles(box, renderer);
                    
                    /* If circle is colliding with another circle or line, don't apply gravity */
                    /* This is a hacky solution to preventing objects from clipping into eachother */
                    /* Once we have repulsion implemented, we can repulse objects when they intersect */
                    if(!circles[c]->getCollisionWithCircle()) {
                        circles[c]->ApplyForce(gravity);
                    }
                    else {
                        Vec2 reverseGravity(0, -0.1);
                        circles[c]->ApplyForce(reverseGravity);
                    }
                    if(!circles[c]->getCollisionWithBoundary()) {
                        circles[c]->ApplyForce(gravity);
                    }
                    else {
                        Vec2 reverseGravity(0, -0.1);
                        circles[c]->ApplyForce(reverseGravity);
                    }

                    /* Reset collisions flag to false and draw the circle */
                    circles[c]->setCollisionWithBoundary(false);
                    circles[c]->setCollisionWithCircle(false);
                    circles[c]->Draw(renderer);
                }

                /* Draw all of the lines (boundaries) */
                for(int i = 0; i < (int) boundaries.size(); i++) {
                    boundaries[i]->Draw(renderer);
                }

                box.Draw(renderer);
                box.Fill(renderer);
                
                SDL_RenderPresent(renderer);
                SDL_Delay(1000 / FRAMERATE);
            }
            return 0;
        }

        void FillScreen(int r, int g, int b, int a){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }

        int CheckForErrors() {

            if ( init_error < 0 ) {
                cout << "SDL Could not be initialized: " << endl;
                return EXIT_FAILURE;
            }
            if( !window ) {
                cout << "Window could not be created! SDL_Error: " << endl;
                return EXIT_FAILURE;
            }
            if( !renderer ) {
                cout << "Renderer could not be created! SDL_Error: " << endl;
                return EXIT_FAILURE;
            }
            return 0;
        }
        


    private:
        SDL_Window* window = NULL;
        SDL_Renderer* renderer = NULL;
        SDL_Event e;

        const int WIDTH = 1000;
        const int HEIGHT = 600;
        const int FRAMERATE = 60;

        int init_error;
        bool quit_flag;

        vector<Circle*> circles;
        vector<Circle*> pegs; 
        vector<Boundary*> boundaries;  
        Vec2 gravity;

        bool pointASelected;
        Vec2 pA;
        Vec2 pB;
};


int WinMain () {

    Simulation physics_engine;

    // Check for any initialization errors
    
    if(physics_engine.CheckForErrors()) return EXIT_FAILURE;
    physics_engine.MainLoop();

    return EXIT_SUCCESS;
}