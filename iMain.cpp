# include "iGraphics.h"
# include <stdlib.h>
# include <time.h>
 
int screen_height, screen_width, window_height, window_width;

int grid_initialized = 0;
double start_x = 50, start_y = 50, size = 50, padding = 4, text_offset = 10;
int rows = 10, cols = 15; // 10, 15 before
int mine_count = 20, page = 0, marked_counter = 0;
int life_height = 55+40; // image + offset

int max_lives = 9;
int lives = max_lives; 

//function iDraw() is called again and again by the system.

int di[] = {-1, -1, -1,  0,  0,  1, 1, 1};
int dj[] = {-1,  0, +1, +1, -1, -1, 0, 1};

struct Effect{
	char sound_path[50];
	char image_path[50];
	double time;

	Effect(char sp[], char ip[], double t){
		strcpy(sound_path, sp);
		strcpy(image_path, ip);
		time = t;
	}
};

Effect effects[] = {Effect("gah_sound.wav", "gah.bmp", 1.3), Effect("explosion_3.wav", "explosion.bmp", 2.5)};

struct Cell
{
	int uncovered = 0, mine = 0, clue = 0, marker = 0, sound_played = 0, sound_started = 0, chosen_effect = -1, sound_start_time = 0;
	int left_x, left_y, sz;

	Cell(double xx = 0, double yy = 0, double size = 0){
		left_x = xx;
		left_y = yy;
		sz = size;		
	}

	void draw_cell(){
		if(uncovered){
			iRectangle(left_x, left_y, sz, sz);
			if(mine){
				iShowBMP(left_x + (size - 24 - padding)/2, left_y + (size - 24 - padding)/2, "mine.bmp");

				if( !sound_started) {
					PlaySound(TEXT(effects[chosen_effect].sound_path), NULL, SND_ASYNC);
					sound_started = 1;
					marked_counter++;
					lives--;
					sound_start_time = time(NULL);
				}else{
					if(time(NULL) - sound_start_time <= effects[chosen_effect].time){
						iShowBMP(window_width - 510, window_height/4, effects[chosen_effect].image_path);
					}
				}

			}else{
				if(clue != 0){
					char s[] = "0";
					s[0] += clue;
					iText(left_x + (size - padding - text_offset)/2.0, left_y + (size - padding - text_offset)/2.0, s, GLUT_BITMAP_HELVETICA_18);
				}
			}
		}else{
			iFilledRectangle(left_x, left_y, sz, sz);

			if(marker){
				iShowBMP(left_x + (size - padding - 20)/2.0, left_y + (size - padding -23)/2.0,"flag.bmp");
			}
		}
	}
};

struct Pair
{
	int x, y;
	Pair(int xx = 0, int yy = 0){
		x = xx;
		y = yy;
	}

	void show(){
		printf("x = %d and y = %d\n", x, y);
	}
};


Cell cells[50][50];
bool played[50][50];
Pair mines[2500];

int compress_x_to_j(double x){
	x -= start_x;
	x /= size;
	return (int) x;
}
int compress_y_to_i(double y){
	y -= start_y;
	y /= size;
	return (int) y;
}

void shuffle_mines(){
	int sz = rows*cols;
    for(int i = 0; i < sz - 1; i++){
		size_t j = i + rand() / (RAND_MAX / (sz - i) + 1);
		Pair t = mines[j];
		mines[j] = mines[i];
		mines[i] = t;
    }
}

int get_clue(int i, int j){
	int res = 0;
	for(int idx = 0; idx < 8; idx++){
		int newi = i + di[idx], newj = j + dj[idx];

		if(newi < 0 || newi >= rows || newj < 0 || newj >= cols) continue;
		else{
			if(cells[newi][newj].mine) res++;
		}
	}
	return res;
}

void clear_zeros(int i , int j){
	if(cells[i][j].uncovered) return;

	if(cells[i][j].clue == 0){
		cells[i][j].uncovered = 1;

		for(int idx = 0; idx < 8; idx++){
			int newi = i + di[idx], newj = j + dj[idx];

			if(newi < 0 || newi >= rows || newj < 0 || newj >= cols) continue;
			else clear_zeros(newi, newj);
		}
	}
}

void clear_around_zeros(){
	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			if(cells[i][j].uncovered == 1 && cells[i][j].clue == 0 && cells[i][j].mine == 0){

				for(int idx = 0; idx < 8; idx++){
					int newi = i + di[idx], newj = j + dj[idx];
					if(newi < 0 || newi >= rows || newj < 0 || newj >= cols) continue;
					cells[newi][newj].uncovered = 1;
				}
			}
		}
	}
	
}


