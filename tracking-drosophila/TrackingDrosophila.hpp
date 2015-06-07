/*!
 * TrackingDrosophila.h
 *
 *  Created on: 19/07/2011
 *      Author: chao
 */

#ifndef TRACKING_H_
#define TRACKING_H_

#include "VideoTracker.hpp"
#include "Libreria.h"

#include "Inicializacion.hpp"
#include "Preprocesado.hpp"
#include "Procesado.hpp"
#include "Tracking.hpp"
#include "Stats.hpp"
#include "Visualizacion.hpp"

#define MAX_BUFFER IMAGE_BUFFER_LENGTH //!< máximo número de frames que almacenará la estructura.

//#define INTERVAL_BACKGROUND_UPDATE 10000


//PROTOTIPOS DE FUNCIONES
//#ifndef PROTOTIPOS_DE_FUNCIONES
//#define PROTOTIPOS_DE_FUNCIONES

typedef struct{

	double MaxFlat;	// - Este parámetro se usa para calibrar la cámara
								// En caso de no introducir valor las medidas de las
								// distacias serán en pixels. Una vez encontrado el
								// plato, se usará su ancho en píxels y el valor aquí
								// introducido para establecer los pixels por mm.

	int FPS;		// tasa de frames por segundo del vídeo. Se usa para
								// para establecer la unidad de tiempo.
								// NOTA:  En algunos videos la obtención de la tasa
								// de frames es errónea, con el consiguiente error
								// en las medidas que intervengan tiempos. Si se observa
								// que la tasa de frames indicada en la pantalla de
								// visualización difiere de la real del video, se ha de
								// modificar este parámetro y establezcer la real. Se ha
								// observado dicho fallo con archivos *.wmv. Se recomienda
								// codificar los vídeos con MPEG en cualquiera de sus formatos.
								// Consultar documentación para mayor detalle.
	double TotalFrames ;			// Número de frames que contiene el vídeo. Se usa para
								// establecer el progreso y realizar los saltos en
								// el modelo de fondo.

								// NOTA: Como en el caso de FPS, en algunos formatos de vídeo
								// puede resultar errónea. En dicho caso, introducir aquí el
								// valor correcto.

	int InitDelay	;  			// - Los primeros x frames serán ignorados.
								// incrementar o decrementar en función de la estabilidad
								// de la imagen en los primeros frames.

	bool MedirTiempos ;			// - Activa la medición de tiempos de los distintos
								// módulos ( pre_procesado, procesado, tracking,
								// estadísticas y visualización)

								// NOTA. Afecta al rendimiento.

}GlobalConf;

//!\brief Inicialización: Crea las ventanas, inicializa las estructuras (excepto STFlies y STFrame que se inicia en procesado ),
//!asigna espacio a las imagenes, establece los parámetros del modelo de fondo y crea el fichero de datos.
/*!
 * \param frame Imagen fuente 8 bits en escala de grises.
 * \param Shape Estructura con los datos del modelo de forma.
 * \param BGParams Estructura que contiene los parámetros del modelado de fondo.
 * \param argc Argumento que contiene el video a cargar.
 * \param argv Argumento que contiene el video a cargar.
 *
 * \return Un 1 si todas las vetanas y estructuras han sido creadas correctamente.
 *
 * \see VideoTracker.hpp
 */

int Inicializacion( IplImage* frame,
			int argc,
			char* argv[],
			tlcde** FramesBuf,
			STStatFrame** Stats);

void onMouse( int event, int x, int y, int, void* );

void SetGlobalConf(  CvCapture* Cap );

int SetDefaultGlobalParams(  CvCapture* Cap );

void ShowParams( char* Campo );

//!\brief FinalizarTraking: Destruir todas las ventanas,imagenes,estructuras etc.

	void Finalizar(CvCapture **g_capture);


//!\brief onTrackbarSlider: Crea una trackbar para posicionarse en puntos concretos del video.
/*!
 * \return El frame correspondiente a la posición de la trackbar.
 */

	void onTrackbarSlider(  int  );


//#endif


#endif /* TRACKING_H_ */
