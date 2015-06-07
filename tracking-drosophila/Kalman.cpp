/*
 * Kalman.cpp
 *
 *  Created on: 18/11/2011
 *      Authores: Rubén Chao Chao, Germán Garcia Vázquez
 *
 */


#include "Kalman.hpp"


CvMat* CoordReal;
CvMat* H ;

// componentes del vector Zk con las nuevas medidas
float x_Zk;
float y_Zk;
float vx_Zk;
float vy_Zk;
float phiZk;

// componentes del vector Rk de covarianzas. Incertidumbre en la medida: V->N( 0, Rk )
float R_x;
float R_y;
float R_Vx;
float R_Vy;
float R_phiZk;

// componentes del vector Xk. Predicciones
float x_Xk ;
float y_Xk ;
float vx_Xk ;
float vy_Xk ;
float phiXk ;

// variables para establecer la nueva medida
float Vx ;
float Vy ;
int Ax; //incremento de x
int Ay; //incremento de y
float distancia; // sqrt( Ax² + Ay² )

//Imagenes
IplImage* ImKalman = NULL;

void Kalman(STFrame* frameData, tlcde* lsIds,tlcde* lsTracks, int FPS) {

	tlcde* Flies;
	STTrack* Track;
	STFly* Fly = NULL;

	// cargar datos del frame
	Flies=frameData->Flies;

	// Inicializar imagenes y parámetros
	if( !ImKalman ){
		ImKalman = cvCreateImage(cvGetSize(frameData->ImKalman),8,3);
		cvZero(ImKalman);
		CoordReal = cvCreateMat(2,1,CV_32FC1);
	}
	cvZero(frameData->ImKalman);
	if( lsTracks->numeroDeElementos > 0){
		irAlPrincipio(lsTracks);
		for(int i = 0;i < lsTracks->numeroDeElementos ; i++){
			// obtener Track
			Track = (STTrack*)obtenerActual(lsTracks);
			// Esablece estado del track según haya no nueva/s medida/s
			Track->Stats->Estado = establecerEstado( Track, Track->Flysig );

			////////////////////// GENERAR MEDIDA ////////////////////
			// Generar la nueva medida en función del estado
			generarMedida( Track, Track->Stats->Estado );
			// Generar ruido asociado a la nueva medida
			generarRuido( Track, Track->Stats->Estado );

			////////////////////// FASE DE CORRECCIÓN ////////////////////////
			// corregir kalman
			if( Track->Stats->Estado != SLEEPING ){
				cvKalmanCorrect( Track->kalman, Track->z_k);
			}
			irAlSiguiente( lsTracks );
		}
		/////////////// ACTUALIZACIÓN DE TRACKS CON LOS NUEVOS DATOS //////////////
		updateTracks( lsTracks,frameData->Flies, FPS );
	}


	// CREAR NUEVOS TRACKS, si es necesario.
	// si hay mosca/s sin asignar a un track/s (id = -1), se crea un nuevo track para esa mosca
	if( Flies->numeroDeElementos > 0) irAlPrincipio( Flies );
	for(int i = 0; i< Flies->numeroDeElementos ;i++ ){
		Fly = (STFly*)obtenerActual( Flies );
		if( Fly->etiqueta == -1 ){
			Track = initTrack( Fly, lsIds , 1 );
			anyadirAlFinal( Track , lsTracks );
		}
		irAlSiguiente( Flies);
	}

	////////////////////// FASE DE PREDICCION ////////////////////////
	//Recorremos cada track y hacemos la predicción para el siguiente frame
	if( lsTracks->numeroDeElementos > 0) irAlPrincipio( lsTracks);
	for(int i = 0;i < lsTracks->numeroDeElementos ; i++){
		Track = (STTrack*)obtenerActual(lsTracks);
		if(Track->Stats->Estado != SLEEPING ){
			if( Track->x_k_Pre != NULL ){
				// guardamos los datos de la predicción anterior
				cvCopy(Track->x_k_Pre,Track->x_k_Pre_);
				cvCopy(Track->P_k_Pre,Track->P_k_Pre_);
			}
			// nueva predicción
			Track->x_k_Pre = cvKalmanPredict( Track->kalman, 0);
		}
		irAlSiguiente(lsTracks);
	}

	///////////////////// VISUALIZAR RESULTADOS ////////////////
	if( obtenerVisParam( SHOW_KALMAN ) ) {
		visualizarKalman( frameData, lsTracks );
	}



	return;
}// Fin de Kalman



// etiquetar fly e iniciar nuevo track

