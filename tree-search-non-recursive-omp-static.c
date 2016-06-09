#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <GL/glut.h>
#include <assert.h>

#define N 12 /**Numero de CIUDADES**/
#define NO_CONNECTED 0 /**Display**/
#define WIDTH 700 /**Display**/
#define HEIGHT 700 /**Display**/
#define thread_count 4 /**Numero de threads**/
const double G = 16; /**Constante de ESPACIO**/

omp_lock_t best_tour_lock;

typedef struct Tour{
	double cost;
	int path[N];
	int first;
	int last;
	int n;
}tour;

typedef struct STACK{
	tour *content;
	int top;
	int max;
}Stack;

void push(Stack *s, tour city){
	s->top++;
	if (s->top==s->max){	
		s->content = realloc(s->content,(N+s->max)*sizeof(tour));
		s->max += N;
	}
	s->content[s->top] = city;
	//printf("push: %d\n",s->content[s->top]);
}

tour pop(Stack *s){
	if (0<=s->top){	
		s->top--;
		return s->content[s->top+1];
	}
	tour tmp;
	return tmp;
}

static double digraph[N][N];
static double point[N][2];
static Stack *stack;
static Stack **stacks;
static tour *best_tour;
tour t1;

void add_city(tour *t, int city){
	t->last = t->path[t->last];
	t->path[t->last] = city;
	t->cost += digraph[t->last][city];
	t->n++;
}

void remove_last_city(tour *t){
	t->cost -= digraph[t->last][t->path[t->last]];
	t->n--;
	t->path[t->last] = 0;
	for (int i = 0; i < N; i++){
		if (t->path[i]==t->last){
			t->last = i;
			break;	
		}		
	}
}

void copy(tour *t, tour *t2){
	for (int i=0; i<N; i++){
		t->path[i] = t2->path[i];
	}	
	t->cost = t2->cost;
	t->last = t2->last;
	t->first = t2->first;
	t->n = t2->n;
}

int feasible(tour t, int city){
	if (t.path[city] || t.path[t.last]==city || digraph[t.path[t.last]][city]==NO_CONNECTED) return 0;
	return 1;
}

float Ranf( float, float );
void display();
void reshape(int width, int height) ;

void* TSP(void *argument){
	Stack *s = (Stack *)argument;
	while (0<=s->top){
		tour city=pop(s);
		if (city.n==N){
			omp_set_lock(&best_tour_lock);
			if (city.cost<best_tour->cost){
				/**imprime los mejores costos
				for (int i = 0; i < thread_count; i++){
					if (s==stacks[i])
						printf("id: %d\n",i);
				}				
				printf("costo: %f\n",best_tour->cost);**/
				copy(best_tour, &city);			
				display();
				reshape(WIDTH, HEIGHT);					
			}
			omp_unset_lock(&best_tour_lock);
		}
		else{
			for (int i=N-1; 1<=i; i--){
				if (feasible(city,i)){
					add_city(&city, i);
					push(s, city);
					remove_last_city(&city);
				}
			}
		
		}	
	}	
	return NULL;
}

void partition(Stack **s,tour t){
	s = malloc((thread_count)*sizeof(Stack*));
	for (int i = 0; i < thread_count; i++){
		s[i] = malloc(sizeof(Stack));
		s[i]->max = N*((N-1)*0.5);
		s[i]->top = -1;
		s[i]->content = malloc((s[i]->max)*sizeof(tour));
	}
	for (int i=N-1,k=0; 1<=i; i--){
		if (feasible(t,i)){
			add_city(&t, i);
			push(s[k%thread_count], t);
			remove_last_city(&t);
			k++;
		}
	}
	stacks=s;
}

