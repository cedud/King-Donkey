#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define PLATFORM_HEIGHT		16
#define PLAYER_SPEED		5
#define BARREL_SPEED		2
#define NUMBER_OF_BARRELS	7
#define BARRELS_DELAY		2000
#define NUMBER_OF_LIVES		5

struct Platform {
	int x;
	int y;
	int width;
};

struct Ladder {
	int x;
	int y;
	int height;
};

struct EndZone {
	int left;
	int right;
	int up;
	int down;
};

class Level {
public:
	void init(int level)
	{
		for (int i = 0; i < 100; i++)
		{
			p[i] = { 0,0,0 };
			l[i] = { 0,0,0 };
		}
		switch (level)
		{
		case 1:
			p[0] = Platform{ 0, 480 - PLATFORM_HEIGHT, 640};
			p[1] = Platform{ 0, 380 - PLATFORM_HEIGHT, 528};
			p[2] = Platform{ 112, 280 - PLATFORM_HEIGHT, 528};
			p[3] = Platform{ 0, 180 - PLATFORM_HEIGHT, 528};
			p[4] = Platform{ 112, 80 - PLATFORM_HEIGHT, 528};
			pNo = 5;
			l[0] = Ladder{512, 364, 100};
			l[1] = Ladder{128, 264, 100};
			l[2] = Ladder{512, 164, 100};
			l[3] = Ladder{128, 64, 100};
			lNo = 4;
			end = EndZone{ 528, SCREEN_WIDTH, 0, 80 - PLATFORM_HEIGHT };
			currentLevel = 1;
			trophyX = 240;
			trophyY = 456;
			break;
		case 2:
			p[0] = Platform{ 0, 480 - PLATFORM_HEIGHT, 640 };
			p[1] = Platform{ 0, 380 - PLATFORM_HEIGHT, 320 };
			p[2] = Platform{ 384, 380 - PLATFORM_HEIGHT, 256 };
			p[3] = Platform{ 64, 280 - PLATFORM_HEIGHT, 80 };
			p[4] = Platform{ 224, 280 - PLATFORM_HEIGHT, 416 };
			p[5] = Platform{ 0, 180 - PLATFORM_HEIGHT, 336 };
			p[6] = Platform{ 416, 180 - PLATFORM_HEIGHT, 144};
			p[7] = Platform{ 96, 80 - PLATFORM_HEIGHT, 544};
			end = EndZone{ 528, SCREEN_WIDTH, 0, 80 - PLATFORM_HEIGHT };
			pNo = 8;
			l[0] = Ladder{592, 364, 100};
			l[1] = Ladder{80, 264 , 100};
			l[2] = Ladder{512, 164, 100};
			l[3] = Ladder{112, 64, 100};
			lNo = 4;
			currentLevel = 2;
			trophyX = 624;
			trophyY = 256;
			break;
		case 3:
			for (int i = 0; i < 20; i++) {
				p[i] = Platform{ i * 32, 480 - PLATFORM_HEIGHT - 2 * i, 32 };
			}
			for (int i = 20; i < 36; i++) {
				p[i] = Platform{ SCREEN_WIDTH - ((i - 20) * 32) - 128, 350 - PLATFORM_HEIGHT - 2 * i, 32 };
			}
			for (int i = 36; i < 53; i++) {
				p[i] = Platform{ ((i - 36) * 32) + 96, 200 - PLATFORM_HEIGHT - 2 * i, 32 };
			}
			pNo = 50;
			l[0] = Ladder{ 525, 298, 138 };
			l[1] = Ladder{ 110, 116, 138 +16};
			lNo = 2;
			end = EndZone{ 528, SCREEN_WIDTH, 0, 80 - PLATFORM_HEIGHT };
			currentLevel = 3;
			trophyX = 335;
			trophyY = 282 - 8;
			break;
		default:
			break;
		}
	}

	int getNo(char x) {
		if (x == 'p')
			return pNo;
		else if (x == 'l')
			return lNo;
	}
	int getLevelId() { return currentLevel; }
	int getTrophy(char c) {
		if (c == 'x')
			return trophyX;
		else if (c == 'y')
			return trophyY;
	}
	EndZone getZone() {
		return end;
	}
	Platform getPlatform(int i){
		return p[i];
	}
	Ladder getLadder(int i) {
		return l[i];
	}

private:
	Platform p[100];
	Ladder l[100];
	EndZone end;
	int trophyX, trophyY;
	int pNo, lNo, currentLevel;
};

