/*!
 * Libreria.cpp
 *
 *  Created on: 12/07/2011
 *      Author: chao
 */

#include "Libreria.h"

void help(){
	printf("\n Para ejecutar el programa escriba en la consola: "
			"TrackingDrosophila [nombre_video.avi] [Nombre_Fichero.csv]\n  "
			"Donde:\n - [nombre_video.avi] es el nombre del video a analizar. Ha de "
			"estar en formato avi. Se deberán tener instalados los codecs ffmpeg.\n"
			"[Nombre_Fichero.csv] Será el nombre del fichero donde se guardarán los datos."
			"Si no se especifica se le solicitará uno al usuario. Si continúa sin especificarse"
			"se establecerá [Data_n] por defecto hasta un máximo de n = 29. ");
}
///////////////////// MEDIDA DE TIEMPOS //////////////////////////////

float obtenerTiempo( timeval ti, int unidad ){

	timeval tf;
	float tiempo;
	gettimeofday( &tf , NULL);
	switch( unidad ){
		case 0: // milisegundos
			tiempo= (tf.tv_sec - ti.tv_sec)*1000 +
															(tf.tv_usec - ti.tv_usec)/1000.0;
			break;
		case 1: //segundos
			tiempo= ( (tf.tv_sec - ti.tv_sec)*1000 +
															(tf.tv_usec - ti.tv_usec)/1000.0 )/1000;
			break;
		case 2: // minutos
			tiempo= ( (tf.tv_sec - ti.tv_sec)*1000 +
									(tf.tv_usec - ti.tv_usec)/1000.0 )/ 1000000;
			break;
		case 3: // horas
			tiempo= ( (tf.tv_sec - ti.tv_sec)*1000 +
					(tf.tv_usec - ti.tv_usec)/1000.0 )/ 1000000000;
	}
	return tiempo;
}

///////////////////// TRATAMIENTO DE IMAGENES //////////////////////////////

int RetryCap( CvCapture* g_capture, IplImage* frame ){

	frame = cvQueryFrame(g_capture); // intentar de nuevo
	int i = 1;
	if ( !frame ) { // intentar de nuevo cn los siguientes frame
		double n_frame = cvGetCaptureProperty( g_capture, 1);

		printf("\n Fallo en captura del frame %0.1f",n_frame);
		while( !frame && i<4){
			printf("\n Intentándo captura del frame %0.1f",n_frame+i);
			cvSetCaptureProperty( g_capture,
								CV_CAP_PROP_POS_AVI_RATIO,
								n_frame +i );
			frame = cvQueryFrame(g_capture);
			if ( !frame ) printf("\n Fallo en captura");
			i++;
		}
		if ( !frame ) error(2);
	}
	return i;
}

int getAVIFrames(char * fname) {
	char tempSize[4];
	// Trying to open the video file
	ifstream  videoFile( fname , ios::in | ios::binary );
	// Checking the availablity of the file
	if ( !videoFile ) {
		cout << "Couldn’t open the input file " << fname << endl;
		exit( 1 );
	}
	// get the number of frames
	videoFile.seekg( 0x30 , ios::beg );
	videoFile.read( tempSize , 4 );
	int frames = (unsigned char ) tempSize[0] + 0x100*(unsigned char ) tempSize[1] + 0x10000*(unsigned char ) tempSize[2] +    0x1000000*(unsigned char ) tempSize[3];
	videoFile.close(  );
	return frames;
}

void invertirBW( IplImage* Imagen )
{
	//añadir para invertir con  roi
	if (Imagen->roi){
		for( int y=Imagen->roi->yOffset;  y< Imagen->roi->yOffset + Imagen->roi->height ; y++){

				uchar* ptr = (uchar*) ( Imagen->imageData + y*Imagen->widthStep + 1*Imagen->roi->xOffset);

				for (int x = 0; x<Imagen->roi->width; x++) {
					if (ptr[x] == 255) ptr[x] = 0;
					else ptr[x] = 255;
				}
		}
	}
	else{
		for( int y=0;  y< Imagen->height ; y++){
				uchar* ptr = (uchar*) ( Imagen->imageData + y*Imagen->widthStep);

				for (int x = 0; x<Imagen->width; x++) {
					if (ptr[x] == 255) ptr[x] = 0;
					else ptr[x] = 255;
				}
		}
	}
}

