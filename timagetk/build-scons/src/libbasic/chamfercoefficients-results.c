/*************************************************************************
 * chamfercoefficients-results.c - computation of chamfer distance coefficients
 *
 * $Id$
 *
 * Copyright (c) INRIA 2016
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Mer 16 mar 2016 16:21:18 CET
 *
 * COMMENTS
 *
 * This file is aimed at being included into an other file, not to
 * be compiled by itself.
 *
 * ADDITIONS, CHANGES
 *
 * -
 *
 *
 *
 *
 */

/**************************************************
 *
 * MATLAB
 *
 **************************************************/



void _print_2D_mask_matlab( FILE *f, VectorList *vl, ConeList *cl )
{
  int i;
  double emin, emax;
  double x1, y1, n1;
  double x2, y2, n2;

  emin = emax = cl->allocatedCones[0].error;
  for ( i=1; i<cl->n_cones; i++ ) {
    if ( emin > cl->allocatedCones[i].error )
      emin = cl->allocatedCones[i].error;
    if ( emax < cl->allocatedCones[i].error )
      emax = cl->allocatedCones[i].error;
  }
  if ( emax == emin ) emax += 0.01;
  
  fprintf( f, "\n" );
  fprintf( f, "emin=%f;\n", emin );
  fprintf( f, "emax=%f;\n", emax );
  fprintf( f, "\n" );

  fprintf( f, "figure\n" );
  fprintf( f, "hold on\n" );
  fprintf( f, "\n" );
  fprintf( f, "col=colormap(jet);\n" );
  fprintf( f, "cmin=1;\n" );
  fprintf( f, "cmax=size(col,1);\n" );
  fprintf( f, "alpha=0.75;\n");
  fprintf( f, "\n" );
  


  /* display unit vectors (in lattice coordinates)
   */

  fprintf( f, "if ( 1 )\n" );
  for (i=0; i<vl->n_vectors; i++ ) {
    x1 = vl->allocatedVectors[i].x;
    y1 = vl->allocatedVectors[i].y;
    n1 = sqrt( x1*x1 + y1*y1 );
    fprintf( f, "  plot([0,%f],[0,%f], 'k', 'LineWidth', 2 );\n", 
	     x1/n1, y1/n1 );
  }
  fprintf( f, "end\n" );
  fprintf( f, "\n" );
    


  /* display cones
   */

  for ( i=0; i<cl->n_cones; i++ ) {

    fprintf( f, "  ic=cmin+round( (log(%f)-log(emin))*(cmax-cmin)/(log(emax)-log(emin)) );\n",
	     cl->allocatedCones[i].error );

    x1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].x;
    y1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].y;
    x2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].x;
    y2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].y;
    n1 = sqrt( x1*x1 + y1*y1 );
    n2 = sqrt( x2*x2 + y2*y2 );

    fprintf( f, "  p=patch( [0 %f %f 0], [0 %f %f 0], col(ic,:) );\n",
	     x1/n1, x2/n2, y1/n1, y2/n2 );
    fprintf( f, "  set(p, 'FaceAlpha', alpha );\n" );
  }

  fprintf( f, "\n" );
  fprintf( f, "axis equal;\n" );
  fprintf( f, "axis( [0 1 0 1] );\n" );
  fprintf( f, "view(2);\n" );
  fprintf( f, "grid;\n" );
  fprintf( f, "cb = colorbar;\n" );
  fprintf( f, "ct = 0.01*round(10000*exp(log(emin):(log(emax)-log(emin))/10:log(emax)));\n" );
  fprintf( f, "set(cb,'YTickLabel',num2str(ct') );\n" );
  fprintf( f, "if ( 0 )\n" );
  fprintf( f, "  print( gcf, '-djpeg85', 'figure.jpg' );\n" );
    fprintf( f, "end\n" );
  fprintf( f, "\n" );
}



