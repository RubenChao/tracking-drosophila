/*
 * Procesado.cpp
 *
 *  Created on: 20/10/2011
 *      Author: chao
 *
 *   Esta función realiza las siguientes acciones:
 *
 * - Limpieza del foreground en tres etapas :
 *   1_Actualización de fondo y resta de fondo obteniendo el foreground
 *   2_Nueva actualización de fondo usando la máscacara de foreground obtenida.
 *   Resta de fondo
 *   Redefinición de la máscara de foreground mediante ajuste por elipses y
 *   obtención de los parámetros de los blobs en segmentación.
 *   3_Repetir de nuevo el ciclo de actualizacion-resta-segmentación con la nueva máscara
 *   obteniendo así el foreground y el background definitivo
 *
 * - Rellena la lista lineal doblemente enlazada ( Flies )con los datos de cada uno de los blobs
 * - Rellena la estructura FrameData con las nuevas imagenes y la lista Flies.
 * - Finalmente añade la lista Flies a la estructura FrameData
 *   y la estructura FrameData al buffer FramesBuf   */

/********************************************************************
 *			 			MODELO MEDIANA      						*
 * 				 SIN ACTUALIZACIÓN SELECTIVA						*

 *			 			MODELO GAUSSIANO      						*
 * 				  CON ACTUALIZACIÓN SELECTIVA						*

 *			 			MODELO GAUSSIANO      						*
 * 				   SIN ACTUALIZACIÓN SELECTIVA						*
 ********************************************************************/


#include "Procesado.hpp"




IplImage *Imagen = NULL; // imagen preprocesada
IplImage *FGMask = NULL; // mascara del foreground
IplImage *lastBG = NULL;
IplImage *lastIdes = NULL;

/********************************************************************
 *			 			MODELO MEDIANA      						*
 * 				 CON ACTUALIZACIÓN SELECTIVA
 * 			OBTENCION DE MODELO DINAMICO Y ESTÁTICO	DEL FONDO		*
 ********************************************************************/


STFrame* Procesado( IplImage* frame, StaticBGModel* BGModel,SHModel* Shape, ValParams* valParams, BGModelParams *BGPrParams ){

	extern double NumFrame;
	struct timeval ti, tif; // iniciamos la estructura
	float tiempoParcial;
	static int update_count = 0;
	// otros parámetros
	int fr = 0;


	STFrame* frameData = NULL;
	tlcde* OldFGFlies = NULL;
	tlcde* FGFlies = NULL;



#ifdef	MEDIR_TIEMPOS
	gettimeofday(&tif, NULL);
	printf("\n1)Procesado:\n");
	gettimeofday(&ti, NULL);
	printf("\t0)Preprocesado \n");
#endif
	AllocateDataProcess( frame );
	ImPreProcess( frame, Imagen, BGModel->ImFMask, 0, BGModel->DataFROI);
#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t\t-Tiempo: %5.4g ms\n", tiempoParcial);
#endif
	// Cargar datos iniciales y establecer parametros
	// Cargamos datos
	if( !frameData ) { //en la primera iteración iniciamos el modelo dinámico al estático
		cvCopy( BGModel->Imed,lastBG);
		cvCopy( BGModel->IDesvf,lastIdes);
	}
	// inicializar parámetros de modelo de fondo y de validación
	if( !BGPrParams) {
		DefaultBGMParams( &BGPrParams );
		putBGModelParams( BGPrParams );
	}
	if(!valParams) iniciarValParams( &valParams, Shape);

	// Iniciar estructura para datos del nuevo frame
	frameData = InitNewFrameData( frame );
	// cargamos las últimas imágenes del frame y del fondo.
	cvCopy( lastBG, frameData->BGModel);
	cvCopy( lastIdes, frameData->IDesvf);
	cvCopy( frame,frameData->Frame);

	//// BACKGROUND UPDATE
	//	obtener la mascara del FG y la lista con los datos de sus blobs.
	// Actualización de fondo modelo dinámico
#ifdef	MEDIR_TIEMPOS
	gettimeofday(&ti, NULL);
	printf("\t1)Actualización del modelo de fondo:\n");
#endif
	if( update_count == BGPrParams->BG_Update  )
		UpdateBGModel( Imagen,frameData->BGModel,frameData->IDesvf, BGPrParams, BGModel->DataFROI, BGModel->ImFMask );

#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t\t-Tiempo: %5.4g ms\n", tiempoParcial);
	gettimeofday(&ti, NULL);
	printf("\t2)Resta de Fondo \n");
#endif
	/////// BACKGROUND DIFERENCE. Obtención de la máscara del foreground
	BackgroundDifference( Imagen, frameData->BGModel,frameData->IDesvf, frameData->FG ,BGPrParams, BGModel->DataFROI);
	DraWWindow( NULL,NULL, NULL, SHOW_PROCESS_IMAGES, COMPLETO  );
	if( SHOW_PROCESS_IMAGES ){

	}
#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t\t-Tiempo: %5.4g ms\n", tiempoParcial);
	gettimeofday(&ti, NULL);
	printf("\t3)Segmentación:\n");
#endif
	//// SEGMENTACIÓN

	frameData->Flies = segmentacion2(Imagen, frameData->BGModel,frameData->FG, BGModel->DataFROI, NULL);
	dibujarBGFG( frameData->Flies,frameData->FG,1);
	DraWWindow( NULL,NULL, NULL, SHOW_PROCESS_IMAGES, COMPLETO  );
#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t\t-Tiempo: %5.4g ms\n", tiempoParcial);
	gettimeofday(&ti, NULL);
	printf("\t4)Validación de blobs\n", tiempoParcial);

#endif

	/////// VALIDACIÓN

	Validacion2(Imagen, frameData , Shape, FGMask, valParams);
	dibujarBGFG( frameData->Flies,frameData->FG,1);

	DraWWindow( NULL,NULL, NULL, SHOW_PROCESS_IMAGES, COMPLETO  );

#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t\t-Tiempo %5.4g ms\n", tiempoParcial);
	gettimeofday(&ti, NULL);
#endif
	/////// ACTUALIZACIÓN SELECTIVA. Modelo de fondo estático.
	//	Obtención de máscara de foreground
	if( update_count == BGPrParams->BG_Update  ){
		cvZero( FGMask);
		cvDilate( frameData->FG, FGMask, 0, 2 );
		cvAdd( BGModel->ImFMask, FGMask, FGMask);
		UpdateBGModel( Imagen, lastBG, lastIdes, BGPrParams, BGModel->DataFROI, FGMask );
		cvCopy( lastBG, frameData->BGModel);
		cvCopy(  lastIdes, frameData->IDesvf);
		update_count = 0;
	}
	else{
		cvCopy( frameData->BGModel, lastBG );
		cvCopy( frameData->IDesvf, lastIdes);
	}
	/////// GUARDAMOS las imagenes definitivas para la siguiente iteración


#ifdef MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti, 0);
	printf("\t5)Actualización selectiva del Modelo de fondo: %5.4g ms\n", tiempoParcial);
	tiempoParcial = obtenerTiempo( tif , NULL);
	printf("Procesado correcto.Tiempo total %5.4g ms\n", tiempoParcial);
#endif
	update_count++;

	return frameData;
}

