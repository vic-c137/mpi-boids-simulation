/*	Author: Vicor A. Colton
	File: MPIBoids.c
	Description: BOIDS implementation in LAM/MPI
	
	Compile: mpicc MPIBoids.c -o Boids
	Run:	 mpirun -np <number of processes> ./Boids 
			[boids] [loops] [k] [maxv] [acc] [width] [height]
	
	Notes: 
*/

// -------------------< Library inclusions >-------------------
#include <stdio.h>
#include <unistd.h>     
#include <stdlib.h>     
#include <string.h>     
#include <time.h>       
#include <mpi.h>
#include <sys/wait.h>
#include <math.h>
#include <fcntl.h>

// -------------------< MPI message tags >------------------- 
#define DATA	1	// Message to client with boid data to handle
#define DONE	2	// Message from client with handled boid data

// -------------------< MPI source tags >------------------- 
#define SERVER	0	// Message to client with boid data to handle

// -------------------< Debug flags >------------------- 
#define DELAY 1
//#define DEBUG_DELAY
//#define DEBUG_ARGS 
//#define DEBUG_SBOID_INIT
//#define DEBUG_CBOID_INIT
//#define DEBUG_CLIENT_INIT
//#define DEBUG_CYCLE   
//#define DEBUG_PHASE  
//#define DEBUG_PHASE1
//#define DEBUG_PHASE2
//#define DEBUG_CLIENT_QUEUES
//#define DEBUG_SERVER_QUEUES
//#define DEBUG_CLIENT2
//#define DEBUG_SORT
//#define DEBUG_INSERT
//#define DEBUG_KNN

// -------------------< Data Output flags >------------------- 
#define RUN_DATA
#define BOID_DATA
//#define TIME_DATA

// -------------------< Default input parameters >-------------------
#define LOOPS	500		// Number of loops to run
#define BOIDS	20		// Number of boids to run
#define WIDTH	500		// Width of the program area
#define HEIGHT	500		// Height of the program area
#define MAXV	10		// Maximum boid velocity
#define ACC		2.5	 	// Boid acceleration
#define K		6		// K nearest neighbors

// -------------------< Rule handling parameters >-------------------
#define SF1		1.0		// Scale factor 1
#define MIN		10.0	// The minimum distance between boids before repulsion 
#define SF3		8.0		// Scale factor 3
#define SF4		10.0	// Scale factor 4

// -------------------< Rule application flags >-------------------
#define RULE1		
#define RULE2
#define RULE3
#define RULE4

// -------------------< Struct definitions >-------------------
// Vector data struct
typedef struct Vector
{
	// data members
	double x;
	double y;
}Vector;

// Boid data struct
typedef struct Boid
{
	// data members
	int id;
	struct Vector * pos;
	struct Vector * vel;
}Boid;

// Node data struct
typedef struct Node
{
	// data members
	struct Boid * boid;
	double distance;
	
	// pointers
	struct Node * next;
	struct Node * prev;
}Node;

// Client data struct
typedef struct Client
{
	// data members
	int rank;
	struct Boid * boid;
	
	// pointers
	struct Client * next;
	struct Client * prev;
}Client;

// -------------------< Pre-declarations for Worker processes >-------------------
void Server(int size, int argc, char **argv[]);

void ClientProc(int rank, int argc, char **argv[]);		

// -------------------< Debug functions >-------------------
// Print the arguments recieved
void printargs(int argc, char **argv)
{
	int x;
	
	for(x = 0; x < argc; x++)
		printf("%s\n", argv[x]);
}

// Print the vector
void print_vector(Vector **vec)
{
	Vector *v = (*vec);
	printf("X component: %lf\n", v->x);
	printf("Y component: %lf\n", v->y);
}

// Print the boid
void print_boid(Boid **boid)
{
	Boid *b = (*boid);
	Vector *v = b->vel;
	Vector *p = b->pos;
	printf("Boid ID: %d\n",b->id);
	printf("Boid velocity:\n");
	print_vector(&v);
	printf("Boid position:\n");
	print_vector(&p);
}

// Print the node
void print_node(Node **node)
{
	Node *cur = (*node);
	printf("Printing node:\n");
	print_boid(&cur->boid);	
	printf("Distance: %lf\n", cur->distance);
}

// Print the linked list
void print_node_list(Node *list)
{
	Node *cur, *next;
	cur = list;
	
	if(list == NULL)
	{	
		printf("Empty list\n");
		return;
	}

	while(cur->next != list)
	{
		print_node(&cur);
		cur = cur->next;
	}
	print_node(&cur);
}

// Print the Client
void print_client(Client **client)
{
	Client *cur = (*client);
	printf("Printing client %d:\n", cur->rank);
}

