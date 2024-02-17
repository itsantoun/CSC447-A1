#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

struct complex {
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
        lengthsq = z_real2 + z_imag2;
        iter++;
    } while ((iter < MAX_ITER) && (lengthsq < 4.0));
    
    return iter;
}

void save_pgm(const char *filename, int **image) {
    FILE *pgmimg;
    int temp;
    pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n"); 
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT); 
    fprintf(pgmimg, "255\n"); 
    
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            temp = image[i][j];
            fprintf(pgmimg, "%d ", temp); 
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
}

int main(int argc, char *argv[]) {
    int rank, size;
    struct complex c;
    int **image;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        image = (int **)malloc(HEIGHT * sizeof(int *));
        for (int i = 0; i < HEIGHT; i++) {
            image[i] = (int *)malloc(WIDTH * sizeof(int));
        }
    }

    double start_time = MPI_Wtime();

   
    for (int i = rank; i < HEIGHT; i += size) {
        for (int j = 0; j < WIDTH; j++) {
            c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
            image[i][j] = cal_pixel(c);
        }
    }

    double end_time = MPI_Wtime(); 

    if (rank == 0) {
        printf("Execution time: %f seconds\n", end_time - start_time);
        save_pgm("mandelbrot_static.pgm", image);
    }

    MPI_Finalize();

    return 0;
}