void partition2(Stack **s,tour t){
	s = malloc((thread_count)*sizeof(Stack*));
	for (int i = 0; i < thread_count; i++){
		s[i] = malloc(sizeof(Stack));
		s[i]->max = N*((N-1)*0.5);
		s[i]->top = -1;
		s[i]->content = malloc((s[i]->max)*sizeof(tour));
	}
	for (int i=N-1,k=0; 1<=i; i--){
		if (feasible(t,i)){
			add_city(&t, i);
			for (int j=N-1; 1<=j; j--){
				if (feasible(t,j)){
					add_city(&t, j);
					push(s[k%thread_count], t);
					remove_last_city(&t);
					k++;
				}
			}
			remove_last_city(&t);
		}
	}
	stacks=s;
} 
 
void start(){	
	best_tour= malloc(sizeof(tour));
	stack= malloc(sizeof(Stack));
	for(int i=0;i<N;i++){
		digraph[i][i] = 0;
		point[i][0] = Ranf( -G, G );
		point[i][1] = Ranf( -G, G );
		best_tour->path[i] = 0;
		t1.path[i] = 0;
	}
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			digraph[i][j] = sqrt((point[i][0]-point[j][0])*(point[i][0]-point[j][0])+(point[i][1]-point[j][1])*(point[i][1]-point[j][1]));
		} 
	}
	best_tour->cost = RAND_MAX;
	best_tour->last = 0;
	best_tour->first = 0;
	best_tour->n = 1;
	t1.cost = 0;
	t1.last = 0;
	t1.first = 0;
	t1.n = 1;	
	stack->max = N*((N-1)*0.5);
	stack->top = -1;
	stack->content = malloc((stack->max)*sizeof(tour));
} 
void disconnect(){	
	for(int i=0;i<3;i++){
		int a = rand()%N;
		int b = rand()%N;
		digraph[a][b] = NO_CONNECTED;
		digraph[b][a] = NO_CONNECTED;
	}
}
 
void init(){
    glClearColor(0,0,0,0);    
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0-G, G, 0-G, G, 0-G, G);
    glMatrixMode(GL_MODELVIEW);
}
 
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(.1,.1,.1);
    glLoadIdentity();
	for(int i=0;i<N;i++){
		for (int j = 0; j < N; j++){
			if (digraph[i][j]!=NO_CONNECTED){
			glBegin(GL_LINES);
				glVertex3f(point[i][0], point[i][1], 0.0f);
				glVertex3f(point[j][0], point[j][1], 0.0f);
			glEnd(); 			
			}
		} 
	}	
    glColor3f(1,1,1);	
	for(int i=0;i<N;i++){
		if(best_tour->path[i]){
		glBegin(GL_LINES);
			glVertex3f(point[i][0], point[i][1], 0.0f);
			glVertex3f(point[best_tour->path[i]][0], point[best_tour->path[i]][1], 0.0f);
		glEnd();  
		}
	}
    glColor3f(1,0,0);
	glBegin(GL_POINTS);
			glVertex3f(point[0][0], point[0][1], 0.0f);
	glEnd(); 
    glFlush();
}

void idle(){
    display();
}

int main(int argc, char **argv){
	srand(time(NULL));
    start();    
    //disconnect();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(1, 1);
	glutInitWindowSize(700, 700);
	glutCreateWindow("TREE SEARCH");
	init();
	glutDisplayFunc(display); 	
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);	
	
	omp_init_lock(&best_tour_lock);
	
	printf("Procesando...\n");
	double time0 = omp_get_wtime();
	partition(stacks,t1);
	#pragma omp parallel for schedule(static, 1)
	for (int i = 0; i < thread_count; i++){
		TSP((void *) stacks[i]);
	}
	omp_destroy_lock(&best_tour_lock);
	double time1 = omp_get_wtime();		
	printf("Costo: %f\n",best_tour->cost);
	/**Impresion del tiempo que tardo el algoritmo para determinar las
	 * posiciones y velocidades de las particulas ***/
	printf("duracion: %f\n",time1-time0);	
	
	glutMainLoop();	
	return 0;
}

float Ranf( float low, float high ) {
	float r = (float) rand();		// 0 - RAND_MAX
	return(low + r*(high-low) / (float)RAND_MAX);
}