void _print_3D_mask_matlab( FILE *f, VectorList *vl, ConeList *cl )
{
  int i;
  double emin, emax;
  double x1, y1, z1, n1;
  double x2, y2, z2, n2;
  double x3, y3, z3, n3;
  
  emin = emax = cl->allocatedCones[0].error;
  for ( i=1; i<cl->n_cones; i++ ) {
    if ( emin > cl->allocatedCones[i].error )
      emin = cl->allocatedCones[i].error;
    if ( emax < cl->allocatedCones[i].error )
      emax = cl->allocatedCones[i].error;
  }
  if ( emax == emin ) emax += 0.01;



  fprintf( f, "\n" );
  fprintf( f, "emin=%f;\n", emin );
  fprintf( f, "emax=%f;\n", emax );
  fprintf( f, "longaxis=1;\n" );
  fprintf( f, "\n" );

  fprintf( f, "col_edge_convexe='k';\n");
  fprintf( f, "col_edge_concave='r';\n");
  fprintf( f, "\n" );

  fprintf( f, "alpha=0.90;\n");
  fprintf( f, "maximumlocalization=0;\n");
  fprintf( f, "\n" );

  fprintf( f, "displayvectors=0;\n" );
  fprintf( f, "printfigure=0;\n" );
  fprintf( f, "figurename='FIGURENAME';\n" );
  fprintf( f, "\n" );

  
  /* to display inner and outer normals
   */
  if ( 1 ) { 

    fprintf( f, "if ( maximumlocalization )\n" );
    fprintf( f, "\n" );
    
    fprintf( f, "  col_max_inside='b';\n");
    fprintf( f, "  col_max_outside='g';\n");
    fprintf( f, "  col_max_undef='k';\n");
    fprintf( f, "\n" );
    
    fprintf( f, "  figure\n" );
    fprintf( f, "  hold on\n" );
    fprintf( f, "\n" );

    for ( i=0; i<cl->n_cones; i++ ) {

      x1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].x;
      y1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].y;
      z1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].z;
      x2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].x;
      y2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].y;
      z2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].z;
      x3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].x;
      y3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].y;
      z3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].z;
      n1 = sqrt( x1*x1 + y1*y1 + z1*z1 );
      n2 = sqrt( x2*x2 + y2*y2 + z2*z2 );
      n3 = sqrt( x3*x3 + y3*y3 + z3*z3 );
      
      switch( cl->allocatedCones[i].maximumType ) {
      case _UNSET_ :
	fprintf( f, "  p=patch( [%f %f %f %f], [%f %f %f %f], [%f %f %f %f], col_max_undef, 'EdgeColor', 'none' );\n",
		 x1/n1, x2/n2, x3/n3, x1/n1,
		 y1/n1, y2/n2, y3/n3, y1/n1,
		 z1/n1, z2/n2, z3/n3, z1/n1 );
	break;
      case _INSIDE_ :
	fprintf( f, "  p=patch( [%f %f %f %f], [%f %f %f %f], [%f %f %f %f], col_max_inside, 'EdgeColor', 'none' );\n",
		 x1/n1, x2/n2, x3/n3, x1/n1,
		 y1/n1, y2/n2, y3/n3, y1/n1,
		 z1/n1, z2/n2, z3/n3, z1/n1 );
	break;
      case _EDGE_IJ_ :
      case _EDGE_JK_ :
      case _EDGE_KI_ :
	fprintf( f, "  p=patch( [%f %f %f %f], [%f %f %f %f], [%f %f %f %f], col_max_outside, 'EdgeColor', 'none' );\n",
		 x1/n1, x2/n2, x3/n3, x1/n1,
		 y1/n1, y2/n2, y3/n3, y1/n1,
		 z1/n1, z2/n2, z3/n3, z1/n1 );
	break;
      }

      fprintf( f, "  set(p, 'FaceAlpha', alpha );\n" );

      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ ) {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x1/n1, x2/n2, y1/n1, y2/n2, z1/n1, z2/n2 );
      } 
      else {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x1/n1, x2/n2, y1/n1, y2/n2, z1/n1, z2/n2 );
      }
      
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ 
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ ) {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x2/n2, x3/n3, y2/n2, y3/n3, z2/n2, z3/n3 );
      } 
      else {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x2/n2, x3/n3, y2/n2, y3/n3, z2/n2, z3/n3 );
      }
      
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ 
	   || cl->allocatedCones[i].convexity == _EDGE_KI_ ) {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x3/n3, x1/n1, y3/n3, y1/n1, z3/n3, z1/n1 );
      }
      else {
	fprintf( f, "  plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x3/n3, x1/n1, y3/n3, y1/n1, z3/n3, z1/n1 );
      }
      fprintf( f, "\n" );
    }

    fprintf( f, "\n" );
    fprintf( f, "  if ( longaxis )\n" );
    fprintf( f, "    axis( [0 1 0 1 0 1] );\n" );
    fprintf( f, "  else\n" );
    fprintf( f, "    axis( [0.4 1 0 0.8 0 0.6] );\n" );
    fprintf( f, "  end\n" );
    fprintf( f, "  set( gca, 'LineWidth', 2 );\n" );
    fprintf( f, "\n" );
    fprintf( f, "%% axis equal;\n" );
    fprintf( f, "  view( [110 20]);\n" );
    fprintf( f, "  grid;\n" );
    fprintf( f, "  if (0)\n" );
    fprintf( f, "    print( gcf, '-djpeg85', 'figure.jpg' );\n" );
    fprintf( f, "  end\n" );
    fprintf( f, "  hold off\n" );
    fprintf( f, "\n" );
    fprintf( f, "end\n" );
  }

  
  
  if ( 0 ) {
    fprintf( f, "if (0)\n" );
    fprintf( f, "  figure\n" );
    fprintf( f, "  hold on\n" );
    fprintf( f, "  set(gca, 'FontWeight','bold','FontSize', 18 );\n" );
    fprintf( f, "\n" );
    fprintf( f, "  colp = [0.5 0.5 0.5];\n" );
    fprintf( f, "\n" );
    for ( i=0; i<cl->n_cones; i++ ) {
      
      if ( 1 ) {
	x1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].x;
	y1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].y;
	z1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].z;
	x2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].x;
	y2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].y;
	z2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].z;
	x3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].x;
	y3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].y;
	z3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].z;
	n1 = sqrt( x1*x1 + y1*y1 + z1*z1 );
	n2 = sqrt( x2*x2 + y2*y2 + z2*z2 );
	n3 = sqrt( x3*x3 + y3*y3 + z3*z3 );
	fprintf( f, "    p=patch( [0 %f %f 0], [0 %f %f 0], [0 %f %f 0], colp );\n",
		 x2/n2, x3/n3, y2/n2, y3/n3, z2/n2, z3/n3 );
	fprintf( f, "%%    set(p, 'FaceAlpha', alpha );\n" );
	fprintf( f, "    p=patch( [0 %f %f 0], [0 %f %f 0], [0 %f %f 0], colp );\n",
		 x3/n3, x1/n1, y3/n3, y1/n1, z3/n3, z1/n1 );
	fprintf( f, "%%    set(p, 'FaceAlpha', alpha );\n" );
	fprintf( f, "    p=patch( [0 %f %f 0], [0 %f %f 0], [0 %f %f 0], colp );\n",
		 x1/n1, x2/n2, y1/n1, y2/n2, z1/n1, z2/n2 );
	fprintf( f, "%%    set(p, 'FaceAlpha', alpha );\n" );
      }
      
    }
    fprintf( f, "\n" );
    for (i=0; i<vl->n_vectors; i++ ) {
      if ( 1 ) {
	x1 = vl->allocatedVectors[i].x;
	y1 = vl->allocatedVectors[i].y;
	z1 = vl->allocatedVectors[i].z;
	n1 = sqrt( x1*x1 + y1*y1 + z1*z1 );
	fprintf( f, "    plot3([%f %f],[%f %f],[%f %f], 'k', 'LineWidth', 3 );\n", 
		 x1/n1, 1.1 * x1/n1, y1/n1, 1.1 * y1/n1, z1/n1, 1.1 * z1/n1 );
      }
    }
    fprintf( f, "\n" );
    fprintf( f, "  if ( longaxis )\n" );
    fprintf( f, "    axis( [0 1.1 0 1.1 0 1.1] );\n" );
    fprintf( f, "  else\n" );
    fprintf( f, "    axis( [0.4 1 0 0.8 0 0.6] );\n" );
    fprintf( f, "  end\n" );
    fprintf( f, "\n" );
    fprintf( f, "  %% axis equal;\n" );
    fprintf( f, "  view( [110 20]);\n" );
    fprintf( f, "  grid;\n" );
    fprintf( f, "\n" );
    fprintf( f, "  if (0)\n" );
    fprintf( f, "    print( gcf, '-djpeg85', 'figure.jpg' );\n" );
    fprintf( f, "  end\n" );
    fprintf( f, "\n" );
    fprintf( f, "  hold off\n" );
    fprintf( f, "end\n" );
    fprintf( f, "\n" );
  }






  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );

  fprintf( f, "figure\n" );
  fprintf( f, "hold on\n" );
  fprintf( f, "set(gca, 'FontWeight','bold','FontSize', 18 );\n" );
  fprintf( f, "\n" );
  fprintf( f, "col=colormap(jet);\n" );
  fprintf( f, "cmin=1;\n" );
  fprintf( f, "cmax=size(col,1);\n" );
  fprintf( f, "\n" );
  fprintf( f, "cb = colorbar;\n" );

  fprintf( f, "ct = 0.01*round(10000*exp(log(emin):(log(emax)-log(emin))/5:log(emax)));\n" );
  fprintf( f, "set(cb,'YTick',cmin+1:(cmax-cmin-2)/5:cmax-1);\n" );
  fprintf( f, "set(cb,'YTickLabel',num2str(ct'), 'FontWeight','bold','FontSize', 24 );\n" );
  fprintf( f, "\n" );

  fprintf( f, "if ( displayvectors )\n" );

  for (i=0; i<vl->n_vectors; i++ ) {

    x1 = vl->allocatedVectors[i].x;
    y1 = vl->allocatedVectors[i].y;
    z1 = vl->allocatedVectors[i].z;
    n1 = sqrt( x1*x1 + y1*y1 + z1*z1 );
    fprintf( f, "  plot3([0,%f],[0,%f],[0,%f], 'k', 'LineWidth', 2 );\n", 
	     x1/n1, y1/n1, z1/n1 );
  }

  fprintf( f, "end\n" );
  fprintf( f, "\n" );
    
  for ( i=0; i<cl->n_cones; i++ ) {

    fprintf( f, "ic=cmin+round( (log(%f)-log(emin))*(cmax-cmin)/(log(emax)-log(emin)) );\n",
	     cl->allocatedCones[i].error );
  
    x1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].x;
    y1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].y;
    z1 = vl->allocatedVectors[cl->allocatedCones[i].iv1].z;
    x2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].x;
    y2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].y;
    z2 = vl->allocatedVectors[cl->allocatedCones[i].iv2].z;
    x3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].x;
    y3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].y;
    z3 = vl->allocatedVectors[cl->allocatedCones[i].iv3].z;
    n1 = sqrt( x1*x1 + y1*y1 + z1*z1 );
    n2 = sqrt( x2*x2 + y2*y2 + z2*z2 );
    n3 = sqrt( x3*x3 + y3*y3 + z3*z3 );
    fprintf( f, "p=patch( [%f %f %f %f], [%f %f %f %f], [%f %f %f %f], col(ic,:), 'EdgeColor', 'none' );\n",
	     x1/n1, x2/n2, x3/n3, x1/n1,
	     y1/n1, y2/n2, y3/n3, y1/n1,
	     z1/n1, z2/n2, z3/n3, z1/n1 );
    
    fprintf( f, "set(p, 'FaceAlpha', alpha );\n" );
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ ) {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x1/n1, x2/n2, y1/n1, y2/n2, z1/n1, z2/n2 );
      } 
      else {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x1/n1, x2/n2, y1/n1, y2/n2, z1/n1, z2/n2 );
      }
      
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_JK_
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ 
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ ) {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x2/n2, x3/n3, y2/n2, y3/n3, z2/n2, z3/n3 );
      } 
      else {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x2/n2, x3/n3, y2/n2, y3/n3, z2/n2, z3/n3 );
      }
      
      if ( cl->allocatedCones[i].convexity == _EDGE_IJ_ +_EDGE_JK_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_IJ_ + _EDGE_KI_
	   || cl->allocatedCones[i].convexity == _EDGE_JK_ + _EDGE_KI_ 
	   || cl->allocatedCones[i].convexity == _EDGE_KI_ ) {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_convexe, 'LineWidth', 2 );\n", 
		 x3/n3, x1/n1, y3/n3, y1/n1, z3/n3, z1/n1 );
      }
      else {
	fprintf( f, "plot3([%f %f], [%f %f], [%f %f], col_edge_concave, 'LineWidth', 2 );\n", 
		 x3/n3, x1/n1, y3/n3, y1/n1, z3/n3, z1/n1 );
      }

      fprintf( f, "\n" );
  }
  
  fprintf( f, "\n" );
  fprintf( f, "if ( longaxis )\n" );
  fprintf( f, "  axis( [0 1 0 1 0 1] );\n" );
  fprintf( f, "else\n" );
  fprintf( f, "  axis( [0.4 1 0 0.8 0 0.6] );\n" );
  fprintf( f, "end\n" );
  fprintf( f, "set( gca, 'LineWidth', 2 );\n" );
  fprintf( f, "\n" );
  fprintf( f, "%% axis equal;\n" );
  fprintf( f, "view( [110 20]);\n" );
  fprintf( f, "grid;\n" );
  fprintf( f, "\n" );
  fprintf( f, "if ( printfigure )\n" );
  fprintf( f, "  print( gcf, '-djpeg85', strcat(figurename, '.jpg') );\n" );
  fprintf( f, "end\n" );
  fprintf( f, "\n" );
  fprintf( f, "hold off\n" );
}










