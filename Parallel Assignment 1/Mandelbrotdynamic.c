#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

#define WORK_TAG    1           /* "work" message (master to worker) */
#define RESULT_TAG  2           /* "data" message (worker to master) */
#define STOP_TAG    3           /* "stop" message (master to worker) */


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
    fprintf(pgmimg, "P2\n"); // Writing Magic Number to the File   
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);  // Writing Width and Height
    fprintf(pgmimg, "255\n");  // Writing the maximum gray value 
    int count = 0; 
    
    for (int i = 0; i < HEIGHT; i++) { 
        for (int j = 0; j < WIDTH; j++) { 
            temp = image[i][j]; 
            fprintf(pgmimg, "%d ", temp); // Writing the gray values in the 2D array to the file 
        } 
        fprintf(pgmimg, "\n"); 
    } 
    fclose(pgmimg); 
} 


int main(int argc, char** argv) {
	MPI_Init(NULL, NULL);
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	int rank;
	int ind;
	int image[HEIGHT][WIDTH];
	int curr_row[WIDTH+1];
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
    	if(rank == 0){ // master proceesor
    		int count = 0;
    		int row = 0;
    		int id;
    		clock_t start_time = clock();
    		for(int i = 1; i<worldSize; i++){
    			
    			MPI_Send(&row, 1, MPI_INT, i, WORK_TAG, MPI_COMM_WORLD);
    			count++;
    			row++;
    		}
    		do{
    			MPI_Recv(curr_row,WIDTH+1,MPI_INT,MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
    			id = status.MPI_SOURCE;
    			int y = curr_row[0];
    			for(int i = 1; i<=WIDTH; i++){
    				image[y][i-1] = curr_row[i];
    			}
    			count--;
    			
    			if(row < HEIGHT){
    				MPI_Send(&row, 1,MPI_INT,id, WORK_TAG, MPI_COMM_WORLD);
    				row++;
    				count++;
    			}else{
    				MPI_Send(NULL, 0, MPI_INT, id, STOP_TAG, MPI_COMM_WORLD);
    			}
    			
    		} while(count>0);
    		clock_t end_time = clock();
    		printf("Total time needed is: %f s\n", (double)(end_time - start_time)/CLOCKS_PER_SEC);
    		save_pgm("dynamic.pgm",image);
    	}
    	else {
    		int y;
		int row[WIDTH+1];
		MPI_Recv(&y, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		struct complex c;
		
		while(status.MPI_TAG != STOP_TAG){
			if(status.MPI_TAG == WORK_TAG) {
			    row[0] = y;
			    c.imag = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
			    for(int col = 1; col<=WIDTH; col++){
				c.real = (col-1 - WIDTH / 2.0) * 4.0 / WIDTH;
				
				row[col] = cal_pixel(c);
			    }
			    
			    MPI_Send(row, WIDTH+1, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
			}
			MPI_Recv(&y, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		    }
		}

   	MPI_Finalize();
    	return 0;
}
