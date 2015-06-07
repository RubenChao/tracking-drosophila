/*
 * Stats.cpp
 *
 *  Tenemos un proceso estocástico , es decir, una serie de variables aleatorias ordenadas en el tiempo (serie temporal)
 *  En esta función se calcularán una serie de parámetros estadísticos.
 *  Por un lado se calcularán estadísticas de los blobs en conjunto y por otro lado de cada blob de forma individual.
 *
 *  En cálculo de estadísticas de blobs en conjunto obtendremos la cantidad de movimiento medio u su desviación en distintos intervalos
 *  de tiempo. Se usarán medias móviles. Para ello se mantiene una cola FIFO de una dimensión tal que almacena 8 horas de datos
 *
 *
 *
 *  Created on: 31/01/2012
 *      Author: chao
 */

#include "Stats.hpp"



STStatsCoef* Coef = NULL;
tlcde* vectorSumFr = NULL;
STStatFrame* Stats = NULL;


void CalcStatsFrame(STFrame* frameDataOut ){

	struct timeval ti;
	float TiempoParcial;
	static int init = 1;

	if( !frameDataOut) return;
#ifdef MEDIR_TIEMPOS
	gettimeofday(&ti, NULL);
	printf( "Rastreo finalizado con éxito ..." );
	printf( "Comenzando análisis estadístico de los datos obtenidos ...\n" );
	printf("\n3)Cálculo de estadísticas en tiempo de ejecución:\n");

#endif


	// Iniciar estructuras de estadísticas.
	 frameDataOut->Stats = InitStatsFrame(  frameDataOut->GStats->fps );

	// estadísticas de cada blob
	statsBloB( frameDataOut );

	// cálculos estadísticos del movimiento de los blobs en conjunto
	if( CALC_STATS_MOV ) statsBlobS( frameDataOut );


#ifdef MEDIR_TIEMPOS
	printf( "Análisis finalizado ...\n" );
	TiempoParcial = obtenerTiempo( ti , NULL);
	printf("Cálculos realizados. Tiempo total %5.4g ms\n", TiempoParcial);
#endif
}

STGlobStatF* SetGlobalStats( int NumFrame, timeval tif, timeval tinicio, int TotalFrames, float FPS  ){
	//FrameData->Stats->totalFrames = 0;
	// FRAME
	STGlobStatF* GStats = NULL;
	GStats = ( STGlobStatF *) malloc( sizeof(STGlobStatF));
	if(!GStats) {error(4); return 0;}

	GStats->TiempoFrame = obtenerTiempo( tif, 0 );
	GStats->TiempoGlobal = obtenerTiempo( tinicio, 1);
	GStats->numFrame = NumFrame;
	GStats->fps = FPS;
	GStats->totalFrames = TotalFrames;

	return GStats;

}