/**************************************************
 *
 * TXT
 *
 **************************************************/



void _print_cone_information( FILE *fout, double optimal_err, 
			       double previous_err,
			       double err, double dmin,
			       double dmax, double epsilon,
			      double ct, double it )
{
  fprintf( fout, "\n" );
  fprintf( fout, "Error = %f   %g %% / OPT --- Epsilon = %f", 
	   err, 100.0*err/optimal_err, epsilon );
  fprintf( fout, " --- Elapsed time = %f ms",
	   (ct - it) * 10000 * 1000 / (double)CLOCKS_PER_SEC );
  fprintf( fout, "\n" );
  if ( _verbose_ >= 3 ) {
    fprintf( fout, "--- Incr = %g\n", previous_err - err );
  }
  fprintf( fout, "--- dmin = %f   dmax = %f", dmin, dmax );
  fprintf( fout, "\n" );
}



void _print_mask_information( FILE *fout, double optimal_err, 
			       double previous_err,
			       double err, double dmin,
             double dmax,
			       double lerr, double ldmin,
			       double ldmax, double lepsilon,
			      double ct, double it )
{
  fprintf( fout, "\n" );
  fprintf( fout, "Error = %f   %g %% / OPT --- Epsilon = %f", 
	   lerr, 100.0*lerr/optimal_err, lepsilon );
  fprintf( fout, " --- Elapsed time = %f ms",
	   (ct - it) * 10000 * 1000 / (double)CLOCKS_PER_SEC );
  fprintf( fout, "\n" );
  if ( _verbose_ >= 3 ) {
    fprintf( fout, "--- Incr = %g\n", previous_err - lerr );
    fprintf( fout, "--- Expected = %f   Diff = %g\n", 
	     err, lerr-err);
  }
  fprintf( fout, "--- dmin = %f   dmax = %f", ldmin, ldmax );
  if ( _verbose_ >= 3 ) {
    fprintf( fout, "   (Exp dmin = %f dmax =%f)", dmin, dmax );
  }
  fprintf( fout, "\n" );
}



