#include <sys/time.h>

#define SWAP(a,b) temp = (a); (a) = (b); (b) = temp;
#define ASIZE 25

double exTime;

void printarr(int size, int *arr);
int Select(int k, int n, int arr[]);
int cmpr(int *p1, int *p2);
void StartTime(void);
void StopTime(void);

int main(int argc, char *argv[]) {
	int arr[ASIZE];
	int i, j, n, median;

    for(j = 0; j < 20; j++) {
	for(i = 0; i < ASIZE; i++) {
	    arr[i] = random() & 1023;
	}
/*	printarr(ASIZE, arr); */
	n = (ASIZE+1)/2;
	StartTime();
	median =  Select(n, ASIZE, arr);
/*	printf("median is arr[%d] = %d\n", n - 1, median); */
	if((ASIZE & 1) == 0) {
	    int next;

	    next = arr[n];
	    while(++n < ASIZE) {
	    	if(arr[n] < next) next = arr[n];
	    }
/*	    printf("next = %d\n", next); */
	    median = (median + next)/2;
	}
	StopTime();
	printf("median %d, execution time %12.5f\n", median, exTime);
    }
#if 0
	printf("The median is %5d\n", median);
	qsort(arr, ASIZE, sizeof(int), cmpr);
	printarr(ASIZE, arr);
#endif
}

void printarr(int size, int *arr) {
	int i;

	for(i = 0; i < size; i++) {
	    printf("%5d", arr[i]);
	    if((i %10) == 9) putchar('\n');
	}
	putchar('\n');
}

int Select(int k, int n, int arr[]) {
	int i, j, l, m, r;
	int a, temp;

printf("Called Select(%d, %d, arr)\n", k, n);
	l = 0;
	r = n-1;
	k--;
	for(;;) {
/*	    printf("l = %d, r = %d\n", l, r); */
	/*    printarr(n, arr); */
	    if(r <= l+1) {
	    	if(r == l+1 && arr[r] < arr[l]) {
		    SWAP(arr[l], arr[r]);
		}
		return(arr[k]);
	    } else {
	    	m = (l + r) >> 1;
		SWAP(arr[m], arr[r])
		if(arr[l+1] > arr[r]) {
		    SWAP(arr[l+1], arr[r])
		}
		if(arr[l] > arr[r]) {
		    SWAP(arr[l], arr[r])
		}
		if(arr[l+1] > arr[l]) {
		    SWAP(arr[l+1], arr[l])
		}
		i = l+1;
		j = r;
		a = arr[l];
/* printf("m = %d, ar[l] = %d, arr[l+1] = %d, arr[r] = %d\n", m, arr[l], arr[l+1], arr[r]); */
		for(;;) {
		    do i++; while(arr[i] < a);
		    do j--; while(arr[j] > a);
		    if(j < i) break;
		    SWAP(arr[i], arr[j]);
		}
		arr[l] = arr[j];
		arr[j] = a;
		if(j >= k) r = j-1;
		if(j <= k) l = i;
	    }
	}
}

int cmpr(int *p1, int *p2) {
	int i, j;

	i = *p1;
	j = *p2;
	if(i > j) return(1);
	if(i < j) return(-1);
	return(0);
}

struct timeval tv1, tv2;
struct timezone tz;

void StartTime(void) {
        gettimeofday(&tv1, &tz);
}

void StopTime(void) {
        gettimeofday(&tv2, &tz);
        exTime = tv2.tv_sec - tv1.tv_sec + (double)(tv2.tv_usec - tv1.tv_usec) /
 1000000.;
}