// Recibe la imagen del video y devuelve la imagen en un canal de niveles
// de gris con el plato extraido.

void ImPreProcess( IplImage* src,IplImage* dst, IplImage* ImFMask,bool bin, CvRect ROI){

//cvNamedWindow( "Im", CV_WINDOW_AUTOSIZE);
	// Imagen a un canal de niveles de gris
	// if(( src->depth > 8)&&(src->nChannels>1))
	cvCvtColor( src, dst, CV_BGR2GRAY);
	cvSetImageROI( dst, ROI );
	if (bin == true){
		cvAdaptiveThreshold( dst, dst,
			255, CV_ADAPTIVE_THRESH_MEAN_C,CV_THRESH_BINARY,75,40);
		//bin = false;
	}
// Filtrado gaussiano 5x

	cvSmooth(dst,dst,CV_GAUSSIAN,5,5);

// Extraccion del plato
	cvResetImageROI( dst );
	cvAndS(dst, cvRealScalar( 0 ) , dst, ImFMask );
}

void verMatrizIm( IplImage* Im, CvRect roi){

	if( !Im ) return;
	if( Im->depth == IPL_DEPTH_32F) {
		int step1       = Im->widthStep/sizeof(float);
		float *ptr1   = (float *)Im->imageData;
		for (int i = roi.y; i< roi.y + roi.height; i++){
			printf(" \n\n"); // espacio entre filas
			for (int j = roi.x; j < roi.x + roi.width; j++){
				printf("%0.2f\t", ptr1[i*step1+j*Im->nChannels]); // columnas
			}
		}
//		for (int y = roi.y; y< roi.y + roi.height; y++){
//			float* ptr1 = (float*)( Im->imageData + y*Im->widthStep + 1*roi.x);
//			printf(" \n\n"); // espacio entre filas
//			for (int x = 0; x<roi.width; x++){
//				if( ( y == roi.y) && ( x == 0) ){
//					printf("\n Origen: ( %d , %d )",(x + roi.x),y);
//					printf(" Width = %d  Height = %d \n\n",roi.width,roi.height);
//				}
//
//				printf("%f\t", ptr1[x]); // columnas
//			}
//		}
	}
	else{
		int step1       = Im->widthStep/sizeof(uchar);
		uchar *ptr1   = (uchar *)Im->imageData;
		for (int i = roi.y; i< roi.y + roi.height; i++){
			printf(" \n\n"); // espacio entre filas
			for (int j = roi.x; j < roi.x + roi.width; j++){
				printf("%d\t", ptr1[i*step1+j*Im->nChannels]); // columnas
			}
		}
//		for (int y = roi.y; y< roi.y + roi.height; y++){
//			uchar* ptr1 = (uchar*) ( Im->imageData + y*Im->widthStep + 1*roi.x);
//			printf(" \n\n"); // espacio entre filas
//			for (int x = 0; x<roi.width; x++){
//				if( ( y == roi.y) && ( x == 0) ){
//					printf("\n Origen: ( %d , %d )",(x + roi.x),y);
//					printf(" Width = %d  Height = %d \n\n",roi.width,roi.height);
//				}
//				 printf("%d\t", ptr1[x]); // columnas
//			}
//		}
	}
}

void muestrearLinea( IplImage* rawImage, CvPoint pt1,CvPoint pt2, int num_frs){

	static int count = 0;
	int max_buffer;

	CvLineIterator iterator;

	if(!existe("SamplelinesPC.csv")) crearFichero("SamplelinesPC.csv");

	FILE *fptr = fopen("SLine120_220_267_5000.csv","a"); // Store the data here

	// MAIN PROCESSING LOOP:
	//
	if( count < num_frs){
		max_buffer = cvInitLineIterator(rawImage,pt1,pt2,&iterator,8,0);
		for(int j=0; j<max_buffer; j++){
			fprintf(fptr,"%d;", iterator.ptr[0]); //Write value
			CV_NEXT_LINE_POINT(iterator); //Step to the next pixel
		}
		// OUTPUT THE DATA IN ROWS:
		//
		fprintf(fptr,"\n");
		count++;
	}
	fclose(fptr);
}

