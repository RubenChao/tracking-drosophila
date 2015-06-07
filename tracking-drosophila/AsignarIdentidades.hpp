/*
 * AsignarIdentidades.hpp
 *
 *  Created on: 02/09/2011
 *      Author: chao
 */

#ifndef ASIGNARIDENTIDADES_HPP_
#define ASIGNARIDENTIDADES_HPP_

#include "VideoTracker.hpp"
#include "Tracking.hpp"
#include "Kalman.hpp"
#include "Libreria.h"
#include "opencv2/video/tracking.hpp"

///!  brief -Resuelve la ambiguedad en la orientación mediante el cálculo
///!    del gradiente global del movimiento de la escena tras ser segmentado
///!    en movimientos locales.
///!   -Hace una primera asignación de identidad mediante una plantilla de movimiento.
void MotionTemplate( tlcde* framesBuf,tlcde* Etiquetas );

///! brief Realiza la asignación de identidades. El primer parámetro es la lista
///! el segundo parámetro es un flag que indica qué individuos asignar. 1 para los
///! del foreground ( estado dinámico )  y 0 para los del oldforeground ( estado estático )

STFly* matchingIdentity( STFrame* frameActual , STFrame*frameAnterior, tlcde* ids , CvRect MotionRoi, double angle );

int asignarIdentidades( tlcde* lsTraks , tlcde *Flies);

int asignarIdentidades2( tlcde* lsTraks , tlcde *Flies);

int enlazarFlies( STFly* flyAnterior, STFly* flyActual);

void corregirEstado( STFrame* frame0, STFrame* frame1, STFrame* frame2, int pos );

void establecerEstado( STFrame* frame0, STFrame* frame1, STFrame* frame2,IplImage* orient);

int buscarFlies( STFrame* frameData ,CvRect MotionRoi, int *p );

void allocateMotionTemplate( IplImage* im);

void releaseMotionTemplate();

void TrackbarSliderMHI(  int pos );

void TrackbarSliderDMin(  int pos );

void TrackbarSliderDMax(  int pos );

double PesosKalman(const CvMat* Matrix,const CvMat* Predict,CvMat* CordReal);

#endif /* ASIGNARIDENTIDADES_HPP_ */
