/*
 * Visualizacion.cpp
 *
 *  Created on: 27/10/2011
 *      Author: chao
 */

#include "Visualizacion.hpp"
#include <opencv2/video/tracking.hpp>
#include <ctype.h>

using namespace cv;
using namespace std;

extern float TiempoGlobal ;
extern float TiempoFrame;
extern double NumFrame ; /// contador de frames absolutos ( incluyendo preprocesado )



IplImage* ImVisual = NULL; // imagen del video en la que si dibujaran los blobs y demas datos
IplImage* Window = NULL; // ventana de visualización donde se incrustará ImVisual y los datos
IplImage* ImScale= NULL; // Imagen escalada de ImVisual si ImVisual es menor o igual a 320 de ancho

// parametros de visualización
VisParams* visParams = NULL;
CvVideoWriter* VWriter;

void VisualizarEl( tlcde* frameBuf, int pos,  StaticBGModel* Flat ){

	struct timeval tif;
	float TiempoParcial;
	STFrame* frameData;
	tlcde* flies;

#ifdef MEDIR_TIEMPOS
	 gettimeofday(&tif, NULL);
#endif
	printf("\n4)Visualización:\n");


	// Establecer la posición en del bufer para visualizar
	if( pos == PENULTIMO ) pos = frameBuf->numeroDeElementos-2;
	if( pos == ULTIMO ) pos = frameBuf->numeroDeElementos-1;

	if(visParams->VisualPos == -1){
		if(pos>-1)irAl( pos, frameBuf );
		else irAl( 0 , frameBuf );
	}
	else irAl(visParams->VisualPos, frameBuf );

	// OBTENER FRAME
	frameData = (STFrame*)obtenerActual(frameBuf);
	flies = frameData->Flies;

	if ( visParams->ModoCompleto ){

		if(!ImVisual) {
			ImVisual = cvCreateImage( cvGetSize( frameData->Frame ),8,3);
			//CreateWindows( ImVisual );
		}
		cvCopy(frameData->Frame,ImVisual);

		// DIBUJAR PLATO
		if( Flat->PCentroX > 0 ){
			cvCircle( ImVisual, cvPoint( Flat->PCentroX,Flat->PCentroY ), 3, CV_RGB(0,0,0), -1, 8, 0 );
			cvCircle( ImVisual, cvPoint(Flat->PCentroX,Flat->PCentroY ),Flat->PRadio, CV_RGB(0,0,0),2 );
		}

		// DIBUJAR BLOBS Y DIRECCIÓN DE DESPLAZAMIENTO
		dibujarBlobs( ImVisual, frameData->Flies );

		// MOSTRAR datos estadísticos en la ventana de visualización
		if(frameData->Stats) ShowStatDataFr( frameData->Stats,frameData->GStats, ImVisual );

		if ( visParams->ModoCompleto ){
			// MOSTRAMOS IMAGENES
			printf( "\t-Mostrando ventana de visualización...");
			cvShowImage( "Visualización", ImVisual );
			if (visParams->ShowBGremoval){
				printf( "\t-Mostrando Background, Foreground, OldBackground... ");
				cvShowImage("Background", frameData->BGModel);
				cvShowImage( "Foreground",frameData->FG);
				//cvShowImage( "OldForeground",frameData->OldFG);

				printf("Hecho\n");
			}

			if(visParams->ShowKalman){
				printf("\t-Mostrando predicciones del filtro de Kalman...");
				cvShowImage( "Filtro de Kalman", frameData->ImKalman );
				printf("Hecho\n");
			}
			//cvWaitKey(0);

			// OPCIONES
			if(visParams->HightGUIControls){
				// si se pulsa p ó P  => pause = true
				if( (cvWaitKey(5) & 255) == 80 || (cvWaitKey(5) & 255) == 112 ){
					visParams->pause = true;
				}
				// si se pulsa s ó S => stop = true
				if( (cvWaitKey(5) & 255) == 83 || (cvWaitKey(5) & 255) == 115 ){
					visParams->stop = true;
				}
				// si se pulsa v ó V => grab = true
				if( (cvWaitKey(5) & 255) == 'v' || (cvWaitKey(5) & 255) == 'V' ){
					visParams->RecWindow = true;
				}
				// PAUSA
				if(visParams->pause){
					cvShowImage( "Visualización", frameData->Frame );
					if (visParams->ShowBGremoval ){
						cvShowImage("Background", frameData->BGModel);
						cvShowImage( "Foreground",frameData->FG);

					}

					fflush( stdin);
					if( cvWaitKey(0) == 'f' || cvWaitKey(0) == 'F'){
						cvSaveImage( "Captura.jpg", frameData->Frame);
					}
				}
				// si se pulsa c ó C  => pause = false
				if( (cvWaitKey(5) & 255) == 67 || (cvWaitKey(5) & 255) == 99 ){
					visParams->pause = false;
				}
				while(visParams->stop){
					if(visParams->VisualPos == -1) visParams->VisualPos = pos;
					visualizarBuffer( frameBuf,Flat);
					// mientras no se presione c ó C ( continue ) continuamos en el while
					if(!visParams->stop) break;
				}
				irAlFinal( frameBuf );
			}//FIN OPCIONES

		} // FIN VISUALIZAR

	}// FIN VISUALIZAR O GRABAR
#ifdef MEDIR_TIEMPOS
	TiempoParcial = obtenerTiempo( tif , NULL);
	printf("Visualización finalizada.Tiempo total %5.4g ms\n", TiempoParcial);
#endif
}

void VisualizarFr( STFrame* frameData, StaticBGModel* Flat ){

	if (visParams->ModoCompleto ){

		if(!ImVisual) ImVisual = cvCreateImage( cvGetSize( frameData->Frame ),8,3);

		cvCopy(frameData->Frame,ImVisual);

		// DIBUJAR PLATO
		if( Flat->PCentroX > 0 ){
			cvCircle( ImVisual, cvPoint( Flat->PCentroX,Flat->PCentroY ), 3, CV_RGB(0,0,0), -1, 8, 0 );
			cvCircle( ImVisual, cvPoint(Flat->PCentroX,Flat->PCentroY ),Flat->PRadio, CV_RGB(0,0,0),2 );
		}

		// DIBUJAR BLOBS Y DIRECCIÓN DE DESPLAZAMIENTO
		dibujarBlobs( ImVisual, frameData->Flies );

		// MOSTRAR datos estadísticos en la ventana de visualización
		if(frameData->Stats) ShowStatDataFr( frameData->Stats,frameData->GStats, ImVisual );

		// MOSTRAMOS IMAGENES
		printf( "\t-Mostrando ventana de visualización...");
		cvShowImage( "Visualización", ImVisual );
		if (SHOW_BG_REMOVAL == 1){
			printf( "\t-Mostrando Background, Foreground, OldBackground... ");
			cvShowImage("Background", frameData->BGModel);
			cvShowImage( "Foreground",frameData->FG);
			//cvShowImage( "OldForeground",frameData->OldFG);

			printf("Hecho\n");
		}
		if ( SHOW_MOTION_TEMPLATE == 1){
			printf("\t-Mostrando Motion...");
			cvShowImage( "Motion",frameData->ImMotion);
			printf("Hecho\n");
		}

		if(SHOW_KALMAN){
			printf("\t-Mostrando predicciones del filtro de Kalman...");
			cvShowImage( "Filtro de Kalman", frameData->ImKalman );
			printf("Hecho\n");
		}

	}

}