STTrack* initTrack( STFly* Fly ,tlcde* ids, float fps ){

	STTrack* Track = NULL;


	Track = (STTrack*)malloc(sizeof(STTrack));
	if(!Track) {error(4); exit(1);}

	asignarNuevaId( Track , ids );
	//TRACK
	// iniciar estadísticas
	Track->Stats = ( STStatTrack * )malloc( sizeof(STStatTrack ));
	if( !Track->Stats ) {error(4);exit(1);}

	Track->Stats->CMov1SMed = 0;  //!< Cantidad de movimiento medio mm/s.
	Track->Stats->CMov1MMed = 0;  //!< Cantidad de movimiento medio.mm/min
	Track->Stats->CMov1HMed = 0;  //!< Cantidad de movimiento medio.mm/h
	Track->Stats->TOn = 0;  //!< Tiempo en movimiento desde el inicio.
	Track->Stats->TOff = 0; //!< Tiempo parado desde el inicio.
	Track->Stats->SumatorioMed = 0;
	Track->Stats->VectorSumB = ( tlcde * )malloc( sizeof(tlcde ));
	if( !Track->Stats->VectorSumB ) {error(4);exit(1);}
	iniciarLcde( Track->Stats->VectorSumB );
	Track->Stats->InitTime =Fly->num_frame;
	Track->Stats->InitPos =  Fly->posicion;

	// añadir al final en Fly->Tracks el track

	Track->Stats->Estado = CAM_CONTROL;
	Track->Stats->EstadoCount = 1;
	Track->Stats->FrameCount = 1;
	Track->Stats->EstadoBlob = 1;
	Track->Stats->EstadoBlobCount = 0;
	Track->Stats->dstTotal = 0;

	// iniciamos parámetros del track
	// iniciar kalman
	Track->kalman = initKalman( Fly , fps);

	Track->x_k_Pre_ = cvCreateMat( 5,1, CV_32FC1 );
	Track->P_k_Pre_ = cvCreateMat( 5,5, CV_32FC1 );
	cvZero(Track->x_k_Pre_);
	cvZero(Track->P_k_Pre_);

	Track->x_k_Pos = Track->kalman->state_post ;
	Track->P_k_Pos = Track->kalman->error_cov_post;

	Track->x_k_Pre = NULL; // es creada por kalman
	Track->P_k_Pre = Track->kalman->error_cov_pre;

	Track->Medida = cvCreateMat( 5, 1, CV_32FC1 );
	Track->Measurement_noise = cvCreateMat( 5, 1, CV_32FC1 );
	Track->Measurement_noise_cov = Track->kalman->measurement_noise_cov;
	Track->z_k = cvCreateMat( 5, 1, CV_32FC1 );

	cvZero(Track->Medida);
	cvZero(Track->z_k );

	Fly->etiqueta = Track->id;
	Fly->Color = Track->Color;
	Fly->Estado = 1;
	Fly->Stats->EstadoBlobCount = Track->Stats->EstadoBlobCount;
	Fly->Stats->CMov1SMed = Track->Stats->CMov1SMed;
	Fly->Stats->EstadoTrack = Track->Stats->Estado;
	Fly->Stats->EstadoTrackCount = Track->Stats->EstadoCount;

	//Asignamos la fly al track
	Track->FlyActual = Fly;
	Track->Flysig = NULL;

	return Track;
	//		direccionR = (flyData->direccion*CV_PI)/180;
	// Calcular e iniciar estado

}

CvKalman* initKalman( STFly* Fly, float dt ){

	float Vx=0;// componente X de la velocidad.
	float Vy=0;// componente Y de la velociad.

	// Crear el flitro de Kalman

	CvKalman* Kalman = cvCreateKalman(5,5,0);

	// Inicializar las matrices parámetros para el filtro de Kalman
	// en la primera iteración iniciamos la dirección con la orientación

	Vx= VELOCIDAD*cos(Fly->orientacion*CV_PI/180);
	Vy=-VELOCIDAD*sin(Fly->orientacion*CV_PI/180);

	const float F[] = {1,0,dt,0,0, 0,1,0,dt,0, 0,0,1,0,0, 0,0,0,1,0, 0,0,0,0,1}; // Matriz de transición F
	// Matriz R inicial. Errores en medidas. ( Rx, Ry,RVx, RVy, Rphi ). Al inicio el error en el angulo es de +-180º
	const float R_inicial[] = {100,0,0,0,0, 0,100,0,0,0, 0,0,200,0,0, 0,0,0,200,0, 0,0,0,0,360};//{50,50,50,50,180};
	float X_k[] = { Fly->posicion.x, Fly->posicion.y, Vx, Vy, Fly->orientacion };// Matiz de estado inicial

	// establecer parámetros
	memcpy( Kalman->transition_matrix->data.fl, F, sizeof(F)); // F

	cvSetIdentity( Kalman->measurement_matrix,cvRealScalar(1) ); // H Matriz de medida
	cvSetIdentity( Kalman->process_noise_cov,cvRealScalar(1 ) ); // error asociado al modelo del sistema Q = f(F,B y H).
	cvSetIdentity( Kalman->error_cov_pre, cvScalar(1) ); // Incertidumbre en predicción. (P_k' = F P_k-1 Ft) + Q
	cvSetIdentity( Kalman->error_cov_post,cvRealScalar(1)); // Incertidumbre tras añadir medida. P_k = ( I - K_k H) P_k'

	memcpy( Kalman->measurement_noise_cov->data.fl, R_inicial, sizeof(R_inicial)); // Incertidumbre en la medida Vk; Vk->N(0,R)
//	cvSetIdentity( Kalman->error_cov_post,cvRealScalar(1)); // Incertidumbre tras añadir medida. P_k = ( I - K_k H) P_k'
	//copyMat(&x_k, Kalman->state_post);
	memcpy( Kalman->state_post->data.fl, X_k, sizeof(X_k)); // Estado inicial

	return Kalman;
}


