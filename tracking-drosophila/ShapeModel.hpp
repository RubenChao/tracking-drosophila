/*
 * ShapeModel.hpp
 *
 *  Created on: 19/09/2011
 *      Author: chao
 */

#ifndef SHAPEMODEL_HPP_
#define SHAPEMODEL_HPP_

#include "VideoTracker.hpp"
#include "Libreria.h"
#include "BGModel.h"
#include "Procesado.hpp"
#include <BlobResult.h>
#include <Blob.h>

#define SM_FRAMES_TRAINING 200 /// Numero de frames utilizados para los calculos del modelado de forma.

//!\brief ShapeModel: Establece los valores del area media y la desviacion tipica de las areas
//! del total de los blobs detectados en los frames_training, con el fin de realizar el
//! modelado de forma para una posterior segementación y validacion de los blobs.
/*!
 * \param g_capture frame actual.
 * \param SH estructura que contiene los datos de la mediana y la desviciaón típica de las areas.
 * \param ImMask Imagen de 8 bits que contiene la mascara del plato.
 * \param ROI contiene los datos para establcer la region de interes ( Plato).
 *
 * \return Los valores de la mediana y la desvicion tipica de las areas de los blobs.
 *
 * \see ViodeoTracker.hpp
 *
 */
void ShapeModel( CvCapture* g_capture, SHModel *SH,IplImage* ImMask, CvRect ROI );

SHModel* ShapeModel2( CvCapture* g_capture,StaticBGModel* BGModel );

#endif /* SHAPEMODEL_HPP_ */