void DraWWindow( IplImage* frame,STFrame* FrameDataOut, StaticBGModel* BGModel, int type, int Mode  ){

	tlcde* flies = NULL;
	STStatFrame* Stats = NULL;
	IplImage* ImRaw = NULL;
	static int first = 1;

	if(type == SHOW_PRESENT ) {
		if( visParams->ShowPresent) DraWPresent(  );
	}
	else {
		switch( Mode ){
				case SIMPLE :
					/// VISUALIZACIÓN MODO SIMPLE
					if( visParams->ShowWindow || visParams->RecWindow ){
						// ESTABLECEMOS LA IMAGEN BASE ( RAW ) SEGÚN EL TIPO DE PROCESO A VISUALIZAR
						if( type == FLAT || type == BG_MODEL){
							ImRaw =BGModel->Imed;
							flies = NULL ;
							Stats = NULL ;
						}
						else if( type == SHAPE  ){
							ImRaw = FrameDataOut->FG;
							flies = NULL ;
							Stats = NULL ;
						}
						else if( type == TRAKING ){
							// si el tipo es TRAKING, hasta que no esté lleno el buffer no hace nada
							if( !FrameDataOut) return;
							ImRaw = FrameDataOut->Frame ;
							flies = FrameDataOut->Flies ;
							Stats = FrameDataOut->Stats ;
						}
						if( ImRaw->nChannels == 1)	cvCvtColor( ImRaw , ImVisual, CV_GRAY2BGR);
						else cvCopy(ImRaw,ImVisual);

						// DIBUJAMOS LAS ANIMACIONES DE INICIO Y FIN DE ETAPAS
						if( type == FLAT && first ){
							Transicion("Iniciando preprocesado...", 1,1000, 50 );
							Transicion2( "Buscando plato...",50); // desaparece la ventana y hace aparecer
							first = 0;
						}
						if( type == BG_MODEL && !first){
							Transicion4("Buscando plato...", 50);
							first = 1;
						}
						if( type == SHAPE && first ){
							 Transicion4("Aprendiendo fondo...", 50);
							 first = 0;
						}
						if( type == TRAKING && !first  ){
								Transicion3( "", 20 );
								first = 1;
						}
						//DIBUJAMOS ELEMENTOS EN VENTANA DE VIDEO
						// DIBUJAR PLATO
						if( BGModel->PRadio > 0 ){

							//cvCircle( ImVisual, cvPoint(BGModel->PCentroX,BGModel->PCentroY ),BGModel->PRadio, CVX_RED, 1, 8, 0);
							cvCircle( ImVisual, cvPoint(cvRound( BGModel->PCentroX),cvRound( BGModel->PCentroY ) ), cvRound( BGModel->PRadio ), CVX_BLUE, 1, 8, 0);
						}
						// DIBUJAR BLOBS Y DIRECCIÓN DE DESPLAZAMIENTO
						if( flies && flies->numeroDeElementos>0 ) dibujarBlobs( ImVisual, flies );

						cvZero( Window);

						//REESCALAR
						// si la imagen es de 320 o menos la escalamos al doble
						if(ImVisual->width <= 320){
							cvPyrUp( ImVisual, ImScale,IPL_GAUSSIAN_5x5);
						}
						else  ImScale = ImVisual;

						//DIBUJAMOS ELEMENTOS EN VENTANA DE VISUALIZACION DEPENDIENDO DEL TIPO
						if( type == FLAT || type == BG_MODEL || type  == SHAPE ){

							DrawPreprocesWindow( frame );

						}
						else if (type == TRAKING){

							DrawTrackingWindow( frame, FrameDataOut,  BGModel);
						}
						IncrustarTxt( type );
						// GUARDAR VISUALIZACION
						if(visParams->RecWindow){
							cvWriteFrame( VWriter,Window);
						}

						if ( visParams->ShowWindow ) cvShowImage("TrackingDrosophila",Window);
						else if(visParams->pasoApaso){
							cvShowImage("TrackingDrosophila",Window);
							cvWaitKey(0);
						}

					}// FIN SHOW WINDOW

					break;
				case COMPLETO:
					/// VISUALIZACIÓN MODO COMPLETO
					if( visParams->ModoCompleto){
						switch( type ){
							case SHOW_LEARNING_FLAT :
								if( visParams->ShowLearningFlat) cvShowImage( "Buscando Plato...",BGModel->Imed);
								break;
							case SHOW_INIT_BACKGROUND:
								if( visParams->ShowInitBackground ) cvShowImage("Aprendiendo fondo...", BGModel->Imed);
								break;
							case SHOW_SHAPE_MODELING:
								if( visParams->ShowShapeModel ){
									cvShowImage("Modelando forma...",frame);
									cvMoveWindow("Modelando forma...", 0, 0 );
									cvShowImage("Foreground", FrameDataOut->FG);
									cvMoveWindow("Foreground", FrameDataOut->FG->width, 0 );
								}
								break;
							case SHOW_PROCESS_IMAGES:
								if( visParams->ShowProcessPhases){
									cvShowImage( "Foreground", FrameDataOut->FG);
									cvWaitKey(0);
								}
								break;
							case SHOW_BG_DIF_IMAGES:
								if( visParams->ShowBGdiffImages){
									cvShowImage( "Foreground", FrameDataOut->FG);
									cvShowImage( "Background",FrameDataOut->BGModel);
									cvWaitKey(0);
								}
								break;

							case SHOW_VALIDATION_IMAGES :
								if( visParams->ShowValidationPhases) {
									cvShowImage( "Foreground",FrameDataOut->FG);
									cvWaitKey(0);
								}
								break;

						}
					}
		}
	}

}

int obtenerVisParam( int type ){
	if ( type == SHOW_KALMAN ) return visParams->ShowKalman;
	if ( type == MODE ) return visParams->ModoCompleto;
}

void DraWPresent(  ){

	CvFont fuente1;
	CvFont fuente2;

		//CreateWindows( ImVisual );
	IncrustarLogo("logos.jpg", Window , CENTRAR ,visParams->DelayLogo,false);
	desvanecer( Window, 10 );
	IncrustarLogo("logo-opencv.jpg", Window , CENTRAR ,visParams->DelayLogo,false);
	desvanecer( Window, 10 );
	IncrustarLogo("Logo_IBGM.png", Window , CENTRAR_INF,visParams->DelayLogo,false);
	desvanecer( Window, 10 );
	IncrustarLogo("LogoUVAnegro.jpg", Window , CENTRAR,visParams->DelayLogo,false);
	cvWaitKey(1000);
	desvanecer( Window, 10 );
	cvInitFont( &fuente1, CV_FONT_HERSHEY_COMPLEX, 2, 2, 0, 3, CV_AA);
//	cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 0.5, 0.5, 0, 1, CV_AA);
	cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
	CvSize textsize = getTextSize("Tracking Drosophila", CV_FONT_HERSHEY_COMPLEX, 2, 5, 0);
	CvSize textsize2 = getTextSize("Presione una tecla para comenzar", CV_FONT_HERSHEY_PLAIN, 1, 1, 0);

	for( int i = 0; i < 255; i += 2 )
	{
		cvPutText( Window, "Tracking Drosophila",
				cvPoint( (Window->width-textsize.width)/2,( Window->height-textsize.height)/2),
				&fuente1,
				cvScalar(i,i,i) );
		cvShowImage("TrackingDrosophila", Window);
		cvWaitKey(5);
	}
	int comenzar = 0;
	for( int i = 0; i < 255; i += 5 )
	{
		cvPutText( Window, "Presione una tecla para comenzar...",
				cvPoint( (Window->width-textsize2.width)/2,( Window->height-textsize2.height)/2+ 50),
				&fuente2,
				cvScalar(i,i,i) );
		cvShowImage("TrackingDrosophila", Window);
		if(cvWaitKey(5) >= 0){
			comenzar = 1;
			break;
		}
		if(i == 250 ){
			for( i = 250 ; i > 0; i -= 5 )
			{
				cvPutText( Window, "Presione una tecla para comenzar...",
				cvPoint( (Window->width-textsize2.width)/2,( Window->height-textsize2.height)/2+ 50),
				&fuente2,
				cvScalar(i,i,i) );
				cvShowImage("TrackingDrosophila", Window);
				if(cvWaitKey(2) >= 0){
					comenzar = 1;
					break;
				}
			}
			i = 0;
		}
		if(comenzar) break;
	}


//	IncrustarLogo("logo_subversion_.png", ImLogos, cvPoint( 0,0 ),visParams->DelayLogo,true);

//	CvSize textsize = getTextSize("OpenCV forever!", CV_FONT_HERSHEY_COMPLEX, 3, 5, 0);
//	CvPoint org((width - textsize.width)/2, (height - textsize.height)/2);
//	Mat image2;
//	for( i = 0; i < 255; i += 2 )
//	{
//		image2 = Window  - Scalar::all(i);
//		putText(image2, "Tracking Drosophila", org, CV_FONT_HERSHEY_COMPLEX, 3,
//				Scalar(i, i, i), 5, lineType);
//
//		imshow("TrackingDrosophila", image2);
//		if(waitKey(5) >= 0)
//			return 0;
//	}

}

void DrawPreprocesWindow( IplImage* frame){
	// incrustamos el video en pequeño en la parte superior
	IplImage* video;

	CvSize size = cvSize(320,240 );
	if(frame->width == 640) size = cvSize(frame->width/2,frame->height/2 );
	if(frame->width == 1280) size = cvSize(frame->width/4,frame->height/4 );
	if(frame->width == 1280) size = cvSize(frame->width,frame->height);
	video = cvCreateImage( size,8,3);
	CvRect rect = cvRect( (Window->width-video->width)/2,
							( Window->height-video->height)/8,
							video->width,
							video->height);
	// Reescalar video
	if(frame->width == 640){
		 cvPyrDown( frame, video,IPL_GAUSSIAN_5x5);
	}
	else if(frame->width == 1280){
		IplImage* video1;
		CvSize size = cvSize(640,480 );
		video1 = cvCreateImage( cvSize(frame->width/2,frame->height/2 ),8,3);
		cvPyrDown( frame, video1,IPL_GAUSSIAN_5x5);
		cvPyrDown( video1, video,IPL_GAUSSIAN_5x5);
		cvReleaseImage(&video1);
	}
	else cvCopy( frame, video );

	// incrustamos las imagenes del preprocesado
	Incrustar( Window, ImScale, Window, visParams->ROIPreProces);
	Incrustar( Window, video, Window, rect);
	// dibujamos sendos rectangulos blancos entorno a cada imagen incrustada
	cvRectangle( Window,
												cvPoint(rect.x,rect.y),
												cvPoint(rect.x + rect.width,
														rect.y +rect.height),
												CVX_WHITE, 1 );
	cvRectangle( Window,
			cvPoint(visParams->ROIPreProces.x,visParams->ROIPreProces.y),
			cvPoint(visParams->ROIPreProces.x + visParams->ROIPreProces.width,
					visParams->ROIPreProces.y +visParams->ROIPreProces.height),
			CVX_WHITE, 1 );

	cvReleaseImage(&video);
}