class Barrel {
public:
	Barrel() : x(520), y(64), direction('l'), jumpedOver(0) {};
	int getX() { return x; };
	int getY() { return y; };
	void setX(int i) { x = i; }
	void setY(int i) { y = i; }
	int wasJumped(int x = 0) { 
		if (!x)
			return jumpedOver;
		else jumpedOver = x;
	};
	void move() {
		if (direction == 'l')
			x -= BARREL_SPEED;
		else if (direction == 'r')
			x+= BARREL_SPEED;

		if (!onPlatform())
			y+= BARREL_SPEED;

		if (x <= 8) {
			if (y >= 464) {
				if (x <= -8) {

					x = 520;
					y = 64;
					jumpedOver = 0;
				}
			}
			else 
				direction = 'r';
		}
		else if (x >= SCREEN_WIDTH - 8) {
			direction = 'l';
		}
	}
	void getPlatforms(Level* lvl) {
		level = lvl;
	}
	void reset() {
		x = 520;
		y = 64;
		direction = 'l';
		jumpedOver = 0;
	}
private:
	int x;
	int y;
	int jumpedOver;
	char direction;
	Level* level;
	int onPlatform() {
		for (int i = 0; i <= level->getNo('p'); i++) {
			Platform p = level->getPlatform(i);
			if ((x > p.x && x < p.x + p.width) && y == p.y)
				return 1;
		}
		return 0;
	}
};

class Game {
public:
	void init(int fullscreen) {
		if (fullscreen)
			rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
		else
			rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
		if (rc != 0) {
			SDL_Quit();
			printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
			abort();
		};

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Donkey King: A Nuntindo Game");

		screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);

		SDL_ShowCursor(SDL_DISABLE);


		charset = SDL_LoadBMP("./cs8x8.bmp");
		if (charset == NULL) {
			printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
			destroy();
		};
		SDL_SetColorKey(charset, true, 0x000000);

