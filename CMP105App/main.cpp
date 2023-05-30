//Important headers
#include "ThreadPool.h"//
#include<iostream>
#include<string.h>
#include<SFML/Graphics.hpp>
#include<SFML/Graphics/Sprite.hpp>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <time.h>
#include <ppl.h>
#include <thread>
#include <mutex>
#include <vector>
#include<atomic>
#include<stdlib.h> 


using std::chrono::duration_cast;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::complex;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread; //for making multiple threads
using std::atomic; //for inter-thread synchronisation
using std::vector; //for defining vectors
using std::mutex;  //for protecting shared data from being 
				   //accessed simultaneously by multiple threads 




typedef std::chrono::steady_clock the_clock;
the_clock::time_point start;
the_clock::time_point stop;

//declaring variables for height and width of mandelbrot set

const int H = 720;
const int W = 1280;

//defining vertex array 
sf::VertexArray p(sf::Points, W* H);

//for observing state of the thread and returning the corresponding output
double threadAmount = std::thread::hardware_concurrency();


//These variables are defined for calculating the
//height and width of the segments
const float hos = H / threadAmount;
const float wos = 0;

//Variables for 
//defining x coordinate
//defining y coordinate
//defining number of iterations 
//defining a constant value for magnifying in and out
//of the mandelbrot set
int cordy = H / 2;
int cordx = W / 2;
int iterAmount = 100;
float magnify = 100.0f;

//declaring a variable to check whether the 
bool Tl = true;

//for declaring complex variables 
struct complex_number
{

	long double im;//imaginary
	long double re;//real
};

//Function modified given to us in the lecture
//for computing the mandelbrot on the cpu
void mandelbrot_cpu(double beg, double term, int cordx, int cordy, int iterAmount, float magnify)
{

	//loop for calculating postion of each pixel
	for (int indy = beg; indy < term; indy++)
	{
		for (int indx = 0; indx < W; indx++)
		{
			//this helps in defining the pixel location to the 
			//complex plane for calculations
			long double iy = ((long double)indy - cordy) / magnify;
			long double ix = ((long double)indx - cordx) / magnify;


			//defining complex number
			complex_number cn;
			cn.im = iy;//
			cn.re = ix;//


			//passing the value into another variable
			complex_number z_com = cn;
			int it = 0;
			for (int k = 0; k < iterAmount; k++)
			{
				complex_number com;
				com.re = z_com.re * z_com.re - z_com.im * z_com.im;
				com.im = 2 * z_com.re * z_com.im;
				com.re += cn.re;
				com.im += cn.im;
				z_com = com;
				it++;
				if (z_com.re * z_com.re + z_com.im * z_com.im > 4)
					break;
			}


			//Condition for adding colour to 
			//the mandelbrot set
			if (it < iterAmount / 4.0f)

			{
				p[indy * W + indx].position = sf::Vector2f(indx, indy);
				sf::Color color(0, it * 255.0f / (iterAmount / 4.0f), 0);
				p[indy * W + indx].color = color;
			}																   //This block of code
			else if (it < iterAmount / 2.0f)								   //outlines the mandelbrot
			{																   //green in colour
				p[indy * W + indx].position = sf::Vector2f(indx, indy);
				sf::Color color(it * 255.0f / (iterAmount / 2.0f), 0, 0);
				p[indy * W + indx].color = color;
			}
			else if (it < iterAmount)
			{
				p[indy * W + indx].position = sf::Vector2f(indx, indy);
				sf::Color color(0, 0, it * 255.0f / iterAmount);
				p[indy * W + indx].color = color;
			}


		}
	}


}





int main()
{

	ThreadPool cluster(threadAmount);

	//Command for input
	sf::String title_string = "Mandelbrot";




	//Command for rendering a window
	sf::RenderWindow window(sf::VideoMode(W, H), title_string);




	//for displaying information regarding 
	//zooming of mandelbrot

	sf::Font font;
	if (!font.loadFromFile("arial.ttf"))
	{
		cout << "ERROR";
		// error...
	}

	sf::Text text;
	// select the font
	text.setFont(font); // font is a sf::Font

	// set the string to display
	text.setString("Hello world");

	// set the character size
	text.setCharacterSize(24); // in pixels, not points!

	// set the color
	text.setFillColor(sf::Color::Red);

	// set the text style
	text.setStyle(sf::Text::Bold | sf::Text::Underlined);




	for (int i = 0; i < threadAmount; i++)
	{

		mandelbrot_cpu((H / threadAmount) * (i), (H / threadAmount) * (i + 1), cordx, cordy, iterAmount, magnify);  //this loop helps 
																													//in rendering the first mandelbrot
	}																												//set on the sfml window 



	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		static bool clicked = false;

		//Syntax for magnifying the mandelbrot set
		//on left click of the mouse

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (!clicked) {
				clicked = true;
				sf::Vector2i position = sf::Mouse::getPosition(window);
				cordx -= position.x - cordx;
				cordy -= position.y - cordy;
				magnify *= 2;
				iterAmount += 100;
				for (int i = 0; i < W * H; i++)
				{
					p[i].color = sf::Color::Black;
				}

				for (int i = 0; i < threadAmount; i++)
				{
					cluster.enqueue(mandelbrot_cpu, (H / threadAmount) * (i), (H / threadAmount) * (i + 1), cordx, cordy, iterAmount, magnify);
				}
				while (!cluster.isEmpty());
			}
		}

		//Syntax for magnifying out of the mandelbrot set
		//on right click of the mouse

		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			if (!clicked) {
				clicked = true;
				sf::Vector2i position = sf::Mouse::getPosition(window);
				cordx -= position.x - cordx;
				cordy -= position.y - cordy;
				magnify /= 2;
				iterAmount -= 100;
				for (int i = 0; i < W * H; i++)
				{
					p[i].color = sf::Color::Black;
				}

				for (int i = 0; i < threadAmount; i++)
				{
					cluster.enqueue(mandelbrot_cpu, (H / threadAmount) * (i), (H / threadAmount) * (i + 1), cordx, cordy, iterAmount, magnify);
				}
				while (!cluster.isEmpty());
			}
		}



		else


			clicked = false;

		window.clear();
		window.draw(p);
		char str[100];
		sprintf_s(str, "Magnify:x%2.2lf", magnify);
		text.setString(str);
		window.draw(text);
		window.display();


	}





	return 0;

}