void DrawTrackingWindow( IplImage* frame, STFrame* FrameDataOut, StaticBGModel* BGModel ){

	DibujarFondo( );
	Incrustar( Window, ImScale, Window, visParams->ROITracking);
	// MOSTRAR datos estadísticos en la ventana de visualización
	// frame
	if(FrameDataOut->Stats) ShowStatDataFr(FrameDataOut->Stats,FrameDataOut->GStats, Window);
	// blobs
	ShowStatDataBlobs( FrameDataOut->Flies , FrameDataOut->Tracks);
	// Barra de progreso
	float contadorX = 0;
	float x;
	if( FrameDataOut->GStats->totalFrames > 0){
		contadorX = contadorX + (visParams->BPrWidth*FrameDataOut->GStats->numFrame/ FrameDataOut->GStats->totalFrames);
		x =  contadorX ;
		cvRectangle( Window, cvPoint(  (Window->width-visParams->BPrWidth )/2,( Window->height - 30 - Window->height/64) ),
							cvPoint( (Window->width-visParams->BPrWidth )/2 + cvRound(x), (Window->height - 30 + Window->height/64) ), CVX_BLUE, -1 );
	}
	if(ACTIVAR_OPCIONES_VISUALIZACION){
		// si se pulsa p ó P  => pause = true
		if( (cvWaitKey(1) & 255) == 'p' || (cvWaitKey(5) & 255) == 'P' ){
			visParams->pause = true;
			fflush( stdin);
		}
		if( (cvWaitKey(1) & 255) == 'r' || (cvWaitKey(5) & 255) == 'R' ){
			visParams->RecWindow = true;
			fflush( stdin);
		}
		if( (cvWaitKey(1) & 255) == 's' || (cvWaitKey(5) & 255) == 'S' ){
			visParams->RecWindow = false;
			fflush( stdin);
		}
		if( (cvWaitKey(1) & 255) == 'f' || (cvWaitKey(5) & 255) == 'F' ){
			visParams->pasoApaso = true;
			fflush( stdin);
		}
		if( (cvWaitKey(1) & 255) == 'c' || (cvWaitKey(5) & 255) == 'C')	{
			visParams->pasoApaso = false;
			fflush( stdin);
		}
	}
	while(visParams->pause){
		DibujarFondo( );
		Incrustar( Window, ImScale, Window, visParams->ROITracking);
		// MOSTRAR datos estadísticos en la ventana de visualización
		// frame
		if(FrameDataOut->Stats) ShowStatDataFr(FrameDataOut->Stats,FrameDataOut->GStats, Window);
		// blobs
		ShowStatDataBlobs( FrameDataOut->Flies , FrameDataOut->Tracks);
		// si se pulsa c ó C  => pause = false
		if( (cvWaitKey(5) & 255) == 'c' || (cvWaitKey(5) & 255) == 'C'){
			visParams->pause = false;
			fflush( stdin);
		}
		if( cvWaitKey(5) == 'g' || cvWaitKey(5) == 'G'){
			cvSaveImage( "Captura1.jpg", Window );
			fflush( stdin);
		}
	}
}

void dibujarBlobs( IplImage* Imagen,tlcde* flies ){

	STFly* fly;

	if(flies!= NULL && flies->numeroDeElementos>0) {
		for( int i = 0; i <  flies->numeroDeElementos; i++ ){
			// obtener lista de blobs.
			fly = (STFly*)obtener(i, flies);
			// Dibujar un trianguno isósceles que representa el blob
			CvPoint A   = cvPoint( cvRound( fly->posicion.x + fly->a*cos(fly->orientacion*CV_PI/180) ),
								   cvRound( fly->posicion.y - fly->a*sin(fly->orientacion*CV_PI/180)));
			CvPoint Mcb = cvPoint( cvRound( fly->posicion.x - fly->a*cos(fly->orientacion*CV_PI/180)),
								   cvRound( fly->posicion.y + fly->a*sin(fly->orientacion*CV_PI/180)));
			CvPoint B =   cvPoint( cvRound( Mcb.x + fly->b*cos( (fly->orientacion+90)*CV_PI/180) ),
								   cvRound( Mcb.y - fly->b*sin( (fly->orientacion+90)*CV_PI/180)));
			CvPoint C =   cvPoint( cvRound( Mcb.x + fly->b*cos( (fly->orientacion-90)*CV_PI/180) ),
								   cvRound( Mcb.y - fly->b*sin( (fly->orientacion-90)*CV_PI/180)));
//			if( fly->Estado == 1){
				cvLine( Imagen,A,B,fly->Color,1,CV_AA, 0 );
				cvLine( Imagen,B,C,fly->Color,1,CV_AA, 0 );
				cvLine( Imagen,C,A,fly->Color,1,CV_AA, 0 );
//			}
//			else{
//				cvLine( Imagen,A,B,CVX_WHITE,1,CV_AA, 0 );
//				cvLine( Imagen,B,C,CVX_WHITE,1,CV_AA, 0 );
//				cvLine( Imagen,C,A,CVX_WHITE,1,CV_AA, 0 );
//			}
			muestrearPosicion( flies, 4 );
//			Mat img(Imagen);
//			rectangle(img,
//					Point(fly->Roi.x,fly->Roi.y),
//					Point(fly->Roi.x+fly->Roi.width,fly->Roi.y+fly->Roi.height),
//					Scalar(255), 1,8,0);
//			imshow("CamShift Demo",img);
			// visualizar direccion
			//double op_angle = 360.0 - fly->direccion;  // adjust for images with top-left origin
			if(fly->etiqueta == 11)	cvCircle( Imagen, fly->posicion, 10, fly->Color, 3, CV_AA, 0 );

			double magnitude = 30;
			cvLine( Imagen,
					fly->posicion,
					cvPoint( cvRound( fly->posicion.x + magnitude*cos(fly->direccion*CV_PI/180)),
							 cvRound( fly->posicion.y - magnitude*sin(fly->direccion*CV_PI/180))  ),
					CVX_RED,
					1, CV_AA, 0 );
			// dirección de kalman
			if(fly->Estado != 2){
				cvLine( Imagen,
									fly->posicion,
									cvPoint( cvRound( fly->posicion.x + magnitude*cos(fly->dir_filtered*CV_PI/180)),
											 cvRound( fly->posicion.y - magnitude*sin(fly->dir_filtered*CV_PI/180))  ),
									CVX_GREEN,
									1, CV_AA, 0 );
			}else{
				cvLine( Imagen,
						fly->posicion,
						cvPoint( cvRound( fly->posicion.x + magnitude*cos(fly->dir_filtered*CV_PI/180)),
								 cvRound( fly->posicion.y - magnitude*sin(fly->dir_filtered*CV_PI/180))  ),
						CVX_BLUE,
						1, CV_AA, 0 );
			}
			visualizarId( Imagen,fly->posicion, fly->etiqueta, fly->Color);
		}
	}
	else return;
}



