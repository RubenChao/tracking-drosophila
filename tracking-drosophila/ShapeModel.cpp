/*
 * ShapeModel.cpp
 *
 *  Created on: 19/09/2011
 *      Author: chao
 */

#include "ShapeModel.hpp"

SHModel* ShapeModel2( CvCapture* g_capture,StaticBGModel* BGModel ){

	int num_frames = 0;
	int total_blobs=0;
	int k=0;
	float Dif=0;
	float MaxContourArea = 0;
	float MaxContourPerimeter = 0;

	float areas[SM_FRAMES_TRAINING*15]; // almacena las areas de todos los blobs encontrados

	IplImage* frame = NULL;

	STFrame* frameData = NULL;
	BGModelParams* BGParams = NULL;
	SHModel* Shape = NULL;
	VisParams* visParams = NULL;
	CBlobResult blobs;
	CBlob *currentBlob;

	IplImage* ImGris = cvCreateImage(cvGetSize( BGModel->Imed ), 8, 1 );
	IplImage* Imblob = cvCreateImage(cvGetSize( BGModel->Imed ), 8, 3 );
	IplImage* lastBG = cvCreateImage( cvGetSize( BGModel->Imed ),8, 1 );
	IplImage* lastIdes = cvCreateImage( cvGetSize( BGModel->Imed ), IPL_DEPTH_32F, 1);
	cvZero(Imblob);
	// Iniciar estructura para modelo de forma

	Shape = ( SHModel *) malloc( sizeof( SHModel));
	if ( !Shape ) {error(4);return 0;}
	Shape->FlyAreaDes = 0;
	Shape->FlyAreaMed = 0;
	Shape->FlyAreaMedia=0;
	//Pone a 0 los valores del vector areas

	for(int i=0;i<SM_FRAMES_TRAINING*15;i++){
		areas[i]=0;
	}

	//EXTRACCION DE LOS BLOBS Y CALCULO DE MEDIANA/MEDIA Y DESVIACION TIPICA PARA TODOS LOS FRAMES

	while( num_frames < SM_FRAMES_TRAINING ){
		frame = cvQueryFrame( g_capture );
		if ( !frame ) {
			error(2);
			break;
		}
		if ( (cvWaitKey(10) & 255) == 27 ) break;

		ImPreProcess( frame, ImGris, BGModel->ImFMask, 0, BGModel->DataFROI);

		// Cargamos datos del fondo
		if(!frameData ) { //en la primera iteración iniciamos el modelo dinamico al estático
			// Iniciar estructura para datos del nuevo frame
			frameData = InitNewFrameData( frame );
			cvCopy(  BGModel->Imed,frameData->BGModel);
			cvSet(frameData->IDesvf, cvScalar(1));
			cvCopy(  BGModel->Imed,lastBG);
		}
		else{	// cargamos los últimos parámetros del fondo.
			cvCopy( lastBG, frameData->BGModel);
			cvCopy( lastIdes,frameData->IDesvf );
		}
	//	obtener la mascara del FG y la lista con los datos de sus blobs.
		//// BACKGROUND UPDATE
		// Actualización del fondo
		// establecer parametros
		if( BGParams == NULL ) DefaultBGMParams( &BGParams);
		UpdateBGModel( ImGris,frameData->BGModel,frameData->IDesvf, BGParams, BGModel->DataFROI, BGModel->ImFMask );
		/////// BACKGROUND DIFERENCE. Obtención de la máscara del foreground
		BackgroundDifference( ImGris, frameData->BGModel,frameData->IDesvf, frameData->FG ,BGParams, BGModel->DataFROI);

		// guardamos las imagenes para iniciar el siguiente frame
		cvCopy( frameData->BGModel, lastBG);
		cvCopy(  frameData->IDesvf,lastIdes);

		//Obtener los Blobs y excluir aquellos que no interesan por su tamaño
		cvSetImageROI(  frameData->FG , BGModel->DataFROI);

		blobs = CBlobResult( frameData->FG, NULL, 100, true );
		blobs.Filter( blobs, B_EXCLUDE, CBlobGetArea(),B_GREATER,100);
		blobs.Filter( blobs, B_EXCLUDE, CBlobGetPerimeter(),B_GREATER,1000);

		int j = blobs.GetNumBlobs();//numero de blos encontrados en el frame

		total_blobs=total_blobs+j; // Contabiliza los blobs encontrados para todos los frames

		// Guardar los Blobs en un archivo txt (OPCIONAL)

		//Recorrer Blob a blob y obtener las caracteristicas del AREA de cada uno de ellos

		for (int i = 0; i < blobs.GetNumBlobs(); i++ ){ //for 1

			currentBlob = blobs.GetBlob(i);

			CBlobGetArea();

			areas[k]=currentBlob->area;// almacenar el valor del area del blob en el vector

			// Mediana de las Areas

			if(currentBlob->area < Shape->FlyAreaMed) Shape->FlyAreaMed=Shape->FlyAreaMed-1;
			if(currentBlob->area > Shape->FlyAreaMed) Shape->FlyAreaMed=Shape->FlyAreaMed+1;

			//Media de las Areas

			Shape->FlyAreaMedia=Shape->FlyAreaMedia+currentBlob->area;//sumatorio de los valores de las areas

			if(SHOW_SHAPE_MODEL_DATA_AREAS) printf("Area blob %d = %f ",i,currentBlob->area);

			currentBlob->FillBlob( Imblob, CV_RGB(255,0,0));

			k++;//incrementar indice del vector que contiene las areas

		}//Fin del For 1
		num_frames += 1;
		cvResetImageROI(frameData->FG);

		DraWWindow(Imblob, frameData, BGModel, SHOW_SHAPE_MODELING, COMPLETO);
		DraWWindow(Imblob, frameData, BGModel, SHAPE,SIMPLE );

	}//Fin del while
//	if(SHOW_WINDOW){
		desvanecer( NULL, 20);
//	}
	Shape->FlyAreaMedia=Shape->FlyAreaMedia/total_blobs;// Media de las Areas para cada frame

	//Calcular la desvición típica

	for(int l=0;l<SM_FRAMES_TRAINING*15;l++){ // For 2

		Dif=abs(areas[l]-Shape->FlyAreaMed);// valor del area - mediana

		int valor=areas[l];

		if(valor == 0) break;

		if(Dif < Shape->FlyAreaDes) Shape->FlyAreaDes=Shape->FlyAreaDes-1;
		if(Dif > Shape->FlyAreaDes) Shape->FlyAreaDes=Shape->FlyAreaDes+1;

	} // Fin del For 2

	//Mostrar mediana y media para todos los frames

	if(SHOW_SHAPE_MODEL_DATA_MEDIANA )
		printf("\n MEDIANA AREAS: %f \t MEDIA AREAS: %f \t DESVIACION AREAS: %f",Shape->FlyAreaMed,Shape->FlyAreaMedia,Shape->FlyAreaDes);

	free( BGParams);
	liberarSTFrame( frameData );
	cvReleaseImage( &ImGris);
	cvReleaseImage( &Imblob);
	cvReleaseImage( &lastIdes);
	cvReleaseImage( &lastBG);

	return Shape;

}//Fin de la función ShapeModel2