void draw_grid(){
	// iLine take double as input

	if(!grid_initialized){
		for(int i = 0; i < rows; i++){
			for(int j = 0; j < cols; j++){
				cells[i][j] = Cell(start_x + j*size, start_y + i*size, size - padding);
			}
		}

		for(int i = 0; i < rows; i++){
			for(int j = 0; j < cols; j++){
				mines[j + i*cols] = Pair(i, j);
			}
		}

		shuffle_mines();
		for(int i = 0; i < mine_count; i++){
			cells[ mines[i].x ][ mines[i].y ].mine = 1; 
			cells[ mines[i].x ][ mines[i].y ].chosen_effect = rand() % 2;
		}

		for(int i = 0; i < rows; i++){
			for(int j = 0; j < cols; j++){
				if(cells[i][j].mine ) continue;

				cells[i][j].clue = get_clue(i, j);	
			}
		}
		grid_initialized = 1;
		return;
	}
	
	

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			cells[i][j].draw_cell();
		}
	}	
}

void side_stuff(){

	char buffer[50];
	sprintf(buffer, "MINES LEFT : %d", mine_count - marked_counter);
	

	iText(window_width - 250, window_height - 75, buffer, GLUT_BITMAP_HELVETICA_18);

	int life_x_offset = 50, life_width = 60, life_spacing = 50; // HARDCODED. MIGHT HAVE TO EDIT
	for(int i = 0; i < lives; i++){ 
		// iShowBMP(0 + life_x_offset, window_height - life_height - life_y_offset, "heart.bmp");
		iShowBMP(0 + life_x_offset, window_height - life_height, "heart.bmp");
		life_x_offset += life_width + life_spacing;
	}
	for(int i = 0; i < (max_lives - lives); i++){
		iShowBMP(0 + life_x_offset, window_height - life_height, "broken.bmp");
		life_x_offset += life_width + life_spacing;
	}

}

void board_dimensions(int p){
	if(p == 5){
		rows = 9; cols = 9; mine_count = 10; size = 50;
	}else if(p == 6){
		rows = 16, cols = 16, mine_count = 40, size = 40;
	}else if(p == 7){
		rows = 16; cols = 30; mine_count = 99; size = 30;
	}
	start_x = (window_width - cols*size - 510)/2.0;
	start_y = (window_height - rows*size - life_height)/2.0;
}

void event_handler(){
	if(lives == 0){
		page = 2; // 2 for loss
		return;
	}
	if(marked_counter == mine_count){
		page = 3; // 3 for win
		return;
	}
}

void iDraw() {

	//place your drawing codes here
	iClear();
	event_handler();
	if(page == 0){
		iShowBMP(0, 0, "homepage.bmp");
	}else if(page == 1){
		iSetColor(200, 20, 0);
		draw_grid();
		side_stuff();
	}else if(page == 2){
		iShowBMP(0, 0, "lost_page.bmp");
	}else if(page == 3){
		iShowBMP(0, 0, "win_page.bmp");
	}else if(page == 4){
		iShowBMP(0, 0, "levels.bmp");
	}else if(page == 5){
		iShowBMP(0, 0, "levels_beginner.bmp");
	}else if(page == 6){
		iShowBMP(0, 0, "levels_intermediate.bmp");
	}else if(page == 7){
		iShowBMP(0, 0, "levels_expert.bmp");
	}
}

void iMouseMove(int mx, int my) {
	// printf("%d %d\n", mx, my);

}



void iMouse(int button, int state, int mx, int my) {
	// printf("%d %d\n", mx, my);

	if(page >= 4){
		if(mx >= 82 && mx <= 348 && my >= 340 && my <= 586){
			page = 5;
		}else if(mx >= 464 && mx <= 812 && my >= 340 && my <= 586){
			page = 6;
		}else if(mx >= 935 && mx <= 1195 && my >= 340 && my <= 586){
			page = 7;
		}else if(mx >= 493 && mx <= 760 && my >= 137 && my <= 210){
			board_dimensions(page);
			page = 1;
		}
	}

	int i = compress_y_to_i(my);
	int j = compress_x_to_j(mx);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

		if(0 <= i && i < rows && 0 <= j && j < cols){	
			if(cells[i][j].clue == 0 && !cells[i][j].mine) {
				clear_zeros(i, j);
				clear_around_zeros();
			}
			cells[i][j].uncovered = 1; // i = y aixs, j is x axis
		}
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if(0 <= i && i < rows && 0 <= j && j < cols && !cells[i][j].uncovered){
			cells[i][j].marker ^= 1;

			if(cells[i][j].marker) marked_counter++;
			else marked_counter--;
		}
	}
}

void iKeyboard(unsigned char key) {
	if (key == 'q') {
		exit(0);
	}

	if(key == '\r' && page == 0){
		page = 4; // to levels
	}
}


void iSpecialKeyboard(unsigned char key) {

	if (key == GLUT_KEY_END) {
		exit(0);
	}
}


int main() {
	//place your own initialization codes here.

 	screen_height = GetSystemMetrics(SM_CYSCREEN);
    screen_width = GetSystemMetrics(SM_CXSCREEN);

	// window_height = (screen_height*3)/4;
	// window_width = (screen_width*3)/4;
	window_height = 790;
	window_width = 1440;

	iInitialize(window_width, window_height, "Minesweeper");
	return 0;
}