// Print the linked list
void print_client_list(Client *list)
{
	Client *cur, *next;
	cur = list;
	
	if(list == NULL)
	{	
		printf("Empty list\n");
		return;
	}

	while(cur->next != list)
	{
		print_client(&cur);
		cur = cur->next;
	}
	print_client(&cur);
}
// -------------------< Utility functions >-------------------
// Parse the input arguments into parameter variables
void parseargs(int argc, char **argv, int *boids, int *loops, int *k, 
			   double *maxv, double *acc, int *width, int *height)
{
	if(argc > 1)// Parse boids
		*boids = atoi(argv[1]);
	else
		return;
	
	if(argc > 2)// Parse loops
		*loops = atoi(argv[2]);
	else
		return;
		
	if(argc > 3)// Parse k
		*k = atoi(argv[3]);
	else
		return;
		
	if(argc > 4)// Parse maxv
		*maxv = atof(argv[4]);
	else
		return;
		
	if(argc > 5)// Parse acc
		*acc = atof(argv[5]);
	else
		return;
		
	if(argc > 6)// Parse width
		*width = atoi(argv[6]);
	else
		return;
		
	if(argc > 7)// Parse height
		*height = atoi(argv[7]);
	else
		return;
	
	return;
}

// Print the parameters for the run
void print_run_data(int boids, int loops, int k, double maxv,
					double acc, int width, int height)
{
	printf("#header\n");
	printf("boids:%d;loops:%d;k:%d;maxv:%lf;acc:%lf;width:%d;height:%d\n",
			boids, loops, k, maxv, acc, width, height);
	printf("#endheader\n");
}
					
// -------------------< Random number generation functions >-------------------
// Ensure uniform distribution in get_random_XXX
double uniform_deviate(int seed)
{
	return seed * ( 1.0 / ( RAND_MAX + 1.0 ) );
}

// Return a random integer between M and N (inclusive)
int get_random_int(int M, int N)
{
	int r = M + uniform_deviate ( rand() ) * ( N - M );
	return r;
}

// Return a random double between M and N (exclusive)
double get_random_dbl(double M, double N)
{
	double r = M + uniform_deviate ( rand() ) * ( N - M );
	return r;
}

// -------------------< Vector manipulation functions >-------------------
// Return the magnitude of the vector
double mag(Vector *v)
{
	return sqrt(v->x * v->x + v->y * v->y);
}

// Add the two vectors
Vector * add(Vector *v1, Vector *v2)
{
	Vector *v = (Vector *)malloc(sizeof(Vector));
	v->x = v1->x + v2->x;
	v->y = v1->y + v2->y;
	return v;
}

// Add the two vectors, returning the value in the first
void addv(Vector **v1, Vector *v2)
{
	Vector *v = (*v1);
	v->x = v->x + v2->x;
	v->y = v->y + v2->y;
	*v1 = v;
}

// Divide the vector by the double
Vector * divide(Vector *v1, double a)
{
	Vector *v = (Vector *)malloc(sizeof(Vector));
	v->x = v1->x / a;
	v->y = v1->y / a;
	return v;
}

// Divide the vector by the double, returning the result in the vector
void dividev(Vector **v1, double a)
{
	Vector *v = (*v1);
	v->x = v->x / a;
	v->y = v->y / a;
	*v1 = v;
}

// -------------------< List manipulation functions >-------------------
// Calculate the distance between the position vectors
double dist(Boid *end, Boid *start)
{
	int dx, dy;
	dx = end->pos->x - start->pos->x;
	dy = end->pos->y - start->pos->y;
	return sqrt(dx*dx + dy*dy);
}

// Add a newNode to the end of the current list
Node * push_node(Node **newNode, Node **h)
{
	Node *head = (*h);
	Node *cur = (*newNode);
	
	if(head == NULL)
	{
		cur->next = cur;
		cur->prev = cur;
		head = cur;
	}	
	else
	{
		head->prev->next = cur;
		cur->prev = head->prev;
		cur->next = head;
		head->prev = cur;
	}
	return head;
}

// Add a newClient to the end of the current list
Client * push_client(Client **newClient, Client **h)
{
	Client *head = (*h);
	Client *cur = (*newClient);
	
	if(head == NULL)
	{
		cur->next = cur;
		cur->prev = cur;
		head = cur;
	}	
	else
	{
		head->prev->next = cur;
		cur->prev = head->prev;
		cur->next = head;
		head->prev = cur;
	}
	return head;
}

// Remove and return the current head Node of a list
Node * pop_node(Node **q)
{		
	Node *queue = (*q);
	Node *first;
	Node *next;
		
	if ( queue == NULL )// Empty list
		return NULL;	
	
	first = queue;
	next = queue->next;
	
	if( next == queue )// Single node list
	{	
		*q = NULL;
		return first;
	}
	
	// Multiple nodes - remove and return the first	
	*q = next;
	first->prev->next = first->next;
	first->next->prev = first->prev;
	first->next = first;
	first->prev = first;
	return first;
}

// Remove and return the Client at the head of a list
Client * pop_client(Client **q)
{
	Client *queue = (*q);
	Client *first;
	Client *next;
	
	// Empty list
	if ( queue == NULL )
		return NULL;
		
	first = queue;
	next = queue->next;
	
	// Single Client list
	if( next == queue )
	{
		*q = NULL;
		return first;
	}
	
	// Multiple Clients - remove and return the first
	*q = next;
	first->prev->next = first->next;
	first->next->prev = first->prev;
	first->next = first;
	first->prev = first;
	return first;
}

