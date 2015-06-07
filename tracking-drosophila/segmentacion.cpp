/*
 * segmentacion2.cpp
 *
 *  Created on: 13/09/2011
 *      Author: german
 *
 *  Cada blob se ajusta mediante una elipse. Los parámetros de la elipse se obtienen a partir de la
 *  gaussiana 2d del blob mediante la eigendescomposición de su matriz de covarianza. Los parámetros
 *  de la gaussiana guardan proporción con el peso de cada pixel siendo este peso proporcional
 *  a la distancia normalizada de la intensidad del pixel y el valor probable del fondo de ese pixel,
 *   de modo que el centro de la elipse conicidirá con el valor esperado de la gausiana 2d (la región
 *   más oscura) que a su vez coincidirá aproximadamente con el centro del blob.
 *  Una vez hecho el ajuste se rellena una estructura donde se almacenan los parámetros que definen
 *  las características del blob ( identificacion, posición, orientación, semieje a, semieje b y roi
 *  entre otras).
 */


// Segmentación optimizada para modelo de mediana simple

#include "segmentacion.hpp"

IplImage *FGTemp = 0;
IplImage *IDif = 0;
IplImage *IDifF = 0;
IplImage *pesos = 0;

CvMat *vector_u; // Matriz de medias
CvMat *vector_resta; // (pi-u)
CvMat *matrix_mul;// Matriz (pi-u)(pi-u)T
CvMat *MATRIX_C;// MATRIZ DE COVARIANZA
CvMat *evects;// Matriz de EigenVectores
CvMat *evals;// Matriz de EigenValores

CvMat *Diagonal; // Matriz Diagonal,donde se extraen los ejes
CvMat *R;// Matriz EigenVectores.
CvMat *RT;

CvMemStorage* storage;
CvSeq* first_contour;

tlcde* segmentacion2( IplImage *Brillo, IplImage* BGModel, IplImage* FG ,CvRect Roi,IplImage* Mask){

	extern double NumFrame;
	struct timeval ti, tf; // iniciamos la estructura
	float tiempoParcial;
	//Iniciar lista para almacenar las moscas
	tlcde* flies = NULL;
	flies = ( tlcde * )malloc( sizeof(tlcde ));
	iniciarLcde( flies );

	// crear imagenes y datos si no se ha hecho e inicializar
	CreateDataSegm( Brillo );
	cvCopy( FG , FGTemp);
	cvSetImageROI( Brillo , Roi);
	cvSetImageROI( BGModel, Roi );
	cvSetImageROI( FG, Roi );
	cvSetImageROI( FGTemp, Roi );
	cvSetImageROI( pesos, Roi );
	cvZero( Mask);


//		cvShowImage("Foreground",FGTemp);
//				cvWaitKey(0);
#ifdef MEDIR_TIEMPOS
	gettimeofday(&ti, NULL);
#endif

	//////  DISTANCIA DE CADA PIXEL A SU MODELO DE FONDO.
	cvAbsDiff(Brillo,BGModel,pesos);// |I(p)-u(p)|

	if( SHOW_SEGMENTATION_MATRIX == 1){
				printf("\n ANTES \n");

				CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
				//ventana = rect;
				printf("\n\n Matriz Brillo ");
				verMatrizIm(Brillo, ventana);
				printf("\n\n Matriz BGModel ");
				verMatrizIm(BGModel, ventana);
				printf("\n\n Matriz pesos(p) = |I(p)-u(p)|/0(p) ");
				verMatrizIm(pesos, ventana);
			}

	//Buscamos los contornos de las moscas en movimiento en el foreground

	int Nc = cvFindContours(FGTemp,
			storage,
			&first_contour,
			sizeof(CvContour),
			CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_NONE,
			cvPoint(Roi.x,Roi.y));
	int id=0; // contador de blobs
	if( SHOW_SEGMENTATION_DATA ) printf( "\nTotal Contornos Detectados: %d ", Nc );

	for( CvSeq *c=first_contour; c!=NULL; c=c->h_next) {
		// Parámetros para calcular el error del ajuste en base a la diferencia de areas entre el Fg y la ellipse
		//Inicializar estructura para almacenar los datos cada mosca
		STFly *flyData = NULL;
		float err;
		float areaFG;
		float areaElipse;
		areaFG = cvContourArea(c);
		id++;

		CvRect rect=cvBoundingRect(c,0); // Hallar los rectangulos para establecer las ROIs
		//verMatrizIm(pesos, rect);
		if( areaFG <= 5 ) continue;
		//reserva de espacio para los datos del blob

		flyData = parametrizarFly( rect, pesos, FG, NumFrame);
		// si no se consigue parametrizar pasamos al siguiente contorno
		if( !flyData ) continue;

		// error de ajuste

		err = abs(flyData->areaElipse - areaFG);
		// Añadir a lista
		if( flyData ) insertar( flyData, flies );
		if( SHOW_SEGMENTATION_MATRIX == 1){
			printf("\n BLOB %d\n",id);
			id++; //incrmentar el Id de las moscas
			CvRect ventana;// = cvRect(137,261,30,14);	// 321,113,17,14
			ventana = rect;
			printf("\n\n Matriz Brillo ");
			verMatrizIm(Brillo, ventana);
			printf("\n\n Matriz BGModel ");
			verMatrizIm(BGModel, ventana);
			printf("\n\n Matriz pesos(p) = |I(p)-u(p)|/0(p) ");
			verMatrizIm(pesos, ventana);
		}
		if (SHOW_SEGMENTATION_DATA == 1){
			printf("\n AreaElipse = %f  Area FG = %f  Error del ajuste = %f", areaElipse,areaFG,err);
			printf("\n\nElipse\nEJE MAYOR : %f EJE MENOR: %f ORIENTACION: %f ",
					2*flyData->a,
					2*flyData->b,
					flyData->orientacion);
		}
		//verMatrizIm(pesos, Roi);
		// dibujar la máscara
		if( Mask != NULL ){
		 // corrige el origen de ángulos
			float angle;
			if ( flyData->orientacion >=0 && flyData->orientacion < 180) angle = 180 - flyData->orientacion;
			else angle = (360-flyData->orientacion)+180;
			CvSize axes = cvSize( cvRound(flyData->a) , cvRound(flyData->b) );
			cvEllipse( Mask, flyData->posicion , axes, angle, 0, 360, cvScalar( 255,0,0,0), -1, 8);
		}

	}// Fin de contornos

	cvResetImageROI( Brillo );
	cvResetImageROI( BGModel );
	cvResetImageROI( FG );
	cvResetImageROI( FGTemp );
	cvResetImageROI( pesos );

	return flies;

}//Fin segmentacion2