void _print_2D_weighted_vector_list( FILE *f, WeightedVectorList *wvl, int n )
{
  int i;
  for (i=0; i<n; i++ ) {
    fprintf( f, "    v[#%2d] = ", i );
    _print_2D_vector( f, wvl->allocatedVectors[i].vec );
    fprintf( f, "   w=%4d\n", wvl->allocatedVectors[i].w );
  }
}


void _print_3D_weighted_vector_list( FILE *f, WeightedVectorList *wvl, int n )
{
  int i;
  for (i=0; i<n; i++ ) {
    fprintf( f, "    v[#%2d] = ", i );
    _print_3D_vector( f, wvl->allocatedVectors[i].vec );
    fprintf( f, "   w=%4d\n", wvl->allocatedVectors[i].w );
  }
}


void _print_weights_list( FILE *f, WeightsList *wl,  VectorList *vl )
{
  char *proc = "_print_weights_list";
  int *p;
  int i, j;
  int z;

  Vector **v = NULL;

  v = (Vector**)malloc( vl->n_vectors * sizeof(Vector*) );
  if ( v == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return;
  }
  for ( i=0; i<vl->n_vectors; i++ ) {
    vl->allocatedVectors[i].index = i;
    v[i] = &(vl->allocatedVectors[i]);
  }
  _sort_vectors_in_lexicographic_order( v, 0, vl->n_vectors-1 );
  
  p = (int*)malloc( vl->n_vectors * sizeof(int) );
  if ( p == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    free( v );
    return;
  }
  for ( i=0; i<vl->n_vectors; i++ ) {
    p[i] = v[i]->index;
  }
  free( v );

  fprintf( f, "\n" );
  fprintf( f, " Best possible error = %f\n", wl->error_optimal );
  fprintf( f,"   " );
  for ( i=0; i<vl->n_vectors; i++ ) 
    fprintf( f,"   [%2d]", vl->allocatedVectors[p[i]].x );
  fprintf( f, "\n" );
  fprintf( f,"   " );
  for ( i=0; i<vl->n_vectors; i++ ) 
    fprintf( f,"   [%2d]", vl->allocatedVectors[p[i]].y );
  fprintf( f,"  ---       err      %%/opt      eps         time\n" );


  z = vl->allocatedVectors[0].z;
  for ( i=1; i<vl->n_vectors; i++ ) 
    if ( z < vl->allocatedVectors[p[i]].z ) z = vl->allocatedVectors[p[i]].z;
  if ( z > 0 ) {
    fprintf( f,"   " );
    for ( i=0; i<vl->n_vectors; i++ ) 
      fprintf( f,"   [%2d]", vl->allocatedVectors[p[i]].z );
    fprintf( f, "\n" );
  }

 fprintf( f, "\n" );

 for ( j=0; j<wl->n_weights; j++ ) {
   fprintf( f,"#%2d", j );
   for ( i=0; i<vl->n_vectors; i++ ) 
     fprintf( f,"  %4d ", wl->allocatedWeights[j].w[p[i]] );
   fprintf( f,"  ---    %9.6f (%6.1f)  %9.6f  %9.2f\n",
	    wl->allocatedWeights[j].error * 100.0,
	    wl->allocatedWeights[j].error * 100.0 / wl->error_optimal,
	    wl->allocatedWeights[j].epsilon,
	    wl->allocatedWeights[j].elapsed_time );
 }

 free( p );
}