//Establecer el estado del Track según haya o no medida/s. en caso de cambio de estado inicia los contadores de estado.
int establecerEstado( STTrack* Track, STFly* flySig ){


	if ( !flySig || !Track->FlyActual ){ // si no hay flysig o fly actual SLEEPING
		if(!flySig && Track->FlyActual){ // Si cambio de estado: iniciar contador de estado
			Track->Stats->EstadoCount = 0;
		}
		return SLEEPING;
	}
	else if( flySig->Tracks->numeroDeElementos == 1 ){
		if( Track->Stats->Estado != CAM_CONTROL){
			Track->Stats->EstadoCount = 0; // Cambio de estado
		}
		return CAM_CONTROL;
	}

	else if(flySig->Tracks->numeroDeElementos > 1){
		if( Track->Stats->Estado != KALMAN_CONTROL){
			Track->Stats->EstadoCount = 0;
		}
		return KALMAN_CONTROL;
	}
	else return -1;

}


void generarMedida( STTrack* Track, int EstadoTrack ){

	STFly* flyActual = Track->FlyActual;
	STFly* flySig = Track->Flysig;

	//Caso 0: No se ha encontrado ninguna posible asignación. No hay medida.
	if ( EstadoTrack == SLEEPING )	return ;

	x_Zk = flySig->posicion.x;
	y_Zk = flySig->posicion.y;

	Ax = x_Zk - flyActual->posicion.x;
	Ay = y_Zk - flyActual->posicion.y;

	vx_Zk = Ax / 1;
	vy_Zk = Ay / 1;

	//Establecemos la dirección phi y el modulo del vector de desplazamiento
	EUDistance( vx_Zk, vy_Zk, &phiZk, &distancia );
	//Caso 1: Se ha encontrado la siguiente posición. la medida se genera a partir de los datos obtenidos de los sensores
	if( EstadoTrack == CAM_CONTROL ) {

		// si no hay movimiento la dirección es la orientación.( o la dirección anterior? o la filtrada??? )
		if( distancia == 0 ){
			// si no se ha el minimo error medio
			//phiZk = flyActual->dir_filtered;
			//else
			phiZk = flyActual->orientacion;
		}
		else{
			// CORREGIR DIRECCIÓN.
			//Sumar ó restar n vueltas para que la dif sea siempre <=180
			phiXk = Track->x_k_Pre->data.fl[4]; // valor filtrado de la direccion

			if( abs(phiXk - phiZk ) > 180 ){
				int n = (int)phiXk/360;
				if( n == 0) n = 1;
				if( phiXk > phiZk ) phiZk = phiZk + n*360;
				else phiZk = phiZk - n*360;
			}
		}

	}
	//Caso 2:Un blob contiene varias flies.La medida se genera a partir de las predicciones de kalman
		// La incertidumbre en la medida de posición será más elevada
	else if( EstadoTrack == KALMAN_CONTROL ){
		phiZk = flySig->orientacion;

	}

	// ESTABLECER Z_K
	H = Track->kalman->measurement_matrix;

	const float Zk[] = { x_Zk, y_Zk, vx_Zk, vy_Zk, phiZk };
	memcpy( Track->Medida->data.fl, Zk, sizeof(Zk)); // Medida;

	float V[] = {0,0,0,0,0}; // media del ruido
	memcpy( Track->Measurement_noise->data.fl, V, sizeof(V));

	cvGEMM(H, Track->Medida,1, Track->Measurement_noise, 1, Track->z_k,0 ); // Zk = H Medida + V
	Track->VInst = distancia; // para las estadisticas
	return ;
}

void generarRuido( STTrack* Track, int EstadoTrack ){

	//0) No hay nueva medida
	if (EstadoTrack == SLEEPING ) return ;

	// GENERAR RUIDO.
	// 1) Caso normal. Un blob corresponde a un fly
	if (EstadoTrack == CAM_CONTROL ) {
		//1) Posición
		R_x = 1;	// implementar método de cuantificación del ruido.
		R_y = R_x;

		//2) Velocidad
		R_Vx = 2*R_x;
		R_Vy = 2*R_y;

		// 3) Dirección
		R_phiZk = generarR_PhiZk( );
	}
	//1) Varias flies en un mismo blob
	else if( EstadoTrack == KALMAN_CONTROL ){

		R_x = Track->Flysig->Roi.width/2;
		R_y = Track->Flysig->Roi.height/2;
		R_Vx = 2*R_x;
		R_Vy = 2*R_y;
		R_phiZk = 1000;
	}
	// INSERTAR RUIDO en kalman
	// cargamos el error en la medida en el filtro para calcular la ganancia de kalman
	float R[] = { R_x,0,0,0,0, 0,R_y,0,0,0, 0,0,R_Vx,0,0, 0,0,0,R_Vy,0, 0,0,0,0,R_phiZk};//covarianza del ruido
	memcpy( Track->kalman->measurement_noise_cov->data.fl, R, sizeof(R)); // V->N(0,R);

	return ;
}

