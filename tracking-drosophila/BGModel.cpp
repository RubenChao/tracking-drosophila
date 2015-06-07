/*!
 * BGModel.cpp
 *
 * Modelado de fondo
 *
 *
 *
 *
 *  Created on: 27/06/2011
 *      Author: chao
 */

#include "BGModel.h"

//int FRAMES_TRAINING = 20;
//int HIGHT_THRESHOLD = 20;
//int LOW_THRESHOLD = 10;
//double ALPHA = 0 ;

int g_slider_position = 50;


// Float 1-Channel
IplImage* Idesf; // Almacena el valor median|I(p)- median(p)| del frame t-1
IplImage *Idif;// Almacena |I(p)- median(p)|

//Byte 1-Channel
IplImage *ImGray; /// Imagen preprocesada

IplImage *Imaskt; // mascara para actualización no selectiva de fondo
IplImage *fgTemp; // copia del fg para buscar los contornos

IplImage *bgTemp = NULL; // imagen temporal para almacenar el fondo antes de actualización no selectiva
IplImage *desTemp = NULL;

/********************************************************************
 *			 			MODELO MEDIANA      						*
 * 																	*
 ********************************************************************/
StaticBGModel* initBGModel(  CvCapture* t_capture, BGModelParams* Param){
	struct timeval ti; // iniciamos la estructura
	float TiempoParcial;
	int num_frames = 0;
	static int count_update;
	static int Delay = 0;
	IplImage* frame = cvQueryFrame( t_capture );
	if ( !frame ) {
		error(2);
		exit(-1);
	}
	// aplicamos el retardo
	while( Delay < Param->initDelay){
		frame = cvQueryFrame( t_capture );
		Delay ++;
	}
	if ( (cvWaitKey(10) & 255) == 27 ) exit(-1);

	if( Param == NULL ) DefaultBGMParams( &Param );

	//Iniciar estructura para el modelo de fondo estático
	StaticBGModel* bgmodel;

	bgmodel = ( StaticBGModel*) malloc( sizeof( StaticBGModel));
	if ( !bgmodel ) {error(4);return(0);}

	bgmodel->PCentroX = 0;
	bgmodel->PCentroY = 0;
	bgmodel->PRadio = 0;

	AllocateBGMImages(frame, bgmodel );

	/////// BUSCAR PLATO Y OBTENER MÁSCARA /////

#ifdef	MEDIR_TIEMPOS gettimeofday(&ti, NULL);
#endif


	if  ( Param->FLAT_DETECTION ){

		if( Param->FLAT_FRAMES_TRAINING == 0) {
			printf("\n No ha establecido el número de frames que se usarán para la"
					" detección del plato. Se establecerán 500 por defecto.");
			Param->FLAT_FRAMES_TRAINING = 500; // por defecto
		}
		printf("\n\t\t1)Localizando y parametrizando plato...\n ");
		MascaraPlato( t_capture, bgmodel, Param->FLAT_FRAMES_TRAINING);

		if (bgmodel->PRadio == 0  ){
			char Opcion[2];
			int hecho = 0;
			error(3);
			do{
				printf("\n Indique si desea detener la ejecución (D/d)"
						"o si prefiere continuar sin la detección del plato (C/c):");
				fscanf(stdin,"%s",Opcion );
				fflush(stdin);
				if (Opcion[0] == 'd'|| Opcion[0] =='D') return 0;
				else if (Opcion[0] == 'c'||Opcion[0] =='C'){
					bgmodel->DataFROI = cvRect(0,0,frame->width,frame->height );
					bgmodel->PCentroX = 0;
					bgmodel->PCentroY = 0;
					bgmodel->PRadio = 0;
					cvZero(bgmodel->ImFMask);
					hecho = 1;
				}
				else{
					printf("\nPorfavor, indique D/d detener ó c/C continuar\n");
				}
			}
			while(!hecho);
		}
	}
	// Datos para establecer ROI si no se quiere detectar el plato
	else{
		printf("\n No ha activado la detección del plato.Esto afectará a la velocidad"
				"de ejecución.\nEscoja si desea activar dicha opción:");

		bgmodel->DataFROI = cvRect(0,0,frame->width,frame->height );
		cvZero(bgmodel->ImFMask);
	}

#ifdef	MEDIR_TIEMPOS
	TiempoParcial = obtenerTiempo( ti , 1);
	printf("\n\t\t-Tiempo total %0.2f s\n", TiempoParcial);
#endif

	printf("\n\t\t2)Aprendiendo fondo...\n ");
#ifdef	MEDIR_TIEMPOS gettimeofday(&ti, NULL);
#endif

	///// APRENDER FONDO /////

	// Acumulamos el fondo para obtener la mediana y la varianza de cada pixel en FRAMES_TRAINING frames  ////
	Delay = 0; // retardo para iniciar la captura. Para descartar los primeros frames
	count_update = Param->BG_Update-1;

	int totalFrames = 0;
	totalFrames = cvGetCaptureProperty( t_capture, CV_CAP_PROP_FRAME_COUNT);
	if(!totalFrames) totalFrames = getAVIFrames("Drosophila.avi"); // en algun linux no funciona lo anterior
	int intervalJump = totalFrames/ Param->Jumps; // numero de frames entre cada posición de salto

	int jumpFrCount = 0; // contador del numero de frames para el salto.
	int frForJump = cvRound(Param->FRAMES_TRAINING / Param->Jumps); // numero de frames por salto
	double Seek = 0 + Param->initDelay ; // puntero a las partes del video donde se saltará

	while( num_frames < Param->FRAMES_TRAINING ){

		if( jumpFrCount == frForJump && Param->Jumps ){ // si se alcanza el numero de frames por salto
			Seek = Seek + intervalJump; // incrementamos el puntero al siguiente intervalo
			cvSetCaptureProperty( t_capture,1,Seek ); // establecemos la posición
			Param->Jumps --;
			jumpFrCount = 0;
		}
		frame = cvQueryFrame( t_capture );
		if ( !frame ) {
			error(2);
			break;
		}
		if ( (cvWaitKey(10) & 255) == 27 ) break;
		while( Delay < Param->initDelay){
			frame = cvQueryFrame( t_capture );
			Delay ++;
		}
		if( count_update == Param->BG_Update-1) {
			ImPreProcess( frame, ImGray, bgmodel->ImFMask, false, bgmodel->DataFROI);
			//Inicialización
			if ( num_frames == 0){
				// iniciar mediana a la primera imagen
				cvCopy( ImGray, bgmodel->Imed );
				cvCopy( ImGray, bgTemp );
				// Iniciamos la desviación a un valor muy pequeño
				//apropiado para fondos con poco movimiento (unimodales).
				iniciarIdesv( desTemp,Param->INITIAL_DESV, bgmodel->ImFMask, Param->K );
				cvCopy( desTemp, bgmodel->IDesvf );
			}
			// Aprendizaje del fondo
			UpdateBGModel(ImGray, bgTemp, desTemp, Param,  bgmodel->DataFROI, bgmodel->ImFMask );
			// Actualización selectiva
			if( num_frames > 20 ){
				/////// BACKGROUND DIFERENCE. Obtención de la máscara del foreground

				BackgroundDifference( ImGray, bgmodel->Imed,bgmodel->IDesvf, Imaskt ,Param, bgmodel->DataFROI);
				// dilatamos
				cvSetImageROI( Imaskt, bgmodel->DataFROI);
				cvDilate( Imaskt, Imaskt, 0, 2 );
				cvResetImageROI( Imaskt );

				if( Param->MODEL_TYPE == MEDIAN_S_UP)
					UpdateBGModel(ImGray, bgmodel->Imed, NULL, Param,  bgmodel->DataFROI, Imaskt );
				else
					UpdateBGModel( ImGray, bgmodel->Imed, bgmodel->IDesvf, Param,  bgmodel->DataFROI, Imaskt );
				cvCopy(bgmodel->Imed, bgTemp);
				cvCopy(bgmodel->IDesvf, desTemp);
			}

			DraWWindow( frame, NULL , bgmodel, SHOW_INIT_BACKGROUND, COMPLETO );

			DraWWindow( frame, NULL , bgmodel, BG_MODEL, SIMPLE );

			count_update = 0;

		}
		count_update +=1;
		num_frames += 1;
		jumpFrCount +=1;
	}
//	if(SHOW_WINDOW) Transicion4("Aprendiendo fondo...", 50);
#ifdef	MEDIR_TIEMPOS
	TiempoParcial = obtenerTiempo( ti , 1);
	printf("\t\t-Tiempo total %0.2f s\n", TiempoParcial);
#endif

	return bgmodel;
}

