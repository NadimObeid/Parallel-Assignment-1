#include <stdio.h>
#include <mpi.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

struct complex{
  double real;
  double imag;
};


int cal_pixel(struct complex c) {
    

            double z_real = 0;
            double z_imag = 0;

            double z_real2, z_imag2, lengthsq;

            int iter = 0;
            do {
                z_real2 = z_real * z_real;
                z_imag2 = z_imag * z_imag;

                z_imag = 2 * z_real * z_imag + c.imag;
                z_real = z_real2 - z_imag2 + c.real;
                lengthsq =  z_real2 + z_imag2;
                iter++;
            }
            while ((iter < MAX_ITER) && (lengthsq < 4.0));

            return iter;

}

void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE* pgmimg; 
    int temp;
    pgmimg = fopen(filename, "wb"); 
    fprintf(pgmimg, "P2\n"); 
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);  
    fprintf(pgmimg, "255\n"); 
    int count = 0; 
    
    for (int i = 0; i < HEIGHT; i++) { 
        for(int j = 0; j<WIDTH; j++){
		temp = image[i][j]; 
		fprintf(pgmimg, "%d ", temp); 
		fprintf(pgmimg, "\n"); 
    	} 
    }
    fclose(pgmimg); 
} 


int main(int argc, char** argv) {
	MPI_Init(NULL, NULL);
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	
	
	int rows = HEIGHT / worldSize;
	int image[HEIGHT][WIDTH];
	int start = rows * rank;
	int end = start + rows;

    	double total_time;
    	struct complex c;
    	int region[rows][WIDTH];
    	int k;
        clock_t start_time;
        int i, j;
        for(int k = 0; k< 10; k++){
        	if(rank == 0) {start_time = clock();}
		for (i = start; i < end; i++) {
		    for (j = 0; j < WIDTH; j++) {
		        c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
		        c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
		        region[i-start][j] = cal_pixel(c); 
		    }
		}
		
		MPI_Gather(region, rows * WIDTH, MPI_INT, image, rows * WIDTH, MPI_INT, 0, MPI_COMM_WORLD);
	       
		

	    	if(rank == 0){
	    		clock_t end_time = clock(); 
			total_time += (double)(end_time - start_time)/CLOCKS_PER_SEC;
	    		printf("The execution time of trial[%d] is: %f s\n",k+1, (double)(end_time - start_time)/CLOCKS_PER_SEC);}
	    }
    	
   	if(rank == 0){
   		save_pgm("static3.pgm", image);
   		printf("The average running time of the programming is: %f s\n", (double) total_time/10);	
   	}
   	MPI_Finalize();
    

    	return 0;
}