void ShowStatDataFr( STStatFrame* Stats,STGlobStatF* GStats,IplImage* Window ){

	CvFont fuente1;
	CvFont fuente2;

	// margenes y dimensiones( en pixels)
	unsigned int margenIz = 10; // margen izquierdo
	unsigned int margenSup = visParams->ROITracking.y + visParams->ROITracking.height/2+5;
	unsigned int anchoCol = 150; // margen entre columnas
	unsigned int margenFil = 5; // margen entre filas
	unsigned int linea = 15;
	unsigned int margenTxt = 10;
	unsigned int margenTxtSup = 10;

	char NFrame[100];
	char TProcesF[100];
	char TProces[100];
	char PComplet[100];
	char FPS[100];

	char BlobsUp[50];
	char BlobsDown[50];
	char TotalBlobs[50];

	char CMov1SMed[50];
	char CMov1SDes[50];
	char CMov30SMed[50];
	char CMov30SDes[50];
	char CMov1Med[50];
	char CMov1Des[50];
	char CMov5Med[50];
	char CMov5Des[50];
	char CMov10Med[50];
	char CMov10Des[50];
	char CMov15Med[50];
	char CMov15Des[50];
	char CMov30Med[50];
	char CMov30Des[50];
	char CMov1HMed[50];
	char CMov1HDes[50];
	char CMov2HMed[50];
	char CMov2HDes[50];
	char CMov4HMed[50];
	char CMov4HDes[50];
	char CMov8HMed[50];
	char CMov8HDes[50];
	char CMov16HMed[50];
	char CMov16HDes[50];
	char CMov24HMed[50];
	char CMov24HDes[50];
	char CMov48HMed[50];
	char CMov48HDes[50];

	/// ESTADISTICAS FRAME
	sprintf(NFrame,"Frame %d ",GStats->numFrame);
	sprintf(TProcesF,"Tiempo de procesado del Frame: %5.4g ms",GStats->TiempoFrame);
	sprintf(TProces,"Segundos de video procesados: %0.f seg ", GStats->TiempoGlobal);
	sprintf(PComplet,"Porcentaje completado: %.2f %% ",(float)(GStats->numFrame/GStats->totalFrames)*100 );
	sprintf(FPS,"FPS: %.2f ",(1000/GStats->TiempoFrame));

	sprintf(TotalBlobs,"Total Blobs detected: %d ", Stats->TotalBlobs);
	sprintf(BlobsUp,"Blobs activos: %0.1f %% ", Stats->dinamicBlobs);
	sprintf(BlobsDown, "Blobs inactivos: %0.1f %%",Stats->staticBlobs);

	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1.1, 1.1, 0, 1, 8);
	cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 0.9, 0.9, 0, 1, 8);

	CvSize textsize = getTextSize(TProcesF, CV_FONT_HERSHEY_PLAIN, 1.1, 1, 0);

	// estadísticas Frame
	cvPutText( Window, NFrame, cvPoint( 11,32), &fuente1, CVX_BLUE );
	cvPutText( Window, TProcesF, cvPoint( 11,52), &fuente2, CVX_WHITE );
	cvPutText( Window, TProces, cvPoint( 11,72), &fuente2, CVX_WHITE);
	cvPutText( Window, PComplet, cvPoint( 11,92), &fuente2, CVX_WHITE);
	cvPutText( Window, FPS, cvPoint( 11,112), &fuente1, CVX_RED);

	cvPutText( Window, TotalBlobs, cvPoint( 11,132), &fuente1, CVX_WHITE );
	cvPutText( Window, BlobsUp, cvPoint( 11,152), &fuente1, CVX_WHITE );
	cvPutText( Window, BlobsDown, cvPoint( 11,172), &fuente1, CVX_WHITE);

	/// ESTADÍSTICAS MOVIMIENTO BLOBS
	if( CALC_STATS_MOV ){
		sprintf( CMov1SMed,"MovMed_1s:  %0.1f ",Stats->CMov1SMed);
		sprintf( CMov1SDes,"MovDes_1s:  %0.1f ",Stats->CMov1SDes);
		cvPutText( Window,  CMov1SMed, cvPoint( margenIz + margenTxt ,margenSup + linea), &fuente2, CVX_BLUE );
		cvPutText( Window,  CMov1SDes, cvPoint( margenIz + margenTxt ,margenSup + 2*linea), &fuente2, CVX_BLUE );

		sprintf( CMov30SMed,"MovMed_30s:  %0.1f ",Stats->CMov30SMed);
		sprintf( CMov30SDes,"MovDes_30s:  %0.1f ",Stats->CMov30SDes);
		cvPutText( Window,  CMov30SMed, cvPoint( margenIz + margenTxt ,margenSup + 3*linea), &fuente2, CVX_BLACK );
		cvPutText( Window,  CMov30SDes, cvPoint( margenIz + margenTxt ,margenSup + 4*linea), &fuente2, CVX_BLACK );

		sprintf( CMov1Med,"MovMed_1m:  %0.1f ",Stats->CMov1Med);
		sprintf( CMov1Des,"MovDes_1m:  %0.1f ",Stats->CMov1Des);
		cvPutText( Window,  CMov1Med, cvPoint( margenIz + margenTxt ,margenSup + 5*linea), &fuente2, CVX_BLUE );
		cvPutText( Window,  CMov1Des, cvPoint( margenIz + margenTxt ,margenSup + 6*linea), &fuente2, CVX_BLUE );

		sprintf( CMov5Med,"MovMed_5m:  %0.1f ",Stats->CMov5Med);
		sprintf( CMov5Des,"MovDes_5m:  %0.1f ",Stats->CMov5Des);
		cvPutText( Window,  CMov5Med, cvPoint( margenIz + margenTxt ,margenSup + 7*linea), &fuente2, CVX_BLACK );
		cvPutText( Window,  CMov5Des, cvPoint( margenIz + margenTxt ,margenSup + 8*linea), &fuente2, CVX_BLACK );

		sprintf( CMov10Med,"MovMed_10m: %0.1f ",Stats->CMov10Med);
		sprintf( CMov10Des,"MovDes_10m: %0.1f ",Stats->CMov10Des);
		cvPutText( Window,  CMov10Med, cvPoint( margenIz + margenTxt ,margenSup + 9*linea), &fuente2, CVX_BLUE );
		cvPutText( Window,  CMov10Des, cvPoint( margenIz + margenTxt ,margenSup + 10*linea), &fuente2, CVX_BLUE );

		sprintf( CMov15Med,"MovMed_15m: %0.1f ",Stats->CMov15Med);
		sprintf( CMov15Des,"MovDes_15m: %0.1f ",Stats->CMov15Des);
		cvPutText( Window,  CMov15Med, cvPoint( margenIz + margenTxt + anchoCol ,margenSup + linea), &fuente2, CVX_BLACK  );
		cvPutText( Window,  CMov15Des, cvPoint( margenIz + margenTxt + anchoCol,margenSup + 2*linea), &fuente2, CVX_BLACK  );

		sprintf( CMov30Med,"MovMed_30m: %0.1f ",Stats->CMov30Med);
		sprintf( CMov30Des,"MovDes_30m: %0.1f ",Stats->CMov30Des);
		cvPutText( Window,  CMov30Med, cvPoint( margenIz + margenTxt + anchoCol ,margenSup + 3*linea), &fuente2, CVX_BLUE  );
		cvPutText( Window,  CMov30Des, cvPoint( margenIz + margenTxt + anchoCol,margenSup + 4*linea), &fuente2, CVX_BLUE  );

		sprintf( CMov1HMed,"MovMed_1h:  %0.1f ",Stats->CMov1HMed);
		sprintf( CMov1HDes,"MovDes_1h:  %0.1f ",Stats->CMov1HDes);
		cvPutText( Window,  CMov1HMed, cvPoint( margenIz + margenTxt + anchoCol ,margenSup + 5*linea), &fuente2, CVX_BLACK  );
		cvPutText( Window,  CMov1HDes, cvPoint( margenIz + margenTxt + anchoCol,margenSup + 6*linea), &fuente2, CVX_BLACK  );

		sprintf( CMov2HMed,"MovMed_2h:  %0.1f ",Stats->CMov2HMed);
		sprintf( CMov2HDes,"MovDes_2h:  %0.1f ",Stats->CMov2HDes);
		cvPutText( Window, CMov2HMed, cvPoint( margenIz + margenTxt + anchoCol ,margenSup + 7*linea), &fuente2, CVX_BLUE  );
		cvPutText( Window,  CMov2HDes, cvPoint( margenIz + margenTxt + anchoCol,margenSup + 8*linea), &fuente2, CVX_BLUE   );

		sprintf( CMov4HMed,"MovMed_4h:  %0.1f ",Stats->CMov4HMed);
		sprintf( CMov4HDes,"MovDes_4h:  %0.1f ",Stats->CMov4HDes);
		cvPutText( Window, CMov4HMed, cvPoint( margenIz + margenTxt + anchoCol ,margenSup + 9*linea), &fuente2, CVX_BLACK  );
		cvPutText( Window,  CMov4HDes, cvPoint( margenIz + margenTxt + anchoCol,margenSup + 10*linea), &fuente2, CVX_BLACK   );

		sprintf( CMov8HMed,"MovMed_8h:  %0.1f ",Stats->CMov8HMed);
		sprintf( CMov8HDes,"MovDes_8h:  %0.1f ",Stats->CMov8HDes);
		cvPutText( Window, CMov8HMed, cvPoint( margenIz + margenTxt + 2*anchoCol ,margenSup + linea), &fuente2, CVX_BLUE  );
		cvPutText( Window, CMov8HDes, cvPoint( margenIz + margenTxt + 2*anchoCol,margenSup + 2*linea), &fuente2, CVX_BLUE   );

		sprintf( CMov16HMed,"MovMed_16h: %0.1f ",Stats->CMov16HMed);
		sprintf( CMov16HDes,"MovDes_16h: %0.1f ",Stats->CMov16HDes);
		cvPutText( Window, CMov16HMed, cvPoint( margenIz + margenTxt + 2*anchoCol ,margenSup + 3*linea), &fuente2, CVX_BLACK  );
		cvPutText( Window, CMov16HDes, cvPoint( margenIz + margenTxt + 2*anchoCol,margenSup + 4*linea), &fuente2, CVX_BLACK   );

		sprintf( CMov24HMed,"MovMed_24h: %0.1f ",Stats->CMov4HMed);
		sprintf( CMov24HDes,"MovDes_24h: %0.1f ",Stats->CMov4HDes);
		cvPutText( Window, CMov24HMed, cvPoint( margenIz + margenTxt + 2*anchoCol ,margenSup + 5*linea), &fuente2, CVX_BLUE  );
		cvPutText( Window, CMov24HDes, cvPoint( margenIz + margenTxt + 2*anchoCol,margenSup + 6*linea), &fuente2, CVX_BLUE   );

		sprintf( CMov48HMed,"MovMed_48h: %0.1f ",Stats->CMov48HMed);
		sprintf( CMov48HDes,"MovDes_48h: %0.1f ",Stats->CMov48HDes);
		cvPutText( Window, CMov8HMed, cvPoint( margenIz + margenTxt + 2*anchoCol ,margenSup + 7*linea), &fuente2, CVX_BLACK  );
		cvPutText( Window, CMov8HDes, cvPoint( margenIz + margenTxt + 2*anchoCol,margenSup + 8*linea), &fuente2, CVX_BLACK   );
	}
	else{
		sprintf( CMov1SMed,"NO STATS ",Stats->CMov1SMed);
		CvSize textsize = getTextSize(CMov1SMed, CV_FONT_HERSHEY_PLAIN, 1.1, 1, 0);
		cvPutText( Window,  CMov1SMed, cvPoint( margenIz + margenTxt ,margenSup + linea), &fuente2, CVX_BLUE );

	}
	// estadísticas Frame

}

void ShowStatDataBlobs( tlcde* Flies, tlcde* Tracks ){

	CvFont fuente1;
	CvFont fuente2;

	CvPoint Origen1; // origen para la fila 1
	CvPoint Origen2; // origen para la fila 2

	CvPoint Fin;

	STTrack* Track = NULL;
	STFly* Fly = NULL;

	// margenes y dimensiones( en pixels)
	unsigned int margenIz = 20; // margen izquierdo
	unsigned int margenSup = visParams->ROITracking.y + visParams->ROITracking.height+ 15;
	unsigned int margenCol = 5; // margen entre columnas
	unsigned int margenFil = 5; // margen entre filas
	unsigned int alto = 120; // alto rectangulo blob
	unsigned int ancho= 150; // ancho rectangulo blob
	unsigned int linea = 15;
	unsigned int margenTxt = 10;
	unsigned int margenTxtSup = 10;

	char BlobId[50];
	char EstadoTrack[50];
	char T_estadoT[50];
	char EstadoBlob[50];
	char T_estadoB[50];
	char dstTotal[50];
	char Direccion[50];
	char velocidad[50];
	char vMedia[50];

	char FPS[100];

	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1.1, 1.1, 0, 1, 8);
	cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 0.9, 0.9, 0, 1, 8);


	Origen1 = cvPoint(  margenIz, margenSup);
	Origen2 = cvPoint(  margenIz, margenSup + alto + margenCol );
	Fin = cvPoint(Window->width - 10, Window->height - 65);
