/*!
 * BGModel.h
 *
 *  Created on: 19/07/2011
 *      Author: chao
 */

#ifndef BGMODEL_H_
#define BGMODEL_H_

#include "VideoTracker.hpp"
#include "Libreria.h"
#include "Visualizacion.hpp"

#define GAUSSIAN 1	//MODELO GAUSSIANO SIN ACTUALIZACIÓN SELECTIVA
#define GAUSSIAN_S_UP 2 //MODELO GAUSSIANO CON ACTUALIZACIÓN SELECTIVA
#define MEDIAN 3	//MODELO MEDIANA SIN ACTUALIZACIÓN SELECTIVA
#define MEDIAN_S_UP 4	//MODELO MEDIANA CON ACTUALIZACIÓN SELECTIVA

/// ESTRUCTURA DE DATOS DEL MODELO DE FONDO ///

typedef struct{
	//Parametros inicialización del modelo de fondo
	int Jumps; // frames de salto a distintas partes del video
	int BG_Update; // Intervalo de actualización del fondo
	int initDelay; // El aprendizaje comenzará initDelay frames despues del comienzo
	bool FLAT_DETECTION; //! Activa o desactiva la detección del plato
	int MODEL_TYPE; // tipo de modelo de fondo. Afecta a la resta y a la actualización.
					//Cuando se inicia se hace automaticamente el gaussiano
	int FLAT_FRAMES_TRAINING;//!< Nº de frames para aprendizaje del plato.
	int FRAMES_TRAINING ;//!< Nº de frames para el aprendizaje del fondo.
	float INITIAL_DESV;
	float K; 			 //!<Para la corrección de la MAD ( Median Absolute Deviation )
							//!<con el que se estima la desviación típica para el modelo gaussiano.
	double ALPHA;

	//Parametros resta de fondo y de limpieza de foreground
	int LOW_THRESHOLD ; //!< Umbral bajo para la resta de fondo.
	int HIGHT_THRESHOLD; //!< Umbral alto para la limpieza de fondo tras resta.
	bool MORFOLOGIA ; //<! si esta a 1 se aplica erosión y cierre ( no se usa ).
	int CVCLOSE_ITR ; //<! Número de iteraciones en op morfológicas.
	int MAX_CONTOUR_AREA;//!< Máxima area del blob.
	int MIN_CONTOUR_AREA;//!< Mínima area del blob.

}BGModelParams;

// PROTOTIPOS DE FUNCIONES //

//!\brief Establece los parametros por defecto para el modelado de fondo.
/*!
 * \param Parameters Estructura que contiene lo parametros para el modelado de fondo.
 *
 * \see Viodeotracker.hpp
 */

void DefaultBGMParams(BGModelParams **Parameters);

//! \brief Inicializa el modelo de fondo como una Gaussiana con una estimación
//! de la mediana y de la de desviación típica según:
//! Para el aprendizaje del fondo toma un número de frames igual a FRAMES_TRAINING
/*!
	  \param t_capture : Imagen fuente de 8 bit de niveles de gris preprocesada.
	  \param BG : Imagen fuente de 8 bit de niveles de gris. Contiene la estimación de la mediana de cada pixel
	  \param ImMask : Máscara  de 8 bit de niveles de gris para el preprocesdo (extraccion del plato).

	  \return : El modelo de fondo como una distribución Gaussiana.
	*/
StaticBGModel* initBGModel( CvCapture* t_capture,BGModelParams* Param );

void iniciarIdesv( IplImage* Idesv,float Valor, IplImage* mask, float K );


//!\brief Binariza la Imagen.
/*!
 * \param image Imagen fuente de 8 bits en escala de grises.
 *
 * \return Una imagen Binarizada.
 */
IplImage* getBinaryImage(IplImage * image);

//! \brief Recibe una imagen en escala de grises preprocesada. En la primera ejecución inicializa el modelo.
//!Estima a la mediana	en BGMod y la varianza en IvarF según:
//!\n
/*!Mediana :
	\f[
		mu(p)=\frac{median}{I_t(p)}
	\f]

      \param ImGray Imagen fuente de 8 bit niveles de gris.
      \param BGMod Imagen de fondo sobre la que se estima la mediana.
      \param Idesf Imagen sobre la que se estimada la desviación típica.
      \param DataROI Región de interes perteneciente al plato.
      \param mask Imagen sobre la que se actualizará el fondo.
      \param K Para la corrección de la MAD ( Median Absolute Deviation )
      		   con el que se estima la desviación típica para el modelo gaussiano.

      \return : La estimación de la mediana y la desviación tí pica para establecer la distribucion Gaussiana del fondo.
    */

void accumulateBackground( IplImage* ImGray, IplImage* BGMod,IplImage *Idesf,CvRect DataROI,float K, IplImage* mask  );

