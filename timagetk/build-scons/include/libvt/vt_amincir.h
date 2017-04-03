
#ifndef _vt_amincir_h_
#define _vt_amincir_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <vt_common.h>
#include <vt_neighborhood.h>
#include <vt_distance.h>
#include <vt_seuil.h>

#define VT_GONGBERTRAND 1
#define VT_GRG          2
#define VT_GRG_PLANES   2
#define VT_GRG_CURVES   3
#define VT_GRG_2D       4

#define VT_BDD          1
#define VT_DIST         2
#define VT_OLD          3

typedef struct vt_amincir {
    float seuil;
    int type_method;
    int type_option;
    int bool_shrink;
    int bool_end_surfaces;
    int bool_end_curves;
    vt_distance par_dist;
    int connexite;
    Neighborhood epaisseur;
} vt_amincir;

#define VT_HASBEENDELETED  150
#define VT_TOBEDELETED     200
#define VT_WILLBEDELETED   220
#define VT_RECONSTRUCTABLE 225
#define VT_DELETABLE       230
#define VT_RECONSTRUCTED   235
#define VT_U_DELETABLE     240
#define VT_UNDELETABLE     255

typedef struct vt_pt_amincir {
	vt_ipt pt;
	int inside;
	int status;
} vt_pt_amincir;

typedef struct vt_vpt_amincir {
    vt_4vpt pt;
    int inside;
    int status;
} vt_vpt_amincir;

#include <vt_amseuil.h>
#include <vt_bdd_amincir.h>
#include <vt_old_amincir.h>
#include <vt_dst_amincir.h>




extern int VT_Intern_Thinning();

extern void VT_Amincir( vt_amincir *par );
extern int  VT_Thinning( vt_image *im1, vt_image *im2, vt_amincir *par );




#ifdef __cplusplus
}
#endif

#endif /* _vt_amincir_h_ */
