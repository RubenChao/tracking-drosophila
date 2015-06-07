/*
 * validacion.hpp
 *
 *  Created on: 22/09/2011
 *      Author: chao
 */

#ifndef VALIDACION_HPP_
#define VALIDACION_HPP_

#include "VideoTracker.hpp"
#include "segmentacion.hpp"
#include "BGModel.h"


/// Parametros validación

typedef struct {
	int UmbralCirc; //!< Máxima circularidad a partir de la cual un blob se considerará no válido
	float Umbral_L;	//!< Establece el tamaño máximo del blob válido en Umbral_L desviaciones típicas de la media
	float PxiMax; 	//!< Establece la máxima probabilidad permitida para defecto. ( el área máxima )
	float Umbral_H;	//!< Establece el tamaño mínimo del blob válido en Umbral_H desviaciones típicas de la media
	float PxiMin; 	//!< // establece la mínima probabilidad permitida para exceso ( el área mínima )

	int MaxIncLTHIters; //!< Número máximo de iteraciones que se incrementará el umbral bajo para intentar dividir el blob
	int MaxDecLTHIters; //!< Número máximo de iteraciones que se incrementará el umbral bajo para aumentar P(xi) ( el área del blob )
	int MaxLowTH;		 //!< Limite superior para el umbral bajo
	int MinLowTH;		 //!< Limite inferior para el umbral bajo



}ValParams;

//!\brief Realiza la división y combinación de los componentes o blobs conectados, de la siguiente manera:
//!\n
//!\n Si la densidad o probabilidad a priori de una de elipse para un determinado blob es pequeña ( su area es grande)
//! este componente conectado en realidad pueden corresponder a varios blobs, que forma parte de una mosca,
//! o la detección de espurios.En este caso se aumenta el umbral ( por lo que aumentará la probabilidad)
//! hasta que la elipse es segementada en dos o mas componentes conectadas o hasta que la elipse alcanza una
//! probabilidad minima.
//!\n
//!\n Si si la densidad es demasiado pequeña debido a que el area de la elipse es pequeña entonces se
//! decrementa el umbral para aumentar el area hasta que la elipse se fusiona con elipses cercanas o
//! hasta que la elipse desaparezca.
/*!
 * \param Imagen Imagen fuente de 8 bits, contiene el frame actual.
 * \param FrameData Estructura que contiene las capas.
 * \param SH Estructuraque contiene los valores del modelado de la forma.
 * \param Segroi Establece la region de interes del Plat.
 * \param BGParams Contiene los parametros para el modelado de fondo.
 * \param VisParams Parametros de validación.
 * \param Mask Mascara qe contiene el Foreground.
 *
 * \return Una Lista circular doblemente enlazada, con los blobs que han sido validados correctamente.
 *
 */

tlcde* Validacion(IplImage *Imagen, STFrame* FrameData, SHModel* SH,CvRect Segroi,BGModelParams* BGParams, ValParams* VisParams,IplImage* Mask);

void Validacion2(IplImage *Imagen, STFrame* FrameData, SHModel* SH ,IplImage* Mask, ValParams* Params);

tlcde* ValidarBLOB( tlcde* Flies,int posBlob,int numFlies,STFrame* FrameData,SHModel* SH,ValParams* Params	);
//!\brief Inicializada los Parametros del modelado de fondo.
/*!
 * \param Params Parametros para el modelado de fondo.
 */

void iniciarBGModParams( BGModelParams** Params);

void setBGModParams(BGModelParams* Params);


float CalcPxMin( SHModel* SH,float Umbral_L, int areaMin );

float CalcPxMax( SHModel* SH,float Umbral_H, int areaMax );

//!\brief Inicializa los Parametros para la validación.
/*!
 * \param Params Parametros de validación.
 */


void iniciarValParams( ValParams** Parameters, SHModel* SH);

//!\brief Calcula la probabilidad total de todos los blos detectados en el frame.
/*!
 * \param Lista Lista con los dato los blobs.
 * \param SH Valores del modelado de forma.
 * \param VisParams Parametros de validación.
 * \param FlyData Estructura que contiene los datos del blob.
 *
 * \return La probabilidad total para todos los blobs ( PX).
 *
 * \note Esta probabilidad se usa como criterio de validación.
 */

double CalcProbTotal(tlcde* Lista,SHModel* SH,ValParams* VisParams,STFly* FlyData);

//!\brief Calcula la probabilidad del blob u comprueba si ésta está dentro de los límites. Almacena el valor en FlyData->Px
/*!
 * \param SH Valores del modelado de forma.
 * \param Flie Estructura que contiene los datos del blob.
 * \param ValParams Estructura que contiene los límites superior e inferior de la probabilidad.
 * \return 1 si exceso, -1 si defecto , 0 si está dentro de los limites.
 *
 * \note FlyData->Px = exp( -abs( areablob - SH->FlyAreaMedia) / SH->FlyAreaDesviacion);
 */


int CalcProbFly( SHModel* SH , STFly* FlyData, ValParams* VisParams );
//!\brief Calcula la circlaridad de la mosca.
/*!
 * \param Flie Estructura que contiene los datos del blob.
 *
 * \return el valor de la circularidad del blob.
 *
 * \note Si el blob tiene mucha circularidad este no es valido, al poder tratarse de una sombra.
 *
 */

double CalcCircul( STFly* Flie);

//!\brief Establece si un blob no cumple las condiciones para ser validado, ya sea por un exceso de area
//!o por un defecto de area. Calcula tambien los umbrales establecidos como criterios de validación.
/*!
 * \param SH Valores del modelado de forma.
 * \param VisParams VisParams Parametros de validación.
 * \param Flie Estructura que contiene los datos del blob.
 *
 * \return Si la probabilidad es pequeña debido a un area excesiva devuelve un 1,por el contrario devuelve un 0;
 *
 * Ejemplo:
 *
 * \verbatim
	if ((area_blob - SH->FlyAreaMedia)<0)  return 0;
	else return 1;
	\endverbatim
 */



int CalcProbUmbral( SHModel* SH ,ValParams* VisParams,STFly* Flie);

//!\brief Obtiene la maxima distancia de los pixeles al fonfo.
/*!
 * \param Imagen Imagen fuente de 8 bits de niveles de gris.
 * \param FrameData Estructura que contiene las capas.
 * \param Roi Región de interes perteneciente a cada blob.
 *
 * \return el valor maximo correspondiente a la maxima distancia de los pixels al fondo.
 *
 * \note Se utiliza como límite superior a la hora de aumentar el umbral.
 */
double* ObtenerMaximo(IplImage* Imagen, STFrame* FrameData,CvRect Roi );

void Restaurar(tlcde* lista, IplImage* Fg, IplImage* Origin, CvRect Roi, int elemento);

void RestaurarElMejor(tlcde* lista, IplImage* Fg, STFly* MejorFly, CvRect Roi, int elemento );

//!\brief Comprueba si todas las px de la lista estan dentro de los limites
/*!
 * \param TempSeg. Lista
 * \param ValParams Parametros de validación
 *
 *
 * \return 1 si son todas correctas y 0 si alguna no lo es.
 *
 *
 */
int comprobarPxi( tlcde* TempSeg,SHModel* SH, ValParams* Params);


#endif /* VALIDACION_HPP_ */