void MascaraPlato(CvCapture* t_capture,
		StaticBGModel* Flat,int frTrain){

	//int* centro_x, int* centro_y, int* radio
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* circles = NULL;
	IplImage* Im = NULL;



	int minRadio;
	int medX;
	int medY;
	int medR;

	// LOCALIZACION
	int num_frames = 0;

	//	t_capture = cvCaptureFromAVI( "Drosophila.avi" );
	if ( !t_capture ) {
		fprintf( stderr, "ERROR: capture is NULL \n" );
		getchar();
		return ;
	}

	// Obtención del centro de máximo radio del plato en 50 frames

	//	GlobalTime = (double)cvGetTickCount() - GlobalTime;
	//	printf( " %.1f\n", GlobalTime/(cvGetTickFrequency()*1000.) );

	while (num_frames < frTrain) {

		IplImage* frame = cvQueryFrame( t_capture );
		if ( !frame ) {
			fprintf( stderr, "ERROR: frame is null...\n" );
			getchar();
			break;
		}
		if ( (cvWaitKey(10) & 255) == 27 ) break;

		//		VerEstadoBGModel( frame );
		if(!Im){ // inicializar
			Im = cvCreateImage(cvSize(frame->width,frame->height),8,1);
			minRadio = cvRound(frame->height / 3); // radio minimo inicial
			medX = frame->width /2;
			medY = frame->height/2;
			medR = minRadio;
			if(SHOW_WINDOW){

			//	Transicion4( "Buscando plato... ", 20 );
			}
		}
		// Imagen a un canal de niveles de gris
		cvCvtColor( frame , Im, CV_BGR2GRAY);
		cvSmooth(Im,Im,CV_GAUSSIAN,5,5);
		cvCopy( Im, Flat->Imed ); // usaremos esta imagen de forma temporal para visualizar resultados

		// Filtrado gaussiano 5x
		//cvSmooth(Im,Im,CV_GAUSSIAN,5,0,0,0);

		//		cvShowImage( "Foreground",Im);
		//		cvWaitKey(0);
		// establecemos el radio mínimo según la resolución de la imagen

		cvCanny(Im,Im, 50, 100, 3);
//		cvShowImage( "Foreground",Im);
//		cvWaitKey(0);

		circles = cvHoughCircles(Im, storage,CV_HOUGH_GRADIENT,4,Im->height/4,200,100, minRadio,
				cvRound( Im->height )/2 + 20);
		int i;
		for (i = 0; i < circles->total; i++)
		{

			// Excluimos los radios que se salgan de la imagen
			float* p = (float*)cvGetSeqElem( circles, i );

			if(  (  cvRound( p[0]) < cvRound( p[2] )  ) || // si x es menor que r ( circ se sale por la izda )
					( ( Im->width - cvRound( p[0]) ) < cvRound( p[2] )  ) || //o si el ancho menos x es menor que r ( se sale por la dercha)
					(  cvRound( p[1] )  < cvRound( p[2]-10 ) ) || // o si y es menor que r (se sale por arriba )
					( ( Im->height - cvRound( p[1]) ) < cvRound( p[2]-10 ) ) // se sale por abajo
			) continue;
			if ( cvRound( p[0] )  > medX ) medX++;
							else if ( cvRound( p[0] )  < medX ) medX--;
							if ( cvRound( p[1] )  > medY ) medY++;
							else if ( cvRound( p[1] )  < medY ) medY--;
							if ( cvRound( p[2] )  > medR ) medR++;
							else if ( cvRound( p[2] )  < medR ) medR--;

			// Buscamos el de mayor radio de entre todos los frames;
			if (  Flat->PRadio <= cvRound( p[2] ) ){

				Flat->PRadio = cvRound( p[2] );
				Flat->PCentroX = cvRound( p[0] );
				Flat->PCentroY = cvRound( p[1] );
				if(SHOW_BGMODEL_DATA ) printf("\t\t\t-Centro x:\t%d \t\t\t-Centro y:\t%d \t\t\t-Radio:\t%d \n"
						,Flat->PCentroX,Flat->PCentroY , Flat->PRadio);
			}

			if (Flat->PRadio>0){
				 cvCircle( Flat->Imed, cvPoint(Flat->PCentroX,Flat->PCentroY ), Flat->PRadio, CVX_RED, 1, 8, 0);
				 cvCircle( Flat->Imed, cvPoint(cvRound( p[0]),cvRound( p[1] ) ), cvRound( p[2] ), CVX_BLUE, 1, 8, 0);
			}
			cvAdd( Im, Flat->Imed,Flat->Imed);
			DraWWindow( NULL,NULL, Flat, SHOW_LEARNING_FLAT, COMPLETO );		// dibujar modo completo
			DraWWindow(frame, NULL, Flat, FLAT, SIMPLE); // dibujar visualización preprocesado

		} // FIN FOR Hecho

		if (Flat->PRadio>0)	cvCircle(Flat->Imed , cvPoint(medX,medY ), medR, CVX_GREEN, 1, 8, 0);

		DraWWindow(frame, NULL, Flat, SHOW_LEARNING_FLAT, COMPLETO);
		DraWWindow(frame, NULL, Flat, FLAT, SIMPLE );

		minRadio = medR ;
		num_frames +=1;
		cvClearMemStorage( storage);
	}
	cvReleaseMemStorage( &storage );
	if (Flat->PRadio>0){
		Flat->PRadio = medR;
		Flat->PCentroX = medX;
		Flat->PCentroY = medY;
	}

	printf("\t\t-Plato localizado. Estableciendo parámetros :\n ");
	printf("\t\t\t-Centro x : %d \n\t\t\t-Centro y %d \n\t\t\t-Radio: %d \n"
			,Flat->PCentroX,Flat->PCentroY , Flat->PRadio);

	// establecemos la roi del plato
	int x;
	int y;
	int width;
	int height;

	if(  Flat->PCentroX < Flat->PRadio  ) x = 0; // si x es menor que r ( circ se sale por la izda )
	else x = Flat->PCentroX-Flat->PRadio;
	if(  Flat->PCentroY < Flat->PRadio  )  y = 0; //  si y es menor que r (se sale por arriba )
	else y = Flat->PCentroY-Flat->PRadio;
	if( ( Im->width - Flat->PCentroX ) < Flat->PRadio  ) width = Im->width - x ;//o si el ancho menos x es menor que r ( se sale por la derecha)
	else width = 2*Flat->PRadio;
	if( ( Im->height - Flat->PCentroY ) < Flat->PRadio ) height = Im->height- y;
	else height = 2*Flat->PRadio;

	Flat->DataFROI = cvRect(x, y, width, height ); // Datos para establecer ROI del plato


	// Creacion de mascara
	//	GlobalTime = (double)cvGetTickCount() - GlobalTime;
	//	printf( " %.1f\n", GlobalTime/(cvGetTickFrequency()*1000.) );
	for (int y = 0; y< Im->height; y++){
		//puntero para acceder a los datos de la imagen
		uchar* ptr = (uchar*) ( Flat->ImFMask->imageData + y*Flat->ImFMask->widthStep);
		// Los pixeles fuera del plato se ponen a 255;
		for (int x= 0; x<Flat->ImFMask->width; x++){
			if ( sqrt( pow( abs( x - Flat->PCentroX ) ,2 )
					+ pow( abs( y - Flat->PCentroY  ), 2 ) )
					> Flat->PRadio) ptr[x] = 255;
			else	ptr[x] = 0;
		}
	}
//	if(SHOW_WINDOW) Transicion4("Buscando plato...", 50);
	printf("\t\t-Creación de máscara de plato finalizada.");
	cvReleaseImage(&Im);

	return;
}
void accumulateBackground( IplImage* ImGray, IplImage* BGMod,IplImage *Idesvf,CvRect ROI , float K, IplImage* mask = NULL ) {
	// si la máscara es null(POR DEFECTO), se crea una inicializada a 0, lo que implicará
	// la actualización de todos los pixeles del background
	struct timeval ti; // iniciamos la estructura
	double tiempoParcial;
	if (mask == NULL){
		cvZero( Imaskt);
	}
	else cvCopy(mask, Imaskt);

	//	muestrearLinea( ImGray,cvPoint( 120, 267 ),	cvPoint( 220, 267 ), 5000);

	if( SHOW_BGMODEL_DATA ){
		printf("\n\n Imagenes antes de actualizar fondo" );
		CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
		printf("\n\n Matriz Brillo ");
		verMatrizIm(ImGray, ventana);
		printf("\n\n Matriz BGModel ");
		verMatrizIm(BGMod, ventana);
		printf("\n\n Matriz Idif(p) = Brillo(p)-BGModel(p) ");
		verMatrizIm(Idif, ventana);
		printf("\n\n Matriz Idesvf(p) ");
		verMatrizIm(Idesvf, ventana);
	}

	// Se estima la mediana. Se actualiza el fondo usando la máscara.
	// median(p)
#ifdef	MEDIR_TIEMPOS gettimeofday(&ti, NULL);
#endif
	updateMedian( ImGray, BGMod, Imaskt, ROI );
	if( Idesvf != NULL){
		updateDesv( ImGray, BGMod, Idesvf, Imaskt, ROI, K );
	}
#ifdef	MEDIR_TIEMPOS
	tiempoParcial = obtenerTiempo( ti , NULL);
	printf("\t\tActualización de mediana: %5.4g ms\n", tiempoParcial);
#endif
	if( SHOW_BGMODEL_DATA ){
		printf("\n\n Imagenes tras actualizar fondo" );
		CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
		printf("\n\n Matriz Brillo ");
		verMatrizIm(ImGray, ventana);
		printf("\n\n Matriz BGModel ");
		verMatrizIm(BGMod, ventana);
		printf("\n\n Matriz Idif(p) = Brillo(p)-BGModel(p) ");
		verMatrizIm(Idif, ventana);
	}
}