int *_build_buffer_mask( int *dims, VectorList *vl, ConeList *cl, double *voxel_size, int anisotropy_y, int anisotropy_z )
{
  char *proc = "_build_buffer_mask";
  VectorList nvl;
  ConeList ncl;
  int notinplane;
  
  int mx=0, my=0, mz=0;
  int dx, dy, dz;
  int x, y, z;
  int i;
  int *buf;

  if ( _trace_ >= 3 )
    _print_3D_vector_list( stdout, vl );


  _init_vector_list( &nvl );
  _init_cone_list( &ncl );
  
  notinplane = 0;
  for ( i=0; i<vl->n_vectors; i++ ) {
    (void)_add_vector_to_list( &nvl, &(vl->allocatedVectors[i]) );
    if ( vl->allocatedVectors[i].z != 0 ) notinplane++;
  }
  for ( i=0; i<cl->n_cones; i++ ) {
    (void)_add_cone_to_list( &ncl, &(cl->allocatedCones[i]) );
  }
  
  if ( !notinplane ) 
    _complete_2D_mask_in_quadrant( &nvl, &ncl, voxel_size, anisotropy_y );
  else 
    _complete_3D_mask_in_octant( &nvl, &ncl, voxel_size, anisotropy_y, anisotropy_z );

  if ( _trace_ >= 3 )
  _print_3D_vector_list( stdout, &nvl );


  
  for ( i=0; i<nvl.n_vectors; i++ ) {
    if ( mx < nvl.allocatedVectors[i].x ) mx = nvl.allocatedVectors[i].x;
    if ( my < nvl.allocatedVectors[i].y ) my = nvl.allocatedVectors[i].y;
    if ( mz < nvl.allocatedVectors[i].z ) mz = nvl.allocatedVectors[i].z;
  }
  
  dims[0] = dx = 2*mx+1;
  dims[1] = dy = 2*my+1;
  dims[2] = dz = 2*mz+1;

  buf = (int*)malloc( dx*dy*dz * sizeof(int) );
  if ( buf == (int*)NULL ) {
    fprintf( stderr, "%s: allocation failed\n", proc );
    return( NULL );
  }
  for ( i=0; i<dx*dy*dz; i++ ) buf[i] = 0;
  
  for ( i=0; i<nvl.n_vectors; i++ ) {
    x = nvl.allocatedVectors[i].x;
    y = nvl.allocatedVectors[i].y;
    z = nvl.allocatedVectors[i].z;
    buf[ (mz+z)*dx*dy + (my+y)*dx + (mx+x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz+z)*dx*dy + (my+y)*dx + (mx-x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz+z)*dx*dy + (my-y)*dx + (mx+x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz+z)*dx*dy + (my-y)*dx + (mx-x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz-z)*dx*dy + (my+y)*dx + (mx+x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz-z)*dx*dy + (my+y)*dx + (mx-x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz-z)*dx*dy + (my-y)*dx + (mx+x) ] = nvl.allocatedVectors[i].w;
    buf[ (mz-z)*dx*dy + (my-y)*dx + (mx-x) ] = nvl.allocatedVectors[i].w;
  }

  _free_vector_list( &nvl );
  _free_cone_list( &ncl );

  return( buf );

}