// realiza el ajuste a una elipse de los pesos del rectangulo que coincidan con la máscara
// y rellena los datos de la estructura fly. Si no es posible el ajuste, devuelve null
STFly* parametrizarFly( CvRect rect,IplImage* pesos, IplImage* mask, int num_frame){

	STFly* fly = NULL;

	fly = ( STFly *) malloc( sizeof( STFly));
	if ( !fly ) {error(4);	exit(1);}


	if( ellipseFit( rect, pesos, mask,
					&fly->b,  // semieje menor
					&fly->a,  // senueje mayor
					&fly->posicion, // centro elipse
					&fly->orientacion ) ) // orientación en grados 0-360
	{

		fly->Tracks = ( tlcde * )malloc( sizeof(tlcde ));
		fly->Stats = ( STStatFly * )malloc( sizeof(STStatFly ));
		iniciarLcde( fly->Tracks );
		fly->siguiente = NULL;
		fly->etiqueta = -1; // Identificación del blob
		fly->Color = cvScalar( 0,0,0,0); // Color para dibujar el blob
//		fly->FrameCount = 0;
//		fly->StaticFrames = 0;
		fly->direccion = 0;
		fly->dstTotal = 0;
	//		fly->perimetro = cv::arcLength(contorno,0);
		fly->Roi = rect;
		fly->Estado = 1;  // Flag para indicar que si el blob permanece estático ( 0 ) o en movimiento (1)
		fly->num_frame = num_frame;
		fly->salto = false;	//!< Indica que la mosca ha saltado

		fly->Zona = 0; //!< Si se seleccionan zonas de interes en el plato,
		fly->failSeg = false;
		fly->flag_seg = false;
		fly->Px = 0;
		fly->areaElipse = CV_PI*fly->b*fly->a;

		fly->dir_filtered = 0;

		return fly;
	}
	else{
		free(fly);
		fly = NULL;
		return fly;
	}

}

