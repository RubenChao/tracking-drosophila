/*!
 * TrackingDrosophila.cpp
 *
 * Cuerpo principal del algoritmo de rastreo.
 *
 *  Created on: 27/06/2011
 *      Author: chao
 */

#include "TrackingDrosophila.hpp"

using namespace cv;
using namespace std;

extern double NumFrame ; /// contador de frames absolutos ( incluyendo preprocesado )

double NumFrame ; /// contador de frames absolutos ( incluyendo preprocesado )

/// GLOBAL
GlobalConf* GParams;
/// PREPROCESADO
//Modelado de fondo
StaticBGModel* BGModel = NULL;
// Modelado de forma
SHModel* Shape;

/// PROCESADO
/// Estructura frame
STFrame* FrameDataIn = NULL;
STFrame* FrameDataOut = NULL;

///Parámetros fondo para procesado
BGModelParams *BGPrParams = NULL;
///Parámetros Validación para procesado
ValParams* valParams = NULL;

///HightGui
int g_slider_pos = 0;

char nombreFichero[30];
char nombreVideo[30];

int main(int argc, char* argv[]) {

	//  CAPTURA  //
	CvCapture* g_capture = NULL;/// puntero a una estructura de tipo CvCapture

	IplImage* frame;

	struct timeval ti,  tif, tinicio; // iniciamos la estructura de medida de tiempos

	if( argc<1) {help(); return -1;};

	///////////  INICIALIZACIÓN ////////////

	printf( "Iniciando captura...\n" );
	gettimeofday(&tinicio, NULL);//para obtener el tiempo transcurrido desde el inicio del programa
	g_capture = cvCaptureFromAVI( argv[1] );
	if ( !g_capture ) {
		error( 1 );
		help();
		return -1;
	}

	printf("\nInicializando parámetros...");

//  Iniciar parámetros generales y de visualización y crear interface.
	if (!Inicializacion( argc, argv,nombreFichero,nombreVideo ) ) return -1;
	SetGlobalConf( g_capture );
	SetHightGUIParams( cvQueryFrame( g_capture ) , nombreVideo, (double)GParams->FPS);

	////////// PRESENTACIÓN ////////////////
	DraWWindow( NULL,NULL, NULL, SHOW_PRESENT, 0  );

	//////////  PREPROCESADO   ////////////
	if (!PreProcesado( argv[1], &BGModel, &Shape) )  Finalizar(&g_capture);

	/////////	PROCESADO   ///////////////
	printf("\n\nIniciando procesado...\n");
	if(SHOW_WINDOW) Transicion("Iniciando Tracking...", 1,1000, 50 );
	//NumFrame = 0;

	/*********** BUCLE PRINCIPAL DEL ALGORITMO ***********/
	for( int i = 0; i < GParams->InitDelay; i++ )	frame = cvQueryFrame( g_capture );	// retardo al inicio
	NumFrame = GParams->InitDelay;
    while( true ){
    	///////// CAPTURAR ///////////////
    	frame = cvQueryFrame(g_capture);
    	//NumFrame = cvGetCaptureProperty( g_capture, 1 );
    	NumFrame++;
    	printf("\t\t\tFRAME %0.f\n ",NumFrame);
    	/*Posteriormente  Escribir en un fichero log el error. Actualizar el contador
    	  de frames absolutos. */
    	gettimeofday(&tif, NULL);
    	gettimeofday(&ti, NULL);
    	if( !frame ){
    		NumFrame = NumFrame + RetryCap( g_capture, frame );
    		if( !frame ) break; //Finalizar(&g_capture, &VWriter);
    		//NumFrame = cvGetCaptureProperty( g_capture, 1 );
    	}
		if ( (cvWaitKey(10) & 255) == 27 ) break; // si se puls ESC salir

		//////////  PROCESAR  ////////////
		FrameDataIn = Procesado(frame, BGModel, Shape, valParams, BGPrParams );

		//////////  RASTREAR  ////////////
		FrameDataOut = Tracking( FrameDataIn, 10, BGModel,GParams->FPS );

		////////// ESTADISTICAS //////////
		CalcStatsFrame( FrameDataOut );

		//////////  VISUALIZAR  //////////
//			VisualizarFr( FrameDataOut , BGModel, VWriter );
		DraWWindow( frame, FrameDataOut, BGModel, TRAKING, SIMPLE );

		//////////  ALMACENAR ////////////
		if(!GuardarSTFrame( FrameDataOut, nombreFichero ) ){error(6);Finalizar(&g_capture);}

		////////// LIBERAR MEMORIA  ////////////
		liberarSTFrame( FrameDataOut );

		FrameDataIn->GStats = SetGlobalStats( NumFrame, tif, tinicio, GParams->TotalFrames, GParams->FPS );

		printf("\n//////////////////////////////////////////////////\n");
		printf("\nTiempo de procesado del  Frame %.0f : %5.4g ms\n",NumFrame, FrameDataIn->GStats->TiempoFrame);
		printf("Segundos de video procesados: %0.f seg \n", FrameDataIn->GStats->TiempoGlobal);
		printf("Porcentaje completado: %.2f %% \n",((NumFrame-1)/GParams->TotalFrames)*100 );
		printf("\n//////////////////////////////////////////////////\n");
		printf("\n//////////////////////////////////////////////////\n");
//		if(!SHOW_WINDOW)	VisualizarFr( FrameDataIn, BGModel, VWriter, visParams );
//		liberarSTFrame( FrameDataIn );
    }

    ///////// POST-PROCESADO ///////////////////

	///////// LIBERAR MEMORIA Y TERMINAR////////
    Finalizar(&g_capture);

}