STStatFrame* InitStatsFrame(  int fps){

	if (!Stats){
		Stats = ( STStatFrame *) malloc( sizeof(STStatFrame));
		if(!Stats) {error(4); exit(1) ;}
	}

	if(!Coef){
		Coef =  ( STStatsCoef *) malloc( sizeof(STStatsCoef));
		calcCoef( fps );
	}

	// iniciar vector de cantidad de movimiento por frame
	if(!vectorSumFr) {
		vectorSumFr = ( tlcde * )malloc( sizeof(tlcde ));
		if( !vectorSumFr ) {error(4);exit(1);}
		iniciarLcde( vectorSumFr );
	}

//	// FRAME
//	Stats->TiempoFrame = obtenerTiempo( tif, 0 );
//	Stats->TiempoGlobal = obtenerTiempo( tinicio, 1);
//	Stats->totalFrames = TotalFrames;
//	Stats->numFrame = NumFrame;
//	Stats->fps = FPS;
	// BLOBS
	Stats->TProces = 0;
	Stats->TTacking= 0;
	Stats->staticBlobs= 0; //!< blobs estáticos en tanto por ciento.
	Stats->dinamicBlobs= 0; //!< blobs en movimiento en tanto por ciento
	Stats->TotalBlobs= 0; //!< Número total de blobs.

	Stats-> CMov1SMed = 0;  //!< Cantidad de movimiento medio en los últimos 30 seg.
	Stats-> CMov1SDes = 0;
	Stats-> CMov30SMed = 0;  //!< Cantidad de movimiento medio en los últimos 30 seg.
	Stats-> CMov30SDes = 0;
	Stats-> CMov1Med = 0;  //!< Cantidad de movimiento medio en el último min.
	Stats-> CMov1Des = 0;
	Stats-> CMov5Med = 0;  //!< Cantidad de movimiento medio en los últimos 5 min.
	Stats-> CMov5Des = 0;
	Stats-> CMov10Med = 0;  //!< Cantidad de movimiento medio en los últimos 10 min.
	Stats-> CMov10Des = 0;
	Stats-> CMov15Med = 0;  //!< Cantidad de movimiento medio en los últimos 15 min.
	Stats-> CMov15Des = 0;
	Stats-> CMov30Med = 0;  //!< Cantidad de movimiento medio en los últimos 30 min.
	Stats-> CMov30Des = 0;
	Stats->CMov1HMed = 0;  //!< Cantidad de movimiento medio en la última hora.
	Stats-> CMov1HDes = 0;
	Stats-> CMov2HMed = 0;	//!< Cantidad de movimiento medio en  últimas 2 horas.
	Stats-> CMov2HDes = 0;
	Stats->CMov4HMed = 0;
	Stats-> CMov4HDes = 0;
	Stats->CMov8HMed = 0; //!<//!< Cantidad de movimiento medio en  últimas 2 horas.
	Stats-> CMov8HDes = 0;
	Stats->CMov16HMed = 0;
	Stats-> CMov16HDes = 0;
	Stats->CMov24HMed = 0;
	Stats-> CMov24HDes = 0;
	Stats->CMov48HMed = 0;
	Stats-> CMov48HDes = 0;
	Stats->CMovMedio = 0;
	Stats-> CMovMedioDes = 0;

	return Stats;
}

void statsBloB(STFrame* frameDataOut ){

	STFly* fly = NULL;
	frameDataOut->Stats->TotalBlobs = frameDataOut->Flies->numeroDeElementos;
	for(int i = 0; i< frameDataOut->Stats->TotalBlobs ; i++){
		fly = (STFly*)obtener(i,frameDataOut->Flies);
		if( fly->Estado == 1 ) frameDataOut->Stats->dinamicBlobs +=1;
		else frameDataOut->Stats->staticBlobs +=1;

		// velocidad instantánea
//		EUDistance( fly->Vx, fly->Vy, NULL, &fly->Stats->VInst);
	}
	frameDataOut->Stats->dinamicBlobs = (frameDataOut->Stats->dinamicBlobs/frameDataOut->Stats->TotalBlobs)*100;
	frameDataOut->Stats->staticBlobs = (frameDataOut->Stats->staticBlobs/ frameDataOut->Stats->TotalBlobs)*100;



}
/*!\brief Calcula las medias y varianzas moviles para la cantidad de movimiento
 *
 * @param frameDataStats
 * @param frameDataOut
 */
void statsBlobS( STFrame* frameDataOut ){

	STFly* flyOut = NULL;

	valorSum* valor = NULL;

	if(frameDataOut->Flies->numeroDeElementos == 0 ) return; // si no hay datos del frame, no computar

	// Obtener el nuevo valor
	valor =  ( valorSum *) malloc( sizeof(valorSum));
	valor->sum = sumFrame( frameDataOut ->Flies );
	anyadirAlFinal( valor, vectorSumFr );

	// Si es el primer elemento, iniciar medias
	if(vectorSumFr->numeroDeElementos == 1){
		iniciarMedias( frameDataOut->Stats, valor->sum );
		return;
	}
	// movimiento global. Calculo de medias y desviaciones
	mediaMovil( Coef, vectorSumFr, frameDataOut->Stats );


	if( (unsigned)vectorSumFr->numeroDeElementos == Coef->FMax){
		valor = (valorSum*)liberarPrimero( vectorSumFr );
		free(valor);
	}

}
// Calcula los coeficientes para hayar las medias y desviaciones de la serie temporal
void calcCoef( int FPS ){

	Coef->SumFrame = 0;

	Coef->F1S = FPS ;
	Coef->F30S = 30*FPS ;	//!< frames para calcular la media de 30 s
	Coef->F1  =  60*FPS;
	Coef->F5  = 300*FPS;
	Coef->F10 = 600*FPS;
	Coef->F15 = 900*FPS;
	Coef->F30 = 1800*FPS;
	Coef->F1H = 3600*FPS;
	Coef->F2H = 7200*FPS;
	Coef->F4H = 14400*FPS;
	Coef->F8H = 28800*FPS;
	Coef->F16H = 57600*FPS;
	Coef->F24H = 115200*FPS;
	Coef->F48H = 234000*FPS;
	Coef->FMax = Coef->F48H;
}