// Remove the Client with a given rank from the list and return it
Client * find(Client **q, int rank)
{
	Client *queue = (*q);
	Client *cur = queue;
	Client *next = cur->next;
	Client *ret = NULL;		// Default return value
	
	// Error, got an empty list, return null
	if ( queue == NULL )
		return ret;
	
	// Single Client list
	if ( cur == next )
	{
		// If our Client is in the list, remove it
		if ( cur->rank == rank )
		{
			ret = cur;
			*q = NULL;
		}
		
		// Return the result
		return ret;
	}
	
	// Check if cur is the Client we want
	if ( cur->rank == rank )
	{
		*q = next;
		// Remove cur from the list 		
		cur->prev->next = cur->next;
		cur->next->prev = cur->prev;
		cur->next = cur;
		cur->prev = cur;
		
		return cur;
	}
	
	// Look for the Client in the list
	while ( next != queue )
	{
		// If cur is not what we want, move to the next
		cur = next;
		next = cur->next;
		
		// Check if cur is the Client we want
		if ( cur->rank == rank )
		{
			// Remove cur from the list 	
			*q = next;			
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;
			cur->next = cur;
			cur->prev = cur;
		
			return cur;
		}
	}
	// Return NULL if none found
	return ret;
}

// Insert the node in increasing order of distance
Node * insert_in_order(Node **newNode, Node **q)
{
	Node *queue = (*q);
	Node *new = (*newNode);
	Node *cur;
	
#ifdef DEBUG_INSERT
	printf("\nAdding node %d with distance %lf\n",new->boid->id,new->distance);
#endif	

	if(queue == NULL)// Empty list, add the node to the start
	{
#ifdef DEBUG_INSERT
	printf("Empty list - adding to head\n");
#endif	
		new->next = new;
		new->prev = new;
		queue = new;
	}
	else// The list is not empty, so find the place to insert the node
	{
#ifdef DEBUG_INSERT
	printf("Searching list for insertion location\n");
#endif	
		cur = queue;
	
		// Search until we either hit the end or find the first larger distance
		while( cur->distance < new->distance && cur->next != queue )
		{
#ifdef DEBUG_INSERT
	printf("Current node has distance %lf\n", cur->distance);
	printf("Searching next node\n");
#endif		
			cur = cur->next;
		}
#ifdef DEBUG_INSERT
	printf("Current node has distance %lf\n", cur->distance);
	printf("Search ended\n");
#endif		
		
		// Found a larger distance, so shift the cur node over to make room
		if( new->distance < cur->distance )
		{
#ifdef DEBUG_INSERT
	printf("Found a larger distance node, so shift it over\n");
#endif		
			new->next = cur;
			new->prev = cur->prev;
			cur->prev->next = new;
			cur->prev = new;
			
			// If we just shifted the start of the queue, make the queue start at new
			if( cur == queue )
			{
#ifdef DEBUG_INSERT
	printf("We shifted the head, so change the head to the new node\n\n");
#endif			
				queue = new;			
			}
		}
		else// We got to the end, so add the new node onto the end 
		{
#ifdef DEBUG_INSERT
	printf("We got to the end, add the node here\n\n");
#endif		
			new->prev = cur;
			new->next = cur->next;
			cur->next->prev = new;
			cur->next = new;
		}
	
	}
	
	return queue;
}

// Insertion sort the specified list
void i_sort(Boid **b, Node **q)
{
	Node *qin = (*q);
	Boid *boid = (*b);
	Node *cur;
	Node *qout = NULL;
	
	// Start the cur pointer at the head of the input queue
	cur = qin;
	
	// Calculate the distance of the head node
	cur->distance = dist(boid, cur->boid);
	
	// Calculate the distance for each other boid in the list
	for ( cur = qin->next; cur != qin; cur = cur->next )
	{
		// Calculate the starting distance for cur and new
		cur->distance = dist(boid, cur->boid);
	}
	
	// While qin is not empty, do
	while( qin != NULL )
	{
		cur = pop_node(&qin);
		qout = insert_in_order(&cur, &qout);
	}
	
	// Return the queue to the caller
	*q = qout;
}

// Sort the list with respect to proximity to the given boid
void sort(Boid **b, Node **q, int n)
{
	Boid *boid = (*b);
	Node *queue = (*q);
	
	// Sort queue in order of increasing distance wrt boid
	i_sort(&boid, &queue);
	*q = queue;
}

// -------------------< Boid manipulation rules >-------------------
/* Rule 1: move each boid toward its percieved center of mass
PROCEDURE rule1(boid bJ)
	Vector pcJ

	FOR EACH BOID b
		pcJ = pcJ + b.position
	END

	pcJ = pcJ / K

	RETURN (pcJ - bJ.position) / 100
END PROCEDURE */
Vector * rule1(Boid *b, Node **k, int n)
{
	Node *knn = (*k);
	Node *cur = knn;
	Vector *bpos = (Vector *)malloc(sizeof(Vector)); 
	Vector *center = (Vector *)malloc(sizeof(Vector));
	int i;
	
	bpos->x = b->pos->x;
	bpos->y = b->pos->y;
	
	// For each boid in knn, add the position to center
	for ( i = 0; i < n; i++ )
	{
		addv(&center, cur->boid->pos);
		cur = cur->next;
	}
	
	// Divide center by the number of neighbors
	dividev(&center, n);
	
	// Invert the position of b
	dividev(&bpos, -1);
	
	// Add center and bpos together
	addv(&center, bpos);
	
	// Multiply the center by SF1 / 100
	dividev(&center, 100.0 / SF1 );
	
	// Free the position vector before returning
	free(bpos);
	
	// Return center
	return center;
}