void muestrearPosicion( tlcde* flies, int id ){

//	for( int i = 0; i < 1000; i++ ){
//				sprintf(nombreFichero,"SamplePosfly1.csv ",i);
//				if( !existe( nombreFichero) ) {
//					crearFichero( nombreFichero );
//					break;
//			    }
//			}
	if(!existe("SamplePosFly1.csv")) crearFichero("SamplePosFly1.csv");

	FILE *fptr = fopen("SamplePosFly1.csv","a");

	if(!flies || flies->numeroDeElementos < 1 ) return;
	// Mostrar todos los elementos de la lista
	int i = 0, tam = flies->numeroDeElementos;
	STFly* flydata = NULL;

	while( i < tam ){
			flydata = (STFly*)obtener(i, flies);
			if (flydata->etiqueta == id){
				fprintf(fptr,"%d;%d;%0.f;%0.f",flydata->posicion.x,flydata->posicion.y,flydata->direccion,flydata->dir_filtered);
				fprintf(fptr,"\n");
			}
			i++;
	}
	fclose(fptr);
}
///////////////////// INTERFAZ PARA MANIPULAR UNA LCDE //////////////////////////////

//


// Crear un nuevo elemento de la lista

Elemento *nuevoElemento()
{
  Elemento *q = (Elemento *)malloc(sizeof(Elemento));
  if (!q) error(4);
  return q;
}

void iniciarLcde(tlcde *lcde)
{
  lcde->ultimo = lcde->actual = NULL;
  lcde->numeroDeElementos = 0;
  lcde->posicion = -1;
}

void insertar(void *e, tlcde *lcde)
{
  // Obtener los parámetros de la lcde
  Elemento *ultimo = lcde->ultimo;
  Elemento *actual = lcde->actual;
  int numeroDeElementos = lcde->numeroDeElementos;
  int posicion = lcde->posicion;

  // Añadir un nuevo elemento a la lista a continuación
  // del elemento actual; el nuevo elemento pasa a ser el
  // actual
  Elemento *q = NULL;

  if (ultimo == NULL) // lista vacía
  {
    ultimo = nuevoElemento();
    // Las dos líneas siguientes inician una lista circular
    ultimo->anterior = ultimo;
    ultimo->siguiente = ultimo;
    ultimo->dato = e;      // asignar datos
    actual = ultimo;
    posicion = 0;          // ya hay un elemento en la lista
  }
  else // existe una lista
  {
    q = nuevoElemento();

    // Insertar el nuevo elemento después del actual
    actual->siguiente->anterior = q;
    q->siguiente = actual->siguiente;
    actual->siguiente = q;
    q->anterior = actual;
    q->dato = e;

    // Actualizar parámetros.
    posicion++;

    // Si el elemento actual es el último, el nuevo elemento
    // pasa a ser el actual y el último.
    if( actual == ultimo )
      ultimo = q;

    actual = q; // el nuevo elemento pasa a ser el actual
  } // fin else

  numeroDeElementos++; // incrementar el número de elementos

  // Actualizar parámetros de la lcde
  lcde->ultimo = ultimo;
  lcde->actual = actual;
  lcde->numeroDeElementos = numeroDeElementos;
  lcde->posicion = posicion;
}

void *sustituirEl( void *e, tlcde *lcde, int i){
	irAl( i, lcde);
	insertar( e, lcde );
	irAlAnterior( lcde );
	borrar(lcde);
}

void *borrarEl(int i,tlcde *lcde)
{
	int n = 0;
	// comprobar si la posicion está en los limites
	if ( i >= lcde->numeroDeElementos || i < 0 ) return NULL;
	// posicionarse en el elemento i
	irAlPrincipio( lcde );
	for( n = 0; n < i; n++ ) irAlSiguiente( lcde );
	// borrar
	  // Obtener los parámetros de la lcde
	  Elemento *ultimo = lcde->ultimo;
	  Elemento *actual = lcde->actual;
	  int numeroDeElementos = lcde->numeroDeElementos;
	  int posicion = lcde->posicion;

	  // La función borrar devuelve los datos del elemento
	  // apuntado por actual y lo elimina de la lista.
	  Elemento *q = NULL;
	  void *datos = NULL;

	  if( ultimo == NULL ) return NULL;  // lista vacía.
	  if( actual == ultimo ) // se trata del último elemento.
	  {
	    if( numeroDeElementos == 1 ) // hay un solo elemento
	    {
	      datos = ultimo->dato;
	      free(ultimo);
	      ultimo = actual = NULL;
	      numeroDeElementos = 0;
	      posicion = -1;
	    }
	    else // hay más de un elemento
	    {
	      actual = ultimo->anterior;
	      ultimo->siguiente->anterior = actual;
	      actual->siguiente = ultimo->siguiente;
	      datos = ultimo->dato;
	      free(ultimo);
	      ultimo = actual;
	      posicion--;
	      numeroDeElementos--;
	    }  // fin del bloque else
	  }    // fin del bloque if( actual == ultimo )
	  else // el elemento a borrar no es el último
	  {
	    q = actual->siguiente;
	    actual->anterior->siguiente = q;
	    q->anterior = actual->anterior;
	    datos = actual->dato;
	    free(actual);
	    actual = q;
	    numeroDeElementos--;
	  }

	  // Actualizar parámetros de la lcde
	  lcde->ultimo = ultimo;
	  lcde->actual = actual;
	  lcde->numeroDeElementos = numeroDeElementos;
	  lcde->posicion = posicion;

	  return datos;

}

