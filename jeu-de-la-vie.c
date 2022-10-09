#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define FREE freeMemory(window, renderer)


//par argument
int NB_CELL_HEIGHT;
int NB_CELL_WIDTH;
char NOM_FICHIER[30];
int estTorique;
int RULES[2][9] = {{0, 0, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 1, 1 ,0, 0, 0, 0, 0}};


//calculées
int CELL_SIZE;
int SCREEN_WIDTH = 1920; //default
int SCREEN_HEIGHT = 1000; //default
int SIZE_SNAPSHOT;

//CONSTANTES
int const TAILLE_MENU = 300;
SDL_Color const ALIVE_COLOR = {0, 255, 0, 255};
SDL_Color const DEAD_COLOR = {100, 100, 100, 255};
SDL_Color const MENU_COLOR = {125, 200, 214, 255};
int TAILLE_BOUCLE = 15;


enum {
    naiss_0, surv_0,
    naiss_1, surv_1,
    naiss_2, surv_2,
    naiss_3, surv_3,
    naiss_4, surv_4,
    naiss_5, surv_5,
    naiss_6, surv_6,
    naiss_7, surv_7,
    naiss_8, surv_8,
    play_pause, fini, torique
};

enum {
    x, y, taille
};


int detectionCycle(char snapshots[TAILLE_BOUCLE][SIZE_SNAPSHOT]){
	int res = 0;
	for (int i=1;i<TAILLE_BOUCLE; i++){
		if(strcmp(snapshots[0], snapshots[i])==0){
			res = i;
			break;
		}
	}
	return res;
}

int car2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}