void _print_mask( FILE *f, int *buf, int *dims )
{
  int x, y, z;

  for ( z=0; z<dims[2]; z++ ) {
    fprintf( f, "\n" );
    for ( y=0; y<dims[1]; y++ ) {
      for ( x=0; x<dims[0]; x++ ) {
	fprintf( f, " %3d", buf[z*dims[0]*dims[1] + y*dims[0] + x] );
      }
      fprintf( f, "\n" );
    }
  }
  fprintf( f, "\n" );
}









/**************************************************
 *
 * LATEX
 *
 **************************************************/



void _print_weights_list_in_latex( FILE *f, WeightsList *wl,  VectorList *vl )
{
  char *proc = "_print_weights_list_in_latex";
  int *p;
  int i, j;
  int z;
  int min, s;

  Vector **v = NULL;

  v = (Vector**)malloc( vl->n_vectors * sizeof(Vector*) );
  if ( v == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    return;
  }
  for ( i=0; i<vl->n_vectors; i++ ) {
    vl->allocatedVectors[i].index = i;
    v[i] = &(vl->allocatedVectors[i]);
  }
  _sort_vectors_in_lexicographic_order( v, 0, vl->n_vectors-1 );
  
  p = (int*)malloc( vl->n_vectors * sizeof(int) );
  if ( p == NULL ) {
    fprintf( stderr, "%s: allocation error\n", proc );
    free( v );
    return;
  }
  for ( i=0; i<vl->n_vectors; i++ ) {
    p[i] = v[i]->index;
  }
  free( v );

  fprintf( f, "\n" );
  fprintf( f, "\\begin{table}\n" );
  fprintf( f, "\\begin{center}\n" );
  fprintf( f, "\\small\n" );
  fprintf( f, "\\begin{tabular}{|" );
  for ( i=0; i<vl->n_vectors; i++ ) {
    fprintf( f, "c" );
    if ( i<vl->n_vectors-1 ) fprintf( f, "|" );
  }
  fprintf( f, "|c|c|c|c|}\n" );
  fprintf( f, "\\hline\n" );
  
  z = vl->allocatedVectors[0].z;
  for ( i=1; i<vl->n_vectors; i++ ) 
    if ( z < vl->allocatedVectors[p[i]].z ) z = vl->allocatedVectors[p[i]].z;

  for ( i=0; i<vl->n_vectors; i++ ) {
    fprintf( f,"\\tiny " );
    fprintf( f,"$ " );
    fprintf( f,"\\!\\!\\!\\!" );
    if ( 0 ) fprintf( f,"\\left\( " );
    fprintf( f,"\\begin{array}{c}" );
    fprintf( f," %d \\\\ %d", vl->allocatedVectors[p[i]].x, vl->allocatedVectors[p[i]].y );
    if ( z > 0 ) 
      fprintf( f," \\\\ %d ", vl->allocatedVectors[p[i]].z );
    fprintf( f,"\\end{array} ");
    if ( 0 ) fprintf( f,"\\right) " );
    fprintf( f,"\\!\\!\\!\\!" );
    fprintf( f,"$ &\n" );
  }
  fprintf( f," error & \\%% / opt & $\\varepsilon$ & time \\\\ \\hline\n" );
  
 for ( j=0; j<wl->n_weights; j++ ) {
   for ( i=0; i<vl->n_vectors; i++ ) {
     fprintf( f,"  %4d & ", wl->allocatedWeights[j].w[p[i]] );
     if ( (i+1) % 10 == 0 ) fprintf( f,"\n" );
   }
   fprintf( f, "\n" );
   fprintf( f," %5.2f & %6.1f & %6.3f &\n",
	    wl->allocatedWeights[j].error * 100.0,
	    wl->allocatedWeights[j].error * 100.0 / wl->error_optimal,
	    wl->allocatedWeights[j].epsilon);
   if ( wl->allocatedWeights[j].elapsed_time < 1000 ) {
     fprintf( f, " %d ms ", (int)wl->allocatedWeights[j].elapsed_time );
   }
   else {
     if ( wl->allocatedWeights[j].elapsed_time < 60 * 1000 ) {
       fprintf( f, " %5.2f s ", wl->allocatedWeights[j].elapsed_time / 1000 );
     }
     else {
       if ( wl->allocatedWeights[j].elapsed_time < 10 * 60 * 1000 ) {
	 min = (int)(wl->allocatedWeights[j].elapsed_time/(60*1000));
	 s = (int)((wl->allocatedWeights[j].elapsed_time - min * 60 * 1000)/1000);
	 fprintf( f, " %d min %d s ", min, s );
       }
       else {
	 min = (int)(wl->allocatedWeights[j].elapsed_time/(60*1000));
	 if ( min < 60 ) {
	   fprintf( f, " %d min ", min );
	 }
	 else {
	   fprintf( f, " %d h %d min ", min / 60, min % 60 );
	 }
       }
     }
   }
   fprintf( f," \\\\ \\hline\n" );
 }

  fprintf( f, "\\end{tabular}\n" );
  fprintf( f, "\\end{center}\n" );
  fprintf( f, "\\caption{Best possible error = %f.}\n", wl->error_optimal );
  fprintf( f, "\\end{table}\n\n" );


 free( p );
}
