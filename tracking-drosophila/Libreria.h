/*
 * Libreria.h
 *
 *  Created on: 09/08/2011
 *      Author: chao
 */

#include "VideoTracker.hpp"
#include "Errores.hpp"
#include "Kalman.hpp"

#ifndef LIBRERIA_H_
#define LIBRERIA_H_

void help();

///////////////////// MEDIDA DE TIEMPOS //////////////////////////////

#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#ifndef _TIEMPOS_
#define _TIEMPOS_

float obtenerTiempo( timeval ti, int unidad );
int gettimeofday( struct timeval *tv, struct timezone *tz );

#endif //_TIEMPOS_

///////////////////// TRATAMIENTO DE IMAGENES //////////////////////////////

#ifndef _IMAGEN_
#define _IMAGEN_

//!\brief getAVIFrames: Función para obtener el número de frames en algunos S.O Linux.
/*!
 * \return el número de frames que contiene el video.
 */

int getAVIFrames( char* );

int RetryCap( CvCapture* g_capture, IplImage* frame );
//! \brief ImPreProcess: Recibe una imagen RGB 8 bit y devuelve una imagen en escala de grises con un
//!   filtrado gaussiano 5x5. Si el último parámetro es verdadero se binariza, si

//!   el parámetro ImFMask no es NULL se aplica la máscara.
    /*!
      \param src Imagen fuente de 8 bit RGB.
      \param dst Imagen destino de 8 bit de niveles de gris.
      \param ImFMask Máscara.
      \param bin Si está activo se aplica un AdaptiveTreshold a la imagen antes de filtrar.

      \return Imagen preprocesada con la extracción del plato.
    */


void ImPreProcess( IplImage* src,IplImage* dst, IplImage* ImFMask,bool bin, CvRect ROI);

void verMatrizIm( IplImage* Im, CvRect roi);

void muestrearLinea( IplImage* rawImage, CvPoint pt1,CvPoint pt2, int num_frs);

void muestrearPosicion( tlcde* flies, int id );

void invertirBW( IplImage* Imagen );

#endif // _IMAGEN_


////////////////////// INTERFAZ PARA MANIPULAR UNA LCDE //////////////////////////////

#ifndef _ELEMENTO_H
#define _ELEMENTO_H

	// Tipo Elemento (un elemento de la lista) //
	typedef struct s
	{
	  void *dato;          // área de datos
	  struct s *anterior;  // puntero al elemento anterior
	  struct s *siguiente; // puntero al elemento siguiente
	} Elemento;


#endif // _ELEMENTO_H


#ifndef _INTERFAZ_LCSE_H
#define _INTERFAZ_LCSE_H

// Parámetros de la lista

typedef struct
{
  Elemento *ultimo;      // apuntará siempre al último elemento
  Elemento *actual;      // apuntará siempre al elemento accedido
  int numeroDeElementos; // número de elementos de la lista
  int posicion;          // índice del elemento apuntado por actual
} tlcde;

#endif //_INTERFAZ_LCSE_H

#ifndef _INTERFAZ_LCSE_
#define _INTERFAZ_LCSE_

//!\brief Crear un nuevo elemento de la lista.

Elemento *nuevoElemento();

//!brief  Iniciar una estructura de tipo tlcde.
/*!
 * \param lcde lista circular doblemente enlazada.
 */
void iniciarLcde(tlcde *lcde);

//!\brief Añadir un nuevo elemento a la lista a continuación
//! del elemento actual; el nuevo elemento pasa a ser el actual.
/*!
 * \param e Datos del elemento insertado.
 * \param lcde Lista circular doblemente enlazada.
 */
void insertar(void *e, tlcde *lcde);

//!\brief La función borrar devuelve los datos del elemento
//! apuntado por actual y lo elimina de la lista.
//!\n
//!\n Borra el elemento apuntado por actual.
//! Devuelve un puntero al área de datos del objeto borrado
//! o NULL si la lista está vacía.Si la posición es el último
//! y el único elemento, se inicia la lista. Si es el último
//! pero no el único, actual apuntará al nuevo último. Si no
//! es el último, acual apuntará al siguiente elemento.
/*!
 *\param lcde Lista circular doblemente enlazada.
 */

void *borrar(tlcde *lcde);



void *sustituirEl( void *e, tlcde *lcde, int i);

//!\brief Borra el elemento apuntado de la posición posición i.
//! Devuelve un puntero al área de datos del objeto borrado
//! o NULL si la lista está vacía. Si la posición es el último
//! y el único elemento, se inicia la lista. Si es el último
//! pero no el único, actual apuntará al nuevo último. Si no
//! es el último, acual apuntará al siguiente elemento.

void *borrarEl(int i,tlcde *lcde);


//!\brief Avanzar la posición actual al siguiente elemento dentro de l lista.
/*!
 *\param lcde Lista circular doblemente enlazada.
 */

void irAlSiguiente(tlcde *lcde);

//!\brief Retrasar la posición actual al elemento anterior.
/*!
 * \param lcde Lista circular doblemente enlazada.
 */

void irAlAnterior(tlcde *lcde);

//!\brief Hacer que la posición actual sea el principio de la lista.
/*!
 * \param lcde Lista circular doblemente enlazada.
 */

void irAlPrincipio(tlcde *lcde);

//!\brief El final de la lista es ahora la posición actual.
/*!
 * \param lcde Lista circular doblemente enlazada.
 */

