/*
 * VideoTracker.hpp
 *
 *  Created on: 12/08/2011
 *      Author: chao
 */

#ifndef VIDEOTRACKER_HPP_
#define VIDEOTRACKER_HPP_

#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <time.h>
#include <sys/time.h>
#include <cvaux.h>
#include <stdio.h>
#include <cxcore.h>
#include <libconfig.h>

using namespace cv;
using namespace std;

#define CVX_RED CV_RGB(255,0,0)
#define CVX_BLUE CV_RGB(0,0,255)
#define CVX_GREEN CV_RGB(0,255,0)
#define CVX_WHITE CV_RGB(255,255,255)
#define CVX_BLACK CV_RGB(0,0,0)

// OPCIONES GENERALES DE PROGRAMA

#define DETECTAR_PLATO 1 // activa la detección del plato
//#define MEDIR_TIEMPOS 1 // activa la medida de tiempos
#define CALC_STATS_MOV 1

//OPCIONES DE VISUALIZACIÓN DE DATOS Y TIEMPOS DE PROCESOS EN CONSOLA
#define MODO_DEPURACION

#define SHOW_SHAPE_MODEL_DATA_AREAS 1//!< Switch de 0 a 1 para visualizar el valor de las areas de cada blob.
#define SHOW_SHAPE_MODEL_DATA_MEDIANA 1//!< Switch de 0 a 1 para visualizar el valor mediana para todos los blobs.

#define SHOW_PROCESS_TIMES

#define SHOW_BGMODEL_DATA 0 //!< Switch de 0 a 1 para mostrar en consola datos del modelo de fondo.
#define SHOW_BGMODEL_TIMES 0

#define SHOW_SEGMENTATION_TIMES 0 //!< Switch de 0 a 1 para visualizar los tiempos.
#define SHOW_SEGMENTATION_DATA 0 //!< Switch de 0 a 1 para visualizar los resulados de la segmentación.
#define SHOW_SEGMENTATION_MATRIX 0

#define SHOW_VALIDATION_DATA 0//!< Switch de 0 a 1 para visualizar los datos de la validación.
#define SHOW_VALIDATION_TIMES 0

#define SHOW_AI_DATA 0
#define SHOW_MT_DATA 0

#define SHOW_KALMAN_DATA 0

// VISUALIZACIÓN DE IMAGENES
#define SHOW_WINDOW 1 //!< Switch de 0 a 1 para visualizar resultado
//#define SHOW_PRESENT 0 //!< Switch de 0 a 1 para visualizar presentacion
#define CREATE_TRACKBARS 0 //!< Switch de 0 a 1 para visualizar trackbars.
#define ACTIVAR_OPCIONES_VISUALIZACION 1

// DEPURACION
#define SHOW_VISUALIZATION 0 //!< Switch de 0 a 1 GENERAL para visualizar resultados de depuración.

// Depuración preprocesado
//#define SHOW_LEARNING_FLAT 1
//#define SHOW_INIT_BACKGROUND 1
//#define SHOW_SHAPE_MODELING 1//!< Switch de 0 a 1 para visualizar los resultados del modelado de forma.

// Depuración procesado
//#define SHOW_PROCESS_IMAGES 0 //!< Switch de 0 a 1 para visualizar los resultados del procesado etapa a etapa.
//#define SHOW_BG_DIF_IMAGES 0 //!< Switch de 0 a 1 para visualizar las imagenes antes y despues de la limpieza de FG.
//#define SHOW_VALIDATION_IMAGES 0//!< Switch de 0 a 1 para visualizar las imagenes de la validación etapa a etapa.

#define SHOW_BG_REMOVAL 1 //!< Switch de 0 a 1 para visualizar el Background y Foreground.
#define SHOW_OPTICAL_FLOW 0 //!< Switch de 0 a 1 para visualizar el flujo optico.
#define SHOW_MOTION_TEMPLATE 0//!< Switch de 0 a 1 para visualizar el gradiente.
#define SHOW_KALMAN 1