void updateMedian(IplImage* ImGray,IplImage* BGMod,IplImage* mask,CvRect ROI );

void updateDesv( IplImage* ImGray,IplImage* BGMod,IplImage* Idesvf,IplImage* mask, CvRect ROI, float K );

//! \brief Recibe una imagen en escala de grises preprocesada. estima a la mediana
//!	en BGMod y la varianza en IvarF según:
//!\n
/*! Mediana:
	\f[
		mu(p)=\frac{ median}{I_t(p)}
	\f]

      \param tmp_frame : Imagen fuente de 8 bit niveles de gris.
      \param BGModel : Imagen de fondo sobre la que estima la mediana.
      \param DESVI : Imagen sobre la que se estima la desviación típica.
      \param Param Puntero a estructura que almacena las distintos parametros del modelo de fondo.
      \param DataROI: Contiene los datos para establecer la ROI del plato.
      \param Mask: Nos permite actualizar el fondo de forma selectiva.
    */
void UpdateBGModel(IplImage * tmp_frame, IplImage* BGModel,IplImage* DESVI,BGModelParams* Param,  CvRect DataROI, IplImage* Mask = NULL);


//! \brief Crea una mascara binaria (0,255) donde 255 significa primer  plano
/*!
      \param ImGray : Imagen fuente de 8 bit de niveles de gris preprocesada.
      \param Cap : Imagen fuente de 8 bit de niveles de gris. Contiene la estimación de la mediana de cada pixel
      \param fg : Imagen destino ( máscara ) de 8 bit de niveles de gris.


    */

void RunningBGGModel( IplImage* Image, IplImage* median, IplImage* Idesf,double ALPHA, CvRect dataroi );

//! \brief Crea una mascara binaria (0,255) donde 255 significa primer  plano.
//! Se establece un umbral bajo ( LOW_THRESHOLD ) para los valores de la normal
//! tipificada a partir del cual el píxel es foreground ( 255 )
/*!
      \param ImGray : Imagen fuente de 8 bit de niveles de gris preprocesada.
      \param bg_model : Imagen fuente de 8 bit de niveles de gris. Contiene la estimación de la mediana de cada pixel.
      \param Idesf : Contiene la estimación de la desviación típica de cada pixel.
      \param Param : Puntero a estructura que almacena las distintos parametros del modelo de fondo.
      \param fg : Imagen destino ( máscara ) de 8 bit de niveles de gris.
      \param dataroi : Contiene los datos para establecer la ROI.

      \return : La Mascara de primer palto o foreground.
    */
void BackgroundDifference( IplImage* ImGray, IplImage* bg_model,IplImage* Idesf,IplImage* fg,BGModelParams* Param, CvRect dataroi);

//! \brief Aplica Componentes conexas para limpieza de la imagen:
//!\n - Realiza operaciones de morphologia para elimirar ruido.
//!\n - Establece un área máxima y mínimo para que el contorno no sea borrado
//!\n - Aplica un umbral alto para los valores de la normal tipificada. Si alguno
//! de los píxeles del blob clasificado como foreground con el LOW_THRESHOLD no
//! desaparece al aplicar el HIGHT_THRESHOLD, es un blob válido.
//! (- Establece el nº máximo de contornos a devolver )
//!
/*!
      \param FG : Imagen fuente de 8 bit de niveles de gris que contine la imagen a limpiar
      \param DES : Contiene la estimación de la desviación típica de cada pixel.
      \param Param : Puntero a estructura que almacena las distintos parametros del modelo de fondo.
      \param dataroi : Contiene los datos para establecer la ROI.

      \return En el foreground solo los blobs que se encuentran en movimiento.

    */
void FGCleanup( IplImage* FG, IplImage* DES, BGModelParams* Param, CvRect dataroi);


//!\brief Extraer el Plato.
/*!
 * \param * : Frame capturado.
 * \param Flat : Parametros del palto.
 * \param frTrain : Numero de frames procesado para extraer el plato.
 *
 * \return Los datos con los valores del plato,los cuales estableceran su ROI.
 */
void MascaraPlato(CvCapture*, StaticBGModel* Flat, int frTrain);

//!\brief Crea una trackbar para posicionarse en puntos concretos del video.
/*!
 * \param pos : numero de frame.
 * \param Param : Puntero a estructura que almacena las distintos parametros del modelo de fondo.
 */
void onTrackbarSlide(int pos, BGModelParams* Param);

//!\brief Crea las Imagenes y estructuras.

void AllocateBGMImages( IplImage*, StaticBGModel* bgmodel);

//!\brief destrulle las imagenes y estructuras.

void DeallocateBGM( StaticBGModel* BGModel );

//!\brief Crea las imagnes temporales.

void AllocateTempImages( IplImage *I );

//!\brief destrulle las imagenes temporales

void DeallocateTempImages();

#endif /* BGMODEL_H_ */