void irAlFinal(tlcde *lcde);

//!\brief Posicionarse en el elemento i de la lista.
/*!
 * \param i posición del elemento dentro de la lista.
 */
int irAl(int i, tlcde *lcde);

//!\brief Devolver el puntero a los datos asociados
//! con el elemento actual.
/*!
 * \param lcde Lista circular doblemente enlazada.
 */

void *obtenerActual(tlcde *lcde);

//!\brief Devolver el puntero a los datos asociados
//! con el elemento de índice i.
/*!
 * \param i posición del elemento dentro de la lista.
 * \param lcde Lista circular doblemente enlazada.
 */

void *obtener(int i, tlcde *lcde);

//!\brief Establecer nuevos datos para el elemento actual.
/*!
 * \param pNuevosDatos Espacio donde se alamcenan los nuevos datos.
 * \param lcde Lista circular doblemente enlazada.
 */

void modificar(void *pNuevosDatos, tlcde *lcde);

//!\brief Posicionar un elemento al final de la lista,
//! a continuación del ultimo elemento.
/*!
 * \param e Datos del elemento insertado.
 * \param lcde Lista circular doblemente enlazada.
 */
void anyadirAlFinal(void *e, tlcde *lcde );

//tlcde* fusionarListas( tlcde* lcde1, tlcde* lcde2);

#endif // _INTERFAZ_LCSE_

/////////////////////// INTERFACE PARA GESTIONAR LISTA FLIES //////////////////////////
#ifndef _FLIES_
#define _FLIES_

#define OVER_WRITE 0	// indica que al dibujar una lista flies, se sobreescriba la imagen
#define CLEAR 1	// indica que al dibujar una lista flies, se borre primero la imagen


int dibujarFG( tlcde* flies, IplImage* dst,bool clear);

int dibujarBG( tlcde* flies, IplImage* dst,bool clear);

int dibujarBGFG( tlcde* flies, IplImage* dst,bool clear);

void dibujarBlob( STFly* blob, IplImage* dst );

//!\brief Mostrar llos elementos de la lista con sus datos.
/*!
 * \param pos posicion del elemento mostrado.
 * \param lcde Lista circular doblemente enlazada.
 */


//!\brief Mostrar los datos de los elementos del frame indicado por pos
/*!
 * \param pos posicion del elemento mostrado.
 * \param lcde Lista circular doblemente enlazada.
 */

void mostrarListaFlies(int pos,tlcde *lista);

void mostrarFliesFrame(STFrame *frameData);
//!\brief Liberar espacio de datos de la lista.
/*!
 * \param lcde Lista circular doblemente enlazada.
 */
void liberarListaFlies(tlcde *lista);

tlcde* fusionarListas(tlcde* FGFlies,tlcde* OldFGFlies );

#endif //_FLIES_

///////////////////// INTERFACE PARA GESTIONAR BUFFER //////////////////////////////

#ifndef _BUFFER_
#define _BUFFER_

//!\ brief Borra y libera el espacio del primer elemento del buffer ( el frame mas antiguo ).
/*
 * \param FramesBuf \param FramesBuf Lista circular doblemente enlazada que contiene los frames con las listas de las moscas validadas..
 */

void* liberarPrimero(tlcde *FramesBuf );

//!\brief borra todos los elementos del buffer.
/*!
 * \param FramesBuf Lista circular doblemente enlazada que contiene los frames con la listas de las moscas validadas.
 */
void liberarBuffer(tlcde *FramesBuf);

//!\brief Liberar y borrar datos de la estructura STFrame.
/*!
 * \param frameData Datos de la estructura STFrame.
 */

void liberarSTFrame( STFrame* frameData );

#endif //_BUFFER_

///////////////////////// INTERFACE PARA GESTIONAR IDENTIDADES /////////////////////

void CrearIdentidades(tlcde* Etiquetas);

static Scalar randomColor(RNG& rng);

void liberarIdentidades(tlcde* lista);

void mostrarIds( tlcde* Ids);

void visualizarId(IplImage* Imagen,CvPoint pos, int id , CvScalar color );


/////////////////////////// GESTION FICHEROS //////////////////////////////

#ifndef _FICHEROS_
#define _FIHEROS_

//!\brief verifica si existe el fichero creado que contiene lo datos de las listas almcenadas en e buffer.
/*!
 * \param nombreFichero nombre del fichero.
 *
 * \return Un 1 o True si el fichero no existe.
 */

//!\brief
int existe(char *nombreFichero);

//!\brief Crear el fichero que contiene los datos de las listas alamcenadas en el buffer.
/*!
 * \param nombreFichero nombre del fichero que contiene los datos de las moscas.
 */
void crearFichero(char *nombreFichero );

//!\brief almacena en fichero los datos de la lista Flies correspondientes al primer frame.
/*!
 * \param framesBuf \param FramesBuf Lista circular doblemente enlazada que contiene los frames con la listas de las moscas validadas.
 * \param nombreFichero nombre del fichero que contiene los datos de las moscas.
 *
 * \return Si la lista está vacía mostrará un error, si se ha guardado con éxito devuelve un uno.
 */

int GuardarPrimero( tlcde* framesBuf , char *nombreFichero);

int GuardarSTFrame( STFrame* frameData , char *nombreFichero);

void QuitarCR (char *cadena);


#endif //_FICHEROS_

#endif // LIBRERIA_H_


