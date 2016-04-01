#include "depth_converter.h"
#include <algorithm>

namespace mf { namespace node {

void depth_converter::process_() {
	Eigen_scalar temp1 = 1.0/z_near - 1.0/z_far;
	Eigen_scalar temp2 = 1.0/z_far;
	std::transform(
		input.view().begin(),
		input.view().end(),
		output.view().begin(),
		[&](mono_color col) -> Eigen_scalar {
			Eigen_scalar d = Eigen_scalar(col.intensity) / 255.0; // TODO generalize
			return 1.0 / (temp1*d + temp2);
		}
	);
}

}}


/*
  double distance, temp1, temp2;

  for(i=0; i<2; i++)
  {
    lMat_in[i] = cvCreateMat(3, 3, CV_64F); //intrinsic parameter of camera (3x3 matrix)
    lMat_ex_c2w[i] = cvCreateMat(3, 4, CV_64F); //extrinsic parameter of camera (3x4 matrix)
    lMat_proj_w2i[i] = cvCreateMat(3, 4, CV_64F); // projection matrix
  }

  if( !init_camera_param(Mat_In_Ref, Mat_Ex_Ref, Mat_Trans_Ref, Mat_In_Vir, Mat_Ex_Vir, Mat_Trans_Vir, lMat_in, lMat_ex_c2w, lMat_proj_w2i) )
  {
    for(i=0; i<2; i++)
    {
      cvReleaseMat(&lMat_in[i]);
      cvReleaseMat(&lMat_ex_c2w[i]);
      cvReleaseMat(&lMat_proj_w2i[i]);
    }
    return false;
  }

  int kernel[49]={0,0,1,1,1,0,0,
                  0,1,1,1,1,1,0,
                  1,1,1,1,1,1,1,
                  1,1,1,1,1,1,1,
                  1,1,1,1,1,1,1,
                  0,1,1,1,1,1,0,
                  0,0,1,1,1,0,0};
  m_pConvKernel = cvCreateStructuringElementEx(7, 7, 3, 3, CV_SHAPE_CUSTOM, kernel);

  temp1 = 1.0/Z_near - 1.0/Z_far;
  temp2 = 1.0/Z_far;

  switch(uiDepthType)
  {
  case 0:
    for(i=0; i<MAX_DEPTH; i++)
    {
      distance = 1.0 / ( double(i)*temp1/(MAX_DEPTH-1.0) + temp2 );
      //tableD2Z[i] = cvmGet(lMat_ex_c2w[0], 2, 2) * distance + cvmGet(lMat_ex_c2w[0], 2, 3);
      tableD2Z[i] = cvmGet(lMat_ex_c2w[0], 2, 2) * distance + Mat_Trans_Ref[2];
#ifdef POZNAN_GENERAL_HOMOGRAPHY
      m_dTableD2Z[i] = tableD2Z[i];
#endif
    }
    break;
  case 1:
    for(i=0; i<MAX_DEPTH; i++)
    {
      tableD2Z[i] = 1.0 / ( double(i)*temp1/(MAX_DEPTH-1.0) + temp2 );
*/