void *borrar(tlcde *lcde)
{
  // Obtener los parámetros de la lcde
  Elemento *ultimo = lcde->ultimo;
  Elemento *actual = lcde->actual;
  int numeroDeElementos = lcde->numeroDeElementos;
  int posicion = lcde->posicion;

  // La función borrar devuelve los datos del elemento
  // apuntado por actual y lo elimina de la lista.
  Elemento *q = NULL;
  void *datos = NULL;

  if( ultimo == NULL ) return NULL;  // lista vacía.
  if( actual == ultimo ) // se trata del último elemento.
  {
    if( numeroDeElementos == 1 ) // hay un solo elemento
    {
      datos = ultimo->dato;
      free(ultimo);
      datos = NULL;
      ultimo = actual = NULL;
      numeroDeElementos = 0;
      posicion = -1;
    }
    else // hay más de un elemento
    {
      actual = ultimo->anterior;
      ultimo->siguiente->anterior = actual;
      actual->siguiente = ultimo->siguiente;
      datos = ultimo->dato;
      free(ultimo);
      ultimo = actual;
      posicion--;
      numeroDeElementos--;
    }  // fin del bloque else
  }    // fin del bloque if( actual == ultimo )
  else // el elemento a borrar no es el último
  {
    q = actual->siguiente;
    actual->anterior->siguiente = q;
    q->anterior = actual->anterior;
    datos = actual->dato;
    free(actual);
    actual = q;
    numeroDeElementos--;
  }

  // Actualizar parámetros de la lcde
  lcde->ultimo = ultimo;
  lcde->actual = actual;
  lcde->numeroDeElementos = numeroDeElementos;
  lcde->posicion = posicion;

  return datos;
}



void irAlSiguiente(tlcde *lcde)
{
  // Avanza la posición actual al siguiente elemento.
  if (lcde->posicion < lcde->numeroDeElementos - 1)
  {
    lcde->actual = lcde->actual->siguiente;
    lcde->posicion++;
  }
}

void irAlAnterior(tlcde *lcde)
{
  // Retrasa la posición actual al elemento anterior.
  if ( lcde->posicion > 0 )
  {
    lcde->actual = lcde->actual->anterior;
    lcde->posicion--;
  }
}

void irAlPrincipio(tlcde *lcde)
{
  // Hace que la posición actual sea el principio de la lista.
  lcde->actual = lcde->ultimo->siguiente;
  lcde->posicion = 0;
}

void irAlFinal(tlcde *lcde)
{
  // El final de la lista es ahora la posición actual.
  lcde->actual = lcde->ultimo;
  lcde->posicion = lcde->numeroDeElementos - 1;
}

int irAl(int i, tlcde *lcde)
{
  int n = 0;
  if (i >= lcde->numeroDeElementos || i < 0) return 0;

  irAlPrincipio(lcde);
  // Posicionarse en el elemento i
  for (n = 0; n < i; n++)
    irAlSiguiente(lcde);
  return 1;
}

void *obtenerActual(tlcde *lcde)
{
  // La función obtener devuelve el puntero a los datos
  // asociados con el elemento actual.
  if ( lcde->ultimo == NULL ) return NULL; // lista vacía

  return lcde->actual->dato;
}