void iniciarIdesv( IplImage* Idesv,float Valor, IplImage* mask, float K ){

	int step0		= Idesv->widthStep/sizeof(float);
	int step1       = Idesf->widthStep/sizeof(float);
	int step2		 = mask->widthStep/sizeof(uchar);

	float * ptr0   = (float *)Idesv->imageData;
	float * ptr1   = (float *)Idesf->imageData;
	uchar* ptr2 = (uchar*) mask->imageData;
	// ojo, la mascara está invertida. El plato es 0
	for (int i = 0; i< Idesv->height; i++){
		for (int j = 0; j < Idesv->width; j++){
			if( ptr2[i*step2+j*Idesv->nChannels ] == 0){
				ptr1[i*step1+j*Idesv->nChannels ] = Valor/K; // columnas
				ptr0[i*step0+j*Idesv->nChannels ] = Valor;
			}
			else{
				ptr1[i*step1+j*Idesv->nChannels ] = 0; // columnas
				ptr0[i*step1+j*Idesv->nChannels ] = 0;
			}
		}
	}
}

void updateMedian(IplImage* ImGray,IplImage* Median,IplImage* mask,CvRect ROI ){

	for (int y = ROI.y; y< ROI.y + ROI.height; y++){
		uchar* ptr0 = (uchar*) ( mask->imageData + y*mask->widthStep + 1*ROI.x);
		uchar* ptr1 = (uchar*) ( ImGray->imageData + y*ImGray->widthStep + 1*ROI.x);
		uchar* ptr2 = (uchar*) ( Median->imageData + y*Median->widthStep + 1*ROI.x);
		for (int x = 0; x < ROI.width; x++){
			// Incrementar o decrementar fondo en una unidad
			if ( ptr0[x] == 0 ){ // si el pixel de la mascara es 0 actualizamos
				if ( ptr1[x] < ptr2[x] ) ptr2[x] = ptr2[x]-1;
				if ( ptr1[x] > ptr2[x] ) ptr2[x] = ptr2[x]+1;
			}
		}
	}

}

