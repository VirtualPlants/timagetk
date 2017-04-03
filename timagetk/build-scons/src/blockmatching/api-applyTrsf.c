/*************************************************************************
 * api-applyTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 24 jui 2015 17:33:43 CEST
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <chunks.h>
#include <reech4x4.h>
#include <reech4x4-coeff.h>
#include <reech-def.h>

#include <api-applyTrsf.h>






static int _verbose_ = 1;
static int _debug_ = 0;


static void _API_ParseParam_applyTrsf( char *str, lineCmdParamApplyTrsf *p );



/************************************************************
 *
 * main API
 *
 ************************************************************/

/* this one is kept for historical reasons,
 * ie Stracking compilation but should disappear
 */
int applyTrsf( char *theim_name,
               char *resim_name,
               char *real_transformation_name,
               char *voxel_transformation_name,
               char *result_real_transformation_name,
               char *result_voxel_transformation_name,
               char *template_image_name,
               bal_integerPoint dim,
               bal_floatPoint voxel,
               int resize,
               enumTransformationInterpolation interpolation,
               ImageType type,
               int isDebug,
               int isVerbose )
{
    char *proc = "applyTrsf";
    char str[256], *s;

    _debug_ = isDebug;

    _verbose_ = isVerbose;
    setVerboseInReech4x4( isVerbose );
    setVerboseInReechDef( isVerbose );
    BAL_SetVerboseInBalTransformation( isVerbose );
    BAL_SetVerboseInBalTransformationTools( isVerbose );

    if ( _verbose_ )
        fprintf( stderr, "Warning, '%s' is obsolete\n", proc );

    s = str;

    if ( dim.x > 0 && dim.y > 0 ) {
        sprintf( s, "-dim %d %d ", dim.x, dim.y );
        s = &(str[strlen(str)]);
        if ( dim.z > 0 ) {
            sprintf( s, "%d ", dim.z );
            s = &(str[strlen(str)]);
        }
    }

    if ( voxel.x > 0 && voxel.y > 0 ) {
        sprintf( s, "-voxel %f %f ", voxel.x, voxel.y );
        s = &(str[strlen(str)]);
        if ( voxel.z > 0 ) {
            sprintf( s, "%f ", voxel.z );
            s = &(str[strlen(str)]);
        }
    }

    if ( resize == 1 ) {
        sprintf( s, "-resize " );
        s = &(str[strlen(str)]);
    }


    switch( interpolation ) {
    default :
        break;
    case NEAREST :
        sprintf( s, "-nearest " ); break;
    case LINEAR :
        sprintf( s, "-linear " ); break;
    case CSPLINE :
        sprintf( s, "-cspline " ); break;
    }
    s = &(str[strlen(str)]);

    switch( type ) {
    default :
        break;
    case SCHAR :
        sprintf( s, "-type s8 " ); break;
    case UCHAR :
        sprintf( s, "-type u8 " ); break;
    case SSHORT :
        sprintf( s, "-type s16 " ); break;
    case USHORT :
        sprintf( s, "-type u16 " ); break;
    case SINT :
        sprintf( s, "-type s32 " ); break;
    case UINT :
        sprintf( s, "-type u32 " ); break;
    case SLINT :
        sprintf( s, "-type s64 " ); break;
    case ULINT :
        sprintf( s, "-type u64 " ); break;
    case FLOAT :
        sprintf( s, "-type r32 " ); break;
    case DOUBLE :
        sprintf( s, "-type r64 " ); break;
    }
    s = &(str[strlen(str)]);

    if ( API_INTERMEDIARY_applyTrsf( theim_name, resim_name,
                                     template_image_name,
                                     real_transformation_name,
                                     voxel_transformation_name,
                                     result_real_transformation_name,
                                     result_voxel_transformation_name,
                                     str, (char*)NULL ) != 1 ) {
        if ( _verbose_ )
            fprintf( stderr, "%s: some error occurs\n", proc );
        return( -1 );
    }

    return( 0 );

}





int API_INTERMEDIARY_applyTrsf( char* theimage_name,
                                char* resimage_name,
                                char* template_image_name,
                                char *real_transformation_name,
                                char *voxel_transformation_name,
                                char *result_real_transformation_name,
                                char *result_voxel_transformation_name,
                                char *param_str_1, char *param_str_2 )

