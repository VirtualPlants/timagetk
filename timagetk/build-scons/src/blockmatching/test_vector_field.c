//
//  test_vector_field.c
//  
//
//  Created by Leo Guignard on 07/06/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <transformation.h>
#include <balimage.h>

int main(int argc, char **argv) {
	printf("hello world\n");
	_TRANSFORMATION t1, t2, t3;
	bal_image ref;
	BAL_InitImage (&ref, "test", 10, 10, 1, 1, FLOAT);
	BAL_AllocImage (&ref);
	_alloc_transformation(&t1, NON_LINEAR, &ref);
	_alloc_transformation(&t2, NON_LINEAR, &ref);
	_alloc_transformation(&t3, NON_LINEAR, &ref);
	int i, j, k;
	float ***buf = (float***)ref.array;
	for (k=0; k<ref.nplanes; k++) {
		for (j=0; j<ref.nrows; j++) {
			for (i=0; i<ref.ncols; i++) {
				fflush(stdout);				
				buf[k][j][i]=0.;
			}
		}
	}
	ref.array = (void***)buf;
	BAL_CopyImage(&ref, &t2.z);
	BAL_CopyImage(&ref, &t1.z);
	BAL_CopyImage(&ref, &t1.y);
	BAL_CopyImage(&ref, &t2.y);
	buf = (float***)ref.array;
	for (i=2; i<8; i++) {
		buf[0][i][i]=1.;
	}
	//buf[0][3][4]=1.;
	ref.array = (void***)buf;
	BAL_CopyImage(&ref, &t1.x);
	buf = (float***)ref.array;
	for (i=2; i<8; i++) {
		buf[0][i][i]=0.;
	}
	for (i=2; i<8; i++) {
		buf[0][i][i+1]=-.9;
	}
	//buf[0][3][4]=0.;
	ref.array = (void***)buf;
	BAL_CopyImage(&ref, &t2.x);
	_compose_transformation(&t1, &t2, &t3);
	
	buf = (float***)t3.x.array;
	for (k=0; k<ref.nplanes; k++) {
		for (j=0; j<ref.nrows; j++) {
			for (i=0; i<ref.ncols; i++) {
				printf("%f ", buf[k][j][i]);
			}
			printf("\n");
		}
	}
	BAL_WriteImage(&(ref), "ref.hdr");
	BAL_WriteImage(&(t1.x), "x1.hdr");
	BAL_WriteImage(&(t2.x), "x2.hdr");
	BAL_WriteImage(&(t3.x), "x3.hdr");
	
	return 0;
}