void *obtener(int i, tlcde *lcde)
{
  // La función obtener devuelve el puntero a los datos
  // asociados con el elemento de índice i.
  if (!irAl(i, lcde)) return NULL;
  return obtenerActual(lcde);
}

void modificar(void *pNuevosDatos, tlcde *lcde)
{
  // La función modificar establece nuevos datos para el
  // elemento actual.
  if(lcde->ultimo == NULL) return; // lista vacía

  lcde->actual->dato = pNuevosDatos;
}

void anyadirAlFinal(void *e, tlcde *lcde ){
	irAlFinal( lcde );
	insertar( e, lcde);
}

void anyadirAlPrincipio(void *e, tlcde *lcde ){
	irAlPrincipio( lcde );
	insertar( e, lcde);
}


/////////////////////// INTERFACE PARA GESTIONAR LISTA FLIES //////////////////////////

int dibujarFG( tlcde* flies, IplImage* dst,bool clear){
	STFly* fly;
	if( flies->numeroDeElementos < 1 ) return 0;
	if( dst == NULL ) return 0;
	if ( clear ) cvZero( dst );
	irAlPrincipio( flies);
	for( int j = 0; j < flies->numeroDeElementos; j++){
		fly = (STFly*)obtener( j, flies);
		float angle;
		if ( fly->orientacion >=0 && fly->orientacion < 180) angle = 180 - fly->orientacion;
		else angle = (360-fly->orientacion)+180;
		if( fly->Estado ){
			CvSize axes = cvSize( cvRound(fly->a) , cvRound(fly->b) );
			cvEllipse( dst, fly->posicion, axes, angle, 0, 360, cvScalar( 255,0,0,0), -1, 8);
		}
	}
	return 1;
}

int dibujarBG( tlcde* flies, IplImage* dst, bool clear){
	STFly* fly;
	if( flies->numeroDeElementos < 1 ) return 0;
	if( dst == NULL ) return 0;
	if ( clear ) cvZero( dst );
	irAlPrincipio( flies);
	for( int j = 0; j < flies->numeroDeElementos; j++){
		fly = (STFly*)obtener( j, flies);
		float angle;
		if ( fly->orientacion >=0 && fly->orientacion < 180) angle = 180 - fly->orientacion;
		else angle = (360-fly->orientacion)+180;
		if( !fly->Estado ){
			CvSize axes = cvSize( cvRound(fly->a) , cvRound(fly->b) );
			cvEllipse( dst, fly->posicion, axes, angle, 0, 360, cvScalar( 255,0,0,0), -1, 8);
		}
	}
	return 1;
}

int dibujarBGFG( tlcde* flies, IplImage* dst,bool clear){
	STFly* fly;
	if( !flies || flies->numeroDeElementos < 1 ) return 0;
	if( dst == NULL ) return 0;
	if ( clear ) cvZero( dst );
	irAlPrincipio( flies);
	for( int j = 0; j < flies->numeroDeElementos; j++){
		fly = (STFly*)obtener( j, flies);
		float angle;
		if ( fly->orientacion >=0 && fly->orientacion < 180) angle = 180 - fly->orientacion;
		else angle = (360-fly->orientacion)+180;
		CvSize axes = cvSize( cvRound(fly->a) , cvRound(fly->b) );
		cvEllipse( dst, fly->posicion, axes, angle, 0, 360, cvScalar( 255,0,0,0), -1, 8);
	}
	return 1;
}

void dibujarBlob( STFly* blob, IplImage* dst ){
	float angle;
	if ( blob->orientacion >=0 && blob->orientacion < 180) angle = 180 - blob->orientacion;
	else angle = (360-blob->orientacion)+180;
	CvSize axes = cvSize( cvRound(blob->a) , cvRound(blob->b) );
	cvEllipse( dst, blob->posicion, axes, angle, 0, 360, cvScalar( 255,0,0,0), -1, 8);
}


void mostrarListaFlies(int pos,tlcde *lista)
{
	int n;
	tlcde* flies;
	STFrame* frameData;
	int pos_act;
	pos_act = lista->posicion;

	if (pos >= lista->numeroDeElementos || pos < 0) return;
	  irAlPrincipio(lista);
	  // Posicionarse en el frame pos
	for (n = 0; n < pos; n++) irAlSiguiente(lista);
	// obtenemos la lista de flies
	frameData = (STFrame*)obtenerActual( lista );
	// Mostrar todos los elementos de la lista
	mostrarFliesFrame(frameData);
	// regresamos lista al punto en que estaba antes de llamar a la funcion de mostrar lista.
	irAlPrincipio(lista);
	for (n = 0; n < pos_act; n++) irAlSiguiente(lista);
}