unsigned int sumFrame( tlcde* Flies ){

	STFly* flyOut = NULL;
	float sumatorio = 0;
	unsigned int sumUint;
	for(int i = 0; i< Flies->numeroDeElementos ; i++){
		flyOut = (STFly*)obtener(i,Flies);
		sumatorio = flyOut->VInst + sumatorio;
	}
	sumUint = ( unsigned int)cvRound(sumatorio);
	return sumUint;
}

void mediaMovil(  STStatsCoef* Coef, tlcde* vector, STStatFrame* Stats ){

	valorSum* valor;
	valorSum* valorT;

	// Se suma el nuevo valor
	valorT = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 , vectorSumFr );
	Coef->F1SSum = Coef->F1SSum + valorT->sum;
	Coef->F1SSum2 = Coef->F1SSum2 + valorT->sum*valorT->sum;

	Coef->F30SSum = Coef->F30SSum + valorT->sum;
	Coef->F30SSum2 = Coef->F30SSum2 + valorT->sum*valorT->sum;

	Coef->F1Sum = Coef->F1Sum + valorT->sum;
	Coef->F1Sum2 = Coef->F1Sum + valorT->sum*valorT->sum;

	Coef->F5Sum = Coef->F5Sum + valorT->sum;
	Coef->F5Sum2 = Coef->F5Sum2 + valorT->sum*valorT->sum;

	Coef->F10Sum = Coef->F10Sum + valorT->sum;
	Coef->F10Sum2 = Coef->F10Sum2 +valorT->sum*valorT->sum;

	Coef->F30Sum = Coef->F30Sum + valorT->sum;
	Coef->F30Sum2 = Coef->F30Sum2 + valorT->sum*valorT->sum;

	Coef->F1HSum = Coef->F1HSum + valorT->sum;
	Coef->F1HSum2 = Coef->F1HSum2 + valorT->sum*valorT->sum;

	Coef->F2HSum = Coef->F2HSum + valorT->sum;
	Coef->F2HSum2 = Coef->F2HSum2 + valorT->sum*valorT->sum;

	Coef->F4HSum = Coef->F4HSum + valorT->sum;
	Coef->F4HSum2 = Coef->F4HSum2 + valorT->sum*valorT->sum;

	Coef->F8HSum = Coef->F48HSum + valorT->sum;
	Coef->F8HSum2 = Coef->F48HSum2 + valorT->sum*valorT->sum;

	if((unsigned)vector->numeroDeElementos > Coef->F1S){
	// los que hayan alcanzado el valor máximo restamos al sumatorio el primer valor
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F1S, vectorSumFr );
		Coef->F1SSum = Coef->F1SSum - valor->sum; // sumatorio para la media
		Coef->F1SSum2 = Coef->F1SSum2 - (valor->sum*valor->sum);  // sumatorio para la varianza
		Stats->CMov1SMed   =  Coef->F1SSum / Coef->F1S; // cálculo de la media
		Stats->CMov1SDes  =  sqrt( abs(Coef->F1SSum2 / Coef->F1S - Stats->CMov1SMed*Stats->CMov1SMed ) ); // cálculo de la desviación
	}
	if(vector->numeroDeElementos > Coef->F30S){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F30S, vectorSumFr );
		Coef->F30SSum = Coef->F30SSum - valor->sum; // sumatorio para la media
		Coef->F30SSum2 = Coef->F30SSum2 - (valor->sum*valor->sum);  // sumatorio para la varianza
		Stats->CMov30SMed  =  Coef->F30SSum / Coef->F30S;
		Stats->CMov30SDes  =  sqrt( abs(Coef->F30SSum2 / Coef->F30S - Stats->CMov30SMed*Stats->CMov30SMed ));
	}
	if(vector->numeroDeElementos > Coef->F1){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F1, vectorSumFr );
		Coef->F1Sum = Coef->F1Sum - valor->sum;
		Coef->F1Sum2 = Coef->F1Sum2 - (valor->sum*valor->sum);
		Stats->CMov1Med    =  Coef->F1Sum / Coef->F1;
		Stats->CMov1Des    =  sqrt( abs(Coef->F1Sum2 / Coef->F1 -  Stats->CMov1Med *Stats->CMov1Med )) ;
	}
	if(vector->numeroDeElementos > Coef->F5){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F5, vectorSumFr );
		Coef->F5Sum = Coef->F5Sum - valor->sum;
		Coef->F5Sum2 = Coef->F5Sum2 - (valor->sum*valor->sum);
		Stats->CMov5Med    =  Coef->F5Sum / Coef->F5;
		Stats->CMov5Des    =  sqrt( abs(Coef->F5Sum2 / Coef->F5 - Stats->CMov5Med *Stats->CMov5Med ));
	}
	if(vector->numeroDeElementos > Coef->F10){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F10, vectorSumFr );
		Coef->F10Sum = Coef->F10Sum - valor->sum;
		Coef->F10Sum2 = Coef->F10Sum2 - (valor->sum*valor->sum);
		Stats->CMov10Med   =  Coef->F10Sum / Coef->F10;
		Stats->CMov10Des   =  sqrt( abs(Coef->F10Sum2 / Coef->F10 - Stats->CMov10Med*Stats->CMov10Med));
	}
	if(vector->numeroDeElementos > Coef->F15){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F15, vectorSumFr );
		Coef->F15Sum = Coef->F15Sum - valor->sum;
		Coef->F15Sum2 = Coef->F15Sum2 - (valor->sum*valor->sum);
		Stats->CMov15Med   =  Coef->F15Sum / Coef->F15;
		Stats->CMov15Des   =  sqrt( abs(Coef->F15Sum2 / Coef->F15 - Stats->CMov15Med*Stats->CMov15Med));
	}
	if(vector->numeroDeElementos > Coef->F30){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F30, vectorSumFr );
		Coef->F30Sum = Coef->F30Sum - valor->sum;
		Coef->F30Sum2 = Coef->F30Sum2- (valor->sum*valor->sum);
		Stats->CMov30Med   =  Coef->F30Sum / Coef->F30;
		Stats->CMov30Des   =  sqrt( abs(Coef->F30Sum2 / Coef->F30 - Stats->CMov30Med*Stats->CMov30Med));
	}
	if(vector->numeroDeElementos > Coef->F1H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F1H, vectorSumFr );
		Coef->F1HSum = Coef->F1HSum - valor->sum;
		Coef->F1HSum2 = Coef->F1HSum2 - (valor->sum*valor->sum);
		Stats->CMov1HMed   =  Coef->F1HSum / Coef->F1H;
		Stats->CMov1HDes   =  sqrt( abs(Coef->F1HSum2 / Coef->F1H - Stats->CMov1HMed*Stats->CMov1HMed));
	}
	if(vector->numeroDeElementos > Coef->F2H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F2H, vectorSumFr );
		Coef->F2HSum = Coef->F2HSum - valor->sum;
		Coef->F2HSum2 = Coef->F2HSum2 - (valor->sum*valor->sum);
		Stats->CMov2HMed   =  Coef->F2HSum / Coef->F2H;
		Stats->CMov2HDes   =  sqrt( abs(Coef->F2HSum2 / Coef->F2H - Stats->CMov2HMed*Stats->CMov2HMed));
	}
	if(vector->numeroDeElementos > Coef->F4H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F4H, vectorSumFr );
		Coef->F4HSum = Coef->F4HSum - valor->sum;
		Coef->F4HSum2 = Coef->F4HSum2- (valor->sum*valor->sum);
		Stats->CMov4HMed   =  Coef->F4HSum / Coef->F4H;
		Stats->CMov4HDes   =  sqrt( abs(Coef->F4HSum2 / Coef->F4H - Stats->CMov4HMed*Stats->CMov4HMed));
	}
	if(vector->numeroDeElementos > Coef->F8H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F8H, vectorSumFr );
		Coef->F8HSum = Coef->F48HSum - valor->sum;
		Coef->F8HSum2 = Coef->F8HSum2 - (valor->sum*valor->sum);
		Stats->CMov8HMed   = Coef->F8HSum / Coef->F8H;
		Stats->CMov8HDes   =  sqrt( abs(Coef->F8HSum2 / Coef->F8H - Stats->CMov8HMed*Stats->CMov8HMed));
	}
	if(vector->numeroDeElementos > Coef->F16H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F16H, vectorSumFr );
		Coef->F16HSum = Coef->F16HSum - valor->sum;
		Coef->F16HSum2 = Coef->F16HSum2 - (valor->sum*valor->sum);
		Stats->CMov16HMed   = Coef->F16HSum / Coef->F16H;
		Stats->CMov16HDes  =  sqrt( abs(Coef->F16HSum2 / Coef->F16H - Stats->CMov16HMed*Stats->CMov16HMed));
	}
	if(vector->numeroDeElementos > Coef->F24H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F24H, vectorSumFr );
		Coef->F24HSum = Coef->F24HSum - valor->sum;
		Coef->F24HSum2 = Coef->F24HSum2 - (valor->sum*valor->sum);
		Stats->CMov24HMed   = Coef->F24HSum / Coef->F24H;
		Stats->CMov24HDes  =  sqrt( abs(Coef->F24HSum2 / Coef->F24H - Stats->CMov24HMed*Stats->CMov24HMed));
	}
	if(vector->numeroDeElementos > Coef->F48H){
		valor = ( valorSum*)obtener( vectorSumFr->numeroDeElementos-1 - Coef->F48H, vectorSumFr );
		Coef->F48HSum = Coef->F48HSum - valor->sum;
		Coef->F48HSum2 = Coef->F48HSum2 - (valor->sum*valor->sum);
		Stats->CMov48HMed   = Coef->F48HSum / Coef->F48H;
		Stats->CMov48HDes  =  sqrt( abs(Coef->F48HSum2 / Coef->F48H - Stats->CMov48HMed*Stats->CMov48HMed));
	}

}