void updateDesv( IplImage* ImGray,IplImage* BGMod,IplImage* Idesvf,IplImage* Imaskt, CvRect ROI, float K ){

	int step1       = Idesf->widthStep/sizeof(float);
	int step4       = Idesf->widthStep/sizeof(float);
	int step2		= Idif->widthStep/sizeof(uchar);
	int step3		= Imaskt->widthStep/sizeof(uchar);
	int step5		= ImGray->widthStep/sizeof(uchar);
	int step6		= BGMod->widthStep/sizeof(uchar);

	float * des   = (float *)Idesf->imageData;
	float * desvf   = (float *)Idesvf->imageData;
	uchar* dif = (uchar*) Idif->imageData;
	uchar* mask = (uchar*) Imaskt->imageData;
	uchar* Gray = (uchar*) ImGray->imageData;
	uchar* BG = (uchar*) BGMod->imageData;

	// ojo, la mascara está invertida. El plato es 0
	// Corregimos la estimación de la desviación mediante la función de error
	// Así se asegura que la fracción correcta de datos esté dentro de una desviación estándar
	// ( escalado horizontal de la normal).
	// Idesf(p) = k*MAD = k*median(I(p)-median(p)) si mask == 0
	for (int i = ROI.y; i< ROI.y + ROI.height; i++){
		for (int j = ROI.x; j < ROI.x + ROI.width; j++){
			if ( mask[i*step3+j] == 0 ){
				dif[ i*step2+j ] = abs( Gray[i*step5+j] - BG[i*step6+j]);
				if ( dif[ i*step2+j ] < des[ i*step1+j ] ){
					des[ i*step1+j ] -=1;
					desvf[i*step4+j] = K*des[ i*step1+j ]; // Corrección MAD
				}
				if ( dif[ i*step2+j ] > des[ i*step1+j ] ){
					des[ i*step1+j ] +=1;
					desvf[i*step4+j] = K*des[ i*step1+j ];
				}
			}
		}
	}

}