void mostrarFliesFrame(STFrame *frameData)
{
	int n;
	tlcde* flies;
	flies = frameData->Flies;
	if(!flies || flies->numeroDeElementos < 1 ) return;
	// Mostrar todos los elementos de la lista
	int i = 0, tam = flies->numeroDeElementos;
	STFly* flydata = NULL;
	for(int j = 0; j < 6; j++){
		while( i < tam ){
			flydata = (STFly*)obtener(i, flies);
			if (j == 0){
				if (i == 0) printf( "\n\netiquetas");
				printf( "\t%d",flydata->etiqueta);
			}
			if( j == 1 ){
				if (i == 0) printf( "\nPosiciones");
				int x,y;
				x = flydata->posicion.x;
				y = flydata->posicion.y;
				printf( "\t%d %d",x,y);
			}

			if( j == 2 ){
				if (i == 0) printf( "\nOrientacion");
				printf( "\t%0.1f",flydata->orientacion);
			}
			if( j == 3 ){
				if (i == 0) printf( "\nDirección");
				printf( "\t%0.1f",flydata->direccion);
			}
			if( j == 4 ){
				if (i == 0) printf( "\nDir_filtered");
				printf( "\t%0.1f",flydata->dir_filtered);
			}
//			if( j == 5 ){
//				if (i == 0) printf( "\nVx\t");
//				printf( "\t%0.1f",flydata->Vx);
//			}
//			if( j == 6 ){
//				if (i == 0) printf( "\nVy\t");
//				printf( "\t%0.1f",flydata->Vy);
//			}
//			if( j == 7 ){
//				if (i == 0) printf( "\nArea\t");
//				printf( "\t%0.1f",flydata->areaElipse);
//			}
//			if( j == 8 ){
//				if (i == 0) printf( "\nEstado\t");
//				printf( "\t%d",flydata->Estado);
//			}
//			if( j == 9 ){
//				if (i == 0) printf( "\nFrameCount");
//				printf( "\t%d",flydata->FrameCount);
//			}
//
//			if( j == 10 ){
//				if (i == 0) printf( "\nStaticFrames");
//				printf( "\t%d",flydata->StaticFrames);
//			}
			if( j == 5 ){
				if (i == 0) printf( "\nNumFrame");
				printf( "\t%d",flydata->num_frame);
			}
			i++;
		}
		i=0;
	}
	if (tam == 0 ) printf(" Lista vacía\n");

}

void liberarListaFlies(tlcde *lista)
{
  // Borrar todos los elementos de la lista
  STFly *flydata = NULL;
  // Comprobar si hay elementos
  if(lista == NULL || lista->numeroDeElementos<1)  return;
  // borrar: borra siempre el elemento actual

  irAlPrincipio(lista);
  flydata = (STFly *)borrar(lista);
  while (flydata)
  {
	if (flydata->Tracks ) free(flydata->Tracks);
	if (flydata->Stats) free(flydata->Stats);
    free(flydata); // borrar el área de datos del elemento eliminado

    flydata = NULL;
    flydata = (STFly *)borrar(lista);
  }
}



tlcde* fusionarListas(tlcde* FGFlies,tlcde* OldFGFlies ){
	tlcde* flies = NULL;
	flies = ( tlcde * )malloc( sizeof(tlcde ));
	iniciarLcde( flies );
	//Inicializar estructura para almacenar los datos cada mosca
	STFly *fly = NULL;

	if( !OldFGFlies && !FGFlies ) return flies;
	if( (FGFlies) && (FGFlies->numeroDeElementos > 0 ) ){
		irAlPrincipio( FGFlies);
		for( int j = 0; j < FGFlies->numeroDeElementos; j++){
			fly = (STFly*)obtener( j, FGFlies);
			anyadirAlFinal( fly, flies);
		}
	}
	if(OldFGFlies!=NULL && OldFGFlies->numeroDeElementos > 0 ){
			irAlPrincipio( OldFGFlies);
			for( int j = 0; j < OldFGFlies->numeroDeElementos; j++){
				fly = (STFly*)obtener( j, OldFGFlies);
				anyadirAlFinal( fly, flies);
			}
	}
	return flies;

}

