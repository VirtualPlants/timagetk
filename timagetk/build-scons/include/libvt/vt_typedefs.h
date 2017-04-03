#ifndef _vt_typedefs_h_
#define _vt_typedefs_h_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* les typedefs des differents types
 */
#include <typedefs.h>



/* La dimension du traitement
 */
typedef enum {
  VT_1D = 1,
  VT_2D = 2,
  VT_3D = 3,
  VT_4D = 4,
  OneD = 1,
  TwoD = 2,
  ThreeD = 3
} DimType;

/* Point a trois coordonnees entieres (x,y,z)
 */
typedef struct {
	int x;
	int y;
	int z;
} vt_ipt;

/* Point a quatre coordonnees entieres (x,y,z,i)
 */
typedef struct {
    int x;
    int y;
    int z;
    int i;
} vt_4ipt;

/* Point a quatre coordonnees entieres (x,y,z,t)
 */
typedef struct {
    int x;
    int y;
    int z;
    int t;
} vt_4tpt;

/* Point a quatre coordonnees entieres (v,x,y,z)
 */
typedef struct {
    int v;
    int x;
    int y;
    int z;
} vt_4vpt;

/* Point a quatre coordonnees entieres (v,x,y,z)
   de type size_t (pour cause de grande image)
 */
typedef struct {
    size_t v;
    size_t x;
    size_t y;
    size_t z;
} vt_4vsize;


/* Point a trois coordonnees flottantes (x,y,z)
 */
typedef struct {
	float x;
	float y;
	float z;
} vt_fpt;

/* Point a quatre coordonnees flottantes (x,y,z,i)
 */
typedef struct {
    float x;
    float y;
    float z;
    float i;
} vt_4fpt;



#ifdef __cplusplus
}
#endif

#endif
