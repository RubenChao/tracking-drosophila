/*
 * Inicializacion.cpp
 *
 *  Created on: 31/01/2012
 *      Author: chao
 */

#include "Inicializacion.hpp"

int Inicializacion( int argc,
		char* argv[],
		char* nombreFichero,
		char* nombreVideo ){


	/// Crear fichero de datos.
	/// si no se especifican nombres en la ejecución establecerlos por defecto

	if( argc == 2 ){
		argc = 4;
		printf("\nNo se ha especificado nombre para el fichero de Datos.\n ");
		for( int i = 0; i < 1000; i++ ){
			sprintf(nombreFichero,"Data_%d.csv",i);
			if( !existe( nombreFichero) ) {
				crearFichero( nombreFichero );
				printf("Se estableció por defecto %s",nombreFichero);
				break;
			}
		}
		printf("\nNo se ha especificado nombre para el fichero de video.\n ");
		for( int i = 0; i < 1000; i++ ){
			sprintf(nombreVideo,"Visual_%d.avi",i);
			if( !existe( nombreVideo) ){
				printf("Se estableció por defecto %s",nombreVideo);
				break;
			}
		}
	}
	// si se especifica nombre de fichero de datos y no de video
	else if( argc == 3 ){
		//añadir extensiones
		int pos;
		char CSV[4];
		char resp[2];
		char* pdest;
		strcpy(CSV, ".csv");

		strcpy(nombreFichero, argv[2]);


		pdest = strrchr( nombreFichero, '.');
		pos = pdest-nombreFichero;
		if( pdest == NULL ){
			strncat(nombreFichero,CSV, 4);
		}
		else{
			int max = strlen(nombreFichero);
			for(int i = pos; i < max ; i++)
				nombreFichero [i] = 0;

			strncat(nombreFichero,CSV, 4);
		}
		// verificar si el nombre del archivo de datos existe.
		if( existe(nombreFichero) ){
			int hecho = 0;
			do
			{
				printf("\n\nEl fichero de datos %s existe. Indique si desea sobrescribirlo (s/n): ",nombreFichero);
				fscanf( stdin, "%s",resp);
				fflush(stdin);
				QuitarCR( resp );
				if (resp[0] == 's'|| resp[0] =='S'){
					crearFichero(nombreFichero);
					hecho = 1;
				}
				else if (resp[0] == 'n'||resp[0] =='N'){

					printf("\nEscriba el nuevo nombre o pulse intro para nombre por defecto: ");
					fscanf(stdin,"%s",nombreFichero );
					fflush(stdin);
					QuitarCR( nombreFichero );
					// añadir extensión csv si no se ha hecho.

					pdest = strrchr( nombreFichero, '.');
					pos = pdest-nombreFichero;
					if( pdest == NULL ){
						strncat(nombreFichero,CSV, 4);
					}
					else{
						int max = strlen(nombreFichero);
						for(int i = pos; i < max ; i++)
							nombreFichero [i] = 0;

						strncat(nombreFichero,CSV, 4);
					}

					if (strlen(nombreFichero)<5) {// nombre por defecto
						for( int i = 0; i < 1000; i++ ){
							printf("\nNombre por defecto: ");
							sprintf(nombreFichero,"Data_%d.csv ",i);
							if( !existe( nombreFichero) ) {
								crearFichero( nombreFichero );
								printf("%s",nombreFichero);
								break;
							}
						}
						hecho = 1;
					}
					if (existe(nombreFichero)) hecho = 0;
					else hecho = 1;
				}
				else printf("\nResponda s/n.\n");

			}
			while (!hecho);
		}
		else{
			crearFichero( nombreFichero );
		}
		// creamos el fichero de video.
//		int hecho = 0;
		printf(" No se ha especificado nombre para el fichero de video.\n ");

//		do{
//			fscanf( stdin, "%s",&resp);
//			fflush(stdin);
//			QuitarCR( resp );
//			if (resp[0] == 's'|| resp[0] =='S'){
//				GRABAR_VISUALIZACION = 1;
//				for( int i = 0; i < 1000; i++ ){
//					sprintf(nombreVideo,"Visual_%d.avi",i);
//					if( !existe( nombreVideo) )  break;
//				}
//				hecho = 1;
//			}
//			else if (resp[0] == 'n'||resp[0] =='N'){
//				GRABAR_VISUALIZACION = 0;
//				hecho = 1;
//			}
//			else printf("Respuesta incorrecta. Indique s/n:");
//		}
//		while (!hecho);
		for( int i = 0; i < 1000; i++ ){
			sprintf(nombreVideo,"Visual_%d.avi",i);
			printf("Se estableció por defecto %s",nombreVideo);
			if( !existe( nombreVideo) )  break;
		}
	} // si se especifica nombre de fichero de datos y video
	else if( argc == 4 ){
		//añadir extensiones
		int pos;
		char CSV[4];
		char AVI[4];
		char* pdest;
		strcpy(CSV, ".csv");
		strcpy(AVI, ".avi");
		strcpy(nombreFichero, argv[2]);
		strcpy(nombreVideo, argv[3]);

		pdest = strrchr( nombreFichero, '.');
		pos = pdest-nombreFichero;
		if( pdest == NULL ){
			strncat(nombreFichero,CSV, 4);
		}
		else{
			int max = strlen(nombreFichero);
			for(int i = pos; i < max ; i++)
				nombreFichero [i] = 0;
			strncat(nombreFichero,CSV, 4);
		}

		pdest = strrchr( nombreVideo, '.');
		pos = pdest-nombreVideo;
		if( pdest == NULL ){
			strncat(nombreVideo,AVI, 4);
		}
		else{
			int max = strlen(nombreVideo);
			for(int i = pos; i < max ; i++)
				nombreVideo [i] = 0;
			strncat(nombreVideo,AVI, 4);
		}
		// verificar si los nombres de los archivos existen.
		if( existe(nombreFichero) ){
			int hecho = 0;
			char resp[2];
			do
			{
				printf("\n\nEl fichero de datos %s existe. Indique si desea sobrescribirlo (s/n): ",nombreFichero);
				fscanf( stdin, "%s",resp);
				fflush(stdin);
				QuitarCR( resp );
				if (resp[0] == 's'|| resp[0] =='S'){
					crearFichero(nombreFichero);
					hecho = 1;
				}
				else if (resp[0] == 'n'||resp[0] =='N'){

					printf("\nEscriba el nuevo nombre o pulse intro para nombre por defecto: ");
					fscanf(stdin,"%s",nombreFichero );
					fflush(stdin);
					QuitarCR( nombreFichero );
					// añadir extensión csv si no se ha hecho.

					pdest = strrchr( nombreFichero, '.');
					pos = pdest-nombreFichero;
					if( pdest == NULL ){
						strncat(nombreFichero,CSV, 4);
					}
					else{
						int max = strlen(nombreFichero);
						for(int i = pos; i < max ; i++)
							nombreFichero [i] = 0;

						strncat(nombreFichero,CSV, 4);
					}

					if (strlen(nombreFichero)<5) {// nombre por defecto
						for( int i = 0; i < 1000; i++ ){
							printf("\nNombre por defecto: ");
							sprintf(nombreFichero,"Data_%d.csv ",i);
							if( !existe( nombreFichero) ) {
								crearFichero( nombreFichero );
								printf("%s",nombreFichero);
								break;
							}
						}
						hecho = 1;
					}
					if (existe(nombreFichero)) hecho = 0;
					else hecho = 1;
				}
				else printf("\nResponda s/n.\n");

			}
			while (!hecho);
		}
		else{
			crearFichero( nombreFichero );
		}

		// verificar si el nombre del fichero de video existe.
		if( existe(nombreVideo) ){
			int hecho = 0;
			char resp[2];
			do
			{
				printf("\nEl fichero de video %s existe. Indique si desea sobrescribirlo (s/n): ",nombreVideo);
				fscanf( stdin, "%s",resp);
				fflush(stdin);
				QuitarCR( resp );
				if (resp[0] == 's'||resp[0] =='S')  hecho = 1;
				else if (resp[0] == 'n'||resp[0] =='N'){

					printf("\nEscriba el nuevo nombre o pulse intro para nombre por defecto: ");
					fscanf(stdin,"%s",nombreVideo );
					fflush(stdin);
					QuitarCR( nombreVideo );
					// añadir extensión avi si no se ha hecho.
					pdest = strrchr( nombreVideo, '.');
					pos = pdest-nombreVideo;
					if( pdest == NULL ){
						strncat(nombreVideo,AVI, 4);
					}
					else{
						int max = strlen(nombreVideo);
						for(int i = pos; i < max ; i++)
							nombreVideo [i] = 0;

						strncat(nombreVideo,AVI, 4);
					}

					if (strlen(nombreVideo)<5) {// nombre por defecto
						printf("\nNombre por defecto: ");
						for( int i = 0; i < 1000; i++ ){
							sprintf(nombreVideo,"Video_%d.avi ",i);
							if( !existe( nombreVideo) ) {
								printf("%s",nombreVideo);
								break;
							}
						}
						hecho = 1;
					} //fin nombre por defecto
					if (existe(nombreVideo)) hecho = 0;
					else hecho = 1;
				}
				else printf("\nResponda s/n.\n");
			}
			while (!hecho);
		}
	}
	else {
		help();
		return 0;
	}


	// Inicializar estructura de analisis estadístico

//	STStatFrame* stats = ( STStatFrame * )malloc( sizeof(STStatFrame));
//	if( !stats ) {error(4);return 0;}
//	*Stats = stats;
//	if( argc == 4 ) setMouseCallback( "CamShift Demo", onMouse, 0 );

	return 1;
}