///////////////////// INTERFACE PARA GESTIONAR BUFFER //////////////////////////////


///! libera el primer elemento del buffer  ( el frame mas antiguo )
///! El puntero actual seguirá apuntando al mismo elemento que apuntaba antes de llamar
///! a la función.

void* liberarPrimero(tlcde *FramesBuf ){

	STFrame *frameData = NULL;

//Guardamos la posición actual.
	int i = FramesBuf->posicion;
	if( FramesBuf->numeroDeElementos == 0 ) {printf("\nBuffer vacio");return 0;}
	irAl(0, FramesBuf);
	frameData = (STFrame*)obtenerActual( FramesBuf );
	 // por cada nuevo frame se libera el espacio del primer frame

	frameData = (STFrame *)borrar( FramesBuf );
	if( !frameData ) {
		printf( "Se ha borrado el último elemento.Buffer vacio" );
		frameData = NULL;
		return 0;
	}
	else{
		irAl(i - 1, FramesBuf);
		return frameData;
	}
}

void liberarSTFrame( STFrame* frameData ){

	if( !frameData) return;

	cvReleaseImage(&frameData->Frame);
	cvReleaseImage(&frameData->BGModel);
	cvReleaseImage(&frameData->FG);
	cvReleaseImage(&frameData->IDesvf);
	cvReleaseImage(&frameData->OldFG);
	cvReleaseImage(&frameData->ImAdd);
	cvReleaseImage(&frameData->ImMotion);
	cvReleaseImage(&frameData->ImKalman);

	liberarListaFlies( frameData->Flies);
	if (frameData->Flies ) free( frameData->Flies);
	if( frameData->GStats) free(frameData->GStats);

	frameData->Flies = NULL;
	frameData->GStats = NULL;
    free(frameData); // borrar el área de datos del elemento eliminado
    frameData = NULL;
}

void liberarBuffer(tlcde *FramesBuf)
{
  // Borrar todos los elementos del buffer
  STFrame *frameData = NULL;
  if (FramesBuf->numeroDeElementos == 0 ) return;
	irAlPrincipio( FramesBuf);
	frameData = (STFrame*)borrar( FramesBuf);
	while(frameData){
		liberarSTFrame( frameData );
		frameData = NULL;
		frameData = (STFrame*)borrar( FramesBuf);
	}
}
void CrearIdentidades(tlcde* Identities){

 	Identity* Id;
 	RNG rng(0xFFFFFFFF); // para generar un color aleatorio
 //	char nombre[10];
 //	nombre = "Jonh";
 //	nombre = "Pepe";
 //	nombre = "Tomas";
 //	nombre = "Manuel";
 	int i = NUMBER_OF_IDENTITIES-1;
 	for(i=NUMBER_OF_IDENTITIES-1; i >= 0 ; i--){
 		Id = ( Identity* )malloc( sizeof(Identity ));
 		Id->etiqueta = i + 1;
 		Id->color = randomColor(rng);
 		anyadirAlFinal( Id, Identities);
 	}
 }

 void liberarIdentidades(tlcde* lista){
 	  // Borrar todos los elementos de la lista
 	Identity* id;
 	  // Comprobar si hay elementos
 	  if (lista->numeroDeElementos == 0 ) return;
 	  // borrar: borra siempre el elemento actual
 	  irAlPrincipio( lista );
 	  id = (Identity *)borrar(lista);
 	  while( id ){
 		  free (id);
 		  id = NULL;
 		  id = (Identity *)borrar(lista);
 	  }
 }

 void mostrarIds( tlcde* Ids){

 	Identity* id;
 	irAlFinal(Ids);

 	for(int i = 0; i <Ids->numeroDeElementos ; i++ ){
 		id = (Identity*)obtener(i, Ids);
 		printf("Id = %d\n", id->etiqueta);
 	}
 }



 void visualizarId(IplImage* Imagen,CvPoint pos, int id , CvScalar color ){

 	char etiqueta[10];
 	CvFont fuente1;
 	CvPoint origen;

 	origen = cvPoint( pos.x-5 , pos.y - 15);

 	sprintf(etiqueta,"%d",id );
 	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
 	cvPutText( Imagen, etiqueta,  origen, &fuente1, color );
 }

 void reasignarIds(){

 }

 static Scalar randomColor(RNG& rng)
 {
     int icolor = (unsigned)rng;
     return Scalar(icolor&255, (icolor>>8)&255, (icolor>>16)&255);
 }