void EUDistance( int a, int b, float* direccion, float* distancia){

	float phi;
	if( a == 0){
		if( b == 0 ) phi = -1; // No hay movimiento.
		if( b > 0 ) phi = 270; // Hacia abajo.
		if( b < 0 ) phi = 90; // Hacia arriba.
	}
	else if ( b == 0) {
		if (a < 0) phi = 180; // hacia la izquierda.
		else phi = 0;			// hacia la derecha.
	}
	else	phi = atan2( (float)-b , (float)a )* 180 / CV_PI;
	// calcular modulo del desplazamiento.
	*distancia = sqrt( pow( a ,2 )  + pow( b ,2 ) ); // sqrt( a² + b² )

	if( direccion == NULL ) return;
	else *direccion = phi;
}

// Al inicio queremos una convergencia rápida de la dirección. En las primeras iteraciones
// penalizaremos en función del error medio entre phiZK y phiXK ( si fuese el error instantáneo se vería
// muy afectado por el ruido). Inicialmente cuanto mayor sea el error absoluto medio menor será la penalización. A medida que se reduzca
// dicho error las penalizaciónes se irán incrementando hasta tomar los valores normales.
// menor penalización. Una vez que la diferencia media entre phiZK y phiXK se ha reducido
// lo suficiente,

float generarR_PhiZk( ){
	// Error debido a la velocidad. Esta directamente relaccionada con la resolución

	float phiDif; // error debido a la diferencia entre el nuevo valor y el filtrado
	float errorV; // incertidumbre en la dirección debido a la velocidad
	float errIz; // error debido a la velocidad por la izqd del ángulo
	float errDr; // error debido a la velocidad por la drcha del ángulo


	errIz = atan2( abs(vy_Zk), abs( vx_Zk) - 0.5 )  - atan2( abs(vy_Zk) , abs( vx_Zk) );
	errDr = atan2( abs(vy_Zk) , abs( vx_Zk) ) - atan2((abs(vy_Zk) - 0.5) , abs( vx_Zk) );

	if (errIz >= errDr ) errorV = errIz*180/CV_PI;
	else errorV = errDr*180/CV_PI;

	// Error debido a la diferencia entre el valor corregido y el medido. Si es mayor de PI/2 penalizamos
	phiDif = abs( phiXk - phiZk );

	if( phiDif > 90 ) {

		if ( vx_Zk >= 2 || vy_Zk >= 2) phiDif = phiDif/4; // si la velocidad es elevada, penalizamos poco
												//a pesar de ser mayor de 90
		return phiDif + errorV; // el error es la suma del error devido a la velocidad y a la diferencia
	}
	else {

		if ( vx_Zk <=1 && vy_Zk <= 1) errorV = 90; // si la velocidad es de 1 pixel penalizamos al máximo
		else if ( vx_Zk <=2 && vy_Zk <= 2) errorV = 45; // si es de dos pixels penalizamos con 45º

		return errorV; // si la dif es menor a 90 el error es el debido a la velocidad
	}
}

float corregirTita( float phiXk, float tita ){

	if( abs(phiXk - tita ) > 180 ){
		int n = (int)phiXk/360;
		if( n == 0) n = 1;
		if( phiXk > tita ) tita = tita + n*360;
		else tita = tita - n*360;
	}

	if( abs(phiXk - tita ) > 90 ){
		if( phiXk > tita ) tita = tita + 180;
		else tita = tita - 180;
	}
	return tita;
}

void visualizarKalman( STFrame* frameData, tlcde* lsTracks) {

	STTrack* Track;


		for(int i = 0; i < lsTracks->numeroDeElementos ; i++){
			Track = (STTrack*)obtener(i, lsTracks);
			// EN CONSOLA
			if(SHOW_KALMAN_DATA ){
				printf("\n*********************** FRAME %d; BLOB NUMERO %d ************************",frameData->num_frame,Track->id);
				showKalmanData( Track );
			}
			if( obtenerVisParam( SHOW_KALMAN ) ){
				double magnitude = 30;
				// EN IMAGEN
				cvCircle(ImKalman,cvPoint(cvRound(Track->z_k->data.fl[0]),cvRound(Track->z_k->data.fl[1] )),1,CVX_BLUE,-1,8); // observado
				cvLine( frameData->ImKalman,
					cvPoint( Track->z_k->data.fl[0],Track->z_k->data.fl[1]),
					cvPoint( cvRound( Track->z_k->data.fl[0] + magnitude*cos(Track->z_k->data.fl[4]*CV_PI/180)),
							 cvRound( Track->z_k->data.fl[1] - magnitude*sin(Track->z_k->data.fl[4]*CV_PI/180))  ),
					CVX_RED,
					1, CV_AA, 0 );
				cvCircle(ImKalman,cvPoint(cvRound(Track->x_k_Pre_->data.fl[0]),cvRound(Track->x_k_Pre_->data.fl[1] )),1,CVX_RED,-1,8); // predicción en t
				cvCircle(ImKalman,cvPoint(cvRound(Track->x_k_Pos->data.fl[0]),cvRound(Track->x_k_Pos->data.fl[1])),1,CVX_WHITE,-1,8); // Corrección en t
				// dirección de kalman (dirección fliltrada/corregida )
				cvLine(frameData->ImKalman,
						cvPoint( Track->x_k_Pos->data.fl[0],Track->x_k_Pos->data.fl[1]),
					cvPoint( cvRound( Track->x_k_Pos->data.fl[0] + magnitude*cos(Track->x_k_Pos->data.fl[4]*CV_PI/180)),
							 cvRound( Track->x_k_Pos->data.fl[1] - magnitude*sin(Track->x_k_Pos->data.fl[4]*CV_PI/180))  ),
					CVX_GREEN,
					1, CV_AA, 0 );
				cvCircle(ImKalman,cvPoint(cvRound(Track->x_k_Pre->data.fl[0]),cvRound(Track->x_k_Pre->data.fl[1])),1,CVX_GREEN,-1,8); // predicción t+1

				visualizarId( frameData->ImKalman,cvPoint( Track->z_k->data.fl[0],Track->z_k->data.fl[1]), Track->id, CVX_WHITE);

			}

		}
		if( obtenerVisParam( SHOW_KALMAN ) ) cvAdd(ImKalman,frameData->ImKalman,frameData->ImKalman );



}