void UpdateBGModel( IplImage* tmp_frame, IplImage* BGModel,IplImage* ImDesv, BGModelParams* Param, CvRect DataROI,IplImage* Mask){

	// mediana
	if(Param->MODEL_TYPE == MEDIAN || Param->MODEL_TYPE == MEDIAN_S_UP){
		//no selectiva
		if ( Param->MODEL_TYPE == MEDIAN || Mask == NULL) accumulateBackground( tmp_frame, BGModel,NULL, DataROI ,Param->K, 0);
		else{
			//selectiva
			accumulateBackground( tmp_frame, BGModel,NULL, DataROI, Param->K, Mask);
		}
	}
	// gausiana simple
	else if(Param->MODEL_TYPE == GAUSSIAN || Param->MODEL_TYPE == GAUSSIAN_S_UP){
		if ( Param->MODEL_TYPE == GAUSSIAN || Mask == NULL) accumulateBackground( tmp_frame, BGModel,ImDesv, DataROI ,Param->K, 0);
		else{
			// selectiva
			accumulateBackground( tmp_frame, BGModel,ImDesv, DataROI ,Param->K, Mask );
		}
	}
}
void RunningBGGModel( IplImage* Image, IplImage* median, IplImage* Idesf, double ALPHA,CvRect dataroi ){

	IplImage* ImTemp;
	CvSize sz = cvGetSize( Image );
	ImTemp = cvCreateImage( sz, 8, 1);

	cvCopy( Image, ImTemp);

	cvSetImageROI( ImTemp, dataroi );
	cvSetImageROI( median, dataroi );
	cvSetImageROI( Idif, dataroi );
	cvSetImageROI( Idesf, dataroi );

	// Para la mediana
	cvConvertScale(ImTemp, ImTemp, ALPHA,0);
	cvConvertScale( median, median, (1 - ALPHA),0);
	cvAdd(ImTemp , median , median );

	// Para la desviación típica
	cvConvertScale(Idif, ImTemp, ALPHA,0);
	cvConvertScale( Idesf, Idesf, (1 - ALPHA),0);
	cvAdd(ImTemp , Idesf , Idesf );

	cvResetImageROI( median );
	cvResetImageROI( Idif );
	cvResetImageROI( Idesf );

	cvReleaseImage( &ImTemp );

	DeallocateTempImages();
}
void BackgroundDifference( IplImage* ImGray, IplImage* bg_model,IplImage* Idesvf,IplImage* fg,BGModelParams* Param, CvRect ROI){

	struct timeval ti; // iniciamos la estructura
	double tiempoParcial;


	if( SHOW_BGMODEL_DATA ){
		printf("\n\nANTES de resta y limpieza de fondo");
		CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
		printf("\n\n Matriz Brillo ");verMatrizIm(ImGray, ventana);
		printf("\n\n Matriz BGModel ");verMatrizIm(bg_model, ventana);
		printf("\n\n Matriz Idesvf(p) ");verMatrizIm(Idesvf, ventana);
		printf("\n\n Matriz Idif(p) = Brillo(p)-BGModel(p) ");
		verMatrizIm(Idif, ventana);
		printf("\n\n Matriz FG(p) ");verMatrizIm(fg, ventana);
		//			cvShowImage( "Foreground", fg);
		//			cvShowImage( "Background",bg_model);
		//			cvWaitKey(0);
	}
#ifdef	MEDIR_TIEMPOS gettimeofday(&ti, NULL);
#endif
	if(Param->MODEL_TYPE == MEDIAN || Param->MODEL_TYPE == MEDIAN_S_UP){
		// Calcular (I(p)-u(p))/0(p) y umbralizar la diferencia
		for (int y = ROI.y; y < ROI.y + ROI.height; y++){
			uchar* ptr1 = (uchar*) ( ImGray->imageData + y*ImGray->widthStep + 1*ROI.x);
			uchar* ptr2 = (uchar*) ( bg_model->imageData + y*bg_model->widthStep + 1*ROI.x);
			uchar* ptr3 = (uchar*) ( Idif->imageData + y*Idif->widthStep + 1*ROI.x);
			uchar* ptr4 =  (uchar*) ( fg->imageData + y*fg->widthStep + 1*ROI.x);
			for (int x = 0; x<ROI.width; x++){
				// si (I(p) - Me(p)) >= 0 Idif(p) = 0 => pixel de bg mas oscuro que fg ( fantasma ). lo eliminamos
				if ( (ptr1[x]-ptr2[x]) > 0 ) ptr3[x] = 0;
				// sino Idif = |I(p) - Me(p)|
				else ptr3[x] = abs( ptr1[x]-ptr2[x]);
				// Si Idif(p) > LowT => Al FG sino al BG
				if ( ptr3[x] > (Param->LOW_THRESHOLD) ) ptr4[x] = 255; //Normal: (Param->LOW_THRESHOLD)*ptr4[i*step4+j]
				else ptr4[x] = 0;
			}
		}
	}
	else{
		// Umbralizamos la diferencia normalizada
		int step1		= ImGray->widthStep/sizeof(uchar);
		int step2		= bg_model->widthStep/sizeof(uchar);
		int step3		= Idif->widthStep/sizeof(uchar);
		int step4       = Idesvf->widthStep/sizeof(float);
		int step5		= fg->widthStep/sizeof(uchar);

		uchar* ptr1    = (uchar*) ImGray->imageData;
		uchar* ptr2    = (uchar*) bg_model->imageData ;
		uchar* ptr3    = (uchar*) Idif->imageData ;
		float* ptr4    = (float*) Idesvf->imageData;
		uchar* ptr5    = (uchar*) fg->imageData;

		for (int i = ROI.y; i< ROI.y + ROI.height; i++){
			for (int j = ROI.x; j < ROI.x + ROI.width; j++){
				// si (I(p) - Me(p)) >= 0 Idif(p) = -1 => pixel de bg mas oscuro que fg ( fantasma ). lo eliminamos
				if ( (ptr1[i*step1+j]-ptr2[i*step2+j]) > 0 ) ptr3[i*step3+j] = 0;
				// sino Idif = |I(p) - Me(p)|
				else ptr3[i*step3+j] = abs( ptr1[i*step1+j]-ptr2[i*step2+j]);
				// Si Idif(p) > LowT*Idesv(p) => Al FG sino al BG
				if ( ptr3[i*step3+j] > (Param->LOW_THRESHOLD)*ptr4[i*step4+j] ) ptr5[i*step5+j] = 255; //Normal:
				else ptr5[i*step5+j] = 0;
			}
		}
	}

#ifdef	MEDIR_TIEMPOS
	if(SHOW_BGMODEL_TIMES) {tiempoParcial = obtenerTiempo( ti , NULL);
	printf("\t\tUmbralizacion low trhreshold: %5.4g ms\n", tiempoParcial);}
#endif
	if( SHOW_BGMODEL_DATA ){
		printf("\n\nTRAS resta de fondo");
		CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
		printf("\n\n Matriz Brillo ");verMatrizIm(ImGray, ventana);
		printf("\n\n Matriz BGModel ");verMatrizIm(bg_model, ventana);

		printf("\n\n Matriz Idif(p) = Brillo(p)-BGModel(p) ");
		verMatrizIm(Idif, ventana);
		printf("\n\n Matriz FG(p) ");verMatrizIm(fg, ventana);

	}
	DraWWindow( NULL,NULL, NULL, SHOW_BG_DIF_IMAGES, COMPLETO  );

	// limpieza de FG
#ifdef	MEDIR_TIEMPOS
	if(SHOW_BGMODEL_TIMES) gettimeofday(&ti, NULL);
#endif
	if(Param->MODEL_TYPE == MEDIAN || Param->MODEL_TYPE == MEDIAN_S_UP)
		FGCleanup( fg, NULL,Param, ROI );
	else FGCleanup( fg, Idesvf,Param, ROI );
#ifdef	MEDIR_TIEMPOS
	if(SHOW_BGMODEL_TIMES){tiempoParcial = obtenerTiempo( ti , NULL);
	printf("\t\tLimpieza de FG: %5.4g ms\n", tiempoParcial);}
#endif
	if( SHOW_BGMODEL_DATA ){
		printf("\n\nTRAS limpieza de fondo");
		CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
		printf("\n\n Matriz Brillo ");verMatrizIm(ImGray, ventana);
		printf("\n\n Matriz BGModel ");verMatrizIm(bg_model, ventana);
		printf("\n\n Matriz Idesf(p) ");verMatrizIm(Idesf, ventana);
		printf("\n\n Matriz Idif(p) = Brillo(p)-BGModel(p) ");
		verMatrizIm(Idif, ventana);
		printf("\n\n Matriz FG(p) ");verMatrizIm(fg, ventana);

	}

	DraWWindow( NULL,NULL, NULL, SHOW_BG_DIF_IMAGES, COMPLETO  );
}
/// Limpia y redibuja el FG
void FGCleanup( IplImage* FG, IplImage* DES, BGModelParams* Param, CvRect dataroi){

	static CvMemStorage* mem_storage = NULL;
	static CvSeq* contours = NULL;
	// Aplicamos morfologia: Erosión y dilatación
	if( Param->MORFOLOGIA == true){
		cvSetImageROI( FG, dataroi);
		cvErode( FG, FG, 0, 1);
		cvDilate( FG, FG, 0, 1 );
		cvResetImageROI( FG );
	}
	cvCopy(FG, fgTemp);
	cvZero ( FG );

	if( mem_storage == NULL ){
		mem_storage = cvCreateMemStorage(0);
	} else {
		cvClearMemStorage( mem_storage);
	}

	cvSetImageROI( fgTemp, dataroi);
	CvContourScanner scanner = cvStartFindContours(
			fgTemp,
			mem_storage,
			sizeof(CvContour),
			CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE,
			cvPoint(dataroi.x,dataroi.y) );
	CvSeq* c;

	int numCont = 0;
	while( (c = cvFindNextContour( scanner )) != NULL ) {
		CvSeq* c_new;
		bool flag = false;

		// los contornos que no estén dentro de un rango de areas no se tienen en cuenta
		if ( Param->MIN_CONTOUR_AREA > 0 || Param->MAX_CONTOUR_AREA > 0 ){
			double area = cvContourArea( c );
			area = fabs( area );
			if ( ( (area < Param->MIN_CONTOUR_AREA)&& Param->MIN_CONTOUR_AREA > 0) ||
					( (area > Param->MAX_CONTOUR_AREA)&& Param->MAX_CONTOUR_AREA > 0) ) {
				continue;
			}
		}
		// No se dibujan los contornos que no sobrevivan al HIGHT_THRESHOLD
		// Primero obtenermos ROI del contorno del LOW_THRESHOLD
		CvRect ContROI = cvBoundingRect( c );
		flag = false;
		if( !DES){
			for (int y = ContROI.y; y< ContROI.y + ContROI.height; y++){
				uchar* ptr0 = (uchar*) ( fgTemp->imageData + y*fgTemp->widthStep + ContROI.x);
				uchar* ptr1 = (uchar*) ( Idif->imageData + y*Idif->widthStep + ContROI.x);
				for (int x = 0; x < ContROI.width; x++){
					if ( ptr1[x] > Param->HIGHT_THRESHOLD ){
						flag = true;
						break;
					}
				}
				if (flag == true) break;
			}
		}
		else{
			int step1		= Idif->widthStep/sizeof(uchar);
			int step2       = DES->widthStep/sizeof(float);


			uchar* ptr1    = (uchar*) Idif->imageData ;
			float* ptr2    = (float*) DES->imageData;

			flag = false;
			for (int i = ContROI.y; i< ContROI.y + ContROI.height; i++){
				for (int j = ContROI.x; j < ContROI.x + ContROI.width; j++){
					// Si alguno de los pixeles del blob supera en HiF veces la
					// desviación típica del modelo,activamos el flag para
					// dibujar el contorno
					//if( ptr0[i*step0 + j] == 255){
					if ( ptr1[ i*step1 + j]*ptr2[i*step2 + j] > Param->HIGHT_THRESHOLD ){//
						flag = true;
						break;
					}
					//}
				}
				if (flag == true) break;
			}
		}
		if ( flag == true){ // Se dibuja el contorno
			c_new = cvConvexHull2( c, mem_storage, CV_CLOCKWISE, 1);
			cvDrawContours( FG, c_new, CVX_WHITE,CVX_WHITE,-1,CV_FILLED,8 );
			numCont++;
		}
		else continue; // Se pasa al siguiente

	}// Fin contornos
	contours = cvEndFindContours( & scanner );
	cvResetImageROI( fgTemp);

}




