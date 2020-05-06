#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "main.h"

#define SIZE 20

int main(void) {
	int *da, *db, *dc, *ha, *hb, *hc;

	cudaMalloc(&da, SIZE * sizeof(int));
	cudaMalloc(&db, SIZE * sizeof(int));
	cudaMalloc(&dc, SIZE * sizeof(int));
	ha = (int*)malloc(SIZE * sizeof(int));
	hb = (int*)malloc(SIZE * sizeof(int));
	hc = (int*)malloc(SIZE * sizeof(int));

	for (int i = 0; i < SIZE; i ++) {
		ha[i] = i;
		hb[i] = i;
		hc[i] = 0;
	}

	cudaMemcpy(da, ha, SIZE * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(db, hb, SIZE * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(hc, dc, SIZE * sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(da);
	cudaFree(db);
	cudaFree(dc);
	free(ha);
	free(hb);
	free(hc);

	Timer timer;
	timer.start(L"Reading from file ...");
	unsigned int numlines = 0;
	Textfile wordlist(L"input.txt");
	unsigned int numberstrings = wordlist.countlines();
	timer.stop();

	timer.start(L"Sorting with Burstsort");
	Burstsort<Minuscule> bs;
	for (unsigned int i = 0; i < numberstrings; i++)
		bs.insert((wchar_t*)wordlist.getline(i));
	bs.sort();
	timer.stop();
	bs.print(true, false);
	bs.clear();

	wordlist.close();

	return 0;
}