void showKalmanData( STTrack *Track){

	printf("\n\nReal State: ");
	printf("\n\tCoordenadas: ( %f , %f )",Track->Medida->data.fl[0],Track->Medida->data.fl[1]);
	printf("\n\tVelocidad: ( %f , %f )",Track->Medida->data.fl[2],Track->Medida->data.fl[3]);
	printf("\n\tDirección:  %f ",Track->Medida->data.fl[4]);

	printf("\n\n Observed State: "); // sumandole el error de medida.
	printf("\n\tCoordenadas: ( %f , %f )",Track->z_k->data.fl[0],Track->z_k->data.fl[1]);
	printf("\n\tVelocidad: ( %f , %f )",Track->z_k->data.fl[2],Track->z_k->data.fl[3]);
	printf("\n\tDirección:  %f ",Track->z_k->data.fl[4]);

	printf("\n Media Measurement noise:\n\n\t%0.f\n\t%0.f\n\t%0.f\n\t%0.f\n\t%0.f",
			Track->Measurement_noise->data.fl[0],
			Track->Measurement_noise->data.fl[1],
			Track->Measurement_noise->data.fl[2],
			Track->Measurement_noise->data.fl[3],
			Track->Measurement_noise->data.fl[4]);

	printf("\n Error Process noise:\n\n\t%0.3f\t0\t0\t0\t0\n\t0\t%0.3f\t0\t0\t0\n\t0\t0\t%0.3f\t0\t0\n\t0\t0\t0\t%0.3f\t0\n\t0\t0\t0\t0\t%0.3f",
			Track->Measurement_noise_cov->data.fl[0],
			Track->Measurement_noise_cov->data.fl[6],
			Track->Measurement_noise_cov->data.fl[12],
			Track->Measurement_noise_cov->data.fl[18],
			Track->Measurement_noise_cov->data.fl[24]);

	printf("\n Error Process noise:\n\n\t%0.3f\t0\t0\t0\t0\n\t0\t%0.3f\t0\t0\t0\n\t0\t0\t%0.3f\t0\t0\n\t0\t0\t0\t%0.3f\t0\n\t0\t0\t0\t0\t%0.3f",
			Track->kalman->process_noise_cov->data.fl[0],
			Track->kalman->process_noise_cov->data.fl[6],
			Track->kalman->process_noise_cov->data.fl[12],
			Track->kalman->process_noise_cov->data.fl[18],
			Track->kalman->process_noise_cov->data.fl[24]);


	printf("\n\n Predicted State t:");
	printf("\n\tCoordenadas: ( %f , %f )",Track->x_k_Pre_->data.fl[0],Track->x_k_Pre_->data.fl[1]);
	printf("\n\tVelocidad: ( %f , %f )",Track->x_k_Pre_->data.fl[2],Track->x_k_Pre_->data.fl[3]);
	printf("\n\tDirección:  %f ",Track->x_k_Pre_->data.fl[4]);
	printf("\n Error Cov Predicted:\n\n\t%0.3f\t0\t0\t0\t0\n\t0\t%0.3f\t0\t0\t0\n\t0\t0\t%0.3f\t0\t0\n\t0\t0\t0\t%0.3f\t0\n\t0\t0\t0\t0\t%0.3f",
			Track->P_k_Pre_->data.fl[0],
			Track->P_k_Pre_->data.fl[6],
			Track->P_k_Pre_->data.fl[12],
			Track->P_k_Pre_->data.fl[18],
			Track->P_k_Pre_->data.fl[24]);

	printf("\n\n Corrected State:" );
	printf("\n\tCoordenadas: ( %f , %f )",Track->x_k_Pos->data.fl[0],Track->x_k_Pos->data.fl[1]);
	printf("\n\tVelocidad: ( %f , %f )",Track->x_k_Pos->data.fl[2],Track->x_k_Pos->data.fl[3]);
	printf("\n\tDirección:  %f ",Track->x_k_Pos->data.fl[4]);
	printf("\n Error Cov Corrected:\n\n\t%0.3f\t0\t0\t0\t0\n\t0\t%0.3f\t0\t0\t0\n\t0\t0\t%0.3f\t0\t0\n\t0\t0\t0\t%0.3f\t0\n\t0\t0\t0\t0\t%0.3f",
			Track->P_k_Pos->data.fl[0],
			Track->P_k_Pos->data.fl[6],
			Track->P_k_Pos->data.fl[12],
			Track->P_k_Pos->data.fl[18],
			Track->P_k_Pos->data.fl[24]);

	printf("\n\n Predicted State t+1:");
	printf("\n\tCoordenadas: ( %f , %f )",Track->x_k_Pre->data.fl[0],Track->x_k_Pre->data.fl[1]);
	printf("\n\tVelocidad: ( %f , %f )",Track->x_k_Pre->data.fl[2],Track->x_k_Pre->data.fl[3]);
	printf("\n\tDirección:  %f ",Track->x_k_Pre->data.fl[4]);
	printf("\n Error Cov Predicted:\n\n\t%0.3f\t0\t0\t0\t0\n\t0\t%0.3f\t0\t0\t0\n\t0\t0\t%0.3f\t0\t0\n\t0\t0\t0\t%0.3f\t0\n\t0\t0\t0\t0\t%0.3f",
			Track->P_k_Pre->data.fl[0],
			Track->P_k_Pre->data.fl[6],
			Track->P_k_Pre->data.fl[12],
			Track->P_k_Pre->data.fl[18],
			Track->P_k_Pre->data.fl[24]);
	printf("\n\n");
}
//void ReInitTrack( STTrack* Track, STFly* Fly , float dt ){
//
//	float Vx=0;// componente X de la velocidad.
//	float Vy=0;// componente Y de la velociad.
//	float dist;
//	float phi;
//
//	EUDistance( Fly->posicion.x-Track->z_k->data.fl[0] ,Fly->posicion.x-Track->z_k->data.fl[1], &phi, &dist );
//
//	Track->FlyActual = Fly;
//	Track->Flysig = NULL;
//	Track->dstTotal = Track->dstTotal + dist;
//
//	Fly->etiqueta = Track->id;
//	Fly->Color = Track->Color;
//	Fly->dstTotal = Track->dstTotal ;
//
//	Track->Stats->Estado = CAM_CONTROL;
//	Track->EstadoCount = 1;
//	Track->FrameCount = Track->FrameCount + 1;
//
//	Track->EstadoBlob = 0;
//
//	Vx= VELOCIDAD*cos(Fly->orientacion*CV_PI/180);
//	Vy=-VELOCIDAD*sin(Fly->orientacion*CV_PI/180);
//
//	const float F[] = {1,0,dt,0,0, 0,1,0,dt,0, 0,0,1,0,0, 0,0,0,1,0, 0,0,0,0,1}; // Matriz de transición F
//	// Matriz R inicial. Errores en medidas. ( Rx, Ry,RVx, RVy, Rphi ). Al inicio el error en el angulo es de +-180º
//	const float R_inicial[] = {1,0,0,0,0, 0,1,0,0,0, 0,0,2,0,0, 0,0,0,2,0, 0,0,0,0,360};//{50,50,50,50,180};
//	const float X_k[] = { Fly->posicion.x, Fly->posicion.y, Vx, Vy, Fly->orientacion };// Matiz de estado inicial
//	float initialState[] = {Fly->posicion.x, Fly->posicion.y, Vx, Vy, Fly->orientacion};
//
//	CvMat x_k =cvMat(1, 5, CV_32FC1, initialState);
//
//	// establecer parámetros
//	memcpy( Track->kalman->transition_matrix->data.fl, F, sizeof(F)); // F
//
//	cvSetIdentity( Track->kalman->measurement_matrix,cvRealScalar(1) ); // H Matriz de medida
//	cvSetIdentity( Track->kalman->process_noise_cov,cvRealScalar(1 ) ); // error asociado al modelo del sistema Q = f(F,B y H).
//	cvSetIdentity(Track->kalman->error_cov_pre, cvScalar(1) ); // Incertidumbre en predicción. (P_k' = F P_k-1 Ft) + Q
//	cvSetIdentity( Track->kalman->error_cov_post,cvRealScalar(1)); // Incertidumbre tras añadir medida. P_k = ( I - K_k H) P_k'
//
//	memcpy( Track->kalman->measurement_noise_cov->data.fl, R_inicial, sizeof(R_inicial)); // Incertidumbre en la medida Vk; Vk->N(0,R)
////	cvSetIdentity( Kalman->error_cov_post,cvRealScalar(1)); // Incertidumbre tras añadir medida. P_k = ( I - K_k H) P_k'
//	//copyMat(&x_k, Kalman->state_post);
//	memcpy( Track->kalman->state_post->data.fl, X_k, sizeof(X_k)); // Estado inicial
//
//	Track->x_k_Pre = NULL; // es creada por kalman
//	cvZero(Track->Medida);
//	cvZero(Track->z_k );
//	cvZero(Track->x_k_Pre_);
//	cvZero(Track->P_k_Pre_);
//}
void updateTracks( tlcde* lsTracks,tlcde* Flies, int FPS ){

	STTrack* Track;

	if( lsTracks->numeroDeElementos < 1) return;
	irAlPrincipio( lsTracks);
	for(int i = 0;i < lsTracks->numeroDeElementos ; i++){
		// obtener Track
		Track = (STTrack*)obtenerActual( lsTracks);
		// ACTUALIZAR TRACK
		// actualizar estadísticas del track en base a su estado
		updateStatsTrack( Track, FPS );
		// ETIQUETAR Y ACTUALIZAR FLY DEL TRACK
		// en caso de Kalman_control se crea una mosca para simular la posición
		updateFlyTracked( Track , Flies);

		if ( Track->Stats->Estado == SLEEPING){
			Track->FlyActual = NULL;
			Track->Flysig = NULL;
		}
		else{
			// Actualizar fly actual.
			Track->FlyActual->siguiente = (STFly*)Track->Flysig;
			// fly actual pasa a ser fly siguiente.
			Track->FlyActual = Track->Flysig;
			Track->Flysig = NULL;
		}
		irAlSiguiente( lsTracks);
	}
}