void iniciarMedias( STStatFrame* Stats, unsigned int valor ){

	Coef->F30SSum = valor;
	Coef->F1Sum   = valor;
	Coef->F5Sum   = valor;
	Coef->F10Sum  = valor;
	Coef->F15Sum  = valor;
	Coef->F30Sum  = valor;
	Coef->F1HSum  = valor;
	Coef->F2HSum  = valor;
	Coef->F4HSum  = valor;
	Coef->F8HSum  = valor;
	Coef->F16HSum = valor;
	Coef->F24HSum = valor;
	Coef->F48HSum = valor;

	Coef->F30SSum2 = valor*valor;
	Coef->F1Sum2   = valor*valor;
	Coef->F5Sum2   = valor*valor;
	Coef->F10Sum2  = valor*valor;
	Coef->F15Sum2  = valor*valor;
	Coef->F30Sum2  = valor*valor;
	Coef->F1HSum2  = valor*valor;
	Coef->F2HSum2  = valor*valor;
	Coef->F4HSum2  = valor*valor;
	Coef->F8HSum2  = valor*valor;
	Coef->F16HSum2 = valor*valor;
	Coef->F24HSum2 = valor*valor;
	Coef->F48HSum2 = valor*valor;

}

void releaseStats(  ){
	// Borrar todos los elementos de la lista
	valorSum *valor = NULL;
	// Comprobar si hay elementos
	if(vectorSumFr == NULL || vectorSumFr->numeroDeElementos<1)  return;
	// borrar: borra siempre el elemento actual

	valor = (valorSum *)borrar(vectorSumFr);
	while (valor)
	{
		free(valor);
		valor = NULL;
		valor = (valorSum *)borrar(vectorSumFr);
	}
	free( Stats );
	free(Coef);
}