/////////////////////////// GESTION FICHEROS //////////////////////////////

int existe(char *nombreFichero)
{
  FILE *pf = NULL;
  // Verificar si el fichero existe
  int exis = 0; // no existe
  if ((pf = fopen(nombreFichero, "r")) != NULL)
  {
    exis = 1;   // existe
    fclose(pf);
  }
  return exis;
}

void crearFichero(char *nombreFichero )
{
  FILE *pf = NULL;
  // Abrir el fichero nombreFichero para escribir "w"
  if ((pf = fopen(nombreFichero, "wb")) == NULL)
  {
    printf("El fichero no puede crearse.");
    exit(1);
  }
 fclose(pf);
}

//!< El algoritmo mantiene un buffer de 50 frames ( 2 seg aprox ) para los datos y para las imagenes.
//!< Una vez que se han llenado los buffer, se almacena en fichero los datos de la lista
//!< Flies correspondientes al primer frame. La posición actual no se modifica.
//!< Si la lista está vacía mostrará un error. Si se ha guardado con éxito devuelve un uno.

int GuardarPrimero( tlcde* framesBuf , char *nombreFichero){

	tlcde* Flies = NULL;
	FILE * pf;
	STFly* fly = NULL;
	int posicion = framesBuf->posicion;

	if (framesBuf->numeroDeElementos == 0) {printf("\nBuffer vacío\n");return 0;}

	// obtenemos la direccion del primer elemento
	irAlPrincipio( framesBuf );
	STFrame* frameData = (STFrame*)obtenerActual( framesBuf );
	//obtenemos la lista

	Flies = frameData->Flies;
	if (Flies->numeroDeElementos == 0||!Flies) {printf("\nElemento no guardado.Lista vacía\n");return 1;}
	int i = 0, tam = Flies->numeroDeElementos;
	// Abrir el fichero nombreFichero para añadir "a".
	if ((pf = fopen(nombreFichero, "a")) == NULL)
	{
	printf("El fichero no puede abrirse.");
	exit(1);
	}
	while( i < tam ){
		fly = (STFly*)obtener(i, Flies);
		fwrite(&fly, sizeof(STFly), 1, pf);
		if (ferror(pf))
		{
		  perror("Error durante la escritura");
		  exit(2);
		}
		i++;
	}
	fclose(pf);
	irAl( posicion, framesBuf);
	return 1;
}

int GuardarSTFrame( STFrame* frameData , char *nombreFichero){

	tlcde* Flies = NULL;
	FILE * pf;
	STFly* fly = NULL;

	//obtenemos la lista
	if(!frameData) return 1;
	// Abrir el fichero nombreFichero para añadir "a".
		if ((pf = fopen(nombreFichero, "a")) == NULL)
		{
			printf("El fichero no puede abrirse.");

			return 0;
		}
	Flies = frameData->Flies;
	if (Flies->numeroDeElementos == 0||!Flies) {
		printf("\nElemento no guardado.Lista vacía\n");
		fprintf(pf,"\n"); // linea en blanco
		fclose(pf);
		return 1;
	}
	int i = 0, tam = Flies->numeroDeElementos;

	// cuando haya una con etiqueta 0 no se computarán, se almacenará la posición de la fly con la id del track
	for( int i= 1; i <= Flies->numeroDeElementos; i++){ //etiquetas
		for(int j = 0; j < Flies->numeroDeElementos; j++){
			fly =  (STFly*)obtener(j, Flies);
			if( fly->etiqueta == i ) break;
			else fly = NULL;
		}
		if(fly)	fprintf(pf,"%d;%d;%0.1f;",fly->posicion.x,fly->posicion.y,fly->dir_filtered); // si se encuentra
		else {

			fprintf(pf," ; ; ;"); // campos en blanco
		}
	}
	fprintf(pf,"\n"); // siguiente línea
	fclose(pf);
	return 1;

}

void QuitarCR (char *cadena)
{

	int i;
	for(i = strlen(cadena)-1; i && cadena[i] < ' '; i--)
	cadena [i] = 0;
}
///!brief inicia la estructura para grabar un video.