#define GRABAR_VISUALIZACION 1

#ifndef _ESTRUCTURAS_
#define _ESTRUCTURAS_

#ifndef _ELEMENTO_H
#define _ELEMENTO_H

/// Tipo Elemento (un elemento de la lista)

	typedef struct s
	{
	  void *dato;          //!< área de datos
	  struct s *anterior;  //!< puntero al elemento anterior
	  struct s *siguiente; //!< puntero al elemento siguiente
	} Elemento;


#endif // _ELEMENTO_H


#ifndef _INTERFAZ_LCSE_H
#define _INTERFAZ_LCSE_H

/// Parámetros de la lista.

	typedef struct
{
  Elemento *ultimo;      //!< apuntará siempre al último elemento
  Elemento *actual;      //!< apuntará siempre al elemento accedido
  int numeroDeElementos; //!< número de elementos de la lista
  int posicion;          //!< índice del elemento apuntado por actual
} tlcde;

#endif //_INTERFAZ_LCSE_H

typedef struct {

	float CMov1SMed;  //!< Cantidad de movimiento medio mm/s.
	float CMov1MMed;  //!< Cantidad de movimiento medio.mm/min
	float CMov1HMed;  //!< Cantidad de movimiento medio.mm/h
	unsigned int EstadoTrack;  //!< Indica el estado en que se encuentra el track: KALMAN_CONTROL(0) CAMERA_CONTROL (1) o SLEEP (2).
	unsigned int EstadoBlobCount;  //!< Tiempo que el blob permanece en un estado
	unsigned int EstadoTrackCount;
}STStatFly;

	typedef struct {

		// Identificación
		int etiqueta;  //!< Identificación del blob
		CvScalar Color; //!< Color para dibujar el blob

		// Situación
		CvPoint posicion; //!< Posición del blob
		float orientacion; //!< Almacena la orientación

		// Forma
		float a,b; //!< semiejes de la elipse
		float areaElipse;
		double perimetro;
		unsigned int num_frame; //!< Almacena el numero de frame (tiempo)

		CvRect Roi; //!< region de interes para el blob

		// Validacion
		bool flag_seg; //!< Indica si el blog ha sido segmentado
		bool failSeg; //!< indica si el blob ha  desaparecido durante el analisis del defecto
		int bestTresh;//!< Mejor umbral de binarización obtenido tras validar.
		float Px; // almacena la probabilidad del blob que dependerá de su area y del modelo de forma:
				  // Px = e^(⁻|areaElipse - area_media|/desviacion )

		// Tracking. Dinámica
		float VInst; //!< Velocidad instantánea
		float dstTotal; //!< almacena la distancia total recorrida hasta el momento
		float direccion; //!<almacena la dirección del desplazamiento
		float dir_filtered; //!< Dirección del blob tras aplicar el filtro de kalman.

		unsigned int Estado;
		bool salto;	//!< Indica que la mosca ha saltado
		int Zona; //!< Si se seleccionan zonas de interes en el plato,
						///este flag indicará si el blob se encuentra en alguna de las regiones marcadas

		void* siguiente;
		STStatFly* Stats;
		tlcde* Tracks; //!< Indica los tracks que han sido asignados a este blob
	}STFly;



/// Estructura para almacenar el modelo de fondo estático.

	typedef struct {
		IplImage* Imed; //!< Imagen que contiene el modelo de fondo estático (Background Model).
		IplImage* IDesvf; //!< Imagen que contiene la desviación tipica del modelo de fondo estático.
		IplImage* ImFMask; //!<Imagen que contiene la  Mascara del plato.
		int PCentroX ;//!< Coordenada x del centro del plato.
		int PCentroY ;//!< Coordenada y del centro del plato.
		int PRadio ;//!< Radio del plato.
		CvRect DataFROI;//!< Region de interes de la imagen.
	}StaticBGModel;


	typedef struct {
		CvRect ROIS; //!< Zonas de interes
		int totalFrames; //!< Numero de Frames que posee en video
		int numFrame; //!< Numero de Frame que se está procesando.
		float fps;
		float TiempoFrame;
		float TiempoGlobal;
	}STGlobStatF;