		pfm = SDL_LoadBMP("./pfm.bmp");
		if (pfm == NULL) {
			printf("SDL_LoadBMP(pfm.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		bar = SDL_LoadBMP("./bar.bmp");
		if (bar == NULL) {
			printf("SDL_LoadBMP(bar.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		lad = SDL_LoadBMP("./lad.bmp");
		if (lad == NULL) {
			printf("SDL_LoadBMP(lad.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		plr = SDL_LoadBMP("./plr.bmp");
		if (plr == NULL) {
			printf("SDL_LoadBMP(plr.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		tph = SDL_LoadBMP("./tph.bmp");
		if (tph == NULL) {
			printf("SDL_LoadBMP(tph.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		hrt = SDL_LoadBMP("./hrt.bmp");
		if (hrt == NULL) {
			printf("SDL_LoadBMP(hrt.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			destroy();
		};

		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			barrelSpawn[i] = -1;
		}

		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			barrels[i].reset();
		}

		czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
		zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
		czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
		niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

		t1 = SDL_GetTicks();

		posX = 20;
		posY = 464;

		frames = 0;
		fpsTimer = 0;
		fps = 0;
		quit = 0;
		worldTime = 0;
		distance = 0;
		etiSpeed = 1;
		full = fullscreen;

		jumping = 0;
		jumpEnd = 0;
		jumpStart = 0;
		trophyCollected = 0;

		running = 1;
		if (level->getLevelId() == 1) {
			savedPoints = 0;
			points = 0;
			lives = NUMBER_OF_LIVES;
		}
	}

	void destroy() {
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		abort();
	}

	void infoOld() {
		// info text
		DrawRectangle(4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen->w / 2 - strlen(text) * 8 / 2, 10, text);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		sprintf(text, "Esc - wyjscie, A - lewo, D - prawo");
		DrawString(screen->w / 2 - strlen(text) * 8 / 2, 26, text);
	}

	void info() {
		// info text
		//DrawRectangle(4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "%d points", points);
		DrawString(screen->w / 2 - strlen(text) * 8 / 2 - 260, 20, text);

		sprintf(text, "%.1lf s", worldTime);
		DrawString(screen->w / 2 - strlen(text) * 8 / 2 + 250, 10, text);

		sprintf(text, "%dX, %dY", posX,posY);
		DrawString(screen->w / 2 - strlen(text) * 8 / 2 + 250, 20, text);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		//DrawString(screen->w / 2 - strlen(text) * 8 / 2, 26, text);
	}

	void render() {
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	void free() {
		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			barrels[i].reset();
			barrelSpawn[i] = -1;
		}
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void setLevel(Level* lvl) {
		level = lvl;
		for (int i = 0; i < NUMBER_OF_BARRELS; i++)
			barrels[i].getPlatforms(level);
	}

	int isRunning() { return running; };

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
	void DrawString(int x, int y, const char* text) {
		int px, py, c;
		SDL_Rect s, d;
		s.w = 8;
		s.h = 8;
		d.w = 8;
		d.h = 8;
		while (*text) {
			c = *text & 255;
			px = (c % 16) * 8;
			py = (c / 16) * 8;
			s.x = px;
			s.y = py;
			d.x = x;
			d.y = y;
			SDL_BlitSurface(charset, &s, screen, &d);
			x += 8;
			text++;
		};
	};

// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
	void DrawSurface(SDL_Surface* sprite, int x, int y) {
		SDL_Rect dest;
		dest.x = x - sprite->w / 2;
		dest.y = y - sprite->h / 2;
		dest.w = sprite->w;
		dest.h = sprite->h;
		SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

	// draw a single pixel
	void DrawPixel(int x, int y, Uint32 color) {
		int bpp = screen->format->BytesPerPixel;
		Uint8* p = (Uint8*)screen->pixels + y * screen->pitch + x * bpp;
		*(Uint32*)p = color;
	};

	// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
	void DrawLine(int x, int y, int l, int dx, int dy, Uint32 color) {
		for (int i = 0; i < l; i++) {
			DrawPixel(x, y, color);
			x += dx;
			y += dy;
		};
	};

	// draw a rectangle of size l by k
	void DrawRectangle(int x, int y, int l, int k,
		Uint32 outlineColor, Uint32 fillColor) {
		int i;
		DrawLine(x, y, k, 0, 1, outlineColor);
		DrawLine(x + l - 1, y, k, 0, 1, outlineColor);
		DrawLine(x, y, l, 1, 0, outlineColor);
		DrawLine(x, y + k - 1, l, 1, 0, outlineColor);
		for (i = y + 1; i < y + k - 1; i++)
			DrawLine(x + 1, i, l - 2, 1, 0, fillColor);
	};

	void changeLevel(int i) {
		//this->free();
		SDL_DestroyWindow(window);
		level->init(i);
		this->init(full);
	}

	void SpawnBarrels() {
		if (barrelSpawn[0] == -1) {
			barrelSpawn[0] = t2;
			for (int i = 1; i < NUMBER_OF_BARRELS; i++) {
				barrelSpawn[i] = barrelSpawn[i - 1] + BARRELS_DELAY;
			}
		}
	}

	int win() {
		EndZone end = level->getZone();
		if (posX >= end.left && posX <= end.right && posY >= end.up && posY <= end.down)
			return 1;
		return 0;
	}

	int dead() {
		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			int bx = barrels[i].getX();
			int by = barrels[i].getY();
			if (((posX - 8 >= bx - 8 && posX - 8 <= bx + 8)			// left side
				|| (posX + 8 >= bx - 8 && posX + 8 <= bx + 8))		// right side
				&& ((posY >= by - 16 && posY <= by)					// up
				|| (posY - 32 >= by - 32 && posY - 32 <= by))){		// down
				lives--;
				return 1;
			}
			if (lives < 1) {
				//free();
				running = 0;
			}
		}
		return 0;
	}

	void Points() {
		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			int bx = barrels[i].getX();
			int by = barrels[i].getY();
			if (!barrels[i].wasJumped() && !onLadder(posY) && (posX >= bx - 8 && posX <= bx + 8)) {
				if (by >= posY && by <= posY + 100) {
					points += 100;
					barrels[i].wasJumped(1);
					DrawString(posX - 8, posY - 48, "100");
				}
			}
		}
		int tx = level->getTrophy('x');
		int ty = level->getTrophy('y');
		if	(!trophyCollected &&
			(((posX - 8 >= tx - 8 && posX - 8 <= tx + 8)		// left side
			|| (posX + 8 >= tx - 8 && posX + 8 <= tx + 8))		// right side
			&& ((posY >= ty - 16 && posY <= ty)					// up
			|| (posY - 32 >= ty - 32 && posY - 32 <= ty)))) {	// down
			
			points += 500;
			DrawString(posX - 8, posY - 48, "500");
			trophyCollected = 1;
		}
	}

	void DrawBarrels() {
		for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
			int bx = barrels[i].getX();
			int by = barrels[i].getY();
			if (by == 64 && bx == 520 && currentLevel == 3) {
				barrels[i].setX(460);
				barrels[i].setY(94);
			}
			if (t2 >= barrelSpawn[i]){
				barrels[i].move();
				DrawSurface(bar, bx, by  - 8);
			}
		}
	}

	int menu(int fullscreen) {
		if (running == 1)free();
		init(fullscreen);
		DrawString(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 10,
			"N     - Nowa gra");
		DrawString(SCREEN_WIDTH / 2 - 70, (SCREEN_HEIGHT / 2),
			"1,2,3 - Wybor poziomu");
		DrawString(SCREEN_WIDTH / 2 - 70, (SCREEN_HEIGHT / 2) + 10,
			"G     - Tabela wynikow");
		DrawString(SCREEN_WIDTH / 2 - 70, (SCREEN_HEIGHT / 2) + 20,
			"ESC   - Wyjdz z gry");
		DrawString(400, SCREEN_HEIGHT,
			"ESC   - Wyjdz z gry");
		render();
		while (1) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) return 0;
					else if (event.key.keysym.sym == SDLK_n) return 1;
					else if (event.key.keysym.sym == SDLK_1) {changeLevel(1); return 1; }
					else if (event.key.keysym.sym == SDLK_2) {changeLevel(2); return 1; }
					else if (event.key.keysym.sym == SDLK_3) {changeLevel(3); return 1; }
					else if (event.key.keysym.sym == SDLK_g) {leaderboard(); }
				}
			}
		}
	}


	void update() {
		t2 = SDL_GetTicks();
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		SpawnBarrels();
		worldTime += delta;

		distance += etiSpeed * delta;

		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		};

		DrawBarrels();
		Points();

		if (!trophyCollected) {
			DrawSurface(tph, level->getTrophy('x'), level->getTrophy('y'));
		}

		if (jumping)
			jumpPhysics();

		if(!jumping || (onLadder(posY) && jumping)) fall();

		if (dead()) {
			if (lives >= 1) {
				DrawString(SCREEN_WIDTH / 2 - 110, (SCREEN_HEIGHT / 2),
					"Chcesz kontynuowac? (T)ak (N)ie");
				render();
				trophyCollected = 0;
				points = savedPoints;
				while (1) {
					if (SDL_PollEvent(&event)) {
						switch (event.type) {
						case SDL_KEYDOWN:
							if (event.key.keysym.sym == SDLK_t) {
								running = 1;
								posX = 15;
								posY = 464;
								for (int i = 0; i < NUMBER_OF_BARRELS; i++) {
									barrels[i].reset();
									barrelSpawn[i] = -1;
								}
								return;
							}
							else if (event.key.keysym.sym == SDLK_n) {
								running = 0;
								return;
							}

						};
					}
				}
			}
			else running = 0;
		}

		if (win()) {
			printf("WYGRALES!!!\n");
			points += 1000;
			savedPoints = points;
			if (level->getLevelId() == 3) {
				while (1) {
					DrawString(SCREEN_WIDTH / 2 - 110, (SCREEN_HEIGHT / 2),
						"GRATULACJE! WYGRALES!");
					render();
				}
			}
			changeLevel(level->getLevelId() + 1);
			
		}
		 
		DrawSurface(plr, posX, posY - 15);

		SDL_Delay(floor(16.666f - delta));

	}

	void DrawLevel() {
		SDL_FillRect(screen, NULL, czarny);
		for (int i = 0; i <= level->getNo('p'); i++) {
			Platform p = level->getPlatform(i);
			//DrawLine(p.x, p.y - 1, p.width, 1, 0, zielony);
			for (int i = 8; i <= p.width; i += 16)
				DrawSurface(pfm, p.x + i, p.y + 8);
		}
		for (int i = 0; i <= level->getNo('l'); i++) {
			Ladder l = level->getLadder(i);
			//DrawLine(l.x, l.y, l.height, 0, 1, zielony);
			for (int i = 0; i <= l.height; i += 10)
				DrawSurface(lad, l.x, l.y + i);
		}
		for (int i = 0; i < lives; i++) {
			DrawSurface(hrt, 16 * i + 8, 8);
		}
	}

	void leaderboard() {
		DrawString(SCREEN_WIDTH / 2 - 70, (SCREEN_HEIGHT / 2) + 40,
			"Funkcja niezaimplementowana");
		render();
	}

	void jump() {
		if (!inAir() && !onLadder(posY-1)) {
			posY -= 1;
			jumpStart = t2;
			jumpEnd = jumpStart + 700;
			jumpHalf = jumpStart + 350;
			jumping = 1;
		}
		else
			return;
	}

	void fall() {
		if (inAir()) {
			printf("SPADASZ\n");
			posY += 1;
		}
	}

	void jumpPhysics() {
		if (jumping) {
			Uint32 currentTicks = SDL_GetTicks();
			printf("worldTime: %.2lf, jumpStart: %.2lf, jumpHalf: %.2lf, jumpEnd: %.2lf\n",
				worldTime, jumpStart, jumpHalf, jumpEnd);
			if (!inAir() ) {
				jumping = 0;
				printf("KONIEC SKOKU\n");
			}
			else if (currentTicks <= jumpHalf) {
				posY -= 2;
				printf("SKACZESZ\n");
			}
			else{
				fall();
			}
		}
	}

	int inAir() {
		if (!onLadder(posY) && !onPlatform(posX)) {
			return 1;
		}
		return 0;
	}

	int onLadder(int y){
		for (int i = 0; i <= level->getNo('l'); i++)
		{
			Ladder l = level->getLadder(i);
			if (posX > l.x - 10 && posX < l.x + 10)
			{ 
				if (y >= l.y && y <= l.y + l.height)
					return 1;
			}
		}
		return 0;
	}

	int onPlatform(int x, int y = 0) {
		if (!y) { y = posY; };
		for (int i = 0; i <= level->getNo('p'); i++)
		{
			Platform p = level->getPlatform(i);
			if (y == p.y) {
				if (x >= p.x - 8 && x <= p.x + p.width + 8 &&  x > 0 && x < SCREEN_WIDTH) {
					return 1;
				}
			}
		}
		return 0;
	}

	void handleEvents() {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
				else if (event.key.keysym.sym == SDLK_a && (posX - PLAYER_SPEED) > 0) {
					posX -= PLAYER_SPEED;
					if (onPlatform(posX, posY - 2))
						posY -= 2;
					else if (onPlatform(posX, posY + 2))
						posY += 2;
				}
				else if (event.key.keysym.sym == SDLK_d && (posX + PLAYER_SPEED) < SCREEN_WIDTH) {
					posX += PLAYER_SPEED;
					if (onPlatform(posX, posY - 2))
						posY -= 2;
					else if (onPlatform(posX, posY + 2))
						posY += 2;
				}
				else if (event.key.keysym.sym == SDLK_w && onLadder(posY - 1)) posY -= PLAYER_SPEED;
				else if (event.key.keysym.sym == SDLK_s) {
					if (onLadder(posY + PLAYER_SPEED))
						posY += PLAYER_SPEED;
					else if (onLadder(posY + 1))
						posY += 1;
				}
				else if (event.key.keysym.sym == SDLK_LEFT) posX -= 100;
				else if (event.key.keysym.sym == SDLK_RIGHT) posX += 100;
				else if (event.key.keysym.sym == SDLK_UP) posY -= 100;
				else if (event.key.keysym.sym == SDLK_DOWN) posY += 100;
				else if (event.key.keysym.sym == SDLK_SPACE) jump();
				else if (event.key.keysym.sym == SDLK_1) changeLevel(1);
				else if (event.key.keysym.sym == SDLK_2) changeLevel(2);
				else if (event.key.keysym.sym == SDLK_3) changeLevel(3);
				break;
			/*case SDL_KEYUP:
				break;*/
			case SDL_QUIT:
				running = 0;
				break;
			};
		};
		frames++;
	};
private:
	int t1, t2, quit, frames, points, savedPoints, full, rc, running, posX, posY, trophyCollected, jumping, lives, currentLevel;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed, jumpStart, jumpHalf, jumpEnd;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* eti;
	SDL_Surface* pfm;
	SDL_Surface* lad;
	SDL_Surface* plr;
	SDL_Surface* bar;
	SDL_Surface* tph;
	SDL_Surface* hrt;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	Level* level;
	Barrel barrels[NUMBER_OF_BARRELS];
	int barrelSpawn[NUMBER_OF_BARRELS];

	char text[128];
	int czarny, zielony, czerwony, niebieski;
};


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	Game game;
	Level level;
	level.init(1);
	game.setLevel(&level);
	printf("wyjscie printfa trafia do tego okienka\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	
	int escape = 0;
	int fullscreen = 0;

	while (game.menu(fullscreen)) {
		while (game.isRunning()) {
			game.DrawLevel();
			//game.infoOld();
			game.info();
			game.handleEvents();
			game.update();
			game.render();
		}
		game.free();
	}
	
	//game.free();

	return 0;
}