//for( int i = 0; i < MAX_FILS; i++){
		// Recorremos todas las posiciones de la matriz ( considerado como un vector de numero de elementos = MAX_ELEMENTS
		for( int i = 0; i <  MAX_ELEMENTS ; i++ ){
			// establecer origen
			if( i < MAX_COLS ){ // primera fila
				Origen1 =  cvPoint(  margenIz + i* ( ancho + margenCol), margenSup ); // desplazarse por columnas
			}
			else if( i >= MAX_COLS && i < MAX_ELEMENTS ){ // siguiente fila
				Origen1 = cvPoint(  margenIz + (i-MAX_COLS)*( ancho + margenCol), margenSup + alto + margenFil); //desplazarse por columnas
			}
			else break;
			// obtener track de esa posición
			for( int j = 0; j < Tracks->numeroDeElementos; j++){
				Track = (STTrack*)obtener(j, Tracks);
				// si se ha encontrado el track de esa posición
				if(Track->id == i + 1) break;
				else Track = NULL;
			}
			if( Track ){
				// dibujar recuadro
		//		cvRectangle( Window, cvPoint(Origen1.x , Origen1.y ),cvPoint( Origen1.x + ancho, Origen1.y + alto )	,cvScalar(0,0,0), -1 );

				cvRectangle( Window, cvPoint(Origen1.x , Origen1.y + margenTxtSup + linea ),cvPoint( Origen1.x + ancho, Origen1.y + alto )	,Track->Color, -1 );
				// obtener fly con la id del track.
				for(int k = 0; k < Flies->numeroDeElementos; k++){
					Fly =  (STFly*)obtener(k, Flies);
					if( Fly->etiqueta == Track->id) break;
					else Fly = NULL;
				}
				// si se ha encontrado dibujar sus datos
				if( Fly ){
					// estadísticas Frame
					sprintf(BlobId,"Blob %d ",Fly->etiqueta);

					sprintf(EstadoTrack,"Track: Activo ");
					sprintf(T_estadoT,"Tiempo: %d", Fly->Stats->EstadoTrackCount);

					if( Fly->Estado == 1) sprintf(EstadoBlob,"Fly: Activa ");
					else if( Fly->Estado == 0) sprintf(EstadoBlob,"Fly: Pasiva ");
					else if( Fly->Estado == 2) sprintf(EstadoBlob,"Fly: Oclusión ");
					sprintf(T_estadoB,"Tiempo: %d", Fly->Stats->EstadoBlobCount);

					sprintf(dstTotal,"Distancia: %0.1f ",Fly->dstTotal);
					sprintf(Direccion,"Phi: %0.1f ",Fly->dir_filtered);
					sprintf(velocidad,"Velocidad: %0.1f ", Fly->Stats->CMov1SMed );

					CvSize textsize = getTextSize(BlobId, CV_FONT_HERSHEY_PLAIN, 1.1, 1, 0);
			//		cvPutText( Window, PComplet,  cvPoint(( (ancho-textsize2.width)/2, Window->height/2),
			//						&fuente2, CVX_WHITE );
					cvPutText( Window, BlobId, cvPoint( Origen1.x + (ancho-textsize.width)/2 , Origen1.y  + linea), &fuente1, CVX_WHITE );
					cvPutText( Window, EstadoTrack, cvPoint( Origen1.x + margenTxt , Origen1.y + margenTxtSup + 2*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, T_estadoT, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 3*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, EstadoBlob, cvPoint( Origen1.x+ margenTxt, Origen1.y + margenTxtSup + 4*linea), &fuente2,CVX_WHITE );
					cvPutText( Window, T_estadoB, cvPoint( Origen1.x+ margenTxt, Origen1.y + margenTxtSup + 5*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, Direccion, cvPoint( Origen1.x+ margenTxt, Origen1.y + margenTxtSup + 6*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, velocidad, cvPoint( Origen1.x+ margenTxt, Origen1.y + margenTxtSup + 7*linea), &fuente2, CVX_WHITE );
				}
				else{ // si no se ha encontrado
					sprintf(BlobId,"Blob %d ",Track->id);

					sprintf(EstadoTrack,"Track: Durmiendo ");
					sprintf(T_estadoT,"Tiempo: %d", Track->Stats->EstadoCount);

					sprintf(EstadoBlob,"Fly: Perdida ");
					sprintf(T_estadoB,"Tiempo: -----");

					sprintf(dstTotal,"Distancia: %0.1f ",Track->Stats->dstTotal);
					sprintf(Direccion,"Dirección: ----- ");
					sprintf(velocidad,"Velocidad: ----- ");

					CvSize textsize = getTextSize(BlobId, CV_FONT_HERSHEY_PLAIN, 1.1, 1, 0);
			//		cvPutText( Window, PComplet,  cvPoint((Window->width-textsize2.width)/2, Window->height/2),
			//						&fuente2, CVX_WHITE );
					cvPutText( Window, BlobId,cvPoint( Origen1.x + (ancho-textsize.width)/2 , Origen1.y + linea ), &fuente1, CVX_WHITE );
					cvPutText( Window, EstadoTrack, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 2*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, T_estadoT, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 3*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, EstadoBlob, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 4*linea), &fuente2,CVX_WHITE );
					cvPutText( Window, T_estadoB, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 5*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, Direccion, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 6*linea), &fuente2, CVX_WHITE );
					cvPutText( Window, velocidad, cvPoint( Origen1.x + margenTxt, Origen1.y + margenTxtSup + 7*linea), &fuente2, CVX_WHITE );
				}

			}
			else{
				// si esa posición no tiene track, dibujar rectangulo negro sin rellenar y escribir NO TRACK y pasar a la siguiente posición
				// dibujar recuadro
				cvRectangle( Window, cvPoint(Origen1.x , Origen1.y + margenTxtSup + linea ),cvPoint( Origen1.x + ancho, Origen1.y + alto )	,cvScalar(255,255,255), 1 );
				sprintf(BlobId,"NO TRACK ");
				CvSize textsize = getTextSize(BlobId, CV_FONT_HERSHEY_PLAIN, 1.1, 1, 0);
				cvPutText( Window, BlobId, cvPoint( Origen1.x + (ancho-textsize.width)/2, Origen1.y  + linea), &fuente1, CVX_WHITE );
			}
		}
}
// Genera una imagen que representa el llenado del buffer
void VerEstadoBuffer( IplImage* Imagen,int num,int max ){
	static int count = 0 ;
	static float anchoBuf; // longitud en pixels del ancho del buffer
	char PComplet[100];
	CvFont fuente1;
	CvFont fuente2;
	float porcentaje;

	if (SHOW_WINDOW || SHOW_VISUALIZATION || GRABAR_VISUALIZACION ){

		anchoBuf = Window->width/2;
		cvZero( Window);
		// rectángulo roi video
//		cvRectangle( Window, cvPoint(params->ROITracking.x,params->ROITracking.y),cvPoint(params->ROITracking.x + params->ROITracking.width,params->ROITracking.y + params->ROITracking.height), CVX_WHITE, 1 );
		// rectángulo exterior
		cvRectangle( Window, cvPoint(  (Window->width-anchoBuf)/2-2,( Window->height/2 + 60 - Window->height/64)-2 ),
						cvPoint((Window->width + anchoBuf)/2+2, (Window->height/2 + 60 + Window->height/64)+2 ), CVX_WHITE, 1 );
		// rectángulo interior
		count = count + Window->width/2/max;
		porcentaje = (count/anchoBuf)*100;
		if( num == max-1 ){
			count = Window->width/2;
			porcentaje = 100;
		}
		cvRectangle( Window, cvPoint( (Window->width-anchoBuf)/2,Window->height/2 + 60 - Window->height/64 ),
		cvPoint( (Window->width-anchoBuf)/2 + count, Window->height/2 + 60 + Window->height/64) , CVX_GREEN, -1 );
		// rectangulo imagen
//		cvRectangle( Window, cvPoint(params->ROIPreProces.x,params->ROIPreProces.y),cvPoint(params->ROIPreProces.x + params->ROIPreProces.width,params->ROIPreProces.y + params->ROIPreProces.height), CVX_WHITE, 1 );
//		contadorX = contadorX + (visParams->BPrWidth*Stats->numFrame/ Stats->totalFrames);
//				x =  contadorX ;

		sprintf(PComplet," %.0f %% ",porcentaje );
	//	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);

		cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
		CvSize textsize1;
		CvSize textsize2;
		textsize1 = getTextSize("Llenando Buffer...", CV_FONT_HERSHEY_PLAIN, 2, 2, 0);
		textsize2 = getTextSize(PComplet, CV_FONT_HERSHEY_PLAIN, 2, 2, 0);

		//cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
		cvPutText( Window, PComplet,  cvPoint((Window->width-textsize2.width)/2, Window->height/2),
				&fuente2, CVX_WHITE );
		cvPutText( Window, "Llenando Buffer...",
				cvPoint( (Window->width-textsize1.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30),
				&fuente2, CVX_WHITE );


		if( num == IMAGE_BUFFER_LENGTH - 1){
//			if(SHOW_WINDOW) Transicion3( "", 20 );
		}

		cvShowImage( "TrackingDrosophila", Window );
	}
//	cvWaitKey(0);
}



//Representamos los blobs mediante triangulos isosceles
// dibujamos triangulo isosceles de altura el eje mayor de la elipse, formando el segmento
// (A,mcb), y de anchura el eje menor dando lugar al segmento (B,C), perpendicular
// a (A,mcb) cuyo centro es mcb. La unión de A,B,C dará el triangulo resultante.


void visualizarBuffer( tlcde* Buffer,StaticBGModel* Flat  ){

	STFrame* frameData;

	unsigned char opcion = 0;

//	visParams->RecWindow = false;
	fflush( stdin);


	irAl( visParams->VisualPos,Buffer );
	frameData = (STFrame*)obtenerActual(Buffer);
//	cvCopy(frameData->Frame,ImVisual);


	VisualizarFr( frameData, Flat );
	CvFont fuente1;
	char PBuf[100];
	sprintf(PBuf," Posicion del Buffer: %d ",visParams->VisualPos );
	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
	cvPutText( ImVisual, PBuf,  cvPoint( 10,frameData->Frame->height-10), &fuente1, CVX_RED );
	cvShowImage( "Visualización", ImVisual );
	opcion = cvWaitKey(0);
	if( opcion == 'C' || opcion == 'c' ){
			visParams->stop = false;
	}
	// si pulsamos +, visualizamos el siguiente elemento del buffer
	if( opcion == 43 ){
		if (visParams->VisualPos < Buffer->numeroDeElementos-1) visParams->VisualPos=visParams->VisualPos+1;
	}
	// si pulsamos -, visualizamos el elemento anterior del buffer
	if( opcion == 45 ){
		if ( visParams->VisualPos > 0 ) visParams->VisualPos=visParams->VisualPos-1;
	}
	// si pulsamos i ó I, visualizamos el primer elemento del buffer
	if( opcion == 49 || opcion == 69)  {
		visParams->VisualPos = 0;
	}
	// si pulsamos f +o F, visualizamos el último elemento del buffer
	if( opcion == 70 || opcion == 102)  {
		visParams->VisualPos = Buffer->numeroDeElementos-1;
	}
	// tomar instantanea del frame
	if( opcion == 'g' || opcion == 'G'){
		irAl( visParams->VisualPos, Buffer );
		frameData = (STFrame*)obtenerActual(Buffer);
		cvSaveImage( "Captura.jpg", frameData->Frame);
	}
}

void DibujarFondo( ){

	CvFont fuente1;

	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
	cvPutText( Window, "Progreso:",
							cvPoint( (Window->width-visParams->BPrWidth )/2-2, Window->height - 50 ),
							&fuente1,
							CVX_WHITE );

	cvSet(Window,cvScalar(255,255,255) );
	// imagen
	cvRectangle( Window, cvPoint(visParams->ROITracking.x-1,visParams->ROITracking.y-1),
			cvPoint(visParams->ROITracking.x + visParams->ROITracking.width+1,visParams->ROITracking.y + visParams->ROITracking.height+1), cvScalar(0, 0, 0), -1 );

	//estadisticas
	cvRectangle( Window, cvPoint(  10,10 ),
					cvPoint( visParams->ROITracking.x-10-1, (visParams->ROITracking.y + visParams->ROITracking.height)/2-5), cvScalar(0, 0, 0), -1 );
	cvRectangle( Window, cvPoint(  10,(visParams->ROITracking.y + visParams->ROITracking.height)/2+5 ),
					cvPoint(visParams->ROITracking.x-10-1, visParams->ROITracking.y + visParams->ROITracking.height+1 ), cvScalar(0, 0, 0), 1 );
	cvRectangle( Window, cvPoint(  10,visParams->ROITracking.y + visParams->ROITracking.height+1+ 10),
					cvPoint(Window->width - 10, Window->height - 65), cvScalar(0, 0, 0), -1 );
	// progreso
	cvPutText( Window, "Progreso:",
					cvPoint( (Window->width-visParams->BPrWidth )/2-2, Window->height - 50 ),
					&fuente1,
					cvScalar(0,0,0) );
	cvRectangle( Window, cvPoint(  (Window->width-visParams->BPrWidth )/2-2,( Window->height - 30 - Window->height/64)-2 ),
					cvPoint((Window->width + visParams->BPrWidth)/2+2, (Window->height - 30 + Window->height/64)+2 ), cvScalar(0, 0, 0), -1 );



//	// rectangulo de imagen
//	cvRectangle( Window, cvPoint(visParams->ROITracking.x-1,visParams->ROITracking.y-1),cvPoint(visParams->ROITracking.x + visParams->ROITracking.width+1,visParams->ROITracking.y + visParams->ROITracking.height+1), CVX_WHITE, -1 );
//	// rectangulo de barra progreso
//	cvRectangle( Window, cvPoint(  (Window->width-visParams->BPrWidth )/2-2,( Window->height - 30 - Window->height/64)-2 ),
//							cvPoint((Window->width + visParams->BPrWidth)/2+2, (Window->height - 30 + Window->height/64)+2 ), CVX_WHITE, 1 );
//	// rectangulos de datos estadisticos
//	cvRectangle( Window, cvPoint(  10,10 ),
//						cvPoint( visParams->ROITracking.x-10-1, (visParams->ROITracking.y + visParams->ROITracking.height)/2-5), cvScalar(109, 109, 109), -1 );
//	cvRectangle( Window, cvPoint(  10,(visParams->ROITracking.y + visParams->ROITracking.height)/2+5 ),
//						cvPoint(visParams->ROITracking.x-10-1, visParams->ROITracking.y + visParams->ROITracking.height+1 ), cvScalar(109, 109, 109), -1 );
//	cvRectangle( Window, cvPoint(  10,visParams->ROITracking.y + visParams->ROITracking.height+1+ 10),
//						cvPoint(Window->width - 10, Window->height - 65), cvScalar(0, 0, 0), -1 );

}
void Incrustar( IplImage* src1, IplImage* src2, IplImage* dst, CvRect ROI ){

	int y2 = 0;
	for (int y = ROI.y; y < ROI.y + src2->height; y++){
		uchar* ptr1 = (uchar*) ( src1->imageData + y*src1->widthStep + 3*ROI.x);
		uchar* ptr2 = (uchar*) ( src2->imageData + y2*src2->widthStep );
		for (int x = 0; x<src2->width; x++){
			ptr1[3*x ] = ptr2[3*x ];
			ptr1[3*x +1] = ptr2[3*x + 1 ];
			ptr1[3*x +2] = ptr2[3*x + 2 ];

		}
		y2++;;
	}
}

void IncrustarTxt( int num){
	    CvFont fuente1;
	    CvFont fuente2;
		char Texto[100];

		cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
		cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
		CvSize textsize ;
		switch( num ){
			case 1:
				sprintf(Texto,"Buscando plato...");
					textsize = getTextSize(Texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);
					cvPutText( Window, Texto, cvPoint( (Window->width-textsize.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30), &fuente1, CVX_RED );

				break;
			case 2:
				sprintf(Texto,"Aprendiendo fondo...");
				textsize = getTextSize(Texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);
				cvPutText( Window, Texto, cvPoint( (Window->width-textsize.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30), &fuente1, CVX_RED );
				break;
			case 3:
				sprintf(Texto,"Aprendiendo forma...");
				textsize = getTextSize(Texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);
				cvPutText( Window, Texto, cvPoint( (Window->width-textsize.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30), &fuente1, CVX_RED );
				break;
			case 4:
//				sprintf(Texto,"Rastreando...");
//				textsize = getTextSize(Texto, CV_FONT_HERSHEY_PLAIN, 2, 2, 0);
//				cvPutText( Window, Texto, cvPoint( (Window->width-textsize.width)/2,VParams->ROITracking.y + VParams->ROITracking.height + 30), &fuente1, CVX_RED );
			break;
			case 5:

				break;
		}
}

void IncrustarLogo(const char Cad[100] , IplImage* ImLogos, CvPoint Origen,int Delay, bool Clear){

	IplImage* Logo = NULL;
	CvPoint Centrar;
	//CreateWindows( ImVisual );
	Logo = cvLoadImage( Cad ,1);
	if(!Logo) return;
	if( Origen.x == -1 || Origen.y == -1){
		Centrar = cvPoint( (ImLogos->width-Logo->width)/2,( ImLogos->height-Logo->height)/2);
		Origen = Centrar;
	}
	if( Origen.x == -2 || Origen.y == -2){
			Centrar = cvPoint( (Window->width-Logo->width)/2,( Window->height-Logo->height)/4 );
			Origen = Centrar;
	}
	if( Origen.x == -3 || Origen.y == -3){
				Centrar = cvPoint( (ImLogos->width-Logo->width)/2,(( ImLogos->height+Logo->height)/4));
				Origen = Centrar;
		}
	Incrustar( ImLogos, Logo, ImLogos,cvRect(Origen.x,Origen.y,ImLogos->width,ImLogos->height)  );
	cvShowImage("TrackingDrosophila",ImLogos);
	cvWaitKey(Delay);
	if( Clear ) cvZero( ImLogos );
	cvReleaseImage(&Logo);
}

void desvanecer( IplImage* Imagen ,int Delay ){

	CvRect ROI;

	if ( Imagen == NULL ){
		Imagen = Window;
	}
	for( int i = 0; i < 255; i += 4 )
	{
		cvAddS(Imagen,cvScalar(-i,-i,-i), Imagen, NULL );
		if( Imagen->roi ){
			ROI.height = Imagen->roi->height ;
			ROI.width = Imagen->roi->width ;
			ROI.x = Imagen->roi->xOffset ;
			ROI.y = Imagen->roi->yOffset ;
			cvResetImageROI(Imagen);
			cvShowImage("TrackingDrosophila", Window);
			cvSetImageROI(Imagen,ROI);
		}
		else cvShowImage("TrackingDrosophila", Window);
		if(i<100) waitKey(Delay);
	}
	cvZero(Imagen);
}
// TransicioneS entre el inicio, el preprocesado y el procesado
void Transicion( const char texto[], int delay_up, int delay_on,int delay_down){

	if( visParams->TransOff) return;
	CvFont fuente1;

	CvRect rect = cvRect(0,0,Window->width,Window->height);
	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, CV_AA);
	CvSize textsize = getTextSize(texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);

	desvanecer( Window, 20 );
	for( int i = 0; i < 255; i += 2 )
	{
		cvPutText( Window, texto,
				cvPoint( (Window->width-textsize.width)/2,( Window->height-textsize.height)/2),
				&fuente1,
				cvScalar(i,i,i) );
		cvShowImage("TrackingDrosophila", Window);
		cvWaitKey(delay_up);
	}
	cvWaitKey(delay_on);
	desvanecer( Window, delay_down );

}

// transición entre partes del preprocesado
void Transicion2( const char texto[], int delay_up ){

	if( visParams->TransOff) return;

	CvFont fuente1;

	CvRect rect = cvRect(0,0,Window->width,Window->height);
	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
	CvSize textsize = getTextSize(texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);
	desvanecer( Window, 20 );
	for( int i = 0; i < 255; i += 4 )
		{

		CvRect rect = cvRect( (Window->width-Window->width/4)/2,
											( Window->height-Window->height/4)/8,
											Window->width/4,
											Window->height/4);
		cvRectangle( Window,
					cvPoint(rect.x,rect.y),
					cvPoint(rect.x + rect.width,
							rect.y +rect.height),
							cvScalar(i,i,i), 1 );
		cvRectangle( Window, cvPoint(visParams->ROIPreProces.x,visParams->ROIPreProces.y),
				cvPoint(visParams->ROIPreProces.x + visParams->ROIPreProces.width,visParams->ROIPreProces.y + visParams->ROIPreProces.height),
				cvScalar(i,i,i), 1 );
			cvShowImage("TrackingDrosophila", Window);
			cvWaitKey(delay_up);
		}
	for( int i = 0; i < 255; i += 2 )
	{
		cvPutText( Window, texto,
				cvPoint( (Window->width-textsize.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30),
				&fuente1,
				cvScalar(i,i,i) );
		cvShowImage("TrackingDrosophila", Window);
		cvWaitKey(delay_up);
	}

}
///Transición tras llenar el buffer
void Transicion3( const char texto[], int delay_up ){

	if( visParams->TransOff) return;

	CvFont fuente1;
	CvFont fuente2;

	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
	cvInitFont( &fuente2, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
	CvSize textsize = getTextSize("Progreso:", CV_FONT_HERSHEY_PLAIN, 1, 1, 0);
	desvanecer( Window, 20 );
	// dibujar rectángulo de la imagen
		for( int i = 0; i < 255; i += 4 )
			{
			cvRectangle( Window, cvPoint(0,0),
					cvPoint(visParams->Resolucion.width ,visParams->Resolucion.height), cvScalar(i,i,i), -1 );
			// imagen
			cvRectangle( Window, cvPoint(visParams->ROITracking.x-1,visParams->ROITracking.y-1),
					cvPoint(visParams->ROITracking.x + visParams->ROITracking.width+1,visParams->ROITracking.y + visParams->ROITracking.height+1), cvScalar(0, 0, 0), 1 );

			//estadisticas
			cvRectangle( Window, cvPoint(  10,10 ),
							cvPoint( visParams->ROITracking.x-10-1, (visParams->ROITracking.y + visParams->ROITracking.height)/2-5), cvScalar(0, 0, 0), -1 );
			cvRectangle( Window, cvPoint(  10,(visParams->ROITracking.y + visParams->ROITracking.height)/2+5 ),
							cvPoint(visParams->ROITracking.x-10-1, visParams->ROITracking.y + visParams->ROITracking.height+1 ), cvScalar(0, 0, 0), 1 );
			cvRectangle( Window, cvPoint(  10,visParams->ROITracking.y + visParams->ROITracking.height+1+ 10),
							cvPoint(Window->width - 10, Window->height - 65), cvScalar(0, 0, 0), -1 );
			// progreso
			cvPutText( Window, "Progreso:",
							cvPoint( (Window->width-visParams->BPrWidth )/2-2, Window->height - 50 ),
							&fuente2,
							cvScalar(0, 0, 0) );
			cvRectangle( Window, cvPoint(  (Window->width-visParams->BPrWidth )/2-2,( Window->height - 30 - Window->height/64)-2 ),
							cvPoint((Window->width + visParams->BPrWidth)/2+2, (Window->height - 30 + Window->height/64)+2 ), cvScalar(0, 0, 0), -1 );


			cvShowImage("TrackingDrosophila", Window);
			cvWaitKey(delay_up);
		}

//	// dibujar rectángulo de la imagen
//	for( int i = 0; i < 255; i += 4 )
//		{
//		cvRectangle( Window, cvPoint(visParams->ROITracking.x-1,visParams->ROITracking.y-1),
//				cvPoint(visParams->ROITracking.x + visParams->ROITracking.width+1,visParams->ROITracking.y + visParams->ROITracking.height+1), cvScalar(i,i,i), 1 );
//
//		cvShowImage("TrackingDrosophila", Window);
//		cvWaitKey(delay_up);
//		}

	// Dibujar rectangulo exterior de la barra de progreso y rectangulos de estadísticas
//	for( int i = 0; i < 255; i += 2 )
//		{
//		cvPutText( Window, "Progreso:",
//						cvPoint( (Window->width-visParams->BPrWidth )/2-2, Window->height - 50 ),
//						&fuente2,
//						cvScalar(i,i,i) );
//		cvRectangle( Window, cvPoint(  (Window->width-visParams->BPrWidth )/2-2,( Window->height - 30 - Window->height/64)-2 ),
//						cvPoint((Window->width + visParams->BPrWidth)/2+2, (Window->height - 30 + Window->height/64)+2 ), cvScalar(i,i,i), 1 );
//
//		cvShowImage("TrackingDrosophila", Window);
//		cvWaitKey(delay_up);
//
//		}
//	for( int i = 0; i < 105; i += 2 )
//	{
//		//
//		cvRectangle( Window, cvPoint(  10,10 ),
//						cvPoint( visParams->ROITracking.x-10-1, (visParams->ROITracking.y + visParams->ROITracking.height)/2-5), cvScalar(i, i, i), -1 );
//		cvRectangle( Window, cvPoint(  10,(visParams->ROITracking.y + visParams->ROITracking.height)/2+5 ),
//						cvPoint(visParams->ROITracking.x-10-1, visParams->ROITracking.y + visParams->ROITracking.height+1 ), cvScalar(i, i, i), -1 );
//		cvRectangle( Window, cvPoint(  10,visParams->ROITracking.y + visParams->ROITracking.height+1+ 10),
//						cvPoint(Window->width - 10, Window->height - 65), cvScalar(i, i, i), -1 );

//		cvShowImage("TrackingDrosophila", Window);
//		cvWaitKey(delay_up);
//
//	}
	// Texto de estado
	for( int i = 255; i > 0; i -= 2 )
	{
		cvPutText( Window, texto,
				cvPoint( (Window->width-textsize.width)/2,visParams->ROITracking.y + visParams->ROITracking.height + 30),
				&fuente1,
				cvScalar(i,i,i) );
		cvShowImage("TrackingDrosophila", Window);
		cvWaitKey(delay_up);
	}


}
// Transicion entre partes del preprocesado
void Transicion4(const char texto[], int delay_down){

	if( visParams->TransOff) return;

	CvFont fuente1;
	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 2, 2, 0, 1, 8);
	CvSize textsize ;
	textsize = getTextSize(texto, CV_FONT_HERSHEY_PLAIN, 2, 1, 0);
	cvPutText( Window, texto, cvPoint( (Window->width-textsize.width)/2,visParams->ROIPreProces.y + visParams->ROIPreProces.height + 30), &fuente1, CVX_GREEN );
	CvRect rect = cvRect(visParams->ROIPreProces.x +1,
						visParams->ROIPreProces.y +1,visParams->ROIPreProces.width -1,
						visParams->ROIPreProces.height -1);
	cvSetImageROI(Window,rect);
	desvanecer( Window, delay_down);
	cvResetImageROI(Window);
}

void SetHightGUIParams(  IplImage* ImRef,char* nombreVideo, double FPS ){
    //init parameters
	config_t cfg;
	config_setting_t *setting;
	char settingName[30];
	char configFile[30];
	char settingFather[30];


	int EXITO;
	int DEFAULT = false;

	// si no han sido localizados hacerlo.
	if( !visParams){
		visParams = ( VisParams *) malloc( sizeof( VisParams) );
		if(!visParams) {error(4); return;}
	}
	printf("\nFlujo de fichero de vídeo iniciado.");
	VWriter = iniciarAvi(  nombreVideo, FPS);
	printf("\nCargando parámetros de configuración globales...");
	config_init(&cfg);

	sprintf( configFile, "config.cfg");
	sprintf( settingFather,"Visualizacion" );

	 /* Leer archivo. si hay un error, informar y cargar configuración por defecto */
	if(! config_read_file(&cfg, configFile))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));

		fprintf(stderr, "Error al acceder al fichero de configuración %s .\n"
						" Estableciendo valores por defecto.\n"
						,configFile);
		DEFAULT = true;
	}
	else
	{
		setting = config_lookup(&cfg, settingFather);
		/* Si no se se encuentra la setting o bien existe la variable hijo Auto y ésta es true, se establecen TODOS los valores por defecto.*/
		if(setting != NULL)
		{
			sprintf(settingName,"Auto");
			/* Obtener el valor */
			EXITO = config_setting_lookup_bool ( setting, settingName, &DEFAULT);
			if(!EXITO) DEFAULT = true;
			else if( EXITO && DEFAULT ) fprintf(stderr, "Opción Auto activada para el campo %s.\n"
												" Estableciendo valores por defecto.\n",settingFather);
			else if( EXITO && !DEFAULT) fprintf(stderr, "Opción Auto desactivada para el campo %s.\n"
												" Estableciendo valores del fichero de configuración.\n",settingFather);
		}
		else {
			DEFAULT = true;
			fprintf(stderr, "Error.No se ha podido leer el campo %s.\n"
							" Estableciendo valores por defecto.\n",settingFather);
		}
	}

	if( DEFAULT ) SetDefaultHightGUIParams( ImRef );
	/* Valores leídos del fichero de configuración. Algunos valores puedes ser establecidos por defecto si se indica
	 * expresamente en el fichero de configuración. Si el valor es erroneo o no se encuentra la variable, se establecerán
	 * a los valores por defecto.
	 */
	else{
		 /* Get the store name. */
		sprintf(settingName,"ShowWindow");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowWindow )  ){
			visParams->ShowWindow = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
										"Establecer por defecto a %d \n",settingName,visParams->ShowWindow);
		}

		sprintf(settingName,"ShowPresent");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowPresent )  ){
			visParams->ShowPresent = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowPresent);

		}

		sprintf(settingName,"ShowTransition");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowTransition )  ){
			visParams->ShowTransition  = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowTransition );

		}

		sprintf(settingName,"HightGUIControls");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->HightGUIControls )  ){
			visParams->HightGUIControls = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->HightGUIControls);

		}

		sprintf(settingName,"ModoCompleto");

		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ModoCompleto )  ){
			visParams->ModoCompleto = false;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ModoCompleto);

		}

		sprintf(settingName,"ShowBGdiffImages");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowBGdiffImages )  ){
			visParams->ShowBGdiffImages  = false ;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowBGdiffImages);

		}

		sprintf(settingName,"ShowLearningFlat");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowLearningFlat )  ){
			visParams->ShowLearningFlat = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowLearningFlat);

		}

		sprintf(settingName,"ShowInitBackground");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowInitBackground )  ){
			visParams->ShowInitBackground = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowInitBackground);

		}

		sprintf(settingName,"ShowShapeModel");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowShapeModel)  ){
			visParams->ShowShapeModel= true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowShapeModel);

		}

		sprintf(settingName,"ShowProcessPhases");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowProcessPhases )  ){
			visParams->ShowProcessPhases = false ;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowProcessPhases);

		}

		sprintf(settingName,"ShowValidationPhases");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowValidationPhases )  ){
			visParams->ShowValidationPhases = false;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowValidationPhases);

		}

		sprintf(settingName,"ShowBGremoval");
		if(! config_setting_lookup_bool ( setting, settingName, &visParams->ShowBGremoval ) ){
			visParams->ShowBGremoval = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowBGremoval);

		}

		sprintf(settingName,"ShowKalman");
		if(!config_setting_lookup_bool ( setting, settingName, &visParams->ShowKalman ) ){
			visParams->ShowKalman = true;
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
							"Establecer por defecto a %d \n",settingName,visParams->ShowKalman);

		}
	}

	SetPrivatetHightGUIParams(  ImRef );