/// Estructura que almacena cálculos estadísticos globales simples para mostrar en tiempo de ejecución.

	typedef struct {
//		CvRect ROIS; //!< Zonas de interes
//		int totalFrames; //!< Numero de Frames que posee en video
//		int numFrame; //!< Numero de Frame que se está procesando.
//		float fps;
//		float TiempoFrame;
//		float TiempoGlobal;

		float TProces;
		float TTacking;
		float staticBlobs; //!< blobs estáticos en tanto por ciento.
		float dinamicBlobs; //!< blobs en movimiento en tanto por ciento
		int TotalBlobs; //!< Número total de blobs.

		float CMov1SMed;
		float CMov1SDes;
		float CMov30SMed;
		float CMov30SDes;
		float CMov1Med;
		float CMov1Des;
		float CMov5Med;
		float CMov5Des;
		float CMov10Med;
		float CMov10Des;
		float CMov15Med;
		float CMov15Des;
		float CMov30Med;  //!< Cantidad de movimiento medio en los últimos 30 min.
		float CMov30Des;
		float CMov1HMed;  //!< Cantidad de movimiento medio en la última hora.
		float CMov1HDes;
		float CMov2HMed;	//!< Cantidad de movimiento medio en  últimas 2 horas.
		float CMov2HDes;
		float CMov4HMed;
		float CMov4HDes;
		float CMov8HMed;
		float CMov8HDes;
		float CMov16HMed;
		float CMov16HDes;
		float CMov24HMed;
		float CMov24HDes;
		float CMov48HMed;
		float CMov48HDes;
		float CMovMedio; //!< Cantidad de movimiento medio desde el comienzo.
		float CMovMedioDes;
	}STStatFrame;

/// Estructura que almacena las capas del frame, los datos para realizar calculos estadisticos simples en
/// tiempo de ejecución y la lista de los con los datos de cada blob.

	typedef struct {
		int num_frame; //!< Identificación del Frame procesado.
		IplImage* Frame;//!< Imagen fuente de 3 canales ( RAW ).
		IplImage* BGModel;//!< Imagen de 8 bits que contiene el  BackGround Model Dinámico.
		IplImage* IDesvf;//!< Imagen de 32 bits que contiene la Desviación Típica del modelo de fondo dinámico.
		IplImage* OldFG; //!< Imagen que contiene el OldForeGround ( blobs estáticos ).
		IplImage* FG;  //!< Imagen que contiene el Foreground ( blobs en movimiento ).
		IplImage* ImAdd;//!< FG + OldFG
		IplImage* ImMotion;//!< Imagen que contiene la orientación de moscas.
		IplImage* ImKalman;
		STStatFrame * Stats;
		STGlobStatF * GStats;
		tlcde* Flies; //!< Puntero a lista circular doblemente enlazada (tlcde) con los datos de cada Mosca.
		int numTracks;
		tlcde* Tracks; //!< Puntero a lista circular doblemente enlazada (tlcde) con cada Track.
	}STFrame;

/// Estructura para el modelo de forma de los blobs

	typedef struct {
	float FlyAreaMed ; //!< Mediana de las areas de los blobs que se encuentran en movimiento en cada frame.
	float FlyAreaMedia;//!< Media de las areas de los blobs que se encuentran en movimiento en cada frame.
	float FlyAreaDes ; //!< Desviación típica de las areas de los blobs que se encuentran en movimiento en cada frame.
}SHModel;


#endif /*Estructuras*/

#endif /* VIDEOTRACKER_HPP_ */