STFrame* InitNewFrameData(IplImage* I ){

	STFrame* FrameData;
	FrameData = ( STFrame *) malloc( sizeof(STFrame));

	extern double NumFrame;
	CvSize size = cvGetSize( I );

	FrameData->Frame = cvCreateImage(size,8,3);
	FrameData->BGModel = cvCreateImage(size,8,1);
	FrameData->FG = cvCreateImage(size,8,1);
	FrameData->IDesvf = cvCreateImage(size,IPL_DEPTH_32F,1);
	FrameData->OldFG = cvCreateImage(size,8,1);
	FrameData->ImAdd = cvCreateImage(size,8,1);
	FrameData->ImMotion = cvCreateImage( size, 8, 3 );
	FrameData->ImMotion->origin = I->origin;
	FrameData->ImKalman = cvCreateImage( size, 8, 3 );
	cvZero( FrameData->Frame );
	cvZero( FrameData->BGModel );
	cvZero( FrameData->FG );
	cvZero( FrameData->IDesvf );
	cvZero( FrameData->ImMotion);
	cvZero( FrameData->ImKalman);
	cvZero( FrameData->ImAdd);
	FrameData->Flies = NULL;
	FrameData->Stats = NULL;
	FrameData->Tracks = NULL;
	FrameData->GStats = NULL;

	FrameData->num_frame = (int)NumFrame;

	return FrameData;
}

void putBGModelParams( BGModelParams* Params){
	 static int first = 1;
	 Params->BG_Update = 1;
	 Params->initDelay = 0;
	 Params->MODEL_TYPE = MEDIAN_S_UP;
	 Params->FRAMES_TRAINING = 20;
	 Params->ALPHA = 0 ;
	 Params->MORFOLOGIA = true;
	 Params->CVCLOSE_ITR = 1;
	 Params->MAX_CONTOUR_AREA = 0 ; //200
	 Params->MIN_CONTOUR_AREA = 0; //5
	 Params->K = 0.6745;

	 if (CREATE_TRACKBARS == 1){
				 // La primera vez inicializamos los valores.
				 if (first == 1){
					 Params->HIGHT_THRESHOLD = 20;

					 Params->LOW_THRESHOLD = 10;

					 first = 0;
				 }
	 			cvCreateTrackbar( "HighT",
	 							  "Foreground",
	 							  &Params->HIGHT_THRESHOLD,
	 							  100  );
	 			cvCreateTrackbar( "LowT",
	 							  "Foreground",
	 							  &Params->LOW_THRESHOLD,
	 							  100  );
	 }else{
		 Params->HIGHT_THRESHOLD = 20;
		 Params->LOW_THRESHOLD = 15;
	 }
}

void AllocateDataProcess( IplImage *I ) {

	CvSize size = cvGetSize( I );

		if(!Imagen) {
			Imagen = cvCreateImage( size ,8,1);
			FGMask = cvCreateImage( size, 8, 1);
			lastIdes = cvCreateImage( size ,IPL_DEPTH_32F,1);
			lastBG = cvCreateImage( size, 8, 1);
		}
		cvZero( Imagen );
		cvZero( FGMask );

}
void releaseDataProcess(ValParams* valParams, BGModelParams *BGPrParams){
	free(BGPrParams);
	free( valParams);
	cvReleaseImage( &Imagen );
	cvReleaseImage( &FGMask );
	cvReleaseImage( &lastBG );
	cvReleaseImage( &lastIdes );
	ReleaseDataSegm( );
}