void updateStatsTrack( STTrack* Track, int FPS ){

	valorSumB* valor;

	if ( Track->Stats->Estado == SLEEPING){
		Track->Stats->FrameCount +=1;
		Track->Stats->EstadoCount += 1;
		return;
	}
	// para el caso kalman control, establecemos la distancia en base a lo pos corregida
	if( Track->Stats->Estado == KALMAN_CONTROL){

		float Ax = Track->x_k_Pos->data.fl[0] - Track->FlyActual->posicion.x;
		float Ay = Track->x_k_Pos->data.fl[1] - Track->FlyActual->posicion.y;

		//Establecemos la dirección phi y el modulo del vector de desplazamiento
		EUDistance( Ax, Ay, NULL, &Track->VInst );
	}
	Track->Stats->dstTotal = Track->Stats->dstTotal + Track->VInst;
	Track->Stats->EstadoCount += 1;
	Track->Stats->FrameCount ++;

	// calculamos la velocidad en 1 seg
	  // al ser t= 1 la distancia coincide con Vt
	// Obtener el nuevo valor
	valor =  ( valorSumB *) malloc( sizeof(valorSumB));
	valor->sum = Track->VInst;
	anyadirAlFinal( valor, Track->Stats->VectorSumB );
//	calculo de la velocidad media de los  frames equivalentes a 1 seg.( 30 frames )
	mediaMovilFPS( Track->Stats, FPS );
	// si el vector llega a fps elementos eliminamos el primero.
	if( Track->Stats->VectorSumB->numeroDeElementos > FPS){
		valor = (valorSumB*)liberarPrimero(  Track->Stats->VectorSumB );
		free(valor);
	}

//	mostrarVMedia(Track->Stats->VectorSumB);
	// en función de la velocidad anterior establecemos el estado del blob
	if( Track->Stats->CMov1SMed > 1 ) {
		if( Track->Stats->EstadoBlob == IN_BG){
			Track->Stats->EstadoBlobCount = 0;
		}
		Track->Stats->EstadoBlob = IN_FG;
		Track->Stats->EstadoBlobCount += 1;
	}
	else{
		if( Track->Stats->EstadoBlob == IN_FG){
			Track->Stats->EstadoBlobCount = 0;
		}
		Track->Stats->EstadoBlob = IN_BG;
		Track->Stats->EstadoBlobCount += 1;
	}

}