//void onTrackbarSlide(pos, BGModelParams* Param) {
//   Param->ALPHA = pos / 100;
//}
void DefaultBGMParams( BGModelParams **Parameters){
	//init parameters
	BGModelParams *Params;

	if( *Parameters == NULL )
	{
		Params = ( BGModelParams *) malloc( sizeof( BGModelParams) );
		Params->FLAT_DETECTION = true; //! Activa o desactiva la detección del plato
		Params->FLAT_FRAMES_TRAINING = 50;
		Params->MODEL_TYPE = MEDIAN_S_UP;
		Params->Jumps = 0;
		Params->initDelay = 0;

		Params->FRAMES_TRAINING = 20;
		Params->ALPHA = 0.5 ;
		Params->MORFOLOGIA = true;
		Params->CVCLOSE_ITR = 0;
		Params->MAX_CONTOUR_AREA = 0 ;
		Params->MIN_CONTOUR_AREA = 0;
		Params->HIGHT_THRESHOLD = 20;
		Params->LOW_THRESHOLD = 10;
		Params->INITIAL_DESV = 0.05;
		Params->K = 0.6745;
		*Parameters = Params ;
	}
	else
	{
		Params = *Parameters;
		Params->FLAT_DETECTION = true;
		Params->FLAT_FRAMES_TRAINING = 50;
		Params->FRAMES_TRAINING = 20;
		Params->MODEL_TYPE = MEDIAN_S_UP;
		Params->Jumps = 0;
		Params->initDelay = 0;

		Params->ALPHA = 0.5 ;
		Params->MORFOLOGIA = true;
		Params->CVCLOSE_ITR = 0;
		Params->MAX_CONTOUR_AREA = 0 ;
		Params->MIN_CONTOUR_AREA = 0;
		Params->HIGHT_THRESHOLD = 20;
		Params->LOW_THRESHOLD = 10;
		Params->INITIAL_DESV = 0.05;
		Params->K = 0.6745;
	}

}
/// localiza en memoria las imágenes necesarias para la ejecución

