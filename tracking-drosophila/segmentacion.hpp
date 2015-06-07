/*
 * segmentacion.hpp
 *
 *  Created on: 29/08/2011
 *      Author: german
 */

#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_

#include "VideoTracker.hpp"
#include "Libreria.h"

//!\brief segmentacion: Ajusta cada blob a una elipse.
//!\n
//!\n Los parámetros de la elipse se obtienen a partir de la gaussiana 2d del blob mediante la
//! eigendescomposición de su matriz de covarianza. Los parámetros de la gaussiana guardan
//! proporción con el peso de cada pixel siendo este peso proporcional a la distancia normalizada
//! de la intensidad del pixel y el valor probable del fondo de ese pixel,de modo que el centro
//! de la elipse conicidirá con el valor esperado de la gausiana 2d (la región más oscura) que a
//! su vez coincidirá aproximadamente con el centro del blob.
//!\n
//!\n Una vez hecho el ajuste se rellena una estructura donde se almacenan los parámetros que definen
//! las características del blob ( identificacion, posición, orientación, semieje a, semieje b y roi
//! entre otras).
/*!
 * \param Brillo Imagen fuente de 8 bits de niveles de gris del frame actual.
 * \param BGModel Modelo de fondo.Contiene el valor medio de cada píxel
 * \param FG Foreground
 * \param Roi Región a segmentar
 * \param Opcionalmete se le puede pasar una máscara.
 *
 * \return La Lista con los blobs detectados y aproximados a una elipse en el frame.
 *
 */

tlcde* segmentacion2( IplImage *Brillo, IplImage* BGModel, IplImage* FG ,CvRect Roi,IplImage* Mask);

//!\brief Crea las imagenes, matrices y estructuras utilizadas en la segementación.
/*!
 * \param Brillo Imagen fuente de 8 bits de nivel de gris, contiene el frame actual.
 */
void CreateDataSegm( IplImage* Brillo );

//!\brief // realiza el ajuste a una elipse de los pesos del rectangulo que coincidan con la máscara
//! y rellena los datos de la estructura fly. Si no es posible el ajuste, devuelve null
/*!
 * \param rect Region de interes corespondiente a cada blob.
 * \param pesos Imagen que contiene la distancia normalizada de cada pixel a su modelo de fondo.
 * \param mask Mascara correspondiente a cada blob.
 * \param num_frame El numero de frame para introducir su dato en la lista
 *
 * \return Los valores de la elipse para cada blob.
 */

STFly* parametrizarFly( CvRect rect,IplImage* pesos, IplImage* mask, int num_frame);

//!\brief Extrae los datos de las elipses que corresponden a cada blob.
/*!
 * \param rect Region de interes corespondiente a cada blob.
 * \param pesos Imagen que contiene la distancia normalizada de cada pixel a su modelo de fondo.
 * \param mask Mascara correspondiente a cada blob.
 * \param semiejemenor semieje menor de la elipse calculada.
 * \param semiejemayor semieje mayor de la elise calculada.
 * \param axex Longitud de los ejes de la elipse calculada.
 * \param centron Coordenadas (x,y) donde se halla el centro de la elipse.
 * \param tita orientación de la elise calculada.
 *
 * \return Los valores de la elipse para cada blob.
 */
int ellipseFit( CvRect rect,IplImage* pesos, IplImage* mask,
		float *semiejemenor,float* semiejemayor,CvPoint* centro,float* tita );

//!\brief Limpiar y borrar las imagenes, matrices y estructuras usadas durante la segmentación.

void ReleaseDataSegm( );

//!\brief Establecer las regiones de interes de cada blob.
/*!
 * \param FrameData Estructura que contiene las capas.
 * \param Brillo Imagen fuente de 8 bits de nivel de gris, contiene el frame actual.
 * \param Roi region de interes del blob.
 */

void establecerROIS( STFrame* FrameData, IplImage* Brillo,CvRect Roi );

//!\brief Eliminar las regiones de interes usadas en la segmentación.
/*!
 * \param FrameData Estructura que contiene las capas.
 * \param Brillo Imagen fuente de 8 bits de nivel de gris, contiene el frame actual.
 */
void resetearROIS( STFrame* FrameData, IplImage* Brillo );

#endif /* SEGMENTACION_H_ */