void SetGlobalConf(  CvCapture* Cap ){
    //init parameters
	config_t cfg;
	config_setting_t *setting;
	char settingName[30];
	char configFile[30];
	char settingFather[30];


	int EXITO;
	int DEFAULT = false;

	// si no han sido localizados hacerlo.
	if( !GParams){
		GParams = ( GlobalConf *) malloc( sizeof( GlobalConf) );
		if(!GParams) {error(4); exit(1);}
	}
	printf("\nCargando parámetros de configuración globales...");
	config_init(&cfg);

	sprintf( configFile, "config.cfg");
	sprintf( settingFather,"General" );

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
	/* Valores leídos del fichero de configuración. Algunos valores puedes ser establecidos por defecto si se indica
	 * expresamente en el fichero de configuración. Si el valor es erroneo o no se encuentra la variable, se establecerán
	 * a los valores por defecto.
	 */
	if(!DEFAULT ) {
		 /* Get the store name. */
		sprintf(settingName,"MaxFlat");
		/* Obtener el valor */
		EXITO = config_setting_lookup_float ( setting, settingName, &GParams->MaxFlat);
		/* Si no se encuentra la variable o su valor es erroneo establecer valor por defecto*/
		if(! EXITO || GParams->MaxFlat == 0 ){
			fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o"
					" su valor es erróneo.\n No se puede Calibrar la cámara.Las unidades de medida para "
					" los cálculos estadísticos se establecerán en pixels.\n",settingName);
			GParams->MaxFlat = 0;
		}

		sprintf(settingName,"FPS");
		EXITO = config_setting_lookup_int ( setting, settingName, &GParams->FPS);
		if(! EXITO || GParams->FPS == 0 ){
			GParams->FPS = cvGetCaptureProperty( Cap, 5 );
			if( !EXITO) fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
										"Establecido por defecto a %0.1f \n",settingName,GParams->FPS);
			if( GParams->FPS == 0 ) fprintf(stderr, "Establecer %s por defecto a %0.1f \n",settingName,GParams->TotalFrames);
		}

		sprintf(settingName,"TotalFrames");
		EXITO = config_setting_lookup_float ( setting, settingName, &GParams->TotalFrames);
		if(!EXITO || GParams->TotalFrames == 0 ){
			GParams->TotalFrames = 0;
			GParams->TotalFrames = cvGetCaptureProperty( Cap, 7);
			if( !EXITO) fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
					"Establecido por defecto a %0.1f \n",settingName,GParams->TotalFrames);
			if( GParams->TotalFrames == 0 ) fprintf(stderr, "Establecer %s por defecto a %0.1f \n",settingName,GParams->TotalFrames);
			//if(!GParams->TotalFrames) GParams->TotalFrames = getAVIFrames(argv[1]);
		}

		sprintf(settingName,"InitDelay");
		EXITO = config_setting_lookup_int ( setting, settingName, &GParams->InitDelay);
		if(! EXITO ){
			GParams->InitDelay = 50;
			if( !EXITO) fprintf(stderr, "No se encuentra la variable %s en el archivo de configuración o su valor es erróneo.\n "
										"Establecido por defecto a %d \n",settingName,GParams->InitDelay);
		}
	}
	else {

		fprintf(stderr, " Calibración de cámara suspendida. Falta el parámetro MaxFlat."
						" \n Unidades de medida para"
						" cálculos estadísticos establecidas en pixels.\n");
		SetDefaultGlobalParams( Cap);


	}
	ShowParams( settingFather );
	config_destroy(&cfg);
}

int SetDefaultGlobalParams(  CvCapture* Cap ){

	GParams->TotalFrames = 0;
	GParams->TotalFrames = cvGetCaptureProperty( Cap, 7);
		//	if(!GParams->TotalFrames) GParams->TotalFrames = getAVIFrames(argv[1]); // en algun linux no funciona lo anterior
	GParams->FPS = cvGetCaptureProperty( Cap, 5 );
	GParams->MaxFlat = 0;

	GParams->InitDelay = 50	;
	GParams->MedirTiempos = false ;
	return 1;
}

void ShowParams( char* Campo ){

	printf(" \nVariables para el campo %s : \n", Campo);
	printf(" -TotalFrames = %0.1f \n", GParams->TotalFrames);
	printf(" -FPS = %d \n", GParams->FPS);
	printf(" -MaxFlat = %0.1f \n", GParams->MaxFlat);
	printf(" -InitDelay = %d \n", GParams->InitDelay);
	printf(" -MedirTiempos = %d \n", GParams->MedirTiempos);

}

void Finalizar(CvCapture **g_capture){

	CvCapture *capture;
	CvVideoWriter *Writer;

	// liberar parámetros globales de configuración
	free( GParams);
	// liberar imagenes y datos de preprocesado
	releaseDataPreProcess();
	//liberar modelo de fondo
	DeallocateBGM( BGModel );
	//liberar modelo de forma
	if( Shape) free(Shape);

	// liberar imagenes y datos de procesado
	releaseDataProcess(valParams, BGPrParams);
	if( FrameDataIn ) liberarSTFrame( FrameDataIn);
//	if( FrameDataOut ) liberarSTFrame( FrameDataOut);

	// liberar imagenes y datos de tracking
	ReleaseDataTrack( );

	// liberar estructura estadísticas
	releaseStats();

	// liberar imagenes y datos de visualización
	releaseVisParams( );
	DestroyWindows( );

	//liberar resto de estructuras
	capture = *g_capture;

	if (capture)cvReleaseCapture(&capture);


	exit (1);
}