//	ShowParams( settingFather );
	config_destroy(&cfg);
}

void SetPrivatetHightGUIParams(  IplImage* ImRef ){

	// Parametros no configurables
	visParams->pause = false;
	visParams->stop = false;
	visParams->RecWindow = true;
	visParams->pasoApaso = false;
	visParams->VisualPos = -1;
	visParams->Resolucion = cvSize(1280,800 );
	visParams->DelayLogo = 2000;
	visParams->DelayDown = 20;
	visParams->DelayUp = 20 ;
	visParams->DelayTr = 20;
	visParams->TransOff = true;
	visParams->BPrWidth = visParams->Resolucion.width - 20;

	if(!Window)	Window = cvCreateImage( visParams->Resolucion,8,3);
	cvZero(Window);
	if( ImRef ){
		if(!ImVisual) {
			ImVisual = cvCreateImage( cvGetSize( ImRef ),8,3);
			if( ImRef->width <= 320){
				CvSize size = cvSize(2*ImRef->width,2*ImRef->height);
				ImScale = cvCreateImage( size ,8,3);
				visParams->ROITracking = cvRect(visParams->Resolucion.width-(2*ImRef->width + 10),
						10,2*ImRef->width,
						2*ImRef->height);
				visParams->ROIPreProces = cvRect( (Window->width-2*ImRef->width)/2,
						( Window->height-2*ImRef->height)/2,
						2*ImRef->width,2*ImRef->height);
			}
			else{
				ImScale = ImVisual;
				visParams->ROITracking = cvRect(visParams->Resolucion.width-(ImRef->width + 10),10,ImRef->width,ImRef->height);
				visParams->ROIPreProces = cvRect( (Window->width-ImRef->width)/2,
													( Window->height-ImRef->height)/1.5,
													ImRef->width,
													ImRef->height);
			}
		}
	}
}

