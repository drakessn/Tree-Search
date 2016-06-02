#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <GL/glut.h>

#define N 12 /**Numero de particulas**/
#define thread_count 4 /**Numero de threads**/
const double G = 6.673e-11f; /**Constante de Gravedad**/

static double digraph[N][N];
static double tour[N][2];
static double best_tour[N][2];
static double best_cost;

float Ranf( float, float );

void solve(double t[N][2], int last, int n){
	if (n==N){
		if (t[last][1]<best_cost){
			for (int i = 0; i < N; i++){
				best_tour[i][0]=t[i][0];
				best_tour[i][1]=t[i][1];
			}			
			best_cost=t[(int)t[last][0]][1];
		}				
	}
	else{	
		//#pragma omp parallel for schedule(dynamic, 1)
		for (int i = 1; i < N; i++)
		{
			if (t[i][1]==0)
			{
				double tmp[N][2];
				for (int j = 0; j < N; j++){
					tmp[j][0]=t[j][0];
					tmp[j][1]=t[j][1];
				}	
				tmp[(int)t[last][0]][0]=i;
				tmp[i][1]=tmp[(int)t[last][0]][1]+digraph[(int)t[last][0]][i];	
				solve(tmp, (int)t[last][0], n+1);
			}
			
		}
		
	}
	
}
 
 
void start(){	
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			digraph[i][j] = Ranf( 0.1f, 50.f );
		} 
		digraph[i][i] = 0;
		tour[i][0] = 0;
		tour[i][1] = 0;
		best_tour[i][0] = 0;
		best_tour[i][1] = 0;
	}
	best_cost=RAND_MAX;
	printf("Procesando...\n");
	double time0 = omp_get_wtime();
    solve(tour, 0, 1);
	double time1 = omp_get_wtime();
	/*
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%.2f\t",digraph[i][j]);	
		}
			printf("\n");	
	}
			printf("\n");	
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			printf("%.2f\t",best_tour[i][j]);	
		}
			printf("\n");	
	}*/
			printf("DISTANCIA MINIMA %f\n",best_cost);	
	
	/**Impresion del tiempo que tardo el algoritmo para determinar las
	 * posiciones y velocidades de las particulas ***/
	printf("duracion: %f\n",time1-time0);	
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
    glColor3f(1,1,1);
    glLoadIdentity();
    for(int i=0;i<N;i++){
		glBegin(GL_POINTS);
			glVertex3f(best_tour[i][0], best_tour[i][0], 0.0f);
		glEnd();  
	}
    glFlush();
}

void idle(){
    display();
    solve(tour, 0, 1);
}

int main(int argc, char **argv){
    start();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(1, 1);
	glutInitWindowSize(700, 700);
	//glutCreateWindow("N BODY");
	init();
	//glutDisplayFunc(display); 
	//glutIdleFunc(idle);
	//glutReshapeFunc(reshape);
	//glutMainLoop();
	return 0;
}

float Ranf( float low, float high ) {
	float r = (float) rand();		// 0 - RAND_MAX
	return(low + r*(high-low) / (float)RAND_MAX);
}