void updateFlyTracked( STTrack* Track, tlcde* Flies ){

	if( Track->Stats->Estado == CAM_CONTROL ){

		Track->Flysig->etiqueta = Track->id;
		Track->Flysig->Color = Track->Color;

		Track->Flysig->direccion = Track->z_k->data.fl[4];
		Track->Flysig->dir_filtered =Track->x_k_Pos->data.fl[4];
		Track->Flysig->orientacion = corregirTita( Track->Flysig->dir_filtered, Track->Flysig->orientacion );
		Track->Flysig->VInst = Track->VInst;
		Track->Flysig->dstTotal = Track->Stats->dstTotal;
		Track->Flysig->Estado = Track->Stats->EstadoBlob;

		Track->Flysig->Stats->EstadoBlobCount = Track->Stats->EstadoBlobCount;
		Track->Flysig->Stats->CMov1SMed = Track->Stats->CMov1SMed;
		Track->Flysig->Stats->EstadoTrack = Track->Stats->Estado;
		Track->Flysig->Stats->EstadoTrackCount = Track->Stats->EstadoCount;
	}
	if( Track->Stats->Estado ==  KALMAN_CONTROL){
		// actualizar fly ( Eliminarla??? )
		Track->Flysig->etiqueta = 0;
		Track->Flysig->Color = cvScalar( 255, 255, 255);

		Track->Flysig->direccion = Track->Flysig->orientacion;
		Track->Flysig->dstTotal = 0;

		Track->Flysig->Stats = NULL;  /// todo mirar esto

		generarFly( Track,Flies);

	}
}
/*!brief Genera flies ficticias en el buffer para los tracks que están en estado KALMAN_CONTROL ( prediccion )
 *
 *
 * @param lsTracks
 * @param Flies
 */