/* Rule 2: avoid colliding with nearby boids
PROCEDURE rule2(boid b)
	Vector dv = 0;

	FOR EACH BOID cur
		IF cur != b THEN
			IF |cur.position - b.position| < 100 THEN
				dv = dv - (cur.position - b.position)
			END IF
		END IF
	END

	RETURN dv
END PROCEDURE */
Vector * rule2(Boid *b, Node **k, int n)
{
	Node *knn = (*k);
	Node *cur = knn;
	Vector *bpos = (Vector *)malloc(sizeof(Vector));
	Vector *cpos = (Vector *)malloc(sizeof(Vector));
	Vector *dv = (Vector *)malloc(sizeof(Vector));
	int i;
	
	bpos->x = -(b->pos->x);
	bpos->y = -(b->pos->y);
	
	for ( i = 0; i < n; i++ )
	{
		if( cur->distance < MIN )
		{
			cpos->x = cur->boid->pos->x;
			cpos->y = cur->boid->pos->y;
			addv(&cpos, bpos);
			if( cur->distance > 0 )
				dividev(&cpos, -cur->distance);
			else
				dividev(&cpos, -1);
			addv(&dv, cpos);
		}
		
		cur = cur->next;
	}
	free(bpos);
	free(cpos);
	
	return dv;
}
	
/* Rule 3: match velocity with nearby boids
PROCEDURE rule3(boid bJ)
	Vector pvJ

	FOR EACH BOID b
		IF b != bJ THEN
			pvJ = pvJ + b.velocity
		END IF
	END

	pvJ = pvJ / N-1

	RETURN (pvJ - bJ.velocity) / 8
END PROCEDURE */
Vector * rule3(Node **k, int n)
{
	Node *knn = (*k);
	Node *cur = knn;
	Vector * dv = (Vector *)malloc(sizeof(Vector));
	int i;
	
	dv->x = 0;
	dv->y = 0;
	
	for ( i = 0; i < n; i++ )
	{
		addv(&dv, cur->boid->vel);	
		cur = cur->next;
	}
	
	dividev(&dv, n * SF3);
	
	return dv;
}

/* Rule 4: bound the position of the boid
PROCEDURE rule4(Boid b)
	Integer Xmin, Xmax, Ymin, Ymax
	Vector v

	IF b.position.x < Xmin THEN
		v.x = 10
	ELSE IF b.position.x > Xmax THEN
		v.x = -10
	END IF
	IF b.position.y < Ymin THEN
		v.y = 10
	ELSE IF b.position.y > Ymax THEN
		v.y = -10
	END IF
		
	RETURN v
END PROCEDURE */
Vector * rule4(Boid *b, int xmax, int ymax)
{
	Vector * dv = (Vector *)malloc(sizeof(Vector));
	dv->x = 0;
	dv->y = 0;
	
	if( b->pos->x < 0 )
		//dv->x = SF4; 
		b->pos->x = (xmax + (int)b->pos->x)%xmax;
	else if( b->pos->x > xmax )
		//dv->x = -SF4;
		b->pos->x = (xmax + (int)b->pos->x)%xmax;
	if( b->pos->y < 0 )
		//dv->y = SF4;
		b->pos->y = (xmax + (int)b->pos->y)%xmax;
	else if( b->pos->y > ymax )
		//dv->y = -SF4;
		b->pos->y = (xmax + (int)b->pos->y)%xmax;
		
	return dv;
}

/* move_boid_to_new_position(boid b)
PROCEDURE 
    Vector v1, v2, ...

    v1 = rule1(b)
    v2 = rule2(b)
	...
    b.velocity = b.velocity + v1 + v2 + ...
	...
    b.position = b.position + b.velocity
END PROCEDURE */ 
void update_boid(Boid **b, Node **k, int n, int w, int h, double acc, double maxv)
{
	Boid *boid = (*b);
	Node *knn = (*k);
	Vector *v1, *v2, *v3, *v4;
	Vector *dv = (Vector *)malloc(sizeof(Vector));
	int xmax = w;
	int ymax = h;
	double m;
	
	// Initialize the acceleration vector
	dv->x = 0;
	dv->y = 0;
	
#ifdef RULE1	
	// Apply rule 1
	v1 = rule1(boid, &knn, n);
	addv(&dv, v1);
	free(v1);
#endif

#ifdef RULE2	
	// Apply rule 2
	v2 = rule2(boid, &knn, n);
	addv(&dv, v2);
	free(v2);
#endif	
	
#ifdef RULE3	
	// Apply rule 3
	v3 = rule3(&knn, n);
	addv(&dv, v3);
	free(v3);
#endif	
	
#ifdef RULE4	
	// Apply rule 4
	v4 = rule4(boid, w, h);
	addv(&dv, v4);
	free(v4);
#endif	
	
	// Limit the acceleration of the boid
	m = mag(dv);
	if( m > acc )
		dividev(&dv, m/acc);
	
	// Update the velocity of the boid
	addv(&boid->vel, dv);
	
	// Limit the velocity of the boid
	m = mag(boid->vel);
	if(  m > maxv )
		dividev(&boid->vel, m/maxv);
	free(dv);
	
	// Update the position  of the boid
	addv(&boid->pos, boid->vel);
	
	// Return the boid
	*b = boid;
}

