/*
 *
 * Code by Eckart Ferdinand Cobo Briesewitz ( alias Tupryk )
 * Date: 10/2021
 *
*/

#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h> // //g++ raytracer.cpp -o raytracer -F/Library/Frameworks -framework SDL2

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

SDL_Renderer *renderer;
SDL_Window *window;

float FOV = 10; // Distance between camera and screen

struct Vector3D
{
	float x, y, z;
};

float cos_of_vectors( Vector3D vec1, Vector3D vec2 )
{
	return ( (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z) ) /
	( sqrt( (vec1.x*vec1.x) + (vec1.y*vec1.y) + (vec1.z*vec1.z) ) * sqrt( (vec2.x*vec2.x) + (vec2.y*vec2.y) + (vec2.z*vec2.z) ) );
}

Vector3D camera;

struct Ball
{
	float radious;
	Vector3D position;

	void init ( float x, float y, float z, float rad )
	{
		position.x = x;
		position.y = y;
		position.z = z;
		radious = rad;
	}
};

void draw_figure ( Ball ball, Vector3D light )
{
	// Camera postion is always xyz = 0 to make things easier
	for ( int j = 0; j < WINDOW_HEIGHT; j++ ) {
		for ( int i = 0; i < WINDOW_WIDTH; i++ )
		{
			// Distances between objects that we will use later...
			float camera_ball;
			float camera_light = sqrt( pow(camera.x - light.x, 2) + pow(camera.y - light.y, 2) + pow(camera.z - light.z, 2) );
			float light_ball;

			// First create a vector from camera to pixel
			// Second check is it hits the ball / sphere
			// If true do raytracing calculations

			Vector3D camera_ray; // vector of the camera ( position of the vector is xyz = 0 )
			camera_ray.x = -0.5*WINDOW_WIDTH + i;
			camera_ray.y = FOV;
			camera_ray.z = -0.5*WINDOW_HEIGHT + j;

			// We need to use the ecuation of a sphere S => (x - P1)^2 + (y - P2)^2 + (z - P3)^2 = R^2
			// And the ecuation of a line x = V1*t + P1; y = V3*t + P3; z = V3*t + P3;
			// We replace de values of xyz of the line on the sphere and calculate t
			// Before we calculate t in a*t^2 + bt +c = 0, we need to check if this is posible, we do this by checking if b^2 - 4ac >= 0

			// The reason why there are unecesary ceros is because those represent the camera position, which I find makes it easier to understand
			float a = camera_ray.x*camera_ray.x + camera_ray.y*camera_ray.y + camera_ray.z*camera_ray.z;
			float b = -2*( camera_ray.x*( camera.x - ball.position.x ) + camera_ray.y*( camera.y - ball.position.y ) + camera_ray.z*( camera.z - ball.position.z ) );
			float c = pow( camera.x - ball.position.x, 2 ) + pow( camera.y - ball.position.y, 2 ) + pow( camera.z - ball.position.z, 2 ) - pow( ball.radious, 2);

			bool hit = false;
			if ( b*b - 4*a*c >= 0 )
			{
				hit = true;
			}

			if (hit) // Raytracing stuff :D
			{
				// t has two results, whe will take the hit that has the lowest distance
				float t1 = (-b + sqrt( pow(b, 2) - (4*a*c) )) / (2*a);
				float t2 = (-b - sqrt( pow(b, 2) - (4*a*c) )) / (2*a);

				// Calculate distances in both cases ( t1 and t2 )
				// Once again, 0 represents the coordinates of the camera
				Vector3D hit1;
				hit1.x = camera_ray.x*t1 + camera.x;
				hit1.y = camera_ray.y*t1 + camera.y;
				hit1.z = camera_ray.z*t1 + camera.z;

				Vector3D hit2;
				hit2.x = camera_ray.x*t2 + camera.x;
				hit2.y = camera_ray.y*t2 + camera.y;
				hit2.z = camera_ray.z*t2 + camera.z;

				Vector3D normal;
				Vector3D light_object_vec;

				float distance1 = sqrt( pow(hit1.x - camera.x, 2) + pow(hit1.y - camera.y, 2) + pow(hit1.z - camera.z, 2) );
				float distance2 = sqrt( pow(hit2.x - camera.x, 2) + pow(hit2.y - camera.y, 2) + pow(hit2.z - camera.z, 2) );

				if ( distance1 < distance2 ) {
					camera_ball = distance1;
					light_ball = sqrt( pow(hit1.x - light.x, 2) + pow(hit1.y - light.y, 2) + pow(hit1.z - light.z, 2) );
					normal.x = hit1.x - ball.position.x;
					normal.y = hit1.y - ball.position.y;
					normal.z = hit1.z - ball.position.z;
					light_object_vec.x = hit1.x - light.x;
					light_object_vec.y = hit1.y - light.y;
					light_object_vec.z = hit1.z - light.z;
				} else {
					camera_ball = distance2;
					light_ball = sqrt( pow(hit2.x - light.x, 2) + pow(hit2.y - light.y, 2) + pow(hit2.z - light.z, 2) );
					normal.x = hit2.x - ball.position.x;
					normal.y = hit2.y - ball.position.y;
					normal.z = hit2.z - ball.position.z;
					light_object_vec.x = hit2.x - light.x;
					light_object_vec.y = hit2.y - light.y;
					light_object_vec.z = hit2.z - light.z;
				}

				// Now whe just need to calculate the cosine of the angle formed by the camera to object ray and the object to light ray with trigonometry
				float cos = ( pow(light_ball, 2) + pow(camera_ball, 2) - pow(camera_light, 2) ) / ( 2 * camera_ball * light_ball );

				// Last step! apply the cosine to the color or light level on the screen
				float light_intensity = 255-(127 * cos + 128);

				// Apply the normal of the hitten pixel to the calculated color
				light_intensity = light_intensity - (light_intensity*0.5 * cos_of_vectors( normal, light_object_vec ) + light_intensity*0.5);

				SDL_SetRenderDrawColor(renderer, light_intensity, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, i, j);
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawPoint(renderer, i, j);
			}
		}
	}
}

int main ()
{
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer);

	camera.x = 0;
	camera.y = 0;
	camera.z = 0;

	Vector3D light;
	light.x = 0;
	light.y = -100;
	light.z = 0;

	Ball ball;
	ball.init( 0, 120, 0, 119 );

	bool switch_ = true;
	while (1) {
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
			break;
		}
		SDL_RenderClear(renderer);

		if (light.x <= 150 && switch_)
		{
			light.x += 20;
			light.y += 20;
			light.z += 20;
		}
		else if (switch_)
		{
			switch_ = false;
		}
		if (light.x >= -150 && !switch_)
		{
			light.x -= 20;
			light.y -= 20;
			light.z -= 20;
		}
		else if (!switch_)
		{
			switch_ = true;
		}
		draw_figure ( ball, light );
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
