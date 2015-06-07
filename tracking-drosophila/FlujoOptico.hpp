/*
 * LKOpticalFlow.hpp
 *
 *  Created on: 31/10/2011
 *      Author: chao
 */

#ifndef LKOPTICALFLOW_HPP_
#define LKOPTICALFLOW_HPP_

#include "VideoTracker.hpp"
#include "opencv2/video/tracking.hpp"

void LKOptFlow( IplImage* ImLast, IplImage* velX, IplImage* velY);

void PLKOptFlow( IplImage* imgA, IplImage* imgB, IplImage* imgC);

#endif /* LKOPTICALFLOW_HPP_ */