void SetDefaultHightGUIParams(  IplImage* ImRef ){

	visParams->ShowWindow = true;

	visParams->ShowPresent = true ;

	visParams->ShowTransition = true;
	visParams->HightGUIControls = true ;
	visParams->ModoCompleto = false;
	visParams->ShowBGdiffImages  = false ;

	// Resultados de preprocesado
	visParams->ShowLearningFlat = true;
	visParams->ShowInitBackground = true;
	visParams->ShowShapeModel= true;

	// Resultados de procesado
	visParams->ShowProcessPhases = false ;
	visParams->ShowValidationPhases = false;
	visParams->ShowBGremoval = true;
	visParams->ShowKalman = true;

}

CvVideoWriter* iniciarAvi(  char* nombreVideo, double fps){

	CvVideoWriter *writer = NULL;

	CvSize size = cvSize(1280,800); //( 1280, 800)
	fps = 30;
	writer = cvCreateVideoWriter(
							nombreVideo,
							CV_FOURCC('P','I','M','1'),
							fps,
							size
							);//'F', 'M', 'P', '4';'M', 'J', 'P', 'G';'D','I','V','X';'P','I','M','1'

	return writer;
}

void releaseVisParams( ){
	free( visParams);
	if(ImVisual) cvReleaseImage(&ImVisual);
	ImVisual = NULL;
//	if( ImScale) cvReleaseImage( &ImScale);
	cvReleaseImage(&Window);
	if (VWriter) cvReleaseVideoWriter(&VWriter);
}
//void VerEstadoSHModel( IplImage* Imagen,int num ){
//
//}

