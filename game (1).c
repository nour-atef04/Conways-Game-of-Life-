#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GRID_SIZE 20
#define NUM_THREADS 4
#define GENERATIONS 32

int grid[GRID_SIZE][GRID_SIZE];
pthread_barrier_t barrier;


void print_grid() {
    system("clear"); 
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                printf("# ");
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    usleep(500000); 
}

// function to count number of live neighbours relative to the given block coordinates
int num_live_neighbors(int row, int col){
    
    int num = 0;
    int start_row = row-1;
    int end_row = row+1;
    int start_col = col-1;
    int end_col = col+1;

    for(int i = start_row; i <= end_row; i++){
        for(int j = start_col; j <= end_col; j++){
            if(i >= 0 && i < GRID_SIZE && j >= 0 && j < GRID_SIZE && !(i==row && j==col)){
                num += grid[i][j];
            }
        }
    }

    return num;

}

// Function to compute next generation of Game of Life
// each thread executes this function, each responsible for a subset of rows
// each thread will process exactly 5 rows (grid_size / num_threads)
void* compute_next_gen(void* arg) {
  
    // calculate segment of rows for each thread
    int thread_id = *(int* )arg;
    int rows_per_thread = GRID_SIZE / NUM_THREADS;
    int start = thread_id * (rows_per_thread);
    int end = (thread_id == NUM_THREADS - 1) ? GRID_SIZE : start + rows_per_thread; // to prevent out of bounds

    // temp grid for storing updates
    int temp_grid[GRID_SIZE][GRID_SIZE] = {0};

    // loop over generations
    for(int generation = 0; generation < GENERATIONS; generation++){
        // loop over segment of rows
        for(int row = start; row < end; row++){
            for(int col = 0; col < GRID_SIZE; col++){
                int live_neighbors = num_live_neighbors(row, col);
                switch(live_neighbors){
                    case 3:
                        // cell is born or survives
                        temp_grid[row][col] = 1;
                        break;
                    case 2:
                        // cell survives if already alive
                        if(grid[row][col]) temp_grid[row][col] = 1;
                        break;
                    default:
                        // cell dies
                        temp_grid[row][col] = 0;
                }
            }
        }

        // wait for all other threads for synchronization 
        pthread_barrier_wait(&barrier);

        // copy temp to original grid
        for(int row = start; row < end; row++){
            for(int col = 0; col < GRID_SIZE; col++){
                grid[row][col] = temp_grid[row][col];
            }
        }

        // wait for all other threads for synchronization before printing next grid
        pthread_barrier_wait(&barrier);
    
        if(thread_id == 0){
            print_grid();
        }

        // wait for all other threads for synchronization after printing next grid
        pthread_barrier_wait(&barrier);

    }

    return NULL;

}


void initialize_grid(int grid[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;  // Set every cell to 0 (dead)
        }
    }
    }
void initialize_patterns(int grid[GRID_SIZE][GRID_SIZE]) {
    
    initialize_grid(grid);

    // Initialize Still Life (Square) at top-left
    grid[1][1] = 1;
    grid[1][2] = 1;
    grid[2][1] = 1;
    grid[2][2] = 1;

    // Initialize Oscillator (Blinker) in the middle
    grid[5][6] = 1;
    grid[6][6] = 1;
    grid[7][6] = 1;

    // Initialize Spaceship (Glider) in the bottom-right
    grid[10][10] = 1;
    grid[11][11] = 1;
    grid[12][9] = 1;
    grid[12][10] = 1;
    grid[12][11] = 1;
}

int main() {
    initialize_patterns(grid) ;
    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    pthread_t threads[NUM_THREADS]; // array of threads
    int thread_ids[NUM_THREADS]; // array of thread ids

    // create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, compute_next_gen, &thread_ids[i]);
    }

    // wait for other threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }


    pthread_barrier_destroy(&barrier);
    return 0;
}