// se intenta ajustar el contorno de la roi a una elipse. si no se puede, se devuelve 0.
int ellipseFit( CvRect rect,IplImage* pesos, IplImage* mask,
		float *semiejemenor,float* semiejemayor,CvPoint* centro,float* tita ){

	CvScalar v;
	CvScalar d1,d2; // valores de la matriz diagonal de eigen valores,semiejes de la elipse.
	CvScalar r1,r2; // valores de la matriz de los eigen vectores, orientación.

	float z=0;  // parámetro para el cálculo de la matriz de covarianza

	// Inicializar matrices a 0
	cvZero( vector_u);
	cvZero( vector_resta);
	cvZero( matrix_mul);
	cvZero( MATRIX_C);
	cvZero( evects);
	cvZero( evals);
	cvZero( Diagonal);
	cvZero( R);
	cvZero( RT);


	for (int y = rect.y; y< rect.y + rect.height; y++){
		uchar* ptr0 = (uchar*) ( mask->imageData + y*mask->widthStep + rect.x);
		uchar* ptr1 = (uchar*) ( pesos->imageData + y*pesos->widthStep + rect.x);
		for (int x = 0; x < rect.width; x++){
			if( ptr0[x] == 255){
				z = z + ptr1[x]; // Sumatorio de los pesos
				*((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) = CV_MAT_ELEM( *vector_u, float, 0,0 )+ (x + rect.x)*ptr1[x];
				*((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) = CV_MAT_ELEM( *vector_u, float, 1,0 )+ y*ptr1[x];
						//vector_u[0][0] = vector_u[0][0] + x*ptr2[x]; // sumatorio del peso por la pos x
						//vector_u[1][0] = vector_u[1][0] + y*ptr2[x]; // sumatorio del peso por la pos y
			}
		}
	}
	if (SHOW_SEGMENTATION_DATA == 1){
				printf( "\n\nSumatorio de pesos:\n Z = %f",z);
				printf( "\n Sumatorio (wi*pi) = [ %f , %f ]",
						CV_MAT_ELEM( *vector_u, float, 0,0 ),
						CV_MAT_ELEM( *vector_u, float, 1,0 ));
	}
	if ( z != 0) cvConvertScale(vector_u, vector_u, 1/z,0); // vector de media {ux, uy}
	else {
		error(5);
		return 0;
	}
	if (SHOW_SEGMENTATION_DATA == 1){
		printf("\n\nCentro (vector u)\n u = [ %f , %f ]\n",CV_MAT_ELEM( *vector_u, float, 0,0 ),CV_MAT_ELEM( *vector_u, float, 1,0 ));
	}
	for (int y = rect.y; y< rect.y + rect.height; y++){
		uchar* ptr0 = (uchar*) ( mask->imageData + y*mask->widthStep + 1*rect.x);
		uchar* ptr1 = (uchar*) ( pesos->imageData + y*pesos->widthStep + 1*rect.x);
		for (int x= 0; x<rect.width; x++){

			if ( ptr0[x] == 255 ){
				//vector_resta[0][0] = x - vector_u[0][0];
				//vector_resta[1][0] = y - vector_u[1][0];
				*((float*)CV_MAT_ELEM_PTR( *vector_resta, 0, 0 )) = (x + rect.x) - CV_MAT_ELEM( *vector_u, float, 0,0 );
				*((float*)CV_MAT_ELEM_PTR( *vector_resta, 1, 0 )) = y - CV_MAT_ELEM( *vector_u, float, 1,0 );
				cvGEMM(vector_resta, vector_resta, ptr1[x] , NULL, 0, matrix_mul,CV_GEMM_B_T);// Multiplicar Matrices (pi-u)(pi-u)T
				cvAdd( MATRIX_C, matrix_mul	, MATRIX_C ); // sumatorio
			}
		}
	}
	cvConvertScale(MATRIX_C, MATRIX_C, 1/z,0); // Matriz de covarianza

	// Mostrar matriz de covarianza

	if (SHOW_SEGMENTATION_MATRIX == 1) {
		printf("\nMatriz de covarianza");
		for(int i=0;i<2;i++){
			printf("\n\n");
			for(int j=0;j<2;j++){
				v=cvGet2D(MATRIX_C,i,j);
				printf("\t%f",v.val[0]);
			}
		}
	}

	// EXTRAER LOS EIGENVALORES Y EIGENVECTORES
	cvEigenVV(MATRIX_C,evects,evals,2);// Hallar los EigenVectores
	cvSVD(MATRIX_C,Diagonal,R,RT,0); // Hallar los EigenValores, MATRIX_C=R*Diagonal*RT

	//Extraer valores de los EigenVectores y EigenValores

	d1=cvGet2D(Diagonal,0,0);
	d2=cvGet2D(Diagonal,1,1);
	r1=cvGet2D(R,0,0);
	r2=cvGet2D(R,0,1);

	//Hallar los semiejes y la orientación
	//tita varia entre 0 y 359.99999999 [0,360)
	*semiejemayor=2*(sqrt(d1.val[0]));
	*semiejemenor=2*(sqrt(d2.val[0]));
	*tita = 0;
	float cost = r1.val[0];
	float sint = r2.val[0];

	if(( sint == 0)&&( cost == 0)) *tita = 0;
	else{
		*tita=atan2( sint,cost );
		*tita = *tita * 180 / CV_PI;//a grados
		if (*tita < 0 ) *tita = *tita + 360; // tita [0 , 360)
		if (*tita == 360) *tita = 0;
		//corregimos ángulo para el origen del sistema cartesiano de derecha a izquierda
		if ( *tita >=0 && *tita < 180)  *tita = 180 - *tita;
		else *tita = ( 360-*tita)+180;
	}
	// Obtenemos el centro de la elipse
	*centro = cvPoint( cvRound( *((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) ),
			cvRound( *((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) ) );

	return 1;
}


void CreateDataSegm( IplImage* Brillo ){
	CvSize size = cvSize(Brillo->width,Brillo->height); // get current frame size

	// crear imagenes si no se ha hecho o redimensionarlas si cambia su tamaño
	if( !pesos || pesos->width != size.width || pesos->height != size.height ) {

			// CREAR IMAGENES
			cvReleaseImage( &pesos );

			FGTemp = cvCreateImage(size, IPL_DEPTH_8U, 1);
			pesos=cvCreateImage(size, IPL_DEPTH_8U, 1);//Imagen resultado wi ( pesos)

			// CREAR MATRICES
			vector_u=cvCreateMat(2,1,CV_32FC1); // Matriz de medias
			vector_resta=cvCreateMat(2,1,CV_32FC1); // (pi-u)
			matrix_mul=cvCreateMat(2,2,CV_32FC1);// Matriz (pi-u)(pi-u)T
			MATRIX_C=cvCreateMat(matrix_mul->rows,matrix_mul->cols,CV_32FC1);// MATRIZ DE COVARIANZA
			evects=cvCreateMat(2,2,CV_32FC1);// Matriz de EigenVectores
			evals=cvCreateMat(2,2,CV_32FC1);// Matriz de EigenValores

			Diagonal=cvCreateMat(2,2,CV_32FC1); // Matriz Diagonal,donde se extraen los ejes
			R=cvCreateMat(2,2,CV_32FC1);// Matriz EigenVectores.
			RT=cvCreateMat(2,2,CV_32FC1);

			//Crear storage y secuencia de los contornos

			storage = cvCreateMemStorage();
	}
	first_contour=NULL;
	cvZero( FGTemp);
	cvZero(pesos);

}


void ReleaseDataSegm( ){
	cvReleaseImage( &FGTemp);

	cvReleaseImage(&pesos);

	cvReleaseMemStorage( &storage);

	cvReleaseMat(&vector_resta);
	cvReleaseMat(&matrix_mul);
	cvReleaseMat(&MATRIX_C);
	cvReleaseMat(&evects);
	cvReleaseMat(&evals);
	cvReleaseMat(&Diagonal);
	cvReleaseMat(&R);
	cvReleaseMat(&RT);

}

void establecerROIS( STFrame* FrameData, IplImage* Brillo,CvRect Roi ){
	cvSetImageROI( Brillo , Roi);
	cvSetImageROI( FrameData->BGModel, Roi );
	cvSetImageROI( FrameData->FG, Roi );
	cvSetImageROI( FGTemp, Roi );
	cvSetImageROI( pesos, Roi );
}

void resetearROIS( STFrame* FrameData, IplImage* Brillo ){
	cvResetImageROI( Brillo );
	cvResetImageROI( FrameData->BGModel );
	cvResetImageROI( FrameData->FG );
	cvResetImageROI( FGTemp );

	cvResetImageROI( pesos );

}



// Segmentación optimizada para modelo gaussiano

/*!
 * \note wi = pesos = distancia normalizada de cada pixel a su modelo de fondo.
	\f[

		w_i=\frac{|I(p_i)-u(p_i)|}{\sigma(p_i)}

	\f]

	\f[
		Z=\sum_{i}^{n}(w_i)
	\f]

	\f[
		u=\frac{1}{z}\sum_{i}^{n}(w_i p_i)
	\f]

 * \note MATRIX_C = Matriz de covarianza.
 *
	 \f[
	 \sum=\frac{1}{z}\sum_{i}^{n}(w_i(p_i - u)(p_i - u)^T)
	 \f]
 * \note  Los datos de la elipse se obtienen a partir de los eigen valores y eigen vectores
 * \note de la matriz de covarianza.
 * \n
 * \n
 * 		\f[
 * 		MatrizCovarianza=R * Diagonal * R^T
 * 		\f]
 * \note R = Matriz EigenVectores, de ella obtenemos la orientación.
 * \note evals = Matriz de EigenValores, de ella obtenemos ejes.
 * \note Diagonal = Matriz Diagonal,de ella obtenemos igualmente los ejes.
 */

//IplImage *IDif = 0;
//IplImage *IDifF = 0;

//tlcde* segmentacion( IplImage *Brillo, STFrame* FrameData ,CvRect Roi,IplImage* Mask){
//
//	struct timeval ti, tf,tif,tff; // iniciamos la estructura
//	float tiempoParcial;
//	//Iniciar lista para almacenar las moscas
//	tlcde* flies = NULL;
//	flies = ( tlcde * )malloc( sizeof(tlcde ));
//	iniciarLcde( flies );
//	//Inicializar estructura para almacenar los datos cada mosca
//	STFly *flyData = NULL;
//
//	// crear imagenes y datos si no se ha hecho e inicializar
//	CreateDataSegm( Brillo );
//	cvCopy(FrameData->FG,FGTemp);
//	establecerROIS( FrameData, Brillo, Roi );
//	cvZero( Mask);
//
//	// Distancia de cada pixel a su modelo de fondo.
////		cvShowImage("Foreground",FGTemp);
////				cvWaitKey(0);
//	#ifdef MEDIR_TIEMPOS gettimeofday(&ti, NULL);
//
//	cvAbsDiff(Brillo,FrameData->BGModel,IDif);// |I(p)-u(p)|
//	if( SHOW_SEGMENTATION_MATRIX == 1){
//				printf("\n ANTES \n");
//
//				CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
//				//ventana = rect;
//				printf("\n\n Matriz Brillo ");
//				verMatrizIm(Brillo, ventana);
//				printf("\n\n Matriz BGModel ");
//				verMatrizIm(FrameData->BGModel, ventana);
//				printf("\n\n Matriz IdifF(p) = Brillo(p)-BGModel(p) ");
//				verMatrizIm(IDifF, ventana);
//				printf("\n\n Matriz Idesv ");
//				verMatrizIm(FrameData->IDesvf, ventana);
//				printf("\n\n Matriz pesos(p) = |I(p)-u(p)|/0(p) ");
//				verMatrizIm(pesos, ventana);
//			}
//
//	cvConvertScale(IDif ,IDifF,1,0);// A float
//	cvDiv( IDifF,FrameData->IDesvf,pesos );// Calcular |I(p)-u(p)|/0(p)
//
//	#ifdef MEDIR_TIEMPOS{ gettimeofday(&tf, NULL);
//		tiempoParcial= (tf.tv_sec - ti.tv_sec)*1000 + \
//									(tf.tv_usec - ti.tv_usec)/1000.0;
//		printf("\t\t0)Calculo de |I(p)-u(p)|/0(p) : %5.4g ms\n", tiempoParcial);
//		printf("\t\t1)Ajuste de blobs a elipses\n");
//	}
//
//	if( SHOW_SEGMENTATION_MATRIX == 1){
//				printf("\n DESPUES \n");
//				CvRect ventana = cvRect(137,261,30,14);	// 321,113,17,14
//				printf("\n\n Matriz Brillo ");
//				verMatrizIm(Brillo, ventana);
//				printf("\n\n Matriz BGModel ");
//				verMatrizIm(FrameData->BGModel, ventana);
//				printf("\n\n Matriz IdifF(p) = Brillo(p)-BGModel(p) ");
//				verMatrizIm(IDifF, ventana);
//				printf("\n\n Matriz Idesv ");
//				verMatrizIm(FrameData->IDesvf, ventana);
//				printf("\n\n Matriz pesos(p) = |I(p)-u(p)|/0(p) ");
//				verMatrizIm(pesos, ventana);
//			}
//	//Buscamos los contornos de las moscas en movimiento en el foreground
//
//	int Nc = cvFindContours(FGTemp,
//			storage,
//			&first_contour,
//			sizeof(CvContour),
//			CV_RETR_EXTERNAL,
//			CV_CHAIN_APPROX_NONE,
//			cvPoint(Roi.x,Roi.y));
//	int id=0; // contador de blobs
//	if( SHOW_SEGMENTATION_DATA ) printf( "\nTotal Contornos Detectados: %d ", Nc );
//
//	for( CvSeq *c=first_contour; c!=NULL; c=c->h_next) {
//		// Parámetros para calcular el error del ajuste en base a la diferencia de areas entre el Fg y la ellipse
//		float err;
//		float areaFG;
//		float areaElipse;
//		areaFG = cvContourArea(c);
//		id++;
//		flyData = ( STFly *) malloc( sizeof( STFly));
//		if ( !flyData ) {error(4);	exit(1);}
//
//		CvRect rect=cvBoundingRect(c,0); // Hallar los rectangulos para establecer las ROIs
//		// realiza el ajuste a una elipse de los pesos del rectangulo que coincidan con la máscara
//		#ifdef MEDIR_TIEMPOS gettimeofday(&ti, NULL);
//		ellipseFit( rect, pesos, FrameData->FG,
//				&flyData->b,  // semieje menor
//				&flyData->a,  // senueje mayor
//				&flyData->posicion, // centro elipse
//				&flyData->orientacion ); // orientación en grados 0-360
//		#ifdef MEDIR_TIEMPOS{ gettimeofday(&tf, NULL);
//				tiempoParcial= (tf.tv_sec - ti.tv_sec)*1000 + \
//											(tf.tv_usec - ti.tv_usec)/1000.0;
//				printf("\t\t  -Ajuste a elipse de blob %d: %5.4g ms\n",id,tiempoParcial);
//			}
//		areaElipse = CV_PI*flyData->b*flyData->a;
//		err = abs(areaElipse - areaFG);
//		if( SHOW_SEGMENTATION_MATRIX == 1){
//			printf("\n BLOB %d\n",id);
//			id++; //incrmentar el Id de las moscas
//			CvRect ventana;// = cvRect(137,261,30,14);	// 321,113,17,14
//			ventana = rect;
//			printf("\n\n Matriz Brillo ");
//			verMatrizIm(Brillo, ventana);
//			printf("\n\n Matriz BGModel ");
//			verMatrizIm(FrameData->BGModel, ventana);
//			printf("\n\n Matriz IdifF(p) = Brillo(p)-BGModel(p) ");
//			verMatrizIm(IDifF, ventana);
//			printf("\n\n Matriz Idesv ");
//			verMatrizIm(FrameData->IDesvf, rect);
//			printf("\n\n Matriz pesos(p) = |I(p)-u(p)|/0(p) ");
//			verMatrizIm(pesos, ventana);
//		}
//		if (SHOW_SEGMENTATION_DATA == 1){
//			printf("\n AreaElipse = %f  Area FG = %f  Error del ajuste = %f", areaElipse,areaFG,err);
//			printf("\n\nElipse\nEJE MAYOR : %f EJE MENOR: %f ORIENTACION: %f ",
//					2*flyData->a,
//					2*flyData->b,
//					flyData->orientacion);
//		}
//
//		flyData->etiqueta = 0; // Identificación del blob
//		flyData->Color = cvScalar( 0,0,0,0); // Color para dibujar el blob
//
//		flyData->StaticFrames = 0;
//		flyData->direccion = 0;
//		flyData->dstTotal = 0;
////		flyData->perimetro = cv::arcLength(contorno,0);
//		flyData->Roi = rect;
//		flyData->Estado = 1;  // Flag para indicar que si el blob permanece estático ( 0 ) o en movimiento (1)
//		flyData->num_frame = FrameData->num_frame;
//		// Añadir a lista
//		insertar( flyData, flies );
//
//		//Mostrar los campos de cada mosca o blob
//		if(SHOW_SEGMENTATION_DATA == 1){
//		printf("\n EJE A : %f\t EJE B: %f\t ORIENTACION: %f",flyData->a,flyData->b,flyData->orientacion);
//		}
//
//		if( Mask != NULL ){
//		 // para evitar el offset de find contours
//		float angle;
//		if ( fly->orientacion >=0 && fly->orientacion < 180) angle = 180 - fly->orientacion;
//		else angle = (360-fly->orientacion)+180;
//		CvSize axes = cvSize( cvRound(flyData->a) , cvRound(flyData->b) );
//		cvEllipse( Mask, flyData->posicion , axes, flyData->orientacion, 0, 360, cvScalar( 255,0,0,0), -1, 8);
//		}
//
//	}// Fin de contornos
//
//	resetearROIS( FrameData, Brillo );
//
//	return flies;
//
//}//Fin de la función
//
//
//
//void ellipseFit( CvRect rect,IplImage* pesos, IplImage* mask,
//		float *semiejemenor,float* semiejemayor,CvPoint* centro,float* tita ){
//
//	CvScalar v;
//	CvScalar d1,d2; // valores de la matriz diagonal de eigen valores,semiejes de la elipse.
//	CvScalar r1,r2; // valores de la matriz de los eigen vectores, orientación.
//
//	float z=0;  // parámetro para el cálculo de la matriz de covarianza
//
//	// Inicializar matrices a 0
//	cvZero( vector_u);
//	cvZero( vector_resta);
//	cvZero( matrix_mul);
//	cvZero( MATRIX_C);
//	cvZero( evects);
//	cvZero( evals);
//	cvZero( Diagonal);
//	cvZero( R);
//	cvZero( RT);
//
//	int step1   = pesos->widthStep/sizeof(float);
//	int step2	= mask->widthStep/sizeof(uchar);
//
//	float* ptr1 = (float *)pesos->imageData;
//	uchar* ptr2 = (uchar*) mask->imageData;
//	// Hallar Z y u={ux,uy}
//	for (int i = rect.y; i< rect.y+rect.height; i++){
//		for (int j = rect.x; j < rect.x+rect.width; j++){
//			if( ptr2[i*step2+j] == 255){
//				z = z + ptr1[i*step1+j]; // Sumatorio de los pesos
//				*((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) = CV_MAT_ELEM( *vector_u, float, 0,0 )+ j*ptr1[i*step1+j];
//				*((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) = CV_MAT_ELEM( *vector_u, float, 1,0 )+ i*ptr1[i*step1+j];
//				//vector_u[0][0] = vector_u[0][0] + x*ptr2[x]; // sumatorio del peso por la pos x
//				//vector_u[1][0] = vector_u[1][0] + y*ptr2[x]; // sumatorio del peso por la pos y//
//			}
//		}
//	}
////	for (int y = rect.y; y< rect.y + rect.height; y++){
////		uchar* ptr0 = (uchar*) ( mask->imageData + y*mask->widthStep + rect.x);
////		const float* ptr1 = (const float*) ( pesos->imageData + y*pesos->widthStep + rect.x);
////		for (int x = 0; x < rect.width; x++){
////			if( ptr0[x] == 255){
////				z = z + ptr1[x]; // Sumatorio de los pesos
////				*((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) = CV_MAT_ELEM( *vector_u, float, 0,0 )+ (x + rect.x)*ptr1[x];
////				*((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) = CV_MAT_ELEM( *vector_u, float, 1,0 )+ y*ptr1[x];
////						//vector_u[0][0] = vector_u[0][0] + x*ptr2[x]; // sumatorio del peso por la pos x
////						//vector_u[1][0] = vector_u[1][0] + y*ptr2[x]; // sumatorio del peso por la pos y
////			}
////		}
////	}
//	if (SHOW_SEGMENTATION_DATA == 1){
//				printf( "\n\nSumatorio de pesos:\n Z = %f",z);
//				printf( "\n Sumatorio (wi*pi) = [ %f , %f ]",
//						CV_MAT_ELEM( *vector_u, float, 0,0 ),
//						CV_MAT_ELEM( *vector_u, float, 1,0 ));
//	}
//	if ( z != 0) cvConvertScale(vector_u, vector_u, 1/z,0); // vector de media {ux, uy}
//	else error(5);
//	if (SHOW_SEGMENTATION_DATA == 1){
//		printf("\n\nCentro (vector u)\n u = [ %f , %f ]\n",CV_MAT_ELEM( *vector_u, float, 0,0 ),CV_MAT_ELEM( *vector_u, float, 1,0 ));
//	}
//
//	for (int i = rect.y; i< rect.y+rect.height; i++){
//		for (int j = rect.x; j < rect.x+rect.width; j++){
//			if( ptr2[i*step2+j] == 255){
//				//vector_resta[0][0] = x - vector_u[0][0];
//				//vector_resta[1][0] = y - vector_u[1][0];
//				*((float*)CV_MAT_ELEM_PTR( *vector_resta, 0, 0 )) = j - CV_MAT_ELEM( *vector_u, float, 0,0 );
//				*((float*)CV_MAT_ELEM_PTR( *vector_resta, 1, 0 )) = i - CV_MAT_ELEM( *vector_u, float, 1,0 );
//				cvGEMM(vector_resta, vector_resta, ptr1[i*step1 +j] , NULL, 0, matrix_mul,CV_GEMM_B_T);// Multiplicar Matrices pesos*(pi-u)(pi-u)T
//				cvAdd( MATRIX_C, matrix_mul	, MATRIX_C ); // sumatorio
////				if (SHOW_SEGMENTATION_DATA == 1) {
////						printf("\nMatriz de covarianza");
////						for(int i=0;i<2;i++){
////							printf("\n\n");
////							for(int j=0;j<2;j++){
////								v=cvGet2D(MATRIX_C,i,j);
////								printf("\t%f",v.val[0]);
////							}
////				        }
////					}
//			}
//		}
//	}
////	for (int y = rect.y; y< rect.y + rect.height; y++){
////		uchar* ptr0 = (uchar*) ( mask->imageData + y*mask->widthStep + 1*rect.x);
////		const float* ptr1 = (const float*) ( pesos->imageData + y*pesos->widthStep + 1*rect.x);
////		for (int x = 0; x < rect.width; x++){
////			if( ptr0[x] == 255){
////				z = z + ptr1[x]; // Sumatorio de los pesos
////				*((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) = CV_MAT_ELEM( *vector_u, float, 0,0 )+ (x + rect.x)*ptr1[x];
////				*((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) = CV_MAT_ELEM( *vector_u, float, 1,0 )+ y*ptr1[x];
////						//vector_u[0][0] = vector_u[0][0] + x*ptr2[x]; // sumatorio del peso por la pos x
////						//vector_u[1][0] = vector_u[1][0] + y*ptr2[x]; // sumatorio del peso por la pos y
////
////			}
////		}
////	}
//	if ( z != 0) cvConvertScale(MATRIX_C, MATRIX_C, 1/z,0); // Matriz de covarianza
//
//	// Mostrar matriz de covarianza
//
//	if (SHOW_SEGMENTATION_MATRIX == 1) {
//		printf("\nMatriz de covarianza");
//		for(int i=0;i<2;i++){
//			printf("\n\n");
//			for(int j=0;j<2;j++){
//				v=cvGet2D(MATRIX_C,i,j);
//				printf("\t%f",v.val[0]);
//			}
//        }
//	}
//
//	// EXTRAER LOS EIGENVALORES Y EIGENVECTORES
//
//	cvEigenVV(MATRIX_C,evects,evals,2);// Hallar los EigenVectores
//	cvSVD(MATRIX_C,Diagonal,R,RT,0); // Hallar los EigenValores, MATRIX_C=R*Diagonal*RT
//
//	//Extraer valores de los EigenVectores y EigenValores
//
//	d1=cvGet2D(Diagonal,0,0);
//	d2=cvGet2D(Diagonal,1,1);
//	r1=cvGet2D(R,0,0);
//	r2=cvGet2D(R,0,1);
//
//	//Hallar los semiejes y la orientación
//	// Tita valdra entre 0 y360ª
//
//	*semiejemayor=2*(sqrt(d1.val[0]));
//	*semiejemenor=2*(sqrt(d2.val[0]));
//	*tita = 0;
//	float cost = r1.val[0];
//	float sint = r2.val[0];
//
//	if(( sint == 0)&&( cost == 0)) *tita = 0;
//	else{
//		*tita=atan2( sint,cost );
//		*tita = *tita * 180 / CV_PI;
//		if (*tita < 0 ) *tita = *tita + 360;
//		if (*tita == 360) *tita = 0; // tita varia entre 0 y 359
//	}
//
//	// Obtenemos el centro de la elipse
//
//	*centro = cvPoint( cvRound( *((float*)CV_MAT_ELEM_PTR( *vector_u, 0, 0 )) ),
//			cvRound( *((float*)CV_MAT_ELEM_PTR( *vector_u, 1, 0 )) ) );
//}
//
//
//void CreateDataSegm( IplImage* Brillo ){
//	CvSize size = cvSize(Brillo->width,Brillo->height); // get current frame size
//
//	// crear imagenes si no se ha hecho o redimensionarlas si cambia su tamaño
//	if( !IDif || IDif->width != size.width || IDif->height != size.height ) {
//		    // CREAR IMAGENES
//	        cvReleaseImage( &IDif );
//	        cvReleaseImage( &IDifF );
//	        cvReleaseImage( &pesos );
//
//	        FGTemp = cvCreateImage(size, IPL_DEPTH_8U, 1);
//	        IDif=cvCreateImage(size, IPL_DEPTH_8U, 1); // imagen diferencia abs(I(pi)-u(p(i))
//	        IDifF=cvCreateImage(size, IPL_DEPTH_32F, 1);// IDif en punto flotante
//	        pesos=cvCreateImage(size, IPL_DEPTH_32F, 1);//Imagen resultado wi ( pesos)
//
//
//			// CREAR MATRICES
//
//			vector_u=cvCreateMat(2,1,CV_32FC1); // Matriz de medias
//			vector_resta=cvCreateMat(2,1,CV_32FC1); // (pi-u)
//			matrix_mul=cvCreateMat(2,2,CV_32FC1);// Matriz (pi-u)(pi-u)T
//			MATRIX_C=cvCreateMat(matrix_mul->rows,matrix_mul->cols,CV_32FC1);// MATRIZ DE COVARIANZA
//			evects=cvCreateMat(2,2,CV_32FC1);// Matriz de EigenVectores
//			evals=cvCreateMat(2,2,CV_32FC1);// Matriz de EigenValores
//
//			Diagonal=cvCreateMat(2,2,CV_32FC1); // Matriz Diagonal,donde se extraen los ejes
//			R=cvCreateMat(2,2,CV_32FC1);// Matriz EigenVectores.
//			RT=cvCreateMat(2,2,CV_32FC1);
//
//			//Crear storage y secuencia de los contornos
//
//			storage = cvCreateMemStorage();
//	}
//	first_contour=NULL;
//	cvZero( FGTemp);
//    cvZero( IDif);
//    cvZero( IDifF);
//    cvZero(pesos);
//
//}
//
//
//void ReleaseDataSegm( ){
//	cvReleaseImage( &FGTemp);
//	cvReleaseImage(&IDif);
//	cvReleaseImage(&IDifF);
//	cvReleaseImage(&pesos);
//
//	cvReleaseMemStorage( &storage);
//
//	cvReleaseMat(&vector_resta);
//	cvReleaseMat(&matrix_mul);
//	cvReleaseMat(&MATRIX_C);
//	cvReleaseMat(&evects);
//	cvReleaseMat(&evals);
//	cvReleaseMat(&Diagonal);
//	cvReleaseMat(&R);
//	cvReleaseMat(&RT);
//
//}
//
//void establecerROIS( STFrame* FrameData, IplImage* Brillo,CvRect Roi ){
//	cvSetImageROI( Brillo , Roi);
//	cvSetImageROI( FrameData->BGModel, Roi );
//	cvSetImageROI( FrameData->IDesvf, Roi );
//	cvSetImageROI( FrameData->FG, Roi );
//	cvSetImageROI( FGTemp, Roi );
//
//	cvSetImageROI( IDif, Roi );
//	cvSetImageROI( IDifF, Roi );
//	cvSetImageROI( pesos, Roi );
//
//
//}
//
//void resetearROIS( STFrame* FrameData, IplImage* Brillo ){
//	cvResetImageROI( Brillo );
//	cvResetImageROI( FrameData->BGModel );
//	cvResetImageROI( FrameData->IDesvf );
//	cvResetImageROI( FrameData->FG );
//	cvResetImageROI( FGTemp );
//
//	cvResetImageROI( IDif );
//	cvResetImageROI( IDifF);
//	cvResetImageROI( pesos );
//
//}