//void VerEstadoBGModel( IplImage* Imagen ){
//
//	static int count = 0 ;
//	float anchoBuf = 200; // longitud en pixels del ancho del buffer
//	IplImage* bgmod;
//	char PComplet[100];
//	CvFont fuente1;
//	float porcentaje;
//
//	CvSize size = cvSize(Imagen->width,Imagen->height); // get current frame size
//	bgmod= cvCreateImage(size, IPL_DEPTH_8U, 3);
//	cvZero( bgmod);
//
//	cvRectangle( bgmod, cvPoint( 218, 228 ),cvPoint( 422, 252), CVX_WHITE, 1 );
//	cvRectangle( bgmod, cvPoint( 220, 230 ),cvPoint( 220 + count , 250 ), CVX_GREEN, -1 );
//
//	count = count + 4;
//	porcentaje = (count/anchoBuf)*100;
//	sprintf(PComplet," %.0f %% ",porcentaje );
////	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
//	cvInitFont( &fuente1, CV_FONT_HERSHEY_PLAIN, 1, 1, 0, 1, 8);
//	cvPutText(bgmod, PComplet,  cvPoint( 300,265), &fuente1, CVX_WHITE );
//	cvPutText( bgmod, "Modelando fondo...",cvPoint( 250,225 ) ,&fuente1,CVX_WHITE );
//	cvShowImage( "Visualización", bgmod );
//	cvReleaseImage(&bgmod);
//}

// Creación de ventanas
void CreateWindows( IplImage* ImRef){

//	cvNamedWindow( "Drosophila.avi", CV_WINDOW_AUTOSIZE );
	if (SHOW_BG_REMOVAL == 1){
		cvNamedWindow( "Background",CV_WINDOW_AUTOSIZE);
		cvNamedWindow( "Foreground",CV_WINDOW_AUTOSIZE);
		cvMoveWindow("Background", 0, ImRef->height );
		cvMoveWindow("Foreground", ImRef->width, ImRef->height);
	}
	if ( SHOW_MOTION_TEMPLATE == 1){
		cvNamedWindow( "Motion", 1 );
	}


	if (SHOW_OPTICAL_FLOW == 1){
		cvNamedWindow( "Flujo Optico X",CV_WINDOW_AUTOSIZE);
		cvNamedWindow( "Flujo Optico X",CV_WINDOW_AUTOSIZE);
		cvMoveWindow("Flujo Optico X", 0, 0 );
		cvMoveWindow("Flujo Optico Y", 640, 0);
	}
	if (SHOW_VISUALIZATION == 1){
			cvNamedWindow( "Visualización",CV_WINDOW_AUTOSIZE);
			cvMoveWindow("Visualización", 0, 0 );
			cvNamedWindow( "TrackingDrosophila",CV_WINDOW_AUTOSIZE);
			cvMoveWindow("TrackingDrosophila", 0, 0 );
	}
	//        cvNamedWindow( "Imagen", CV_WINDOW_AUTOSIZE);
    //	cvNamedWindow( "Region_Of_Interest", CV_WINDOW_AUTOSIZE);


}
// Destrucción de ventanas y parametros de visualización
void DestroyWindows( ){

	//cvDestroyWindow( "Drosophila.avi" );

	if (SHOW_BG_REMOVAL == 1){
		cvDestroyWindow( "Background");
		cvDestroyWindow( "Foreground");
	}
	if (SHOW_OPTICAL_FLOW == 1){
		cvDestroyWindow( "Flujo Optico X");
		cvDestroyWindow( "Flujo Optico Y");
	}
	if ( SHOW_MOTION_TEMPLATE == 1){
		cvDestroyWindow( "Motion");
	}
	if (SHOW_VISUALIZATION == 1) {
		cvDestroyWindow( "Visualización" );
	}

}