void generarFly( STTrack* Track, tlcde* Flies ){

		STFly* fly = NULL;
		fly = ( STFly *) malloc( sizeof( STFly));
		if ( !fly ) {error(4);	exit(1);}
		fly->Tracks = NULL;
		fly->Stats = ( STStatFly * )malloc( sizeof(STStatFly ));
		iniciarLcde( fly->Tracks );
		anyadirAlFinal( Track, fly->Tracks);
		fly->siguiente = NULL;

		fly->etiqueta = Track->id; // Identificación del blob
		fly->Color = Track->Color; // Color para dibujar el blob
		fly->a = Track->Flysig->a;
		fly->b = Track->Flysig->b;

		fly->direccion = Track->x_k_Pos->data.fl[4]; // la dirección es la filtrada
		fly->dir_filtered = fly->direccion;
		fly->orientacion = fly->direccion;

		fly->dstTotal = Track->Stats->dstTotal;
	//		fly->perimetro = cv::arcLength(contorno,0);
//		fly->Roi = rect;
		  // Flag para indicar que si el blob permanece estático ( 0 ) o en movimiento (1) u oculta (2)
		fly->num_frame = Track->Flysig->num_frame;
		fly->salto = false;	//!< Indica que la mosca ha saltado

		fly->Zona = 0; //!< Si se seleccionan zonas de interes en el plato,
		fly->failSeg = false;
		fly->flag_seg = false;
		fly->Px = 0;
		fly->areaElipse = CV_PI*fly->b*fly->a;
		// estadisticas
		fly->dstTotal = Track->Stats->dstTotal;
		fly->Estado = Track->Stats->EstadoBlob;
		fly->Stats->EstadoTrack = Track->Stats->Estado;
		fly->Stats->EstadoTrackCount = Track->Stats->EstadoCount;
		fly->Stats->EstadoBlobCount = Track->Stats->EstadoBlobCount;
		fly->Stats->CMov1SMed = Track->Stats->CMov1SMed;
		fly->VInst = Track->VInst;

		Track->Flysig = fly;
		anyadirAlFinal(( STFly*) fly, Flies );
}


void mediaMovilFPS( STStatTrack* Stats, int FPS ){

	valorSumB* valor;
	valorSumB* valorT;

	// Se suma el nuevo valor
	irAlFinal(Stats->VectorSumB);
	valorT = ( valorSumB*)obtenerActual( Stats->VectorSumB );
	Stats->SumatorioMed = Stats->SumatorioMed + valorT->sum;

	if(Stats->VectorSumB->numeroDeElementos > FPS){
	// los que hayan alcanzado el valor máximo restamos al sumatorio el primer valor
		irAlPrincipio( Stats->VectorSumB);
		valor = ( valorSumB*)obtenerActual(  Stats->VectorSumB );
		Stats->SumatorioMed =Stats->SumatorioMed - valor->sum; // sumatorio para la media
		Stats->CMov1SMed   =  Stats->SumatorioMed / FPS; // cálculo de la media
	}
}

void mostrarVMedia( tlcde* Vector){

	valorSumB* valor;
 	irAlFinal(Vector);
 	printf("\n");
 	for(int i = 0; i <Vector->numeroDeElementos ; i++ ){
 		valor = (valorSumB*)obtener(i, Vector);
 		printf("valor = %0.f\n", valor->sum);
 	}
 }

int deadTrack( tlcde* Tracks, int id ){

	STTrack* Track = NULL;
	STStatTrack* Stats= NULL;
	valorSumB* val = NULL;
	Track = (STTrack*)borrarEl( id , Tracks);
	if(Track) {
		// matrices de kalman
		cvReleaseKalman(&Track->kalman);
		cvReleaseMat( &Track->x_k_Pre_ );
		cvReleaseMat( &Track->P_k_Pre_ );
		cvReleaseMat( &Track->Medida );
		cvReleaseMat(&Track->z_k );
		cvReleaseMat(&Track->Measurement_noise );
		if(Track->Stats->VectorSumB->numeroDeElementos > 0){
			val = (valorSumB *)borrar(Track->Stats->VectorSumB);
			while (val)
			{
				free(val);
				val = NULL;
				val = (valorSumB *)borrar(Track->Stats->VectorSumB);
			}
		}
		free( Track->Stats);

		free(Track);
		Track = NULL;
		return 1;
	}
	else return 0;
}


void liberarTracks( tlcde* lista){
	// Borrar todos los elementos de la lista
	// Comprobar si hay elementos
	int track;
	if(lista == NULL || lista->numeroDeElementos<1)  return;
	track = deadTrack( lista, 0);
	while( track) track = deadTrack( lista, 0);

}

int dejarId( STTrack* Track, tlcde* identities ){
	Identity *Id = NULL;
	Id = ( Identity* )malloc( sizeof(Identity ));
	if( !Id){error(4);return 0 ;}
	Id->etiqueta = Track->id;
	Id->color = Track->Color;
	anyadirAlFinal( Id , identities );
	return 1;
}

void asignarNuevaId( STTrack* Track, tlcde* identities){
	Identity *id;
	id = (Identity* )borrarEl( identities->numeroDeElementos - 1, identities);
	Track->id = id->etiqueta;
	Track->Color = id->color;
	free(id);
	id = NULL;
}
/// Limpia de la memoria las imagenes usadas durante la ejecución
void DeallocateKalman( tlcde* lista ){

	cvReleaseImage( &ImKalman);

	cvReleaseMat( &CoordReal);

	liberarTracks( lista);

}

void copyMat (CvMat* source, CvMat* dest){

	int i,j;

	for (i=0; i<source->rows; i++)
		for (j=0; j<source->cols;j++)
			dest->data.fl[i*source->cols+j]=source->data.fl[i*source->cols+j];

}