void afficherMonde(int **monde){
    for(int i = 0; i < NB_CELL_HEIGHT; i++) {
        for(int j = 0; j < NB_CELL_WIDTH; j++) {
			printf("%d", monde[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}


int** initMonde(){
	int** tab = (int**)malloc(NB_CELL_HEIGHT*sizeof(int*));
	for (int i=0; i<NB_CELL_HEIGHT;i++){
		tab[i] = (int*)calloc(NB_CELL_WIDTH, sizeof(int));
	}
	return tab;
}
int** freeMonde(int **monde){
	for (int i=0; i<NB_CELL_HEIGHT;i++){
		free(monde[i]);
	}
	free(monde);
	return NULL;
}

void makeSnapshot(int **monde, char snapshots[TAILLE_BOUCLE][SIZE_SNAPSHOT]){
	char str[SIZE_SNAPSHOT];
    memset(str,'\0',SIZE_SNAPSHOT*sizeof(char));
	char tmp[10];
	int num = 0;
	int etape=0;
    for(int i = 0; i < NB_CELL_HEIGHT; i++) {
        for(int j = 0; j < NB_CELL_WIDTH; j++) {
			num = (num<<1)|monde[i][j];
			if (etape % 4 == 3){
				sprintf(tmp, "%X",num);
				str[etape/4] = tmp[0];
				num=0;
			}
			etape++;
		}
	}
	while(etape % 4 !=0){
		num= (num<<1)|0;
		if(etape %4 == 3){
			sprintf(tmp, "%X",num);
			str[etape/4] = tmp[0];
		}
		etape++;
	}

	//RANGER LA SNAPSHOT
	for(int i=TAILLE_BOUCLE-1; i>0; i--){
		strcpy(snapshots[i],snapshots[i-1]);
	}
	strcpy(snapshots[0], str);
}	

void calculConstantes(){
    CELL_SIZE = (int)fmin(SCREEN_WIDTH/NB_CELL_WIDTH, SCREEN_HEIGHT/NB_CELL_HEIGHT);
    SCREEN_HEIGHT = CELL_SIZE*NB_CELL_HEIGHT;
    SCREEN_WIDTH = CELL_SIZE*NB_CELL_WIDTH + TAILLE_MENU;
	SIZE_SNAPSHOT = NB_CELL_HEIGHT*NB_CELL_WIDTH/4+2;
}

void chargement(int ***monde){
	FILE* fichier = NULL;
    fichier = fopen(NOM_FICHIER, "r");
	if (fichier == NULL){
		fprintf(stderr, "ERREUR OUVERTURE FICHIER SAUVEGARDE");
		exit(1);
	}

	fscanf(fichier, "%d", &NB_CELL_WIDTH);	
	fscanf(fichier, "%d", &NB_CELL_HEIGHT);	
	fscanf(fichier, "%d", &estTorique);
	char rules[2][9];	
	fscanf(fichier, "%s", rules[0]);	
	fscanf(fichier, "%s", rules[1]);
	for (int i=0;i<9;i++){
		RULES[0][i] = car2int(rules[0][i]);
		RULES[1][i] = car2int(rules[1][i]);
	}
	calculConstantes();
	*monde = initMonde();
	char data[SIZE_SNAPSHOT];
	int tab[4];
	int etape=0, tmp;
	fscanf(fichier, "%s", data);
	for (int i=0; i<SIZE_SNAPSHOT-2; i++){
		tmp = car2int(data[i]);
		for (int j=0;j<4;j++){
			tab[j] = tmp%2;
			tmp/=2;
		}
		for (int j=3;j>=0;j--){
			(*monde)[etape/NB_CELL_WIDTH][etape%NB_CELL_WIDTH] = tab[j];
			etape++;
		}
	}
	int max = NB_CELL_WIDTH*NB_CELL_HEIGHT-etape;
	tmp=car2int(data[SIZE_SNAPSHOT-2]);
	for (int j=0;j<4;j++){
		tab[j] = tmp%2;
		tmp/=2;
	}
	for (int i=0; i<max;i++){
		(*monde)[etape/NB_CELL_WIDTH][etape%NB_CELL_WIDTH] = tab[3-i];
		etape++;
	}
	fclose(fichier);
}



void sauvegarder(char snapshots[TAILLE_BOUCLE][SIZE_SNAPSHOT]){

	FILE* fichier = NULL;
    fichier = fopen(NOM_FICHIER, "w");
	if (fichier == NULL){
		fprintf(stderr, "ERREUR OUVERTURE FICHIER SAUVEGARDE");
		exit(1);
	}

	char rules[20];
	for (int i=0;i<2;i++){
		for(int j =0;j<9;j++){
			rules[i*10+j]=(RULES[i][j])?'1':'0';
		}
		rules[9]=' ';
	}
	rules[19]='\0';
	fprintf(fichier,"%d %d %d %s\n",NB_CELL_WIDTH, NB_CELL_HEIGHT, estTorique, rules);
	fprintf(fichier,"%s\n",snapshots[0]); //La dernière
	printf("sauvegardé !\n");

	fclose(fichier);
}

void freeMemory(SDL_Window * window, SDL_Renderer * renderer){
	if (renderer != NULL){
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window != NULL){
		SDL_DestroyWindow(window);
		window = NULL;
	}
}


void copierTableau(int **src, int **dest) {
    for(int i = 0; i < NB_CELL_HEIGHT; i++) {
        for(int j = 0; j < NB_CELL_WIDTH; j++) {
            dest[i][j] = src[i][j];
        }
    }    
}

void dessinerCellule(SDL_Renderer *renderer, int xPos, int yPos, int state) {
    SDL_Rect rect = {xPos, yPos, CELL_SIZE, CELL_SIZE};
    SDL_Color couleur = (state)?ALIVE_COLOR:DEAD_COLOR;
    SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, couleur.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

int nombreDeVoisinsFini(int **monde, int i, int j) {
    int nb = 0;
    if(i == 0) {
        if(j == 0) {
            nb += monde[i+1][j];
            nb += monde[i+1][j+1];
            nb += monde[i][j+1];
        }
        else if(j == NB_CELL_WIDTH - 1) {
            nb += monde[i][j-1];
            nb += monde[i+1][j];
            nb += monde[i+1][j-1];
        }
        else {
            nb += monde[i+1][j-1];
            nb += monde[i+1][j];
            nb += monde[i+1][j+1];
            nb += monde[i][j-1];
            nb += monde[i][j+1];
        }
    }
    else if(i == NB_CELL_HEIGHT - 1) {
        if(j == 0) {
            nb += monde[i-1][j];
            nb += monde[i-1][j+1];
            nb += monde[i][j+1];
        }
        else if(j == NB_CELL_WIDTH - 1) {
            nb += monde[i][j-1];
            nb += monde[i-1][j];
            nb += monde[i-1][j-1];
        }
        else {
            nb += monde[i-1][j-1];
            nb += monde[i-1][j];
            nb += monde[i-1][j+1];
            nb += monde[i][j-1];
            nb += monde[i][j+1];
        }
    }
    else {
        if(j == 0) {
            nb += monde[i+1][j+1];
            nb += monde[i][j+1];
            nb += monde[i-1][j+1];
            nb += monde[i+1][j];
            nb += monde[i-1][j];
        }
        else if(j == NB_CELL_WIDTH - 1) {
            nb += monde[i+1][j-1];
            nb += monde[i][j-1];
            nb += monde[i-1][j-1];
            nb += monde[i+1][j];
            nb += monde[i-1][j];
        }
        else {
            nb += monde[i+1][j-1];
            nb += monde[i][j-1];
            nb += monde[i-1][j-1];
            nb += monde[i+1][j];
            nb += monde[i-1][j];
            nb += monde[i+1][j+1];
            nb += monde[i][j+1];
            nb += monde[i-1][j+1];
        }
    }
    return nb;
}

int mod(int a, int b) {
    int tmp = a%b;
    if(tmp<0)
        tmp+=b;
    return tmp;
}

int nombreDeVoisinsTorique(int **monde, int i, int j) {
    int nb = 0;
    nb += monde[mod(i-1, NB_CELL_HEIGHT)][mod(j-1, NB_CELL_WIDTH)];
    nb += monde[mod(i-1, NB_CELL_HEIGHT)][j];
    nb += monde[mod(i-1, NB_CELL_HEIGHT)][mod(j+1, NB_CELL_WIDTH)];
    nb += monde[i][mod(j-1, NB_CELL_WIDTH)];
    nb += monde[i][mod(j+1, NB_CELL_WIDTH)];
    nb += monde[mod(i+1, NB_CELL_HEIGHT)][mod(j-1, NB_CELL_WIDTH)];
    nb += monde[mod(i+1, NB_CELL_HEIGHT)][j];
    nb += monde[mod(i+1, NB_CELL_HEIGHT)][mod(j+1, NB_CELL_WIDTH)];

    return nb;
}

void updateCellules(int **monde) {
    int **temp = initMonde();
    int nbVoisins;
    copierTableau(monde, temp);
    for(int i = 0; i < NB_CELL_HEIGHT; i++) {
        for(int j = 0; j < NB_CELL_WIDTH; j++) {
            nbVoisins = estTorique?nombreDeVoisinsTorique(temp, i, j):nombreDeVoisinsFini(temp, i, j);
            monde[i][j] = RULES[monde[i][j]][nbVoisins];
        }
    }
	freeMonde(temp);
}

void afficherCellules(SDL_Renderer *renderer, int **monde) {
    for(int i = 0; i < NB_CELL_HEIGHT; i++) {
        for(int j = 0; j < NB_CELL_WIDTH; j++) {
            dessinerCellule(renderer, j*CELL_SIZE, i*CELL_SIZE, monde[i][j]);
        }
    }
}


void placerBouton(SDL_Renderer * renderer, SDL_Rect * rect, SDL_Color couleur) {
    SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, couleur.a);
    SDL_RenderFillRect(renderer, rect);
    couleur = (SDL_Color){0, 0, 0, 255};
    SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, couleur.a);
    SDL_RenderDrawRect(renderer, rect);
}

void afficherTexte(SDL_Renderer * renderer, SDL_Rect * rect, SDL_Color couleur, const char * texte, TTF_Font * font, SDL_Surface * surface, SDL_Texture * texture) {
    surface = TTF_RenderUTF8_Blended(font, texte, couleur);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, rect);
    SDL_DestroyTexture(texture);
}

void afficherMenu(SDL_Renderer *renderer, int const COORD_BOUTONS[21][3], int pause) {
    SDL_Rect rect = {SCREEN_WIDTH - TAILLE_MENU, 0, TAILLE_MENU, SCREEN_HEIGHT};
    SDL_Color couleur = MENU_COLOR;
    TTF_Font * font = NULL;
    SDL_Surface * surface = NULL;
    SDL_Texture * texture = NULL;
    char texteNbVoisins[2];

    /* Affichage du fond du menu */
    SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, couleur.a);
    SDL_RenderFillRect(renderer, &rect);

    /* Affichage du titre */
    font = TTF_OpenFont("sources/HWYGOTH.TTF", 128);
    rect = (SDL_Rect){SCREEN_WIDTH - TAILLE_MENU, 0, TAILLE_MENU, SCREEN_HEIGHT/10};
    couleur = (SDL_Color){0, 0, 0, 255};
    afficherTexte(renderer, &rect, couleur, "Paramètres", font, surface, texture);

    /* Affichage bouton play / pause */
    rect = (SDL_Rect){COORD_BOUTONS[play_pause][x], COORD_BOUTONS[play_pause][y], COORD_BOUTONS[play_pause][taille], COORD_BOUTONS[play_pause][taille]};
    couleur = (SDL_Color){255, 255, 255, 255};
    placerBouton(renderer, &rect, couleur);

    if(pause) {
        surface = IMG_Load("sources/play_button.png");
        if(surface) {
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
        }
    }
    else {
        rect.x += 10;
        rect.y += 10;
        rect.w = 10;
        rect.h = 30;
        couleur = (SDL_Color){0, 0, 0, 255};
        SDL_SetRenderDrawColor(renderer, couleur.r, couleur.g, couleur.b, couleur.a);
        SDL_RenderFillRect(renderer, &rect);
        rect.x += 20;
        SDL_RenderFillRect(renderer, &rect);
    }

    /* Affichage des boutons de choix monde fini / torique */
    // Bouton monde fini
    rect = (SDL_Rect){COORD_BOUTONS[fini][x], COORD_BOUTONS[fini][y], COORD_BOUTONS[fini][taille], COORD_BOUTONS[fini][taille]};
    couleur = estTorique?(SDL_Color){255, 0, 0, 255}:(SDL_Color){0, 255, 0, 255};
    placerBouton(renderer, &rect, couleur);

    // Affichage texte
    rect = (SDL_Rect){SCREEN_WIDTH - 0.9*TAILLE_MENU, SCREEN_HEIGHT*0.3 - 30, TAILLE_MENU/3, 30};
    couleur = (SDL_Color){0, 0, 0, 255};
    afficherTexte(renderer, &rect, couleur, "Monde fini", font, surface, texture);

    // Bouton monde torique
    rect = (SDL_Rect){COORD_BOUTONS[torique][x], COORD_BOUTONS[torique][y], COORD_BOUTONS[torique][taille], COORD_BOUTONS[torique][taille]};
    couleur = estTorique?(SDL_Color){0, 255, 0, 255}:(SDL_Color){255, 0, 0, 255};
    placerBouton(renderer, &rect, couleur);

    // Affichage texte
    rect = (SDL_Rect){SCREEN_WIDTH - 0.48*TAILLE_MENU, SCREEN_HEIGHT*0.3 - 30, TAILLE_MENU/2.2, 30};
    couleur = (SDL_Color){0, 0, 0, 255};
    afficherTexte(renderer, &rect, couleur, "Monde torique", font, surface, texture);

    /* Affichage des boutons pour les regles */
    // Affichage du texte naissance / survie
    rect = (SDL_Rect){SCREEN_WIDTH - 0.85*TAILLE_MENU, SCREEN_HEIGHT*0.5 - 30, TAILLE_MENU/3, 30};
    couleur = (SDL_Color){0, 0, 0, 255};
    afficherTexte(renderer, &rect, couleur, "Naissance", font, surface, texture);

    rect = (SDL_Rect){SCREEN_WIDTH - 0.45*TAILLE_MENU, SCREEN_HEIGHT*0.5 - 30, TAILLE_MENU/3, 30};
    couleur = (SDL_Color){0, 0, 0, 255};
    afficherTexte(renderer, &rect, couleur, "Survie", font, surface, texture);

    // placement des boutons et affichage du nombre de voisins
    for(int i = naiss_0; i <= surv_8; i++) {
        rect = (SDL_Rect){COORD_BOUTONS[i][x], COORD_BOUTONS[i][y], COORD_BOUTONS[i][taille], COORD_BOUTONS[i][taille]};
        couleur = (RULES[i%2][i/2])?(SDL_Color){0, 255, 0, 255}:(SDL_Color){255, 0, 0, 255};
        placerBouton(renderer, &rect, couleur);

        rect = (SDL_Rect){COORD_BOUTONS[i][x] - 20, COORD_BOUTONS[i][y] + 6, 10, 15};
        couleur = (SDL_Color){0, 0, 0, 255};
        if(i%2 == 0) {
            sprintf(texteNbVoisins, "%d", i/2);
            afficherTexte(renderer, &rect, couleur, texteNbVoisins, font, surface, texture);
        }
    }
    TTF_CloseFont(font);
}

int detecterBouton(int xClic, int yClic, int COORD_BOUTONS[21][3]) {
    int bouton = naiss_0;
    int xBtn, yBtn, tailleBtn;
    int trouve = 0;
    while(!trouve && bouton <= torique) {
        xBtn = COORD_BOUTONS[bouton][x];
        yBtn = COORD_BOUTONS[bouton][y];
        tailleBtn = COORD_BOUTONS[bouton][taille];
        if(xBtn <= xClic && xClic <= xBtn + tailleBtn && yBtn <= yClic && yClic <= yBtn + tailleBtn) {
            trouve = 1;
        }
        else {
            bouton++;
        }
    }
    return trouve?bouton:-1;
}

//===================== INIT ====================

int main(int argc, char **argv) {


	int **monde = NULL;
	//par lecture sauvegarde
	if (argc ==3 && strcmp(argv[1],"AUTO") == 0){
		sscanf(argv[2],"%s",NOM_FICHIER);
		chargement(&monde);
	}
	//TODO creer NOM_FICHIER + le mettre dans sauvegarde
	//par argument
	else if (argc == 6 && strcmp(argv[1],"MANUEL") == 0){
		sscanf(argv[2],"%s",NOM_FICHIER);
		sscanf(argv[3],"%d",&NB_CELL_HEIGHT);
		sscanf(argv[4],"%d",&NB_CELL_WIDTH);
		sscanf(argv[5],"%d",&estTorique);
		
		memset(RULES,0,2*9*sizeof(int));
		RULES[0][3] = 1;
		RULES[1][2] = RULES[1][3] = 1;

		calculConstantes();
		
		monde = initMonde();

	}
	else{
		fprintf(stderr, "Mauvais arguments => voir README\n");
		exit(1);
	}
	//Data
	char snapshots[TAILLE_BOUCLE][SIZE_SNAPSHOT];
	memset(snapshots, '\0', TAILLE_BOUCLE*SIZE_SNAPSHOT*sizeof(char) );

	
	int COORD_BOUTONS[21][3] = {
                                                                                // Boutons regles naissance | survie
                                                                                // pour chaque nombre de voisins

        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.5, 24},           // 0
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.5, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.55, 24},          // 1
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.55, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.6, 24},           // 2
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.6, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.65, 24},          // 3
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.65, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.7, 24},           // 4
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.7, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.75, 24},          // 5
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.75, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.8, 24},           // 6
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.8, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.85, 24},          // 7
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.85, 24},
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.9, 24},           // 8
        {SCREEN_WIDTH - TAILLE_MENU/3 - 12, SCREEN_HEIGHT*0.9, 24},

        {SCREEN_WIDTH - TAILLE_MENU/2 - 25, SCREEN_HEIGHT*0.15, 50},            // Bouton play/pause
        {SCREEN_WIDTH - 2*TAILLE_MENU/3 - 25, SCREEN_HEIGHT*0.3, 50},           // Bouton monde fini
        {SCREEN_WIDTH - TAILLE_MENU/3 - 25, SCREEN_HEIGHT*0.3, 50}              // Bouton monde torique
    };

	/* Initialisation de la SDL  + gestion de l'échec possible */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("Error : SDL initialisation - %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	/* Initialisation de SDL_TTF + gestion de l'echec possible */
    if (TTF_Init() != 0) {
        SDL_Log("Error : SDL_TTF initialisation - %s\n", TTF_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted= 0;

    initted = IMG_Init(flags);

    if((initted&flags) != flags)
    {
        SDL_Log("Error : SDL_TTF initialisation - %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

	SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

	window = SDL_CreateWindow("Fenetre", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (window == NULL) {
		SDL_Log("Error : SDL window creation - %s\n", SDL_GetError()); // échec de la création de la fenêtre
		FREE;
        IMG_Quit();
        TTF_Quit();
		SDL_Quit();                              // On referme la SDL       
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL){
		SDL_Log("Error : SDL renderer creation - %s\n", SDL_GetError());  // échec de la création du renderer
		FREE;
        IMG_Quit();
        TTF_Quit();
    	SDL_Quit();
    }
    
    
	
//==================== BOUCLE ====================


	/* Normalement, on devrait ici remplir les fenêtres... */
	SDL_bool program_on = SDL_TRUE;
	SDL_bool pause_on = SDL_TRUE;
	SDL_Event event;
	int xMouse, yMouse;
    int fps = 1;
	int bouton;
    clock_t begin;
    int iteration = 0;
    char iteration_txt[50];
	int nbCycle;

	while (program_on){
        begin = clock();
		while(SDL_PollEvent(&event)){                 
			switch(event.type){
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
					case SDLK_ESCAPE:
                        program_on = SDL_FALSE;
                        break;
					case SDLK_s:
						if (pause_on) sauvegarder(snapshots);
						break;
					case SDLK_p:
                    case SDLK_SPACE:
						pause_on = !pause_on;
						break;
                    case SDLK_RIGHT:
                        fps = (int)fmin(128, fps*2);
                        break;
                    case SDLK_LEFT:
                        fps = (int)fmax(1, fps/2);
                        break;
					default:
						break;
				}
				break;
			case SDL_QUIT :                        
				program_on = SDL_FALSE;                 
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_GetMouseState(&xMouse, &yMouse);
                if(xMouse <= SCREEN_WIDTH - TAILLE_MENU){
                    // Clics dans le tableau
                    if(pause_on)
                        monde[yMouse/CELL_SIZE][xMouse/CELL_SIZE] = !monde[yMouse/CELL_SIZE][xMouse/CELL_SIZE];
                }
                else {
                    // Clics dans le menu
                    bouton = detecterBouton(xMouse, yMouse, COORD_BOUTONS);
                    switch(bouton) {
                        case -1:
                            break;
                        case play_pause:
                            pause_on = !pause_on;
                            break;
                        case fini:
                            if(pause_on)
                                estTorique = 0;
                            break;
                        case torique:
                            if(pause_on)
                                estTorique = 1;
                            break;
                        default:
                            if(pause_on)
                                RULES[bouton%2][bouton/2] = !RULES[bouton%2][bouton/2];
                            break;
                    }
                }
				break;
			default:                                  
				break;
			}
		}

        if(!pause_on) {
            iteration++;
            updateCellules(monde);
			makeSnapshot(monde, snapshots);
			nbCycle = detectionCycle(snapshots);
			if (nbCycle == 1){
				sprintf(iteration_txt, "Itération %d | Stable", iteration);
			}
			else if (nbCycle > 1){
				sprintf(iteration_txt, "Itération %d | Cycle : %d", iteration, nbCycle);
			}
            else {
                sprintf(iteration_txt, "Itération %d", iteration);
            }
            SDL_SetWindowTitle(window, iteration_txt);
        }
        afficherCellules(renderer, monde);
		afficherMenu(renderer, COORD_BOUTONS, pause_on);
        SDL_RenderPresent(renderer);
        SDL_Delay(pause_on?1:(int)fmax(((1.0/fps) - ((clock() - begin)*10/(float)CLOCKS_PER_SEC))*1000, 1));
	}
	
//==================== FIN ====================


	/* et on referme tout ce qu'on a ouvert en ordre inverse de la création */
	FREE;
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();                                // la SDL
	freeMonde(monde);
	return 0;
}