void AllocateBGMImages( IplImage* I , StaticBGModel* bgmodel){

	// Crear imagenes y redimensionarlas en caso de que cambien su tamaño
	AllocateTempImages( I );
	CvSize size = cvGetSize( I );

	//	if( !FrameData->BGModel ||
	//		 FrameData->BGModel->width != size.width ||
	//		 FrameData->BGModel->height != size.height ) {

	bgmodel->Imed = cvCreateImage(size,8,1);
	bgmodel->ImFMask = cvCreateImage(size,8,1);
	bgmodel->IDesvf = cvCreateImage(size,IPL_DEPTH_32F,1);

	cvZero( bgmodel->Imed);
	//cvZero( bgmodel->IDesvf);
	cvZero( bgmodel->ImFMask);

}

/// Limpia de la memoria las imagenes usadas durante la ejecución
void DeallocateBGM( StaticBGModel* BGModel){

	if(BGModel){
		DeallocateTempImages();
		cvReleaseImage(&BGModel->IDesvf);
		cvReleaseImage(&BGModel->ImFMask);
		cvReleaseImage(&BGModel->Imed);
		free(BGModel);
	}
}

void AllocateTempImages( IplImage *I ) {  // I is just a sample for allocation purposes

	CvSize sz = cvGetSize( I );
	if( !Idif  ) {

		Idesf = cvCreateImage( sz, IPL_DEPTH_32F, 1 );
		Idif = cvCreateImage( sz, 8, 1 );
		ImGray = cvCreateImage( sz, 8, 1 );
		Imaskt = cvCreateImage( sz, 8, 1 );
		fgTemp = cvCreateImage(sz, IPL_DEPTH_8U, 1);
		bgTemp = cvCreateImage(sz, IPL_DEPTH_8U, 1);
		desTemp = cvCreateImage(sz, IPL_DEPTH_32F, 1);

		cvZero( Idesf);
		cvZero( Idif );
		cvZero(ImGray);

		cvZero(Imaskt);
		cvZero(fgTemp);
		cvZero(bgTemp);
		cvZero(desTemp);
	}
}

void DeallocateTempImages() {

	cvReleaseImage( &Idesf);
	cvReleaseImage( &Idif );
	cvReleaseImage( &Imaskt );

	cvReleaseImage( &ImGray );
	cvReleaseImage( &fgTemp );

	cvReleaseImage( &bgTemp );
	cvReleaseImage( &desTemp );

	Idesf = NULL;
	Idif = NULL;
	ImGray = NULL;
	Imaskt = NULL;
	fgTemp = NULL;
	bgTemp = NULL;
	desTemp = NULL;
}









