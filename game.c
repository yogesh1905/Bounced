#include <stdio.h> 
#include <stdlib.h> 
#include<time.h> 
#include <ncurses.h>
#include <unistd.h>
#include<omp.h>
#define DELAY 50000
#define MAX_COMB 4
#define NUM_BALLS 2
#define GROUND_Y 20
#define NUM_SPIKES 3
#define SEC 1000000
// mvprintw to print string and mvaddch to print character

int max_x = 0, max_y = 0;

void initcolor()
{
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_WHITE, COLOR_BLACK);
	init_pair(6, COLOR_BLACK, COLOR_BLACK);
	
	// Change the background color
	//wbkgd(stdscr, COLOR_PAIR(4));
	
}

int min(int x, int y)
{
	return x < y ? x : y;
}
int max(int x, int y)
{
	return x > y ? x : y;
}



void fill_background(int max_x)
{
	int grass_thickness = 1, ground_thickness = 2;
	attron(COLOR_PAIR(3));
	for(int j=0;j<grass_thickness;j++){
		for(int i=0;i<max_x;i++)
			mvaddch(GROUND_Y+j+1, i, '-');
	}
	attroff(COLOR_PAIR(3));
	attron(COLOR_PAIR(4));
	for(int j=0;j<ground_thickness;j++){
		for(int i=0;i<max_x;i++)
			mvaddch(GROUND_Y+j+grass_thickness+1, i, '=');
	}
	int sky_start_y = 2, sky_thickness=3, cloud_thickness=1;
	attroff(COLOR_PAIR(4));
	attron(COLOR_PAIR(2));
	for(int j=0;j<sky_thickness;j++)
		for(int i=0;i<max_x;i++){
			mvaddch(sky_start_y+j, i, '-');
			mvaddch(sky_start_y+j+sky_thickness+cloud_thickness, i, '-');
		}
	
	attroff(COLOR_PAIR(2));
	attron(COLOR_PAIR(5));
	for(int j=0;j<cloud_thickness;j++)
		for(int i=0;i<max_x;i++)
			if(i%4)
			mvaddch(sky_start_y+j+sky_thickness, i, '=');
	attroff(COLOR_PAIR(5));


}

void initspikex(int *spike_x)
{
	int diff = 15;
	int start = 20;
	for(int i=0;i<NUM_SPIKES;i++)
	{
		spike_x[i] = start;
		start += diff;
		diff += 5;
	}
}

int is_collided(int ball_x, int ball_y, int *spike_x)
{
	int dist_from_ground;
	for(int i=0;i<NUM_SPIKES;i++){
		dist_from_ground = GROUND_Y-ball_y;
		if((dist_from_ground == 0 && ball_x >=spike_x[i] && ball_x <= spike_x[i]+4) || (dist_from_ground == 1 && ball_x >=spike_x[i]+1 && ball_x <= spike_x[i]+3) || (dist_from_ground == 2 && ball_x == spike_x[i]+2 ))
			return 1;
	}
	return 0;
}

void display_score_info(int score)
{
	char score_display[100];
	sprintf(score_display, "SCORE:%d", score);
	mvprintw(1, 0, score_display);
	mvprintw(1, 30, "BOUNCE YOUR WAY!");
	attron(COLOR_PAIR(3));
	for(int i=0;i<max_x;i++)
		mvaddch(GROUND_Y, i, '_');
	attroff(COLOR_PAIR(3));
}

int main(int argc, char *argv[]) 
{

 	srand(time(0));
 	char spike[3][10] = {{' ', ' ', '*', '\0'}, {' ', '*', ' ', '*', '\0'}, {'*', ' ', ' ', ' ','*', '\0'}};
 	initscr();
 	initcolor();
 	noecho();
	curs_set(FALSE);
	// Max number of characters in the display;
	int ball_x = 10, ball_y = GROUND_Y, max_jmp_ht = 7, is_in_air = 0;
	int ball_vel = -1;
	int spike_x[NUM_SPIKES], spike_y = GROUND_Y, spike_vel = -1, has_reached_max_ht = 0;
	initspikex(spike_x);
	int score = 0;
	getmaxyx(stdscr, max_y, max_x);
	fill_background(max_x);
	char score_display[100];
	#pragma omp parallel num_threads(2)
	{
		while(1)
		{
			int tid = omp_get_thread_num();
			if(tid == 0){
				//werase(stdscr);
				// Erase only the part of screen displaying ball movement
				for(int i=GROUND_Y-max_jmp_ht-1;i<=GROUND_Y;i++)
					for(int j=0;j<max_x;j++)
						mvaddch(i, j, ' ');
				if(!is_collided(ball_x, ball_y, spike_x)){
					getmaxyx(stdscr, max_y, max_x);

					display_score_info(score);
					
					attron(COLOR_PAIR(1));
					mvaddch(ball_y, ball_x, 'O');
					attroff(COLOR_PAIR(1));
					if(ball_y == GROUND_Y - max_jmp_ht)
						has_reached_max_ht = 1;
					
					if(ball_y == GROUND_Y && has_reached_max_ht)
					{
						is_in_air = 0;
						ball_vel = -1;
						has_reached_max_ht = 0;
					}

					if(ball_vel + ball_y >= min(max_y, GROUND_Y+1) || ball_vel + ball_y < max(0, GROUND_Y-max_jmp_ht))
						ball_vel *= -1;

					if(is_in_air)
						ball_y += ball_vel;
					attron(COLOR_PAIR(5));
					for(int i=0;i<NUM_SPIKES;i++){
						for(int j=0;j<3;j++)
							mvprintw(spike_y-j, spike_x[i], spike[2-j]);
						spike_x[i] = (spike_x[i]+spike_vel+(max_x-5))%(max_x-5);
					}
					attroff(COLOR_PAIR(5));
					usleep(DELAY);
					wrefresh(stdscr);
					score++;
					if(score % 500 == 0)
						spike_vel *= 2;
				}
				else
				{
					werase(stdscr);
					mvprintw(max_y/2, 30, "GAME OVER!");
					sprintf(score_display, "SCORE:%d", score);
					mvprintw(max_y/2+1, 31, score_display);
					wrefresh(stdscr);
					usleep(2*SEC);
					endwin();
					exit(0);
				}
			}
			else{
				char ch = getch();
				if(ch == 'w')
					is_in_air = 1;
					
				
			}

		}
	}

	endwin(); // Restore normal terminal behavior

}