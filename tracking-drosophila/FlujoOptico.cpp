/*
 * FlujoOptico.cpp
 *
 *  Created on: 14/09/2011
 *      Author: chao
 */

#include "FlujoOptico.hpp"



void LKOptFlow( IplImage* ImLast,IplImage* velX,IplImage* velY ){

//	IplImage **buf = 0;
//	int N = 2; // tamaño del bufer
//	int last = N-1;
//	int prev = N-2;
//
//	CvSize size = cvSize(ImLast->width,ImLast->height); // get current frame size
//
//
//	if( buf == 0 ) {
//		buf = (IplImage**)malloc(N*sizeof(buf[0]));
//		memset( buf, 0, N*sizeof(buf[0]));
//
//		for( int i = 0; i < N; i++ ) {
//			buf[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
//			cvZero( buf[i] );
//		}
//		cvCopy(ImLast, buf[last]);
//		return;
//	}
//	cvCopy( buf[last], buf[prev]);
//	cvCopy( ImLast, buf[last]);
//
//	cvCalcLKOptFlow(buf[prev], buf[last], cvSize(3, 3), velX, velY);

}

void PLKOptFlow( IplImage* imgA, IplImage* imgB, IplImage* imgC){

	const int MAX_CORNERS = 500;
	CvSize img_sz = cvGetSize( imgA );
	int	win_size = 10;

	// The first thing we need to do is get the features
	// we want to track.
	//
	IplImage* eig_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	IplImage* tmp_image = cvCreateImage( img_sz, IPL_DEPTH_32F, 1 );
	int	corner_count = MAX_CORNERS;
	CvPoint2D32f* cornersA	= new CvPoint2D32f[ MAX_CORNERS ];

	cvGoodFeaturesToTrack(
	imgA,
	eig_image,
	tmp_image,
	cornersA,
	&corner_count,
	0.01,
	5.0,
	0,
	3,
	0,
	0.04
	);
	cvFindCornerSubPix(
	imgA,
	cornersA,
	corner_count,
	cvSize(win_size,win_size),
	cvSize(-1,-1),
	cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03)
	);
	// Call the Lucas Kanade algorithm
	//
	char features_found[ MAX_CORNERS ];
	float feature_errors[ MAX_CORNERS ];
	CvSize pyr_sz = cvSize( imgA->width+8, imgB->height/3 );
	IplImage* pyrA = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	IplImage* pyrB = cvCreateImage( pyr_sz, IPL_DEPTH_32F, 1 );
	CvPoint2D32f* cornersB	= new CvPoint2D32f[ MAX_CORNERS ];

	cvCalcOpticalFlowPyrLK(
	imgA,
	imgB,
	pyrA,
	pyrB,
	cornersA,
	cornersB,
	corner_count,
	cvSize( win_size,win_size ),
	5,
	features_found,
	feature_errors,
	cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ),
	0
	);

	// Now make some image of what we are looking at:
	//
	for( int i=0; i<corner_count; i++ ) {
		if( features_found[i]==0|| feature_errors[i]>550 ) {
			printf("Error is %f/n",feature_errors[i]);
			continue;
		}
		printf("Got it/n");
		CvPoint p0 = cvPoint(
		cvRound( cornersA[i].x ),
		cvRound( cornersA[i].y )
		);
		CvPoint p1 = cvPoint(
		cvRound( cornersB[i].x ),
		cvRound( cornersB[i].y )
		);
		cvLine( imgC, p0, p1, CV_RGB(255,0,0),2 );
	}
}