{
    char *proc = "API_INTERMEDIARY_applyTrsf";
    bal_image theIm, tempIm;
    bal_image resIm;
    bal_image coefIm;
    bal_transformation theTrsf;
    bal_transformation *ptrTrsf = (bal_transformation *)NULL;

    lineCmdParamApplyTrsf par;



    /* parameter initialization
     */
    API_InitParam_applyTrsf( &par );

    /* parameter parsing
     */
    if ( param_str_1 != (char*)NULL )
        _API_ParseParam_applyTrsf( param_str_1, &par );
    if ( param_str_2 != (char*)NULL )
        _API_ParseParam_applyTrsf( param_str_2, &par );



    /* reading input image
     */
    if ( BAL_ReadImage( &theIm, theimage_name, 0 ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: can not read input image '%s'\n", proc, theimage_name );
      return( -1 );
    }

    /* reading transformation, if any
     */
    BAL_InitTransformation( &theTrsf );
    if ( real_transformation_name != (char*)NULL && real_transformation_name[0] != '\0' ) {
      if ( BAL_ReadTransformation( &theTrsf, real_transformation_name ) != 1 ) {
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read 'real' transformation '%s'\n", proc, real_transformation_name );
        return( -1 );
      }
      theTrsf.transformation_unit = REAL_UNIT;
      ptrTrsf = &theTrsf;
    }
    else if ( voxel_transformation_name != (char*)NULL && voxel_transformation_name[0] != '\0' ) {
      if ( BAL_ReadTransformation( &theTrsf, voxel_transformation_name ) != 1 ) {
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to read 'voxel' transformation '%s'\n", proc, voxel_transformation_name );
        return(-1);
      }
      theTrsf.transformation_unit = VOXEL_UNIT;
      ptrTrsf = &theTrsf;
    }



    /* initializing result image
     * - with transformation, if vector field
     * - with reference image, if any
     * - with parameters, if any
     * - with input image
     */

    /* initialization with vector field transformation
     */
    if ( BAL_IsTransformationVectorField( ptrTrsf ) == 1 ) {
        if ( BAL_InitAllocImage( &resIm, (char*)NULL,
                                 theTrsf.vx.ncols, theTrsf.vx.nrows, theTrsf.vx.nplanes,
                                 theIm.vdim, theIm.type ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from vector field)\n", proc );
          return( -1 );
        }
        resIm.vx = theTrsf.vx.vx;
        resIm.vy = theTrsf.vx.vy;
        resIm.vz = theTrsf.vx.vz;
    }

    /* initialization with reference image
     */
    else if ( template_image_name != (char*)NULL && template_image_name[0] != '\0' ) {
        if ( BAL_ReadImage( &tempIm, template_image_name, 0 ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: can not read template image '%s'\n", proc, template_image_name );
          return(-1);
        }
        if ( BAL_InitAllocImage( &resIm, (char*)NULL,
                                 tempIm.ncols, tempIm.nrows, tempIm.nplanes,
                                 theIm.vdim, theIm.type ) != 1 ) {
          BAL_FreeImage( &tempIm );
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from template image)\n", proc );
          return(-1);
        }
        resIm.vx = tempIm.vx;
        resIm.vy = tempIm.vy;
        resIm.vz = tempIm.vz;
        BAL_FreeImage( &tempIm );
    }

    /* initialisation with parameters (dimensions)
     */
    else if ( par.template_dim.x > 0 && par.template_dim.y > 0 ) {
      if ( par.template_dim.z > 0 ) {
        if ( BAL_InitAllocImage( &resIm, (char*)NULL, par.template_dim.x, par.template_dim.y,
                                 par.template_dim.z, theIm.vdim, theIm.type  ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from parameters, 3D case)\n", proc );
          return(-1);
        }
      }
      else {
        if ( BAL_InitAllocImage( &resIm, (char*)NULL, par.template_dim.x, par.template_dim.y,
                                 1, theIm.vdim, theIm.type  ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from parameters, 2D case)\n", proc );
          return(-1);
        }
      }
      if ( par.resize == 1 || ptrTrsf == (bal_transformation*)NULL ) {
          resIm.vx = ( theIm.ncols * theIm.vx ) / ((float)resIm.ncols);
          resIm.vy = ( theIm.nrows * theIm.vy ) / ((float)resIm.nrows);
          resIm.vz = ( theIm.nplanes * theIm.vz ) / ((float)resIm.nplanes);
      }
      else {
          resIm.vx = theIm.vx;
          resIm.vy = theIm.vy;
          resIm.vz = theIm.vz;
          if ( par.template_voxel.x > 0.0 ) resIm.vx = par.template_voxel.x;
          if ( par.template_voxel.y > 0.0 ) resIm.vy = par.template_voxel.y;
          if ( par.template_voxel.z > 0.0 ) resIm.vz = par.template_voxel.z;
      }
    }

    /* initialisation with parameters (voxel size)
     */
    else if ( ( par.resize == 1 || ptrTrsf == (bal_transformation*)NULL )
              && (par.template_voxel.x > 0 && par.template_voxel.y > 0) ) {
      par.template_dim.x = (int)( 0.5 + theIm.ncols * theIm.vx / par.template_voxel.x );
      par.template_dim.y = (int)( 0.5 + theIm.nrows * theIm.vy / par.template_voxel.y );

      if ( theIm.nplanes > 1 ) {
        if ( par.template_voxel.z <= 0.0 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to calculate result image Z dimension (from parameters, 3D case)\n", proc );
          return(-1);
        }
        par.template_dim.z = (int)( 0.5 + theIm.nplanes * theIm.vz / par.template_voxel.z );
        if ( BAL_InitAllocImage( &resIm, (char*)NULL, par.template_dim.x, par.template_dim.y,
                                 par.template_dim.z, theIm.vdim, theIm.type ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from parameters, 3D case)\n", proc );
          return(-1);
        }
      }
      else {
        if ( BAL_InitAllocImage( &resIm, (char*)NULL, par.template_dim.x, par.template_dim.y,
                                 1, theIm.vdim, theIm.type ) != 1 ) {
          BAL_FreeTransformation( &theTrsf );
          BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to initialize result image (from parameters, 2D case)\n", proc );
          return(-1);
        }
      }
      if ( par.template_voxel.x > 0.0 ) resIm.vx = par.template_voxel.x;
      if ( par.template_voxel.y > 0.0 ) resIm.vy = par.template_voxel.y;
      if ( par.template_voxel.z > 0.0 ) resIm.vz = par.template_voxel.z;
    }

    /* initialisation with input image
     */
    else {
      if ( BAL_InitAllocImage( &resIm, (char*)NULL,
                               theIm.ncols, theIm.nrows, theIm.nplanes,
                               theIm.vdim, theIm.type ) != 1 ) {
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to initialize result image (from input image)\n", proc );
        return(-1);
      }
      resIm.vx = theIm.vx;
      resIm.vy = theIm.vy;
      resIm.vz = theIm.vz;
    }





    /*  resampling transformation
     *  - the given transformation, if any
     *  - calculate a image-to-image transformation
     */

    if ( (real_transformation_name == (char*)NULL || real_transformation_name[0] == '\0')
         && (voxel_transformation_name == (char*)NULL || voxel_transformation_name[0] == '\0') ) {
      if ( BAL_AllocTransformation( &theTrsf, AFFINE_3D, (bal_image *)NULL ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to initialize image to image transformation\n", proc );
        return(-1);
      }

      if ( BAL_ComputeImageToImageTransformation( &resIm, &theIm, &theTrsf ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute image to image transformation\n", proc );
        return(-1);
      }
      ptrTrsf = &theTrsf;
    }



    /************************************************************
     *
     *  here is the computation takes place
     *
     ************************************************************/

    if ( API_applyTrsf( &theIm, &resIm, ptrTrsf, param_str_1, param_str_2 ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to resample image\n", proc );
        return(-1);
    }



    /************************************************************
     *
     *  additional result: linear interpolation coefficients
     *
     ************************************************************/

    if ( par.output_coefficient_name != (char*)NULL
         && par.output_coefficient_name[0] != '\0' ) {
        if ( BAL_InitAllocImageFromImage( &coefIm, par.output_coefficient_name, &resIm, FLOAT ) != 1 ) {
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate coefficient image\n", proc );
            return(-1);
        }
        if ( API_coeffTrsf( &theIm, &coefIm, ptrTrsf, param_str_1, param_str_2 ) != 1 ) {
            BAL_FreeImage( &coefIm );
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to compute coefficient image\n", proc );
            return(-1);
        }
        if ( BAL_WriteImage( &coefIm, par.output_coefficient_name ) != 1 ) {
            BAL_FreeImage( &coefIm );
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write result coefficient image '%s'\n", proc, par.output_coefficient_name );
          return(-1);
        }
        BAL_FreeImage( &coefIm );
    }



    /************************************************************
     *
     *  additional result: transformation amplitude
     *
     ************************************************************/

    if ( par.output_trsf_modulus != (char*)NULL
         && par.output_trsf_modulus[0] != '\0' ) {
        if ( BAL_InitAllocImageFromImage( &coefIm, par.output_trsf_modulus, &resIm, FLOAT ) != 1 ) {
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to allocate amplitude transformation image\n", proc );
            return(-1);
        }
        if ( API_amplitudeTrsf( &coefIm, ptrTrsf ) != 1 ) {
            BAL_FreeImage( &coefIm );
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
            if ( _verbose_ )
              fprintf( stderr, "%s: unable to compute amplitude transformation image\n", proc );
            return(-1);
        }
        if ( BAL_WriteImage( &coefIm, par.output_trsf_modulus ) != 1 ) {
            BAL_FreeImage( &coefIm );
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &theTrsf );
            BAL_FreeImage( &theIm );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write result amplitude transformation image '%s'\n", proc, par.output_trsf_modulus );
          return(-1);
        }
        BAL_FreeImage( &coefIm );
    }



    /************************************************************
     *
     *  additional result: transformation
     *
     ************************************************************/

    if ( result_real_transformation_name != (char*)NULL && result_real_transformation_name[0] != '\0' ) {
      if ( BAL_WriteTransformation( &theTrsf, result_real_transformation_name ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write real transformation\n", proc );
        return(-1);
      }
    }

    if ( result_voxel_transformation_name != (char*)NULL && result_voxel_transformation_name[0] != '\0' ) {
      if ( BAL_ChangeTransformationToVoxelUnit( &theIm, &resIm, &theTrsf, &theTrsf ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to convert transformation\n", proc );
        return(-1);
      }
      if ( BAL_WriteTransformation( &theTrsf, result_voxel_transformation_name ) != 1 ) {
        BAL_FreeImage( &resIm );
        BAL_FreeTransformation( &theTrsf );
        BAL_FreeImage( &theIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write voxel transformation\n", proc );
        return(-1);
      }
    }

    BAL_FreeTransformation( &theTrsf );
    BAL_FreeImage( &theIm );



    /************************************************************
     *
     *  output image
     *
     ************************************************************/

    if ( par.output_type == resIm.type || par.output_type == TYPE_UNKNOWN ) {

      if ( BAL_WriteImage( &resIm, resimage_name ) != 1 ) {
        BAL_FreeImage( &resIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write result image '%s'\n", proc, resimage_name );
        return(-1);
      }

    }
    else {

      if ( BAL_InitAllocImageFromImage( &tempIm, (char*)NULL,
                                        &resIm, par.output_type ) != 1 ) {
        BAL_FreeImage( &resIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate auxiliary result image\n", proc );
        return(-1);
      }

      if ( BAL_CopyImage( &resIm, &tempIm ) != 1 ) {
        BAL_FreeImage( &tempIm );
        BAL_FreeImage( &resIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to convert result image\n", proc );
        return(-1);
      }

      if ( BAL_WriteImage( &tempIm, resimage_name ) != 1 ) {
        BAL_FreeImage( &tempIm );
        BAL_FreeImage( &resIm );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write result image '%s'\n", proc, resimage_name );
        return(-1);
      }

      BAL_FreeImage( &tempIm );

    }

    BAL_FreeImage( &resIm );


    return( 1 );
}





int API_applyTrsf( bal_image *image, bal_image *imres,
                   bal_transformation *trsf,
                   char *param_str_1, char *param_str_2 )
{
  char *proc = "API_applyTrsf";
  lineCmdParamApplyTrsf par;



  /* parameter initialization
   */
  API_InitParam_applyTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_applyTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_applyTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_applyTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( _debug_ )
      BAL_PrintTransformation( stderr, trsf, "resampling transformation" );

  if ( BAL_ResampleImage( image, imres, trsf, par.interpolation ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute resampling\n", proc );
    return(-1);
  }

  return( 1 );
}





/************************************************************
 *
 * make an image of linear interpolation coefficient
 *
 ************************************************************/



int API_coeffTrsf( bal_image *image, bal_image *imres,
                   bal_transformation *trsf,
                   char *param_str_1, char *param_str_2 )
{
  char *proc = "API_coeffTrsf";
  lineCmdParamApplyTrsf par;



  /* parameter initialization
   */
  API_InitParam_applyTrsf( &par );

  /* parameter parsing
   */
  if ( param_str_1 != (char*)NULL )
      _API_ParseParam_applyTrsf( param_str_1, &par );
  if ( param_str_2 != (char*)NULL )
      _API_ParseParam_applyTrsf( param_str_2, &par );

  if ( par.print_lineCmdParam )
      API_PrintParam_applyTrsf( stderr, proc, &par, (char*)NULL );

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( _debug_ )
      BAL_PrintTransformation( stderr, trsf, "resampling coefficient computation" );

  if ( BAL_LinearResamplingCoefficients( image, imres, trsf, par.interpolation, par.coefficient_index ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute resampling coefficient\n", proc );
    return(-1);
  }

  return( 1 );
}






/************************************************************
 *
 * make an image of transformation modulus
 *
 ************************************************************/



int API_amplitudeTrsf( bal_image *imres,
                     bal_transformation *trsf )
{
  char *proc = "API_amplitudeTrsf";
  bal_doublePoint thePts, resPts;
  size_t i, j, k;
  float dx, dy, dz;

  /************************************************************
   *
   *  here is the stuff
   *
   ************************************************************/

  if ( _debug_ )
      BAL_PrintTransformation( stderr, trsf, "resampling coefficient computation" );

  if ( trsf->transformation_unit == VOXEL_UNIT ) {
      if ( _verbose_ )
          fprintf( stderr, "%s: warning, modulus is computed with a voxel based transformation\n", proc );
  }

  switch( imres->type ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case FLOAT :
    {
      float ***theBuf = (float***)(imres->array);
      for ( k=0, thePts.z=0.0; k<imres->nplanes; k++, thePts.z+=imres->vz )
      for ( j=0, thePts.y=0.0; j<imres->nrows; j++, thePts.y+=imres->vy )
      for ( i=0, thePts.x=0.0; i<imres->ncols; i++, thePts.x+=imres->vx ) {
          if ( BAL_TransformDoublePoint( &thePts, &resPts, trsf) != 1 ) {
              if ( _verbose_ )
                fprintf( stderr, "%s: unable to transform point (%lu,%lu,%lu)\n", proc, i, j, k );
              return( -1 );
          }
          dx = resPts.x - thePts.x;
          dy = resPts.y - thePts.y;
          dz = resPts.z - thePts.z;
          theBuf[k][j][i] = sqrt( dx*dx + dy*dy + dz*dz );
      }
    }
    break;
  }


  return( 1 );
}





/************************************************************
 *
 * static functions
 *
 ************************************************************/



static char **_Str2Array( int *argc, char *str )
{
  char *proc = "_Str2Array";
  int n = 0;
  char *s = str;
  char **array, **a;

  if ( s == (char*)NULL || strlen( s ) == 0 ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: empty input string\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* go to the first valid character
   */
  while ( *s == ' ' || *s == '\n' || *s == '\t' )
    s++;

  if ( *s == '\0' ) {
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: weird, input string contains only separation characters\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  /* count the number of strings
   */
  for ( n = 0; *s != '\0'; ) {
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' )
      s ++;
  }

  if ( _verbose_ >= 5 )
    fprintf( stderr, "%s: found %d strings\n", proc, n );

  /* the value of the strings will be duplicated
   * so that the input string can be freed
   */
  array = (char**)malloc( n * sizeof(char*) + (strlen(str)+1) * sizeof(char) );
  if ( array == (char**)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    *argc = 0;
    return( (char**)NULL );
  }

  a = array;
  a += n;
  s = (char*)a;
  (void)strncpy( s, str, strlen( str ) );
  s[ strlen( str ) ] = '\0';

  while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
    *s = '\0';
    s++;
  }

  for ( n = 0; *s != '\0'; ) {
    array[n] = s;
    n ++;
    while ( *s != ' ' && *s != '\n' && *s != '\t' && *s != '\0' )
      s ++;
    while ( *s == ' ' || *s == '\n' || *s == '\t' ) {
      *s = '\0';
      s ++;
    }
  }

  *argc = n;
  return( array );
}





/************************************************************
 *
 * help / documentation
 *
 ************************************************************/



static char *usage = "[image-in] [image-out]\n\
 [-transformation |-trsf %s]\n\
 [-voxel-transformation |-voxel-trsf %s]\n\
 [-result-transformation|-res-trsf %s]\n\
 [-result-voxel-transformation|-res-voxel-trsf %s]\n\
 [-template-image|-template|-dims|-t %s]\n\
 [-template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]]\n\
 [-x %d] [-y %d] [-z %d]\n\
 [-template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]]\n\
 [-resize] [-isotropic-voxel|-iso %lf]\n\
 [-nearest|-linear|-cspline] [-interpolation nearest|linear|cspline]\n\
 [-coefficient-image|-cimage %s] [-coefficient-index|-cindex %d]\n\
 [-parallel|-no-parallel] [-max-chunks %d]\n\
 [-parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread]\n\
 [-omp-scheduling|-omps default|static|dynamic-one|dynamic|guided]\n\
 [output-image-type | -type s8|u8|s16|u16...]\n\
 [-verbose|-v] [-nv|-noverbose] [-debug|-D] [-nodebug]\n\
 [-print-parameters|-param]\n\
 [-print-time|-time] [-notime]\n\
 [-help|-h]";



static char *detail = "\
 if 'image-in' is equal to '-', stdin will be used\n\
 if 'image-out' is not specified or equal to '-', stdout will be used\n\
 if both are not specified, stdin and stdout will be used\n\
# transformations\n\
 -transformation|-trsf %s # transformation to be applied\n\
    in 'real' coordinates. Goes from 'result image' to 'input image'.\n\
    If indicated, '-voxel-transformation' is ignored.\n\
 -voxel-transformation|-voxel-trsf %s:  transformation to be applied\n\
    in 'voxel' coordinates.\n\
 -result-transformation|-res-trsf %s: applied transformation\n\
    in 'real' coordinates. Useful to applied the same resizing transformation\n\
    to an other image or to combine transformations.\n\
 -result-voxel-transformation|-res-voxel-trsf %s # applied transformation\n\
    in 'voxel' coordinates.\n\
# template image for output image geometry. If no information is given,\n\
    input image geometry is used\n\
 -template-image|-template|-dims|-t %s: template image\n\
    to set image geometry for vector field transformation\n\
 -template-dimension[s]|-template-dim|-dimension[s]|-dim %d %d [%d]: dimensions\n\
    of the resulting vector field\n\
 -x %d: dimension along X\n\
 -y %d: dimension along Y\n\
 -z %d: dimension along Z\n\
 -template-voxel|-voxel-size|-voxel|-pixel|-vs %lf %lf [%lf]:\n\
    voxel sizes of the resulting vector field\n\
# specific parameters\n\
 -resize: if output image dimensions are given, output voxel size\n\
    is computed so that the ouput field of view correspond to the\n\
    input's one (only when no transformation is given).\n\
    If  output image voxel sizes are given, output image dimensions\n\
    are computed so that the ouput field of view correspond to the input's\n\
    one (only when no transformation is given)\n\
 -isotropic-voxel|-iso %lf: short for '-resize -voxel %lf %lf %lf'\n\
 -interpolation nearest|linear|cspline: selection of interpolation mode\n\
 -nearest: nearest neighor interpolation mode (for binary or lable images)\n\
 -linear: bi- or tri-linear interpolation\n\
 -cspline: cubic spline\n\
# monitoring linear resampling coefficients\n\
 -coefficient-image|-cimage %s: output coefficient image\n\
 -coefficient-index|-cindex %d: coefficient index (ordered by value)\n\
# monitoring transformation\n\
 -modulus-image|-mimage %s: output transformation amplitude image\n\
# parallelism parameters\n\
 -parallel|-no-parallel:\n\
 -max-chunks %d:\n\
 -parallelism-type|-parallel-type default|none|openmp|omp|pthread|thread:\n\
 -omp-scheduling|-omps default|static|dynamic-one|dynamic|guided:\n\
# general image related parameters\n\
   output-image-type: -o 1    : unsigned char\n\
                      -o 2    : unsigned short int\n\
                      -o 2 -s : short int\n\
                      -o 4 -s : int\n\
                      -r      : float\n\
  -type s8|u8|s16|u16|... \n\
   default is type of input image\n\
# general parameters \n\
  -verbose|-v: increase verboseness\n\
    parameters being read several time, use '-nv -v -v ...'\n\
    to set the verboseness level\n\
  -noverbose|-nv: no verboseness at all\n\
  -debug|-D: increase debug level\n\
  -nodebug: no debug indication\n\
  -print-parameters|-param:\n\
  -print-time|-time:\n\
  -no-time|-notime:\n\
  -h: print option list\n\
  -help: print option list + details\n\
";





char *API_Help_applyTrsf( int h )
{
    if ( h == 0 )
        return( usage );
    return( detail );
}





void API_ErrorParse_applyTrsf( char *program, char *str, int flag )
{
    if ( flag >= 0 ) {
        if ( program != (char*)NULL )
           (void)fprintf(stderr,"Usage: %s %s\n", program, usage);
        else
            (void)fprintf(stderr,"Command line options: %s\n", usage);
    }
    if ( flag == 1 ) {
      (void)fprintf( stderr, "--------------------------------------------------\n" );
      (void)fprintf(stderr,"%s",detail);
      (void)fprintf( stderr, "--------------------------------------------------\n" );
    }
    if ( str != (char*)NULL )
      (void)fprintf(stderr,"Error: %s\n",str);
    exit(0);

}





/************************************************************
 *
 * parameters management
 *
 ************************************************************/



void API_InitParam_applyTrsf( lineCmdParamApplyTrsf *p )
{
    (void)strncpy( p->input_name, "\0", 1 );
    (void)strncpy( p->output_name, "\0", 1 );
    p->output_type = TYPE_UNKNOWN;

    (void)strncpy( p->input_real_transformation, "\0", 1 );
    (void)strncpy( p->input_voxel_transformation, "\0", 1 );

    (void)strncpy( p->output_real_transformation, "\0", 1 );
    (void)strncpy( p->output_voxel_transformation, "\0", 1 );

    (void)strncpy( p->template_name, "\0", 1 );

    p->template_dim.x = 0;
    p->template_dim.y = 0;
    p->template_dim.z = 0;

    p->template_voxel.x = -1.0;
    p->template_voxel.y = -1.0;
    p->template_voxel.z = -1.0;

    p->resize = 0;

    p->interpolation = LINEAR;

    (void)strncpy( p->output_coefficient_name, "\0", 1 );
    p->coefficient_index = 0;

    (void)strncpy( p->output_trsf_modulus, "\0", 1 );

    p->print_lineCmdParam = 0;
    p->print_time = 0;
}





void API_PrintParam_applyTrsf( FILE *theFile, char *program,
                                  lineCmdParamApplyTrsf *p, char *str )
{
  FILE *f = theFile;
  if ( theFile == (FILE*)NULL ) f = stderr;

  fprintf( f, "==================================================\n" );
  fprintf( f, "= in line command parameters" );
  if ( program != (char*)NULL )
    fprintf( f, " for '%s'", program );
  if ( str != (char*)NULL )
    fprintf( f, "= %s\n", str );
  fprintf( f, "\n"  );
  fprintf( f, "==================================================\n" );


  fprintf( f, "# image names\n" );

  fprintf( f, "- input image is " );
  if ( p->input_name != (char*)NULL && p->input_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output image is " );
  if ( p->output_name != (char*)NULL && p->output_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "# transformation names\n" );

  fprintf( f, "- transformation to be applied (real units) is " );
  if ( p->input_real_transformation != (char*)NULL && p->input_real_transformation[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_real_transformation );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- transformation to be applied (voxel units) is " );
  if ( p->input_voxel_transformation != (char*)NULL && p->input_voxel_transformation[0] != '\0' )
    fprintf( f, "'%s'\n", p->input_voxel_transformation );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation (real units) is " );
  if ( p->output_real_transformation != (char*)NULL && p->output_real_transformation[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_real_transformation );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- output transformation (voxel units) is " );
  if ( p->output_voxel_transformation != (char*)NULL && p->output_voxel_transformation[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_voxel_transformation );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "# template for output image geometry\n" );

  fprintf( f, "- template image is " );
  if ( p->template_name != (char*)NULL && p->template_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->template_name );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "- template image dimensions are [%d %d %d]\n",
           p->template_dim.x, p->template_dim.y, p->template_dim.z );

  fprintf( f, "- template image voxel sizes are [%f %f %f]\n",
           p->template_voxel.x, p->template_voxel.y, p->template_voxel.z );

  fprintf( f, "# specific parameter\n" );

  fprintf( f, "- just resize input image = %d\n", p->resize );

  fprintf( f, "- interpolation mode = " );
  switch ( p->interpolation ) {
  default :      fprintf( f, "unknown\n" ); break;
  case NEAREST : fprintf( f, "nearest point\n" ); break;
  case LINEAR :  fprintf( f, "bi- or tri-linear\n" ); break;
  case CSPLINE : fprintf( f, "cubic spline\n" ); break;
  }

  fprintf( f, "# monitoring linear resampling coefficient\n" );

  fprintf( f, "- output coefficient image is " );
  if ( p->output_coefficient_name != (char*)NULL && p->output_coefficient_name[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_coefficient_name );
  else
    fprintf( f, "'NULL'\n" );
  fprintf( f, "- coefficient index (ordered by value) = %d\n", p->resize );

  fprintf( f, "# monitoring transformation\n" );

  fprintf( f, "- output transformation amplitude image is " );
  if ( p->output_trsf_modulus != (char*)NULL && p->output_trsf_modulus[0] != '\0' )
    fprintf( f, "'%s'\n", p->output_trsf_modulus );
  else
    fprintf( f, "'NULL'\n" );

  fprintf( f, "# general image related parameters\n" );

  fprintf( f, "- output image type = " );
  switch ( p->output_type ) {
  default :     fprintf( f, "TYPE_UNKNOWN\n" ); break;
  case SCHAR :  fprintf( f, "SCHAR\n" ); break;
  case UCHAR :  fprintf( f, "UCHAR\n" ); break;
  case SSHORT : fprintf( f, "SSHORT\n" ); break;
  case USHORT : fprintf( f, "USHORT\n" ); break;
  case UINT :   fprintf( f, "UINT\n" ); break;
  case SINT :   fprintf( f, "INT\n" ); break;
  case ULINT :  fprintf( f, "ULINT\n" ); break;
  case FLOAT :  fprintf( f, "FLOAT\n" ); break;
  case DOUBLE : fprintf( f, "DOUBLE\n" ); break;
  }

  fprintf( f, "==================================================\n" );
}





/************************************************************
 *
 * parameters parsing
 *
 ************************************************************/



static void _API_ParseParam_applyTrsf( char *str, lineCmdParamApplyTrsf *p )
{
  char *proc = "_API_ParseParam_applyTrsf";
  char **argv;
  int i, argc;

  if ( str == (char*)NULL || strlen(str) == 0 )
      return;

  argv = _Str2Array( &argc, str );
  if ( argv == (char**)NULL || argc == 0 ) {
      if ( _debug_ ) {
          fprintf( stderr, "%s: weird, no arguments were found\n", proc );
      }
      return;
  }

  if ( _debug_ > 4 ) {
      fprintf( stderr, "%s: translation from\n", proc );
      fprintf( stderr, "   '%s'\n", str );
      fprintf( stderr, "into\n" );
      for ( i=0; i<argc; i++ )
          fprintf( stderr, "   argv[%2d] = '%s'\n", i, argv[i] );
  }

  API_ParseParam_applyTrsf( 0, argc, argv, p );

  free( argv );
}





void API_ParseParam_applyTrsf( int firstargc, int argc, char *argv[],
                                  lineCmdParamApplyTrsf *p )
{
  int i;
  int inputisread = 0;
  int outputisread = 0;
  char text[STRINGLENGTH];
  int status;
  int maxchunks;
  int o=0, s=0, r=0;



  /* option line parsing
   */
  for ( i=firstargc; i<argc; i++ ) {

      /* strings beginning with '-'
       */
      if ( argv[i][0] == '-' ) {
          if ( argv[i][1] == '\0' ) {
            if ( inputisread == 0 ) {
              (void)strcpy( p->input_name,  "<" );  /* standart input */
              inputisread = 1;
            }
            else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name,  ">" );  /* standart output */
              outputisread = 1;
            }
            else {
              API_ErrorParse_applyTrsf( (char*)NULL, "too many file names, parsing '-' ...\n", 0 );
            }
          }

          /* transformation names
           */

          else if ( strcmp ( argv[i], "-transformation") == 0
                    || (strcmp ( argv[i], "-trsf") == 0 && argv[i][5] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -transformation", 0 );
                 (void)strcpy( p->input_real_transformation, argv[i] );
          }
          else if ( strcmp ( argv[i], "-voxel-transformation") == 0
                    || (strcmp ( argv[i], "-voxel-trsf") == 0 && argv[i][11] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -voxel-transformation", 0 );
                 (void)strcpy( p->input_voxel_transformation, argv[i] );
          }
          else if ( strcmp ( argv[i], "-result-transformation") == 0
                    || (strcmp ( argv[i], "-res-trsf") == 0 && argv[i][9] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -result-transformation", 0 );
                 (void)strcpy( p->output_real_transformation, argv[i] );
          }
          else if ( strcmp ( argv[i], "-result-voxel-transformation") == 0
                    || (strcmp ( argv[i], "-res-voxel-trsf") == 0 && argv[i][11] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -result-voxel-transformation", 0 );
                 (void)strcpy( p->output_voxel_transformation, argv[i] );
          }

          /* template
           */

          else if ( strcmp ( argv[i], "-template-image") == 0
                    || (strcmp ( argv[i], "-template") == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-t") == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "-dims") == 0 && argv[i][5] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-image", 0 );
                 (void)strcpy( p->template_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-template-dimensions") == 0
                    || strcmp ( argv[i], "-template-dimension") == 0
                    || strcmp ( argv[i], "-template-dim") == 0
                    || strcmp ( argv[i], "-dimensions") == 0
                    || strcmp ( argv[i], "-dimension") == 0
                    || (strcmp (argv[i], "-dim" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.x) );
            if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-dimensions %d", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            status = sscanf( argv[i], "%d", &(p->template_dim.y) );
            if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-dimensions %d %d", 0 );
            i ++;
            if ( i >= argc) p->template_dim.z = 1;
            else {
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) {
                i--;
                p->template_dim.z = 1;
              }
            }
          }
          else if ( strcmp ( argv[i], "-x") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -x ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.x) );
              if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -x ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-y") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -y ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.y) );
              if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -y ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-z") == 0 && argv[i][2] == '\0' ) {
              i++;
              if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -z ...\n", 0 );
              status = sscanf( argv[i], "%d", &(p->template_dim.z) );
              if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -z ...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-template-voxel") == 0
                    || strcmp ( argv[i], "-voxel-size") == 0
                    || (strcmp (argv[i], "-voxel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-pixel" ) == 0 && argv[i][6] == '\0')
                    || (strcmp (argv[i], "-vs" ) == 0 && argv[i][3] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-voxel %lf", 0 );
            i ++;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.y) );
            if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -template-voxel %lf %lf", 0 );
            i ++;
            if ( i >= argc) p->template_voxel.z = 1;
            else {
              status = sscanf( argv[i], "%lf", &(p->template_voxel.z) );
              if ( status <= 0 ) {
                i--;
                p->template_voxel.z = 1;
              }
            }
          }

          /* specific parameters
           */

          else if ( strcmp ( argv[i], "-resize" ) == 0 ) {
            p->resize = 1;
          }

          else if ( strcmp (argv[i], "-isotropic-voxel" ) == 0
                    || (strcmp (argv[i], "-iso" ) == 0 && argv[i][4] == '\0') ) {
            i ++;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -isotropic-voxel...\n", 0 );
            status = sscanf( argv[i], "%lf", &(p->template_voxel.x) );
            if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -isotropic-voxel...\n", 0 );
            p->template_voxel.y = p->template_voxel.z = p->template_voxel.x;
            p->resize = 1;
          }

          else if ( strcmp ( argv[i], "-interpolation" ) == 0 ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -interpolation...\n", 0 );
            if ( strcmp ( argv[i], "nearest" ) == 0 ) {
               p->interpolation = NEAREST;
            }
            else if ( strcmp ( argv[i], "linear" ) == 0 ) {
              p->interpolation = LINEAR;
            }
            else if ( strcmp ( argv[i], "cspline" ) == 0 ) {
                p->interpolation = CSPLINE;
            }
            else {
              fprintf( stderr, "unknown interpolation mode: '%s'\n", argv[i] );
              API_ErrorParse_applyTrsf( (char*)NULL, "parsing -interpolation ...\n", 0 );
            }
          }

          else if ( strcmp ( argv[i], "-nearest" ) == 0 ) {
            p->interpolation = NEAREST;
          }
          else if ( strcmp ( argv[i], "-linear" ) == 0 ) {
            p->interpolation = LINEAR;
          }
          else if ( strcmp ( argv[i], "-cspline" ) == 0 ) {
              p->interpolation = CSPLINE;
          }

          /* monitoring linear resampling coefficient
           */
          else if ( strcmp ( argv[i], "-coefficient-image") == 0
                    || (strcmp ( argv[i], "-cimage") == 0 && argv[i][7] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -coefficient-image", 0 );
                 (void)strcpy( p->output_coefficient_name, argv[i] );
          }
          else if ( strcmp ( argv[i], "-coefficient-index") == 0
                    || (strcmp ( argv[i], "-cindex") == 0 && argv[i][7] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -coefficient-index", 0 );
                 status = sscanf( argv[i], "%d", &(p->coefficient_index) );
                 if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -coefficient-index...\n", 0 );
          }

          /* monitoring transformation
           */
          else if ( strcmp ( argv[i], "-modulus-image") == 0
                    || (strcmp ( argv[i], "-mimage") == 0 && argv[i][7] == '\0') ) {
                 i++;
                 if ( i >= argc) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -modulus-image", 0 );
                 (void)strcpy( p->output_trsf_modulus, argv[i] );
          }


          /* parallelism parameters
           */
          else if ( strcmp ( argv[i], "-parallel" ) == 0 ) {
             setParallelism( _DEFAULT_PARALLELISM_ );
          }

          else if ( strcmp ( argv[i], "-no-parallel" ) == 0 ) {
             setParallelism( _NO_PARALLELISM_ );
          }

          else if ( strcmp ( argv[i], "-parallelism-type" ) == 0 ||
                      strcmp ( argv[i], "-parallel-type" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             if ( strcmp ( argv[i], "default" ) == 0 ) {
               setParallelism( _DEFAULT_PARALLELISM_ );
             }
             else if ( strcmp ( argv[i], "none" ) == 0 ) {
               setParallelism( _NO_PARALLELISM_ );
             }
             else if ( strcmp ( argv[i], "openmp" ) == 0 || strcmp ( argv[i], "omp" ) == 0 ) {
               setParallelism( _OMP_PARALLELISM_ );
             }
             else if ( strcmp ( argv[i], "pthread" ) == 0 || strcmp ( argv[i], "thread" ) == 0 ) {
               setParallelism( _PTHREAD_PARALLELISM_ );
             }
             else {
               fprintf( stderr, "unknown parallelism type: '%s'\n", argv[i] );
               API_ErrorParse_applyTrsf( (char*)NULL, "parsing -parallelism-type ...\n", 0 );
             }
          }

          else if ( strcmp ( argv[i], "-max-chunks" ) == 0 ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             status = sscanf( argv[i], "%d", &maxchunks );
             if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -max-chunks ...\n", 0 );
             if ( maxchunks >= 1 ) setMaxChunks( maxchunks );
          }

          else if ( strcmp ( argv[i], "-omp-scheduling" ) == 0 ||
                   ( strcmp ( argv[i], "-omps" ) == 0 && argv[i][5] == '\0') ) {
             i ++;
             if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -omp-scheduling, no argument\n", 0 );
             if ( strcmp ( argv[i], "default" ) == 0 ) {
               setOmpScheduling( _DEFAULT_OMP_SCHEDULING_ );
             }
             else if ( strcmp ( argv[i], "static" ) == 0 ) {
               setOmpScheduling( _STATIC_OMP_SCHEDULING_ );
             }
             else if ( strcmp ( argv[i], "dynamic-one" ) == 0 ) {
               setOmpScheduling( _DYNAMIC_ONE_OMP_SCHEDULING_ );
             }
             else if ( strcmp ( argv[i], "dynamic" ) == 0 ) {
               setOmpScheduling( _DYNAMIC_OMP_SCHEDULING_ );
             }
             else if ( strcmp ( argv[i], "guided" ) == 0 ) {
               setOmpScheduling( _GUIDED_OMP_SCHEDULING_ );
             }
             else {
               fprintf( stderr, "unknown omp scheduling type: '%s'\n", argv[i] );
               API_ErrorParse_applyTrsf( (char*)NULL, "parsing -omp-scheduling ...\n", 0 );
             }
          }

          /* general image related parameters
           */

          else if ( strcmp ( argv[i], "-r" ) == 0 && argv[i][2] == '\0' ) {
             r = 1;
          }
          else if ( strcmp ( argv[i], "-s" ) == 0 && argv[i][2] == '\0' ) {
             s = 1;
          }
          else if ( strcmp ( argv[i], "-o" ) == 0 && argv[i][2] == '\0' ) {
             i += 1;
             if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -o...\n", 0 );
             status = sscanf( argv[i],"%d",&o );
             if ( status <= 0 ) API_ErrorParse_applyTrsf( (char*)NULL, "parsing -o...\n", 0 );
          }
          else if ( strcmp ( argv[i], "-type" ) == 0 && argv[i][5] == '\0' ) {
            i += 1;
            if ( i >= argc)    API_ErrorParse_applyTrsf( (char*)NULL, "parsing -type...\n", 0 );
            if ( strcmp ( argv[i], "s8" ) == 0 && argv[i][2] == '\0' ) {
               p->output_type = SCHAR;
            }
            else if ( strcmp ( argv[i], "u8" ) == 0 && argv[i][2] == '\0' ) {
               p->output_type = UCHAR;
            }
            else if ( strcmp ( argv[i], "s16" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SSHORT;
            }
            else if ( strcmp ( argv[i], "u16" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = USHORT;
            }
            else if ( strcmp ( argv[i], "s32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SINT;
            }
            else if ( strcmp ( argv[i], "u32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = UINT;
            }
            else if ( strcmp ( argv[i], "s64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = SLINT;
            }
            else if ( strcmp ( argv[i], "u64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = ULINT;
            }
            else if ( strcmp ( argv[i], "r32" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = FLOAT;
            }
            else if ( strcmp ( argv[i], "r64" ) == 0 && argv[i][3] == '\0' ) {
              p->output_type = DOUBLE;
            }
            else {
              API_ErrorParse_applyTrsf( (char*)NULL, "parsing -type...\n", 0 );
            }
          }

          /* general parameters
           */
          else if ( (strcmp ( argv[i], "-help" ) == 0 && argv[i][5] == '\0')
                    || (strcmp ( argv[i], "--help" ) == 0 && argv[i][6] == '\0') ) {
             API_ErrorParse_applyTrsf( (char*)NULL, (char*)NULL, 1);
          }
          else if ( (strcmp ( argv[i], "-h" ) == 0 && argv[i][2] == '\0')
                    || (strcmp ( argv[i], "--h" ) == 0 && argv[i][3] == '\0') ) {
             API_ErrorParse_applyTrsf( (char*)NULL, (char*)NULL, 0);
          }
          else if ( strcmp ( argv[i], "-verbose" ) == 0
                    || (strcmp ( argv[i], "-v" ) == 0 && argv[i][2] == '\0') ) {
              if ( _verbose_ <= 0 ) _verbose_ = 1;
              else                  _verbose_ ++;
              incrementVerboseInReech4x4();
              incrementVerboseInReech4x4Coeff();
              incrementVerboseInReechDef();
              BAL_IncrementVerboseInBalTransformation();
              BAL_IncrementVerboseInBalTransformationTools();
          }
          else if ( strcmp ( argv[i], "-no-verbose" ) == 0
                    || strcmp ( argv[i], "-noverbose" ) == 0
                    || (strcmp ( argv[i], "-nv" ) == 0 && argv[i][3] == '\0') ) {
              _verbose_ = 0;
              setVerboseInReech4x4( 0 );
              setVerboseInReech4x4Coeff( 0 );
              setVerboseInReechDef( 0 );
              BAL_SetVerboseInBalTransformation( 0 );
              BAL_SetVerboseInBalTransformationTools( 0 );
          }
          else if ( (strcmp ( argv[i], "-debug" ) == 0 && argv[i][6] == '\0')
                    || (strcmp ( argv[i], "-D" ) == 0 && argv[i][2] == '\0') ) {
              if ( _debug_ <= 0 ) _debug_ = 1;
              else                _debug_ ++;
          }
          else if ( (strcmp ( argv[i], "-no-debug" ) == 0 && argv[i][9] == '\0')
                    || (strcmp ( argv[i], "-nodebug" ) == 0 && argv[i][8] == '\0') ) {
              _debug_ = 0;
          }

          else if ( strcmp ( argv[i], "-print-parameters" ) == 0
                    || (strcmp ( argv[i], "-param" ) == 0 && argv[i][6] == '\0') ) {
             p->print_lineCmdParam = 1;
          }

          else if ( strcmp ( argv[i], "-print-time" ) == 0
                     || (strcmp ( argv[i], "-time" ) == 0 && argv[i][5] == '\0') ) {
             p->print_time = 1;
          }
          else if ( (strcmp ( argv[i], "-notime" ) == 0 && argv[i][7] == '\0')
                      || (strcmp ( argv[i], "-no-time" ) == 0 && argv[i][8] == '\0') ) {
             p->print_time = 0;
          }

          /* unknown option
           */
          else {
              sprintf(text,"unknown option %s\n",argv[i]);
              API_ErrorParse_applyTrsf( (char*)NULL, text, 0);
          }
      }

      /* strings beginning with a character different from '-'
       */
      else {
          if ( strlen( argv[i] ) >= STRINGLENGTH ) {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_applyTrsf( (char*)NULL, "too long file name ...\n", 0 );
          }
          else if ( inputisread == 0 ) {
              (void)strcpy( p->input_name, argv[i] );
              inputisread = 1;
          }
          else if ( outputisread == 0 ) {
              (void)strcpy( p->output_name, argv[i] );
              outputisread = 1;
          }
          else {
              fprintf( stderr, "... parsing '%s'\n", argv[i] );
              API_ErrorParse_applyTrsf( (char*)NULL, "too many file names, parsing '-'...\n", 0 );
          }
      }
  }

  /* if not enough file names
   */
  if ( inputisread == 0 ) {
    (void)strcpy( p->input_name,  "<" );  /* standart input */
    inputisread = 1;
  }
  if ( outputisread == 0 ) {
    (void)strcpy( p->output_name,  ">" );  /* standart output */
    outputisread = 1;
  }


  /* output image type
   */
  if ( (o != 0) || (s != 0) || (r != 0) ) {
    if ( (o == 1) && (s == 1) && (r == 0) ) p->output_type = SCHAR;
    else if ( (o == 1) && (s == 0) && (r == 0) ) p->output_type = UCHAR;
    else if ( (o == 2) && (s == 0) && (r == 0) ) p->output_type = USHORT;
    else if ( (o == 2) && (s == 1) && (r == 0) ) p->output_type = SSHORT;
    else if ( (o == 4) && (s == 1) && (r == 0) ) p->output_type = SINT;
    else if ( (o == 0) && (s == 0) && (r == 1) ) p->output_type = FLOAT;
    else {
        API_ErrorParse_applyTrsf( (char*)NULL, "unable to determine uotput image type ...\n", 0 );
    }
  }

}