// -------------------< Main program entry point >-------------------
int main ( int argc, char *argv[] )
{	
	int rank,	// Rank of the current process
		size;	// Size of the processes
		
	// Did MPI initialize 
	if ( MPI_Init(&argc, &argv) != MPI_SUCCESS )
	{ puts("MPI_Init failed"); exit(-1); }
	
	// Where am I and how many are we?
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);

	// Begin the simulation processes
	if ( !rank )// Server process happens here
		Server(size, argc, &argv);
	else// Client processes happen here
		ClientProc(rank, argc, &argv);
	
	// Finalize MPI
	MPI_Finalize();
	
	// Exit gracefully
	return 0;
}
	
// -------------------< Server process entry point >-------------------
void Server(int size, int argc, char ***argv)
{
	int boids = BOIDS;		  // Total number of boids to simulate
	int dealt;				  // Boid counter - tracks boids dealt
	int reported;			  // Boid counter - tracks boids reported
	int loops = LOOPS;		  // Total number of iterations to simulate 
	int loop;				  // Loop counter
	int k = K;				  // K nearest neighbors to monitor
	int width = WIDTH;		  // Width of the simulation area
	int height = HEIGHT;	  // Height of the simulation area
	int flag;				  // Message flag (boolean)
	int quit = 1;			  // Flag to end phase 2
	int client;				  // Client counter
	int cin;				  // Rank of Client sending DATA back
	int rc;					  // Recieve return code
	double maxv = MAXV;		  // Maximum boid velocity
	double acc = ACC;		  // Boid acceleration value
	double speed;			  // Velocity vector magnitude
	double heading;			  // Velocity vector angle (0-360*)
	double msgBuffer[5] = {}; // Array for boid message passing
	Client *available = NULL; // List of available clients
	Client *busy = NULL;	  // List of busy clients
	Client *c = NULL;		  // Current client
	Vector *v = NULL;		  // Current vector
	Boid *boid = NULL;		  // Current boid struct
	Node *queue = NULL;	  	  // Queue of Nodes to deal out
	Node *wait = NULL;		  // Queue of Nodes waiting to be used
	Node *cur = NULL;		  // Current boid Node struct
	MPI_Status *status;	  	  // MPI message status struct
	
	// Read the command line arguments
	parseargs(argc, *argv, &boids, &loops, &k, &maxv, &acc, &width, &height);
	
	// Limit k, if it is greater than boids
	if ( k >= boids )
		k = boids - 1;
	
#ifdef RUN_DATA
	print_run_data(boids, loops, k, maxv, acc, width, height);
#endif	
#ifdef DEBUG_ARGS
	printf("Process 0 reporting args:\n");
	printargs(argc, *argv);
	printf("Process 0 parsed args: boids %d, loops %d, k %d, maxv %lf, acc %lf, width %d, height %d\n", 
		boids, loops, k, maxv, acc, width, height);
#endif	

	// Initialize all boids
	for ( loop = 0; loop < boids; loop++ )
	{
		// Get a new random seed
		srand(MPI_Wtime()*boids + loop);
		
		// Allocate a new boid with random position and velocity
		boid = (Boid *)malloc(sizeof(Boid));
		boid->id = loop;
		
		// Allocate a new random position vector for the boid
		v = (Vector *)malloc(sizeof(Vector));
		v->x = (double)get_random_int(0, width);
		v->y = (double)get_random_int(0, height);
		boid->pos = v;
		
		// Allocate a new random velocity vector for the boid
		v = (Vector *)malloc(sizeof(Vector));
		speed = get_random_dbl(0.0, maxv);
		heading = get_random_dbl(0.0, 360.0);
		v->x = speed*cos(heading);
		v->y = speed*sin(heading);
		boid->vel = v;// Limit the initial velocity here, when implemented
		
		// Allocate a new list node and stuff it with the boid, then add to tail
		cur = (Node *)malloc(sizeof(Node));
		cur->boid = boid;
	
		queue = push_node(&cur, &queue);
		
#ifdef DEBUG_SBOID_INIT
	printf("Server pushing boid %d onto queue\n", loop);
	printf("Printing Server boid queue:\n");
	print_node_list(queue);
	printf("End Server boid queue\n");
#endif		
	}
	
	// Initialize all clients
	for ( loop = 1; loop < size; loop++ )
	{
		// Allocate a new Client and add it to the queue of clients
		c = (Client *)malloc(sizeof(Client));
		c->rank = loop;
		available = push_client(&c, &available);
		
#ifdef DEBUG_CLIENT_INIT
	printf("Server adding client %d to available list\n", loop);
	printf("Printing Client list:\n");
	print_client_list(available);
#endif			
	}

	// Run the boid simulation
	for ( loop = 0; loop < loops; loop++ )
	{
		// Phase 1: reporting and communication - wait for all Clients to enter this phase
		MPI_Barrier(MPI_COMM_WORLD);
		
#ifdef DEBUG_CYCLE
		printf("Server entering cycle %d\n", loop);
#endif		
#ifdef DEBUG_PHASE	
		printf("Server entering phase 1\n");
#endif	
		
		// Flush and broadcast all boid data
		for ( dealt = 0; dealt < boids; dealt++ )
		{	
		
#ifdef DEBUG_PHASE1
			printf("Server popping node number %d from queue\n", dealt);
#endif			

			cur = pop_node(&queue);
			
			// Flush boid data to stdout
#ifdef BOID_DATA			
			printf("%d %lf %lf %lf %lf\n", cur->boid->id, cur->boid->pos->x, cur->boid->pos->y, 
				cur->boid->vel->x, cur->boid->vel->y);
#endif					
			
			// Pack the items into the message buffer
			msgBuffer[0] = (double)cur->boid->id;
			msgBuffer[1] = cur->boid->pos->x;
			msgBuffer[2] = cur->boid->pos->y;
			msgBuffer[3] = cur->boid->vel->x;
			msgBuffer[4] = cur->boid->vel->y;
			
#ifdef DEBUG_PHASE1
			printf("Server broadcasting boid number %d to all Clients\n",dealt);
#endif		
	
			// Broadcast the message buffer
			MPI_Bcast(msgBuffer, 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			
#ifdef DEBUG_PHASE1
			printf("Server broadcast of boid number %d complete\n",dealt);
			printf("Server pushing node %d back onto queue\n", dealt);
#endif	
			
			queue = push_node(&cur, &queue);
		}	
		
#ifdef DEBUG_DELAY
			sleep(DELAY);
#endif	
		
		// Phase 2: boid handling - wait for all Clients to enter this phase
		MPI_Barrier(MPI_COMM_WORLD);
		
#ifdef DEBUG_PHASE	
		printf("Server entering phase 2\n");
#endif

		// Upon entering the phase, reset the dealt and reported counts
		dealt = reported = 0;
		client = size - 1;
		
		// While there are still undealt or unreported boids, deal them out and gather reports
		while( dealt < boids || reported < boids) 
		{
			// While there are boids remaining and free clients, deal boids to Clients
			while ( dealt < boids && client > 0 )
			{
				// Get a Client from the available list
				c = pop_client(&available);
				client--;
				
				// Get a Node from the queue
				cur = pop_node(&queue);
				
				// Attach the boid to the Client
				c->boid = cur->boid;
				
				// Add the pair to the busy list
				busy = push_client(&c, &busy);
				
#ifdef DEBUG_SERVER_QUEUES
			printf("Server pushed Client %d to the busy queue\n", c->rank);
			printf("Busy queue:\n");
			print_client_list(busy);
			printf("Available queue:\n");
			print_client_list(available);
#endif		
				
				// Push the Node on to the wait queue
				wait = push_node(&cur, &wait);

				// Pack the data into the message buffer
				msgBuffer[0] = (double)cur->boid->id;
				msgBuffer[1] = cur->boid->pos->x;
				msgBuffer[2] = cur->boid->pos->y;
				msgBuffer[3] = cur->boid->vel->x;
				msgBuffer[4] = cur->boid->vel->y;
				
#ifdef DEBUG_PHASE2
			printf("Server sending boid number %d to Client %d\n", dealt, c->rank);
#endif					

				// Send the boid data to the correct Client process	
				MPI_Send(msgBuffer, 5, MPI_DOUBLE, c->rank, DATA, MPI_COMM_WORLD);
				
				// Increment the number of boids dealt
				dealt++;
			}
					
#ifdef DEBUG_PHASE2
			printf("Server probing for DATA messages from Clients\n");
#endif				
#ifdef DEBUG_DELAY
			sleep(DELAY);
#endif	

			// Probe for a Client data report to the Server
			MPI_Iprobe(MPI_ANY_SOURCE, DATA, MPI_COMM_WORLD, &flag, status);
			
			// If there are boids to be reported and a Client report, gather the report
			if ( flag && reported < boids )
			{
				flag = 0;
				cin = status->MPI_SOURCE;
				
#ifdef DEBUG_PHASE2
			printf("Server recieving boid number %d from Client %d\n", reported, cin);
#endif				

				// Recieve the client return message data
				rc = MPI_Recv(msgBuffer, 5, MPI_DOUBLE, MPI_ANY_SOURCE, DATA, MPI_COMM_WORLD, status);
				
#ifdef DEBUG_PHASE2
			printf("Return code: %d\n",rc);
			cin = status->MPI_SOURCE;
			printf("Server recieved boid number %d from Client %d\n", reported, cin);
#endif					
				
				// Select the correct client from the busy queue
				c = find(&busy, cin);
				
#ifdef DEBUG_SERVER_QUEUES
			printf("Server found Client %d from the busy queue:\n", c->rank);
			print_client(&c);
			printf("Busy queue:\n");
			print_client_list(busy);
			printf("Available queue:\n");
			print_client_list(available);
#endif			
		
				// Pop a waiting Node from the wait queue
				cur = pop_node(&wait);
								
				// Unpack the updated boid data from the Client into the Node
				cur->boid->id = (int)msgBuffer[0];
				cur->boid->pos->x = msgBuffer[1];
				cur->boid->pos->y = msgBuffer[2];
				cur->boid->vel->x = msgBuffer[3];
				cur->boid->vel->y = msgBuffer[4];
				
				// Place the returned Node into the boid queue
				queue = push_node(&cur, &queue);
				
				// Set the Client boid data to null
				c->boid = NULL;
				
				// Add the Client to the available list
				available = push_client(&c, &available);
				client++;
				
				// Increment the number of boids reported
				reported++;			
			}			
		}
#ifdef DEBUG_DELAY
			sleep(DELAY);
#endif			
		
		// Send a message flag to each Client to exit phase 2
		for ( client = 1; client < size; client++ )
		{
		
#ifdef DEBUG_PHASE2
			printf("Server sending end of cycle command to Client %d\n", client);
#endif		

			MPI_Send(&quit, 1, MPI_INT, client, DONE, MPI_COMM_WORLD);	
		}
	}
#ifdef TIME_DATA
	printf("#timedata\n");
	
	// Print timing statistics
	
	printf("#endtimedata\n");
#endif
	
	
	return;
}

// -------------------< Client process entry point >-------------------
void ClientProc(int rank, int argc, char ***argv)
{
	int boids = BOIDS;	// Same as the parameters in Server above
	int dealt;
	int loops = LOOPS;
	int loop;
	int k = K;
	int flag;
	int quit;
	int width = WIDTH;
	int height = HEIGHT;
	double maxv = MAXV;
	double acc = ACC;
	double msgBuffer[5];			
	Vector *v;	
	Boid *boid = NULL;
	Node *queue = NULL;
	Node *knn = NULL;	// The list of k nearest neighbors of boid 
	Node *cur = NULL;
	MPI_Status *status;
	
	parseargs(argc, *argv, &boids, &loops, &k, &maxv, &acc, &width, &height);
	
#ifdef DEBUG_ARGS
	printf("Process %d reporting args:\n",rank);
	printargs(argc, *argv);
	printf("Process %d parsed args: boids %d, loops %d, k %d, maxv %lf, acc %lf, width %d, height %d\n", 
		rank, boids, loops, k, maxv, acc, width, height);
#endif	

	// Initialize local Client data structs to store boid list data
	for ( loop = 0; loop < boids; loop++ )
	{
		// Allocate a new boid with empty data
		boid = (Boid *)malloc(sizeof(Boid));
		boid->id = loop;
		
		// Allocate a new random position vector for the boid
		v = (Vector *)malloc(sizeof(Vector));
		boid->pos = v;
		
		// Allocate a new random velocity vector for the boid
		v = (Vector *)malloc(sizeof(Vector));
		boid->vel = v;
		
		// Allocate a new list node and stuff it with the boid, then add to tail
		cur = (Node *)malloc(sizeof(Node));
		cur->boid = boid;
		queue = push_node(&cur, &queue);
		
#ifdef DEBUG_CBOID_INIT
	printf("Printing boid queue from Client %d:\n", rank);
	print_node_list(queue);
	printf("End boid queue\n");
#endif			
	}

	// Allocate memory to store a recieved boid
	boid = (Boid *)malloc(sizeof(Boid));
	
	// Allocate a new random position vector for the boid
	v = (Vector *)malloc(sizeof(Vector));
	boid->pos = v;
		
	// Allocate a new random velocity vector for the boid
	v = (Vector *)malloc(sizeof(Vector));
	boid->vel = v;
	
	// Limit k, if it is greater than boids
	if ( k >= boids )
		k = boids - 1;
	
	// Run the boid simulation
	for ( loop = 0; loop < loops; loop++ )
	{
		// Phase 1: communication - wait for everybody to enter this phase
		MPI_Barrier(MPI_COMM_WORLD);
		
#ifdef DEBUG_PHASE	
		printf("Client %d entering phase 1\n", rank);
#endif		

		// Recieve broadcast data from Server to update the current boid list
		for ( dealt = 0; dealt < boids; dealt++ )
		{
		
#ifdef DEBUG_PHASE1
			printf("Client %d popping node number %d from queue\n", rank, dealt);
#endif			
			
			cur = pop_node(&queue);				
				
			// Recieve the boid data broadcast from root				
			MPI_Bcast(msgBuffer, 5, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			
#ifdef DEBUG_PHASE1
			printf("Client %d recieved broadcast boid number %d\n", rank, dealt);
			printf("Client %d unpacking boid number %d\n", rank, dealt);
#endif		
	
			// Unpack the boid data into the current Node
			cur->boid->id = (int)msgBuffer[0];
			cur->boid->pos->x = msgBuffer[1];
			cur->boid->pos->y = msgBuffer[2];
			cur->boid->vel->x = msgBuffer[3];
			cur->boid->vel->y = msgBuffer[4];		

#ifdef DEBUG_PHASE1
			print_node(&cur);
			printf("Client %d pushing boid number %d onto queue\n", rank, dealt);
#endif	

			queue = push_node(&cur, &queue);
		}	
		
#ifdef DEBUG_CLIENT_QUEUES
			printf("Client %d recieved the following list of nodes:\n", rank);
			print_node_list(queue);
#endif			
#ifdef DEBUG_DELAY
			sleep(DELAY);
#endif	
		
		// Phase 2: boid handling - wait for everybody to enter this phase
		MPI_Barrier(MPI_COMM_WORLD);
		
#ifdef DEBUG_PHASE	
		printf("Client %d entering phase 2\n", rank);
#endif	

		// Upon entering phase 2, set the run flag for the phase
		quit = 0; 
		
		// While there are still boids to be handled, get boids from the Server
		while( !quit ) 
		{
		
#ifdef DEBUG_PHASE2
			printf("Client %d probing for DATA message from Server\n", rank);
#endif					

			// Check for a DATA message from the Server with a boid
			MPI_Iprobe(SERVER, DATA, MPI_COMM_WORLD, &flag, status);
			
			// If found, handle the boid
			if( flag )
			{
				flag = 0;

#ifdef DEBUG_CLIENT2
			if ( rank == 2 )
				printf("Client 2 recieved a DATA message\n");
#endif				
	
				// Recieve the boid data from the Server
				MPI_Recv(msgBuffer, 5, MPI_DOUBLE, SERVER, DATA, MPI_COMM_WORLD, status);	
				
#ifdef DEBUG_PHASE2
			printf("Client %d recieved sent boid from Server\n", rank);
#endif			

				// Unpack the boid data into a boid struct
				boid->id = (int)msgBuffer[0];
				boid->pos->x = msgBuffer[1];
				boid->pos->y = msgBuffer[2];
				boid->vel->x = msgBuffer[3];
				boid->vel->y = msgBuffer[4];
											
				// Sort the list of boids in ascending order of proximity to the recieved boid
				sort(&boid, &queue, boids);
				
#ifdef DEBUG_SORT
			printf("\nList after sorting:\n");
			print_node_list(queue);
#endif					
				
				// Remove the closest boid, which is the recieved boid itself
				cur = pop_node(&queue);
				queue = push_node(&cur, &queue);
				
				// Remove the k nearest nieghbors and place them in the knn list
				for ( dealt = 0; dealt < k; dealt++ )
				{
					cur = pop_node(&queue);
					knn = push_node(&cur, &knn);
				}
				
#ifdef DEBUG_KNN
			printf("\nBoid:\n");
			print_boid(&boid);
			printf("K nearest neighbors of boid:\n");
			print_node_list(knn);
#endif					
				
				// Apply each of the rules to the recieved boid based upon the knn
				update_boid(&boid, &knn, k, width, height, acc, maxv);
				
				// Add the knn back into the queue
				for ( dealt = 0; dealt < k; dealt++ )
				{
					cur = pop_node(&knn);
					queue = push_node(&cur, &queue);
				}
			
				// Repack the message buffer
				msgBuffer[0] = (double)boid->id;
				msgBuffer[1] = boid->pos->x;
				msgBuffer[2] = boid->pos->y;
				msgBuffer[3] = boid->vel->x;
				msgBuffer[4] = boid->vel->y;
			
#ifdef DEBUG_PHASE2
			printf("Client %d sending boid back to Server\n", rank);
#endif			
		
				// Return the updated boid data to Server
				MPI_Send(msgBuffer, 5, MPI_DOUBLE, SERVER, DATA, MPI_COMM_WORLD);
				
#ifdef DEBUG_PHASE2
			printf("Client %d sent boid back to Server\n", rank);
#endif				
#ifdef DEBUG_CLIENT2
			if ( rank == 2 )
				printf("Client 2 sent a DATA message\n");
#endif		
			}
			
#ifdef DEBUG_DELAY
			sleep(DELAY);
#endif					
#ifdef DEBUG_PHASE2
			printf("Client %d probing for DONE message from Server\n", rank);
#endif			
		
			// Check for a DONE message from the Server
			MPI_Iprobe(SERVER, DONE, MPI_COMM_WORLD, &flag, status);
			
			// If found, exit phase 2
			if( flag )
			{
				flag = 0;
				
#ifdef DEBUG_CLIENT2
			if ( rank == 2 )
				printf("Client 2 recieved a DONE message\n");
#endif		
			
				// Consume the done message - VERY IMPORTANT!!
				MPI_Recv(&quit, 5, MPI_DOUBLE, SERVER, DONE, MPI_COMM_WORLD, status);
				
#ifdef DEBUG_PHASE2
			printf("Client %d recieved DONE message from Server\n", rank);
#endif				
			}
		}
	}
	return;
}
	